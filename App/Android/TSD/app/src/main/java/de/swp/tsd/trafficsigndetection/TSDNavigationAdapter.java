/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

/**
 * This class is an adapter for displaying the individual items in the navigation menu. It takes
 * {@link TSDFragment TSDFragments} as items and displays their titles and icons defined in there in
 * a list inside the navigation menu.
 */
public class TSDNavigationAdapter extends BaseAdapter
{
    private Context mContext;
    private TSDFragment[] mFragments;


    /**
     * This constructor creates a new instance of TSDNavigationAdapter taking all the
     * {@link TSDFragment TSDFragments} to display.
     * @param context Context the adapter is used in.
     * @param fragments Array of the {@link TSDFragment TSDFragments} that should be displayed as
     *                  menu items. The order of the items is from  array start to end displayed
     *                  from top to bottom.
     */
    TSDNavigationAdapter(Context context, TSDFragment[] fragments)
    {
        mContext = context;
        mFragments = fragments;
    }


    /**
     * This method queries the number of menu items or {@link TSDFragment TSDFragments} to display.
     * @return The number of menu items or {@link TSDFragment TSDFragments} to display.
     */
    @Override
    public int getCount()
    {
        return mFragments.length;
    }

    /**
     * This method queries the {@link TSDFragment} instance at a given position in the internal
     * array which was delivered at construction. This is the same as the position of the menu item.
     * @param position Position of the menu item to get the {@link TSDFragment} from.
     * @return {@link TSDFragment} which is displayed at the given position as menu item.
     */
    @Override
    public Object getItem(int position)
    {
        return mFragments[position];
    }

    /**
     * This method queries the id of the id of the item at the given position. It just returns
     * the position since the fragments itself should be unique in the list.
     * @param position Position of the menu item to get the id from.
     * @return Id or position of the item at this position.
     */
    @Override
    public long getItemId(int position)
    {
        return position;
    }

    /**
     * This method updates a given view to display the menu item for the given position using the
     * information given in the {@link TSDNavigationAdapter#mFragments fragments array} at this
     * position.
     * @param position Position to display the menu item at.
     * @param convertView View to display the information in.
     * @param parent View parent the view is used in.
     * @return The menu item view with the updated information.
     */
    @Override
    public View getView(int position, View convertView, ViewGroup parent)
    {
        // Inflate the layout for the item to display.
        if (convertView == null)
        {
            convertView = LayoutInflater.from(mContext)
                    .inflate(R.layout.navigation_item, parent, false);
        }

        // Get the item to display.
        TSDFragment item = (TSDFragment)getItem(position);

        // Get handles to display the information.
        ImageView iconView = convertView.findViewById(R.id.itemIconView);
        TextView textView = convertView.findViewById(R.id.itemTextView);

        // Set information to display this item.
        textView.setText(item.displayNameId);
        iconView.setImageResource(item.displayIconId);

        return convertView;
    }
}
