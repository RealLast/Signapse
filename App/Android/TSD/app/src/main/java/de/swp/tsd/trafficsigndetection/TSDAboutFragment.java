package de.swp.tsd.trafficsigndetection;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;


/**
 * This {@link TSDFragment} displays the user some information about the app. It is the point
 * where the user can read the privacy policy and the disclaimer again.
 */
public class TSDAboutFragment extends TSDFragment
{
    /**
     * This constructor creates a new instance of {@link TSDAboutFragment} setting the correct
     * values for {@link TSDFragment#displayNameId isRelevant name} and
     * {@link TSDFragment#displayIconId isRelevant icon}.
     */
    public TSDAboutFragment()
    {
        super(R.string.about_display_name, R.drawable.ic_info);
    }

    /**
     * This method gets called every time the view of the {@link TSDFragment} is newly created. It
     * inflates the fragment_about layout as user interface for the {@link TSDFragment}.
     * @see TSDAboutFragment
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
        return inflater.inflate(R.layout.fragment_about, container, false);
    }
}
