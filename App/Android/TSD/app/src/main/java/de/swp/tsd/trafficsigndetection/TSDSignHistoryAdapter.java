/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.arch.lifecycle.LifecycleOwner;
import android.support.annotation.NonNull;
import android.support.v7.util.DiffUtil;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import java.util.List;


/**
 * This class is an adapter to display all but the latest sign combinations in a given sign
 * history to a {@link RecyclerView}. For displaying the single sign combination a
 * {@link TSDSignCombinationViewHolder} is used. The sign itself are drawn transparent to be
 * easily distinguished from the main sign. No speed or warning information are displayed in the
 * history.
 * The display of the history is implemented using a {@link RecyclerView} because this type of
 * list view efficiently reuses some views to display the visible part of the list instead of
 * either creating new ones the whole time when the user is scrolling or instead of holding one
 * view for every sign in the collection, which would also be inefficient, since only a portion of
 * them is visible.
 */
public class TSDSignHistoryAdapter extends RecyclerView.Adapter<TSDSignCombinationViewHolder>
{
    private final static float SIGN_TRANSPARENCY = 0.25f;

    private TSDRingMemory<TSDSignCombination> mSignCombis = new TSDRingMemory<>(25);

    private boolean mIsScrolling;
    private boolean mListChangedWhileScrolling;


    /**
     * This constructor creates a new instance of {@link TSDSignHistoryAdapter} which starts to
     * display the list of given sign combinations in sign card views. For doing this it registers
     * a listener for changes in the history which will listen as long as the given lifecycle owner
     * lives. Every time the history gets updated, the adapter refreshes the internal
     * {@link TSDSignHistoryAdapter#mSignCombis list} and updated the {@link RecyclerView} with
     * the new elements, too. As long as the first card is at least partially visible, the view
     * will automatically scroll to the start when a new card is inserted at the front. Otherwise
     * the user would continue to see the old cards when not scrolling to the start of the list
     * himself.
     * @param lifecycleOwner The lifecycle owner in which this adapter was created so that it does
     *                       not observe changes in the history for too long.
     * @param history Sign history to display.
     * @param layoutManager The layout manager the adapter items are managed and displayed in.
     * @param recyclerView The recycler view the adapter is used with.
     */
    TSDSignHistoryAdapter(@NonNull LifecycleOwner lifecycleOwner, @NonNull TSDSignHistory history,
                          @NonNull LinearLayoutManager layoutManager,
                          @NonNull RecyclerView recyclerView)
    {
        // Get sign collection.
        TSDRingMemory<TSDSignCombination> combis = history.getSignCombinations().getValue();

        // Copy the sign collection over.
        if (combis != null)
        {
            // Copy items over reversed but skip the latest one.
            mSignCombis = combis.getAllSkipLast(1, true);
        }

        // Get whether the view is currently scrolling or not. If the view changed while scrolling
        // then scroll back to start when finished.
        recyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(RecyclerView recyclerView, int newState)
            {
                super.onScrollStateChanged(recyclerView, newState);

                switch (newState)
                {
                    case RecyclerView.SCROLL_STATE_DRAGGING:
                    case RecyclerView.SCROLL_STATE_SETTLING:
                        mIsScrolling = true;
                        break;

                    case RecyclerView.SCROLL_STATE_IDLE:
                    {
                        mIsScrolling = false;
                        if (mListChangedWhileScrolling)
                        {
                            layoutManager.smoothScrollToPosition(recyclerView, null,
                                    0);
                            mListChangedWhileScrolling = false;
                        }

                        break;
                    }
                }
            }
        });

        // Listen for changes in this sign collection.
        history.getSignCombinations().observe(lifecycleOwner, signCombis ->
        {
            if (signCombis != null)
            {
                // Copy items over reversed but skip the latest one.
                TSDRingMemory<TSDSignCombination> signCombisCopy =
                        signCombis.getAllSkipLast(1, true);

                // Get the differences.
                DiffUtil.DiffResult diffResult =
                        DiffUtil.calculateDiff(new DiffCallback(mSignCombis, signCombisCopy));

                // Apply the changes.
                diffResult.dispatchUpdatesTo(this);

                // Update the sign combi collection.
                mSignCombis.clear();
                mSignCombis = signCombisCopy;

                // If the view is currently not scrolling, then go back to start to show the
                // changes. If it is scrolling, then set the flag and let the scroll listener
                // above handle the scrolling back when the view is finished scrolling.
                if (!mIsScrolling)
                {
                    layoutManager.smoothScrollToPosition(recyclerView, null, 0);
                }
                else
                {
                    mListChangedWhileScrolling = true;
                }
            }
        });
    }


    /**
     * This method updates the time display for items in the given inclusive range. This should get
     * called periodically to keep the timespan displayed on the cards up-to-date. Only updating
     * the cards, which are currently visible in the view, can save performance.
     * @param from Inclusive index to update time display in.
     * @param to Inclusive index to update time display in.
     */
    public void updateTimeDisplay(int from, int to)
    {
        if (from > -1 && to > -1)
        {
            for (int i = from; i <= to && i < getItemCount(); i++)
            {
                // A change notify with the same item as payload will only refresh the time.
                notifyItemChanged(i, mSignCombis.get(i));
            }
        }
    }


    /**
     * This method gets called when the content of a card gets replaced. The given view holder is
     * then told to display the sign at the given position in
     * {@link TSDSignHistoryAdapter#mSignCombis}.
     * @param holder View holder to display the selected sign combination in.
     * @param position Position the sign combination is in the collection that should get displayed.
     */
    @Override
    public void onBindViewHolder(@NonNull TSDSignCombinationViewHolder holder, int position)
    {
        TSDSignCombination sign = mSignCombis.get(position);
        holder.displayAll(sign, SIGN_TRANSPARENCY, 0f);
        setMargin(holder, position);
    }


    /**
     * This method gets called when the content of a card gets replaced or updated. The given view
     * holder is then told to display only the changes between the currently displayed sign and the
     * newly selected one to display. This can make the recycler view more efficient.
     * {@link TSDSignHistoryAdapter#mSignCombis}.
     * @param holder View holder to display the selected sign combination in.
     * @param position Position the sign combination is in the collection that should get displayed.
     * @param payloads Contains the {@link TSDSignCombination} that is currently displayed in the
     *                 view.
     */
    @Override
    public void onBindViewHolder(@NonNull TSDSignCombinationViewHolder holder, int position,
                                 @NonNull List<Object> payloads)
    {
        // Catch unwanted calls to this one.
        if (payloads.isEmpty())
        {
            super.onBindViewHolder(holder, position, payloads);
        }
        else
        {
            // Get old and new combination.
            TSDSignCombination oldCombi = mSignCombis.get(position);
            // noinspection unchecked
            TSDSignCombination newCombi = (TSDSignCombination) payloads.get(0);

            // Display only the changes.
            holder.displayAllChanges(oldCombi, newCombi, SIGN_TRANSPARENCY, 0f);
            setMargin(holder, position);
        }
    }


    /**
     * Queries for the number of sign combinations which are in the internal
     * {@link TSDSignHistoryAdapter#mSignCombis sign collection}.
     * @return Number of sign combinations in the internal collection.
     */
    @Override
    public int getItemCount()
    {
        return mSignCombis.size();
    }


    /**
     * This method gets called when a new {@link TSDSignCombinationViewHolder} gets created and
     * linked to the given view. First the card layout is inflated and then this inflated view
     * is used to create the view holder.
     * @param parent Parent the view should be inflated in, but not attached to its root.
     * @param viewType unused
     * @return The new view holder which can be used to display sign combinations in.
     */
    @Override
    public @NonNull TSDSignCombinationViewHolder onCreateViewHolder(@NonNull ViewGroup parent,
                                                                    int viewType)
    {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.sign_card, parent, false);

        // Set width as 1/2 of the parent height.
        ViewGroup.LayoutParams params = view.getLayoutParams();
        params.width = parent.getHeight() / 2;
        view.setLayoutParams(params);

        return new TSDSignCombinationViewHolder(view);
    }


    /**
     * This method sets the correct left and right margins for the sign cards depending on their
     * position in the array to create a simple margin between cards and a double margin at the
     * start and end of the list.
     * @param holder View holder for the card the margin is to set.
     * @param position Index of the card in the list.
     */
    private void setMargin(@NonNull TSDSignCombinationViewHolder holder, int position)
    {
        // Calculate and set the margins for this item position.
        int leftMargin = position == 0 ? 16 : 8;
        int rightMargin = position == (getItemCount() - 1) ? 16 : 0;
        holder.setLeftRightMargin(leftMargin, rightMargin);
    }

    /**
     * This class defines a callback for the DiffUtils to find the difference between two
     * TSDRingMemories with TSDSignCombinations. This is used to identify the specific changes when
     * the life data of the sign history signals an update (Since it does not tell what changed in
     * the list).
     */
    class DiffCallback extends DiffUtil.Callback
    {
        private TSDRingMemory<TSDSignCombination> mOldMemory;
        private TSDRingMemory<TSDSignCombination> mNewMemory;


        /**
         * This constructor creates a new instance of DiffCallback to be used to find differences
         * between two TSDRingMemories with TSDSignCombinations.
         * @param oldMemory Old collection.
         * @param newMemory New Collection.
         */
        DiffCallback(TSDRingMemory<TSDSignCombination> oldMemory,
                     TSDRingMemory<TSDSignCombination> newMemory)
        {
            mOldMemory = oldMemory;
            mNewMemory = newMemory;
        }


        /**
         * This method queries the size of the old list.
         * @return Size of the old list.
         */
        @Override
        public int getOldListSize()
        {
            return mOldMemory.size();
        }


        /**
         * This method queries the size of the new list.
         * @return Size of the new list.
         */
        @Override
        public int getNewListSize()
        {
            return mNewMemory.size();
        }


        /**
         * This method checks whether some sign combinations have the same ids.
         * @param oldPos Index of the combi in the old list.
         * @param newPos Index of the combi in the new list.
         * @return Whether the combis are the same or not.
         */
        @Override
        public boolean areItemsTheSame(int oldPos, int newPos)
        {
            return mOldMemory.get(oldPos).getId() == mNewMemory.get(newPos).getId();
        }


        /**
         * This method checks, when the sign were told to be the same by
         * {@link DiffCallback#areItemsTheSame(int, int)}, whether the content was updated or not.
         * @param oldPos Index of the combi in the old list.
         * @param newPos Index of the combi in the new list.
         * @return Whether the content of the combi was updated or not.
         */
        @Override
        public boolean areContentsTheSame(int oldPos, int newPos)
        {
            return mOldMemory.get(oldPos).equals(mNewMemory.get(newPos));
        }


        /**
         * This method just returns the old sign combi as payload. The differences are found out in
         * {@link TSDSignHistoryAdapter#onBindViewHolder(TSDSignCombinationViewHolder, int, List)}.
         * @param oldPos Index of the combi in the old list.
         * @param newPos Index of the combi in the new list.
         * @return Old sign combination.
         */
        @Override
        public Object getChangePayload(int oldPos, int newPos)
        {
            // Just return the new sign combination.
            return mNewMemory.get(newPos);
        }
    }
}
