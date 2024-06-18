/*
 * Main code work by Florian Köhler
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.content.Context;
import android.support.annotation.NonNull;

/**
 * This enum defines (more than) the traffic signs, that can be detected by the RoadSignAPI.
 * The enums hold a value describing their effect on the current speed limit. The names of
 * the enums correspond to in most cases their full official id, and in some to the starting part
 * of it. Here an "_" stands for "-" and an "x" for an "." in the official id. Only MISC* are not
 * official ids and used for every sign that could not be classified to one of the other signs.
 * VZ278 is an broader id for all sub ids.
 */
public enum TSDSign
{
    MISC(0),
    VZ523_30(0),
    MISC_BLUE(0),
    MISC_RED_BLUE(0),
    MISC_RED_WHITE(0),
    MISC_WHITE(0),
    MISC_YELLOW(0),
    MISC_BROWN(0),
    MISC_GREEN(0),
    VZ1000(0),
    VZ1001_30(0),
    VZ1001_31(0),
    VZ1001_32(0),
    VZ1001_33(0),
    VZ1004_30(0),
    VZ1004_31(0),
    VZ101(0),
    VZ101_11(0),
    VZ103_10(0),
    VZ103_20(0),
    VZ1040_30(0),
    VZ1042_31(0),
    VZ1042_33(0),
    VZ105_10(0),
    VZ105_20(0),
    VZ1053_35(0),
    VZ120(0),
    VZ121_10(0),
    VZ121_20(0),
    VZ123(0),
    VZ131(0),
    VZ133_10(0),
    VZ136_10(0),
    VZ274_10(10),
    VZ274_20(20),
    VZ274_30(30),
    VZ274_40(40),
    VZ274_50(50),
    VZ274_60(60),
    VZ274_70(70),
    VZ274_80(80),
    VZ274_90(90),
    VZ274_100(100),
    VZ274_110(110),
    VZ274_120(120),
    VZ274_130(130),
    VZ274x1(30),        // 30 zone start
    VZ274x2(-1),        // 30 zone end
    VZ278(-1),
    VZ282(-1),
    VZ306(0),
    VZ310(50),          // town zone start
    VZ311(-1),          // town zone end
    VZ325x1(10),        // calmed zone start
    VZ325x2(-1),        // calmed zone end
    VZ330x1(1_000_000), // autobahn start
    VZ330x2(-1),        // autobahn end
    VZ331x1(100),       // automotive start
    VZ331x2(-1);        // automotive end


    /**
     * This value describes the direct effect of the sign on the allowed speed.
     * =0 means no direct effect or no effect that could handled properly (e.g. limits which are
     * limited in range)
     * >0 means specific speed limit.
     * <0 means annulment of the last speed limit, but not necessarily no speed limit.
     */
    public final int speedLimit;


    /**
     * This constructor creates a new instance of TSDSign.
     * @param speedLimit This value describes the direct effect of the sign on the allowed speed.
     *                   =0 means no direct effect or no effect that we could handle properly.
     *                   >0 means specific speed limit.
     *                   <0 means annulment of the last speed limit, but not necessarily no speed
     *                   limit.
     */
    TSDSign(int speedLimit)
    {
        this.speedLimit = speedLimit;
    }


    /**
     * This method checks whether this sign is the start of a special speed zone. Those zones can
     * be a calmed zone, a 30 zone, a town zone, an automotive road and an autobahn.
     * @return Whether this sign is the start of a special speed zone.
     */
    public boolean isZoneStart()
    {
        switch (this)
        {
            case VZ325x1:
            case VZ274x1:
            case VZ310:
            case VZ331x1:
            case VZ330x1:
                return true;

            default:
                return false;
        }
    }


    /**
     * This method checks, whether this sign ends the special speed zone started by the given sign.
     * Mind though, that zones can be nested. For example taking the incoming signs { town start,
     * 30 start, 30 end, town end }, the town zone is only ended with the last sign, not with the
     * start of the 30 zone. This is, because after the end of the 30 zone the town zone is still
     * relevant. A higher level speed zone end will also end every lower level speed zone end, where
     * higher and lower level directly refers the the allowed speed in the zone.
     * @param zoneStart Sign which started the special speed zone (can be detected by
     *                  {@link TSDSign#isZoneStart()}).
     * @return Whether this sign ends the zone or not.
     */
    public boolean endsZone(TSDSign zoneStart)
    {
        switch (zoneStart)
        {
            // Calmed zone is end by every other zone end.
            case VZ325x1:
                switch (this)
                {
                    case VZ325x2:
                    case VZ274x2:
                    case VZ311:
                    case VZ331x2:
                    case VZ330x2:
                        return true;

                    default:
                        return false;
                }

            // 30 zone is end by every other at least as high level zone end.
            case VZ274x1:
                switch (this)
                {
                    case VZ274x2:
                    case VZ311:
                    case VZ331x2:
                    case VZ330x2:
                        return true;

                    default:
                        return false;
                }

            // Town zone is end by every other at least as high level zone end.
            case VZ310:
                switch (this)
                {
                    case VZ311:
                    case VZ331x2:
                    case VZ330x2:
                        return true;

                    default:
                        return false;
                }

            // Automotive zone is end by every other at least as high level zone end.
            case VZ331x1:
                switch (this)
                {
                    case VZ331x2:
                    case VZ330x2:
                        return true;

                    default:
                        return false;
                }

            // Autobahn zone is only ended by its own zone end.
            case VZ330x1:
                return this == VZ330x2;

            default:
                return false;
        }
    }


    /**
     * This function delivers the correct sign enum for the given classification class id returned
     * by the RoadSignAPI.
     * @param id Classification class id from RoadSignAPI.
     * @return Matching sign enum for classification class id or if not found MISC.
     */
    public static TSDSign fromId(int id)
    {
        switch (id)
        {
            case 0: return VZ523_30;
            case 1: return MISC_BLUE;
            case 2: return MISC_RED_BLUE;
            case 3: return MISC_RED_WHITE;
            case 4: return MISC_WHITE;
            case 5: return MISC_YELLOW;
            case 6: return MISC_BROWN;
            case 7: return MISC_GREEN;
            case 8: return VZ1000;
            case 9: return VZ101;
            case 10: return VZ101_11;
            case 11: return VZ103_10;
            case 12: return VZ103_20;
            case 13: return VZ105_10;
            case 14: return VZ105_20;
            case 15: return VZ1053_35;
            case 16: return VZ120;
            case 17: return VZ121_10;
            case 18: return VZ121_20;
            case 19: return VZ123;
            case 20: return VZ131;
            case 21: return VZ133_10;
            case 22: return VZ136_10;
            case 23: return VZ274_10;
            case 24: return VZ274_100;
            case 25: return VZ274_120;
            case 26: return VZ274_130;
            case 27: return VZ274_20;
            case 28: return VZ274_30;
            case 29: return VZ274_40;
            case 30: return VZ274_50;
            case 31: return VZ274_60;
            case 32: return VZ274_70;
            case 33: return VZ274_80;
            case 34: return VZ274x1;
            case 35: return VZ274x2;
            case 36: return VZ278;
            case 37: return VZ282;
            case 38: return VZ306;
            case 39: return VZ310;
            case 40: return VZ311;
            case 41: return VZ325x1;
            case 42: return VZ325x2;
            case 43: return VZ330x1;
            case 44: return VZ330x2;
            case 45: return VZ331x1;
            case 46: return VZ331x2;
            default: return MISC;
        }
    }


    /**
     * This method queries, whether this sign should be saved into the history or not. This is used
     * for recognized but irrelevant signs to exclude them from history and displaying to the user.
     * @return Whether this sign should be included in the history or not.
     */
    public boolean isRelevant()
    {
        switch(this)
        {
            case MISC:
            case MISC_BLUE:
            case MISC_YELLOW:
            case MISC_BROWN:
            case MISC_GREEN:
            case MISC_RED_BLUE:
            case MISC_RED_WHITE:
            case MISC_WHITE:
            case VZ306:
                return false;

            default:
                return true;
        }
    }


    /**
     * This method returns, whether this sign is appropriate to be displayed alone in one
     * combination or if in this case, that the combination only consists of this or multiple
     * signs with the same result, the sign combination should not get displayed to the user.
     * If one combination consists of standalone and not standalone signs, then the combination
     * gets displayed.
     * @return Whether the sign can stand alone or not.
     */
    public boolean isStandalone()
    {
        switch(this)
        {
            case VZ523_30:
            case VZ274_10:
            case VZ274_20:
            case VZ274_30:
            case VZ274_40:
            case VZ274_50:
            case VZ274_60:
            case VZ274_70:
            case VZ274_80:
            case VZ274_90:
            case VZ274_100:
            case VZ274_110:
            case VZ274_120:
            case VZ274_130:
            case VZ274x1:
            case VZ274x2:
            case VZ278:
            case VZ282:
            case VZ310:
            case VZ311:
            case VZ325x1:
            case VZ325x2:
            case VZ330x1:
            case VZ330x2:
            case VZ331x1:
            case VZ331x2:
                return true;

            default:
                return false;
        }
    }


    /**
     * This method tries to load the matching drawable from resources to the given sign enum
     * instance. For every sign not present in the resources null will be returned.
     * @param database Database to use for drawable loading.
     * @param context Context the sign should be loaded in.
     * @return The found drawable or null.
     */
    public TSDDrawable getDrawable(@NonNull TSDDrawableDatabase database, @NonNull Context context)
    {
        switch(this)
        {
            case VZ101: return database.getDrawable(R.drawable.vz101, context);
            case VZ101_11: return database.getDrawable(R.drawable.vz101_11, context);
            case VZ103_10: return database.getDrawable(R.drawable.vz103_10, context);
            case VZ103_20: return database.getDrawable(R.drawable.vz103_20, context);
            case VZ105_10: return database.getDrawable(R.drawable.vz105_10, context);
            case VZ105_20: return database.getDrawable(R.drawable.vz105_20, context);
            case VZ120: return database.getDrawable(R.drawable.vz120, context);
            case VZ121_10: return database.getDrawable(R.drawable.vz121_10, context);
            case VZ121_20: return database.getDrawable(R.drawable.vz121_20, context);
            case VZ123: return database.getDrawable(R.drawable.vz123, context);
            case VZ131: return database.getDrawable(R.drawable.vz131, context);
            case VZ133_10: return database.getDrawable(R.drawable.vz133_10, context);
            case VZ136_10: return database.getDrawable(R.drawable.vz136_10, context);
            case VZ1053_35: return database.getDrawable(R.drawable.vz1053_35, context);
            case VZ274_10: return database.getDrawable(R.drawable.vz274_10, context);
            case VZ274_20: return database.getDrawable(R.drawable.vz274_20, context);
            case VZ274_30: return database.getDrawable(R.drawable.vz274_30, context);
            case VZ274_40: return database.getDrawable(R.drawable.vz274_40, context);
            case VZ274_50: return database.getDrawable(R.drawable.vz274_50, context);
            case VZ274_60: return database.getDrawable(R.drawable.vz274_60, context);
            case VZ274_70: return database.getDrawable(R.drawable.vz274_70, context);
            case VZ274_80: return database.getDrawable(R.drawable.vz274_80, context);
            case VZ274_90: return database.getDrawable(R.drawable.vz274_90, context);
            case VZ274_100: return database.getDrawable(R.drawable.vz274_100, context);
            case VZ274_120: return database.getDrawable(R.drawable.vz274_120, context);
            case VZ274_110: return database.getDrawable(R.drawable.vz274_110, context);
            case VZ274_130: return database.getDrawable(R.drawable.vz274_130, context);
            case VZ274x1: return database.getDrawable(R.drawable.vz274_1, context);
            case VZ274x2: return database.getDrawable(R.drawable.vz274_2, context);
            case VZ278: return database.getDrawable(R.drawable.vz278, context);
            case VZ282: return database.getDrawable(R.drawable.vz282, context);
            case VZ306: return database.getDrawable(R.drawable.vz306, context);
            case VZ310: return database.getDrawable(R.drawable.vz310, context);
            case VZ311: return database.getDrawable(R.drawable.vz311, context);
            case VZ325x1: return database.getDrawable(R.drawable.vz325_1, context);
            case VZ325x2: return database.getDrawable(R.drawable.vz325_2, context);
            case VZ330x1: return database.getDrawable(R.drawable.vz330_1, context);
            case VZ330x2: return database.getDrawable(R.drawable.vz330_2, context);
            case VZ331x1: return database.getDrawable(R.drawable.vz331_1, context);
            case VZ331x2: return database.getDrawable(R.drawable.vz331_2, context);
            default: return null;
        }
    }
}
