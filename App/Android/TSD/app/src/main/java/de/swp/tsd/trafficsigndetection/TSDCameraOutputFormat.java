/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

/**
 * Describes the possible output formats from the camera device according to the Android API
 * Documents for
 * <a href="https://developer.android.com/reference/android/graphics/ImageFormat">ImageFormat</a>
 * and
 * <a href="https://developer.android.com/reference/android/graphics/PixelFormat">PixelFormat</a>.
 * For detailed descriptions of the enum values please visit these documents.
 */
public enum TSDCameraOutputFormat
{
    // Image Formats
    DEPTH16(1144402265), DEPTH_POINT_CLOUD(257), FLEX_RGBA_8888(42), FLEX_RGB_888(41), JPEG(256),
    NV16(16), NV21(17), PRIVATE(34), RAW10(37), RAW12(38), RAW_PRIVATE(36), RAW_SENSOR(32),
    RGB_565(4), UNKNOWN(0), YUV_420_888(35), YUV_422_888(39), YUV_444_888(40),YUY2(20),
    YV12(842094169),
    // Pixel Formats
    A_8(8), LA_88(10), L_8(9), OPAQUE(-1), RGBA_1010102(43), RGBA_4444(7), GBA_5551(6),
    RGBA_8888(1), RGBA_F16(22), RGBX_8888(2), RGB_332(11), RGB_888(3), TRANSLUCENT(-3),
    TRANSPARENT(-2);

    /**
     * This value describes the value of the constant in the Android API describing the same format
     * this enum wraps.
     */
    public final int value;

    /**
     * This constructor creates a new enum instance setting the internal enum value to the given
     * one.
     * @param value This value should match the value of the constant in the Android API describing
     *              the same format.
     */
    TSDCameraOutputFormat(int value)
    {
        this.value = value;
    }

    /**
     * This function can convert a given value which is one of the constant describing image or
     * pixel formats in the Android API to the corresponding enum value. If the given value does not
     * match any defined enum, then UNKNOWN will be returned.
     * @param value Integer that should be converted.
     * @return Enum value that corresponds to the given int value. If not matching one is found,
     * UNKNOWN will be returned.
     */
    public static TSDCameraOutputFormat fromInt(int value)
    {
        for (TSDCameraOutputFormat mode : TSDCameraOutputFormat.values())
        {
            if (value == mode.value)
            {
                return mode;
            }
        }

        return UNKNOWN;
    }
}
