
/*  EVE_Defines.h
 *   this file defines hard-coded values for game item classes
 *   we also define C-type macros for ease of readabilty in various item conditionals
 *
 */

/*
 *******************************
 *********   ITEM CLASSES BASED ON UniqueIdetifier (UID)
 *  <0    private chat channels
 *   1    static chat channels
 *  1k    temp Planet Structures (PinIDs - sent from client)
 * 100k    bookmark folders  ** not implemented yet
 * 350k    market groups
 * 500k    Factions
 *  1m    NPC Corp
 * 2m1    trial-friendly chat channels
 *  3m    Agent
 *  9m    Universe (not sure)
 * 10m    K-Space Region  (Known space)
 * 11m    W-Space Region  (Wormhole space)
 * 12m    A-Space Region  (Abyssal space - not in crucible)
 * 20m    K-Space Constellation
 * 20m    W-Space Constellation
 * 20m    A-Space Constellation
 * 30m    K-Space Solar System
 * 31m    W-Space  Solar System
 * 32m    A-Space  Solar System
 * 40m    Celestial  (star:multiple_types,groupID=6, planet:multiple_types,groupID=7, moon:typeID=14,groupID=8; belt:typeID=15,groupID=9)
 * 50m    Stargate  (groupid 10)
 * 60m    Stations  (groupid 15, catid 3)
 * 61m    Outposts   ** need to put Outpost creation here @ 61m, and save in mapDenormalize table.
 * 64m    Trading     ** containers for trade.  this is the inventory item all traded items are contained in, but separate, ofc.
 * 66m    OfficeFolder
 * 68m    FactoryFolder
 * 70m    UniverseAsteroid  ** this is NON-targetable, NON-selectable roids.  deco only
 * 80m    Control Bunker (to 80m1)  ** not sure what this is yet
 * 81m    Promenades (WIS shit)
 * 82m    Planatary Districts
 * 90m    PlayerCharacters   (-1 = max NPC Item)
 * 98m    PlayerCorps
 * 99m    Alliance
 * 100m    Station Office      ** corp officeID.  used for locationID of items in corp hangar, based on flag.
 * 110m    temp SEs
 * 130m    Planet Structures (PinIDs)
 * 135m    Customs Offices (67253 planets)  ** 1004746013567 on live
 * 140m    saved DSEs, player items
 * 450m    asteroid SEs
 * 500m    player drones
 * 600m    bookmarks - player created  ** 1130462570 on live
 * 635m    bookmarks - mission created  ** not implemented yet
 * 650m    blueprints ** not implemented yet   ** 1007111250519 on live
 *** begin temp, non-saved UIDs
 * 750m    npcs and jetcans used as tracking markers
 * 950m    fleetIDs
 * 960m    wingIDs
 * 970m    squadIDs
 *  1b    temp DSEs (missiles only at this time)  ** 9000000000001190696 on live
 * 1b2    dungeon SEs
 * 2b1    max int32
 * 9e0    fake items
 */

#ifndef EVE_DEFINES_H
#define EVE_DEFINES_H

// defined breakPoint  --doesnt work.  not sure why.  dont care
#define SrvPause()  do { printf("%s %d\n", __FILE__, __LINE__); getchar(); } while (0)

// bulkdata version
#define bulkDataBranch          4
//  bulkDataChangeID rddmmyyyy
//  where r=revision, dd=day, mm=month, yyyy=year
#define bulkDataChangeID        107232020

//  define number of rentable offices in stations (config option later?)
//  this replaces the hard-coded count previously used in station.h
#define maxRentableOffices      10

//  defines based on itemID, per client
#define maxNonCapModuleSize     500

#define minCharType             1373
#define maxCharType             1386

#define minEveMarketGroup       0
#define maxStaticChannel        1000
#define minJunkyardID           1000
#define maxJunkyardID           1999
#define minBMFolder             100000
#define maxBMFolder             300000
#define maxEveMarketGroup       350000
#define minDustMarketGroup      350001
#define maxDustMarketGroup      399999
#define minFaction              500000
#define maxFaction              999999
#define minNPCCorporation       1000000
#define maxNPCCorporation       1100000
#define minAgent                3000000
#define maxAgent                3999999
#define npcTraderJoe            4000000
#define minTrader               4000001
#define maxTrader               5000000
#define minRegion               10000000
#define maxRegion               19999999
#define minWHRegion             11000000
#define maxWHRegion             11999999
#define minConstellation        20000000
#define maxConstellation        29999999
#define minWHConstellation      21000000
#define maxWHConstellation      21999999
#define minSolarSystem          30000000
#define maxSolarSystem          31999999
#define minWHSolarSystem        31000000
#define maxWHSolarSystem        31999999
#define minValidLocation        30000000
#define minValidShipLocation    30000000
#define minUniverseCelestial    40000000
#define maxUniverseCelestial    49999999
#define minStargate             50000000
#define maxStargate             59999999
#define minValidCharLocation    60000000
#define minStation              60000000
#define maxNPCStation           60999999
#define minOutpost              61000000
#define maxStation              63999999
#define minTradeCont            64000000
#define maxTradeCont            65999999
#define minOfficeFolder         66000000
#define maxOfficeFolder         67999999
#define minFactoryFolder        68000000
#define maxFactoryFolder        69999999
#define minUniverseAsteroid     70000000        // deco only - not targetable, no bracket in overview
#define maxUniverseAsteroid     79999999
#define minControlBunker        80000000        // not sure what this is yet.
#define maxControlBunker        80099999
#define maxNPCItem              89999999
#define minCharacter            90000000        // client NPC def ends here
#define maxCharacter            97999999
#define minPCCorporation        98000000        // player corps start here
#define minAlliance             99000000        // alliances start here
#define minOffice               100000000
#define maxOffice               109999999
#define minTempItemID           110000000
#define minPIStructure          130000000       // Planet Structures
#define minCustomsOffice        135000000
#define minPlayerItem           140000000
#define maxPlayerItem           300000000
#define minAsteroidItem         450000000
#define minDroneItem            500000000
#define minBookmark             600000000
#define minMissionBookmark      635000000
#define minBlueprint            650000000
#define minNPC                  750000000
#define maxNPC                  760000000
#define minFleet                950000000
#define maxFleet                959000000
#define minWing                 960000000
#define maxWing                 969000000
#define minSquad                970000000
#define maxSquad                979000000
#define minMissile              1000000000
#define minDungeon              1200000000
#define maxEveItem              2147483647      // max int32

#define maxHangarCapy           9000000000000000

#define minFakeItem             9000000000000000000     // min missileID on live.  not needed here?

//  DSTLOCALBALLS = 0x0C0000000h  (3,221,225,472 decimal)      unknown where this is from


//  static defines to ease code checks
#define STATION_OFFICE_OFFSET     6000000
#define TEMP_ENTITY_ID          110000000
#define PLANET_PIN_ID           130000000
#define ASTEROID_ID             450000000
#define DRONE_ID                500000000
#define NPC_ID                  750000000
#define FLEET_ID                950000000
#define WING_ID                 960000000
#define SQUAD_ID                970000000
#define MISSILE_ID             1000000000
#define DUNGEON_ID             1200000000

#define MAX_SHORT_ID           2147483647
#define MAX_LONG_ID   9223372036854775807     // max int64.


// defines based on UID
#define IsTempPinID(pinID) \
 (pinID <= 1000)

#define IsStaticChannel(itemID) \
 ((itemID >= 1) && (itemID <= maxStaticChannel))

#define IsCharType(typeID) \
 ((typeID >= minCharType) && (typeID <= maxCharType))

#define IsWreckTypeID(typeID) \
 ((typeID > 26467) && (typeID < 30826))

#define IsCharacterID(itemID) \
 ((itemID >= minCharacter) && (itemID <= maxCharacter))

#define IsValidLocationID(itemID) \
  ((IsSolarSystemID(itemID)) \
  or (IsRegionID(itemID)) \
  or (IsStationID(itemID)) \
  or (IsPlayerItem(itemID)) \
  or (IsCharacterID(itemID)))

#define IsValidOwner(itemID) \
((itemID == 1) || ((itemID >= minNPCCorporation) && (itemID <= maxNPCCorporation)) \
|| ((itemID >= minPCCorporation) && (itemID < minAlliance)) \
|| ((itemID >= minCharacter) && (itemID <= maxCharacter)))

#define IsCharacterLocation(itemID) \
 (itemID >= minValidCharLocation)

#define IsContainerLocation(itemID) \
 (itemID >= minValidShipLocation)

#define IsFleetID(itemID) \
((itemID >= minFleet) && (itemID < maxFleet))

#define IsWingID(itemID) \
((itemID >= minWing) && (itemID < maxWing))

#define IsSquadID(itemID) \
((itemID >= minSquad) && (itemID < maxSquad))

#define IsCorp(itemID) \
(((itemID >= minNPCCorporation) && (itemID <= maxNPCCorporation)) \
|| ((itemID >= minPCCorporation) && (itemID < minAlliance)))

#define IsNPCCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID < maxNPCCorporation))

#define IsPlayerCorp(itemID) \
((itemID >= minPCCorporation) && (itemID < minAlliance))

#define IsAlliance(itemID) \
((itemID >= minAlliance) && (itemID < minOffice))

#define IsAgent(itemID) \
((itemID >= minAgent) && (itemID < maxAgent))

#define IsFaction(itemID) \
((itemID >= minFaction) && (itemID < maxFaction))

#define IsOfficeID(itemID) \
((itemID >= minOffice) && (itemID < minTempItemID))

#define IsPlayerItem(itemID) \
((itemID > minPlayerItem) && (itemID < maxPlayerItem))

#define IsAsteroidID(itemID) \
((itemID >= minAsteroidItem) && (itemID < minDroneItem))

#define IsDrone(itemID) \
((itemID >= minDroneItem) && (itemID < minBookmark))

// this covers all static items
#define IsStaticItem(itemID) \
 (itemID <= maxNPCItem)

// this covers ALL static celestial-type items
#define IsStaticMapItem(itemID) \
((itemID >= minRegion) && (itemID < maxNPCItem))

#define IsRegionID(itemID) \
((itemID >= minRegion) && (itemID < minConstellation))

#define IsConstellationID(itemID) \
((itemID >= minConstellation) && (itemID < minSolarSystem))

#define IsSolarSystemID(itemID) \
((itemID >= minSolarSystem) && (itemID < maxSolarSystem))

#define IsWSpaceID(itemID) \
((itemID >= minWHSolarSystem) && (itemID < maxWHSolarSystem))

#define IsKSpaceID(itemID) \
((itemID >= minSolarSystem) && (itemID < minWHSolarSystem))

#define IsCelestialID(itemID) \
((itemID >= minUniverseCelestial) && (itemID < minStargate))

#define IsStargateID(itemID) \
((itemID >= minStargate) && (itemID < minStation))

#define IsStationID(itemID) \
((itemID >= minStation) && (itemID < minTradeCont))

#define IsNPCStation(itemID) \
((itemID >= minStation) && (itemID < minOutpost))

#define IsNPC(itemID) \
((itemID >= minNPC) && (itemID <= maxNPC))

#define IsOutpost(itemID) \
((itemID >= minOutpost) && (itemID < minTradeCont))

#define IsTrading(itemID) \
((itemID >= minTradeCont) && (itemID < minOfficeFolder))

#define IsOfficeFolder(itemID) \
((itemID >= minOfficeFolder) && (itemID < minFactoryFolder))

#define IsFactoryFolder(itemID) \
((itemID >= minFactoryFolder) && (itemID < minUniverseAsteroid))

#define IsUniverseAsteroid(itemID) \
((itemID >= minUniverseAsteroid) && (itemID < minControlBunker))

#define IsControlBunker(itemID) \
((itemID >= minControlBunker) and (itemID < 80100000))

#define IsScenarioItem(itemID) \
((itemID > minCharacter) && (itemID < minPlayerItem))

#define IsFakeItem(itemID) \
 (itemID >= minFakeItem)

#define IsValidTarget(itemID) \
 (((itemID >= minOutpost) && (itemID <= maxStation)) \
 || ((itemID >= minControlBunker) && (itemID <= maxNPCItem)) \
 || ((itemID >= minTempItemID) && (itemID <= minPIStructure)) \
 || (itemID >= minCustomsOffice))

#define IsTempItem(itemID) \
 ((itemID >= minTempItemID) && (itemID < minPIStructure))

#define IsCargoHoldFlag(flag) \
((flag == flagCargoHold) || (flag == flagSecondaryStorage) || (flag == flagShipHangar) \
  || ((flag >= flagFuelBay) && (flag <= flagAmmoHold)))

#define IsSpecialHoldFlag(flag) \
((flag == flagSecondaryStorage) || (flag == flagShipHangar) \
  || ((flag >= flagFuelBay) && (flag <= flagAmmoHold)))

#define IsHangarFlag(flag) \
((flag == flagHangar) || ((flag >= flagCorpHangar2) && (flag <= flagCorpHangar7)))

#define IsOfficeFlag(flag) \
((flag >= flagCorpMarket) && (flag <= flagProperty))

#define IsModuleSlot(flag) \
(((flag >= flagLowSlot0) && (flag <= flagHiSlot7)) \
  || ((flag >= flagRigSlot0) && (flag <= flagRigSlot7)) \
  || ((flag >= flagSubSystem0) && (flag<=flagSubSystem7)))

#define IsFittingSlot(flag) \
((flag >= flagLowSlot0) && (flag <= flagHiSlot7))

#define IsHiSlot(flag) \
((flag >= flagHiSlot0) && (flag <= flagHiSlot7))

#define IsMidSlot(flag) \
((flag >= flagMidSlot0) && (flag <= flagMidSlot7))

#define IsLowSlot(flag) \
((flag >= flagLowSlot0) && (flag <= flagLowSlot7))

#define IsRigSlot(flag) \
((flag >= flagRigSlot0) && (flag <= flagRigSlot7))

#define IsSubSystem(flag) \
((flag >= flagSubSystem0) && (flag <= flagSubSystem7))


#define IsCash(key) \
((key >= 1000) && (key <= 1006))

#define IsAUR(key) \
((key >= 1200) && (key <= 1206))

#define IsDustKey(key) \
(key >= 10000)


#define FlagToSlot(flag) \
(flag - flagSlotFirst)

#define SlotToFlag(slot) \
((EVEItemFlags)(flagSlotFirst + slot))


#define IsDScanImmune(itemID) \
(((itemID >= minNPC) && (itemID <= maxNPC)) \
|| ((itemID >= minTempItemID) && (itemID < minPIStructure)) \

#define IsTraderJoe(itemID) \
(itemID == npcTraderJoe)

#define IsTrader(itemID) \
(((itemID >= minTrader) && (itemID <= maxTrader))


/*
def IsJunkLocation(locationID):
    if locationID >= 2000:
        return 0
    elif locationID in (6, 8, 10, 23, 25):
        return 1
    elif locationID > 1000 and locationID < 2000:
        return 1
    else:
        return 0

*/

#endif  //EVE_DEFINES_H