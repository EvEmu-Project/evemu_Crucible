/*
 *
 *
 *
 */


#ifndef EVE_SKILLS_H
#define EVE_SKILLS_H

namespace EvESkill {
    namespace Event {
        /**  these are the only skill events that client will recognize....there are no messageIDs for others
                 34: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillClonePenalty')
                 36: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingStarted')
                 37: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingComplete')
                 38: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingCanceled')
                 39: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/GMGiveSkill')
                 53: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingComplete')
                307: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillPointsApplied')
        */
        enum {                                    // client msg in skill history
            Gift                          = 24,   //Unknown
            CharCreation                  = 33,   //Unknown
            ClonePenalty                  = 34,   //Skill Clone Penalty
            TaskMaster                    = 35,   //Unknown (used to denote training correction)
            TrainingStarted               = 36,   //Skill Training Started
            TrainingComplete              = 37,   //Skill Training Complete
            TrainingCanceled              = 38,   //Skill Training Canceled
            GMGift                        = 39,   //GM Skill Gift
            QueueTrainingCompleted        = 53,   //Skill Training Complete
            SkillInjected                 = 56,   //Unknown
            Removal                       = 177,  //Unknown
            HaltedAccountLapsed           = 260,  //Unknown
            SkillPointsApplied            = 307,  //Apply Free Skill Points
            GMReverseFreeSkillPointsUsed  = 309   //Unknown
        };
    }

    /*
     'OnSkillStartTraining',
     'OnSkillTrainingStopped',
     'OnSkillTrained',  reloads skill window, if open, and reloads char info
     //  if 'old notifications' is set in client, this will show a list of skills trained, else neocom blink
     'OnMultipleSkillsTrained',
     'OnSkillTrainingSaved',    reloads skill window, if open
     'OnSkillInjected',    reloads skill window, if open
     // this one is used to switch training from completed skill to next in queue
     'OnSkillSwitched',  def OnSkillSwitched(self, oldSkillID, newSkillID, ETA):   client scatters skillTrained and training started when this event is received
     */

 /** Created with MYSQL query:
  * SELECT concat("\tskill", Replace(t.typeName, ' ', ''), "\t\t= ", t.typeID, ',\t\t// group = ', g.groupName)
  * FROM invTypes t, invGroups g, invCategories c
  * WHERE g.groupID = t.groupID AND c.categoryID = g.categoryID AND c.categoryID = 16
  * ORDER BY g.groupName
  */
 //  -allan 21Mar14
  enum {  //thanks positron96 for this query

      //  ***** NOTE:  items marked with '//*' are not published  (incomplete)

// Corporation Management
     CorporationManagement      = 3363,
     StationManagement      = 3364,
     StarbaseManagement     = 3365,
     FactoryManagement      = 3366,
     RefineryManagement     = 3367,
     EthnicRelations        = 3368,
     CFOTraining        = 3369,
     ChiefScienceOfficer        = 3370,
     PublicRelations        = 3371,
     IntelligenceAnalyst        = 3372,
     StarbaseDefenseManagement      = 3373,
     MegacorpManagement     = 3731,
     EmpireControl      = 3732,
     Anchoring      = 11584,
     Sovereignty        = 12241,
 // Drones
     Drones     = 3436,
     ScoutDroneOperation        = 3437,
     MiningDroneOperation       = 3438,
     RepairDroneOperation       = 3439,
     SalvageDroneOperation      = 3440,
     HeavyDroneOperation        = 3441,
     DroneInterfacing       = 3442,
     DroneNavigation        = 12305,
     AmarrDroneSpecialization       = 12484,
     MinmatarDroneSpecialization        = 12485,
     GallenteDroneSpecialization        = 12486,
     CaldariDroneSpecialization     = 12487,
     TESTDroneSkill     = 22172,
     MiningDroneSpecialization      = 22541,
     Fighters       = 23069,
     ElectronicWarfareDroneInterfacing      = 23566,
     SentryDroneInterfacing     = 23594,
     PropulsionJammingDroneInterfacing      = 23599,
     DroneSharpshooting     = 23606,
     DroneDurability        = 23618,
     CombatDroneOperation       = 24241,
     AdvancedDroneInterfacing       = 24613,
     FighterBombers     = 32339,
 // Electronics
     Electronics        = 3426,
     ElectronicWarfare      = 3427,
     LongRangeTargeting     = 3428,
     Targeting      = 3429,
     Multitasking       = 3430,
     SignatureAnalysis      = 3431,
     ElectronicsUpgrades        = 3432,
     SensorLinking      = 3433,
     WeaponDisruption       = 3434,
     PropulsionJamming      = 3435,
     Survey     = 3551,
     AdvancedSensorUpgrades     = 11208,
     Cloaking       = 11579,
     HypereuclideanNavigation       = 12368,
     LongDistanceJamming        = 19759,
     FrequencyModulation        = 19760,
     SignalDispersion       = 19761,
     SignalSuppression      = 19766,
     TurretDestabilization      = 19767,
     TargetPainting     = 19921,
     SignatureFocusing      = 19922,
     CynosuralFieldTheory       = 21603,
     ProjectedElectronicCounterMeasures     = 27911,
     TournamentObservation      = 28604,
     ImperialNavySecurityClearance      = 28631,
 // Engineering
     Engineering        = 3413,
     ShieldOperation        = 3416,
     EnergySystemsOperation     = 3417,
     EnergyManagement       = 3418,
     ShieldManagement       = 3419,
     TacticalShieldManipulation     = 3420,
     EnergyPulseWeapons     = 3421,
     ShieldEmissionSystems      = 3422,
     EnergyEmissionSystems      = 3423,
     EnergyGridUpgrades     = 3424,
     ShieldUpgrades     = 3425,
     AdvancedEnergyGridUpgrades     = 11204,
     AdvancedShieldUpgrades     = 11206,
     ThermicShieldCompensation      = 11566,
     EMShieldCompensation       = 12365,
     KineticShieldCompensation      = 12366,
     ExplosiveShieldCompensation        = 12367,
     ShieldCompensation     = 21059,
     CapitalShieldOperation     = 21802,
     CapitalShieldEmissionSystems       = 24571,
     CapitalEnergyEmissionSystems       = 24572,
 // Fake Skills
     StealthBombersFakeSkill        = 20127,
 // Gunnery
     Gunnery        = 3300,
     SmallHybridTurret      = 3301,
     SmallProjectileTurret      = 3302,
     SmallEnergyTurret      = 3303,
     MediumHybridTurret     = 3304,
     MediumProjectileTurret     = 3305,
     MediumEnergyTurret     = 3306,
     LargeHybridTurret      = 3307,
     LargeProjectileTurret      = 3308,
     LargeEnergyTurret      = 3309,
     RapidFiring        = 3310,
     Sharpshooter       = 3311,
     MotionPrediction       = 3312,
     SurgicalStrike     = 3315,
     ControlledBursts       = 3316,
     TrajectoryAnalysis     = 3317,
     WeaponUpgrades     = 3318,
     SmallRailgunSpecialization     = 11082,
     SmallBeamLaserSpecialization       = 11083,
     SmallAutocannonSpecialization      = 11084,
     AdvancedWeaponUpgrades     = 11207,
     SmallArtillerySpecialization       = 12201,
     MediumArtillerySpecialization      = 12202,
     LargeArtillerySpecialization       = 12203,
     MediumBeamLaserSpecialization      = 12204,
     LargeBeamLaserSpecialization       = 12205,
     MediumRailgunSpecialization        = 12206,
     LargeRailgunSpecialization     = 12207,
     MediumAutocannonSpecialization     = 12208,
     LargeAutocannonSpecialization      = 12209,
     SmallBlasterSpecialization     = 12210,
     MediumBlasterSpecialization        = 12211,
     LargeBlasterSpecialization     = 12212,
     SmallPulseLaserSpecialization      = 12213,
     MediumPulseLaserSpecialization     = 12214,
     LargePulseLaserSpecialization      = 12215,
     CapitalEnergyTurret        = 20327,
     CapitalHybridTurret        = 21666,
     CapitalProjectileTurret        = 21667,
     TacticalWeaponReconfiguration      = 22043,
 // Industry
     Industry       = 3380,
     AmarrTech      = 3381,
     CaldariTech        = 3382,
     GallenteTech       = 3383,
     MinmatarTech       = 3384,
     Refining       = 3385,
     Mining     = 3386,
     MassProduction     = 3387,
     ProductionEfficiency       = 3388,
     RefineryEfficiency     = 3389,
     MobileRefineryOperation        = 3390,
     MobileFactoryOperation     = 3391,
     DeepCoreMining     = 11395,
     ArkonorProcessing      = 12180,
     BistotProcessing       = 12181,
     CrokiteProcessing      = 12182,
     DarkOchreProcessing        = 12183,
     GneissProcessing       = 12184,
     HedbergiteProcessing       = 12185,
     HemorphiteProcessing       = 12186,
     JaspetProcessing       = 12187,
     KerniteProcessing      = 12188,
     MercoxitProcessing     = 12189,
     OmberProcessing        = 12190,
     PlagioclaseProcessing      = 12191,
     PyroxeresProcessing        = 12192,
     ScorditeProcessing     = 12193,
     SpodumainProcessing        = 12194,
     VeldsparProcessing     = 12195,
     ScrapmetalProcessing       = 12196,
     IceHarvesting      = 16281,
     IceProcessing      = 18025,
     MiningUpgrades     = 22578,
     SupplyChainManagement      = 24268,
     AdvancedMassProduction     = 24625,
     GasCloudHarvesting     = 25544,
     DrugManufacturing      = 26224,
     OreCompression     = 28373,
     IndustrialReconfiguration      = 28585,
 // Leadership
     Leadership     = 3348,
     SkirmishWarfare        = 3349,
     SiegeWarfare       = 3350,
     SiegeWarfareSpecialist     = 3351,
     InformationWarfareSpecialist       = 3352,
     WarfareLinkSpecialist      = 3354,
     ArmoredWarfareSpecialist       = 11569,
     SkirmishWarfareSpecialist      = 11572,
     WingCommand        = 11574,
     ArmoredWarfare     = 20494,
     InformationWarfare     = 20495,
     MiningForeman      = 22536,
     MiningDirector     = 22552,
     FleetCommand       = 24764,
 // Mechanics
     Mechanics      = 3392,
     RepairSystems      = 3393,
     HullUpgrades       = 3394,
     FrigateConstruction        = 3395,
     IndustrialConstruction     = 3396,
     CruiserConstruction        = 3397,
     BattleshipConstruction     = 3398,
     OutpostConstruction        = 3400,
     RemoteArmorRepairSystems       = 16069,
     CapitalRepairSystems       = 21803,
     CapitalShipConstruction        = 22242,
     EMArmorCompensation        = 22806,
     ExplosiveArmorCompensation     = 22807,
     KineticArmorCompensation       = 22808,
     ThermicArmorCompensation       = 22809,
     CapitalRemoteArmorRepairSystems        = 24568,
     Salvaging      = 25863,
     JuryRigging        = 26252,
     ArmorRigging       = 26253,
     AstronauticsRigging        = 26254,
     DronesRigging      = 26255,
     ElectronicSuperiorityRigging       = 26256,
     ProjectileWeaponRigging        = 26257,
     EnergyWeaponRigging        = 26258,
     HybridWeaponRigging        = 26259,
     LauncherRigging        = 26260,
     ShieldRigging      = 26261,
     RemoteHullRepairSystems        = 27902,
     TacticalLogisticsReconfiguration       = 27906,
     CapitalRemoteHullRepairSystems     = 27936,
     NaniteOperation        = 28879,
     NaniteInterfacing      = 28880,
 // Missile Launcher Operation
     MissileLauncherOperation       = 3319,
     Rockets        = 3320,
     LightMissiles       = 3321,
     FoFMissiles        = 3322,
     DefenderMissiles       = 3323,
     HeavyMissiles      = 3324,
     Torpedoes      = 3325,
     CruiseMissiles     = 3326,
     MissileBombardment     = 12441,
     MissileProjection      = 12442,
     RocketSpecialization       = 20209,
     LightMissileSpecialization      = 20210,
     HeavyMissileSpecialization     = 20211,
     CruiseMissileSpecialization        = 20212,
     TorpedoSpecialization      = 20213,
     GuidedMissilePrecision     = 20312,
     TargetNavigationPrediction     = 20314,
     WarheadUpgrades        = 20315,
     RapidLaunch        = 21071,
     CitadelTorpedoes       = 21668,
     HeavyAssaultMissileSpecialization      = 25718,
     HeavyAssaultMissiles       = 25719,
     BombDeployment     = 28073,
     CitadelCruiseMissiles      = 32435,
 // Navigation
     Navigation     = 3449,
     Afterburner        = 3450,
     FuelConservation       = 3451,
     AccelerationControl        = 3452,
     EvasiveManeuvering     = 3453,
     HighSpeedManeuvering       = 3454,
     WarpDriveOperation     = 3455,
     JumpDriveOperation     = 3456,
     JumpFuelConservation       = 21610,
     JumpDriveCalibration       = 21611,
 // Planet Management
     AdvancedPlanetology        = 2403,
     Planetology        = 2406,
     InterplanetaryConsolidation        = 2495,
     CommandCenterUpgrades      = 2505,
     RemoteSensing      = 13279,
 // Science
     Science        = 3402,
     Research       = 3403,
     Biology        = 3405,
     LaboratoryOperation        = 3406,
     ReverseEngineering     = 3408,
     Metallurgy     = 3409,
     Astrogeology       = 3410,
     Cybernetics        = 3411,
     Astrometrics       = 3412,
     HighEnergyPhysics      = 11433,
     PlasmaPhysics      = 11441,
     NaniteEngineering      = 11442,
     HydromagneticPhysics       = 11443,
     AmarrianStarshipEngineering        = 11444,
     MinmatarStarshipEngineering        = 11445,
     GravitonPhysics        = 11446,
     LaserPhysics       = 11447,
     ElectromagneticPhysics     = 11448,
     RocketScience      = 11449,
     GallenteanStarshipEngineering      = 11450,
     NuclearPhysics     = 11451,
     MechanicalEngineering      = 11452,
     ElectronicEngineering      = 11453,
     CaldariStarshipEngineering     = 11454,
     QuantumPhysics     = 11455,
     AstronauticEngineering     = 11487,
     MolecularEngineering       = 11529,
     HypernetScience        = 11858,
     ResearchProjectManagement      = 12179,
     Archaeology        = 13278,
     TalocanTechnology      = 20433,
     Hacking        = 21718,
     SleeperTechnology      = 21789,
     CaldariEncryptionMethods       = 21790,
     MinmatarEncryptionMethods      = 21791,
     AmarrEncryptionMethods     = 23087,
     GallenteEncryptionMethods      = 23121,
     TakmahlTechnology      = 23123,
     YanJungTechnology      = 23124,
     InfomorphPsychology        = 24242,
     ScientificNetworking       = 24270,
     JumpPortalGeneration       = 24562,
     DoomsdayOperation      = 24563,
     CloningFacilityOperation       = 24606,
     AdvancedLaboratoryOperation        = 24624,
     NeurotoxinRecovery     = 25530,
     NaniteControl      = 25538,
     AstrometricRangefinding        = 25739,
     AstrometricPinpointing     = 25810,
     AstrometricAcquisition     = 25811,
     Thermodynamics     = 28164,
     DefensiveSubsystemTechnology       = 30324,
     EngineeringSubsystemTechnology     = 30325,
     ElectronicSubsystemTechnology      = 30326,
     OffensiveSubsystemTechnology       = 30327,
     PropulsionSubsystemTechnology      = 30788,
 // Social
     Social     = 3355,
     Negotiation        = 3356,
     Diplomacy      = 3357,
     FastTalk       = 3358,
     Connections        = 3359,
     CriminalConnections        = 3361,
     DEDConnections     = 3362,
     MiningConnections      = 3893,
     DistributionConnections        = 3894,
     SecurityConnections        = 3895,
 // Spaceship Command
     OREIndustrial      = 3184,
     SpaceshipCommand       = 3327,
     GallenteFrigate        = 3328,
     MinmatarFrigate        = 3329,
     CaldariFrigate     = 3330,
     AmarrFrigate       = 3331,
     GallenteCruiser        = 3332,
     MinmatarCruiser        = 3333,
     CaldariCruiser     = 3334,
     AmarrCruiser       = 3335,
     GallenteBattleship     = 3336,
     MinmatarBattleship     = 3337,
     CaldariBattleship      = 3338,
     AmarrBattleship        = 3339,
     GallenteIndustrial     = 3340,
     MinmatarIndustrial     = 3341,
     CaldariIndustrial      = 3342,
     AmarrIndustrial        = 3343,
     GallenteTitan      = 3344,
     MinmatarTitan      = 3345,
     CaldariTitan       = 3346,
     AmarrTitan     = 3347,
     JoveFrigate        = 3755,
     JoveCruiser        = 3758,
     Polaris        = 9955,
     Concord        = 10264,
     JoveIndustrial     = 11075,
     JoveBattleship     = 11078,
     Interceptors       = 12092,
     CovertOps      = 12093,
     AssaultShips       = 12095,
     Logistics      = 12096,
     Destroyers     = 12097,
     Interdictors       = 12098,
     Battlecruisers     = 12099,
     HeavyAssaultShips      = 16591,
     MiningBarge        = 17940,
     Omnipotent     = 19430,
     TransportShips     = 19719,
     AdvancedSpaceshipCommand       = 20342,
     AmarrFreighter     = 20524,
     AmarrDreadnought       = 20525,
     CaldariFreighter       = 20526,
     GallenteFreighter      = 20527,
     MinmatarFreighter      = 20528,
     CaldariDreadnought     = 20530,
     GallenteDreadnought        = 20531,
     MinmatarDreadnought        = 20532,
     CapitalShips       = 20533,
     Exhumers       = 22551,
     ReconShips     = 22761,
     CommandShips       = 23950,
     AmarrCarrier       = 24311,
     CaldariCarrier     = 24312,
     GallenteCarrier        = 24313,
     MinmatarCarrier        = 24314,
     CapitalIndustrialShips     = 28374,
     HeavyInterdictors      = 28609,
     ElectronicAttackShips      = 28615,
     BlackOps       = 28656,
     Marauders      = 28667,
     JumpFreighters     = 29029,
     IndustrialCommandShips     = 29637,
     AmarrStrategicCruiser      = 30650,
     CaldariStrategicCruiser        = 30651,
     GallenteStrategicCruiser       = 30652,
     MinmatarStrategicCruiser       = 30653,
 // Subsystems
     AmarrDefensiveSystems      = 30532,
     AmarrElectronicSystems     = 30536,
     AmarrOffensiveSystems      = 30537,
     AmarrPropulsionSystems     = 30538,
     AmarrEngineeringSystems        = 30539,
     GallenteDefensiveSystems       = 30540,
     GallenteElectronicSystems      = 30541,
     CaldariElectronicSystems       = 30542,
     MinmatarElectronicSystems      = 30543,
     CaldariDefensiveSystems        = 30544,
     MinmatarDefensiveSystems       = 30545,
     GallenteEngineeringSystems     = 30546,
     MinmatarEngineeringSystems     = 30547,
     CaldariEngineeringSystems      = 30548,
     CaldariOffensiveSystems        = 30549,
     GallenteOffensiveSystems       = 30550,
     MinmatarOffensiveSystems       = 30551,
     CaldariPropulsionSystems       = 30552,
     GallentePropulsionSystems      = 30553,
     MinmatarPropulsionSystems      = 30554,
 // Trade
     Trade      = 3443,
     Retail     = 3444,
     BlackMarketTrading     = 3445,//*
     BrokerRelations        = 3446,
     Visibility     = 3447,
     Smuggling      = 3448,     //*
     Test       = 11015,//*
     GeneralFreight     = 12834,//*
     StarshipFreight        = 13069,//*
     MineralFreight     = 13070,//*
     MunitionsFreight       = 13071,//*
     DroneFreight       = 13072,//*
     RawMaterialFreight     = 13073,//*
     ConsumableFreight      = 13074,//*
     HazardousMaterialFreight       = 13075,//*
     Procurement        = 16594,
     Daytrading     = 16595,
     Wholesale      = 16596,
     MarginTrading      = 16597,
     Marketing      = 16598,
     Accounting     = 16622,
     Tycoon     = 18580,
     CorporationContracting     = 25233,
     Contracting        = 25235,
     TaxEvasion     = 28261
 };

 /* skills not published...
  skillStationManagement = 3364, // group = Corporation Management
  skillStarbaseManagement = 3365, // group = Corporation Management
  skillFactoryManagement = 3366, // group = Corporation Management
  skillRefineryManagement = 3367, // group = Corporation Management
  skillCFOTraining = 3369, // group = Corporation Management
  skillChiefScienceOfficer = 3370, // group = Corporation Management
  skillPublicRelations = 3371, // group = Corporation Management
  skillIntelligenceAnalyst = 3372, // group = Corporation Management
  skillSalvageDroneOperation = 3440, // group = Drones
  skillTESTDroneSkill = 22172, // group = Drones
  skillMiningDroneSpecialization = 22541, // group = Drones
  skillPropulsionJammingDroneInterfacing = 23599, // group = Drones
  skillAdvancedSensorUpgrades = 11208, // group = Electronics
  skillHypereuclideanNavigation = 12368, // group = Electronics
  skillTournamentObservation = 28604, // group = Electronics
  skillImperialNavySecurityClearance = 28631, // group = Electronics
  skillAdvancedEnergyGridUpgrades = 11204, // group = Engineering
  skillAdvancedShieldUpgrades = 11206, // group = Engineering
  skillStealthBombersFakeSkill = 20127, // group = Fake Skills
  skillAmarrTech = 3381, // group = Industry
  skillCaldariTech = 3382, // group = Industry
  skillGallenteTech = 3383, // group = Industry
  skillMinmatarTech = 3384, // group = Industry
  skillMobileRefineryOperation = 3390, // group = Industry
  skillMobileFactoryOperation = 3391, // group = Industry
  skillOreCompression = 28373, // group = Industry
  skillHypernetScience = 11858, // group = Science
  skillJoveFrigate = 3755, // group = Spaceship Command
  skillJoveCruiser = 3758, // group = Spaceship Command
  skillPolaris = 9955, // group = Spaceship Command
  skillConcord = 10264, // group = Spaceship Command
  skillJoveIndustrial = 11075, // group = Spaceship Command
  skillJoveBattleship = 11078, // group = Spaceship Command
  skillOmnipotent = 19430, // group = Spaceship Command
  skillBlackMarketTrading = 3445, // group = Trade
  skillSmuggling = 3448, // group = Trade
  skillTest = 11015, // group = Trade
  skillStarshipFreight = 13069, // group = Trade
  skillMineralFreight = 13070, // group = Trade
  skillMunitionsFreight = 13071, // group = Trade
  skillDroneFreight = 13072, // group = Trade
  skillRawMaterialFreight = 13073, // group = Trade
  skillConsumableFreight = 13074, // group = Trade
  skillHazardousMaterialFreight = 13075, // group = Trade
  */

}
 #endif  //EVE_SKILLS_H