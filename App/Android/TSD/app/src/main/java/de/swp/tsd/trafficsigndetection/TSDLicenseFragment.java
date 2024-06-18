package de.swp.tsd.trafficsigndetection;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * This {@link TSDFragment} displays the user the open source parts of this app and the
 * corresponding licenses.
 */
public class TSDLicenseFragment extends TSDFragment
{
    private final License[] mLicenses = new License[]
    {
        new License(R.string.license_title_android_sdk, R.string.license_text_android_sdk),

        new License(R.string.license_title_android_ndk, R.string.license_text_android_ndk),

        new License(R.string.license_title_android_architecture_components,
                R.string.license_text_android_architecture_components),

        new License(R.string.license_title_android_support, R.string.license_text_android_support),

        new License(R.string.license_title_junit, R.string.license_text_junit),

        new License(R.string.license_title_opencv, R.string.license_text_opencv),

        new License(R.string.license_title_tensorflow, R.string.license_text_tensorflow),

        new License(R.string.license_title_stackoverflow, R.string.license_text_stackoverflow),
    };


    /**
     * This constructor creates a new instance of {@link TSDLicenseFragment} setting the correct
     * values for {@link TSDFragment#displayNameId display name} and
     * {@link TSDFragment#displayIconId display icon}.
     */
    public TSDLicenseFragment()
    {
        super(R.string.license_display_name, R.drawable.ic_copyright);
    }


    /**
     * This method gets called every time the view of the {@link TSDFragment} is newly created. It
     * inflates the fragment_license layout as user interface for the {@link TSDFragment}.
     * @see TSDLicenseFragment
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
        View self = inflater.inflate(R.layout.fragment_license, container, false);

        // Get the layout where the license views should get listed in.
        LinearLayout licensesLayout = self.findViewById(R.id.licensesLayout);

        // Add each license defined in the mLicense array.
        for (License license : mLicenses)
        {
            // Inflate a license_item layout.
            View licenseView = inflater.inflate(R.layout.license_item, licensesLayout,
                    false);

            // Set the license specific title and text to the license view.
            ((TextView)licenseView.findViewById(R.id.licenseTitleView))
                    .setText(license.mTitleId);
            ((TextView)licenseView.findViewById(R.id.licenseTextView))
                    .setText(license.mLicenseId);

            // Add the license view to the license layout list.
            licensesLayout.addView(licenseView);
        }

        return self;
    }


    /**
     * Defines a little helper class to hold the resource ids used to show title and license text
     * of an open source component.
     */
    private class License
    {
        /**
         * Title of the open source component.
         */
        final int mTitleId;

        /**
         * License text used by the open source component.
         */
        final int mLicenseId;


        /**
         * This creates a new instance of a license.
         * @param titleId String resource id of the open source component name.
         * @param licenseId String resource id of the license text used by the open source
         *                  component.
         */
        License(int titleId, int licenseId)
        {
            mTitleId = titleId;
            mLicenseId = licenseId;
        }
    }
}
