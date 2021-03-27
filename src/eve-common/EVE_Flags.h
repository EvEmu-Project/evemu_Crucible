 /*
  *
  *
  *
  */


 #ifndef EVE_FLAGS_H
 #define EVE_FLAGS_H


//from invFlags DB table
enum EVEItemFlags {
    flagNone                        = 0,
    flagWallet                      = 1,
    // flagFactory                     = 2,  // not in client
    flagWardrobe                    = 3,  // dunno what this is for  clothes?
    flagHangar                      = 4,  // station hangar and ship corp hangar 1
    flagCargoHold                   = 5,    //AttrCapacity
    flagBriefcase                   = 6,  // dunno what this is for
    flagSkill                       = 7,
    flagReward                      = 8,
    flagConnected                   = 9,    //Character in station connected
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

    flagMidSlot0                    = 19,    //Medium power slot 1
    flagMidSlot1                    = 20,
    flagMidSlot2                    = 21,
    flagMidSlot3                    = 22,
    flagMidSlot4                    = 23,
    flagMidSlot5                    = 24,
    flagMidSlot6                    = 25,
    flagMidSlot7                    = 26,    //Medium power slot 8

    flagHiSlot0                     = 27,    //High power slot 1
    flagHiSlot1                     = 28,
    flagHiSlot2                     = 29,
    flagHiSlot3                     = 30,
    flagHiSlot4                     = 31,
    flagHiSlot5                     = 32,
    flagHiSlot6                     = 33,
    flagHiSlot7                     = 34,    //High power slot 8

    flagFixedSlot                   = 35,

    //factory stuff:   not in crucible client
    flagFactoryBlueprint            = 36,
    flagFactoryMinerals             = 37,
    flagFactoryOutput               = 38,
    flagFactoryActive               = 39,

   /*  not real sure wtf this is, or what it's used for.  have not found references to any of these.  not defined in client
    flagPromenadeSlot1              = 40,  //Promenade slot 1
    flagPromenadeSlot2              = 41,
    flagPromenadeSlot3              = 42,
    flagPromenadeSlot4              = 43,
    flagPromenadeSlot5              = 44,
    flagPromenadeSlot6              = 45,
    flagPromenadeSlot7              = 46,
    flagPromenadeSlot8              = 47,
    flagPromenadeSlot9              = 48,
    flagPromenadeSlot10             = 49,
    flagPromenadeSlot11             = 50,
    flagPromenadeSlot12             = 51,
    flagPromenadeSlot13             = 52,
    flagPromenadeSlot14             = 53,
    flagPromenadeSlot15             = 54,
    flagPromenadeSlot16             = 55,  //Promenade slot 16
    */

    flagCapsule                     = 56,    //Capsule item in space
    flagPilot                       = 57,
    // not sure about these next 3.  not defined, not sure if we can use them (or want to)
    flagPassenger                   = 58,
    flagBoardingGate                = 59,
    flagCrew                        = 60,

    flagSkillInTraining             = 61,

    // these are flags for items in stations' corp offices (from client code)
    flagCorpMarket                  = 62,    //Corporation Market Deliveries / Returns  this item will be in location(officeID)
    flagLocked                      = 63,    //Locked item, can not be moved unless unlocked (corp items only.  locked/unlocked by vote)
    flagUnlocked                    = 64,
    flagOffice                      = 71,    // offices
    flagImpounded                   = 72,    // impounded or junk
    flagProperty                    = 74,    // property
    // not in client
    // flagDelivery                    = 75,    // deliveries

    /*  they were OfficeSlot* in client const, but i cannot find any references to them in code
    flagUnknown1                    = 70,
    flagUnknown4                    = 73,
    flagUnknown7                    = 76,
    flagUnknown8                    = 77,
    flagUnknown9                    = 78,
    flagUnknown10                   = 79,
    flagUnknown11                   = 80,
    flagUnknown12                   = 81,
    flagUnknown13                   = 82,
    flagUnknown14                   = 83,
    flagUnknown15                   = 84,
    flagUnknown16                   = 85,
    */
    flagBonus                       = 86,    //Char bonus/penalty
    flagDroneBay                    = 87,   //AttrDroneCapacity
    flagBooster                     = 88,
    flagImplant                     = 89,
    flagShipHangar                  = 90,    //AttrShipMaintenanceBayCapacity
    flagShipOffline                 = 91,

    flagRigSlot0                    = 92,    //Rig power slot 1
    flagRigSlot1                    = 93,    //Rig power slot 2
    flagRigSlot2                    = 94,    //Rig power slot 3
    flagRigSlot3                    = 95,    //Rig power slot 4
    flagRigSlot4                    = 96,    //Rig power slot 5
    flagRigSlot5                    = 97,    //Rig power slot 6
    flagRigSlot6                    = 98,    //Rig power slot 7
    flagRigSlot7                    = 99,    //Rig power slot 8

    flagFactoryOperation            = 100,

    //  these are flags for items in corp hangars (station, container, ship), by divisionID    all use AttrCorporateHangarCapacity
    flagCorpHangar2                 = 116,   // formerly corpSAG* or Security Access Group.
    flagCorpHangar3                 = 117,
    flagCorpHangar4                 = 118,
    flagCorpHangar5                 = 119,
    flagCorpHangar6                 = 120,
    flagCorpHangar7                 = 121,

    flagSecondaryStorage            = 122,    //Secondary Storage  (strontium bay on POS)
    flagCaptainsQuarters            = 123,    //Captains Quarters
    flagWisPromenade                = 124,    //Wis Promenade       Walking In Station
    // flagWorldSpace = 124

    flagSubSystem0                  = 125,    //Sub system slot 0
    flagSubSystem1                  = 126,    //Sub system slot 1
    flagSubSystem2                  = 127,    //Sub system slot 2
    flagSubSystem3                  = 128,    //Sub system slot 3
    flagSubSystem4                  = 129,    //Sub system slot 4
    flagSubSystem5                  = 130,    //Sub system slot 5
    flagSubSystem6                  = 131,    //Sub system slot 6
    flagSubSystem7                  = 132,    //Sub system slot 7

    flagFuelBay                     = 133,  //AttrFuelBayCapacity  this is not cargo, but ship fuel
    flagOreHold                     = 134,
    flagGasHold                     = 135,
    flagMineralHold                 = 136,
    flagSalvageHold                 = 137,  // also for refinables (drone compounds)
    flagShipHold                    = 138,
    flagSmallShipHold               = 139,
    flagMediumShipHold              = 140,
    flagLargeShipHold               = 141,
    flagIndustrialShipHold          = 142,
    flagAmmoHold                    = 143,  //AttrAmmoCapacity

    flagStructureActive             = 144,
    flagStructureInactive           = 145,

    flagJunkyardReprocessed         = 146, // not sure about this one.
    flagJunkyardTrashed             = 147, // not sure about this one.

    // pi containers
    flagCommandCenterHold           = 148,
    flagPlanetaryCommoditiesHold    = 149,  // silos?
    flagPlanetSurface               = 150,   // not sure about this one.
    flagMaterialBay                 = 151,        // OrbitalUpgradeHold

    flagDustCharacterBackpack       = 152,
    flagDustCharacterBattle         = 153,
    flagQuafeBay                    = 154,
    flagFleetHangar                 = 155, // not sure about this one.

    flagResearchFacilitySlotFirst   = 200,
    flagResearchFacilitySlotLast    = 255,

    flagMissile                     = 300,

    flagClone                       = 400,

    flagIllegal                     = 999

    /*  actual flags defined in client.....
     flagNone = 0
     flagBonus = 86
     flagBooster = 88
     flagBriefcase = 6
     flagCapsule = 56
     flagCargo = 5
     flagCorpMarket = 62
     flagCorpSAG2 = 116
     flagCorpSAG3 = 117
     flagCorpSAG4 = 118
     flagCorpSAG5 = 119
     flagCorpSAG6 = 120
     flagCorpSAG7 = 121
     flagDroneBay = 87
     flagDustBackpack = 152
     flagDustBattle = 153
     flagCorpSAGs = (flagCorpSAG2,
     flagCorpSAG3,
     flagCorpSAG4,
     flagCorpSAG5,
     flagCorpSAG6,
     flagCorpSAG7)
     flagFactoryOperation = 100
     flagFixedSlot = 35
     flagHangar = 4
     flagHangarAll = 1000
     flagHiSlot0 = 27
     flagHiSlot1 = 28
     flagHiSlot2 = 29
     flagHiSlot3 = 30
     flagHiSlot4 = 31
     flagHiSlot5 = 32
     flagHiSlot6 = 33
     flagHiSlot7 = 34
     flagImplant = 89
     flagLoSlot0 = 11
     flagLoSlot7 = 18
     flagLocked = 63
     flagMidSlot0 = 19
     flagMidSlot7 = 26
     flagNone = 0
     flagPilot = 57
     flagPlanetSurface = 150
     flagQuafeBay = 154
     flagReward = 8
     flagRigSlot0 = 92
     flagRigSlot1 = 93
     flagRigSlot2 = 94
     flagRigSlot3 = 95
     flagRigSlot4 = 96
     flagRigSlot5 = 97
     flagRigSlot6 = 98
     flagRigSlot7 = 99
     flagSecondaryStorage = 122
     flagShipHangar = 90
     flagShipOffline = 91
     flagSkill = 7
     flagSkillInTraining = 61
     flagFuelBay = 133
     flagOreHold = 134
     flagGasHold = 135
     flagMineralHold = 136
     flagSalvageHold = 137
     flagShipHold = 138
     flagSmallShipHold = 139
     flagMediumShipHold = 140
     flagLargeShipHold = 141
     flagIndustrialShipHold = 142
     flagAmmoHold = 143
     flagCommandCenterHold = 148
     flagPlanetaryCommoditiesHold = 149
     flagMaterialBay = 151
     flagSlotFirst = 11
     flagSlotLast = 35
     flagStructureActive = 144
     flagStructureInactive = 145
     flagWorldSpace = 124
     flagSubSystemSlot0 = 125
     flagSubSystemSlot1 = 126
     flagSubSystemSlot2 = 127
     flagSubSystemSlot3 = 128
     flagSubSystemSlot4 = 129
     flagSubSystemSlot5 = 130
     flagSubSystemSlot6 = 131
     flagSubSystemSlot7 = 132
     flagUnlocked = 64
     flagWallet = 1
     flagJunkyardReprocessed = 146
     flagJunkyardTrashed = 147
     flagWardrobe = 3
     */
};

//for use in the new module manager
typedef enum {
    NaT                  = 0,
    slotTypeSubSystem    = 1,
    slotTypeRig          = 2,
    slotTypeLowPower     = 3,
    slotTypeMedPower     = 4,
    slotTypeHiPower      = 5
} EVEItemSlotType;

//some alternative names for entries above.
static const EVEItemFlags flagSlotFirst = flagLowSlot0;    //duplicate values
static const EVEItemFlags flagSlotLast = flagFixedSlot;

static const uint8 MAX_MODULE_COUNT = flagSlotLast - flagSlotFirst + 1;
static const uint8 MAX_HIGH_SLOT_COUNT = flagHiSlot7 - flagHiSlot0 + 1;
static const uint8 MAX_MID_SLOT_COUNT = flagMidSlot7 - flagMidSlot0 + 1;
static const uint8 MAX_LOW_SLOT_COUNT = flagLowSlot7 - flagLowSlot0 + 1;
static const uint8 MAX_RIG_COUNT = flagRigSlot7 - flagRigSlot0 + 1;
static const uint8 MAX_ASSEMBLY_COUNT = flagSubSystem7 - flagSubSystem0 + 1;

#endif  // EVE_FLAGS_H
