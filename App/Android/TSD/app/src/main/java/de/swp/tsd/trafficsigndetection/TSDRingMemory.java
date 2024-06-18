/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Simple collection holding only a given maximum number of items.
 * @param <T> Item type
 */
public class TSDRingMemory<T> extends ArrayList<T>
{
    private int mMaxSize;


    /**
     * This constructor creates a new instance of TSDRingMemory setting the maximum number of
     * items this instance should be able to hold.
     * @param maxSize Integer defining the maximum number of items in this collection.
     */
    TSDRingMemory(int maxSize)
    {
        mMaxSize = maxSize;
    }


    /**
     * This method adds a new item to the collection after having removed enough items at index 0 to
     * make space for it.
     * @param item Item to add to collection.
     * @return Whether the collection changed as a result of this call.
     */
    @Override
    public boolean add(T item)
    {
        // Remove old items until there is space for one new item.
        while (size() >= mMaxSize)
        {
            remove(0);
        }

        // Add the new item.
        return super.add(item);
    }


    /**
     * This method adds a new item to the collection at the given position after having removed
     * enough items at index 0 to make space for it.
     * @param index Index to add the item in.
     * @param item Item to add to collection.
     */
    @Override
    public void add(int index, T item)
    {
        // Remove old items until there is space for one new item.
        while (size() >= mMaxSize)
        {
            remove(0);
        }

        // Insert the new item.
        super.add(index, item);
    }


    /**
     * This method adds the given collection to this collection after having removed enough items at
     * index 0 to make space for them.
     * @param c Collection of items to add to this collection.
     * @return Whether the collection changed as a result of this call.
     */
    @Override
    public boolean addAll(Collection<? extends T> c)
    {
        // Remove old items until there is space for the new items.
        while (size() > mMaxSize - c.size())
        {
            remove(0);
        }

        // Add the new items.
        return super.addAll(c);
    }

    /**
     * This method adds the given collection to this collection at the specified index after
     * having removed enough items at index 0 to make space for them.
     * @param index Index to add the items in.
     * @param c Collection of items to add to this collection.
     * @return Whether the collection changed as a result of this call.
     */
    @Override
    public boolean addAll(int index, Collection<? extends T> c)
    {
        // Remove old items until there is space for the new items.
        while (size() > mMaxSize - c.size())
        {
            remove(0);
        }

        // Insert the new items.
        return super.addAll(index, c);
    }


    /**
     * This method creates a new instance of TSDRingMemory with all but the last n items in this
     * instance. It can also reverse the order of the elements if wished. If reversed, still the
     * last items in the original order will be skipped, not the first.
     * @param n Number of last elements to skip.
     * @param reverse Reverse the order of the remaining items.
     * @return TSDRingMemory instance with all of the items of this instance but the last n.
     */
    public TSDRingMemory<T> getAllSkipLast(int n, boolean reverse)
    {
        // Create new memory.
        int size = size();
        TSDRingMemory<T> rm = new TSDRingMemory<>(getMaxSize());

        // Copy over everything but the last n items.
        for (int i = 0; i < size - n; i++)
        {
            if (reverse)
            {
                // For reverse order insert all items at the front.
                rm.add(0, get(i));
            }
            else
            {
                // For normal order just append the items.
                rm.add(get(i));
            }
        }

        return rm;
    }


    /**
     * Set the maximum number of items this collection should be able to hold. When the new max size
     * is smaller than the actual current size, then enough items at index 0 will get removed to
     * match the new given max size.
     * @param maxSize Maximum number of items.
     */
    public void setMaxSize(int maxSize)
    {
        mMaxSize = maxSize;

        // Remove old items until the size matches.
        while (size() > mMaxSize)
        {
            remove(0);
        }
    }


    /**
     * This method returns the maximum number of items this collection is able to hold.
     */
    public int getMaxSize()
    {
        return mMaxSize;
    }
}
