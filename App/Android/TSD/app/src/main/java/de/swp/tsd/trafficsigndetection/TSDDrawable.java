/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;


/**
 * This class wraps a drawable loaded from resources or from somewhere else and additional
 * information like the resource id, the image width and the image height. Since the actual width
 * and height of the image can be overridden in one constructor, those information should only be
 * used for scaling, not for direct access. To get the actual dimensions, they need to be directly
 * queried from the drawable instance.
 */
public class TSDDrawable
{
    private final Integer mDrawableId;
    private final Drawable mDrawable;
    private final int mWidth;
    private final int mHeight;


    /**
     * This constructor creates a new instance of TSDDrawable querying for the actual width and
     * height of the image.
     * @param drawableId Actual resource id of the drawable. If the drawable was not loaded from
     *                   resources, then this should be null.
     * @param drawable Drawable instance also the width and height will be taken from.
     */
    TSDDrawable(Integer drawableId, @NonNull Drawable drawable)
    {
        mDrawableId = drawableId;
        mDrawable = drawable;

        // Get original size of the drawable resource.
        mWidth = drawable.getIntrinsicWidth();
        mHeight = drawable.getIntrinsicHeight();
    }


    /**
     * This constructor creates a new instance of TSDDrawable overriding the actual width and height
     * of the image. This can be used when the image should be scaled like if it had different
     * dimensions.
     * @param drawableId Actual resource id of the drawable. If the drawable was not loaded from
     *                   resources, then this should be null.
     * @param drawable Drawable loaded from resources.
     * @param width Width to save as this image width.
     * @param height Height to save as this image height.
     */
    TSDDrawable(Integer drawableId, Drawable drawable, int width, int height)
    {
        mDrawableId = drawableId;
        mDrawable = drawable;
        mWidth = width;
        mHeight = height;
    }


    /**
     * Getter for the drawable resource id. If the drawable was not loaded from resources, then
     * this will be null.
     * @return Drawable resource id or null, if the drawable was not loaded from resources.
     */
    public Integer getDrawableId()
    {
        return mDrawableId;
    }


    /**
     * Getter for the drawable.
     * @return Drawable instance
     */
    public @NonNull Drawable getDrawable()
    {
        return mDrawable;
    }


    /**
     * Getter for the width of the drawable. Please mind, that this value could be overridden in the
     * constructor and then not match the actual width of the image. Only use this for scaling.
     * @return Width of the drawable
     */
    public int getWidth()
    {
        return mWidth;
    }


    /**
     * Getter for the height of the drawable. Please mind, that this value could be overridden in
     * the constructor and then not match the actual height of the image. Only use this for scaling.
     * @return Height of the drawable
     */
    public int getHeight()
    {
        return mHeight;
    }


    /**
     * This method can be used to compare two {@link TSDDrawable} instances. Two {@link TSDDrawable}
     * instances match, if the following information are equal: the id, the given width and given
     * height and the actual width and height of the drawable.
     * @param obj Other {@link TSDDrawable} to compare to.
     * @return The result of the comparison or false, if the given object to compare to was not
     * of the same class as this instance.
     */
    @Override
    public boolean equals(Object obj)
    {
        if (getClass() == obj.getClass())
        {
            TSDDrawable dr = (TSDDrawable)obj;

            // The resource id has to match.
            return TSDHelper.equalNullable(mDrawableId, dr.mDrawableId)
                    && mWidth == dr.mWidth
                    && mHeight == dr.mHeight
                    && mDrawable.getIntrinsicWidth() == dr.mDrawable.getIntrinsicWidth()
                    && mDrawable.getIntrinsicHeight() == dr.mDrawable.getIntrinsicHeight();
        }

        return false;
    }
}
