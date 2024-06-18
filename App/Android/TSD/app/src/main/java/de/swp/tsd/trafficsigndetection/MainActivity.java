/*
 * Main code work by Florian Köhler
 * Disclaimer by Jakob Hampel
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.CoordinatorLayout;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.ContextCompat;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;


/**
 * This class defines the main activity or the main entry point of the application. Its task is to
 * load and manage the navigational parts of the activity - the {@link TSDFragment TSDFragments}.
 * It also provides the navigation/hamburger menu and the title bar. Each navigational item of the
 * navigation menu is of the base type {@link TSDFragment}. When adding a new navigation item, then
 * this is to define in an extra class and to add in here as instance to the internal items array
 * at the wanted position.
 * When clicking another navigation item, then the associated {@link TSDFragment} will be loaded
 * and displayed as fullscreen only overlapped by the title bar at the top left side of the screen.
 * The before loaded {@link TSDFragment} then is added to the backstack to be navigational by the
 * back key, gets stopped (but not destroyed) and then replaced by the new selected
 * {@link TSDFragment}. When using the back key for back navigation, then the before loaded
 * {@link TSDFragment} gets removed from the backstack and will get destroyed, when it is not longer
 * available at another place in the backstack.
 *
 * @image latex app_class_fragments.png width=16cm
 * @image html app_class_fragments.svg
 */
public class MainActivity extends FragmentActivity
{
    /**
     * Variable to store the fragment manager. It is queried in {@link #onCreate(Bundle) onCreate}
     * and used every time, when the displayed {@link TSDFragment} changes to carry them out.
     */
    private FragmentManager mFragmentManager;

    /**
     * Variable to store the handle to the navigation menu drawer. It is queried in
     * {@link #onCreate(Bundle) onCreate} and used to open and close the menu.
     */
    private DrawerLayout mDrawerLayout;

    /**
     * Variable to store the handle to the main content layout. It is queried in
     * {@link #onCreate(Bundle) onCreate} and used as argument for
     * {@link TSDPermissionManager#onRequestPermissionResult(int, Activity, int[],
     * CoordinatorLayout) onRequestPermissionResult} calls to give the {@link TSDPermissionManager}
     * a place to isRelevant {@link android.support.design.widget.Snackbar Snackbars} in.
     */
    private CoordinatorLayout mContentLayout;

    /**
     * Variable to store the handle to the title text ui element. It is queried in
     * {@link #onCreate(Bundle) onCreate} and the {@link TSDFragment#displayNameId
     * isRelevant name} of the in the menu chosen {@link TSDFragment} is displayed in.
     */
    private TextView mTitleView;

    /**
     * Variable to store the adapter being used to isRelevant the {@link TSDFragment navigation items }
     * to the navigation menu.
     */
    private TSDNavigationAdapter mNavigationAdapter;

    /**
     * Variable to store the adapter being used to isRelevant the
     * {@link TSDFragment info navigation items } to the navigation menu.
     */
    private TSDNavigationAdapter mInfoNavigationAdapter;

    /**
     * Variable to store the layout holding the {@link TSDFragment navigation items }.
     */
    private ListView mNavigationLayout;

    /**
     * Describes whether the initialization of the api was successful.
     */
    private boolean mApiInitialized;

    /**
     * Describes whether the first fragment was displayed. This fragment will become the last one in
     * back navigation before app exit.
     */
    private boolean mFirstFragmentShown;


    /**
     * Native function to initialize the RoadSignAPI which is written in native and accessed through
     * the java native interface.
     * @param numThreads Number of threads TensorFlow should use.
     * @return Boolean indicating whether the initialization was successful or not.
     */
    private native boolean initRoadSignAPI(int numThreads, AssetManager assetManager);


    // Used to load the 'native-lib' library on application startup.
    static
    {
        System.loadLibrary("native-lib");
    }


    /**
     * This method is called on the startup of this activity. It only loads some basic
     * {@link TSDFragment fragments} at start until the privacy policy, the disclosure and
     * the disclaimer gets accepted and it initializes the native RoadSignAPI.
     * @param savedInstanceState Contains key-value pairs that might have been saved at the last
     *                           stop of the app. Currently this does not get used.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Remove title bar and notification bar
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // Set flag to keep screen awake
        this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // Initialize the road sign api in native code and save the result.
        mApiInitialized = initRoadSignAPI(2, getBaseContext().getAssets());

        // Load the base ui.
        setContentView(R.layout.activity_main);

        // Get instances for important ui elements and helpers.
        mTitleView = findViewById(R.id.titleTextView);
        mDrawerLayout = findViewById(R.id.drawer_layout);
        mContentLayout = findViewById(R.id.content_layout);
        mNavigationLayout = findViewById(R.id.navigationItemLayout);
        ListView infoNavigationLayout = findViewById(R.id.infoNavigationItemLayout);
        mFragmentManager = getSupportFragmentManager();

        // Handle on click events for fragment selection in menu.
        mNavigationLayout.setOnItemClickListener(
                (AdapterView<?> parent, View view, int position, long id)
                        -> displayFragment((TSDFragment)mNavigationAdapter.getItem(position)) );

        // Handle on click events for fragment selection in info menu.
        infoNavigationLayout.setOnItemClickListener(
                (AdapterView<?> parent, View view, int position, long id)
                        -> displayFragment((TSDFragment)mInfoNavigationAdapter.getItem(position)) );

        // Load the info fragments always shown.
        TSDFragment[] infoItems = new TSDFragment[]
        {
            new TSDAboutFragment(),
            new TSDLicenseFragment(),
        };

        // Create and register the navigation adapter for the info fragments.
        mInfoNavigationAdapter = new TSDNavigationAdapter(this, infoItems);
        infoNavigationLayout.setAdapter(mInfoNavigationAdapter);

        // Check the shared preferences whether the user already accepted the privacy policy.
        boolean privacyPolicyAccepted = TSDHelper.readBooleanFromSharedPreferences(
                R.string.data_privacy_accepted_key, false, this);

        // If the privacy policy was not accepted already, then show it to the user.
        // Otherwise show the disclaimer.
        if (!privacyPolicyAccepted)
        {
            // Only load the privacy policy and the license notices.
            TSDFragment[] items = new TSDFragment[]
            {
                new TSDPrivacyFragment(),
            };

            // Create and register the adapter for navigation items.
            mNavigationAdapter = new TSDNavigationAdapter(this, items);
            mNavigationLayout.setAdapter(mNavigationAdapter);

            // Set start fragment to load.
            displayFragment(items[0]);
        }
        else
        {
            // When the privacy policy is accepted, the disclaimer will get shown.
            onPrivacyPolicyAccepted(null);
        }
    }


    /**
     * This method is called every time the back button is pressed. It is used to run the standard
     * action. Afterwards it updates the title shown in the app.
     */
    @Override
    public void onBackPressed()
    {
        super.onBackPressed();

        // Get the currently shown fragment and set the corresponding title text.
        TSDFragment fragment = (TSDFragment)mFragmentManager.getFragments().get(0);
        mTitleView.setText(fragment.displayNameId);
    }


    /**
     * This method is called every time a permission request result gets processed. It forwards the
     * delivered result information to {@link TSDPermissionManager#onRequestPermissionResult(int,
     * Activity, int[], CoordinatorLayout) onRequestPermissionResult}.
     * @param requestCode The code set when the permission was requested. This can be used to
     *                    identify the result.
     * @param permissions Array of strings containing the requested permissions.
     * @param grantResults Array containing the users choice for every requested permission.
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
            @NonNull int[] grantResults)
    {
        // Delegate call to permission manager.
        TSDPermissionManager.onRequestPermissionResult(requestCode, this, grantResults,
                mContentLayout);
    }


    /**
     * Getter for the {@link MainActivity#mContentLayout}. This can be used as view for snackbars.
     * @return The {@link MainActivity#mContentLayout}.
     */
    public CoordinatorLayout getContentLayout()
    {
        return mContentLayout;
    }


    /**
     * This method gets called from UI and opens the hamburger menu.
     * @param view View the method is called from.
     */
    public void onClickMenu(View view)
    {
        mDrawerLayout.openDrawer(GravityCompat.START);
    }


    /**
     * This method gets called from UI when the privacy policy was accepted and loads all relevant
     * {@link TSDFragment fragments} for the disclosure. It also saves a flag to the shared
     * preferences indicating that the user accepted the privacy policy. So at the next start
     * it does not has to be shown again.
     * The disclosure gets shown, when the camera permission was not granted, the location
     * permission not granted and the speed tracking is enabled, or when the disclosure was not
     * shown at least once, which is checked by a flag in the shared preferences. In short: It gets
     * shown when teh user is likely to come across permission requests during his following usage
     * of the app.
     * @param view View the method is called from.
     */
    public void onPrivacyPolicyAccepted(View view)
    {
        TSDHelper.writeBooleanToSharedPreferences(R.string.data_privacy_accepted_key, true,
                this);

        // Check, whether the camera permission was accepted,...
        int camRes = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA);
        boolean permissionsOk = camRes == PackageManager.PERMISSION_GRANTED;

        // ...whether the location permission is either granted or the speed tracking disabled...
        int locRes = ContextCompat
                .checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION);
        boolean trackRes = TSDHelper.readBooleanFromSharedPreferences(
            R.string.data_gps_key, true, this);
        permissionsOk = permissionsOk && (locRes == PackageManager.PERMISSION_GRANTED || !trackRes);

        // ...and whether the disclosure got shown at least once.
        permissionsOk = permissionsOk &&  TSDHelper.readBooleanFromSharedPreferences(
                R.string.data_disclosure_accepted_key, false, this);

        // If permissions are not ok, then show the disclosure, otherwise show the disclaimer.
        if (!permissionsOk)
        {
            TSDFragment[] items = new TSDFragment[]
            {
                new TSDDisclosureFragment(),
            };

            // Create and register the adapter for navigation items.
            mNavigationAdapter = new TSDNavigationAdapter(this, items);
            mNavigationLayout.setAdapter(mNavigationAdapter);

            // Set start fragment to load.
            resetBackstack();
            displayFragment(items[0]);
        }
        else
        {
            // When the disclosure is accepted, the disclaimer will get shown.
            onDisclosureAccepted(null);
        }
    }


    /**
     * This method gets called from UI when the disclosure was accepted and loads all relevant
     * {@link TSDFragment fragments} for the disclaimer. It also saves a flag to the shared
     * preferences indicating that the user accepted the disclosure. So at the next start
     * it does not has to be shown again.
     * @param view View the method is called from.
     */
    public void onDisclosureAccepted(View view)
    {
        TSDHelper.writeBooleanToSharedPreferences(R.string.data_disclosure_accepted_key, true,
                this);

        // Only load the disclaimer.
        TSDFragment[] items = new TSDFragment[]
        {
            new TSDDisclaimerFragment(),
        };

        // Create and register the adapter for navigation items.
        mNavigationAdapter = new TSDNavigationAdapter(this, items);
        mNavigationLayout.setAdapter(mNavigationAdapter);

        // Set start fragment to load.
        resetBackstack();
        displayFragment(items[0]);
    }


    /**
     * This method gets called from UI when the disclaimer was accepted and loads all relevant
     * {@link TSDFragment fragments} for the app usage. It also shows an error dialog when the
     * initialization of the api failed.
     * @param view View the method is called from.
     */
    public void onDisclaimerAccepted(View view)
    {
        // Load the fragments used as menu items. If the initialization of native failed, then
        // do not load the fragments which access on api init dependent native functions.
        TSDFragment[] items = mApiInitialized ? new TSDFragment[]
                {
                    new TSDSetupFragment(),
                    new TSDDetectorFragment(),
                    /*new TSDDebugFragment(),*/
                    new TSDSettingsFragment(),
                }
                :
                new TSDFragment[]
                {
                    new TSDSetupFragment(),
                    new TSDSettingsFragment(),
                };

        // Create and register the adapter for navigation items.
        mNavigationAdapter = new TSDNavigationAdapter(this, items);
        mNavigationLayout.setAdapter(mNavigationAdapter);

        // Reset the selected fragment and set start fragment to load.
        resetBackstack();
        displayFragment(items[0]);

        // Show error dialog to inform if api initialization failed.
        if (!mApiInitialized)
        {
            TSDInitializationErrorDialog errorDialog = new TSDInitializationErrorDialog();
            errorDialog.show(getFragmentManager(), "initErrorDialog");
        }
    }


    /**
     * This method gets called from UI and opens the detector fragment. This is used from
     * {@link TSDSetupFragment} its user interface to finish setup.
     * @param view View the method is called from.
     */
    public void openDetectorFragment(View view)
    {
        for (int i = 0; i < mNavigationAdapter.getCount(); i++)
        {
            TSDFragment fragment = (TSDFragment)mNavigationAdapter.getItem(i);
            if (fragment.displayNameId == R.string.detector_display_name)
            {
                displayFragment(fragment);
                return;
            }
        }

        Log.e("MainActivity", "Fragment 'Detector' not found!");
    }


    /**
     * This method displays the given {@link TSDFragment} in fullscreen. This pushes the before
     * shown {@link TSDFragment} to the backstack, making it navigational by the back key, stops but
     * not destroys it and replaces its place on screen by the new {@link TSDFragment}. Which
     * {@link TSDFragment fragments} are available and in which order is defined inside
     * {@link #onCreate(Bundle) onCreate}.
     * @param fragment {@link TSDFragment} to isRelevant
     */
    private void displayFragment(TSDFragment fragment)
    {
        // Display the selected fragment.
        FragmentTransaction transaction = mFragmentManager.beginTransaction();
        transaction.replace(R.id.contentView, fragment);

        // Do not add the first fragment to the backstack to allow closing the app by pressing
        // back.
        if (mFirstFragmentShown)
        {
            transaction.addToBackStack(null);
        }
        else
        {
            // Otherwise set the flag that the first fragment now gets shown.
            mFirstFragmentShown = true;
        }

        transaction.commit();

        // Set title text.
        mTitleView.setText(fragment.displayNameId);

        // Close the drawer after selection.
        mDrawerLayout.closeDrawer(GravityCompat.START);
    }


    /**
     * This method pops all entries from the backstack and resets the
     * {@link MainActivity#mFirstFragmentShown flag}.
     * This way the next displayed fragment will be the base and when the back button is pressed in
     * there the application will close.
     */
    private void resetBackstack()
    {
        mFirstFragmentShown = false;

        for (int i = 0; i < mFragmentManager.getBackStackEntryCount(); i++)
        {
            mFragmentManager.popBackStack();
        }
    }
}
