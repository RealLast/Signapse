/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.arch.lifecycle.ViewModel;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.support.v7.content.res.AppCompatResources;
import android.util.SparseArray;


/**
 * This class acts as a collection for loaded drawables. When loading a drawable via an instance
 * of this class, then if already loaded at some point it will be returned back without further
 * loading, otherwise it will be loaded and stored for later access. The identification is done by
 * the given resource id of the drawable. If the drawable is not already in here, the it will get
 * loaded from the resources using the given id.
 */
public class TSDDrawableDatabase extends ViewModel
{
    private final SparseArray<TSDDrawable> mDrawableMap = new SparseArray<>();


    /**
     * This method queries for a drawable by its resource id. If the drawable was already loaded,
     * then this instance will get returned, otherwise it will be loaded from the resources and
     * stored for later usage. If the id is null, then null will be returned. This can be used in
     * combination with application side created drawables and loading logics which work after
     * having called this method.
     * @param drawableId Resource id of the drawable or null, if the wanted drawable is not in
     *                   resources.
     * @param context Context to load from the resources.
     * @return Drawable if it could be loaded or if it was already in here, otherwise null.
     */
    public TSDDrawable getDrawable(Integer drawableId, Context context)
    {
        // Get the sparse array.
        if (drawableId != null)
        {
            // Try to get the sign from the array.
            TSDDrawable sign = mDrawableMap.get(drawableId, null);

            if (sign != null)
            {
                // If the sign was already in there, then return it.
                return sign;
            }
            else
            {
                // Otherwise try to load the drawable from the resources.
                Drawable drawable = AppCompatResources.getDrawable(context, drawableId);

                if (drawable != null)
                {
                    sign = new TSDDrawable(drawableId, drawable);
                    mDrawableMap.append(drawableId, sign);
                }

                return sign;
            }
        }

        return null;
    }
}
