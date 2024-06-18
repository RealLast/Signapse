/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;


/**
 * Describes wraps the possible lens facing direction of the camera instance according to the
 * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraCharacteristics.html#LENS_FACING">Android API</a>
 * into an enum. See the documentation for detailed information.
 */
public enum TSDLensFacing
{
    /**
     * The camera is located at the front of the device.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#LENS_FACING_FRONT">LENS_FACING_FRONT</a>
     */
    FRONT(0),

    /**
     * The camera is located at the back of the device.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#LENS_FACING_BACK">LENS_FACING_BACK</a>
     */
    BACK(1),

    /**
     * The camera is located externally of the device.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#LENS_FACING_EXTERNAL">LENS_FACING_EXTERNAL</a>
     */
    EXTERNAL(2);

    /**
     * This value describes the value of the constant in the Android API describing the same
     * direction this enum wraps.
     */
    public final int value;

    /**
     * This constructor creates a new enum instance setting the internal enum value to the given
     * one.
     * @param value This value should match the value of the constant in the Android API describing
     *              the same direction.
     */
    TSDLensFacing(int value)
    {
        this.value = value;
    }

    /**
     * This function can convert a given value which is one of the constant describing lens facing
     * directions in the Android API to the corresponding enum value. If the given value does not
     * match any defined enum, then EXTERNAL will be returned.
     * @param value Integer that should be converted.
     * @return Enum value that corresponds to the given int value. If not matching one is found,
     * EXTERNAL will be returned.
     */
    public static TSDLensFacing fromInt(int value)
    {
        for (TSDLensFacing mode : TSDLensFacing.values())
        {
            if (value == mode.value)
                return mode;
        }

        return EXTERNAL;
    }
}
