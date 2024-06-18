/*
 * Main code work by Florian Köhler
 * First core implementation by Jakob Hampel
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.animation.ObjectAnimator;
import android.app.Activity;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.HorizontalScrollView;
import android.widget.TextView;

import java.util.Timer;
import java.util.TimerTask;


/**
 * This {@link TSDFragment} defines an interactive manual to the user to setup his device physically
 * correct. This includes hints for a power source, for the mounting and for the rotation using the
 * device its gravity sensor. This three parts are separated into three different steps displayed
 * in cards the user can scroll though. When the device is rotated correctly, then a green button
 * is displayed with which the user pressing it can continue to the
 * {@link TSDDetectorFragment detector screen}.
 */
public class TSDSetupFragment extends TSDFragment
{
    private final static int ROLL_THRESHOLD = 5;
    private final static int PITCH_THRESHOLD = 5;
    private final static int START_SCROLL_POSITION = 500;
    private final static int SCROLL_ANIMATION_TIME = 2000;

    private Activity mActivity;
    private TSDRotationSensor mRotationSensor;


    /**
     * This constructor creates a new instance of TSDSetupFragment setting the correct values
     * for icon and isRelevant name.
     */
    public TSDSetupFragment()
    {
        super(R.string.setup_display_name, R.drawable.ic_setup);
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
     * inflates the fragment_setup layout as user interface for the {@link TSDFragment}.
     * It initializes the {@link TSDRotationSensor} to get updates about the current device
     * rotation. On every update the rotation gets checked and the user gets displayed the
     * correct instructions how to rotate the device to get it right. When one axis is rotated
     * correctly, its degrees text says so and gets green. If all axis are rotated correctly,
     * then the hint text itself also gets green and the button to continue gets displayed.
     * @see TSDSetupFragment
     * @param inflater Inflater to use for inflating the user interface layout of this fragment.
     * @param container The place the ui should be inflated to, but not attached.
     * @param savedInstanceState Contains key-value pairs that might have been saved at the last
     *                            destroy of the fragment. Currently this does not get used.
     * @return The inflated view containing the user interface of this {@link TSDFragment}.
     */
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState)
    {
        View self = inflater.inflate(R.layout.fragment_setup, container, false);

        // Get important views.
        TextView rollTextView = self.findViewById(R.id.rollTextView);
        TextView pitchTextView = self.findViewById(R.id.pitchTextView);
        TextView hintTextView = self.findViewById(R.id.hintTextView);
        View continueView = self.findViewById(R.id.continueButton);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }

        if (mActivity != null)
        {
            // Display the progress labels for each card.
            ((TextView)self.findViewById(R.id.progress1TextView))
                    .setText(getString(R.string.progress, 1, 6));
            ((TextView)self.findViewById(R.id.progress2TextView))
                    .setText(getString(R.string.progress, 2, 6));
            ((TextView)self.findViewById(R.id.progress3TextView))
                    .setText(getString(R.string.progress, 3, 6));
            ((TextView)self.findViewById(R.id.progress4TextView))
                    .setText(getString(R.string.progress, 4, 6));
            ((TextView)self.findViewById(R.id.progress5TextView))
                    .setText(getString(R.string.progress, 5, 6));
            ((TextView)self.findViewById(R.id.progress6TextView))
                    .setText(getString(R.string.progress, 6, 6));

            // Create rotation sensor.
            if (mRotationSensor != null)
            {
                mRotationSensor.disable();
                mRotationSensor = null;
            }
            mRotationSensor = new TSDRotationSensor((int roll, int pitch) ->
            {
                rollTextView.setText(getString(R.string.rotation, roll));
                pitchTextView.setText(getString(R.string.rotation, pitch));

                // Set text colors to black again.
                rollTextView.setTextColor(getResources()
                        .getColor(R.color.onBackground, null));
                pitchTextView.setTextColor(getResources()
                        .getColor(R.color.onBackground, null));
                hintTextView.setTextColor(getResources()
                        .getColor(R.color.onBackground, null));

                // Hide the continue button again.
                continueView.setVisibility(View.INVISIBLE);

                // Check for correct roll.
                if (roll < -ROLL_THRESHOLD)
                {
                    hintTextView.setText(getString(R.string.current_roll_too_left));
                }
                else if (roll > ROLL_THRESHOLD)
                {
                    hintTextView.setText(getString(R.string.current_roll_too_right));
                }
                else
                {
                    // Set roll text view to green because it is correct.
                    rollTextView.setTextColor(getResources()
                            .getColor(R.color.primaryDark, null));

                    // Check for correct pitch.
                    if (pitch < -PITCH_THRESHOLD)
                    {
                        hintTextView.setText(getString(R.string.current_pitch_too_front));
                    }
                    else if (pitch > PITCH_THRESHOLD)
                    {
                        hintTextView.setText(getString(R.string.current_pitch_too_back));
                    }
                    else
                    {
                        // Set pitch text view to green because it is correct.
                        pitchTextView.setTextColor(getResources()
                                .getColor(R.color.primaryDark, null));

                        // Display text for correct rotation and set text color to green.
                        hintTextView.setText(getString(R.string.current_rotation_good));
                        hintTextView.setTextColor(getResources()
                                .getColor(R.color.primaryDark, null));

                        // Show the continue button.
                        continueView.setVisibility(View.VISIBLE);
                    }
                }

            }, getLifecycle(), mActivity);
        }

        return self;
    }


    @Override
    public void onStart()
    {
        super.onStart();

        // Enable rotation sensor if if was created.
        if (mRotationSensor != null)
        {
            mRotationSensor.enable();
        }
    }
}