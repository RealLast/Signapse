/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.arch.lifecycle.Lifecycle;
import android.support.annotation.NonNull;

import java.util.Timer;
import java.util.TimerTask;


/**
 * This class defines an internal timer triggering the given runnable at a given interval. It is
 * lifecycle aware using the {@link TSDLifecycleObserver} base class.
 */
public class TSDPeriodicTask extends TSDLifecycleObserver
{
    private final long mDelayMillis;
    private final long mPeriodMillis;
    private final Runnable mRunnable;
    private Timer mTimer;


    /**
     * This constructor creates a new instance of {@link TSDPeriodicTask}. This does not enable
     * the object. To enable the object please see {@link TSDLifecycleObserver#enable()}.
     * @param delayMillis Delay after the runnable should get run the first time.
     * @param periodMillis Interval the runnable should get run in in milliseconds.
     * @param lifecycle Lifecycle of the parent starting and stopping the timer.
     */
    TSDPeriodicTask(long delayMillis, long periodMillis, @NonNull Runnable runnable,
                    @NonNull Lifecycle lifecycle)
    {
        // Initialize the lifecycle listening.
        super(lifecycle);

        mRunnable = runnable;
        mDelayMillis = delayMillis;
        mPeriodMillis = periodMillis;
    }


    /**
     * This method gets called when the parent lifecycle hits the ON_RESUME event and the object
     * is enabled. It starts the timer when the object is enabled.
     */
    @Override
    protected void onResume()
    {
        // Create a timer running the given runnable in the given interval.
        mTimer = new Timer();
        mTimer.scheduleAtFixedRate(new TimerTask()
        {
            @Override
            public void run()
            {
                mRunnable.run();
            }

        }, mDelayMillis, mPeriodMillis);
    }


    /**
     * This method gets called when the parent lifecycle hits the ON_PAUSE event and the object is
     * enabled. It cancels the timer and cleans it up.
     */
    @Override
    protected void onPause()
    {
        // Clear the timer.
        if (mTimer != null)
        {
            mTimer.cancel();
            mTimer.purge();
            mTimer = null;
        }
    }
}
