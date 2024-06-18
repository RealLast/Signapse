/*
 * Main code work by Jakob Hampel
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;


/**
 * This class defines a fragment dialog showing an error happened during initialization of the api.
 * This dialog is shown at startup of the application if api initialization failed and is used
 * therefore in the {@link MainActivity}. The error dialog is just a box with the in the resources
 * saved error text and an button to accept it.
 */
public class TSDInitializationErrorDialog extends DialogFragment
{
    /**
     * This function gets called to create the dialog. It sets the error text defined in
     * legal_disclaimer in resources and the accept button.
     */
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setMessage(R.string.init_error)
                .setPositiveButton(R.string.legal_disclaimer_accept,
                        (DialogInterface dialog, int id) -> dialog.cancel());

        return builder.create();
    }
}
