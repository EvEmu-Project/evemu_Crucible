
/*
 *  EVE_Incursion.h
 *   incursion-specific data
 *
 */

#ifndef EVE_INCURSION_H
#define EVE_INCURSION_H

namespace Incursion {

    namespace scenesType {
        enum {
            headquarters = 1,
            assault = 2,
            vanguard = 3,
            staging = 4,
            testscene = 5,
            boss = 1000001,
            endTale = 1000002,
            testscene1 = 2000001,
            testscene2 = 2000002,
            testscene3 = 2000003,
            testscene4 = 2000004,
            testscene5 = 2000005,
            managerInit = 5000001
        };
    }

    // i dont think these are used in client.
    namespace Parameter { //= collections.namedtuple('Parameter', 'name parameterType defaultValue prettyName description')
        enum {
            dungeonID = 1, //: Parameter('dungeonID', int, 0, 'Dungeon ID', 'The ID of the dungeon to spawn'),
            dungeonListID = 2, //: Parameter('dungeonListID', int, None, 'Dungeon list ID', 'The ID of the list of dungeons to spawn'),
            dungeonRespawnTime = 3, //: Parameter('dungeonRespawnTime', int, 1, 'Dungeon respawn time', 'Dungeon respawn time in minutes'),
            dungeonScanStrength = 4, //: Parameter('dungeonScanStrength', int, 100, 'Dungeon scan strength', 'Dungeon scan strength for scanning down the dungeon'),
            dungeonSignatureRadius = 5, //: Parameter('dungeonSignatureRadius', float, 100.0, 'Dungeon signature radius', 'Dungeon signature radius used for scanning down the dungeon'),
            dungeonScanStrengthAttrib = 6, //: Parameter('dungeonScanStrengthAttrib', float, const.attributeScanGravimetricStrength, 'Dungeon scan attribute', 'Dungeon scan attribute'),
            dungeonSpawnLocation = 7, //: Parameter('dungeonSpawnLocation', float, None, 'Dungeon spawn location', 'The locations in space where the dungeon is going to respawn'),
            dungeonSpawnQuantity = 8, //: Parameter('dungeonSpawnQuantity', int, 1, 'Number of Dungeons', 'The number of dungeons which have to be spawned'),
            triggeredScene = 9, //: Parameter('triggeredScene', int, None, 'Triggered Scene', 'The scene which is added to the trigger location when activated'),
            triggeredSceneLocation = 10, //: Parameter('triggeredSceneLocation', int, None, 'Trigger Location', 'The location the triggered scene is added when the trigger is activated'),
            solarSystemSecurityMin = 11, //: Parameter('solarSystemSecurityMin', float, 1.0, 'Security minimum', 'The security level of the solar system has to be above this before the condition is true'),
            solarSystemSecurityMax = 12, //: Parameter('solarSystemSecurityMax', float, 0.0, 'Security maximum', 'The security level of the solar system has to be below this before the condition is true'),
            solarSystemSecurityMinInclusive = 13, //: Parameter('solarSystemSecurityMinInclusive', bool, True, 'Security minimum inclusive', 'This is whether the minimum should be inclusive or exclusive'),
            solarSystemSecurityMinInclusive = 14, //: Parameter('solarSystemSecurityMinInclusive', bool, False, 'Security maximum inclusive', 'This is whether the maximum should be inclusive or exclusive'),
            disableConvoyDjinn = 15, //: Parameter('disableConvoyDjinn', bool, False, 'Disable convoy djinn', 'Disables the convoy djinn during the tale'),
            disableCustomsPoliceDjinn = 16, //: Parameter('disableCustomsPoliceDjinn', bool, False, 'Disable custom police djinn', 'Disables the custom police during the tale'),
            disableEmpirePoliceDjinn = 17, //: Parameter('disableEmpirePoliceDjinn', bool, False, 'Disable empire police djinn', 'Disables the empire police during the tale'),
            disableMilitaryFactionDjinn = 18, //: Parameter('disableMilitaryFactionDjinn', bool, False, 'Disable military faction djinn', 'Disables the military faction djinn during the tale'),
            disablePirateDjinn = 19, //: Parameter('disablePirateDjinn', bool, False, 'Disable pirate djinn', 'Disables the pirate djinn during the tale'),
            disablePirateAutoDjinn = 20, //: Parameter('disablePirateAutoDjinn', bool, False, 'Disable pirate auto djinn', 'Disables the pirate auto djinn during the tale'),
            disablePirateStargateDjinn = 21, //: Parameter('disablePirateStargateDjinn', bool, False, 'Disable pirate stargate djinn', 'Disables the pirate Stargate djinn during the tale'),
            djinnCommandID = 22, //: Parameter('djinnCommandID', int, 0, 'Djinn command ID', 'The djinn command ID in this is added to solar system the scene is running in'),
            systemEffectBeaconTypeID = 23, //: Parameter('systemEffectBeaconTypeID', int, 0, 'System effect beacon type ID', 'The type ID of the systems effect beacon'),
            systemEffectBeaconBlockCynosural = 24, //: Parameter('systemEffectBeaconBlockCynosural', bool, False, 'System effect beacon blocks cyno', 'The system effect beacon will also block cynosural jump'),
            systemInfluenceTriggerDirection = 25, //: Parameter('systemInfluenceTriggerDirection', int, const.systemInfluenceAny, 'Trigger direction', 'What direction the influence should change before the trigger is triggered'),
            systemInfluenceTriggerValue = 26, //: Parameter('systemInfluenceTriggerValue', float, 0.0, 'Trigger value', 'The value around which the trigger should be triggered'),
            dummyParameter = 27, //: Parameter('dummyParameter', float, 0.0, 'Dummy Parameter', 'This is a dummy parameter for actions that take no parameters'),
            surchargeRate = 28 //: Parameter('surchargeRate', float, 0.2, 'Surcharge Rate', 'This is the surcharge rate that will be applied to this system')}
        };
    }
}


#endif  // EVE_INCURSION_H