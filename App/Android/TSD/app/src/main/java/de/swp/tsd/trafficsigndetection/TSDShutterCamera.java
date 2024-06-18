/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.arch.lifecycle.Lifecycle;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.media.Image;
import android.media.ImageReader;
import android.support.annotation.NonNull;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import java.util.ArrayList;
import java.util.List;

/**
 * This class defines an behaviour for the {@link TSDBaseCamera} featuring the functionality to
 * display a camera preview and take images which get send to the given
 * {@link TSDShutterCameraImageListener}.
 * Exposure and white balance are set to auto while focus is set to infinite distance. Pictures
 * are taken in {@link TSDCameraOutputFormat#YUV_420_888} format which the image listener is able to
 * decode.
 * The internal {@link ImageReader} is configured to hold a maximum of 2 images. The camera will
 * fill those slots and only add a new picture, when at least one slot was processed by the
 * {@link TSDShutterCameraImageListener}.
 * @see TSDShutterCameraImageListener
 */
public class TSDShutterCamera
{
    private static TSDCameraOutputFormat IMG_FORMAT = TSDCameraOutputFormat.YUV_420_888;

    private final TSDCameraManager mManager;
    private TSDBaseCamera mBaseCamera;

    private final TextureView mPreview;
    private final ImageReader.OnImageAvailableListener mImageListener;
    private Size mImageSize;
    private Surface mPreviewSurface;
    private ImageReader mImageReader;
    private CameraCaptureSession mSession;

    private final TextureView.SurfaceTextureListener mTextureListener
            = new TextureView.SurfaceTextureListener()
    {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height)
        {
            // Create new surface as camera target.
            mPreviewSurface = new Surface(surface);

            // Signal the camera to continue startup.
            mBaseCamera.onTargetsReady();
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) { }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface)
        {
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surface) { }
    };

    private final TSDBaseCamera.OnCameraEventListener mEventListener
            = new TSDBaseCamera.OnCameraEventListener()
    {
        @Override
        public void onResume()
        {
            // Add image reader if a listener is given.
            if (mImageListener != null)
            {
                // Create image reader for getting the results back.
                mImageReader = ImageReader.newInstance(mImageSize.getWidth(),
                        mImageSize.getHeight(), ImageFormat.YUV_420_888, 2);

                // Set on image listener running on background thread.
                mImageReader.setOnImageAvailableListener(mImageListener,
                        mBaseCamera.getBackgroundHandler());
            }

            // If there is a preview, then wait for the texture to be available. Otherwise return to
            // camera.
            if (mPreview != null)
            {
                // Either directly signal the texture listener when TextureView is ready or let the
                // TextureView do it.
                if (mPreview.isAvailable())
                {
                    mTextureListener.onSurfaceTextureAvailable(mPreview.getSurfaceTexture(),
                            mPreview.getWidth(), mPreview.getHeight());
                }
                else
                {
                    mPreview.setSurfaceTextureListener(mTextureListener);
                }
            }
            else
            {
                // Signal the camera to continue startup.
                mBaseCamera.onTargetsReady();
            }
        }

        @Override
        public void onOpened(CameraDevice camera)
        {
            // Setup the preview session.
            createPreviewSession(camera);
        }

        @Override
        public void onClose()
        {
            // Close and cleanup the session.
            if (mSession != null)
            {
                mSession.close();
                mSession = null;
            }
        }

        @Override
        public void onPaused()
        {
            // Release and cleanup the preview surface.
            if (mPreviewSurface != null)
            {
                mPreviewSurface.release();
                mPreviewSurface = null;
            }

            // Release and cleanup the image reader.
            if (mImageReader != null)
            {
                // Throw already taken images away.
                Image img = mImageReader.acquireLatestImage();
                while (img != null)
                {
                    img.close();
                    img = mImageReader.acquireLatestImage();
                }

                mImageReader.close();
                mImageReader = null;
            }
        }
    };


    /**
     * This constructor creates a new instance of TSDShutterCamera. Taking the manager it uses the
     * best back facing camera available. If both preview and imageListener are null, no capture
     * session will be started.
     * @param manager Camera manager the camera information are taken from.
     * @param lifecycle Lifecycle of the parent used to manage the camera lifecycle.
     * @param imgTargetSize Target size for the output images. The resulting images are at the most
     *                      this big.
     * @param preview TextureView the preview of the camera is displayed to. This might get removed
     *                at a later stage in development. If this is null, no preview will be used.
     * @param imageListener Image observer getting signaled when an image from the camera is ready.
     *                      If this is null, no image reader will be used.
     */
    TSDShutterCamera(@NonNull TSDCameraManager manager,
                     @NonNull Lifecycle lifecycle,
                     @NonNull Size imgTargetSize, TextureView preview,
                     ImageReader.OnImageAvailableListener imageListener)
    {
        mManager = manager;
        mPreview = preview;
        mImageListener = imageListener;

        // Try to find the best fitting camera.
        TSDCameraCharacter cameraChara = getBestCamera(imgTargetSize);

        // Initialize camera settings.
        if (cameraChara != null)
        {
            // Get the best matching output size the camera has to offer.
            mImageSize = cameraChara.getMatchingOutputSize(IMG_FORMAT, imgTargetSize);

            // Only create camera, if there was a matching size.
            if (mImageSize != null)
            {
                // Create the camera.
                mBaseCamera = new TSDBaseCamera(cameraChara, mEventListener, lifecycle);
            }
        }
    }


    public TSDCameraCharacter getCameraCharacter()
    {
        if (mBaseCamera != null)
        {
            return mBaseCamera.getCharacter();
        }

        return null;
    }


    /**
     * Queries whether the camera is enabled or not. Enabled does not necessarily mean that it is
     * running. Enabled just means that it will automatically start, when the parent lifecycle hits
     * the onResume event and automatically stop on onPause.
     * @return Enable state of the camera.
     */
    public boolean isEnabled()
    {
        // Null check in case the constructor did not initialize the camera.
        // If initialized, then return whether the camera is enabled.
        return mBaseCamera != null && mBaseCamera.isEnabled();
    }


    /**
     * This method enables the camera if not already enabled. This does not necessarily open the
     * camera, but it will at the latest when the parent lifecycle hits the ON_RESUME event.
     */
    public void enable()
    {
        // Null check in case the constructor did not initialize the camera.
        // If initialized, then enable the camera.
        if (mBaseCamera != null)
        {
            mBaseCamera.enable();
        }
    }


    /**
     * This method closes and disables the camera if not already the case.
     */
    public void disable()
    {
        // Null check in case the constructor did not initialize the camera.
        // If initialized, then disable the camera.
        if (mBaseCamera != null)
        {
            mBaseCamera.disable();
        }
    }


    /**
     * This method checks, whether the camera was created at construction or not.
     * @return Boolean indication whether the camera was created or not.
     */
    public boolean isInitiated()
    {
        // If no image size is set, then the camera was not created, too.
        return mImageSize != null;
    }


    /**
     * This method returns the value of {@link TSDCameraCharacter#getSensorRotation()} of the
     * internally selected camera for image capture.
     * @return Sensor rotation of selected camera or null, if camera was not initialized.
     */
    public Integer getSensorRotation()
    {
        if (mBaseCamera != null)
        {
            return mBaseCamera.getCharacter().getSensorRotation();
        }

        return null;
    }


    /**
     * This method creates a repeating preview capture session on the given camera device using the
     * preview texture surface and the image reader. This will result in a preview been shown to
     * screen and images being delivered to the image listener.
     * @param camera Camera device to start the session on.
     */
    private void createPreviewSession(CameraDevice camera)
    {
        try
        {
            // Create the list of surfaces to use in this capture session. Only add available
            // surfaces.
            List<Surface> surfaces = new ArrayList<>();

            if (mImageListener != null)
            {
                surfaces.add(mImageReader.getSurface());
            }

            if (mPreview != null)
            {
                surfaces.add(mPreviewSurface);
            }

            // If there is at least one target surface, then create a new session with the given
            // surfaces and process the defined requests.
            if (surfaces.size() > 0)
            {
                camera.createCaptureSession(surfaces, new CameraCaptureSession.StateCallback()
                {
                    @Override
                    public void onConfigured(@NonNull CameraCaptureSession session)
                    {
                        mSession = session;

                        try
                        {
                            // Create preview request.
                            CaptureRequest.Builder request = camera
                                    .createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);

                            // Add the available surfaces as targets.
                            for (Surface surface : surfaces)
                            {
                                request.addTarget(surface);
                            }

                            // Set auto for white balance, exposure and auto focus. Seems to be
                            // necessary to configure exposure and auto focus.
                            request.set(CaptureRequest.CONTROL_MODE,
                                    CameraMetadata.CONTROL_MODE_AUTO);

                            // Set auto white balance.
                            request.set(CaptureRequest.CONTROL_AWB_MODE,
                                    CameraMetadata.CONTROL_AWB_MODE_AUTO);

                            // Set auto exposure.
                            request.set(CaptureRequest.CONTROL_AE_MODE,
                                    CameraMetadata.CONTROL_AE_MODE_ON);

                            // Disable auto focus.
                            request.set(CaptureRequest.CONTROL_AF_MODE,
                                    CameraMetadata.CONTROL_AF_MODE_OFF);

                            // Set focus to infinite.
                            request.set(CaptureRequest.LENS_FOCUS_DISTANCE, 0.0f);

                            // Start the preview as repeating request.
                            session.setRepeatingRequest(request.build(), null,
                                    mBaseCamera.getBackgroundHandler());
                        }
                        catch (Exception e)
                        {
                            e.printStackTrace();
                        }
                    }

                    @Override
                    public void onConfigureFailed(@NonNull CameraCaptureSession session) { }

                }, mBaseCamera.getBackgroundHandler());
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }


    /**
     * This method queries the best available back facing camera able to handle the target size.
     * The cameras are compared by their {@link TSDCameraCharacter#getScore(TSDLensFacing) scores}
     * and the best camera is returned which supports the target size checked by calling
     * {@link TSDCameraCharacter#getMatchingOutputSize(TSDCameraOutputFormat, Size)}.
     * @param targetSize Size the camera should at least match and its aspect ration the camera has
     *                   to match.
     * @return The camera character of the best available back facing camera supporting the target
     * image size.
     */
    private TSDCameraCharacter getBestCamera(Size targetSize)
    {
        TSDCameraCharacter[] chars = mManager.getCameraCharacters();

        if (chars.length > 0)
        {
            // Get scores of cameras.
            int[] scores = new int[chars.length];
            for (int i = 0; i < chars.length; i++)
            {
                scores[i] = chars[i].getScore(TSDLensFacing.BACK);
            }

            // Get camera with max score.
            TSDCameraCharacter maxChar = chars[0];
            int maxScore = scores[0];
            for (int i = 0; i < chars.length; i++)
            {
                // Check for bigger score and
                if (scores[i] > maxScore
                        && chars[i].getMatchingOutputSize(IMG_FORMAT, targetSize) != null)
                {
                    maxChar = chars[i];
                    maxScore = scores[i];
                }
            }

            return maxChar;
        }

        return null;
    }
}