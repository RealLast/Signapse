/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.Manifest;
import android.arch.lifecycle.ViewModelProviders;
import android.graphics.Rect;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.Snackbar;
import android.support.v4.app.FragmentActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.util.Size;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.function.Consumer;

/**
 * This {@link TSDFragment} manages the most important part of the application - the detection of
 * signs inside images taken by a back facing camera of the device. For this a
 * {@link TSDShutterCamera} instance gets creates and initialized to not isRelevant anything but
 * to feed the images into a {@link TSDShutterCameraImageListener} which searches the images for
 * traffic signs using the RoadSignAPI. This requires the TrafficSignAPI to already having been
 * initialized. It then adds the {@link TSDSignCombination sign combinations} to a
 * {@link TSDSignHistory} which is tight to the lifecycle of the {@link MainActivity}.
 * This means, the history is maintained until the {@link MainActivity} gets destroyed, not when the
 * {@link TSDDetectorFragment} gets destroyed.
 * The {@link TSDSignHistory} is listened to for changes and the sign combinations in there are
 * displayed to the user interface in two places: At the left side at every time the latest sign
 * combination is shown while at the right side there is an {@link RecyclerView} displayed
 * containing the remaining sign combinations.
 * The fragment triggers the required permission requests and includes an
 * {@link TSDPeriodicTask} which periodically triggers the GC to clean up rubbish produced by
 * the camera beginning with Android Version 8.0 .
 *
 * @image latex app_class_detector.png width=16cm
 * @image html app_class_detector.svg
 */
public class TSDDetectorFragment extends TSDFragment
{
    private final static float MAIN_SIGN_TRANSPARENCY = 1f;

    private FragmentActivity mActivity;

    private TSDDrawableDatabase mDrawableDatabase;
    private TSDSignHistory mSignHistory;

    private TSDSpeedTracker mSpeedTracker;
    private TSDSignCombination mSignCombination;
    private TSDSignCombinationViewHolder mSignViewHolder;
    private TSDPeriodicTask mTimeTrigger;
    private Integer mSpeed;

    private TSDCameraManager mCameraManager;
    private TSDShutterCamera mShutterCamera;
    private TSDPeriodicTask mGcTrigger;

    private TSDPermissionManager.OnRequestPermissionResultListener mCameraResultListener =
            new TSDPermissionManager.OnRequestPermissionResultListener()
            {
                @Override
                public String[] getRequestedPermissions()
                {
                    // Temporarily ask for storage permission, too.
                    return new String[]{ Manifest.permission.CAMERA };
                }

                @Override
                public void onRequestedPermissionsGranted()
                {
                    // Only enable lifecycle objects if permission was granted.
                    mShutterCamera.enable();
                    mGcTrigger.enable();

                    // Ask for location permission and enable locator if granted, if does not
                    // already has disabled speed tracking.
                    if (TSDHelper.readBooleanFromSharedPreferences(R.string.data_gps_key,
                            true, mActivity))
                    {
                        TSDPermissionManager.getPermissions(mActivity, mLocationResultListener,
                                1000);

                        // Show a snackbar informing the user about the start of the camera.
                        Snackbar.make(((MainActivity)mActivity).getContentLayout(),
                                R.string.detection_started, Snackbar.LENGTH_LONG)
                                .show();
                    }
                }

                @Override
                public void onRequestedPermissionsDeclined() { }

                @Override
                public @NonNull String getDeclineText()
                {
                    return mActivity.getString(R.string.camera_permission_decline_text);
                }
            };

    private TSDPermissionManager.OnRequestPermissionResultListener mLocationResultListener =
            new TSDPermissionManager.OnRequestPermissionResultListener()
            {
                @Override
                public String[] getRequestedPermissions()
                {
                    return new String[]{ Manifest.permission.ACCESS_FINE_LOCATION };
                }

                @Override
                public void onRequestedPermissionsGranted()
                {
                    // Start the location listener if the permission was granted.
                    mSpeedTracker.enable();

                    // Write users choice to preferences.
                    TSDHelper.writeBooleanToSharedPreferences(R.string.data_gps_key, true,
                            mActivity);
                }

                @Override
                public void onRequestedPermissionsDeclined()
                {
                    // Write users choice to preferences.
                    TSDHelper.writeBooleanToSharedPreferences(R.string.data_gps_key, false,
                            mActivity);

                    // Set the current speed to null and hide the speed view.
                    mSpeed = null;
                    if (mSignViewHolder != null) mSignViewHolder.displaySpeed(null);
                }

                @Override
                public @NonNull String getDeclineText()
                {
                    return mActivity.getString(R.string.location_permission_decline_text);
                }
            };


    /**
     * This constructor creates a new instance of TSDDetectorFragment setting the name and the
     * layout resource ids.
     */
    public TSDDetectorFragment()
    {
        super(R.string.detector_display_name, R.drawable.ic_detector);
    }


    /**
     * This method gets called when the fragment is created. It initializes the periodic task to
     * trigger the GC for cleaning up camera data rubbish.
     * @param savedInstanceState Contains key-value pairs that might have been saved at the last
     *                            destroy of the fragment. Currently this does not get used.
     */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }

        // Initialize a periodic task triggering the gc every minute.
        if (mGcTrigger != null)
        {
            mGcTrigger.disable();
            mGcTrigger = null;
        }
        mGcTrigger = new TSDPeriodicTask(60_000, 60_000,
                () -> Runtime.getRuntime().gc(), getLifecycle());
    }


    /**
     * This method gets called every time the view of the {@link TSDFragment} is newly created. It
     * inflates the fragment_detector layout as user interface for the {@link TSDFragment}.
     * It creates and initializes the {@link TSDSignHistory}, the {@link RecyclerView} and the main
     * sign view to isRelevant this history. Furthermore the {@link TSDSpeedTracker} and the
     * {@link TSDShutterCamera} get initialized. There is also a {@link TSDPeriodicTask} to
     * refresh the time isRelevant for all visible {@link TSDSignCombination TSDSignCombinations} on
     * screen.
     * @see TSDDetectorFragment
     * @param inflater Inflater to use for inflating the user interface layout of this fragment.
     * @param container The place the ui should be inflated to, but not attached.
     * @param savedInstanceState Contains key-value pairs that might have been saved at the last
     *                            destroy of the fragment. Currently this does not get used.
     * @return The inflated view containing the user interface of this {@link TSDFragment}.
     */
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState)
    {
        View self = inflater.inflate(R.layout.fragment_detector, container, false);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }

        if (mActivity != null)
        {
            // Create a lifecycle outliving instance of the drawable database and the sign history.
            mDrawableDatabase = ViewModelProviders.of(mActivity).get(TSDDrawableDatabase.class);
            mSignHistory = ViewModelProviders.of(mActivity).get(TSDSignHistory.class);

            // Set max size for history.
            final int historyMaxSize = TSDHelper.readIntegerFromSharedPreferences(
                    R.string.data_history_max_size_key, 20, mActivity);
            TSDRingMemory<TSDSignCombination> signMemory =
                    mSignHistory.getSignCombinations().getValue();
            if (signMemory != null)
            {
                signMemory.setMaxSize(historyMaxSize);
            }

            // Create layout manager.
            LinearLayoutManager historyLayoutManager = new LinearLayoutManager(getActivity());
            historyLayoutManager.setOrientation(LinearLayoutManager.HORIZONTAL);

            // Get the history view.
            RecyclerView historyView = self.findViewById(R.id.historyView);

            // Create history adapter and attach it.
            TSDSignHistoryAdapter historyAdapter = new TSDSignHistoryAdapter(this,
                    mSignHistory, historyLayoutManager, historyView);
            historyView.setAdapter(historyAdapter);

            // Initialize history recycler view
            historyView.setLayoutManager(historyLayoutManager);

            // Initialise speed isRelevant and main sign view holder.
            View mainSignView = self.findViewById(R.id.mainCardView);
            mSignViewHolder = new TSDSignCombinationViewHolder(mainSignView);

            // Query whether the user already has disabled speed tracking and set speed accordingly.
            boolean gpsChecked = TSDHelper.readBooleanFromSharedPreferences(R.string.data_gps_key,
                    true, mActivity);
            mSpeed = gpsChecked ? -1 : null;

            // Display some empty sign combination to the main sign.
            mSignCombination = new TSDSignCombination(0, -1, new TSDSign[0], new Rect[0],
                    null, System.currentTimeMillis(), mSpeed, mDrawableDatabase, mActivity);
            mSignViewHolder.displayAll(mSignCombination, MAIN_SIGN_TRANSPARENCY,
                    0f);

            // Get the threshold for speed warning from preferences.
            int threshold = TSDHelper.readIntegerFromSharedPreferences(
                    R.string.data_warning_threshold, 4, mActivity);

            // Create a closure for function reuse to handle the speed warning.
            Consumer<Integer> handleSpeedWarning = (speed) ->
            {
                // Display speed warning if appropriate.
                if (mSignHistory.isSpeedOk(speed, threshold))
                {
                    mSignViewHolder.setWarningTransparency(0f);
                }
                else
                {
                    mSignViewHolder.setWarningTransparency(1f);
                }
            };

            // Create a new speed tracker listening on this fragments lifecycle.
            if (mSpeedTracker != null)
            {
                mSpeedTracker.disable();
                mSpeedTracker = null;
            }
            mSpeedTracker = new TSDSpeedTracker(getLifecycle(), mActivity, speed ->
            {
                // Display the speed to the main sign view.
                mSignViewHolder.displaySpeed(speed);

                // Display speed warning if appropriate.
                handleSpeedWarning.accept(speed);

                // Save the speed for later redisplay.
                mSpeed = speed;
            });

            // Listen for changes in the sign collection to isRelevant the latest sign
            mSignHistory.getSignCombinations().observe(this, signCombis ->
            {
                if (signCombis != null)
                {
                    if (signCombis.size() > 0)
                    {
                        // Get the latest sign, isRelevant the changes to the view and save the sign.
                        TSDSignCombination latestSign = signCombis.get(signCombis.size() - 1);
                        mSignViewHolder.displayAll(latestSign, MAIN_SIGN_TRANSPARENCY,
                                0f);
                        mSignViewHolder.displaySpeed(mSpeed);
                        mSignCombination = latestSign;
                    }
                    else
                    {
                        // Save and isRelevant some empty sign combination to the main sign.
                        mSignCombination = new TSDSignCombination(0, -1, new TSDSign[0],
                                new Rect[0], null, System.currentTimeMillis(), mSpeed,
                                mDrawableDatabase, mActivity);
                        mSignViewHolder.displayAll(mSignCombination, MAIN_SIGN_TRANSPARENCY,
                                0f);
                    }

                    // Display speed warning if appropriate.
                    handleSpeedWarning.accept(mSpeed);
                }
            });

            // Create a periodic task to refresh the time on the sign views.
            if (mTimeTrigger != null)
            {
                mTimeTrigger.disable();
                mTimeTrigger = null;
            }
            mTimeTrigger = new TSDPeriodicTask(1_000, 1_000, () ->
                mActivity.runOnUiThread(() ->
                {
                    mSignViewHolder.displayTime(mSignCombination.getTimestamp());

                    final int from = historyLayoutManager.findFirstVisibleItemPosition();
                    final int to = historyLayoutManager.findLastVisibleItemPosition();
                    historyAdapter.updateTimeDisplay(from, to);
                }),
                getLifecycle());

            // Get the cut-off warning layout and set it to invisible.
            View cutOffWarningView = self.findViewById(R.id.cutOffWarningLayout);
            cutOffWarningView.setVisibility(View.INVISIBLE);

            // Create a cut-off speed tracker as FrameEventListener to keep track of the current
            // calculated cut-off speed. As a camera angle set an starter value of 60° (in radians)
            // and update it later with the real value as soon there is access to the camera.
            // When the current speed is greater than this cut-off speed, then show a warning.
            TSDCutOffSpeedTracker cutOffSpeedTracker = new TSDCutOffSpeedTracker((cutOffSpeed) ->
            {
                // Set warning to visible if speed and cut-off speed is available and the device
                // is too fast. Otherwise hide the warning.
                if (mSpeed != null && cutOffSpeed != null && mSpeed > cutOffSpeed)
                {
                    if (cutOffWarningView.getVisibility() == View.INVISIBLE)
                    {
                        cutOffWarningView.setVisibility(View.VISIBLE);
                    }
                }
                else
                {
                    if (cutOffWarningView.getVisibility() == View.VISIBLE)
                    {
                        cutOffWarningView.setVisibility(View.INVISIBLE);
                    }
                }
            }, (float)Math.PI * (60f / 180f));

            // Create camera manager.
            mCameraManager = new TSDCameraManager(mActivity);

            // Initialize the camera.
            if (mShutterCamera != null)
            {
                mShutterCamera.disable();
                mShutterCamera = null;
            }
            TSDShutterCameraImageListener scListener = new TSDShutterCameraImageListener(
                    mSignHistory, mDrawableDatabase, null, cutOffSpeedTracker, mActivity);
            mShutterCamera = new TSDShutterCamera(mCameraManager,
                    getLifecycle(), new Size(544, 306), null, scListener);

            // Try to get the real sensor orientation value and set it to the image listener for it
            // to be able to correct rotation issues in captured images.
            Integer sensorRotation = mShutterCamera.getSensorRotation();
            scListener.setSensorRotation(sensorRotation);

            // Try to get the real view angle of the chosen camera and set it to the cut-off speed
            // tracker.
            Double viewAngle = mShutterCamera.getCameraCharacter().getMaximumViewAngle();
            if (viewAngle != null)
            {
                cutOffSpeedTracker.setCameraViewAngle((float)((double)viewAngle));
            }
        }

        return self;
    }


    /**
     * This method gets called every time the {@link TSDFragment} is started. It asks for the
     * required camera permissions and enables the {@link TSDDetectorFragment#mShutterCamera}
     * if those were granted. If the camera was granted, then also the location permission gets
     * asked for and if this one is also granted, the {@link TSDDetectorFragment#mSpeedTracker}
     * gets enabled, too. If one of the permissions does not get granted, the connected objects
     * will not get started. The user will get an information about the consequences of his
     * decision and the possibility to get another ask for the permissions.
     */
    @Override
    public void onStart()
    {
        super.onStart();

        if (mSpeedTracker != null)
        {
            mTimeTrigger.enable();

            // Enable remaining lifecycle objects after permissions were granted.
            TSDPermissionManager.getPermissions(mActivity, mCameraResultListener, 1000);
        }
    }
}
