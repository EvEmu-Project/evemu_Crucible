
/*
 *
 *
 *
 */

#ifndef EVE_TYPEDEFS_H
#define EVE_TYPEDEFS_H

#include "tables/invCategories.h"
//typedef EVEDB::invCategories::invCategories EVEItemCategories;
#include "tables/invGroups.h"
typedef EVEDB::invGroups::invGroups EVEItemGroups;
#include "tables/invTypes.h"
typedef EVEDB::invTypes::invTypes EVEItemTypes;


//List of eve item types which have special purposes in the game.
typedef enum {
    itemTypeCapsule = 670,
    itemTypeTrit    = 34,
    itemTypeCredits = 29,
    itemTypeJetCan  = 23,
    itemCivilianMiner = 3651,
    AllianceTypeID  = 16159,
    itemTypeBillboard = 11136,
    itemCloneAlpha  = 164
} EVEItemTypeID;


//  -allan 7Jul14
typedef enum {
    DungeonStarted      = 0,
    DungeonCompleted    = 1,
    DungeonFailed       = 2
} DungeonState;

//message format argument types:
typedef enum {
    fmtMapping_OWNERID2 = 1,    //not used? owner name
    fmtMapping_OWNERID = 2,    //owner name
    fmtMapping_LOCID = 3,    //locations
    fmtMapping_itemTypeName = 4,    //TYPEID: takes the item ID
    fmtMapping_itemTypeDescription = 5,    //TYPEID2: takes the item ID
    fmtMapping_blueprintTypeName = 6,    //from invBlueprints
    fmtMapping_itemGroupName = 7,    //GROUPID: takes the item group ID
    fmtMapping_itemGroupDescription = 8,    //GROUPID2: takes the item group ID
    fmtMapping_itemCategoryName = 9,    //CATID: takes the item category ID
    fmtMapping_itemCategoryDescription = 10,    //CATID2: takes the item category ID
    fmtMapping_DGMATTR = 11,    //not used...
    fmtMapping_DGMFX = 12,        //not used...
    fmtMapping_DGMTYPEFX = 13,    //not used...
    fmtMapping_dateTime = 14,    //DATETIME: formatted date and time
    fmtMapping_date = 15,        //DATE: formatted date
    fmtMapping_time = 16,        //TIME: formatted time
    fmtMapping_shortTime = 17,    //TIMESHRT: formatted time, short format
    fmtMapping_long = 18,        //AMT: regular number format
    fmtMapping_ISK2 = 19,        //AMT2: ISK format
    fmtMapping_ISK3 = 20,        //AMT3: ISK format
    fmtMapping_distance = 21,        //DIST: distance format
    fmtMapping_message = 22,    //MSGARGS: nested message
    fmtMapping_ADD_THE = 22,    //ADD_THE: prefix argument with 'the '
    fmtMapping_ADD_A = 23,        //ADD_A: prefix argument with 'a ' or 'an ' as appropriate
    fmtMapping_typeQuantity = 24,    //TYPEIDANDQUANTITY: human readable representation of the two arguments: typeID and quantity
    fmtMapping_ownerNickname = 25,    //OWNERIDNICK: first part of owner name (before space)
    fmtMapping_station = 26,    //SESSIONSENSITIVESTATIONID: human readable, fully qualified station name (includes system, constellation and region)
    fmtMapping_system = 27,    //SESSIONSENSITIVELOCID: human readable, fully qualified system name (includes constellation and region)
    fmtMapping_ISK = 28,        //ISK: ISK format
    fmtMapping_TYPEIDL = 29
} fmtMappingType;

typedef enum {
    ownerNone               = 0,
    ownerSystem             = 1,
    ownerBank               = 2,
    ownerStation            = 4,
    ownerCombatSimulator    = 5,
    //ownerUnknown            = 3006
} ownerDefs;

typedef enum {
//locationHiddenSpace = 9000001

    locAbstract    = 0,
    locSystem      = 1,
    locBank        = 2,
    locTemp        = 5,
    locTrading     = 7,
    locGraveyard   = 8,
    locUniverse    = 9,
    locJunkyard    = 10,
    locCorp        = 13,
    locSingletonJunkyard = 25,
    locTradeJunkyard = 1008,
    locCharGraveyard = 1501,
    locCorpGraveyard = 1502,
    locRAMItems    = 2003,
    locAlliance    = 3007
} locations;

typedef enum {
    /* this is a numeric system to organize salvage data types and is arranged as follows:
     * ABCDE
     * A  = tech level of salvage  - 1,2,3
     * B  = type of salvage: 0-, 1-data, 2-archaeology, 3-ancient, 4-wrecks, 5-loot, 6-other, 7-, 8-, 9-
     * C  = size of salvage: 0-tiny, 1-small, 2-med, 3-large, 4-huge, 5-, 6-, 7-, 8-, 9-
     * DE = last 2 actual faction numbers as defined in FactionDef
     */
    salvageSleepers     = 0

} FactionSalvageDef;

typedef enum {
    salvageData     = 1,
    salvageArch     = 2,
    salvageAncient  = 3,
    salvageWreck    = 4,
    salvageLoot     = 5,
    salvageOther    = 6
} SalvageTypes;

// not sure what these are used for...stateFlags??
typedef enum {
    mouseOver = 1,
    selected = 2,
    activeTarget = 3,
    targeting = 4,
    targeted = 5,
    lookingAt = 6,
    threatTargetsMe = 7,
    threatAttackingMe = 8,
    flagCriminal = 9,
    flagDangerous = 10,
    flagSameFleet = 11,
    flagSameCorp = 12,
    flagAtWarCanFight = 13,
    flagSameAlliance = 14,
    flagStandingHigh = 15,
    flagStandingGood = 16,
    flagStandingNeutral = 17,
    flagStandingBad = 18,
    flagStandingHorrible = 19,
    flagIsWanted = 20,
    flagAgentInteractable = 21,
    gbEnemySpotted = 22,
    gbTarget = 23,
    gbHealShield = 24,
    gbHealArmor = 25,
    gbHealCapacitor = 26,
    gbWarpTo = 27,
    gbNeedBackup = 28,
    gbAlignTo = 29,
    gbJumpTo = 30,
    gbInPosition = 31,
    gbHoldPosition = 32,
    gbTravelTo = 33,
    gbJumpBeacon = 34,
    gbLocation = 35,
    flagWreckAlreadyOpened = 36,
    flagWreckEmpty = 37,
    flagWarpScrambled = 38,
    flagWebified = 39,
    flagECMd = 40,
    flagSensorDampened = 41,
    flagTrackingDisrupted = 42,
    flagTargetPainted = 43,
    flagAtWarMilitia = 44,
    flagSameMilitia = 45,
    flagEnergyLeeched = 46,
    flagEnergyNeut = 47,
    flagNoStanding = 48
} stateFlags;

#endif  // EVE_TYPEDEFS_H