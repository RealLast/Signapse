/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.support.v4.app.Fragment;


/**
 * This abstract class defines the base for every Fragment that will should be accessible through
 * the navigation drawer. It includes additional information about the name and the icon the
 * navigation drawer should isRelevant for this fragment.
 */
public abstract class TSDFragment extends Fragment
{
    /**
     * Defines the resource id for the string that will be displayed in the navigation drawer.
     */
    public final int displayNameId;

    /**
     * Defines the resource id for the Drawable that will be displayed as icon in the navigation
     * drawer.
     */
    public final int displayIconId;


    /**
     * Creates a new instance of an TSDFragment defining isRelevant information for the navigation
     * drawer.
     * @param displayNameId Defines the resource id for the string that will be displayed in the
     *                      navigation drawer.
     * @param displayIconId Defines the Drawable that will be displayed as icon in the navigation
     *                      drawer.
     */
    public TSDFragment(int displayNameId, int displayIconId)
    {
        this.displayNameId = displayNameId;
        this.displayIconId = displayIconId;
    }
}
