/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;


import android.support.annotation.NonNull;
import android.util.Log;

/**
 * This class defines an {@link TSDShutterCameraImageListener.FrameEventListener} which uses the
 * given information to calculate an approximation of the maximum speed (cut-off speed) which
 * should still allow the device to properly detect the signs on the captured images. The background
 * to this is, that sign detection is extremely processing intensive and pushes even high-end
 * phones from two years ago to the limits. That is why this class tries to use the given
 * time which was taken to process single frames and further information to calculate an
 * approximated value describing the maximum speed the device can move and still should be fast
 * enough to capture and process enough frames to get every sign around there.
 */
public class TSDCutOffSpeedTracker implements TSDShutterCameraImageListener.FrameEventListener
{
    /**
     * Describes the width of the input images given into the detector which gets used to detect
     * the signs.
     */
    private final static int DETECTOR_INPUT_IMAGE_WIDTH = 300;

    /**
     * Describes the minimum pixel width a sign needs on screen to get detected under normal
     * circumstances in the image with the {@link TSDCutOffSpeedTracker#DETECTOR_INPUT_IMAGE_WIDTH}.
     */
    private final static int SIGN_MINIMUM_PIXEL_WIDTH = 12;

    /**
     * Describes the width of signs in meter used as reference value in calculations. This is the
     * usual diameter in meter of a speed limiting sign in the range from 20 to 80 km/h. This is an
     * important sign and not too big. So we take it as reference value.
     * <a href='http://www.schilder-online-bestellen.de/groesse-verkehrszeichen'>Source</a>
     */
    private final static float SIGN_REFERENCE_WIDTH = 0.64f;

    /**
     * Describes the factor to be multiplied with the maximum distance that a sign can have while
     * still getting detected to calculate a save maximum speed. This should stay positive and below
     * 1. This value at 1 would mean that signs on the edge to being too far away (or too close) for
     * detection are still calculated to get detected. This is a tight fit, so take a lower positive
     * value.
     */
    private final static float DISTANCE_THRESHOLD_FACTOR = 0.95f;

    /**
     * Describes the factor which can be multiplied with a given distance to calculate how much of
     * a ruler one could see in the captured image, if it would be that distance away from the
     * camera.
     * The factor is calculated by the at construction given view angle of the camera:
     * mLensFactor = 2 * cotan((PI - angle) / 2)
     */
    private float mLensFactor;

    /**
     * Describes the maximum distance in meters a sign can be away while still getting detected.
     * It is calculated in this way:
     * The width of a sign in pixel at a given distance on an image with a given pixel width can be
     * calculated with the following formula:
     *
     * signWidth = {@link TSDCutOffSpeedTracker#DETECTOR_INPUT_IMAGE_WIDTH} *
     * {@link TSDCutOffSpeedTracker#SIGN_REFERENCE_WIDTH} /
     * ( distance * {@link TSDCutOffSpeedTracker#mLensFactor} )
     *
     * We use this formula with the given minimum pixel width
     * {@link TSDCutOffSpeedTracker#SIGN_MINIMUM_PIXEL_WIDTH}, which is necessary to detect the
     * signs, to calculate the maximum distance signs can be away to still get detected in this way:
     *
     * mMaxDistance = {@link TSDCutOffSpeedTracker#DETECTOR_INPUT_IMAGE_WIDTH} *
     * {@link TSDCutOffSpeedTracker#SIGN_REFERENCE_WIDTH} /
     * ( {@link TSDCutOffSpeedTracker#mLensFactor} *
     * {@link TSDCutOffSpeedTracker#SIGN_MINIMUM_PIXEL_WIDTH} );
     */
    private float mMaxDistance;

    /**
     * Holds an collection of the last calculated cut-off speeds to be able to calculate an average
     * value.
     */
    private TSDRingMemory<Integer> mSpeedList = new TSDRingMemory<>(5);


    /**
     * Observer which gets signaled when a new cut-off speed was calculated.
     */
    private OnCutOffSpeedCalculatedListener mListener;


    /**
     * This constructor creates a new instance of {@link TSDCutOffSpeedTracker}.
     * @param listener Observer which gets notified when a new cut-off speed was calculated.
     * @param cameraViewAngle View angle of the camera in use to capture the frames in radians.
     */
    TSDCutOffSpeedTracker(@NonNull OnCutOffSpeedCalculatedListener listener, float cameraViewAngle)
    {
        mListener = listener;
        setCameraViewAngle(cameraViewAngle);
    }


    /**
     * Setter for the camera view angle. This will calculate and save the corresponding
     * {@link TSDCutOffSpeedTracker#mLensFactor} and {@link TSDCutOffSpeedTracker#mMaxDistance}.
     * @param cameraViewAngle Maximum view angle the camera supports in radians.
     */
    public void setCameraViewAngle(float cameraViewAngle)
    {
        // Calculate the lens factor as described in the comment to mLensFactor.
        mLensFactor = 2.0f * (1.0f / (float)Math.tan((Math.PI - cameraViewAngle) / 2.0f));

        // Calculate the maximum distance a sign can be away while still getting detected.
        mMaxDistance = DETECTOR_INPUT_IMAGE_WIDTH * SIGN_REFERENCE_WIDTH /
                ( mLensFactor * SIGN_MINIMUM_PIXEL_WIDTH );
    }


    /**
     * This method is called every time one camera frame is finished.
     * The given processing time and image width is used to calculate an approximated value for the
     * cut-off speed described in {@link TSDCutOffSpeedTracker}. The calculated value is buffered
     * inside an ring memory and an average of the last values is signaled to the given
     * {@link TSDCutOffSpeedTracker#mListener listener}.
     *
     * The calculation is as follows:
     *
     * The maximum currently acceptable speed would be this, that would let each sign at least one
     * time to be detected on a captured image. Taking the currently known processing time for one
     * frame and the maximum distance a sign can be away while still getting detected we can
     * calculate this speed by the simple speed formula v = s / t. But to give a little
     * threshold to not depend on edge cases we multiply the distance by a threshold value
     * {@link TSDCutOffSpeedTracker#DISTANCE_THRESHOLD_FACTOR} a little smaller than 1.
     *
     * @param duration Time it took to process the frame in milliseconds.
     */
    @Override
    public void onFrameFinished(long duration)
    {
        // Calculate the maximum speed which still allows correct detection in km/h .
        int cutOffSpeed = Math.round(3.6f * DISTANCE_THRESHOLD_FACTOR * mMaxDistance /
                (duration / 1000f));

        // Save the calculated and signal the average to the listener.
        mSpeedList.add(cutOffSpeed);
        mListener.onCutOffSpeedCalculated(getAverageCutOffSpeed());
    }


    /**
     * This method calculates the average cut-off speed out of the saved speeds in
     * {@link TSDCutOffSpeedTracker#mSpeedList}. As long as the ring memory is not completely filled
     * null will be returned.
     * @return Average cut-off speed or null, if no cut-off speed can be given.
     */
    private Integer getAverageCutOffSpeed()
    {
        // Return null, if the speed list is not completely filled.
        final int size = mSpeedList.size();
        if (size < mSpeedList.getMaxSize())
        {
            return null;
        }
        else
        {
            // Get sum of all speeds.
            int average = 0;
            for (Integer speed : mSpeedList)
            {
                average += speed;
            }

            return Math.round(average / (float)size);
        }
    }


    /**
     * This interface defines an observer to be implemented by users of the
     * {@link TSDCutOffSpeedTracker} signaling them when a new cut-off speed was calculated.
     */
    public interface OnCutOffSpeedCalculatedListener
    {
        /**
         * This method gets called whenever a new cut-off speed was calculated. The cut-off speed
         * describes the maximum speed the device can move while still being fast enough to detect
         * signs.
         * @param cutOffSpeed Maximum speed in km/h which still allows the device to detect the
         *                    signs on the captured images. If this is null, then no cut-off speed
         *                    can currently be given.
         */
        void onCutOffSpeedCalculated(Integer cutOffSpeed);
    }
}
