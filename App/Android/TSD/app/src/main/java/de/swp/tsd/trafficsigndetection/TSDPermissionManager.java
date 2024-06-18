/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.annotation.NonNull;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.ArrayMap;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

/**
 * This class is only for static use. Its functions can be used for requesting app permissions
 * on runtime and reacting on the users result. To use this class, a request handler method needs
 * to be implemented in the request receiving activity like for example here:
 * {@link MainActivity#onRequestPermissionsResult(int, String[], int[])}.
 * Then permissions can be requested by calling
 * {@link TSDPermissionManager#getPermissions(Activity, OnRequestPermissionResultListener, long)}.
 * The delivered listener will get notified, when the users decision is known. This could be
 * even before asking the user for any permission if he has permitted them already or when he
 * accepted all or declined at least one of them. The signaled result will be accordingly. When the
 * user declined at least one of the permissions, a {@link Snackbar} will be displayed to him giving
 * him information about the effects of the decline and a button to ask for the permissions again.
 */
public final class TSDPermissionManager
{
    private final static int SNACKBAR_DURATION = 10_000;

    private static Map<Integer, OnRequestPermissionResultListener> mResultListeners
            = new ArrayMap<>();

    private static Timer mTimer = new Timer();


    /**
     * This function is called from the main activity handling the request permission results.
     * It signals the appropriate listener defined before when having called
     * TSDPermissionManager.getPermissions() which is identified by the requestCode.
     * When at least one requested permission was declined, an info text and the possibility to
     * reask for the permission will be displayed to the user as {@link Snackbar}.
     * @param requestCode Request code of the result used to find correct listener.
     * @param activity Activity the request was processed in.
     * @param grantResults Array of all results of the corresponding permissions
     * @param view Coordinator view a snackbar with the in the listener defined decline text will
     *             be shown, when some of the requested permissions were declined.
     */
    public static void onRequestPermissionResult(int requestCode, Activity activity,
                                                 int[] grantResults, CoordinatorLayout view)
    {
        // Only handle request callback when one is available.
        if (mResultListeners.containsKey(requestCode))
        {
            // check if all permissions are granted.
            boolean allGranted = true;
            for (int res : grantResults)
            {
                allGranted = allGranted && (res == PackageManager.PERMISSION_GRANTED);
            }

            // Get and remove the listener from the map.
            OnRequestPermissionResultListener listener = mResultListeners.remove(requestCode);

            // Only signal granted if all permission were granted and the result array actually
            // is longer than 0 - this would otherwise cause false signal.
            if (allGranted && grantResults.length > 0)
            {
                // Signal the listener, that all permissions were granted.
                listener.onRequestedPermissionsGranted();
            }
            else
            {
                // Show a snackbar informing the user about his choice and offering him to ask him
                // again for the permissions. When the snackbar gets dismissed, then signal the
                // listener the users decision to reject some permissions.
                Snackbar.make(view, listener.getDeclineText(), SNACKBAR_DURATION)
                        .setAction(R.string.permission_decline_ask_again,
                                v -> getPermissions(activity, listener, 250) )
                        .show();

                // Signal the listener, that some permissions were not granted.
                listener.onRequestedPermissionsDeclined();
            }
        }
    }


    /**
     * This function can be called to ask the user for runtime permissions like CAMERA.
     * @param activity The activity this is used in. Make sure to call
     *                 TSDPermissionManager.onRequestPermissionResult inside this activity from the
     *                 onRequestPermissionResult callback. In fragments you can get this with
     *                 getActivity(), but make sure it is not null.
     * @param resultListener Observer which gets signaled, when the users choice upon the
     *                       requested permissions is known. It also contains the asked permissions
     *                       and the message to show to the user when not all permissions were
     *                       granted.
     * @param waitMillis How many milliseconds to wait until requesting the permission. This can
     *                   be used as safety guard because requesting permissions in onResume causes
     *                   an infinite loop or just as convenience to not display the dialog
     *                   immediately when a activity or fragment starts.
     */
    public static void getPermissions(Activity activity,
                                      OnRequestPermissionResultListener resultListener,
                                      long waitMillis)
    {
        // Run request as Timer task with delay, because requesting permissions in onResume causes
        // an infinite loop.
        mTimer.schedule(new TimerTask()
        {
            @Override
            public void run()
            {
                // Get current state of permissions and collect all which are not granted.
                List<String> requestPermissions = new ArrayList<>();
                for (String permission : resultListener.getRequestedPermissions())
                {
                    int res = ContextCompat.checkSelfPermission(activity, permission);
                    if (res != PackageManager.PERMISSION_GRANTED)
                    {
                        requestPermissions.add(permission);
                    }
                }

                // Request all remaining permissions.
                if (requestPermissions.size() > 0)
                {
                    // Create new request code.
                    int requestCode = mResultListeners.size();

                    // Save the callback for later usage.
                    mResultListeners.put(requestCode, resultListener);

                    // Request the collected permissions.
                    ActivityCompat.requestPermissions(activity,
                            requestPermissions.toArray(new String[requestPermissions.size()]),
                            requestCode);
                }
                else
                {
                    // Signal the listener, because all permissions are granted.
                    resultListener.onRequestedPermissionsGranted();
                }
            }

        }, waitMillis);
    }


    /**
     * This interface defines an observer to be implemented by all users wanting to request some
     * permission signaling them, when the users decision is known.
     * @see TSDPermissionManager
     */
    public interface OnRequestPermissionResultListener
    {
        /**
         * This method should return the permissions this request asks for.
         * @return String array of requested permissions
         */
        String[] getRequestedPermissions();


        /**
         * This method gets called, when the users decided to grant all requested permissions.
         */
        void onRequestedPermissionsGranted();


        /**
         * This method gets called, when the user decided to decline at least one of the requested
         * permissions.
         */
        void onRequestedPermissionsDeclined();


        /**
         * This method is called when some of the requested permissions were not granted. It should
         * return a message that can be showed to the user.
         * @return Decline message text
         */
        @NonNull String getDeclineText();
    }
}
