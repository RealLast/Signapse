/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.graphics.Rect;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.util.Size;
import android.util.SizeF;


/**
 * Describes the characteristics or features of a corresponding camera instance. This is an advanced
 * wrapper around {@link CameraCharacteristics} implementing methods to get certain information
 * when necessary already converted from constants to wrapper enums like {@link TSDAutoFocusMode}.
 */
public class TSDCameraCharacter
{
    /**
     * This threshold value describes how much the aspect ratio of the selected image output size of
     * the camera is allowed to differ from the given wanted aspect ratio.
     */
    private final static float ASPECT_RATIO_THRESHOLD = 0.05f;

    /**
     * This is an hardcoded value unique to every camera device inside the device used to identify
     * it.
     */
    private final String mId;

    private final CameraCharacteristics mCharacter;

    private final TSDCameraManager mManager;


    /**
     * This constructor creates a new instance wrapping the given information for later use.
     * @param id Id of the corresponding camera character queried by the given camera manager.
     * @param character Characteristics of the corresponding camera queried by the given camera
     *                  manager.
     * @param manager Camera manager the given information were queried with.
     */
    TSDCameraCharacter(String id, CameraCharacteristics character, TSDCameraManager manager)
    {
        mId = id;
        mCharacter = character;
        mManager = manager;
    }


    /**
     * Getter for the camera id. This is an hardcoded value unique to every camera device inside
     * the device used to identify it.
     * @return Camera id
     */
    public String getId()
    {
        return mId;
    }


    /**
     * Getter for the parent camera manager used to query these characteristics.
     * @return Camera manager
     */
    public TSDCameraManager getManager()
    {
        return mManager;
    }


    /**
     * Getter for raw characteristics of this camera. Use the already given methods to query
     * features instead if sufficient. This is the character given the instance at construction.
     * @return Raw camera characteristics
     */
    public CameraCharacteristics getCameraCharacteristics()
    {
        return mCharacter;
    }


    /**
     * Queries the clockwise angle the output image needs to be rotated to be upright on the screen
     * of the device in its native orientation.
     * @return Sensor orientation in [0, 90, 180, 270] degrees. If the value could not be queried,
     *         then null will be returned.
     */
    public Integer getSensorRotation()
    {
        return mCharacter.get(CameraCharacteristics.SENSOR_ORIENTATION);
    }


    /**
     * This method queries the facing direction of the corresponding camera.
     * @return The facing of the camera. In case there is no such information available,
     * EXTERNAL will be returned.
     */
    public TSDLensFacing getLensFacing()
    {
        Integer value = mCharacter.get(CameraCharacteristics.LENS_FACING);

        if (value != null)
        {
            return TSDLensFacing.fromInt(value);
        }

        return TSDLensFacing.EXTERNAL;
    }


    /**
     * This method queries all of the available modes for autofocus of the corresponding camera
     * instance.
     * @return Array of autofocus modes. In case there are no such information, this will be empty.
     */
    public TSDAutoFocusMode[] getAvailableAutoFocusModes()
    {
        int[] values = mCharacter.get(CameraCharacteristics.CONTROL_AF_AVAILABLE_MODES);

        if (values != null)
        {
            TSDAutoFocusMode[] modes = new TSDAutoFocusMode[values.length];

            for (int i = 0; i < values.length; i++)
            {
                modes[i] = TSDAutoFocusMode.fromInt(values[i]);
            }

            return modes;
        }

        return new TSDAutoFocusMode[0];
    }


    /**
     * This method queries all of the available modes for optical image stabilization of the
     * corresponding camera instance.
     * @return Array of ois modes. In case there are no such information, this will be empty.
     */
    public TSDOpticalStabilizationMode[] getAvailableOisModes()
    {
        int[] values = mCharacter
                .get(CameraCharacteristics.LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION);

        if (values != null)
        {
            TSDOpticalStabilizationMode[] modes = new TSDOpticalStabilizationMode[values.length];

            for (int i = 0; i < values.length; i++)
            {
                modes[i] = TSDOpticalStabilizationMode.fromInt(values[i]);
            }

            return modes;
        }

        return new TSDOpticalStabilizationMode[0];
    }


    /**
     * This method queries all of the available apertures for the corresponding camera instance.
     * @return Array of available apertures. In case there are no such information, this will be
     * empty.
     */
    public float[] getAvailableApertures()
    {
        float[] values = mCharacter.get(CameraCharacteristics.LENS_INFO_AVAILABLE_APERTURES);

        if (values != null)
        {
            return values;
        }

        return new float[0];
    }


    /**
     * This method queries the stream config map holding information to setup the target surfaces
     * for capturing images.
     * @return Map holding stream configuration.
     */
    private StreamConfigurationMap getStreamConfigurations()
    {
        return mCharacter.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
    }


    /**
     * This method queries all of the available output formats for the corresponding camera
     * instance.
     * @return Array of available output formats. This array can be empty.
     */
    public TSDCameraOutputFormat[] getAvailableOutputFormats()
    {
        int[] values = getStreamConfigurations().getOutputFormats();

        if (values != null)
        {
            TSDCameraOutputFormat[] formats = new TSDCameraOutputFormat[values.length];

            for (int i = 0; i < values.length; i++)
            {
                formats[i] = TSDCameraOutputFormat.fromInt(values[i]);
            }

            return formats;
        }

        return new TSDCameraOutputFormat[0];
    }


    /**
     * This method queries the available output image sizes of the corresponding camera instance for
     * the given image format.
     * @param format The format available sizes should be queried for.
     * @return Array of available output sizes for the format.
     */
    public Size[] getAvailableOutputSizes(TSDCameraOutputFormat format)
    {
        return getStreamConfigurations().getOutputSizes(format.value);
    }


    /**
     * This method calculates the maximum supported view angle of the corresponding
     * camera in radians. For more information at the calculation take a look at
     * <a href='https://stackoverflow.com/questions/39965408/what-is-the-android-camera2-api-equivalent-of-camera-parameters-gethorizontalvie'>this question</a>
     * on stackoverflow. Internally it calculates both the vertical and horizontal view angle
     * and returns the greater one of them.
     * @return Either the maximum supported view angle in radians or null, if something went wrong.
     */
    public Double getMaximumViewAngle()
    {
        // Get the supported focal lengths of the camera lens.
        float[] focalLengths = mCharacter
                .get(CameraCharacteristics.LENS_INFO_AVAILABLE_FOCAL_LENGTHS);

        Double xViewAngle = null;
        Double yViewAngle = null;

        if (focalLengths != null && focalLengths.length > 0)
        {
            // Get the actual physical size of the camera sensor.
            SizeF sensorSize = mCharacter.get(CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE);

            // Get the pixel dimensions of the sensor.
            Size pixelSize = mCharacter.get(CameraCharacteristics.SENSOR_INFO_PIXEL_ARRAY_SIZE);

            // Get the pixel dimensions of the sensor actually in use.
            Rect activeSize = mCharacter.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);

            if (sensorSize != null && pixelSize != null && activeSize != null)
            {
                // Calculate the physical dimensions of the active sensor part.
                float width = sensorSize.getWidth() *
                        ( activeSize.width() / (float)pixelSize.getWidth() );
                float height = sensorSize.getHeight() *
                        ( activeSize.height() / (float)pixelSize.getHeight() );

                // Calculate the view angles using the first focal length. This is the
                // smallest one and should result in the biggest possible focal length.
                xViewAngle = 2f * Math.atan( width / (2f * focalLengths[0]) );
                yViewAngle = 2f * Math.atan( height / (2f * focalLengths[0]) );
            }
        }

        // Return null or the biggest angle.
        if (xViewAngle != null)
        {
            return xViewAngle > yViewAngle ? xViewAngle : yViewAngle;
        }

        return null;
    }


    /**
     * This method queries for the best matching at least as big supported output image size of this
     * camera for given target size, aspect ration and format.
     * @param format Format of the output image of the camera which should have this queried size.
     *               This needs to be a by the camera supported format and can be queried by
     *               {@link TSDCameraCharacter#getAvailableOutputFormats()}.
     * @param targetSize Target size for the image the camera should output. The returned size will
     *                   be at least as big as this target size and will have the same aspect ratio.
     * @return Either the found supported size or null, if no matching size was found.
     */
    public Size getMatchingOutputSize(TSDCameraOutputFormat format, Size targetSize)
    {
        Size[] availableSizes = getAvailableOutputSizes(format);
        Size matchSize = null;

        if (availableSizes.length > 0)
        {
            matchSize = availableSizes[0];
            boolean found = false;

            // Get target aspect ration
            float aspect_ratio = targetSize.getWidth() / (float)targetSize.getHeight();

            // Search for the best matching but not bigger supported image size matching the
            // given target.
            for (Size s : availableSizes)
            {
                float ar = s.getWidth() / (float)s.getHeight();

                if (ar <= aspect_ratio + ASPECT_RATIO_THRESHOLD
                        && ar >= aspect_ratio - ASPECT_RATIO_THRESHOLD
                        && s.getWidth() >= targetSize.getWidth()
                        && s.getHeight() >= targetSize.getHeight()
                        && s.getWidth() <= matchSize.getWidth()
                        && s.getHeight() <= matchSize.getHeight())
                {
                    matchSize = s;
                    found = true;
                }
            }

            // If no matching size was found, set it to null again.
            if (!found)
            {
                matchSize = null;
            }
        }

        return matchSize;
    }


    /**
     * This method calculates a score for this camera character. This can be used to compare the
     * cameras built into an device and to choose the best one. A bigger score usually means a
     * better camera for our use case in this app. The score is calculated using the direction of
     * the camera as biggest influence (it should match the given direction).
     * Furthermore the number of available Ois, the minimum supported aperture (smaller is better)
     * and the availability of the autofocus modes AUTO and OFF are taken into consideration for
     * a better score.
     * @param facing The direction the camera should face
     * @return Score of this camera
     */
    public int getScore(TSDLensFacing facing)
    {
        int score = 0;

        // The correct facing direction is most important
        if (getLensFacing() == facing)
        {
            score += 100;
        }

        // Ois is important, just taking the number of ois modes as score.
        score += getAvailableOisModes().length * 50;

        // A small aperture usually means a better camera.
        float[] apertures = getAvailableApertures();
        float minAperture = Float.MAX_VALUE;
        for (float a : apertures)
        {
            if (a < minAperture)
            {
                minAperture = a;
            }
        }
        score += (int)(100 / minAperture);

        // We need an autofocus which can be deactivated but controlled.
        TSDAutoFocusMode[] afModes = getAvailableAutoFocusModes();
        boolean afAuto = false;
        boolean afOff = false;
        for (TSDAutoFocusMode mode : afModes)
        {
            afAuto = afAuto || mode == TSDAutoFocusMode.AUTO;
            afOff = afOff || mode == TSDAutoFocusMode.OFF;
        }

        if (afAuto && afOff)
        {
            score += 100;
        }

        return score;
    }
}
