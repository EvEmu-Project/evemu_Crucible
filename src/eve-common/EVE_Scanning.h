/*
 *
 *
 *
 */

#ifndef EVE_SCANNING_H
#define EVE_SCANNING_H

/*
static float probeResultPerfect     = 1.0f;
static float probeResultInformative = 0.75f;
static float probeResultGood        = 0.25f;
static float probeResultUnusable    = 0.001f;
*/

namespace Scanning {
//  -allan 7Jul14
    namespace Group {
        enum {
            Scrap         = 1,      //wrecks in system (client filter only)
            Signature     = 4,      //advanced anomaly.  need probes to scan
            Ship          = 8,      //abandoned ships
            Structure     = 16,     //all pos structures
            DroneOrProbe  = 32,     //player items
            Celestial     = 64,     //static system entity  (client filter only)
            Anomaly       = 128     //detected using ship sensors
        };
    }
}

namespace Probe {
    namespace State {
        enum {
            Inactive     = 0,
            Idle         = 1,
            Moving       = 2,
            Warping      = 3,
            Scanning     = 4,
            Returning    = 5,
            Waiting      = 6    // internal only
        };
    }
}

/*
 *
    AttrScanRadarStrength = 208,                        //Radar Site (scanning/exploration)
    AttrScanLadarStrength = 209,                        //Ladar Site (scanning/exploration)
    AttrScanMagnetometricStrength = 210,                //Magnetometric Site (scanning/exploration)
    AttrScanGravimetricStrength = 211,                  //Gravimetric Site (scanning/exploration)
    AttrScanAllStrength = 1136,                         //Unknown Site  (scanning/exploration)
    AttrScanGenericStrength = 1169,                     //undefined in client
    *
    *    attributes used/defined in client
    attributeScanAllStrength = 1136
    attributeScanFrequencyResult = 1161
    attributeScanGravimetricStrength = 211
    attributeScanGravimetricStrengthBonus = 238
    attributeScanGravimetricStrengthPercent = 1027
    attributeScanLadarStrength = 209
    attributeScanLadarStrengthBonus = 239
    attributeScanLadarStrengthPercent = 1028
    attributeScanMagnetometricStrength = 210
    attributeScanMagnetometricStrengthBonus = 240
    attributeScanMagnetometricStrengthPercent = 1029
    attributeScanRadarStrength = 208
    attributeScanRadarStrengthBonus = 241
    attributeScanRadarStrengthPercent = 1030
    attributeScanRange = 765
    attributeScanResolution = 564
    attributeScanResolutionBonus = 566
    attributeScanResolutionMultiplier = 565
    attributeScanSpeed = 79


 probeScanGroupScrap = 1
 probeScanGroupSignatures = 4
 probeScanGroupShips = 8
 probeScanGroupStructures = 16
 probeScanGroupDronesAndProbes = 32
 probeScanGroupCelestials = 64
 probeScanGroupAnomalies = 128
 probeScanGroups = {}
 probeScanGroups[probeScanGroupScrap] = set([groupBiomass,
                                            groupCargoContainer,
                                            groupWreck,
                                            groupSecureCargoContainer,
                                            groupAuditLogSecureContainer])
 probeScanGroups[probeScanGroupSignatures] = set([groupCosmicSignature])
 probeScanGroups[probeScanGroupAnomalies] = set([groupCosmicAnomaly])
 probeScanGroups[probeScanGroupShips] = set([groupAssaultShip,
                                            groupBattlecruiser,
                                            groupBattleship,
                                            groupBlackOps,
                                            groupCapitalIndustrialShip,
                                            groupCapsule,
                                            groupCarrier,
                                            groupCombatReconShip,
                                            groupCommandShip,
                                            groupCovertOps,
                                            groupCruiser,
                                            groupDestroyer,
                                            groupDreadnought,
                                            groupElectronicAttackShips,
                                            groupEliteBattleship,
                                            groupExhumer,
                                            groupForceReconShip,
                                            groupFreighter,
                                            groupFrigate,
                                            groupHeavyAssaultShip,
                                            groupHeavyInterdictors,
                                            groupIndustrial,
                                            groupIndustrialCommandShip,
                                            groupInterceptor,
                                            groupInterdictor,
                                            groupJumpFreighter,
                                            groupLogistics,
                                            groupMarauders,
                                            groupMiningBarge,
                                            groupSupercarrier,
                                            groupPrototypeExplorationShip,
                                            groupRookieship,
                                            groupShuttle,
                                            groupStealthBomber,
                                            groupTitan,
                                            groupTransportShip,
                                            groupStrategicCruiser])
 probeScanGroups[probeScanGroupStructures] = set([groupConstructionPlatform,
                                                 groupStationUpgradePlatform,
                                                 groupStationImprovementPlatform,
                                                 groupMobileWarpDisruptor,
                                                 groupAssemblyArray,
                                                 groupControlTower,
                                                 groupCorporateHangarArray,
                                                 groupElectronicWarfareBattery,
                                                 groupEnergyNeutralizingBattery,
                                                 groupForceFieldArray,
                                                 groupJumpPortalArray,
                                                 groupLogisticsArray,
                                                 groupMobileHybridSentry,
                                                 groupMobileLaboratory,
                                                 groupMobileLaserSentry,
                                                 groupMobileMissileSentry,
                                                 groupMobilePowerCore,
                                                 groupMobileProjectileSentry,
                                                 groupMobileReactor,
                                                 groupMobileShieldGenerator,
                                                 groupMobileStorage,
                                                 groupMoonMining,
                                                 groupRefiningArray,
                                                 groupScannerArray,
                                                 groupSensorDampeningBattery,
                                                 groupShieldHardeningArray,
                                                 groupShipMaintenanceArray,
                                                 groupSilo,
                                                 groupStasisWebificationBattery,
                                                 groupStealthEmitterArray,
                                                 groupTrackingArray,
                                                 groupWarpScramblingBattery,
                                                 groupCynosuralSystemJammer,
                                                 groupCynosuralGeneratorArray,
                                                 groupInfrastructureHub,
                                                 groupSovereigntyClaimMarkers,
                                                 groupSovereigntyDisruptionStructures,
                                                 groupOrbitalConstructionPlatforms,
                                                 groupPlanetaryCustomsOffices])
 probeScanGroups[probeScanGroupDronesAndProbes] = set([groupCapDrainDrone,
                                                      groupCombatDrone,
                                                      groupElectronicWarfareDrone,
                                                      groupFighterDrone,
                                                      groupFighterBomber,
                                                      groupLogisticDrone,
                                                      groupMiningDrone,
                                                      groupProximityDrone,
                                                      groupRepairDrone,
                                                      groupStasisWebifyingDrone,
                                                      groupUnanchoringDrone,
                                                      groupWarpScramblingDrone,
                                                      groupScannerProbe,
                                                      groupSurveyProbe,
                                                      groupWarpDisruptionProbe])
 probeScanGroups[probeScanGroupCelestials] = set([groupAsteroidBelt,
                                                 groupForceField,
                                                 groupMoon,
                                                 groupPlanet,
                                                 groupStargate,
                                                 groupSun,
                                                 groupStation])
*/

/* ship scanner result data from client...
 *
 *
EXPLORATION_SITE_TYPES = {attributeScanGravimetricStrength: 'UI/Inflight/Scanner/Gravimetric',
    attributeScanLadarStrength: 'UI/Inflight/Scanner/Ladar',
    attributeScanMagnetometricStrength: 'UI/Inflight/Scanner/Magnetometric',
    attributeScanRadarStrength: 'UI/Inflight/Scanner/Radar',
    attributeScanAllStrength: 'UI/Common/Unknown'}


                if groupID == const.groupCosmicSignature:
                    for signatureType in [const.attributeScanGravimetricStrength,
                     const.attributeScanLadarStrength,
                     const.attributeScanMagnetometricStrength,
                     const.attributeScanRadarStrength,
                     const.attributeScanAllStrength]:
                     ****  if groupID is sig or anom, groupName is based on scanstr.

            result.scanGroupName = self.scanGroups[result.scanGroupID]
        self.scanGroups[const.probeScanGroupAnomalies] = localization.GetByLabel('UI/Inflight/Scanner/CosmicAnomaly')
        self.scanGroups[const.probeScanGroupSignatures] = localization.GetByLabel('UI/Inflight/Scanner/CosmicSignature')
        self.scanGroups[const.probeScanGroupShips] = localization.GetByLabel('UI/Inflight/Scanner/Ship')
        self.scanGroups[const.probeScanGroupStructures] = localization.GetByLabel('UI/Inflight/Scanner/Structure')
        self.scanGroups[const.probeScanGroupDronesAndProbes] = localization.GetByLabel('UI/Inflight/Scanner/DroneAndProbe')
        self.__disallowanalysisgroups = [const.groupSurveyProbe]

                if certainty >= const.probeResultGood: (0.25)
                    explorationSite = result.groupID in (const.groupCosmicAnomaly, const.groupCosmicSignature)
                    if explorationSite:
                        result.groupName = self.GetExplorationSiteType(result.strengthAttributeID)
                    else:
                        result.groupName = cfg.invgroups.Get(result.groupID).name
                    if certainty >= const.probeResultInformative: (0.75)
                        if explorationSite:
                            result.typeName = result.dungeonName
                        else:
                            result.typeName = cfg.invtypes.Get(typeID).name


        **** distance can be fudged....
                if isinstance(result.data, Vector3):
                    dist = (result.data - myPos).Length()
                elif isinstance(result.data, float):
                    dist = result.data
                    certainty = min(0.9999, certainty)
                else:
                    dist = (result.data.point - myPos).Length()
                    certainty = min(0.9999, certainty)


                if result.certainty >= const.probeResultPerfect and isinstance(result.data, Vector3):
                    if result.scanGroupID in (const.probeScanGroupSignatures, const.probeScanGroupAnomalies):
                        self.resultsCached[result.id] = result
 */


#endif  // EVE_SCANNING_H