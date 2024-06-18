/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;


/**
 * This enum wraps the possible modes of optical image stabilization of the camera instance
 * according to the
 * <a href="https://developer.android.com/reference/android/hardware/camera2/CaptureRequest.html#LENS_OPTICAL_STABILIZATION_MODE">Android API</a>
 * into an enum. See the documentation for detailed information.
 */
public enum TSDOpticalStabilizationMode
{
    /**
     * The ois is not available.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#LENS_OPTICAL_STABILIZATION_MODE_OFF">LENS_OPTICAL_STABILIZATION_MODE_OFF</a>
     */
    OFF(0),

    /**
     * The ois is enabled.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#LENS_OPTICAL_STABILIZATION_MODE_ON">LENS_OPTICAL_STABILIZATION_MODE_ON</a>
     */
    ON(1);

    /**
     * This value describes the value of the constant in the Android API describing the same mode
     * this enum wraps.
     */
    public final int value;

    /**
     * This constructor creates a new enum instance setting the internal enum value to the given
     * one.
     * @param value This value should match the value of the constant in the Android API describing
     *              the same mode.
     */
    TSDOpticalStabilizationMode(int value)
    {
        this.value = value;
    }

    /**
     * This function can convert a given value which is one of the constant describing ois
     * modes in the Android API to the corresponding enum value. If the given value does not match
     * any defined enum, then OFF will be returned.
     * @param value Integer that should be converted.
     * @return Enum value that corresponds to the given int value. If not matching one is found,
     * OFF will be returned.
     */
    public static TSDOpticalStabilizationMode fromInt(int value)
    {
        for (TSDOpticalStabilizationMode mode : TSDOpticalStabilizationMode.values())
        {
            if (value == mode.value)
                return mode;
        }

        return OFF;
    }
}
