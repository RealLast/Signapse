/*
 * Core code work by Jakob Hampel
 * Porting and improvements by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.arch.lifecycle.Lifecycle;
import android.content.Context;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Looper;
import android.support.annotation.NonNull;


/**
 * This class defines a location service user which uses the GPS service with the
 * {@link TSDLocationListener} to determine the current speed of the device.
 * It is lifecycle aware using the {@link TSDLifecycleObserver} base class.
 * @see TSDLocationListener
 */
public class TSDSpeedTracker extends TSDLifecycleObserver
{
    private final OnSpeedCalculatedListener mSpeedListener;
    private final LocationManager mLocationManager;
    private LocationListener mLocationListener;


    /**
     * This constructor creates a new instance of TSDSpeedTracker. This does not enable
     * the speed tracking. To enable the object please see {@link TSDLifecycleObserver#enable()}.
     * @param lifecycle Lifecycle of the parent.
     * @param activity Activity the permission request should be handled in.
     * @param speedListener Speed observer being signaled on every speed event.
     */
    TSDSpeedTracker(@NonNull Lifecycle lifecycle, @NonNull Activity activity,
                    @NonNull OnSpeedCalculatedListener speedListener)
    {
        // Initialize lifecycle observation.
        super(lifecycle);

        mSpeedListener = speedListener;
        mLocationManager = (LocationManager)activity.getSystemService(Context.LOCATION_SERVICE);
    }

    /**
     * This method gets called when the parent lifecycle hits the ON_RESUME event and the object
     * is enabled. It initializes and starts the location listening and speed tracking.
     */
    @Override
    protected void onResume()
    {
        // Start location listener
        startLocationListener();
    }

    /**
     * This method gets called when the parent lifecycle hits the ON_PAUSE event and the object is
     * enabled. It stop the location listening and speed tracking.
     */
    @Override
    protected void onPause()
    {
        stopLocationListener();
    }


    /**
     * This method initializes and starts the location listening and speed tracking. Whenever some
     * new speed event happens, the {@link TSDSpeedTracker#mSpeedListener} will be signaled.
     */
    private void startLocationListener()
    {
        try
        {
            // Create a new listener for getting location updates.
            mLocationListener = new TSDLocationListener(mSpeedListener);

            // If location permission was granted, request location updates.
            mLocationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,
                    0, 0, mLocationListener, Looper.getMainLooper());
        }
        catch (SecurityException e)
        {
            e.printStackTrace();
        }
    }


    /**
     * This method stops the already started location listening and speed tracking.
     */
    private void stopLocationListener()
    {
        // Remove and cleanup location listener.
        if (mLocationListener != null)
        {
            mLocationManager.removeUpdates(mLocationListener);
            mLocationListener = null;
        }
    }


    /**
     * This interface defines an observer to be implemented by users of the TSDSpeedTracker class
     * signaling them when the speed was calculated.
     */
    public interface OnSpeedCalculatedListener
    {
        /**
         * This method gets called every time the speed was calculated.
         * @param speed Speed of the device in kilometers per hour. This can be null, if the speed
         *              is disabled or negative, if it is currently not available.
         */
        void onSpeedCalculated(Integer speed);
    }
}
