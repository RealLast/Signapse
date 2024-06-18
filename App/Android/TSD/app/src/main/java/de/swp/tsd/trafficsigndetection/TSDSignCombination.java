/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.support.annotation.NonNull;

import java.util.Arrays;
import java.util.Objects;


/**
 * This class holds all information about the combination of multiple or one traffic
 * signs and additionally the time and speed. This includes one id which should be unique for one
 * pole of traffic signs in real life. For the definition of the signs there an array of
 * {@link TSDSign sign enums} describing their classes and an array of
 * {@link TSDDrawable sign drawables} describing their images which can be displayed to the user.
 * For time isRelevant just the timestamp in milliseconds when the combination was recognized is saved.
 */
public class TSDSignCombination
{
    private final long mId;
    private final long mFrameId;
    private final TSDSign[] mSigns;
    private final TSDDrawable[] mSignDrawables;
    private final long mTimestamp;
    private final int mDistanceToImageCenter;
    private Float mScaleFactor = null;
    private Integer mSpeed;


    /**
     * This constructor creates a new instance of TSDSignCombination loading the drawables by their
     * given ids from the database. If the database cannot deliver a drawable, then the defined
     * cutout will be taken as drawable from the given image for this image. This way it is ensured
     * that every sign, that is returned by the RoadSignAPI, can also be displayed to the user.
     * @param id Id of this sign combination. This can be used for replacing wrong recognition.
     * @param signs Enum for each sign drawables in order top to bottom. Each sign enum must
     *              correspond to the image cutout rect in rects at the same index.
     * @param rects Image cutout rect for each sign in order top to bottom. Each rect must
     *              correspond to the enum in signs at the same index.
     * @param frameId Id describing the number of the frame in which this sign combination was
     *                captured. The frame id should be continuously increased by one for each frame
     *                that is analysed.
     * @param image Image the rects define cutouts in.
     * @param timestamp Time in milliseconds when the signs were recognized.
     * @param speed Current speed or speed when the sign was recognized.
     * @param database Database for drawables for more efficient loading.
     * @param context Context for loading from resources.
     */
    TSDSignCombination(long id, long frameId, @NonNull TSDSign[] signs, @NonNull Rect[] rects,
                       Bitmap image, long timestamp, Integer speed,
                       @NonNull TSDDrawableDatabase database,
                       @NonNull Context context)
    {
        mId = id;
        mFrameId = frameId;
        mTimestamp = timestamp;
        mSpeed = speed;
        mSigns = signs;

        // Try to find the index of an sign that has an drawable in the database.
        Integer referenceRectWidth = null;
        Integer referenceImageWidth = null;

        // Get the sum of the horizontal centers of all given rects.
        int centerSum = 0;

        // Load the signs from database or cut it out of the image.
        mSignDrawables = new TSDDrawable[signs.length];
        for(int i = 0; i < mSignDrawables.length; i++)
        {
            // Try to load the image from the database.
            TSDDrawable drawable = signs[i].getDrawable(database, context);

            // Get the first sign with drawable from resources and save its rect and drawable width.
            if (drawable != null && referenceRectWidth == null)
            {
                referenceRectWidth = rects[i].width();
                referenceImageWidth = drawable.getWidth();
            }

            // If the image is not in database, then try to cut it out from the given image.
            if (drawable == null && image != null)
            {
                // Cutout the sign from the given image using the given rect.
                Bitmap cutout = Bitmap.createBitmap(image, rects[i].left, rects[i].top,
                        rects[i].width(), rects[i].height());

                // Create drawable from cutout.
                drawable = new TSDDrawable(null,
                        new BitmapDrawable(context.getResources(), cutout));
            }

            // Save the drawable to the array.
            mSignDrawables[i] = drawable;

            // Get the horizontal center of the rect in the image and add it unto the sum.
            centerSum += rects[i].centerX();
        }

        // Calculate the average horizontal center of all signs in this combination within the
        // image.
        final int averageCenter = Math.round(centerSum / (float)mSignDrawables.length);

        // Calculate the average distance of this sign combination to the center of the image.
        // If the given image is null then set 0 as distance to the center.
        mDistanceToImageCenter = image != null ?
                Math.abs(averageCenter - (image.getWidth() / 2)) : 0;

        // Correct the size of the cutout drawables to match the size of the resource drawables
        // if the combination contains a reference. Otherwise it does not matter since all
        // cutouts have the correct proportions to one another.
        if (referenceImageWidth != null)
        {
            // Calculate the factor the rect dimensions have to be multiplied with to get
            // the correct sizes.
            mScaleFactor = referenceImageWidth / (float)referenceRectWidth;
            scaleDrawables();
        }
    }


    /**
     * This constructor creates a new instance of TSDSignCombination filling in all field with
     * the given information.
     * @param id See {@link TSDSignCombination#mId}
     * @param frameId See {@link TSDSignCombination#mFrameId}
     * @param timestamp See {@link TSDSignCombination#mTimestamp}
     * @param speed See {@link TSDSignCombination#mSpeed}
     * @param signs See {@link TSDSignCombination#mSigns}
     * @param signDrawables See {@link TSDSignCombination#mSignDrawables}
     * @param distanceToImageCenter See {@link TSDSignCombination#mDistanceToImageCenter}
     * @param scaleFactor See {@link TSDSignCombination#mScaleFactor}
     */
    private TSDSignCombination(long id, long frameId, long timestamp, Integer speed,
                               TSDSign[] signs, TSDDrawable[] signDrawables,
                               int distanceToImageCenter, Float scaleFactor)
    {
        mId = id;
        mFrameId = frameId;
        mTimestamp = timestamp;
        mSpeed = speed;
        mSigns = signs;
        mSignDrawables = signDrawables;
        mDistanceToImageCenter = distanceToImageCenter;
        mScaleFactor = scaleFactor;
    }


    /**
     * Getter for the id of the frame this sign combination was captured in. The frame ids are
     * increased by one for every analysed frame. So frame id 4 means one frame later than frame
     * id 3.
     * @return Id of the frame the image was captured in.
     */
    public long getFrameId()
    {
        return mFrameId;
    }


    /**
     * Getter for the id of the sign combination. The same id does not mean that the content is
     * the same, too. One id describes one sign combination in real life. If the recognition was
     * wrong and some signs gets updated, then the id is still the same.
     * @return Id of this sign combination instance.
     */
    public long getId()
    {
        return mId;
    }


    /**
     * Getter for the average horizontal distance of this sign combination from the image center
     * they were detected in. The distance is calculated between the horizontal center of the image
     * and the average horizontal centers of the given sign combination rects.
     * @return Average horizontal distance of this sign combination from the image center. If the at
     *         constructions given image was null, then this will return 0;
     */
    public int getDistanceToImageCenter()
    {
        return mDistanceToImageCenter;
    }


    /**
     * Getter for the contained sign drawables.
     * @return Array of drawables which are the signs.
     */
    public TSDDrawable[] getSignDrawables()
    {
        return mSignDrawables;
    }


    /**
     * Getter for the contained sign enums.
     * @return Array of enums which define the sign classes.
     */
    public TSDSign[] getSigns()
    {
        return mSigns;
    }


    /**
     * Getter for the timestamp the signs were recognized.
     * @return Timestamp the signs were recognized.
     */
    public long getTimestamp()
    {
        return mTimestamp;
    }


    /**
     * Setter for the current speed or the speed when the sign was recognized.
     * This can also be null.
     */
    public void setSpeed(Integer speed)
    {
        mSpeed = speed;
    }


    /**
     * Getter for the current speed or the speed when the sign was recognized.
     * @return Speed
     */
    public Integer getSpeed()
    {
        return mSpeed;
    }


    /**
     * This method checks, whether this combination contains at least one sign, that is standalone
     * according to {@link TSDSign#isStandalone()}. Then the combination is standalone. If a sign
     * combination is not standalone, then it will not be displayed to the user.
     * @return Whether this sign combination is standalone or not.
     */
    public boolean isStandalone()
    {
        // Check each sign in the combination whether it is a standalone or not. If no standalone
        // is contained, then the combination is not standalone.
        boolean hasStandalone = false;
        for (TSDSign sign : mSigns)
        {
            hasStandalone = hasStandalone || sign.isStandalone();
        }

        return hasStandalone;
    }


    /**
     * This method tests whether the signs of this combination are all contained in a given
     * sign combination in this order. The given sign combination can contain more signs, it just
     * needs to contain the signs in the correct order with others in between allowed. This only
     * tests for sign types, not drawables.
     * @param c Sign combination to test, whether it contains the signs in this order.
     * @return Whether the given sign combination contains the signs of this combination in this
     *         order.
     */
    public boolean isSublistOf(TSDSignCombination c)
    {
        // Check if the signs are contained in the other list in this exact order.
        int found = 0;
        int j = 0;
        for (TSDSign sign : mSigns)
        {
            while (j < c.mSigns.length)
            {
                if (sign.equals(c.mSigns[j++]))
                {
                    found++;
                    break;
                }
            }
        }

        // Return whether all signs are contained inside the other combination.
        return mSigns.length == found;
    }


    /**
     * This method will create a new instance of TSDSignCombination, if the given combi is a sublist
     * of this instance. The signs will be copied over from this instance while getting updated
     * by the same signs in the new combination. All other information will get copied over from
     * the new instance.
     * @param newCombi The sign combination which information should be a update.
     * @return When the given combi was a sublist, a new updated combi will be returned. Otherwise
     *         null will be returned.
     */
    public TSDSignCombination createUpdatedBySublist(TSDSignCombination newCombi)
    {
        // If the given combination is a sublist, then use it to update the own information.
        if (newCombi.isSublistOf(this))
        {
            Float scaleFactor = newCombi.mScaleFactor != null ? newCombi.mScaleFactor
                    : mScaleFactor;
            TSDSign[] signs = mSigns.clone();
            TSDDrawable[] signDrawables = mSignDrawables.clone();

            // Scale the new combi drawables to the consistent factor if they were not already
            // scaled.
            if (newCombi.mScaleFactor == null)
            {
                newCombi.mScaleFactor = scaleFactor;
                newCombi.scaleDrawables();
            }

            // Scale this combi drawables to the consistent factor if they were not already scaled.
            if (mScaleFactor == null)
            {
                mScaleFactor = scaleFactor;
                scaleDrawables();
            }

            // Update the sign combi drawables with the later drawables.
            int o = 0;
            for (int n = 0; n < newCombi.mSigns.length; n++)
            {
                while (o < signs.length)
                {
                    if (signs[o].equals(newCombi.mSigns[n]))
                    {
                        boolean furtherMatch = true;
                        int i = o + 1;
                        int j = n + 1;

                        // Check that as long as in the old list continues to be the same sign class
                        // as the currently selected, that there are also exactly the same number
                        // of signs of the same class in the new list after the selected sign.
                        while (i < signs.length && j < newCombi.mSigns.length
                                && signs[o].equals(signs[i]))
                        {
                            if (!signs[i].equals(newCombi.mSigns[j]))
                            {
                                furtherMatch = false;
                                break;
                            }

                            i++;
                            j++;
                        }

                        // Only update the sign drawable, if this is a further match. Otherwise
                        // the problem could arise, that there are for example two signs of the
                        // same class in the old list one after the other, but only the second one
                        // in the new list. If the further match test would not happen, then the
                        // wrong sign would be replaced by the new drawable, which could result
                        // in an wrong sign pole. So in this case prefer to keep the old sign than
                        // to give out a wrong result.
                        if (furtherMatch)
                        {
                            signDrawables[o++] = newCombi.mSignDrawables[n];
                        }

                        break;
                    }
                    o++;
                }
            }

            // Return a new instance with updated information.
            return new TSDSignCombination(newCombi.mId, newCombi.mFrameId, newCombi.mTimestamp,
                    newCombi.mSpeed, signs, signDrawables, newCombi.mDistanceToImageCenter,
                    scaleFactor);
        }

        return null;
    }


    /**
     * This method can be used to compare {@link TSDSignCombination} instances. Those two instances
     * match, if the following information are equal: the timestamp, the speed, the array of sign
     * drawables and the frame id.
     * @param obj Other {@link TSDSignCombination} to compare to.
     * @return The result of the comparison or false, if the given object to compare to was not
     * of the same class as this instance.
     */
    @Override
    public boolean equals(Object obj)
    {
        if (getClass() == obj.getClass())
        {
            TSDSignCombination sc = (TSDSignCombination)obj;

            // Every field has to be the equal.
            return mTimestamp == sc.mTimestamp
                    && mFrameId == sc.mFrameId
                    && TSDHelper.equalNullable(mSpeed, sc.mSpeed)
                    && Arrays.equals(mSignDrawables, sc.mSignDrawables);
        }

        return false;
    }


    /**
     * This method rescales the dimensions of each cutout sign drawable by the
     * {@link TSDSignCombination#mScaleFactor} if it is not null.
     */
    private void scaleDrawables()
    {
        if (mScaleFactor != null)
        {
            for (int i = 0; i < mSignDrawables.length; i++)
            {
                TSDDrawable drawable = mSignDrawables[i];

                // Only correct the signs with cutout drawables.
                if (drawable.getDrawableId() == null)
                {
                    final int newWidth =
                            Math.round(drawable.getDrawable().getIntrinsicWidth() * mScaleFactor);
                    final int newHeight =
                            Math.round(drawable.getDrawable().getIntrinsicHeight() * mScaleFactor);

                    // Overwrite the drawable in the array with new dimensions.
                    mSignDrawables[i] = new TSDDrawable(null, drawable.getDrawable(),
                            newWidth, newHeight);
                }
            }
        }
    }
}
