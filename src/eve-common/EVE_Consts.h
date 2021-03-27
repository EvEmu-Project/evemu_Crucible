/* EVEmu Constants
 *   this file is a common location for all static-type defined data
 */

#ifndef EVE_CONSTANTS_H
#define EVE_CONSTANTS_H

#include <array>

// define default home page for IGB
const std::string HomePageURL = "http://eve.example.org/";

/*
 *  misc static consts
 */

static const char numList[]     = "0123456789";
static const char hexList[]     = "0123456789ABCDEF";
static const char alphaList[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char asciiList[]   = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()[]{};:'<>?";

/* marshaled Python string "None" */
static const uint8 marshaledNone[] = { 0x74, 0x04, 0x00, 0x00, 0x00, 0x4E, 0x6F, 0x6E, 0x65 };

static const uint16 SHIP_PROCESS_TICK_MS = 5000;    // 5s

static const GPoint NULL_ORIGIN(0,0,0);  // common place for a zero-value gpoint
static const GVector NULL_ORIGIN_V(0,0,0);

const int32 ITEM_DB_SAVE_TIMER_EXPIRY(10);

static const float TIC_DURATION_IN_SECONDS(1000);       // not used yet

static const uint32 minWarpDistance(130000);    // 130km here, client and live defined as 150km

static const float onlineModInSpace(0.75);     // onling modules while NOT docked or using fitting services will take 75% of capacitor capacity.

//   based on client code...
static const int64 ONE_LIGHTYEAR = 9460000000000000;  // in meters
static const int64 ONE_AU_IN_METERS = 149597870700;     // 1 astronomical unit in meters, per EVElopedia: http://wiki.eveonline.com/en/wiki/Astronomical_Unit
static const int64 STATION_HANGAR_MAX_CAPACITY = 9000000000000000;  //per client
static const double MAX_MARKET_PRICE = 9223372036854;  //max int64/1000000  (9223372036854775807/1000000)
static const int32 INCAPACITATION_DISTANCE = 250000;    // drone to ship max distance.  after this, drone goes Offline and is considered 'lost'

// Cosmic Managers constants here  *not used yet*
static const uint32 ASTEROID_GROWTH_INTERVAL_MS = 3600000;  /* this is grow check in ms (1d) */

// gravitational constant (used for orbit math)
static const double Gc = 0.000000000066725985;     //per client (changed from original 6.673e-11)
static const float SMALL_NUMBER = 0.000000000000001;


// verify that NO ONE tries to use "ccp" in their name
// also check for mysql commands
static const std::array<std::string, 28> badWords {
    {"ccp",
    "admin",
    "fucker",
    "cunt",
    "concat",
    "collate",
    "select",
    "drop",
    "truncate",
    "count",
    "char",
    "load",
    "ascii",
    "union",
    "having",
    "group",
    "insert",
    "cast",
    "version",
    "exec ",
    "benchmark",
    "create",
    "md5",
    "sha1",
    "encode",
    "compress",
    "row_",
    "bulk"}
};
// check for common mysql injection hacks
//  special chars are illegal just because
static const std::array<std::string, 18> badChars {
    {";",
    "--",
    "#",
    "/*",
    "/0",
    "0x",
    "|",
    "' ",
    "+",
    "@",
    "!",
    "$",
    "%",
    "^",
    "&",
    "*",
    "(",
    ")"}
};

static const std::array<std::string, 16> badCharsSearch {
    {";",
    "--",
    "#",
    "/*",
    "/0",
    "0x",
    "|",
    "' ",
    "+",
    "@",
    "!",
    "$",
    "^",
    "&",
    "(",
    ")"}
};
#endif  // EVE_CONSTANTS_H

/*  misc data
 * radius constants
 * moon    =  1737km
 * mars    =  3390km
 * earth   =  6371km
 * jupiter = 69911km
 *
 * gravity constants
 * moon    =  1.622 m/s^2
 * mars    =  3.711 m/s^2
 * earth   =  9.807 m/s^2
 * jupiter = 24.790 m/s^2
 */

/* ship agility by class
 * class          agility
 * Capsule          .06
 * Shuttle          1.6
 * Rookie           5
 * Frigates         3 - 6 (adv. 3 - 4)
 * Destroyers       4 - 5
 * Cruisers         4 - 8
 * T3 Cruiser       2.4 - 2.8
 * HAC              5 - 7
 * Battlecruisers   6 - 9
 * Battleships      8 - 14
 * Industrials      8 - 12
 * Marauder         ~12
 * Orca             40
 * Freighters       ~60
 * Supercarrier     ~60
 * Command          ~9
 * Transport        5 or 19
 * Barges           10 - 18
 * Dreadnought      ~55
 * Zephyr           5
 */