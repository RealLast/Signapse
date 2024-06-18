/*
 * First core implementation by Jakob Hampel
 * Improvements and porting by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.arch.lifecycle.Lifecycle;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.support.annotation.NonNull;

import static android.content.Context.SENSOR_SERVICE;


/**
 * This class defines a lifecycle aware wrapper around the gravitation sensor calculating the
 * device roll (rotation around the z axis) and pitch (rotation around the y axis), which get
 * signaled every change to an {@link OnRotationListener}.
 */
public class TSDRotationSensor extends TSDLifecycleObserver {
    private final OnRotationListener mOnRotationListener;
    private Activity mActivity;
    private SensorManager mManager;
    private RotationListener mListener;


    /**
     * This constructor creates a new instance of {@link TSDRotationSensor}. This does not enable
     * the sensor listening. To enable the object please see {@link TSDLifecycleObserver#enable()}.
     *
     * @param rotationListener Observer which should get notified on rotation events.
     * @param lifecycle        Lifecycle of the parent.
     * @param activity         Activity of the parent.
     */
    TSDRotationSensor(@NonNull OnRotationListener rotationListener, @NonNull Lifecycle lifecycle,
                      @NonNull Activity activity) {
        // Initialize the lifecycle listening.
        super(lifecycle);

        mOnRotationListener = rotationListener;
        mActivity = activity;
    }


    /**
     * This method gets called when the parent lifecycle hits the ON_RESUME event and the object
     * is enabled. It starts the sensor listening.
     */
    @Override
    protected void onResume() {
        // Get sensor manager.
        mManager = (SensorManager) mActivity.getSystemService(SENSOR_SERVICE);

        // Ger the accelerometer.
        if (mManager != null) {
            Sensor sensor = mManager.getDefaultSensor(Sensor.TYPE_GRAVITY);

            // Register the listener, if sensor is available.
            if (sensor != null) {
                // Create listener and register it.
                mListener = new RotationListener();
                mManager.registerListener(mListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
            }
        }
    }


    /**
     * This method gets called when the parent lifecycle hits the ON_PAUSE event and the object is
     * enabled. It stops the sensor listening.
     */
    @Override
    protected void onPause() {
        // remove and cleanup rotation listener.
        if (mListener != null) {
            mManager.unregisterListener(mListener);
            mListener = null;
        }
    }


    /**
     * This interface defines the listener interface for rotation events. Implement this and
     * give it to a {@link TSDRotationSensor} constructor to get the rotation of the device
     * every time it changes.
     */
    public interface OnRotationListener {
        /**
         * This method gets called on every sensor event delivering the roll and pitch of the
         * device.
         *
         * @param roll  Device roll (z-axis rotation) in degrees.
         * @param pitch Device pitch (y-axis rotation) in degrees.
         */
        void onRotation(int roll, int pitch);
    }


    /**
     * This class defines the internal listener for sensor events calculating the device rotations
     * roll (z) and pitch (y) in degrees based on the gravity information delivered by the sensor
     * and signaling the results to the given listener.
     */
    private class RotationListener implements SensorEventListener
    {
        /**
         * This method gets called when the information from the sensor changed. This calculates
         * the roll and pitch of the device in degrees and signals this information to the listener.
         * @param event Event containing the new sensor information.
         */
        @Override
        public void onSensorChanged(SensorEvent event)
        {
            // Get gravity towards axis.
            float x = event.values[0];
            float y = event.values[1];
            float z = event.values[2];

            //       x
            //       ^
            //       |
            //   ---------
            //  |         |
            //  |    z    | -> y
            //  |         |
            //   ---------

            // Calculate rotation around the z axis, the roll
            int roll = (int) Math.toDegrees(Math.atan(y / x));

            // Calculate rotation around the y axis, the pith. Use the absolute of the x axis to
            // be invariant to the device rotation.
            int pitch = (int) Math.toDegrees(Math.atan(z / Math.abs(x)));

            // Signal the listener.
            mOnRotationListener.onRotation(roll, pitch);
        }

        /**
         * This method gets called when the accuracy of the sensor changes. This does not get used.
         * @param sensor Sensor the accuracy changes of.
         * @param accuracy The new accuracy.
         */
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) { }
    }
}
