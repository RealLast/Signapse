/*
 * Core code work by Jakob Hampel
 * Porting and improvements by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.location.Location;
import android.location.LocationListener;
import android.location.LocationProvider;
import android.os.Bundle;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * This class defines a {@link LocationListener} being signaled on every location event and
 * signaling a {@link TSDSpeedTracker.OnSpeedCalculatedListener speed listener} with the internally
 * calculated current speed.
 */
public class TSDLocationListener implements LocationListener
{
    private TSDSpeedTracker.OnSpeedCalculatedListener mListener;
    private TSDRingMemory<LocationPoint> mLocationList = new TSDRingMemory<>(5);


    /**
     * This constructor creates a new instance of TSDLocationListener.
     * @param listener Listener to signal the calculated current speed to.
     */
    TSDLocationListener(TSDSpeedTracker.OnSpeedCalculatedListener listener)
    {
        mListener = listener;
    }


    /**
     * This method gets called every time the sensor registers a change in location.
     * Each new location is saved inside the
     * {@link TSDLocationListener#mLocationList location ring memory}, the average speed is
     * calculated by {@link TSDLocationListener#getAverageSpeed()} and signaled to the
     * {@link TSDLocationListener#mListener speed listener}. This could also be -1, if there
     * are not enough location points collected in the memory.
     *
     * @param location New location determined by the location sensor.
     */
    @Override
    public void onLocationChanged(Location location)
    {
        // Save location and timestamp to list.
        mLocationList.add(new LocationPoint(location, System.currentTimeMillis()));

        // Signal the current speed to the listener.
        mListener.onSpeedCalculated(getAverageSpeed());
    }


    /**
     * This function gets called when the status of the location detection has changed. This will
     * clear the {@link TSDLocationListener#mLocationList} and signal -1 to the
     * {@link TSDLocationListener#mListener speed listener} as current speed.
     * @param provider Location provider causing this status change.
     * @param status The new status of location detection.
     * @param extras Some extra information like number of satellites.
     */
    @Override
    public void onStatusChanged(String provider, int status, Bundle extras)
    {
        // Clear the speed list and signal this change to the listener because speed
        // is currently not available.
        switch (status)
        {
            case LocationProvider.TEMPORARILY_UNAVAILABLE:
            case LocationProvider.OUT_OF_SERVICE:
                onProviderDisabled(provider);
                break;
        }
    }


    /**
     * This method gets called when the provider delivering the location information gets enabled.
     * Here this does nothing.
     * @param provider Location provider getting enabled.
     */
    @Override
    public void onProviderEnabled(String provider) { }


    /**
     * This method gets called when the provider delivering the location information gets disabled.
     * This will clear the {@link TSDLocationListener#mLocationList} and signal -1 to the
     * {@link TSDLocationListener#mListener speed listener} as current speed.
     * @param provider Location provider getting disabled.
     */
    @Override
    public void onProviderDisabled(String provider)
    {
        // Clear the speed list.
        mLocationList.clear();

        // Signal the change to the listener.
        mListener.onSpeedCalculated(getAverageSpeed());
    }


    /**
     * Calculates the distance between two geographic coordinates (longitude and latitude) with
     * the approximation of a flat surface.
     *
     * Distance between two latitudes is 60 nautical miles (111.12 kilometers).
     * Distance between two longitudes depends on the latitude.
     *
     * @param prevLoc (location previously) Defines the position A with longitude and latitude
     * @param aftLoc  (location afterwards) Defines the position B with longitude and latitude
     *
     * Parameters prevLoc and aftLoc can be interchanged and the result will stay the same.
     *
     * @return Distance between position A and B in meters
     */
    private double getDistanceBetweenGeographicCoordinates(Location prevLoc, Location aftLoc)
    {
        double distanceLongitude = 111120 * Math.cos(Math.toRadians(prevLoc.getLatitude()))
                * (prevLoc.getLongitude() - aftLoc.getLongitude());
        double distanceLatitude = 111120 * (prevLoc.getLatitude() - aftLoc.getLatitude());

        // because of the very small angle difference, the earth surface will be assumed as flat
        // here
        return Math.sqrt(distanceLatitude * distanceLatitude
                + distanceLongitude * distanceLongitude);
    }


    /**
     * This method calculates the average speed out of the saved locations in
     * {@link TSDLocationListener#mLocationList}. As long as the ring memory is not completely
     * filled -1 will be returned.
     * For smoothing of the data the idea presented
     * <a href='https://stackoverflow.com/a/1134731/5529479'>here</a> is used.
     * The point identified as farthest away from every else point will be skipped while calculating
     * the part speeds A->B, B->C, ... . Those part speeds then are summed up and the average gets
     * returned.
     * @return Average speed or -1, if no speed data is available.
     */
    private Integer getAverageSpeed()
    {
        // Return -1, if the location list is not completely filled.
        final int size = mLocationList.size();
        if (size < mLocationList.getMaxSize())
        {
            return -1;
        }
        else
        {
            // Calculate sums of the distances of the cross product of the speeds.
            // If you have the points A, B, C, D, E then the first sum will be AB+AC+AD+AE.
            double[] distSums = new double[mLocationList.size()];
            for (int i = 0; i < distSums.length; i++)
            {
                for (int j = 0; j < distSums.length; j++)
                {
                    if (i != j)
                    {
                        distSums[i] += getDistanceBetweenGeographicCoordinates(
                                mLocationList.get(i).location, mLocationList.get(j).location);
                    }
                }
            }

            // Find the index of the biggest sum to find out, which point is the farthest from
            // every other point.
            int maxIndex = 0;
            for (int i = 1; i < distSums.length; i++)
            {
                if (distSums[i] > distSums[maxIndex])
                {
                    maxIndex = i;
                }
            }

            // Get a list of all locations except the one farthest away.
            List<LocationPoint> locations = new ArrayList<>();
            for (int i = 0; i < mLocationList.size(); i++)
            {
                if (i != maxIndex)
                {
                    locations.add(mLocationList.get(i));
                }
            }

            // Calculate the sum of all part speeds.
            int speedSum = 0;
            for (int i = 1; i < locations.size(); i++)
            {
                double distance = getDistanceBetweenGeographicCoordinates(
                        locations.get(i - 1).location, locations.get(i).location);
                double timespan = locations.get(i).timestamp - locations.get(i - 1).timestamp;
                speedSum += (int) Math.round(3600 * (distance / timespan));
            }

            // Return the average speed. Do NOT divide by locations.size(). If the array contains
            // 5 points, then just 4 speeds are calculated!
            return Math.round(speedSum / (float)Math.max(locations.size() - 1, 0));
        }
    }


    /**
     * This class is used as a small wrapper to pack location and time data together.
     */
    private class LocationPoint
    {
        final Location location;
        final long timestamp;


        /**
         * This constructor creates a new instance of LocationPoint.
         * @param location Location at the given timestamp.
         * @param timestamp Timestamp the given location was detected.
         */
        LocationPoint(Location location, long timestamp)
        {
            this.location = location;
            this.timestamp = timestamp;
        }
    }
}
