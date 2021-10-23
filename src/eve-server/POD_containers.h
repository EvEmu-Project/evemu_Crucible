/*
 * POD_continers.h
 *
 *    -allan  1Aug16
 */

#ifndef _EVEMU_POD_CONTAINERS_H_
#define _EVEMU_POD_CONTAINERS_H_

#include "eve-server.h"

/** @todo most of these need their own namespace
 *     currently moving to specific eve-common/EVE_xxxx.h files
 */

/* POD structure for certificate data */
struct CharCerts {
    uint8 visibilityFlags;
    uint32 certificateID;
    int64 grantDate;
};
typedef std::map<uint16, CharCerts> CertMap;

/* POD structure for account data */
struct AccountData {
    bool online:1;
    bool banned:1;
    uint8 type;
    int32 id;
    int32 visits;
    uint32 clientID;
    int64 role;
    std::string name;
    std::string hash;
    std::string password;
    std::string last_login;
};

/* POD structure for character data */
struct CharacterData {
    bool gender:1;
    uint8 flag;
    uint8 bloodlineID;
    uint8 raceID;
    uint16 typeID;
    uint32 accountID;
    uint32 shipID;
    uint32 capsuleID;
    uint32 logonMinutes;
    uint32 locationID;
    uint32 stationID;
    uint32 solarSystemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 ancestryID;
    uint32 careerID;
    uint32 schoolID;
    uint32 careerSpecialityID;
    uint32 skillPoints;
    int64 loginTime;
    int64 createDateTime;
    float bounty;
    float balance;
    float aurBalance;
    float securityRating;
    std::string name;
    std::string title;
    std::string description;
};

/* POD structure for corp data */
struct CorpData {
    int16 corpAccountKey;
    int32 allianceID;
    int32 warFactionID;
    uint32 corporationID;
    uint32 corpHQ;
    uint32 baseID;
    int64 startDateTime;
    int64 corpRole;
    int64 rolesAtAll;
    int64 rolesAtBase;
    int64 rolesAtHQ;
    int64 rolesAtOther;
    int64 grantableRoles;
    int64 grantableRolesAtBase;
    int64 grantableRolesAtHQ;
    int64 grantableRolesAtOther;
    float taxRate;
    float secRating;
    std::string name;
    std::string ticker;
};

/* POD structure for fleet data    -allan 31Jul14 */
struct CharFleetData {
    int8 job;
    int8 role;
    int8 booster;
    int32 fleetID;
    int32 wingID;
    int32 squadID;
    int64 joinTime;
};

/* POD structure for bounty timer data  */
struct BountyData {
    uint8 refTypeID;
    uint16 fromKey;
    uint16 toKey;
    uint32 fromID;
    uint32 toID;
    float amount;
    std::string reason;
};


/* POD structure for character kill data  -allan 01May16 */
struct CharKillData {
    uint16 victimShipTypeID;
    uint16 finalShipTypeID;
    uint16 finalWeaponTypeID;
    int32 finalAllianceID;
    int32 victimAllianceID;
    int32 victimFactionID;
    int32 finalFactionID;
    uint32 solarSystemID;
    uint32 victimCharacterID;
    uint32 victimCorporationID;
    uint32 victimDamageTaken;
    uint32 finalCharacterID;
    uint32 finalCorporationID;
    uint32 finalDamageDone;
    uint32 moonID;
    int64 killTime;
    double finalSecurityStatus;
    std::string killBlob;
};

/* POD structure for asteroid */
struct AsteroidData {
    uint16 typeID;
    uint32 itemID;
    uint32 systemID;
    uint32 beltID;
    double quantity;
    double radius;
    GPoint position;
    std::string itemName;
};

/* POD structure for asteroid distribution methods by group */
struct OreTypeChance {
    uint16 typeID;
    float chance;
};

/* POD structure for cosmic signatures/anomalies */
struct CosmicSignature {
    uint8 dungeonType;          // internal for creation checks
    uint16 bubbleID;            // internal for .siglist command
    // typeID of signal
    uint16 sigTypeID;           // type name if scanGroupID is not sig or anom and certainty > 0.75
    // groupID of signal
    uint16 sigGroupID;          // group name if scanGroupID is not sig or anom and certainty > 0.25
    // groupID of signature...must be one of sig, anom, ship, drone, structure
    uint16 scanGroupID;         // ship,drone and structure uses sigGroupID for group name
    uint16 scanAttributeID;     // group naming data if scanGroupID is anom or sig and certainty > 0.25
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;           // itemID of this entry
    float sigStrength;
    GPoint position;
    std::string sigID;          // this is unique xxx-nnn id displayed in scanner.  can be other values
    std::string sigName;        // site name if scanGroupID is sig or anom and certainty > 0.75
};

/* POD structure for spawn groups */
struct SystemSpawnGroup { //reference to this bubble's data for spawn groups.  may need later.
    //SystemBubble* pSysBubble;   //cant use reference or pointer here...
    uint32 bubbleID;
    uint32 systemID;
    uint32 regionID;
    double secRating;
};

/* POD structure for spawn groups */
struct SpawnGroup {
    uint8 quantity; //quantity to spawn for this typeID
    uint16 typeID;  //typeID to spawn
};

/* POD structure for spawn entries */
struct SpawnEntry {     // notes for me while creating/writing/testing
    bool enabled;       // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
    uint8 spawnClass;   // spawn class.  0 = none, 1-7 = easy to insance based on sysSec, 8 = hauler, 9 = commander, 10 = officer  - 20+ are anomalies
    uint8 spawnGroup;   // spawn group.   1 = roaming, 2 = static, 3 = anomaly, 4 = combat, 5 = deadspace, 6 = mission, 7 = incursion, 8 = sleeper, 9 = escalation
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rat's number in group (to match up with above total)
    uint8 level;        // spawn data subtype/wave
    uint8 classID;      // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint16 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint16 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint16 stamp;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
    uint32 itemID;      // rat entity id
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
};

/* POD structure for spawn faction groups */
struct RatFactionGroups {  // notes for me while creating/writing/testing
    uint8 shipClass;      // shipClass as defined in Spawn::Class
    uint16 groupID;     // item groupID
};

/* POD structure for spawn classes */
struct RatSpawnClass { // notes for me while creating/writing/testing
    uint8 type;     // this is spawn type.  Spawn::Type 1 - 10
    uint8 sub;      // this is spawn subtype.  ship grouping.  varies.  enables loop for random pick. no notes
    uint8 f;        // frigate
    uint8 af;       // advanced frigate
    uint8 d;        // destroyer
    uint8 c;        // cruiser
    uint8 ac;       // advanced cruiser
    uint8 bc;       // battlecruiser
    uint8 bs;       // battleship
    uint8 h;        // hauler
    uint8 o;        // officer - swarm for rogue drones
    uint8 cf;       // commander frigate
    //uint8 acf;      // advanced commander frigate
    uint8 cd;       // commander destroyer
    uint8 cc;       // commander cruiser
    uint8 cbc;      // commander battlecruiser
    uint8 cbs;      // commander battleship
};

/* POD structure for loot groups */
struct LootGroup {
    //uint32 groupID;
    uint32 lootGroupID;
    double dropChance;
};

/* POD structure for loot types */
struct LootGroupType {
    uint8 metaLevel;
    uint32 lootGroupID;
    uint32 typeID;
    uint32 minQuantity;
    uint32 maxQuantity;
};

/* POD structure for possible loot drops */
struct LootList {
    uint8 minDrop;
    uint8 maxDrop;
    uint32 itemID;
};

/* POD structure for statistic data */
struct StatisticData {
    uint16 span;        // 45.5d in minutes (max)
    uint16 shipsSalvaged;
    uint16 probesLaunched;
    uint16 sitesScanned;
    uint16 ramJobs;
    uint32 pcShots;
    uint32 pcMissiles;
    double pcBounties;
    double npcBounties;
    double oreMined;
    double iskMarket;
};


/* POD structure for systems. */
struct SystemData {
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 factionID;
    int64 radius;
    float securityRating;
    std::string name;
    std::string securityClass;
};

struct SystemKillData {
    uint16 killsHour;
    uint16 kills24Hour;
    uint16 factionKills;
    uint16 factionKills24Hour;
    uint16 podKillsHour;
    uint16 podKills24Hour;

    int64 killsDateTime;
    int64 kills24DateTime;
    int64 factionDateTime;
    int64 faction24DateTime;
    int64 podDateTime;
    int64 pod24DateTime;
};

/* POD structure for static items. */
struct StaticData {
    uint16 typeID;
    uint32 itemID;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    float radius;
    GPoint position;
};

/* POD structure for attribute type data. */
struct AttrData {
    uint8 categoryID;
    uint8 attributeCategory;
    uint16 attributeID;
    std::string attributeName;
    std::string displayName;
};


/* POD structure for sovereignty */
struct SovereigntyData {
    uint32 claimID;
    uint32 solarSystemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 corporationID;
    uint32 allianceID;
    uint32 claimStructureID;
    int64 claimTime;
    uint32 hubID;
    uint32 beaconID;
    uint32 jammerID;
    uint8 contested;
    uint8 stationCount;
    uint8 militaryPoints;
    uint8 industrialPoints;
};

/* Tags for sov multi-index container */
struct SovDataBySolarSystem {};
struct SovDataByConstellation {};
struct SovDataByRegion {};
struct SovDataByAlliance {};
struct SovDataByClaim {};

/* POD structure for stations. */
struct StationData {
    bool conquerable :1;
    uint8 officeSlots;
    uint8 operationID;
    uint16 typeID;
    uint16 graphicID;
    uint16 descriptionID;
    uint16 hangarGraphicID;
    uint16 dockingBayGraphicID;
    uint16 reprocessingHangarFlag;
    int32 officeRentalFee;
    uint32 stationID;
    uint32 corporationID;
    uint32 maxShipVolumeDockable;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 orbitID;
    int64 serviceMask;
    float radius;
    float security;
    float dockingCostPerVolume;
    float reprocessingEfficiency;
    float reprocessingStationsTake;
    GPoint position;
    GPoint dockEntry;
    GPoint dockPosition;
    GVector dockOrientation;
    std::string name;
    std::string description;
};

/* POD structure for corp office data */
// this is used by multiple systems.  keep here instead of in corpData.h
struct OfficeData {
    bool lockDown :1;
    uint32 officeID;
    uint32 folderID;
    uint16 typeID;
    uint32 stationID;
    uint32 corporationID;
    int64 rentalFee;
    int64 expiryTime;
    std::string ticker;
    std::string name;
};

/* POD structure for loading dynamic items */
struct OwnerData {
    uint32          ownerID;
    uint32          corpID;
    uint32          locID;
};

/* POD structure for container faction data */
struct FactionData {
    int32 allianceID;
    int32 factionID;
    uint32 ownerID;
    uint32 corporationID;
};

/* structure for type attributes */
struct DmgTypeAttribute {
    uint16 attributeID;
    EvilNumber value;
};

/* structure for loading static system items */
struct DBSystemEntity {
    uint16 typeID;
    uint16 groupID;
    uint32 itemID;
    double radius;
};

struct DBSystemDynamicEntity {
    uint8 categoryID;
    uint16 typeID;
    uint16 groupID;
    int32 allianceID;
    int32 factionID;
    uint32 itemID;
    uint32 ownerID;
    uint32 corporationID;
    uint32 planetID;
    GPoint position;
    std::string itemName;
};

struct DBGPointEntity {
    uint8 idx;
    uint32 itemID;
    double radius;
    double x;
    double y;
    double z;
    GPoint position;
};

/* POD structure for decoded probe data */
struct ProbeData {
    uint8 state;
    uint8 rangeStep;
    int64 expiry;
    float scanRange;
    GPoint dest;
};


#endif  // _EVEMU_POD_CONTAINERS_H_
