/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * TSDCameraManager is a Wrapper around the {@link CameraManager}. It adds some implementations to
 * make it compatible with the other wrappers {@link TSDCameraCharacter} and {@link TSDBaseCamera}.
 * 
 * @image latex app_class_camera.png width=16cm
 * @image html app_class_camera.svg
 */
public class TSDCameraManager
{
    private final Activity mActivity;
    private final CameraManager mManager;


    /**
     * Creates a new instance of CameraDetector preparing for loading of camera information.
     * @param activity The Activity running in
     */
    TSDCameraManager(Activity activity)
    {
        mActivity = activity;
        mManager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
    }


    /**
     * Queries the characters of camera devices which are available to this device.
     * @return Array of all camera characters. If there are no cameras accessible or an error occurs
     * this can be empty.
     */
    public TSDCameraCharacter[] getCameraCharacters()
    {
        List<TSDCameraCharacter> characters = new ArrayList<>();

        try
        {
            String[] ids = mManager.getCameraIdList();

            for (String id : ids)
            {
                try {
                    CameraCharacteristics cc = mManager.getCameraCharacteristics(id);
                    characters.add(new TSDCameraCharacter(id, cc, this));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return characters.toArray(new TSDCameraCharacter[characters.size()]);
    }



    /**
     * This method tries to open the given camera instance. Please mind, that this does not
     * include the permission request. This must be done before, otherwise the opening will fail.
     * When the opening was successful, then {@link TSDBaseCamera#mCameraStateCallback} will be
     * signaled which again will signal the {@link TSDBaseCamera.OnCameraEventListener} given to
     * this camera instance.
     * @param camera Camera instance to open
     */
    public void openCamera(TSDBaseCamera camera)
    {
        try
        {
            // First check if CAMERA permission is set to avoid unnecessary error logs.
            int res = mActivity.checkCallingOrSelfPermission(android.Manifest.permission.CAMERA);
            if (res == PackageManager.PERMISSION_GRANTED)
            {
                mManager.openCamera(camera.getCharacter().getId(),
                        camera.getCameraStateCallback(), camera.getBackgroundHandler());
            }
            else
            {
                Log.i("TSDCameraManager", "CAMERA PERMISSION not granted");
            }
        }
        catch (CameraAccessException e)
        {
            e.printStackTrace();
        }
    }
}
