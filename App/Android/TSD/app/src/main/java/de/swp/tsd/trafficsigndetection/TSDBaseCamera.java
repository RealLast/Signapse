/*
 * Code work or ensemble by Florian Köhler based on https://github.com/googlesamples/
 * android-Camera2Basic/blob/master/Application/src/main/java/com/example/android/camera2basic/
 * Camera2BasicFragment.java
 *
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.arch.lifecycle.Lifecycle;
import android.hardware.camera2.CameraDevice;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
import android.support.annotation.NonNull;
import android.util.Log;

import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

/**
 * This class is the base for every specialized camera used in this application. Its task is to
 * manage the opening and closing of the given {@link CameraDevice} taking the lifecycle of the
 * parent into consideration closing and reopening the camera when the parent gets paused and
 * resumed. Using the {@link OnCameraEventListener} the behaviour of the camera can be defined.
 * The lifecycle handling is done by the {@link TSDLifecycleObserver} base class. The camera is run
 * inside a background thread which gets started and stopped together with the camera itself.
 * Startup and Stopping are locked to each other by a semaphore avoiding that the camera could be
 * stopped before it was even completely started. Please mind, that this class does not handle
 * the runtime camera permission. This needs to be done outside the class. Otherwise the camera
 * initialization will fail.
 * To define a new camera behaviour based on this class you need the create an
 * {@link OnCameraEventListener}. This listener gets signaled on every important camera event
 * and should be used for defining the behavior and the used camera target surfaces.
 * @see OnCameraEventListener
 */
public class TSDBaseCamera extends TSDLifecycleObserver
{
    /**
     * The character of the camera used. This is the same that was given to the
     * constructor.
     */
    private final TSDCameraCharacter mCharacter;

    private CameraDevice mCamera;

    private HandlerThread mBackgroundThread;

    /**
     * The looper handler of the background thread for doing the camera work. You can
     * use this handler to let work get done on this thread. Especially use it for camera
     * related work inside the {@link OnCameraEventListener}.
     */
    private Handler mBackgroundHandler;

    private OnCameraEventListener mEventListener;

    private Semaphore mCameraOpenCloseLock = new Semaphore(1);

    /**
     * This callback is used by the camera api to signal back the state of the camera. The
     * implementation releases the in the startup locked semaphore and signals the
     * {@link OnCameraEventListener} with the correct signal according to the state of the camera.
     */
    private final CameraDevice.StateCallback mCameraStateCallback = new CameraDevice.StateCallback()
    {
        @Override
        public void onOpened(@NonNull CameraDevice camera)
        {
            // Save the camera device and release the lock acquired on opening of camera.
            mCamera = camera;
            mCameraOpenCloseLock.release();

            // Signal the listener the opened camera.
            mEventListener.onOpened(camera);
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera)
        {
            // Close the disconnected camera, cleanup and release the lock acquired on opening of
            // camera.
            camera.close();
            mCamera = null;
            mCameraOpenCloseLock.release();
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error)
        {
            // Close the faulty camera device, cleanup and release the lock acquired on opening of
            // camera.
            camera.close();
            mCamera = null;
            mCameraOpenCloseLock.release();
            Log.e("|CAMERA| -> onError", "Code: " + error);
        }
    };


    /**
     * This constructor creates a new instance of TSDBaseCamera setting the listener on the parents
     * lifecycle. This does not enable the camera. To enable the camera please see
     * {@link TSDLifecycleObserver#enable()}.
     * @param character Character defining the camera to use. This can be obtained by using the
     *                  {@link TSDCameraManager}.
     * @param eventListener Observer for all important camera events. This should be used to define
     *                      the behaviour of the camera itself.
     * @param lifecycle Lifecycle of the parent starting and stopping the camera and background
     *                  thread.
     */
    TSDBaseCamera(@NonNull TSDCameraCharacter character,
                  @NonNull OnCameraEventListener eventListener, @NonNull Lifecycle lifecycle)
    {
        // Initialize lifecycle observation.
        super(lifecycle);

        mCharacter = character;
        mEventListener = eventListener;
    }


    /**
     * Getter for the character of the camera used. This is the same that was given to the
     * constructor.
     * @return Character of the camera being used
     */
    public TSDCameraCharacter getCharacter()
    {
        return mCharacter;
    }


    /**
     * Getter for the looper handler of the background thread for doing the camera work. You can
     * use this handler to let work get done on this thread. Especially use it for camera
     * related work inside the {@link OnCameraEventListener}.
     * @return Handler of the background thread looper
     */
    public Handler getBackgroundHandler()
    {
        return mBackgroundHandler;
    }


    /**
     * Getter for the state callback object of the camera. This callback is used by the camera api
     * to signal back the state of the camera. The implementation releases the in the startup locked
     * semaphore and signals the {@link OnCameraEventListener} with the correct signal according to
     * the state of the camera.
     * @return State callback object of the camera
     */
    public CameraDevice.StateCallback getCameraStateCallback()
    {
        return mCameraStateCallback;
    }


    /**
     * This method should only be called after having received the onResume() callback of the
     * OnCameraEventListener and having initialized all necessary camera targets.
     * @see OnCameraEventListener#onResume()
     */
    public void onTargetsReady()
    {
        // Open the camera.
        openCamera();
    }


    /**
     * This method gets called when the parent lifecycle hits the ON_RESUME event and the object is
     * enabled. It starts the background thread and signals
     * {@link OnCameraEventListener#onResume()}.
     */
    @Override
    protected void onResume()
    {
        // Start the background thread.
        startBackgroundThread();

        // Signal the listener to setup the render targets. The listener is responsible of
        // continuing the startup by calling onTargetsReady.
        mEventListener.onResume();
    }


    /**
     * This method gets called when the parent lifecycle hits the ON_PAUSE event and the object is
     * enabled. It closes the camera, stops the background thread and signals
     * {@link OnCameraEventListener#onPaused()}.
     */
    @Override
    protected void onPause()
    {
        // Close camera and stop background thread.
        closeCamera();
        stopBackgroundThread();

        // Signal the listener this pause.
        mEventListener.onPaused();
    }


    /**
     * This method creates and starts a new background thread and looper for the camera to work
     * asynchronously. Here the {@link TSDBaseCamera#mBackgroundHandler looperHandler} get set,
     * which can later be used to load work in the background thread.
     */
    private void startBackgroundThread()
    {
        // Create thread handler, start it and create a handle to the looper.
        // Set highest possible thread priority.
        mBackgroundThread = new HandlerThread("camera_thread");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
        Process.setThreadPriority(mBackgroundThread.getThreadId(), -20);
    }


    /**
     * This method quits the started background thread safely by letting it stop processing already
     * received tasks. Then it joins it to make sure its closed.
     */
    private void stopBackgroundThread()
    {
        if (mBackgroundThread != null)
        {
            // Quit the thread.
            mBackgroundThread.quitSafely();

            // Try to join the thread and clean up.
            try
            {
                mBackgroundThread.join();
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
            finally
            {
                mBackgroundThread = null;
                mBackgroundHandler = null;
            }
        }
    }


    /**
     * This method tries to open the camera using the {@link TSDCameraManager} given inside the
     * {@link TSDBaseCamera#mCharacter camera character}. When the opening was successful then
     * {@link OnCameraEventListener#onResume()} gets signaled.
     */
    private void openCamera()
    {
        try
        {
            if (mCameraOpenCloseLock.tryAcquire(2000,
                    TimeUnit.MILLISECONDS))
            {
                mCharacter.getManager().openCamera(this);
            }
            else
            {
                Log.e("TSDBaseCamera",
                        "Failed to acquire camera lock.");
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }


    /**
     * This method signals {@link OnCameraEventListener#onClose()} and then closes the camera.
     */
    private void closeCamera()
    {
        // Get lock on the camera semaphore. Then close and cleanup session and camera device.
        // Finally release the lock again.
        try
        {
            mCameraOpenCloseLock.acquire();

            // Signal the listener close before camera closing.
            mEventListener.onClose();
            if (mCamera != null)
            {
                mCamera.close();
                mCamera = null;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            mCameraOpenCloseLock.release();
        }
    }


    /**
     * This interface defines an observer to be implemented by users of the TSDBaseCamera class
     * signaling them the events in the lifecycle of the camera.
     */
    public interface OnCameraEventListener
    {
        /**
         * This method gets called when the camera should get started or resumed. It should be used
         * for setting up targets. When having this done, the camera resume can be continued by
         * calling onTargetsReady().
         */
        void onResume();

        /**
         * This method gets called when the camera was successfully opened. It can be used to start
         * a capture session.
         * @param camera Camera device having been opened.
         */
        void onOpened(CameraDevice camera);

        /** This method gets called before the camera gets closed. It should be used for destroying
         * saved session objects.
         */
        void onClose();

        /**
         * This method gets called after the camera was closed. It should be used for closing
         * initialized image readers.
         */
        void onPaused();
    }
}
