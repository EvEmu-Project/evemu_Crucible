/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
*/


#ifndef EVE_PACKET_TYPES_H
#define EVE_PACKET_TYPES_H

enum MACHONETMSG_TYPE
{
    AUTHENTICATION_REQ                = 0,
    AUTHENTICATION_RSP                = 1,
    IDENTIFICATION_REQ                = 2,
    IDENTIFICATION_RSP                = 3,
    __Fake_Invalid_Type                = 4,
    CALL_REQ                        = 6,
    CALL_RSP                        = 7,
    TRANSPORTCLOSED                    = 8,
    RESOLVE_REQ                        = 10,
    RESOLVE_RSP                        = 11,
    NOTIFICATION                    = 12,
    ERRORRESPONSE                    = 15,
    SESSIONCHANGENOTIFICATION        = 16,
    SESSIONINITIALSTATENOTIFICATION    = 18,
    PING_REQ                        = 20,
    PING_RSP                        = 21,
    MOVEMENTNOTIFICATION            = 100,
    MACHONETMSG_TYPE_COUNT
};

enum MACHONETERR_TYPE
{
    UNMACHODESTINATION = 0,
    UNMACHOCHANNEL = 1,
    WRAPPEDEXCEPTION = 2
};
//see PyPacket.cpp
extern const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT];

//item types which we need to know about:
enum EVEItemType {
    AllianceTypeID = 16159
};

//these came from the 'constants' object:
enum EVEItemChangeType {
    ixItemID = 0,    //also ixLauncherCapacity?
    ixTypeID = 1,    //also ixLauncherUsed = 1,
    ixOwnerID = 2,    //also ixLauncherChargeItem?
    ixLocationID = 3,
    ixFlag = 4,
    ixContraband = 5,
    ixSingleton = 6,
    ixGroupID = 8,
    ixQuantity = 7,
    ixCategoryID = 9,
    ixCustomInfo = 10,
    ixSubitems = 11
};

enum EVEContainerTypes {
    containerWallet             = 10001,
    containerGlobal             = 10002,
    containerSolarSystem         = 10003,
    containerHangar             = 10004,
    containerScrapHeap            = 10005,
    containerFactory            = 10006,
    containerBank                 = 10007,
    containerRecycler            = 10008,
    containerOffices            = 10009,
    containerStationCharacters    = 10010,
    containerCharacter             = 10011,
    containerCorpMarket            = 10012
};

enum EVERookieShipTypes {
    amarrRookie                        = 596,
    caldariRookie                    = 601,
    gallenteRookie                    = 606,
    minmatarRookie                    = 588,
};


#include "tables/invCategories.h"
typedef EVEDB::invCategories::invCategories EVEItemCategories;

//from invFlags DB table
typedef enum EVEItemFlags
{
    flagAutoFit                        = 0,
    flagWallet                        = 1,
    flagFactory                        = 2,
    flagHangar                        = 4,
    flagCargoHold                    = 5,
    flagBriefcase                    = 6,
    flagSkill                        = 7,
    flagReward                        = 8,
    flagConnected                    = 9,    //Character in station connected
    flagDisconnected                = 10,    //Character in station offline

    //ship fittings:
    flagLowSlot0                    = 11,    //Low power slot 1
    flagLowSlot1                    = 12,
    flagLowSlot2                    = 13,
    flagLowSlot3                    = 14,
    flagLowSlot4                    = 15,
    flagLowSlot5                    = 16,
    flagLowSlot6                    = 17,
    flagLowSlot7                    = 18,    //Low power slot 8

    flagMedSlot0                    = 19,    //Medium power slot 1
    flagMedSlot1                    = 20,
    flagMedSlot2                    = 21,
    flagMedSlot3                    = 22,
    flagMedSlot4                    = 23,
    flagMedSlot5                    = 24,
    flagMedSlot6                    = 25,
    flagMedSlot7                    = 26,    //Medium power slot 8

    flagHiSlot0                        = 27,    //High power slot 1
    flagHiSlot1                        = 28,
    flagHiSlot2                        = 29,
    flagHiSlot3                        = 30,
    flagHiSlot4                        = 31,
    flagHiSlot5                        = 32,
    flagHiSlot6                        = 33,
    flagHiSlot7                        = 34,    //High power slot 8
    flagFixedSlot                    = 35,

    //factory stuff:
    flagFactoryBlueprint            = 36,
    flagFactoryMinerals                = 37,
    flagFactoryOutput                = 38,
    flagFactoryActive                = 39,
    flagFactory_SlotFirst            = 40,
    flagFactory_SlotLast            = 55,

    flagCapsule                        = 56,    //Capsule item in space
    flagPilot                        = 57,
    flagPassenger                    = 58,
    flagBoardingGate                = 59,
    flagCrew                        = 60,
    flagSkillInTraining                = 61,
    flagCorpMarket                    = 62,    //Corporation Market Deliveries / Returns
    flagLocked                        = 63,    //Locked item, can not be moved unless unlocked
    flagUnlocked                    = 64,

    flagOfficeSlotFirst                = 70,
    flagOfficeSlotLast                = 85,

    flagBonus                        = 86,    //Char bonus/penalty
    flagDroneBay                    = 87,
    flagBooster                        = 88,
    flagImplant                        = 89,
    flagShipHangar                    = 90,
    flagShipOffline                    = 91,

    flagRigSlot0                    = 92,    //Rig power slot 1
    flagRigSlot1                    = 93,    //Rig power slot 2
    flagRigSlot2                    = 94,    //Rig power slot 3
    flagRigSlot3                    = 95,    //Rig power slot 4
    flagRigSlot4                    = 96,    //Rig power slot 5
    flagRigSlot5                    = 97,    //Rig power slot 6
    flagRigSlot6                    = 98,    //Rig power slot 7
    flagRigSlot7                    = 99,    //Rig power slot 8

    flagFactoryOperation            = 100,

    flagCorpSecurityAccessGroup2    = 116,
    flagCorpSecurityAccessGroup3    = 117,
    flagCorpSecurityAccessGroup4    = 118,
    flagCorpSecurityAccessGroup5    = 119,
    flagCorpSecurityAccessGroup6    = 120,
    flagCorpSecurityAccessGroup7    = 121,

    flagSecondaryStorage            = 122,    //Secondary Storage
    flagCaptainsQuarters            = 123,    //Captains Quarters
    flagWisPromenade                = 124,    //Wis Promenade

    flagSubSystem0                    = 125,    //Sub system slot 0
    flagSubSystem1                    = 126,    //Sub system slot 1
    flagSubSystem2                    = 127,    //Sub system slot 2
    flagSubSystem3                    = 128,    //Sub system slot 3
    flagSubSystem4                    = 129,    //Sub system slot 4
    flagSubSystem5                    = 130,    //Sub system slot 5
    flagSubSystem6                    = 131,    //Sub system slot 6
    flagSubSystem7                    = 132,    //Sub system slot 7

    flagSpecializedFuelBay          = 133,
    flagSpecializedOreHold          = 134,
    flagSpecializedGasHold          = 135,
    flagSpecializedMineralHold      = 136,
    flagSpecializedSalvageHold      = 137,
    flagSpecializedShipHold         = 138,
    flagSpecializedSmallShipHold    = 139,

    flagFactorySlotFirst            = 140,
    //flagSpecializedMediumShipHold   = 140,

    flagSpecializedLargeShipHold    = 141,
    flagSpecializedIndustrialShipHold = 142,
    flagSpecializedAmmoHold         = 143,
    flagStructureActive             = 144,
    flagStructureInactive           = 145,
    flagJunkyardReprocessed         = 146,
    flagJunkyardTrashed             = 147,
    flagSpecializedCommandCenterHold = 148,
    flagSpecializedPlanetaryCommoditiesHold = 149,

    flagFactorySlotLast                = 195,

    flagResearchFacilitySlotFirst    = 200,
    flagResearchFacilitySlotLast    = 255,

    flagClone                        = 400,

	flagIllegal						= 9999
} EVEItemFlags;

//for use in the new module manager
typedef enum EVEItemSlotType
{
    NaT                                = 0,
    slotTypeSubSystem                = 1,
    slotTypeRig                        = 2,
    slotTypeLowPower                = 3,
    slotTypeMedPower                = 4,
    slotTypeHiPower                    = 5

} EVEItemSlotType;

//some alternative names for entries above.
static const EVEItemFlags flagSlotFirst = flagLowSlot0;    //duplicate values
static const EVEItemFlags flagSlotLast = flagFixedSlot;
static const EVEItemFlags flagNone = flagAutoFit;

static const EVEItemFlags flagAnywhere = flagAutoFit;
static const uint8 MAX_MODULE_COUNT = flagSlotLast - flagSlotFirst + 1;
static const uint8 MAX_HIGH_SLOT_COUNT = flagHiSlot7 - flagHiSlot0 + 1;
static const uint8 MAX_MEDIUM_SLOT_COUNT = flagMedSlot7 - flagMedSlot0 + 1;
static const uint8 MAX_LOW_SLOT_COUNT = flagLowSlot7 - flagLowSlot0 + 1;
static const uint8 MAX_RIG_COUNT = flagRigSlot7 - flagRigSlot0 + 1;
static const uint8 MAX_ASSEMBLY_COUNT = flagSubSystem7 - flagSubSystem0 + 1;

#define FlagToSlot(flag) \
    (flag - flagSlotFirst)
#define SlotToFlag(slot) \
    ((EVEItemFlags)(flagSlotFirst + slot))

//  -allan 21Mar14
typedef enum {  //thanks positron96 for this query
    skillCorporationManagement      = 3363,     // group = Corporation Management
    skillStationManagement      = 3364,     // group = Corporation Management
    skillStarbaseManagement     = 3365,     // group = Corporation Management
    skillFactoryManagement      = 3366,     // group = Corporation Management
    skillRefineryManagement     = 3367,     // group = Corporation Management
    skillEthnicRelations        = 3368,     // group = Corporation Management
    skillCFOTraining        = 3369,     // group = Corporation Management
    skillChiefScienceOfficer        = 3370,     // group = Corporation Management
    skillPublicRelations        = 3371,     // group = Corporation Management
    skillIntelligenceAnalyst        = 3372,     // group = Corporation Management
    skillStarbaseDefenseManagement      = 3373,     // group = Corporation Management
    skillMegacorpManagement     = 3731,     // group = Corporation Management
    skillEmpireControl      = 3732,     // group = Corporation Management
    skillAnchoring      = 11584,        // group = Corporation Management
    skillSovereignty        = 12241,        // group = Corporation Management
    skillDrones     = 3436,     // group = Drones
    skillScoutDroneOperation        = 3437,     // group = Drones
    skillMiningDroneOperation       = 3438,     // group = Drones
    skillRepairDroneOperation       = 3439,     // group = Drones
    skillSalvageDroneOperation      = 3440,     // group = Drones
    skillHeavyDroneOperation        = 3441,     // group = Drones
    skillDroneInterfacing       = 3442,     // group = Drones
    skillDroneNavigation        = 12305,        // group = Drones
    skillAmarrDroneSpecialization       = 12484,        // group = Drones
    skillMinmatarDroneSpecialization        = 12485,        // group = Drones
    skillGallenteDroneSpecialization        = 12486,        // group = Drones
    skillCaldariDroneSpecialization     = 12487,        // group = Drones
    skillTESTDroneSkill     = 22172,        // group = Drones
    skillMiningDroneSpecialization      = 22541,        // group = Drones
    skillFighters       = 23069,        // group = Drones
    skillElectronicWarfareDroneInterfacing      = 23566,        // group = Drones
    skillSentryDroneInterfacing     = 23594,        // group = Drones
    skillPropulsionJammingDroneInterfacing      = 23599,        // group = Drones
    skillDroneSharpshooting     = 23606,        // group = Drones
    skillDroneDurability        = 23618,        // group = Drones
    skillCombatDroneOperation       = 24241,        // group = Drones
    skillAdvancedDroneInterfacing       = 24613,        // group = Drones
    skillFighterBombers     = 32339,        // group = Drones
    skillElectronics        = 3426,     // group = Electronics
    skillElectronicWarfare      = 3427,     // group = Electronics
    skillLongRangeTargeting     = 3428,     // group = Electronics
    skillTargeting      = 3429,     // group = Electronics
    skillMultitasking       = 3430,     // group = Electronics
    skillSignatureAnalysis      = 3431,     // group = Electronics
    skillElectronicsUpgrades        = 3432,     // group = Electronics
    skillSensorLinking      = 3433,     // group = Electronics
    skillWeaponDisruption       = 3434,     // group = Electronics
    skillPropulsionJamming      = 3435,     // group = Electronics
    skillSurvey     = 3551,     // group = Electronics
    skillAdvancedSensorUpgrades     = 11208,        // group = Electronics
    skillCloaking       = 11579,        // group = Electronics
    skillHypereuclideanNavigation       = 12368,        // group = Electronics
    skillLongDistanceJamming        = 19759,        // group = Electronics
    skillFrequencyModulation        = 19760,        // group = Electronics
    skillSignalDispersion       = 19761,        // group = Electronics
    skillSignalSuppression      = 19766,        // group = Electronics
    skillTurretDestabilization      = 19767,        // group = Electronics
    skillTargetPainting     = 19921,        // group = Electronics
    skillSignatureFocusing      = 19922,        // group = Electronics
    skillCynosuralFieldTheory       = 21603,        // group = Electronics
    skillProjectedElectronicCounterMeasures     = 27911,        // group = Electronics
    skillTournamentObservation      = 28604,        // group = Electronics
    skillImperialNavySecurityClearance      = 28631,        // group = Electronics
    skillEngineering        = 3413,     // group = Engineering
    skillShieldOperation        = 3416,     // group = Engineering
    skillEnergySystemsOperation     = 3417,     // group = Engineering
    skillEnergyManagement       = 3418,     // group = Engineering
    skillShieldManagement       = 3419,     // group = Engineering
    skillTacticalShieldManipulation     = 3420,     // group = Engineering
    skillEnergyPulseWeapons     = 3421,     // group = Engineering
    skillShieldEmissionSystems      = 3422,     // group = Engineering
    skillEnergyEmissionSystems      = 3423,     // group = Engineering
    skillEnergyGridUpgrades     = 3424,     // group = Engineering
    skillShieldUpgrades     = 3425,     // group = Engineering
    skillAdvancedEnergyGridUpgrades     = 11204,        // group = Engineering
    skillAdvancedShieldUpgrades     = 11206,        // group = Engineering
    skillThermicShieldCompensation      = 11566,        // group = Engineering
    skillEMShieldCompensation       = 12365,        // group = Engineering
    skillKineticShieldCompensation      = 12366,        // group = Engineering
    skillExplosiveShieldCompensation        = 12367,        // group = Engineering
    skillShieldCompensation     = 21059,        // group = Engineering
    skillCapitalShieldOperation     = 21802,        // group = Engineering
    skillCapitalShieldEmissionSystems       = 24571,        // group = Engineering
    skillCapitalEnergyEmissionSystems       = 24572,        // group = Engineering
    skillStealthBombersFakeSkill        = 20127,        // group = Fake Skills
    skillGunnery        = 3300,     // group = Gunnery
    skillSmallHybridTurret      = 3301,     // group = Gunnery
    skillSmallProjectileTurret      = 3302,     // group = Gunnery
    skillSmallEnergyTurret      = 3303,     // group = Gunnery
    skillMediumHybridTurret     = 3304,     // group = Gunnery
    skillMediumProjectileTurret     = 3305,     // group = Gunnery
    skillMediumEnergyTurret     = 3306,     // group = Gunnery
    skillLargeHybridTurret      = 3307,     // group = Gunnery
    skillLargeProjectileTurret      = 3308,     // group = Gunnery
    skillLargeEnergyTurret      = 3309,     // group = Gunnery
    skillRapidFiring        = 3310,     // group = Gunnery
    skillSharpshooter       = 3311,     // group = Gunnery
    skillMotionPrediction       = 3312,     // group = Gunnery
    skillSurgicalStrike     = 3315,     // group = Gunnery
    skillControlledBursts       = 3316,     // group = Gunnery
    skillTrajectoryAnalysis     = 3317,     // group = Gunnery
    skillWeaponUpgrades     = 3318,     // group = Gunnery
    skillSmallRailgunSpecialization     = 11082,        // group = Gunnery
    skillSmallBeamLaserSpecialization       = 11083,        // group = Gunnery
    skillSmallAutocannonSpecialization      = 11084,        // group = Gunnery
    skillAdvancedWeaponUpgrades     = 11207,        // group = Gunnery
    skillSmallArtillerySpecialization       = 12201,        // group = Gunnery
    skillMediumArtillerySpecialization      = 12202,        // group = Gunnery
    skillLargeArtillerySpecialization       = 12203,        // group = Gunnery
    skillMediumBeamLaserSpecialization      = 12204,        // group = Gunnery
    skillLargeBeamLaserSpecialization       = 12205,        // group = Gunnery
    skillMediumRailgunSpecialization        = 12206,        // group = Gunnery
    skillLargeRailgunSpecialization     = 12207,        // group = Gunnery
    skillMediumAutocannonSpecialization     = 12208,        // group = Gunnery
    skillLargeAutocannonSpecialization      = 12209,        // group = Gunnery
    skillSmallBlasterSpecialization     = 12210,        // group = Gunnery
    skillMediumBlasterSpecialization        = 12211,        // group = Gunnery
    skillLargeBlasterSpecialization     = 12212,        // group = Gunnery
    skillSmallPulseLaserSpecialization      = 12213,        // group = Gunnery
    skillMediumPulseLaserSpecialization     = 12214,        // group = Gunnery
    skillLargePulseLaserSpecialization      = 12215,        // group = Gunnery
    skillCapitalEnergyTurret        = 20327,        // group = Gunnery
    skillCapitalHybridTurret        = 21666,        // group = Gunnery
    skillCapitalProjectileTurret        = 21667,        // group = Gunnery
    skillTacticalWeaponReconfiguration      = 22043,        // group = Gunnery
    skillIndustry       = 3380,     // group = Industry
    skillAmarrTech      = 3381,     // group = Industry
    skillCaldariTech        = 3382,     // group = Industry
    skillGallenteTech       = 3383,     // group = Industry
    skillMinmatarTech       = 3384,     // group = Industry
    skillRefining       = 3385,     // group = Industry
    skillMining     = 3386,     // group = Industry
    skillMassProduction     = 3387,     // group = Industry
    skillProductionEfficiency       = 3388,     // group = Industry
    skillRefineryEfficiency     = 3389,     // group = Industry
    skillMobileRefineryOperation        = 3390,     // group = Industry
    skillMobileFactoryOperation     = 3391,     // group = Industry
    skillDeepCoreMining     = 11395,        // group = Industry
    skillArkonorProcessing      = 12180,        // group = Industry
    skillBistotProcessing       = 12181,        // group = Industry
    skillCrokiteProcessing      = 12182,        // group = Industry
    skillDarkOchreProcessing        = 12183,        // group = Industry
    skillGneissProcessing       = 12184,        // group = Industry
    skillHedbergiteProcessing       = 12185,        // group = Industry
    skillHemorphiteProcessing       = 12186,        // group = Industry
    skillJaspetProcessing       = 12187,        // group = Industry
    skillKerniteProcessing      = 12188,        // group = Industry
    skillMercoxitProcessing     = 12189,        // group = Industry
    skillOmberProcessing        = 12190,        // group = Industry
    skillPlagioclaseProcessing      = 12191,        // group = Industry
    skillPyroxeresProcessing        = 12192,        // group = Industry
    skillScorditeProcessing     = 12193,        // group = Industry
    skillSpodumainProcessing        = 12194,        // group = Industry
    skillVeldsparProcessing     = 12195,        // group = Industry
    skillScrapmetalProcessing       = 12196,        // group = Industry
    skillIceHarvesting      = 16281,        // group = Industry
    skillIceProcessing      = 18025,        // group = Industry
    skillMiningUpgrades     = 22578,        // group = Industry
    skillSupplyChainManagement      = 24268,        // group = Industry
    skillAdvancedMassProduction     = 24625,        // group = Industry
    skillGasCloudHarvesting     = 25544,        // group = Industry
    skillDrugManufacturing      = 26224,        // group = Industry
    skillOreCompression     = 28373,        // group = Industry
    skillIndustrialReconfiguration      = 28585,        // group = Industry
    skillLeadership     = 3348,     // group = Leadership
    skillSkirmishWarfare        = 3349,     // group = Leadership
    skillSiegeWarfare       = 3350,     // group = Leadership
    skillSiegeWarfareSpecialist     = 3351,     // group = Leadership
    skillInformationWarfareSpecialist       = 3352,     // group = Leadership
    skillWarfareLinkSpecialist      = 3354,     // group = Leadership
    skillArmoredWarfareSpecialist       = 11569,        // group = Leadership
    skillSkirmishWarfareSpecialist      = 11572,        // group = Leadership
    skillWingCommand        = 11574,        // group = Leadership
    skillArmoredWarfare     = 20494,        // group = Leadership
    skillInformationWarfare     = 20495,        // group = Leadership
    skillMiningForeman      = 22536,        // group = Leadership
    skillMiningDirector     = 22552,        // group = Leadership
    skillFleetCommand       = 24764,        // group = Leadership
    skillMechanics      = 3392,     // group = Mechanics
    skillRepairSystems      = 3393,     // group = Mechanics
    skillHullUpgrades       = 3394,     // group = Mechanics
    skillFrigateConstruction        = 3395,     // group = Mechanics
    skillIndustrialConstruction     = 3396,     // group = Mechanics
    skillCruiserConstruction        = 3397,     // group = Mechanics
    skillBattleshipConstruction     = 3398,     // group = Mechanics
    skillOutpostConstruction        = 3400,     // group = Mechanics
    skillRemoteArmorRepairSystems       = 16069,        // group = Mechanics
    skillCapitalRepairSystems       = 21803,        // group = Mechanics
    skillCapitalShipConstruction        = 22242,        // group = Mechanics
    skillEMArmorCompensation        = 22806,        // group = Mechanics
    skillExplosiveArmorCompensation     = 22807,        // group = Mechanics
    skillKineticArmorCompensation       = 22808,        // group = Mechanics
    skillThermicArmorCompensation       = 22809,        // group = Mechanics
    skillCapitalRemoteArmorRepairSystems        = 24568,        // group = Mechanics
    skillSalvaging      = 25863,        // group = Mechanics
    skillJuryRigging        = 26252,        // group = Mechanics
    skillArmorRigging       = 26253,        // group = Mechanics
    skillAstronauticsRigging        = 26254,        // group = Mechanics
    skillDronesRigging      = 26255,        // group = Mechanics
    skillElectronicSuperiorityRigging       = 26256,        // group = Mechanics
    skillProjectileWeaponRigging        = 26257,        // group = Mechanics
    skillEnergyWeaponRigging        = 26258,        // group = Mechanics
    skillHybridWeaponRigging        = 26259,        // group = Mechanics
    skillLauncherRigging        = 26260,        // group = Mechanics
    skillShieldRigging      = 26261,        // group = Mechanics
    skillRemoteHullRepairSystems        = 27902,        // group = Mechanics
    skillTacticalLogisticsReconfiguration       = 27906,        // group = Mechanics
    skillCapitalRemoteHullRepairSystems     = 27936,        // group = Mechanics
    skillNaniteOperation        = 28879,        // group = Mechanics
    skillNaniteInterfacing      = 28880,        // group = Mechanics
    skillMissileLauncherOperation       = 3319,     // group = Missile Launcher Operation
    skillRockets        = 3320,     // group = Missile Launcher Operation
    skillStandardMissiles       = 3321,     // group = Missile Launcher Operation
    skillFoFMissiles        = 3322,     // group = Missile Launcher Operation
    skillDefenderMissiles       = 3323,     // group = Missile Launcher Operation
    skillHeavyMissiles      = 3324,     // group = Missile Launcher Operation
    skillTorpedoes      = 3325,     // group = Missile Launcher Operation
    skillCruiseMissiles     = 3326,     // group = Missile Launcher Operation
    skillMissileBombardment     = 12441,        // group = Missile Launcher Operation
    skillMissileProjection      = 12442,        // group = Missile Launcher Operation
    skillRocketSpecialization       = 20209,        // group = Missile Launcher Operation
    skillStandardMissileSpecialization      = 20210,        // group = Missile Launcher Operation
    skillHeavyMissileSpecialization     = 20211,        // group = Missile Launcher Operation
    skillCruiseMissileSpecialization        = 20212,        // group = Missile Launcher Operation
    skillTorpedoSpecialization      = 20213,        // group = Missile Launcher Operation
    skillGuidedMissilePrecision     = 20312,        // group = Missile Launcher Operation
    skillTargetNavigationPrediction     = 20314,        // group = Missile Launcher Operation
    skillWarheadUpgrades        = 20315,        // group = Missile Launcher Operation
    skillRapidLaunch        = 21071,        // group = Missile Launcher Operation
    skillCitadelTorpedoes       = 21668,        // group = Missile Launcher Operation
    skillHeavyAssaultMissileSpecialization      = 25718,        // group = Missile Launcher Operation
    skillHeavyAssaultMissiles       = 25719,        // group = Missile Launcher Operation
    skillBombDeployment     = 28073,        // group = Missile Launcher Operation
    skillCitadelCruiseMissiles      = 32435,        // group = Missile Launcher Operation
    skillNavigation     = 3449,     // group = Navigation
    skillAfterburner        = 3450,     // group = Navigation
    skillFuelConservation       = 3451,     // group = Navigation
    skillAccelerationControl        = 3452,     // group = Navigation
    skillEvasiveManeuvering     = 3453,     // group = Navigation
    skillHighSpeedManeuvering       = 3454,     // group = Navigation
    skillWarpDriveOperation     = 3455,     // group = Navigation
    skillJumpDriveOperation     = 3456,     // group = Navigation
    skillJumpFuelConservation       = 21610,        // group = Navigation
    skillJumpDriveCalibration       = 21611,        // group = Navigation
    skillAdvancedPlanetology        = 2403,     // group = Planet Management
    skillPlanetology        = 2406,     // group = Planet Management
    skillInterplanetaryConsolidation        = 2495,     // group = Planet Management
    skillCommandCenterUpgrades      = 2505,     // group = Planet Management
    skillRemoteSensing      = 13279,        // group = Planet Management
    skillScience        = 3402,     // group = Science
    skillResearch       = 3403,     // group = Science
    skillBiology        = 3405,     // group = Science
    skillLaboratoryOperation        = 3406,     // group = Science
    skillReverseEngineering     = 3408,     // group = Science
    skillMetallurgy     = 3409,     // group = Science
    skillAstrogeology       = 3410,     // group = Science
    skillCybernetics        = 3411,     // group = Science
    skillAstrometrics       = 3412,     // group = Science
    skillHighEnergyPhysics      = 11433,        // group = Science
    skillPlasmaPhysics      = 11441,        // group = Science
    skillNaniteEngineering      = 11442,        // group = Science
    skillHydromagneticPhysics       = 11443,        // group = Science
    skillAmarrianStarshipEngineering        = 11444,        // group = Science
    skillMinmatarStarshipEngineering        = 11445,        // group = Science
    skillGravitonPhysics        = 11446,        // group = Science
    skillLaserPhysics       = 11447,        // group = Science
    skillElectromagneticPhysics     = 11448,        // group = Science
    skillRocketScience      = 11449,        // group = Science
    skillGallenteanStarshipEngineering      = 11450,        // group = Science
    skillNuclearPhysics     = 11451,        // group = Science
    skillMechanicalEngineering      = 11452,        // group = Science
    skillElectronicEngineering      = 11453,        // group = Science
    skillCaldariStarshipEngineering     = 11454,        // group = Science
    skillQuantumPhysics     = 11455,        // group = Science
    skillAstronauticEngineering     = 11487,        // group = Science
    skillMolecularEngineering       = 11529,        // group = Science
    skillHypernetScience        = 11858,        // group = Science
    skillResearchProjectManagement      = 12179,        // group = Science
    skillArchaeology        = 13278,        // group = Science
    skillTalocanTechnology      = 20433,        // group = Science
    skillHacking        = 21718,        // group = Science
    skillSleeperTechnology      = 21789,        // group = Science
    skillCaldariEncryptionMethods       = 21790,        // group = Science
    skillMinmatarEncryptionMethods      = 21791,        // group = Science
    skillAmarrEncryptionMethods     = 23087,        // group = Science
    skillGallenteEncryptionMethods      = 23121,        // group = Science
    skillTakmahlTechnology      = 23123,        // group = Science
    skillYanJungTechnology      = 23124,        // group = Science
    skillInfomorphPsychology        = 24242,        // group = Science
    skillScientificNetworking       = 24270,        // group = Science
    skillJumpPortalGeneration       = 24562,        // group = Science
    skillDoomsdayOperation      = 24563,        // group = Science
    skillCloningFacilityOperation       = 24606,        // group = Science
    skillAdvancedLaboratoryOperation        = 24624,        // group = Science
    skillNeurotoxinRecovery     = 25530,        // group = Science
    skillNaniteControl      = 25538,        // group = Science
    skillAstrometricRangefinding        = 25739,        // group = Science
    skillAstrometricPinpointing     = 25810,        // group = Science
    skillAstrometricAcquisition     = 25811,        // group = Science
    skillThermodynamics     = 28164,        // group = Science
    skillDefensiveSubsystemTechnology       = 30324,        // group = Science
    skillEngineeringSubsystemTechnology     = 30325,        // group = Science
    skillElectronicSubsystemTechnology      = 30326,        // group = Science
    skillOffensiveSubsystemTechnology       = 30327,        // group = Science
    skillPropulsionSubsystemTechnology      = 30788,        // group = Science
    skillSocial     = 3355,     // group = Social
    skillNegotiation        = 3356,     // group = Social
    skillDiplomacy      = 3357,     // group = Social
    skillFastTalk       = 3358,     // group = Social
    skillConnections        = 3359,     // group = Social
    skillCriminalConnections        = 3361,     // group = Social
    skillDEDConnections     = 3362,     // group = Social
    skillMiningConnections      = 3893,     // group = Social
    skillDistributionConnections        = 3894,     // group = Social
    skillSecurityConnections        = 3895,     // group = Social
    skillOREIndustrial      = 3184,     // group = Spaceship Command
    skillSpaceshipCommand       = 3327,     // group = Spaceship Command
    skillGallenteFrigate        = 3328,     // group = Spaceship Command
    skillMinmatarFrigate        = 3329,     // group = Spaceship Command
    skillCaldariFrigate     = 3330,     // group = Spaceship Command
    skillAmarrFrigate       = 3331,     // group = Spaceship Command
    skillGallenteCruiser        = 3332,     // group = Spaceship Command
    skillMinmatarCruiser        = 3333,     // group = Spaceship Command
    skillCaldariCruiser     = 3334,     // group = Spaceship Command
    skillAmarrCruiser       = 3335,     // group = Spaceship Command
    skillGallenteBattleship     = 3336,     // group = Spaceship Command
    skillMinmatarBattleship     = 3337,     // group = Spaceship Command
    skillCaldariBattleship      = 3338,     // group = Spaceship Command
    skillAmarrBattleship        = 3339,     // group = Spaceship Command
    skillGallenteIndustrial     = 3340,     // group = Spaceship Command
    skillMinmatarIndustrial     = 3341,     // group = Spaceship Command
    skillCaldariIndustrial      = 3342,     // group = Spaceship Command
    skillAmarrIndustrial        = 3343,     // group = Spaceship Command
    skillGallenteTitan      = 3344,     // group = Spaceship Command
    skillMinmatarTitan      = 3345,     // group = Spaceship Command
    skillCaldariTitan       = 3346,     // group = Spaceship Command
    skillAmarrTitan     = 3347,     // group = Spaceship Command
    skillJoveFrigate        = 3755,     // group = Spaceship Command
    skillJoveCruiser        = 3758,     // group = Spaceship Command
    skillPolaris        = 9955,     // group = Spaceship Command
    skillConcord        = 10264,        // group = Spaceship Command
    skillJoveIndustrial     = 11075,        // group = Spaceship Command
    skillJoveBattleship     = 11078,        // group = Spaceship Command
    skillInterceptors       = 12092,        // group = Spaceship Command
    skillCovertOps      = 12093,        // group = Spaceship Command
    skillAssaultShips       = 12095,        // group = Spaceship Command
    skillLogistics      = 12096,        // group = Spaceship Command
    skillDestroyers     = 12097,        // group = Spaceship Command
    skillInterdictors       = 12098,        // group = Spaceship Command
    skillBattlecruisers     = 12099,        // group = Spaceship Command
    skillHeavyAssaultShips      = 16591,        // group = Spaceship Command
    skillMiningBarge        = 17940,        // group = Spaceship Command
    skillOmnipotent     = 19430,        // group = Spaceship Command
    skillTransportShips     = 19719,        // group = Spaceship Command
    skillAdvancedSpaceshipCommand       = 20342,        // group = Spaceship Command
    skillAmarrFreighter     = 20524,        // group = Spaceship Command
    skillAmarrDreadnought       = 20525,        // group = Spaceship Command
    skillCaldariFreighter       = 20526,        // group = Spaceship Command
    skillGallenteFreighter      = 20527,        // group = Spaceship Command
    skillMinmatarFreighter      = 20528,        // group = Spaceship Command
    skillCaldariDreadnought     = 20530,        // group = Spaceship Command
    skillGallenteDreadnought        = 20531,        // group = Spaceship Command
    skillMinmatarDreadnought        = 20532,        // group = Spaceship Command
    skillCapitalShips       = 20533,        // group = Spaceship Command
    skillExhumers       = 22551,        // group = Spaceship Command
    skillReconShips     = 22761,        // group = Spaceship Command
    skillCommandShips       = 23950,        // group = Spaceship Command
    skillAmarrCarrier       = 24311,        // group = Spaceship Command
    skillCaldariCarrier     = 24312,        // group = Spaceship Command
    skillGallenteCarrier        = 24313,        // group = Spaceship Command
    skillMinmatarCarrier        = 24314,        // group = Spaceship Command
    skillCapitalIndustrialShips     = 28374,        // group = Spaceship Command
    skillHeavyInterdictors      = 28609,        // group = Spaceship Command
    skillElectronicAttackShips      = 28615,        // group = Spaceship Command
    skillBlackOps       = 28656,        // group = Spaceship Command
    skillMarauders      = 28667,        // group = Spaceship Command
    skillJumpFreighters     = 29029,        // group = Spaceship Command
    skillIndustrialCommandShips     = 29637,        // group = Spaceship Command
    skillAmarrStrategicCruiser      = 30650,        // group = Spaceship Command
    skillCaldariStrategicCruiser        = 30651,        // group = Spaceship Command
    skillGallenteStrategicCruiser       = 30652,        // group = Spaceship Command
    skillMinmatarStrategicCruiser       = 30653,        // group = Spaceship Command
    skillAmarrDefensiveSystems      = 30532,        // group = Subsystems
    skillAmarrElectronicSystems     = 30536,        // group = Subsystems
    skillAmarrOffensiveSystems      = 30537,        // group = Subsystems
    skillAmarrPropulsionSystems     = 30538,        // group = Subsystems
    skillAmarrEngineeringSystems        = 30539,        // group = Subsystems
    skillGallenteDefensiveSystems       = 30540,        // group = Subsystems
    skillGallenteElectronicSystems      = 30541,        // group = Subsystems
    skillCaldariElectronicSystems       = 30542,        // group = Subsystems
    skillMinmatarElectronicSystems      = 30543,        // group = Subsystems
    skillCaldariDefensiveSystems        = 30544,        // group = Subsystems
    skillMinmatarDefensiveSystems       = 30545,        // group = Subsystems
    skillGallenteEngineeringSystems     = 30546,        // group = Subsystems
    skillMinmatarEngineeringSystems     = 30547,        // group = Subsystems
    skillCaldariEngineeringSystems      = 30548,        // group = Subsystems
    skillCaldariOffensiveSystems        = 30549,        // group = Subsystems
    skillGallenteOffensiveSystems       = 30550,        // group = Subsystems
    skillMinmatarOffensiveSystems       = 30551,        // group = Subsystems
    skillCaldariPropulsionSystems       = 30552,        // group = Subsystems
    skillGallentePropulsionSystems      = 30553,        // group = Subsystems
    skillMinmatarPropulsionSystems      = 30554,        // group = Subsystems
    skillTrade      = 3443,     // group = Trade
    skillRetail     = 3444,     // group = Trade
    skillBlackMarketTrading     = 3445,     // group = Trade
    skillBrokerRelations        = 3446,     // group = Trade
    skillVisibility     = 3447,     // group = Trade
    skillSmuggling      = 3448,     // group = Trade
    skillTest       = 11015,        // group = Trade
    skillGeneralFreight     = 12834,        // group = Trade
    skillStarshipFreight        = 13069,        // group = Trade
    skillMineralFreight     = 13070,        // group = Trade
    skillMunitionsFreight       = 13071,        // group = Trade
    skillDroneFreight       = 13072,        // group = Trade
    skillRawMaterialFreight     = 13073,        // group = Trade
    skillConsumableFreight      = 13074,        // group = Trade
    skillHazardousMaterialFreight       = 13075,        // group = Trade
    skillProcurement        = 16594,        // group = Trade
    skillDaytrading     = 16595,        // group = Trade
    skillWholesale      = 16596,        // group = Trade
    skillMarginTrading      = 16597,        // group = Trade
    skillMarketing      = 16598,        // group = Trade
    skillAccounting     = 16622,        // group = Trade
    skillTycoon     = 18580,        // group = Trade
    skillCorporationContracting     = 25233,        // group = Trade
    skillContracting        = 25235,        // group = Trade
    skillTaxEvasion     = 28261     // group = Trade
} EVESkillID;

//List of eve item types which have special purposes in the game.
//try to keep this list as short as possible, most things should be accomplish able
//by looking at the attributes of an item, not its type.
typedef enum {
    itemTypeCapsule = 670
} EVEItemTypeID;

//raceID as in table 'entity'
enum EVERace {
    raceCaldari = 1,
    raceMinmatar = 2,
    raceAmarr = 4,
    raceGallente = 8,
    raceJove = 16,
    racePirate = 32
};

//eve standing change messages
//If oFromID and oToID != fromID and toID, the following message is added (except for those marked with x):
//This standing change was initiated by a change from _oFromID towards _oToID
typedef enum {    //chrStandingChanges.eventTypeID
    standingGMInterventionReset = 25,        //Reset by a GM.
    standingDecay = 49,                        //All standing decays except when user isn't logged in
    standingPlayerSet = 65,                    //Set by player him/herself. Reason: _msg
    standingCorpSet = 68,                    //Corp stand set by _int1. Reason: _msg
    standingMissionCompleted = 73,            //_msg: name of mission
    standingMissionFailure = 74,            //_msg: name of mission
    standingMissionDeclined = 75,            //_msg: name of mission
    standingCombatAggression = 76,            //Combat - Aggression
    standingCombatShipKill = 77,            //Combat - Ship Kill
    standingCombatPodKill = 78,                //Combat - Pod Kill
    standingDerivedModificationPleased = 82,//_fromID Corp was pleased
    standingDerivedModificationDispleased = 83,    //_fromID Corp was displeased
    standingGMInterventionDirect = 84,        //Mod directly by _int1. Reason: _msg
    standingLawEnforcement = 89,            //Granted by Concord for actions against _int1
    standingMissionOfferExpired = 90,        //Mission Offer Expired - _msg
    standingCombatAssistance = 112,            //Combat - Assistance
    standingPropertyDamage = 154            //Property Damage
    //anything up until 500 is 'Standing Change'
} EVEStandingEventTypeID;


enum:uint64 {
    ROLE_CL                    = 549755813888LL,
    ROLE_CR                    = 1099511627776LL,
    ROLE_CM                    = 2199023255552LL,
    ROLE_BSDADMIN            = 35184372088832LL,
    ROLE_PROGRAMMER            = 2251799813685248LL,
    ROLE_QA                    = 4503599627370496LL,
    ROLE_GMH                = 9007199254740992LL,
    ROLE_GML                = 18014398509481984LL,
    ROLE_CONTENT            = 36028797018963968LL,
    ROLE_ADMIN                = 72057594037927936LL,
    ROLE_VIPLOGIN            = 144115188075855872LL,
    ROLE_ROLEADMIN            = 288230376151711744LL,
    ROLE_NEWBIE                = 576460752303423488LL,
    ROLE_SERVICE            = 1152921504606846976LL,
    ROLE_PLAYER                = 2305843009213693952LL,
    ROLE_LOGIN                = 4611686018427387904LL,
    ROLE_REMOTESERVICE        = 131072LL,
    ROLE_ACCOUNTMANAGEMENT    = 536870912LL,
    ROLE_DBA                = 16384LL,
    ROLE_TRANSLATION        = 524288LL,
    ROLE_CHTADMINISTRATOR    = 2097152LL,
    ROLE_TRANSLATIONADMIN    = 134217728LL,
    ROLE_IGB                = 2147483648LL,
    ROLE_TRANSLATIONEDITOR    = 4294967296LL,
    ROLE_TRANSLATIONTESTER    = 34359738368LL,
    ROLE_PETITIONEE            = 256LL,
    ROLE_CENTURION            = 2048LL,
    ROLE_WORLDMOD            = 4096LL,
    ROLE_LEGIONEER            = 262144LL,
    ROLE_CHTINVISIBLE        = 1048576LL,
    ROLE_HEALSELF            = 4194304LL,
    ROLE_HEALOTHERS            = 8388608LL,
    ROLE_NEWSREPORTER        = 16777216LL,
    ROLE_SPAWN                = 8589934592LL,
    ROLE_WIKIEDITOR            = 68719476736LL,
    ROLE_TRANSFER            = 137438953472LL,
    ROLE_GMS                = 274877906944LL,
    ROLE_MARKET                = 4398046511104LL,
    ROLE_MARKETH            = 8796093022208LL,
    ROLE_CSMADMIN            = 70368744177664LL,
    ROLE_CSMDELEGATE        = 140737488355328LL,
    ROLE_EXPOPLAYER            = 281474976710656LL,
    ROLE_BANNING            = 562949953421312LL,
    ROLE_DUST                = 1125899906842624LL,

    ROLE_ANY                = (18446744073709551615ULL & ~ROLE_IGB),
    ROLE_SLASH                = (ROLE_GML | ROLE_LEGIONEER),
    ROLEMASK_ELEVATEDPLAYER    = (ROLE_ANY & ~(ROLE_LOGIN | ROLE_PLAYER | ROLE_NEWBIE | ROLE_VIPLOGIN)),
    ROLEMASK_VIEW            = (ROLE_ADMIN | ROLE_CONTENT | ROLE_GML | ROLE_GMH | ROLE_QA),
    ROLE_TRANSAM            = (ROLE_TRANSLATION | ROLE_TRANSLATIONADMIN | ROLE_TRANSLATIONEDITOR),
};

enum {
    corpRoleLocationTypeHQ = 1LL,
    corpRoleLocationTypeBase = 2LL,
    corpRoleLocationTypeOther = 3LL,
};
typedef enum:uint64 {
    corpRoleDirector 		= 1LL,
    corpRolePersonnelManager 	= 128LL,
    corpRoleAccountant 		= 256LL,
    corpRoleSecurityOfficer 	= 512LL,
    corpRoleFactoryManager 	= 1024LL,
    corpRoleStationManager 	= 2048LL,
    corpRoleAuditor		= 4096LL,
    corpRoleHangarCanTake1 	= 8192LL,
    corpRoleHangarCanTake2 	= 16384LL,
    corpRoleHangarCanTake3 	= 32768LL,
    corpRoleHangarCanTake4 	= 65536LL,
    corpRoleHangarCanTake5 	= 131072LL,
    corpRoleHangarCanTake6 	= 262144LL,
    corpRoleHangarCanTake7 	= 524288LL,
    corpRoleHangarCanQuery1 	= 1048576LL,
    corpRoleHangarCanQuery2 	= 2097152LL,
    corpRoleHangarCanQuery3 	= 4194304LL,
    corpRoleHangarCanQuery4 	= 8388608LL,
    corpRoleHangarCanQuery5 	= 16777216LL,
    corpRoleHangarCanQuery6 	= 33554432LL,
    corpRoleHangarCanQuery7 	= 67108864LL,
    corpRoleAccountCanTake1 	= 134217728LL,
    corpRoleAccountCanTake2 	= 268435456LL,
    corpRoleAccountCanTake3 	= 536870912LL,
    corpRoleAccountCanTake4 	= 1073741824LL,
    corpRoleAccountCanTake5 	= 2147483648LL,
    corpRoleAccountCanTake6 	= 4294967296LL,
    corpRoleAccountCanTake7 	= 8589934592LL,
    corpRoleDiplomat 		= 17179869184LL,
    corpRoleEquipmentConfig 	= 2199023255552LL,
    corpRoleContainerCanTake1 	= 4398046511104LL,
    corpRoleContainerCanTake2 	= 8796093022208LL,
    corpRoleContainerCanTake3 	= 17592186044416LL,
    corpRoleContainerCanTake4 	= 35184372088832LL,
    corpRoleContainerCanTake5 	= 70368744177664LL,
    corpRoleContainerCanTake6 	= 140737488355328LL,
    corpRoleContainerCanTake7 	= 281474976710656LL,
    corpRoleCanRentOffice 	= 562949953421312LL,
    corpRoleCanRentFactorySlot 	= 1125899906842624LL,
    corpRoleCanRentResearchSlot = 2251799813685248LL,
    corpRoleJuniorAccountant 	= 4503599627370496LL,
    corpRoleStarbaseConfig 	= 9007199254740992LL,
    corpRoleTrader 		= 18014398509481984LL,
    corpRoleChatManager 	= 36028797018963968LL,
    corpRoleContractManager 	= 72057594037927936LL,
    corpRoleStarbaseCaretaker 	= 288230376151711744LL,
    corpRoleFittingManager 	= 576460752303423488LL,
    corpRoleInfrastructureTacticalOfficer = 144115188075855872LL,

    //Some Combos
    corpRoleAllHangar		= (corpRoleHangarCanTake1|corpRoleHangarCanTake2|corpRoleHangarCanTake3|corpRoleHangarCanTake4|corpRoleHangarCanTake5|corpRoleHangarCanTake6|corpRoleHangarCanTake7|corpRoleHangarCanQuery1|corpRoleHangarCanQuery2|corpRoleHangarCanQuery3|corpRoleHangarCanQuery4|corpRoleHangarCanQuery5|corpRoleHangarCanQuery6|corpRoleHangarCanQuery7),
    corpRoleAllAccount 		= (corpRoleJuniorAccountant|corpRoleAccountCanTake1|corpRoleAccountCanTake2|corpRoleAccountCanTake3|corpRoleAccountCanTake4|corpRoleAccountCanTake5|corpRoleAccountCanTake6|corpRoleAccountCanTake7),
    corpRoleAllContainer 	= (corpRoleContainerCanTake1|corpRoleContainerCanTake2|corpRoleContainerCanTake3|corpRoleContainerCanTake4|corpRoleContainerCanTake5|corpRoleContainerCanTake6|corpRoleContainerCanTake7),
    corpRoleAllOffice =		 (corpRoleCanRentOffice|corpRoleCanRentFactorySlot|corpRoleCanRentResearchSlot),
    corpRoleAll 		= (corpRoleAllHangar | corpRoleAllAccount | corpRoleAllContainer | corpRoleAllOffice | corpRoleDirector | corpRolePersonnelManager | corpRoleAccountant | corpRoleSecurityOfficer | corpRoleFactoryManager | corpRoleStationManager | corpRoleAuditor | corpRoleStarbaseConfig |corpRoleEquipmentConfig | corpRoleTrader | corpRoleChatManager),
} CorpRoleFlags;

//these come from dgmEffects.
typedef enum {

	effectShieldBoosting = 4,    //effects.ShieldBoosting
    effectSpeedBoost = 7,    //effects.SpeedBoost
    effectArmorRepair = 27,    //effects.ArmorRepair
    effectEMPWave = 38,
    effectEmpFieldRange = 99,
	effectTractorBeam = 2255,		// effects.TractorBeam
	effectAnchorDrop = 649,
	effectAnchorDropForStructures = 1022,
	effectAnchorLift = 650,
	effectAnchorLiftForStructures = 1023,
	effectBarrage = 263,
	effectBombLaunching = 2971,
	effectCloaking = 607,
	effectCloakingWarpSafe = 980,
	effectCloneVatBay = 2858,
	effectCynosuralGeneration = 2857,
	effectConcordWarpScramble = 3713,
	effectConcordModifyTargetSpeed = 3714,
	effectConcordTargetJam = 3710,
	effectDecreaseTargetSpeed = 586,
	effectDefenderMissileLaunching = 103,
	effectDeployPledge = 4774,
	effectECMBurst = 53,
	effectEmpWave = 38,
	effectEmpWaveGrid = 2071,
	effectEnergyDestabilizationNew = 2303,
	effectEntityCapacitorDrain = 1872,
	effectEntitySensorDampen = 1878,
	effectEntityTargetJam = 1871,
	effectEntityTargetPaint = 1879,
	effectEntityTrackingDisrupt = 1877,
	effectEwTargetPaint = 1549,
	effectEwTestEffectWs = 1355,
	effectEwTestEffectJam = 1358,
	effectFighterMissile = 4729,
	effectFlagshipmultiRelayEffect = 1495,
	effectFofMissileLaunching = 104,
	effectGangBonusSignature = 1411,
	effectGangShieldBoosterAndTransporterSpeed = 2415,
	effectGangShieldBoosteAndTransporterCapacitorNeed = 2418,
	effectGangIceHarvestingDurationBonus = 2441,
	effectGangInformationWarfareRangeBonus = 2642,
	effectGangArmorHardening = 1510,
	effectGangPropulsionJammingBoost = 1546,
	effectGangShieldHardening = 1548,
	effectGangECCMfixed = 1648,
	effectGangArmorRepairCapReducerSelfAndProjected = 3165,
	effectGangArmorRepairSpeedAmplifierSelfAndProjected = 3167,
	effectGangMiningLaserAndIceHarvesterAndGasCloudHarvesterMaxRangeBonus = 3296,
	effectGangGasHarvesterAndIceHarvesterAndMiningLaserDurationBonus = 3302,
	effectGangGasHarvesterAndIceHarvesterAndMiningLaserCapNeedBonus = 3307,
	effectGangInformationWarfareSuperiority = 3647,
	effectGangAbMwdFactorBoost = 1755,
	effectHackOrbital = 4773,
	effectHardPointModifier = 3773,
	effectHiPower = 12,
	effectIndustrialCoreEffect = 4575,
	effectJumpPortalGeneration = 2152,
	effectJumpPortalGenerationBO = 3674,
	effectLauncherFitted = 40,
	effectLeech = 3250,
	effectLoPower = 11,
	effectMedPower = 13,
	effectMineLaying = 102,
	effectMining = 17,
	effectMiningClouds = 2726,
	effectMiningLaser = 67,
	effectMissileLaunching = 9,
	effectMissileLaunchingForEntity = 569,
	effectModifyTargetSpeed2 = 575,
	effectNPCGroupArmorAssist = 4689,
	effectNPCGroupPropJamAssist = 4688,
	effectNPCGroupShieldAssist = 4686,
	effectNPCGroupSpeedAssist = 4687,
	effectNPCRemoteArmorRepair = 3852,
	effectNPCRemoteShieldBoost = 3855,
	effectNPCRemoteECM = 4656,
	effectOffensiveDefensiveReduction = 4728,
	effectOnline = 16,
	effectOnlineForStructures = 901,
	effectOpenSpawnContainer = 1738,
	effectProbeLaunching = 3793,
	effectProjectileFired = 34,
	effectProjectileFiredForEntities = 1086,
	effectRemoteHullRepair = 3041,
	effectRemoteEcmBurst = 2913,
	effectRigSlot = 2663,
	effectSalvaging = 2757,
	effectScanStrengthBonusTarget = 124,
	effectscanStrengthTargetPercentBonus = 2246,
	effectShieldResonanceMultiplyOnline = 105,
	effectSiegeModeEffect = 4877,
	effectSkillEffect = 132,
	effectSlotModifier = 3774,
	effectSnowBallLaunching = 2413,
	effectStructureUnanchorForced = 1129,
	effectSubSystem = 3772,
	effectSuicideBomb = 885,
	effectSuperWeaponAmarr = 4489,
	effectSuperWeaponCaldari = 4490,
	effectSuperWeaponGallente = 4491,
	effectSuperWeaponMinmatar = 4492,
	effectTargetAttack = 10,
	effectTargetAttackForStructures = 1199,
	effectTargetGunneryMaxRangeAndTrackingSpeedBonusHostile = 3555,
	effectTargetGunneryMaxRangeAndTrackingSpeedAndFalloffBonusHostile = 3690,
	effectTargetMaxTargetRangeAndScanResolutionBonusHostile = 3584,
	effectTargetGunneryMaxRangeAndTrackingSpeedBonusAssistance = 3556,
	effectTargetMaxTargetRangeAndScanResolutionBonusAssistance = 3583,
	effectTargetPassively = 54,
	effectTorpedoLaunching = 127,
	effectTorpedoLaunchingIsOffensive = 2576,
	effectTractorBeamCan = 2255,
	effectTriageMode = 4839,
	effectTurretFitted = 42,
	effectTurretWeaponRangeFalloffTrackingSpeedMultiplyTargetHostile = 3697,
	effectUseMissiles = 101,
	effectWarpDisruptSphere = 3380,
	effectWarpScramble = 39,
	effectWarpScrambleForEntity = 563,
	effectWarpScrambleTargetMWDBlockActivation = 3725,
	effectModifyShieldResonancePostPercent = 2052,
	effectModifyArmorResonancePostPercent = 2041,
	effectModifyHullResonancePostPercent = 3791,
	effectShipMaxTargetRangeBonusOnline = 3659,
	effectSensorBoostTargetedHostile = 837,
	effectmaxTargetRangeBonus = 2646
} EVEEffectID;

typedef enum JournalRefType {
    RefType_corpAccountWithdrawal = 37,
    RefType_corpBulkPayment = 47,
    RefType_corpDividendPayment = 38,
    RefType_corpLogoChange = 40,
    RefType_corpPayment = 11,
    RefType_corpRegFee = 39,
    RefType_officeRentalFee = 13,
    RefType_playerDonation = 10
} JournalRefType;

//from market_keyMap
typedef enum {
    accountCash = 1000,
    accountProperty = 1100,
    accountEscrow = 1500,
    accountReceivables = 1800,
    accountPayables = 2000,
    accountGold = 2010,
    accountEquity = 2900,
    accountSales = 3000,
    accountPurchases = 4000
} EVEAccountKeys;

//the constants are made up of:
//  prefix     -> cachedObject
//                config.BulkData.constants
//     category   -> config.BulkData.categories
//     group      -> config.BulkData.groups
//     metaGreoup -> config.BulkData.metagroups
//     attribute  -> config.BulkData.dgmattribs
//     effect     -> config.BulkData.dgmeffects
//    billType   -> config.BulkData.billtypes
//     role       -> config.Roles
//     flag       -> config.Flags
//     race       -> config.Races
//     bloodline  -> config.Bloodlines
//     statistic  -> config.Statistics
//     unit       -> config.Units
//     channelType -> config.ChannelTypes
//     encyclopediaType -> config.EncyclopediaTypes
//     activity   -> config.BulkData.ramactivities
//     completedStatus -> config.BulkData.ramcompletedstatuses
//
// First letter of `Name` field if capitalized when prefixed.
// see InsertConstantsFromRowset


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
    dgmEffPassive = 0,
    dgmEffActivation = 1,
    dgmEffTarget = 2,
    dgmEffArea = 3,
    dgmEffOnline = 4,
} EffectCategories;


/*
 *
ENV_IDX_SELF = 0
ENV_IDX_CHAR = 1
ENV_IDX_SHIP = 2
ENV_IDX_TARGET = 3
ENV_IDX_OTHER = 4
ENV_IDX_AREA = 5
ENV_IDX_EFFECT = 6
 *
 *
 *
 *
*/

/*
 service.ROLE_CHTADMINISTRATOR | service.ROLE_GMH
CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
CHTMODE_OPERATOR = ((4 + 2) + 1)
CHTMODE_CONVERSATIONALIST = (2 + 1)
CHTMODE_SPEAKER = 2
CHTMODE_LISTENER = 1
CHTMODE_NOTSPECIFIED = -1
CHTMODE_DISALLOWED = -2
CHTERR_NOSUCHCHANNEL = -3
CHTERR_ACCESSDENIED = -4
CHTERR_INCORRECTPASSWORD = -5
CHTERR_ALREADYEXISTS = -6
CHTERR_TOOMANYCHANNELS = -7
CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50

CHANNEL_CUSTOM = 0
CHANNEL_GANG = 3


*/


/*
 *from sys/cfg.py
 *




def IsNPC(ownerID):
    return ((ownerID < 100000000) and (ownerID > 10000))



def IsSystemOrNPC(ownerID):
    return (ownerID < 100000000)



def IsFaction(ownerID):
    if ((ownerID >= 500000) and (ownerID < 1000000)):
        return 1
    else:
        return 0



def IsCorporation(ownerID):
    if ((ownerID >= 1000000) and (ownerID < 2000000)):
        return 1
    elif (ownerID < 100000000):
        return 0
    elif ((boot.role == 'server') and sm.StartService('standing2').IsKnownToBeAPlayerCorp(ownerID)):
        return 1
    else:
        return cfg.eveowners.Get(ownerID).IsCorporation()



def IsCharacter(ownerID):
    if ((ownerID >= 3000000) and (ownerID < 4000000)):
        return 1
    elif (ownerID < 100000000):
        return 0
    elif ((boot.role == 'server') and sm.StartService('standing2').IsKnownToBeAPlayerCorp(ownerID)):
        return 0
    else:
        return cfg.eveowners.Get(ownerID).IsCharacter()



def IsOwner(ownerID, fetch = 1):
    if (((ownerID >= 500000) and (ownerID < 1000000)) or (((ownerID >= 1000000) and (ownerID < 2000000)) or ((ownerID >= 3000000) and (ownerID < 4000000)))):
        return 1
    if IsNPC(ownerID):
        return 0
    if fetch:
        oi = cfg.eveowners.Get(ownerID)
        if (oi.groupID in (const.groupCharacter,
         const.groupCorporation)):
            return 1
        else:
            return 0
    else:
        return 0



def IsAlliance(ownerID):
    if (ownerID < 100000000):
        return 0
    elif ((boot.role == 'server') and sm.StartService('standing2').IsKnownToBeAPlayerCorp(ownerID)):
        return 0
    else:
        return cfg.eveowners.Get(ownerID).IsAlliance()




def IsJunkLocation(locationID):
    if (locationID >= 2000):
        return 0
    elif (locationID in [6,
     8,
     10,
     23,
     25]):
        return 1
    elif ((locationID > 1000) and (locationID < 2000)):
        return 1
    else:
        return 0


*/

// This is the
#define EVEMU_MINIMUM_ID 140000000
#define EVEMU_MINIMUM_ENTITY_ID 90000000
#define EVEMU_MAXIMUM_ENTITY_ID (EVEMU_MINIMUM_ID-1)

#define IsAgent(itemID) \
    ((itemID >= 3008416) && (itemID < 3020000))

#define IsStaticMapItem(itemID) \
    ((itemID >= 10000000) && (itemID < 64000000))

#define IsRegion(itemID) \
    ((itemID >= 10000000) && (itemID < 20000000))

#define IsConstellation(itemID) \
    ((itemID >= 20000000) && (itemID < 30000000))

#define IsSolarSystem(itemID) \
    ((itemID >= 30000000) && (itemID < 40000000))

#define IsUniverseCelestial(itemID) \
    ((itemID >= 40000000) && (itemID < 50000000))

#define IsStargate(itemID) \
    ((itemID >= 50000000) && (itemID < 60000000))

#define IsStation(itemID) \
    ((itemID >= 60000000) && (itemID < 64000000))

#define IsTrading(itemID) \
    ((itemID >= 64000000) && (itemID < 66000000))

#define IsOfficeFolder(itemID) \
    ((itemID >= 66000000) && (itemID < 68000000))

#define IsFactoryFolder(itemID) \
    ((itemID >= 68000000) && (itemID < 70000000))

#define IsUniverseAsteroid(itemID) \
    ((itemID >= 70000000) && (itemID < 80000000))

#define IsScenarioItem(itemID) \
    ((itemID >= 90000000) && (itemID < (EVEMU_MINIMUM_ID-1)))

#define IsNonStaticItem(itemID) \
    (itemID >= EVEMU_MINIMUM_ID)

#endif

