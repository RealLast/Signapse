package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

/**
 * This {@link TSDPrivacyFragment} is used to present the user the privacy policy of this
 * application. It is the first {@link TSDPrivacyFragment} shown when the app is started the first
 * time and will not give the possibility to continue to the main application without having it
 * accepted.
 */
public class TSDPrivacyFragment extends TSDFragment
{
    private Activity mActivity;

    /**
     * This constructor creates a new instance of {@link TSDPrivacyFragment} setting the correct
     * values for {@link TSDFragment#displayNameId isRelevant name} and
     * {@link TSDFragment#displayIconId isRelevant icon}.
     */
    public TSDPrivacyFragment()
    {
        super(R.string.privacy_display_name, R.drawable.ic_attention);
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
     * inflates the fragment_license layout as user interface for the {@link TSDFragment}.
     * @see TSDPrivacyFragment
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
        View self = inflater.inflate(R.layout.fragment_privacy, container, false);

        if (mActivity == null)
        {
            mActivity = getActivity();
        }

        if (mActivity != null)
        {

        }

        return self;
    }
}
