/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.arch.lifecycle.Lifecycle;
import android.arch.lifecycle.LifecycleObserver;
import android.arch.lifecycle.OnLifecycleEvent;
import android.support.annotation.NonNull;

/**
 * {@link TSDLifecycleObserver} is the base class for every lifecycle aware object in this project.
 * It is implemented to pause and resume every child instance on ON_PAUSE and ON_RESUME events of
 * the parent lifecycle. It also implements a possibility to disable the objects and to find out
 * whether the object currently is between the {@link TSDLifecycleObserver#onResume()} and
 * {@link TSDLifecycleObserver#onPause()} calls.
 */
public abstract class TSDLifecycleObserver implements LifecycleObserver
{
    private final Lifecycle mLifecycle;
    private boolean mEnabled = false;
    private boolean mBetweenResumeAndPause = false;


    /**
     * This constructor creates a new instance of TSDLifecycleObserver.
     * This does not enable the object.
     * @param lifecycle Lifecycle of the parent
     */
    TSDLifecycleObserver(@NonNull Lifecycle lifecycle)
    {
        mLifecycle = lifecycle;
    }


    /**
     * This method queries whether the object should be running or not. When enabled, the object
     * will get paused and resumed by the lifecycle of the parent.
     * @return Enable flag
     */
    public boolean isEnabled()
    {
        return mEnabled;
    }


    /**
     * This method queries whether the objects lifecycle currently is in between resume and pause
     * events.
     * @return Boolean signaling whether the lifecycle is in between ON_RESUME and ON_PAUSE.
     */
    public boolean betweenResumeAndPause()
    {
        return mBetweenResumeAndPause;
    }


    /**
     * This method starts the object if not already happened.
     * It registers the object as observer for the given lifecycle and thus enables automatic start
     * and stop on parent lifecycle PAUSE and RESUME. Then it also calls
     * {@link TSDLifecycleObserver#resume()}.
     */
    public void enable()
    {
        if (!mEnabled)
        {
            // Register this as observer for the parent lifecycle.
            mLifecycle.addObserver(this);

            mEnabled = true;

            // Trigger resume manually if observer is already in resume state.
            if (mBetweenResumeAndPause)
            {
                resume();
            }
        }
    }


    /**
     * This method clears the object if not already happened.
     * It unregisters the object as observer for the given lifecycle and thus disables automatic
     * start and stop on parent lifecycle PAUSE and RESUME.
     */
    public void disable()
    {
        if (mEnabled)
        {
            // Unregister this as observer for the parent lifecycle.
            mLifecycle.removeObserver(this);

            onPause();
            mEnabled = false;
        }
    }


    /**
     * This method is called when the parent lifecycle hits the ON_START event. This is called if
     * enabled or not and could be used for permission requests.
     */
    @OnLifecycleEvent(Lifecycle.Event.ON_START)
    protected void onStart() { }


    /**
     * This method is called when the object should continue working, when the parents lifecycle
     * hits ON_RESUME and the object is enabled or gets enabled.
     */
    protected abstract void onResume();


    /**
     * This method is called when the object should stop or pause working, when the parents
     * lifecycle hits ON_PAUSE and the object is enabled.
     */
    protected abstract void onPause();


    /**
     * This method is called on the parents lifecycle ON_RESUME event and if enabled resumes the
     * object. It can also get called when the object gets enabled. In every case the object only
     * gets actually resumed, if the parent lifecycle is at least in STARTED state.
     */
    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume()
    {
        // Only start if the lifecycle is at least started and the camera is enabled.
        if (mEnabled && mLifecycle.getCurrentState().isAtLeast(Lifecycle.State.STARTED))
        {
            onResume();
        }

        // Set flag to signal being in between resume and pause.
        mBetweenResumeAndPause = true;
    }

    /**
     * This method is called on the parents lifecycle ON_PAUSE event and pauses the object, when
     * it is enabled.
     */
    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause()
    {
        // Set flag to signal being resume phase.
        mBetweenResumeAndPause = false;

        // Avoid errors when the object was never started because it was disabled.
        if (mEnabled)
        {
            onPause();
        }
    }
}
