/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.arch.lifecycle.MutableLiveData;
import android.arch.lifecycle.ViewModel;
import android.util.Log;

/**
 * This class acts as a collection for the recognized sign combinations. This class extends
 * ViewModel. This way it is possible to maintain the history for the lifetime of the
 * {@link MainActivity} even when it is created inside a {@link TSDFragment}. It is just important
 * to not directly use the constructor but to use the builder factory like this:
 * ViewModelProviders.of(mActivity).get(TSDSignHistory.class)
 * Furthermore the {@link TSDSignCombination sign combinations} are stored inside
 * {@link MutableLiveData} what makes it possible to register as an listener to and to receive
 * signals whenever the collection is changed.
 * The history also contains a {@link TSDSpeedChecker} to track the currently allowed speed limit
 * using the history of recognized signs.
 */
public class TSDSignHistory extends ViewModel
{
    private MutableLiveData<TSDRingMemory<TSDSignCombination>> mSignCombinations =
            new MutableLiveData<>();

    private final TSDSpeedChecker mSpeedChecker;

    /**
     * This constructor creates a new instance of TSDHistorySign. Please do not call this directly
     * but use the builder factory like this:
     * ViewModelProviders.of(mActivity).get(TSDSignHistory.class);
     */
    public TSDSignHistory()
    {
        mSignCombinations.setValue(new TSDRingMemory<>(25));
        mSpeedChecker = new TSDSpeedChecker();
    }


    /**
     * Getter for the live data collection of {@link TSDSignCombination sign combinations}. Please
     * do not use the returned reference to directly change the content of the collection but only
     * for registering to as new change listener.
     * Instead use
     * {@link TSDSignHistory#addSignCombination(TSDSignCombination, Activity)} to add new
     * combinations.
     * @return Collection of sign combinations.
     */
    public MutableLiveData<TSDRingMemory<TSDSignCombination>> getSignCombinations()
    {
        return mSignCombinations;
    }


    /**
     * This method searches the history for the latest frame id. This can be used to use the
     * correct start value when relaunching or starting the detection.
     * @return Latest frame id.
     */
    public long getLatestFrameId()
    {
        TSDRingMemory<TSDSignCombination> signCombis = mSignCombinations.getValue();

        // Make sure a collection is set into the live data.
        if (signCombis == null)
        {
            signCombis = new TSDRingMemory<>(25);
        }

        // Look in all combis for the biggest frame id.
        long latestFrameId = 0;
        for (TSDSignCombination combi : signCombis)
        {
            if (combi.getFrameId() > latestFrameId)
            {
                latestFrameId = combi.getFrameId();
            }
        }

        return latestFrameId;
    }


    /**
     * This method searches the history for the latest id. This can be used to use the
     * correct start value when relaunching or starting the detection.
     * @return Latest id.
     */
    public long getLatestId()
    {
        TSDRingMemory<TSDSignCombination> signCombis = mSignCombinations.getValue();

        // Make sure a collection is set into the live data.
        if (signCombis == null)
        {
            signCombis = new TSDRingMemory<>(25);
        }

        // Look in all combis for the biggest frame id.
        long latestId = 0;
        for (TSDSignCombination combi : signCombis)
        {
            if (combi.getId() > latestId)
            {
                latestId = combi.getId();
            }
        }

        return latestId;
    }


    /**
     * This method adds a new sign combination to the back of the internal collection and signals
     * all listener to the sign live data. It also updates the {@link TSDSignHistory#mSpeedChecker}
     * In case the given sign combination is found to be a match to another combi in the history,
     * then they will can be merged into an updated combination that is added instead of the given
     * one. If the given sign combi is not standalone according to
     * {@link TSDSignCombination#isStandalone()} then it will at most be used for updating purposes
     * and then get discarded because non-standalone combis do not get displayed to the user and
     * once added to the history they cannot become standalone through updates.
     * @param signCombination Sign combination to add to collection.
     */
    public void addSignCombination(TSDSignCombination signCombination, Activity activity)
    {
        TSDRingMemory<TSDSignCombination> signCombis = mSignCombinations.getValue();

        // Make sure a collection is set into the live data.
        if (signCombis == null)
        {
            signCombis = new TSDRingMemory<>(25);
        }

        // Search in all sign combinations taken in this or some last frames whether the new sign
        // combination already was detected partially or whether it is a part of another sign combi.
        // If there is a matching one, update the sign combi and remove the found one from the list.
        final long currentFrameId = signCombination.getFrameId();
        boolean alreadyFound = false;

        for (int indexFound = signCombis.size() - 1; indexFound >= 0
                && signCombis.get(indexFound).getFrameId() >= currentFrameId - 2; indexFound--)
        {
            // If the new combi is a sublist of one old combi, then create an updated version from
            // both the old and the new combi, remove the old from the list and replace the new with
            // this updated instance.
            TSDSignCombination updatedCombi = signCombis.get(indexFound)
                    .createUpdatedBySublist(signCombination);
            if (updatedCombi != null)
            {
                // Remove the old from the list and replace the new one by the new instance.
                alreadyFound = true;
                signCombis.remove(indexFound);
                signCombination = updatedCombi;
            }
            // If an old combi is a sublist of the new one, then just remove the old combi because
            // we wont downgrade the new information with some old.
            else if (signCombis.get(indexFound).isSublistOf(signCombination))
            {
                alreadyFound = true;
                signCombis.remove(indexFound);
            }
        }

        // Add the sign combination as new element to the back of the list if it is a standalone
        // combination. Checking this after the updating ensures that the combi can be used for
        // updating already detected combinations, but it wont be added to the history. This is
        // because not-standalone combis inside the history will never become standalone because
        // of the updating logic. If a new one would be a sublist of it, this would not make it a
        // standalone one. If it is a sublist of an new combi, then it would only get removed.
        if (signCombination.isStandalone())
        {
            signCombis.add(signCombination);
        }

        // If the sign combi was already found and removed, then reset the speed checker and refill
        // it to avoid error caused by change of order. Otherwise just add it as new sign to the
        // checker.
        if (alreadyFound)
        {
            mSpeedChecker.clear();
            for (int i = 0; i < signCombis.size(); i++)
            {
                mSpeedChecker.addSigns(signCombis.get(i).getSigns());
            }
        }
        else
        {
            // Update the speed checker with the new signs only, when it was a new sign combination.
            mSpeedChecker.addSigns(signCombination.getSigns());
        }

        // Set the list again to notify live model observers.
        setValue(signCombis, activity);
    }

    /**
     * This method checks whether the given speed is ok in regards to the information given in the
     * history of recognized signs.
     * @param speed Current speed to check.
     * @param threshold Threshold that the speed can be higher than the limit and still be ok.
     * @return Whether the speed is at most as high as the speed limit plus the threshold.
     */
    public boolean isSpeedOk(Integer speed, int threshold)
    {
        return mSpeedChecker.isSpeedOk(speed, threshold);
    }


    /**
     * This methods sets the given value on the main thread to the live data because this can only
     * be done there. This enables the add and update methods to be called from background threads.
     * @param signCombis Sign combination collection to set to the life data.
     * @param activity Current main activity.
     */
    private void setValue(TSDRingMemory<TSDSignCombination> signCombis, Activity activity)
    {
        activity.runOnUiThread(() -> mSignCombinations.setValue(signCombis));
    }
}
