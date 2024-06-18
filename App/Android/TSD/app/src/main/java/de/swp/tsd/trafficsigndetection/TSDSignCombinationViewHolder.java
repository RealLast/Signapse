/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.Arrays;


/**
 * This class defines an recycler view holder, that can be used to display a
 * {@link TSDSignCombination} to a sign card defined by the sign_card.xml layout.
 * In this layout the signs are displayed one below the one before like a pole of signs would look
 * in real life. Furthermore the timespan since this combination was recognized is displayed in
 * steps (0s, 1s and then every 5s, then every minute, then every hour and at last every day).
 * On the sign card also speed can be displayed. For the display of the speed warning it is possible
 * to set the alpha value of a red border around the card. For differentiation between the latest
 * sign combination and the history it is possible to set the alpha value of the Signs.
 */
public class TSDSignCombinationViewHolder extends RecyclerView.ViewHolder
{
    private Context mContext;
    private View mCardView;
    private View mWarningBorderView;
    private TextView mSpeedTextView;
    private ImageView mSpeedImageView;
    private TextView mTimeTextView;
    private LinearLayout mSignLayout;


    /**
     * This constructor creates a new instance of TSDSignCombinationViewHolder, searching for the
     * necessary child views inside the given one. Therefore the given view must be a view inflated
     * from sign_card.xml.
     * @param view Sign card view inflated from sign_card.xml.
     */
    TSDSignCombinationViewHolder(View view)
    {
        super(view);

        mContext = view.getContext();

        // Find necessary views by id.
        mCardView = view.findViewById(R.id.signCardView);
        mWarningBorderView = view.findViewById(R.id.warningBorderView);
        mSpeedTextView = view.findViewById(R.id.speedTextView);
        mSpeedImageView = view.findViewById(R.id.speedImageView);
        mTimeTextView = view.findViewById(R.id.timeTextView);
        mSignLayout = view.findViewById(R.id.signLayout);
    }


    /**
     * This method sets the left and right margin of the sign card view in dp.
     * @param leftDp Value in dp to set for left margin.
     * @param rightDp Value in dp to set for right margin.
     */
    public void setLeftRightMargin(int leftDp, int rightDp)
    {
        ViewGroup.MarginLayoutParams params =
                (ViewGroup.MarginLayoutParams)mCardView.getLayoutParams();

        params.leftMargin = TSDHelper.dpToPx(leftDp, mContext);
        params.rightMargin = TSDHelper.dpToPx(rightDp, mContext);

        mCardView.setLayoutParams(params);
    }


    /**
     * This method displays every given information from the new combi which has changed from the
     * given old combi. The time will always be refreshed, since its displayed as range to the
     * current time.
     * @param oldCombi The sign combi which was displayed here before.
     * @param newCombi The sign combi which should be displayed.
     * @param signTransparency Transparency of the image views showing the signs.
     * @param warningTransparency Transparency of the warning border around the sign.
     */
    public void displayAllChanges(TSDSignCombination oldCombi, TSDSignCombination newCombi,
                                  float signTransparency, float warningTransparency)
    {
        // Display changes in timestamp.
        displayTime(newCombi.getTimestamp());

        // Display changes in speed.
        if (!TSDHelper.equalNullable(oldCombi.getSpeed(), newCombi.getSpeed()))
        {
            displaySpeed(newCombi.getSpeed());
        }

        // Display changes in signs which can also happen by frame updates (updates in image
        // cutouts)
        if (!Arrays.equals(oldCombi.getSignDrawables(), newCombi.getSignDrawables())
                || oldCombi.getFrameId() != newCombi.getFrameId())
        {
            displaySigns(newCombi.getSignDrawables(), signTransparency);
        }

        // Set transparency of warning border.
        setWarningTransparency(warningTransparency);
    }


    /**
     * This method displays every information given in the sign combination to the sign card view
     * given to this instance.
     * @param signs Sign combination to display.
     * @param signTransparency Transparency of the image views showing the signs. This should
     *                            be a value ranging from 0 to 1.
     * @param warningTransparency Transparency of the warning border around the sign. This should
     *                            be a value ranging from 0 to 1.
     */
    public void displayAll(TSDSignCombination signs, float signTransparency,
                           float warningTransparency)
    {
        displaySpeed(signs.getSpeed());
        displayTime(signs.getTimestamp());
        displaySigns(signs.getSignDrawables(), signTransparency);
        setWarningTransparency(warningTransparency);
    }


    /**
     * This method sets the transparency of the warning border of the sign card.
     * @param warningTransparency Transparency of the warning border around the sign. This should
     *                            be a value ranging from 0 to 1.
     */
    public void setWarningTransparency(float warningTransparency)
    {
        final int transparency = (int)(warningTransparency * 255);

        // Only refresh transparency if it has changed.
        if (mWarningBorderView.getAlpha() != transparency)
        {
            mWarningBorderView.setAlpha(transparency);
        }
    }


    /**
     * This method displays some given speed to the connected speed card view.
     * @param speed Speed to display. If this is null, then the speed view will be hidden.
     *              If the value is negative, then the indicator ~ will be displayed.
     */
    public void displaySpeed(Integer speed)
    {
        // Display speed or hide speed views.
        if (speed != null)
        {
            // Set speed on text view.
            mSpeedTextView.setText(speed < 0 ? mContext.getString(R.string.no_speed)
                    : mContext.getString(R.string.current_speed, speed.toString()));

            // Set speed views to visible.
            mSpeedImageView.setVisibility(View.VISIBLE);
            mSpeedTextView.setVisibility(View.VISIBLE);
        }
        else
        {
            // Set speed views to invisible.
            mSpeedImageView.setVisibility(View.INVISIBLE);
            mSpeedTextView.setVisibility(View.INVISIBLE);
        }
    }


    /**
     * This method displays the timespan from now to the given timestamp to the connected sign card
     * view in steps (0s, 1s and then every 5s, then every minute, then every hour and at last every
     * day).
     * @param timestamp Timestamp in milliseconds.
     */
    public void displayTime(long timestamp)
    {
        // Get seconds since recognition.
        final long ts = System.currentTimeMillis();
        final long timespanSecs = (ts - timestamp) / 1000;

        // Set timespan on text view.
        final String time;

        // Get minutes
        final long timespanMins = timespanSecs / 60;
        if (timespanMins >= 1)
        {
            // Get hours
            final long timespanHours = timespanMins / 60;
            if (timespanHours >= 1)
            {
                // Get days
                final long timespanDays = timespanHours / 24;
                if (timespanDays >= 1)
                {
                    time = timespanDays + "d";
                }
                else
                {
                    time = timespanHours + "h";
                }
            }
            else
            {
                time = timespanMins + "m";
            }
        }
        else
        {
            // Display only 0s, 1s and then every 5 seconds
            time = (timespanSecs > 1 ? Math.max(5 * Math.round(timespanSecs / 5f), 1)
                    : timespanSecs) + "s";
        }

        mTimeTextView.setText(mContext.getString(R.string.current_timespan, time));
    }


    /**
     * This method displays the given sign drawables to the connected sign card view in the given
     * order from top to bottom like a pole with traffic signs in real life. This automatically
     * scales everything so it fits best into the available space on the card.
     * @param signs The sign drawables to display.
     * @param transparency The transparency of the image views showing the signs. This should be a
     *                     value ranging from 0 to 1.
     */
    @SuppressWarnings("WeakerAccess")
    public void displaySigns(TSDDrawable[] signs, float transparency)
    {
        // Create array to hold the image views.
        ImageView[] imgViews = new ImageView[signs.length];

        // Remove or add child image views until there are enough tto match the signs length.
        while (mSignLayout.getChildCount() != signs.length)
        {
            if (mSignLayout.getChildCount() < signs.length)
            {
                mSignLayout.addView(new ImageView(mContext));
            }
            else
            {
                mSignLayout.removeViewAt(0);
            }
        }

        // Read the image views in the correct order.
        for (int i = 0; i < imgViews.length; i++)
        {
            imgViews[i] = (ImageView)mSignLayout.getChildAt(i);
            imgViews[i].setAlpha(transparency);
        }

        int combinationHeight = 0;
        int combinationWidth = 0;

        // Attach each given sign drawable to a new image view and add it to the linear layout.
        for (int i = 0; i < signs.length; i++)
        {
            TSDDrawable sign = signs[i];

            // Add the height of the drawable to the sum to get the combination height of the signs.
            combinationHeight += sign.getHeight();

            // Get the biggest width.
            if (sign.getWidth() > combinationWidth)
            {
                combinationWidth = sign.getWidth();
            }

            // Attach the image to the corresponding image view.
            imgViews[i].setImageDrawable(sign.getDrawable());

            // Clamp the dimensions of the image view to the contained drawable. Otherwise the
            // image view would be way too big.
            imgViews[i].setAdjustViewBounds(true);
            imgViews[i].setScaleType(ImageView.ScaleType.FIT_CENTER);
        }

        // Copy as final for correct usage in runnable.
        final float height = combinationHeight;
        final float width = combinationWidth;
        final int margin = TSDHelper.dpToPx(8, mContext);

        // Post the scaling of the image views into the views queue to get executed when everything
        // is set up. Otherwise the queried sizes could not be correct.
        mSignLayout.post(() ->
        {
            final int childCount = mSignLayout.getChildCount();

            // Calculate the factor to multiply with the dimensions of each drawable to get the
            // target pixel dimensions for the corresponding image view using the complete height of
            // the combination. Include the margin for every but the last view.
            float hScale = (mSignLayout.getHeight() - (childCount - 1) * margin) / height;

            // Calculate the factor to multiply with the dimensions of each drawable to get the
            // target pixel dimensions for the corresponding image view using the maximum width of
            // the combination.
            float wScale = mSignLayout.getWidth() / width;

            // Get the smallest scale to not break the borders of the linear layout.
            float scale = Math.min(hScale, wScale);

            // Set the dimensions for each child (image view) of the linear layout.
            for (int i = 0; i < childCount; i++)
            {
                // This can crash for some reason because of an index out of bounds when scrolling
                // too fast. This probably happens, when the posting is executed after some new
                // signs were already set. So this crash should not matter since the dimensions
                // get replaced afterwards anyway.
                try
                {
                    // Get child and its layout parameter.
                    View view = mSignLayout.getChildAt(i);
                    LinearLayout.LayoutParams params =
                            (LinearLayout.LayoutParams) view.getLayoutParams();

                    params.width = (int)(signs[i].getWidth() * scale);
                    params.height = (int)(signs[i].getHeight() * scale);
                    params.bottomMargin = i < childCount - 1 ? margin : 0;
                    params.gravity = Gravity.CENTER;

                    // Set the new layout parameter.
                    view.setLayoutParams(params);
                    view.requestLayout();
                }
                catch (Exception e)
                {
                    Log.i("TSDSignCombinationViewHolder", "Scrolling too fast!");
                }
            }
        });
    }
}
