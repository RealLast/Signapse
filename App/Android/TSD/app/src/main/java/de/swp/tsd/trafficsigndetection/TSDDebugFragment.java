/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.Manifest;
import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.Snackbar;
import android.util.Size;
import android.view.LayoutInflater;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;


/**
 * This {@link TSDFragment} is used for debugging purposes and will get removed in the final
 * product. It currently prints some information to every camera in the device to screen.
 * Furthermore does it open a {@link TSDShutterCamera} with currently only activated live post view
 * which displays the results of the TrafficSignAPI to screen, which processes live frames taken by
 * the camera. This requires the TrafficSignAPI to already having been initialized.
 * The fragment triggers the required permission requests and includes an
 * {@link TSDPeriodicTask} which periodically triggers the GC to clean up rubbish produced by the
 * camera beginning with Android Version 8.0 .
 */
public class TSDDebugFragment extends TSDFragment
{
    private Activity mActivity;
    private TextView mInfoText;
    private TSDCameraManager mCameraManager;
    private TSDShutterCamera mShutterCamera;
    private TSDPeriodicTask mGcTrigger;

    private TSDPermissionManager.OnRequestPermissionResultListener mCameraResultListener =
            new TSDPermissionManager.OnRequestPermissionResultListener()
            {
                @Override
                public String[] getRequestedPermissions()
                {
                    return new String[]{ Manifest.permission.CAMERA };
                }

                @Override
                public void onRequestedPermissionsGranted()
                {
                    // Only enable camera if permission was granted.
                    mShutterCamera.enable();
                    mGcTrigger.enable();

                    // Show a snackbar informing the user about the start of the camera.
                    Snackbar.make(((MainActivity)mActivity).getContentLayout(),
                            R.string.detection_started, Snackbar.LENGTH_LONG)
                            .show();
                }

                @Override
                public void onRequestedPermissionsDeclined() { }

                @Override
                public @NonNull String getDeclineText()
                {
                    return mActivity.getString(R.string.camera_permission_decline_text);
                }
            };


    /**
     * This constructor creates a new instance of {@link TSDDebugFragment} setting the correct
     * values for {@link TSDFragment#displayNameId display name} and
     * {@link TSDFragment#displayIconId display icon}.
     */
    public TSDDebugFragment()
    {
        super(R.string.debug_display_name, R.drawable.ic_debug);
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
     * inflates the fragment_debug layout as user interface for the {@link TSDFragment}.
     * It initializes the shutter camera and sets the required views. The Shutter camera currently
     * only displays to the postView of this ui layout. Preview and sign history are disabled.
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
        View self = inflater.inflate(R.layout.fragment_debug, container, false);

        ImageView postView = self.findViewById(R.id.postView);

        mInfoText = self.findViewById(R.id.infoText);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }

        if (mActivity != null)
        {
            mCameraManager = new TSDCameraManager(mActivity);

            // Print camera stats
            mInfoText.setText(getCameraChars());

            // Initialize the camera.
            if (mShutterCamera != null)
            {
                mShutterCamera.disable();
                mShutterCamera = null;
            }
            TSDShutterCameraImageListener scListener = new TSDShutterCameraImageListener(
                    null, null, postView, null, mActivity);
            mShutterCamera = new TSDShutterCamera(mCameraManager, getLifecycle(),
                    new Size(544, 306), null, scListener);

            // Try to get the real sensor orientation value and set it to the image listener for it
            // to be able to correct rotation issues in captured images.
            Integer sensorRotation = mShutterCamera.getSensorRotation();
            scListener.setSensorRotation(sensorRotation);
        }

        return self;
    }


    /**
     * This method gets called every time the {@link TSDFragment} is started. It asks for the
     * required permissions and if those are granted enables the already in
     * {@link TSDDebugFragment#onCreateView(LayoutInflater, ViewGroup, Bundle)} created
     * {@link TSDShutterCamera}.
     */
    @Override
    public void onStart()
    {
        super.onStart();

        // Enable camera and gc trigger if camera was created and permission granted.
        if (mShutterCamera != null)
        {
            TSDPermissionManager.getPermissions(mActivity, mCameraResultListener, 1000);
        }
    }


    /**
     * This helper method creates a string containing some information about all cameras in this
     * device. Those information are for each camera: Camera id, Camera score calculated by
     * {@link TSDCameraCharacter#getScore(TSDLensFacing)}, the facing direction, the supported
     * autofocus modes, the supported optical image stabilization modes, the supported apertures,
     * the supported camera output image formats and the supported output image sizes for the
     * JPEG image format. The information are grouped per camera.
     * @return String containing some information about each camera.
     */
    private String getCameraChars()
    {
        TSDCameraCharacter[] charas = mCameraManager.getCameraCharacters();

        StringBuilder text = new StringBuilder();
        for (TSDCameraCharacter chara : charas)
        {
            text.append("Id: \t");
            text.append(chara.getId());
            text.append("\n");

            text.append("Score: \t");
            text.append(chara.getScore(TSDLensFacing.BACK));
            text.append("\n");

            text.append("Direction: \t");
            text.append(chara.getLensFacing().name());
            text.append("\n");

            text.append("AF Modes: \t");
            for (TSDAutoFocusMode mode : chara.getAvailableAutoFocusModes())
            {
                text.append(mode.name());
                text.append(" ");
            }
            text.append("\n");

            text.append("Ois Modes: \t");
            for (TSDOpticalStabilizationMode mode : chara.getAvailableOisModes())
            {
                text.append(mode.name());
                text.append(" ");
            }
            text.append("\n");

            text.append("Apertures: \t");
            for (float aperture : chara.getAvailableApertures())
            {
                text.append(aperture);
                text.append(" ");
            }
            text.append("\n");

            text.append("Formats: \t");
            for (TSDCameraOutputFormat format : chara.getAvailableOutputFormats())
            {
                text.append(format);
                text.append(" ");
            }
            text.append("\n");

            text.append("JPEG Sizes: \t");
            for (Size size : chara.getAvailableOutputSizes(TSDCameraOutputFormat.JPEG))
            {
                text.append(size.toString());
                text.append(" ");
            }
            text.append("\n");

            text.append("\n");
        }

        return text.toString();
    }
}
