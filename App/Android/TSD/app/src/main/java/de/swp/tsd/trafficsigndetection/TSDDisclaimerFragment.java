package de.swp.tsd.trafficsigndetection;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

/**
 * This {@link TSDFragment} displays the user the disclaimer of the app. The user will not be able
 * to continue with the application without accepting the disclaimer. It will be shown on every
 * startup of the app.
 */
public class TSDDisclaimerFragment extends TSDFragment
{
    /**
     * This constructor creates a new instance of {@link TSDDisclaimerFragment} setting the correct
     * values for {@link TSDFragment#displayNameId isRelevant name} and
     * {@link TSDFragment#displayIconId isRelevant icon}.
     */
    public TSDDisclaimerFragment()
    {
        super(R.string.disclaimer_display_name, R.drawable.ic_attention);
    }

    /**
     * This method gets called every time the view of the {@link TSDFragment} is newly created. It
     * inflates the fragment_disclaimer layout as user interface for the {@link TSDFragment}.
     * @see TSDDisclaimerFragment
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
        return inflater.inflate(R.layout.fragment_disclaimer, container, false);
    }
}
