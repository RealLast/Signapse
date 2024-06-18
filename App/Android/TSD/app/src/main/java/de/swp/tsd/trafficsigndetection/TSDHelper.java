/*
 * Main code work or ensemble by Florian Köhler with special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.content.Context;
import android.content.SharedPreferences;
import android.support.annotation.NonNull;
import android.util.TypedValue;


/**
 * This class holds some helper functions and cannot be instantiated.
 */
public final class TSDHelper
{
    /**
     * This function converts a given dimension measure from dp into px using the display metrics
     * of the device.
     * @param dp Dimension measure in dp.
     * @param context Context used to get display metrics.
     * @return Dimension measure in px.
     */
    public static int dpToPx(int dp, Context context)
    {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp,
                context.getResources().getDisplayMetrics());
    }

    /**
     * This function compares two values for equality which can be null. If both are null, they are
     * also equal.
     * @param a First value
     * @param b Second value
     * @param <T> Type of teh values
     * @return Whether the values are equal or not.
     */
    public static <T> boolean equalNullable(T a, T b)
    {
        boolean match = a == null || b == null;
        if (!match)
        {
            match = a.equals(b);
        }
        else
        {
            match = a == null && b == null;
        }

        return match;
    }


    /**
     * This function writes the given boolean value at the given key to the shared preferences. This
     * way the boolean is saved for access even after restarts of the app.
     * @param keyStringId Resource id of the key string.
     * @param value Value to write.
     * @param context Context for shared preferences.
     */
    public static void writeBooleanToSharedPreferences(int keyStringId, boolean value,
                                                       @NonNull Context context)
    {
        // Get shared preferences editor.
        SharedPreferences.Editor prefs = context.getSharedPreferences(
                context.getString(R.string.data_key), Context.MODE_PRIVATE).edit();

        // Set the value and apply.
        prefs.putBoolean(context.getString(keyStringId), value);
        prefs.apply();
    }


    /**
     * This function reads the boolean value from the shared preferences at the given key.
     * @param keyStringId Resource id of the key string.
     * @param defaultValue Value to return if the key was not found.
     * @param context Context for shared preferences.
     * @return The boolean value if it was found or the default value.
     */
    public static boolean readBooleanFromSharedPreferences(int keyStringId, boolean defaultValue,
                                                           @NonNull Context context)
    {
        // Get shared preferences.
        SharedPreferences prefs = context.getSharedPreferences(
                context.getString(R.string.data_key), Context.MODE_PRIVATE);

        // Query for and return the boolean.
        return prefs.getBoolean(context.getString(keyStringId), defaultValue);
    }


    /**
     * This function writes the given integer value at the given key to the shared preferences. This
     * way the integer is saved for access even after restarts of the app.
     * @param keyStringId Resource id of the key string.
     * @param value Value to write.
     * @param context Context for shared preferences.
     */
    public static void writeIntegerToSharedPreferences(int keyStringId, int value,
                                                       @NonNull Context context)
    {
        // Get shared preferences editor.
        SharedPreferences.Editor prefs = context.getSharedPreferences(
                context.getString(R.string.data_key), Context.MODE_PRIVATE).edit();

        // Set the value and apply.
        prefs.putInt(context.getString(keyStringId), value);
        prefs.apply();
    }


    /**
     * This function reads the integer value from the shared preferences at the given key.
     * @param keyStringId Resource id of the key string.
     * @param defaultValue Value to return if the key was not found.
     * @param context Context for shared preferences.
     * @return The boolean value if it was found or the default value.
     */
    public static int readIntegerFromSharedPreferences(int keyStringId, int defaultValue,
                                                           @NonNull Context context)
    {
        // Get shared preferences.
        SharedPreferences prefs = context.getSharedPreferences(
                context.getString(R.string.data_key), Context.MODE_PRIVATE);

        // Query for and return the boolean.
        return prefs.getInt(context.getString(keyStringId), defaultValue);
    }
}
