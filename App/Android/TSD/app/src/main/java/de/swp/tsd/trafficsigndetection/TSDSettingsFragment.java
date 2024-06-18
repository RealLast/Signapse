/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.HorizontalScrollView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

/**
 * This {@link TSDFragment} enables the user to persistently set some settings to the app.
 * Those settings are:
 * Whether or not to use the speed tracking and the connected speed warning with a Switch.
 * The threshold for the speed warning with a seek bar.
 * The maximum number of sign combinations to hold in the sign history.
 */
public class TSDSettingsFragment extends TSDFragment
{
    private final static int HISTORY_MIN_SIZE = 10;
    private final static int HISTORY_DEFAULT_MAX_SIZE = 25;
    private final static int HISTORY_MAX_SIZE = 50;

    private final static int THRESHOLD_MIN_SIZE = -5;
    private final static int THRESHOLD_DEFAULT_MAX_SIZE = 4;
    private final static int THRESHOLD_MAX_SIZE = 20;

    private Activity mActivity;


    /**
     * This constructor creates a new instance of {@link TSDSettingsFragment} setting the correct
     * values for {@link TSDFragment#displayNameId display name} and
     * {@link TSDFragment#displayIconId display icon}.
     */
    public TSDSettingsFragment()
    {
        super(R.string.settings_display_name, R.drawable.ic_settings);
    }


    /**
     * This method gets called when the fragment is created. It initializes the fragment.
     * @param savedInstanceState Contains key-value pairs that might have been saved at the last
     *                            destroy of the fragment. Currently this does not get used.
     */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }
    }


    /**
     * This method gets called every time the view of the {@link TSDFragment} is newly created. It
     * inflates the fragment_settings layout as user interface for the {@link TSDFragment}.
     * It initializes the ranges for both seek bars (threshold and max number), sets the currently
     * in user preferences saved settings to the views or default values and creates the
     * listeners to updates the user preferences when the user changes some settings.
     * @see TSDSettingsFragment
     * @param inflater Inflater to use for inflating the user interface layout of this fragment.
     * @param container The place the ui should be inflated to, but not attached.
     * @param savedInstanceState Contains key-value pairs that might have been saved at the last
     *                            destroy of the fragment. Currently this does not get used.
     * @return The inflated view containing the user interface of this {@link TSDFragment}.
     */
    @SuppressLint("ClickableViewAccessibility")
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState)
    {
        View self = inflater.inflate(R.layout.fragment_settings, container, false);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }

        if (mActivity != null)
        {
            HorizontalScrollView scrollView = self.findViewById(R.id.settingsScrollView);

            // Get gps switch and set the saved state.
            Switch gpsSwitch = self.findViewById(R.id.gpsSwitch);
            boolean gpsChecked = TSDHelper.readBooleanFromSharedPreferences(R.string.data_gps_key,
                    true, mActivity);
            gpsSwitch.setChecked(gpsChecked);

            // Set listener for changing state
            gpsSwitch.setOnCheckedChangeListener((CompoundButton buttonView, boolean isChecked) ->
            {
                // Write the checked value to the preferences.
                TSDHelper.writeBooleanToSharedPreferences(R.string.data_gps_key, isChecked,
                        mActivity);
            });


            // Get threshold seek bar and text view and get currently set max size.
            SeekBar thresholdBar = self.findViewById(R.id.thresholdSeekBar);
            TextView thresholdTextView = self.findViewById(R.id.thresholdValueTextView);
            TextView thresholdHintView = self.findViewById(R.id.thresholdHintTextView);
            int threshold = TSDHelper.readIntegerFromSharedPreferences(
                    R.string.data_warning_threshold, THRESHOLD_DEFAULT_MAX_SIZE, mActivity);

            // Set the range and value of the bar and show value in text view.
            thresholdBar.setMax(THRESHOLD_MAX_SIZE - THRESHOLD_MIN_SIZE);
            thresholdBar.setProgress(threshold - THRESHOLD_MIN_SIZE);
            thresholdTextView.setText(getString(R.string.current_speed, threshold + ""));
            thresholdHintView.setText(threshold < 0 ?
                getString(R.string.warning_threshold_negative_hint, Math.abs(threshold))
                : getString(R.string.warning_threshold_positive_hint, threshold));

            // Set listener for changing max size.
            thresholdBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener()
            {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
                {
                    final int threshold = progress + THRESHOLD_MIN_SIZE;

                    // Write the chosen size to preferences.
                    TSDHelper.writeIntegerToSharedPreferences(R.string.data_warning_threshold,
                            threshold, mActivity);

                    // Display the chosen size to text view.
                    thresholdTextView
                            .setText(getString(R.string.current_speed, threshold + ""));

                    thresholdHintView.setText(threshold < 0 ?
                            getString(R.string.warning_threshold_negative_hint, Math.abs(threshold))
                            : getString(R.string.warning_threshold_positive_hint, threshold));
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) { }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) { }
            });

            // https://stackoverflow.com/a/30306011/5529479
            // Set custom touch handling to fix problem with scroll view and thumb dragging
            thresholdBar.setOnTouchListener((View v, MotionEvent event) ->
            {
                switch (event.getAction())
                {
                    case MotionEvent.ACTION_DOWN:
                    {
                        scrollView.requestDisallowInterceptTouchEvent(true);
                        break;
                    }

                    case MotionEvent.ACTION_UP:
                    {
                        v.performClick();
                        scrollView.requestDisallowInterceptTouchEvent(false);
                        break;
                    }
                }

                v.onTouchEvent(event);
                return true;
            } );


            // Get history count seek bar and text view and get currently set max size.
            SeekBar historyBar = self.findViewById(R.id.historySeekBar);
            TextView maxSizeTextView = self.findViewById(R.id.historyMaxSizeTextView);
            int maxSize = TSDHelper.readIntegerFromSharedPreferences(
                    R.string.data_history_max_size_key, HISTORY_DEFAULT_MAX_SIZE, mActivity);

            // Set the range and value of the bar and show value in text view.
            historyBar.setMax(HISTORY_MAX_SIZE - HISTORY_MIN_SIZE);
            historyBar.setProgress(maxSize - HISTORY_MIN_SIZE);
            String sizeText = maxSize + "";
            maxSizeTextView.setText(sizeText);

            // Set listener for changing max size.
            historyBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener()
            {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
                {
                    final int maxSize = progress + HISTORY_MIN_SIZE;

                    // Write the chosen size to preferences.
                    TSDHelper.writeIntegerToSharedPreferences(R.string.data_history_max_size_key,
                            maxSize, mActivity);

                    // Display the chosen size to text view.
                    String sizeText = maxSize + "";
                    maxSizeTextView.setText(sizeText);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) { }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) { }
            });

            // Set custom touch handling to fix problem with scroll view and thumb dragging
            historyBar.setOnTouchListener((View v, MotionEvent event) ->
            {
                switch (event.getAction())
                {
                    case MotionEvent.ACTION_DOWN:
                    {
                        scrollView.requestDisallowInterceptTouchEvent(true);
                        break;
                    }

                    case MotionEvent.ACTION_UP:
                    {
                        v.performClick();
                        scrollView.requestDisallowInterceptTouchEvent(false);
                        break;
                    }
                }

                v.onTouchEvent(event);
                return true;
            } );
        }

        return self;
    }
}
