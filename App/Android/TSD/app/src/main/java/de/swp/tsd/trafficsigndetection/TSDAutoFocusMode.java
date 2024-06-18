/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;


/**
 * This enum wraps the possible autofocus modes of the camera instance according to the
 * <a href="https://developer.android.com/reference/android/hardware/camera2/CaptureRequest.html#CONTROL_AF_MODE">Android API</a>
 * into an enum. See the documentation for detailed information.
 */
public enum TSDAutoFocusMode
{
    /**
     * The autofocus is disabled and lens distance gets controlled by the application.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#CONTROL_AF_MODE_OFF">CONTROL_AF_MODE_OFF</a>
     */
    OFF(0),

    /**
     * This will activate the autofocus jumping in when it gets triggered.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#CONTROL_AF_MODE_AUTO">CONTROL_AF_MODE_AUTO</a>
     */
    AUTO(1),

    /**
     * This will activate the autofocus for close-up captures when it gets triggered.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#CONTROL_AF_MODE_MACRO">CONTROL_AF_MODE_MACRO</a>
     */
    MACRO(2),

    /**
     * This will activate the autofocus for videos trying to achieve a constant focused image
     * stream.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#CONTROL_AF_MODE_CONTINUOUS_VIDEO">CONTROL_AF_MODE_CONTINUOUS_VIDEO</a>
     */
    CONTINUOUS_VIDEO(3),

    /**
     * This will activate the autofocus for continuous picture taking jumping in on trigger and
     * trying to focus as fast as possible.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#CONTROL_AF_MODE_CONTINUOUS_PICTURE">CONTROL_AF_MODE_CONTINUOUS_PICTURE</a>
     */
    CONTINUOUS_PICTURE(4),

    /**
     * This will activate an autofocus mode automatically delivering extended depth of field
     * effects.
     * <a href="https://developer.android.com/reference/android/hardware/camera2/CameraMetadata#CONTROL_AF_MODE_EDOF">CONTROL_AF_MODE_EDOF</a>
     */
    EDOF(5);


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
    TSDAutoFocusMode(int value)
    {
        this.value = value;
    }

    /**
     * This function can convert a given value which is one of the constant describing autofocus
     * modes in the Android API to the corresponding enum value. If the given value does not match
     * any defined enum, then OFF will be returned.
     * @param value Integer that should be converted.
     * @return Enum value that corresponds to the given int value. If not matching one is found,
     * OFF will be returned.
     */
    public static TSDAutoFocusMode fromInt(int value)
    {
        for (TSDAutoFocusMode mode : TSDAutoFocusMode.values())
        {
            if (value == mode.value)
                return mode;
        }

        return OFF;
    }
}
