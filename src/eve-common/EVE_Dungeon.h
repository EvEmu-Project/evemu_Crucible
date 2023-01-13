/*
 *  EVE_Dungeon.h
 *   dungeon-specific enumerators
 *
 */

#ifndef EVE_DUNGEON_H
#define EVE_DUNGEON_H

namespace Dungeon {
    namespace Type {
        enum {
            Mission         = 1, // npc mission
            Gravimetric     = 2, // roids
            Magnetometric   = 3, // salvage and archeology
            Radar           = 4, // hacking
            Ladar           = 5, // gas mining
            Wormhole        = 6, // wtf is a 'wormhole'??
            Anomaly         = 7, // non-rated dungeon that isnt required to scan with probes
            Unrated         = 8, // non-rated dungeon  no waves, possible escalation to complex
            Escalation      = 9, // new dungeon from previous site. very limited access
            Rated           = 10 // DED rated dungeon
        };
    }

    namespace Spawn {
        enum {
            Belts           = 0,
            Gate            = 1,
            Near            = 2,
            Deep            = 3,
            Reinforcments   = 4,
            Stations        = 5,
            Faction         = 6,
            Concord         = 7
        };
    }
    namespace Status {
        enum {
            Started         = 0,
            Completed       = 1,
            Failed          = 2
        };
    }

    namespace Event {
        namespace Msg {
            enum {
                ImminentDanger      = 1,
                NPC                 = 2,
                Environment         = 3,
                Mood                = 4,
                Story               = 5,
                MissionObjective    = 6,
                MissionInstruction  = 7,
                Warning             = 8
            };
        }

        namespace Trigger {
            enum {
                ActivateGate = 1,
                SpawnGuards = 2,
                SpawnGuardObject = 3,
                RangedNPCHealing = 4,
                RangedPlayerDamageEM = 5,
                RangedPlayerDamageExplosive = 6,
                RangedPlayerDamageKinetic = 7,
                RangedPlayerDamageThermal = 8,
                MissionCompletion = 9,
                Msg = 10,
                DungeonCompletion = 11,
                EffectBeaconActivate = 13,
                EffectBeaconDeactivate = 14,
                ObjectDespawn = 15,
                ObjectExplode = 16,
                EntityDespawn = 18,
                EntityExplode = 19,
                FacWarVictoryPointsGranted = 20,
                AgentTalkTo = 22,
                AgentMsg = 23,
                DropLoot = 24,
                RangedPlayerHealing = 25,
                RangedNPCDamageEM = 26,
                RangedNPCDamageExplosive = 27,
                RangedNPCDamageKinetic = 28,
                RangedNPCDamageThermal = 29,
                SpawnItemInCargo = 30,
                MissionFailure = 31,
                CounterAdd = 32,
                CounterSubtract = 33,
                CounterMultiply = 34,
                CounterDivide = 35,
                CounterSet = 36,
                GrantGroupReward = 37,
                GrantDelayedGroupReward = 38,
                AdjustSystemInfluence = 39,
                WarpShipAwayDespawn = 40,
                WarpShipAwayAndComeBack = 41,
                SupressAllRespawn = 42,
                GrantGroupRewardLimitedRestrictions = 45,
                OpenTutorial = 46,
                SpawnShip = 47
            };
        }
    }

    namespace Trigger {
        enum {
            Attacked = 1,
            ShipEnteredProximity = 2,
            Exploding = 3,
            ShieldConditionLevel = 4,
            ArmorConditionLevel = 5,
            StructureConditionLevel = 6,
            Mined = 7,
            RoomEntered = 8,
            RoomMinedOut = 9,
            RoomMined = 10,
            HackingSuccess = 11,
            HackingFailure = 12,
            SalvagingSuccess = 13,
            SalvagingFailure = 14,
            ArchaeologySuccess = 15,
            ArchaeologyFailure = 16,
            ShipsEnteredRoom = 17,
            RoomCapturedCorp = 18,
            RoomCapturedAlliance = 19,
            RoomCapturedFacWar = 20,
            FWShipEnteredProximity = 21,
            ItemPlacedInMissionContainer = 23,
            PlayerKilled = 26,
            EffectActivated = 27,
            ShipsLeftRoom = 28,
            ShipLeftProximity = 29,
            FWShipLeftProximity = 30,
            RoomWipedOut = 31,
            ItemRemovedFromSpawnContainer = 32,
            ItemInCargo = 33,
            CounterEQ = 34,
            CounterGT = 35,
            CounterGE = 36,
            CounterLT = 37,
            CounterLE = 38,
            FacWarLoyaltyPointsGranted = 48
        };
    }
    /* POD structure entries for dungeon data */
    struct Template {
        uint8 dunTypeID;
        uint8 dunSpawnClass;
        uint16 dunEntryID;
        int32 dunRoomID;
        std::string dunName;
        std::string dunDescription;
        uint32 dunFactionID;
    };

    /* Tags for dungeon multi-index container */
    struct DungeonsByID {};
    struct DungeonsByArchetype {};

    struct RoomObject {
        uint32 objectID;
        uint32 roomID;
        uint16 typeID;
        uint16 groupID;
        double x;
        double y;
        double z;
        double yaw;
        double pitch;
        double roll;
        double radius;
    };

    struct Room {
        uint16 roomID;
        std::string roomName;
        std::vector<RoomObject> objects;
        int16 x;
        int16 y;
        int16 z;
    };

    struct Dungeon {
        uint32 dungeonID;
        std::string name;
        uint8 status;
        uint32 factionID;
        uint8 archetypeID;
        std::map<uint16, Room> rooms;
    };
}

/*
dunArchetypeAgentMissionDungeon = 20
dunArchetypeFacwarDefensive = 32
dunArchetypeFacwarOffensive = 35
dunArchetypeFacwarDungeons = (dunArchetypeFacwarDefensive, dunArchetypeFacwarOffensive)
dunArchetypeWormhole = 38
dunArchetypeZTest = 19
dunExpirationDelay = 48
dungeonGateUnlockPeriod = 66
DUNGEON_EVENT_TYPE_AFFECTS_ENTITY = [dunTriggerEventEntityExplode,
 dunTriggerEventEntityDespawn,
 dunTriggerEventSpawnGuards,
 dunTriggerEventWarpShipAwayDespawn,
 dunTriggerEventWarpShipAwayAndComeBack]
DUNGEON_EVENT_TYPE_AFFECTS_OBJECT = [dunTriggerEventSpawnGuardObject,
 dunTriggerEventEffectBeaconActivate,
 dunTriggerEventEffectBeaconDeactivate,
 dunTriggerEventObjectExplode,
 dunTriggerEventObjectDespawn,
 dunTriggerEventActivateGate]
DUNGEON_ORIGIN_UNDEFINED = None
DUNGEON_ORIGIN_STATIC = 1
DUNGEON_ORIGIN_AGENT = 2
DUNGEON_ORIGIN_PLAYTEST = 3
DUNGEON_ORIGIN_EDIT = 4
DUNGEON_ORIGIN_DISTRIBUTION = 5
DUNGEON_ORIGIN_PATH = 6
DUNGEON_ORIGIN_TUTORIAL = 7
*/

#endif  // EVE_DUNGEON_H


/*{'FullPath': u'UI/Agents/Dialogue', 'messageID': 233757, 'label': u'DungeonShipRestrictionsListShipIsRestricted'}(u'This site is restricted to certain ship types; <font color=red>your {[item]shipTypeID.name} will not be able to access it.</font> You must use one of the following types of ship:<br><br>{shipList}', None, {u'{[item]shipTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'shipTypeID'}, u'{shipList}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipList'}})
 * {'FullPath': u'UI/Agents/Dialogue', 'messageID': 233758, 'label': u'DungeonShipRestrictionsListShipIsNotRestricted'}(u'This site is restricted to certain ship types; you may use your {[item]shipTypeID.name} to access it, or one of the following types of ship:<br><br>{shipList}', None, {u'{[item]shipTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'shipTypeID'}, u'{shipList}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipList'}})
 * {'FullPath': u'UI/Agents/Dialogue', 'messageID': 233759, 'label': u'DungeonShipRestrictionShipIsNotRestricted'}(u'This site is restricted to the following ship type: {groupName}<br/><br />You may use your {[item]typeID.name} to access it.', None, {u'{[item]typeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'typeID'}, u'{groupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'groupName'}})
 * {'FullPath': u'UI/Agents/Dialogue', 'messageID': 233760, 'label': u'DungeonShipRestrictionsShowList'}(u'This site is restricted to certain ship types. It can only be accessed by flying one of the following:<br/><br/>{shipList}', None, {u'{shipList}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipList'}})
 * {'FullPath': u'UI/Agents/Dialogue', 'messageID': 233761, 'label': u'DungeonShipRestrictionsHeader'}(u'Ships permitted', None, None)
 */