
/*
 * EVE_POS.h
 * data containers for POS items
 *
 */


#ifndef EVE_POS_ENUMS_H
#define EVE_POS_ENUMS_H

class GVector;

namespace EVEPOS {
    struct CustomsData {
        bool allowAlliance :1;;
        bool allowStandings :1;;
        int8 state;         /* used to hold POS state (online, reinforced, operating, etc) */
        int8 status;        /* used to hold POS StructureState (online, reinforced, operating, etc) */
        int8 selectedHour;  /* determines timeframe structure will come out of reinforced mode */
        int8 standingValue; /* used to determine allowed access.  see StandingValues:: for details */
        int32 itemID;
        int32 ownerID;
        int64 timestamp;
        std::map<uint8, float> taxRateValues;   /* taxRates based on standings with owner */
    };

    struct StructureData {
        int8 use;
        int8 view;
        int8 take;
        int8 state;         /* used to hold POS state (online, reinforced, operating, etc) */
        int8 status;        /* used to hold POS StructureState (online, reinforced, operating, etc) */
        int32 itemID;
        int32 towerID;
        int32 anchorpointID;       /* used to hold PlanetID for structures which are anchored near moons/planets/stargates */
        int64 timestamp;
    };
    struct TowerData {
        bool allowCorp :1;
        bool allowAlliance :1;
        bool allyStandings :1;
        bool statusDrop :1;
        bool corpWar :1;
        bool showInCalendar :1;
        bool sendFuelNotifications :1;
        int8 anchor;
        int8 unanchor;
        int8 online;
        int8 offline;
        int8 selectedHour;  /* determines timeframe structure will come out of reinforced mode */
        int32 harmonic;       /* this is POS ForceField status */
        int32 standingOwnerID;
        float status;         /* this is different from structure.status:  it is used for defense standings and needs to be renamed */
        float standing;
        std::string password;
    };

    struct JumpBridgeData {
        bool allowCorp :1;
        bool allowAlliance :1;
        int32 itemID;
        int32 corpID;
        int32 allyID;
        int32 towerID;
        int32 systemID;
        int32 toSystemID;
        int32 toItemID;
        int32 toTypeID;
        std::string password;
    };

    struct OrbitalData {
        int8 level;
        uint32 planetID;
        uint32 orbitalHackerID;
        uint32 standingOwnerID; // corp/ally
        float orbitalHackerProgress;
        GVector rotation;      /* yaw,pitch,roll - direction to planet (for correct orientation) */
    };

    struct POS_Connections {
        uint32 toID;
        uint32 sourceID;
    };

    struct POS_Resource {
        uint32 typeID;
        uint32 quantity;
    };

    namespace Harmonic {
        enum  {
            Inactive = -1,  // shows 'offline' in mgr window
            Offline = 0,    // shows 'offline' in mgr window
            Online = 1
        };
    }

    namespace ProcState {
        // these are internal only, used to determine method call on timer tic
        enum {
            Invalid            = 0,
            Unanchoring        = 1,
            Anchoring          = 2,
            Offlining          = 3,
            Onlining           = 4,
            Online             = 5,
            Operating          = 6,
            Reinforcing        = 7,
            SheildReinforcing  = 8,
            ArmorReinforcing   = 9
        };
    }

    namespace StructureState {
        enum  {
            Incapacitated     = -1,
            Unanchored        = 0,
            Anchored          = 1,
            Onlining          = 2,
            Reinforced        = 3,
            Online            = 4,
            Operating         = 5,
            Vulnerable        = 6,
            SheildReinforced  = 7,
            ArmorReinforced   = 8,
            Invulnerable      = 9
        };
    }

    namespace EntityState {
    // not totally sure what these are for....customs offices for one...these are also entityState (drones)
        enum  {
            Offlining             = -7,
            Anchoring             = -6,
            Onlining              = -5,
            Anchored              = -4,
            Unanchoring           = -3,
            Unanchored            = -2,
            Incapacitated         = -1,
            Idle                  = 0,
            Combat                = 1,
            Mining                = 2,
            Approaching           = 3,
            Departing             = 4,
            Departing_2           = 5,
            Pursuit               = 6,
            Fleeing               = 7,
            Reinforced            = 8,
            Operating             = 9,
            Engage                = 10,
            Vulnerable            = 11,
            ShieldReinforce       = 12,
            ArmorReinforce        = 13,
            Invulnerable          = 14,
            WarpAwayAndDie        = 15,
            WarpAwayAndComeCack   = 16,
            WarpToPosition        = 17
        };
    }

    namespace TaxValues {
        // used to determine tax rate on POCO based on relationship with owner
        enum {
            Corp                = 1,
            Alliance            = 2,
            StandingHigh        = 3,
            StandingGood        = 4,
            StandingNeutral     = 5,
            StandingBad         = 6,
            StandingHorrible    = 7,
        };
    }

    namespace StandingValues {
        // if access by standing, these set minimum standing required, and associated tax rate.
        enum {
            StandingHorrible    = -10,
            StandingBad         = -5,
            StandingNeutral     = 0,
            StandingGood        = 5,
            StandingHigh        = 10
        };
    }
/*
 * typedef enum {
 *    posShieldStartLevel = 0.505f,
 *    posMaxShieldPercentageForWatch = 0.95f,
 *    posMinDamageDiffToPersist = 0.05f
 * };
 */
}

#endif  //EVE_POS_ENUMS_H


/*{'FullPath': u'UI/Inflight/MoonMining', 'messageID': 238189, 'label': u'TowerLocated'}(u'Location: <b>{moonLocation}</b>', None, {u'{moonLocation}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moonLocation'}})
 * {'FullPath': u'UI/Inflight/MoonMining', 'messageID': 238190, 'label': u'TowerPowerUsage'}(u'{[numeric]currentValue}/{[numeric]totalPower} MW', None, {u'{[numeric]currentValue}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentValue'}, u'{[numeric]totalPower}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'totalPower'}})
 * {'FullPath': u'UI/Inflight/MoonMining', 'messageID': 238191, 'label': u'TowerCPUUsage'}(u'{[numeric]currentValue}/{[numeric]totalCPU} tf', None, {u'{[numeric]currentValue}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentValue'}, u'{[numeric]totalCPU}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'totalCPU'}})
 */

/*
STRUCTURE_UNANCHORED = 0
STRUCTURE_ANCHORED = 1
STRUCTURE_ONLINING = 2
STRUCTURE_REINFORCED = 3
STRUCTURE_ONLINE = 4
STRUCTURE_OPERATING = 5
STRUCTURE_VULNERABLE = 6
STRUCTURE_SHIELD_REINFORCE = 7
STRUCTURE_ARMOR_REINFORCE = 8
STRUCTURE_INVULNERABLE = 9

 * entities.POS_STRUCTURE_STATE = {const.pwnStructureStateAnchored: localization.GetByLabel('Entities/States/Anchored'),
 * const.pwnStructureStateAnchoring: localization.GetByLabel('Entities/States/Anchoring'),
 * const.pwnStructureStateIncapacitated: localization.GetByLabel('Entities/States/Incapacitated'),
 * const.pwnStructureStateInvulnerable: localization.GetByLabel('Entities/States/Invulnerable'),
 * const.pwnStructureStateOnline: localization.GetByLabel('Entities/States/Online'),
 * const.pwnStructureStateOnlining: localization.GetByLabel('Entities/States/Onlining'),
 * const.pwnStructureStateOperating: localization.GetByLabel('Entities/States/Operating'),
 * const.pwnStructureStateReinforced: localization.GetByLabel('Entities/States/Reinforced'),
 * const.pwnStructureStateUnanchored: localization.GetByLabel('Entities/States/Unanchored'),
 * const.pwnStructureStateUnanchoring: localization.GetByLabel('Entities/States/Unanchoring'),
 * const.pwnStructureStateVulnerable: localization.GetByLabel('Entities/States/Vulnerable'),
 * const.pwnStructureStateAnchor: localization.GetByLabel('UI/Inflight/MoonMining/Structures/Anchor'),
 * const.pwnStructureStateUnanchor: localization.GetByLabel('UI/Inflight/MoonMining/Structures/Unanchor'),
 * const.pwnStructureStateOffline: localization.GetByLabel('UI/Inflight/MoonMining/Structures/Offline'),
 * const.pwnStructureStateOnlineActive: localization.GetByLabel('UI/Inflight/MoonMining/States/OnlineActive'),
 * const.pwnStructureStateOnlineStartingUp: localization.GetByLabel('UI/Inflight/MoonMining/States/OnlineStartingUp')}
 */


/*
STATE_OFFLINING = -7
STATE_ANCHORING = -6
STATE_ONLINING = -5
STATE_ANCHORED = -4
STATE_UNANCHORING = -3
STATE_UNANCHORED = -2
STATE_INCAPACITATED = -1
STATE_IDLE = 0
STATE_COMBAT = 1
STATE_MINING = 2
STATE_APPROACHING = 3
STATE_DEPARTING = 4
STATE_DEPARTING_2 = 5
STATE_PURSUIT = 6
STATE_FLEEING = 7
STATE_REINFORCED = 8
STATE_OPERATING = 9
STATE_ENGAGE = 10
STATE_VULNERABLE = 11
STATE_SHIELD_REINFORCE = 12
STATE_ARMOR_REINFORCE = 13
STATE_INVULNERABLE = 14
STATE_WARPAWAYANDDIE = 15
STATE_WARPAWAYANDCOMEBACK = 16
STATE_WARPTOPOSITION = 17
ENTITY_STATE_NAMES = {STATE_OFFLINING: 'State_OFFLINING',
 STATE_ANCHORING: 'STATE_ANCHORING',
 STATE_ONLINING: 'STATE_ONLINING',
 STATE_ANCHORED: 'STATE_ANCHORED',
 STATE_UNANCHORING: 'STATE_UNANCHORING',
 STATE_UNANCHORED: 'STATE_UNANCHORED',
 STATE_INCAPACITATED: 'STATE_INCAPACITATED',
 STATE_IDLE: 'STATE_IDLE',
 STATE_COMBAT: 'STATE_COMBAT',
 STATE_MINING: 'STATE_MINING',
 STATE_APPROACHING: 'STATE_APPROACHING',
 STATE_DEPARTING: 'STATE_DEPARTING',
 STATE_DEPARTING_2: 'STATE_DEPARTING_2',
 STATE_PURSUIT: 'STATE_PURSUIT',
 STATE_FLEEING: 'STATE_FLEEING',
 STATE_REINFORCED: 'STATE_REINFORCED',
 STATE_OPERATING: 'STATE_OPERATING',
 STATE_ENGAGE: 'STATE_ENGAGE',
 STATE_VULNERABLE: 'STATE_VULNERABLE',
 STATE_SHIELD_REINFORCE: 'STATE_SHIELD_REINFORCE',
 STATE_ARMOR_REINFORCE: 'STATE_ARMOR_REINFORCE',
 STATE_INVULNERABLE: 'STATE_INVULNERABLE',
 STATE_WARPAWAYANDDIE: 'STATE_WARPAWAYANDDIE',
 STATE_WARPAWAYANDCOMEBACK: 'STATE_WARPAWAYANDCOMEBACK',
 STATE_WARPTOPOSITION: 'STATE_WARPTOPOSITION'}
entityStateStrings = {STATE_ANCHORING: localization.GetByLabel('Entities/States/Anchoring'),
 STATE_ONLINING: localization.GetByLabel('Entities/States/Onlining'),
 STATE_ANCHORED: localization.GetByLabel('Entities/States/Anchored'),
 STATE_UNANCHORING: localization.GetByLabel('Entities/States/Unanchoring'),
 STATE_UNANCHORED: localization.GetByLabel('Entities/States/Unanchored'),
 STATE_INCAPACITATED: localization.GetByLabel('Entities/States/Incapacitated'),
 STATE_IDLE: localization.GetByLabel('Entities/States/Idle'),
 STATE_COMBAT: localization.GetByLabel('Entities/States/Fighting'),
 STATE_MINING: localization.GetByLabel('Entities/States/Mining'),
 STATE_APPROACHING: localization.GetByLabel('Entities/States/Approaching'),
 STATE_FLEEING: localization.GetByLabel('Entities/States/Fleeing'),
 STATE_REINFORCED: localization.GetByLabel('Entities/States/Reinforced'),
 STATE_OPERATING: localization.GetByLabel('Entities/States/Operating'),
 STATE_VULNERABLE: localization.GetByLabel('Entities/States/Vulnerable'),
 STATE_INVULNERABLE: localization.GetByLabel('Entities/States/Invulnerable'),
 STATE_SHIELD_REINFORCE: localization.GetByLabel('Entities/States/ShieldReinforced'),
 STATE_ARMOR_REINFORCE: localization.GetByLabel('Entities/States/ArmorReinforced')}

def GetEntityStateString(entityState):
    return entityStateStrings.get(entityState, localization.GetByLabel('Entities/States/Unknown', entityStateID=entityState))


INCAPACITATION_DISTANCE = 250000
COMMAND_DISTANCE = INCAPACITATION_DISTANCE
*/