/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.support.annotation.NonNull;
import android.util.Log;

import java.util.Stack;
import java.util.concurrent.Semaphore;


/**
 * This multithreading proof class analyses given signs and classifies given speeds according to the
 * signs in ok or too fast. This is done by checking the incoming signs for opened or ended special
 * speed zones like described in {@link TSDSign}. When no special zone is open and not speed
 * limit is set, then a base speed limit is used. When a special speed zone is started and no
 * further speed limit is recognized, then the speed limit given by the zone is used. When inside
 * the zone a further speed limit occurs, then this speed limit will get used.
 * If in one combination there are multiple speed limits, then for safety reasons the lowest limit
 * is taken.
 * The multithreading proof is accomplished by using a semaphore to achieve exclusive access for one
 * method call from one thread to the internal variables at a given time.
 */
public class TSDSpeedChecker
{
    private final static int BASE_SPEED = 100;

    private Stack<TSDSign> mZones = new Stack<>();
    private int mSpeedLimit = -1; // If this is < 0, then the zone speed is relevant.
    private Semaphore mLock = new Semaphore(1);


    /**
     * This method resets all sign related internal saved state. This has the same effect like
     * creating a new instance without having create a new object.
     */
    public void clear()
    {
        try
        {
            mLock.acquire();

            // Clear the important variables.
            mZones = new Stack<>();
            mSpeedLimit = -1;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            mLock.release();
        }
    }


    /**
     * This method checks whether the given speed is under or withing the given threshold of the
     * currently recognized speed limit.
     * @see TSDSpeedChecker
     * @param speed Current speed.
     * @param threshold Threshold the speed can be higher than the limit.
     * @return Whether the speed is ok or not.
     */
    public boolean isSpeedOk(Integer speed, int threshold)
    {
        if (speed == null)
        {
            return true;
        }
        else
        {
            int speedLimit = -1;

            try
            {
                mLock.acquire();

                // If there is no speed limit, then check the stack for the latest limit. If the stack
                // is empty, then take the base speed as comparison.
                // If there is a speed limit, then take this for comparison.
                speedLimit = mSpeedLimit < 0 ?
                        (mZones.isEmpty() ? BASE_SPEED : mZones.peek().speedLimit) : mSpeedLimit;
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
            finally
            {
                mLock.release();
            }


            return speed <= speedLimit + threshold;
        }
    }


    /**
     * This method refreshes the internal logic for checking the speed by giving it new detected
     * signs.
     * @param signs New signs to add to consideration for speed checking.
     */
    public void addSigns(@NonNull TSDSign[] signs)
    {
        try
        {
            mLock.acquire();

            int minSpeedLimit = -1;
            boolean annulment = false;

            // Check each given sign.
            for (TSDSign sign : signs)
            {
                // If it starts a zone then add it to the stack and set the flag.
                if (sign.isZoneStart())
                {
                    mZones.push(sign);
                }

                // If it ends any zone on the stack then remove those zones and set the flag (an end
                // zone is like an annulment).
                // noinspection method
                annulment = annulment || mZones.removeIf(sign::endsZone);

                if (sign.speedLimit < 0)
                {
                    // Set a flag that there was an annulment sign. If there is no speed limiting
                    // sign, then this will be relevant.
                    annulment = true;
                }
                else if (sign.speedLimit > 0
                        && (sign.speedLimit < minSpeedLimit || minSpeedLimit < 0))
                {
                    // Save the minimum speed limit the signs contain.
                    minSpeedLimit = sign.speedLimit;
                }
            }

            // Prefer an speed limit over an annulment for safety reasons.
            if (minSpeedLimit > 0)
            {
                // If there was a new speed limit, then set this.
                mSpeedLimit = minSpeedLimit;
            }
            else if (annulment)
            {
                // If there was no new speed limit but an annulment, then reset the speed limit.
                mSpeedLimit = -1;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            mLock.release();
        }
    }
}
