 /**
  * @name DungeonMgr.cpp
  *     Dungeon management system for EVEmu
  *
  * @Author:        James
  * @date:          13 December 2022
  */
 
#include "eve-server.h"

#include "EVEServerConfig.h"

#include "StaticDataMgr.h"
#include "dungeon/DungeonDB.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include <cstring>

// NPC ship types in this Crucible data set use the Entity category (11) and
// faction rat groups named "Asteroid ...". Keep ordinary Entity-category
// scenery and structures out of the NPC selection path.
static bool IsMissionNPCGroup(const Inv::GrpData& group)
{
    return group.catID == EVEDB::invCategories::Ship
        || group.catID == EVEDB::invCategories::Drone
        || (group.catID == EVEDB::invCategories::Entity
            && group.name.compare(0, 9, "Asteroid ") == 0);
}

/*
Dungeon flow:
1. load dungeons from db on init
2. when signature/anomaly is created, spawn first room of dungeon at ship position (how to handle multi-room dungeons?)

Multi-room dungeons have each room with acceleration gate pointing to next room (accel gate is not implemented????)

*/


DungeonDataMgr::DungeonDataMgr()
    : m_dungeonID(0)
{
    m_dungeons.clear();
}

int DungeonDataMgr::Initialize()
{
    // Loads all dungeons from database upon server initialisation
    Populate();
    sLog.Blue("       DunDataMgr", "Dungeon Data Manager Initialized.");
    return 1;
}

void DungeonDataMgr::UpdateDungeon(uint32 dungeonID)
{
    _log(DUNG__INFO, "UpdateDungeon() - Updating dungeon %u's object in DataMgr...", dungeonID);
    // Get dungeon from DB by dungeonID
    DBQueryResult *res = new DBQueryResult();
    DBResultRow row;
    DungeonDB::GetAllDungeonDataByDungeonID(*res, dungeonID);

    // Multi-index view by dungeonID
    auto &byDungeonID = m_dungeons.get<Dungeon::DungeonsByID>();

    // Update a dungeon's in-memory object
    auto it = byDungeonID.find(dungeonID);
    if (it != byDungeonID.end()) {
        // Dungeon already exists, update in-memory object
        byDungeonID.erase(dungeonID);

        if (res->GetRowCount() > 0) {
            while (res->GetRow(row))
            {
                CreateDungeon(row);
                FillObject(row);
            }
        } else {
            _log(DUNG__ERROR, "UpdateDungeon() - Failed to find dungeon %u in database. This should never happen.", dungeonID);
        }

    } else {
        _log(DUNG__ERROR, "UpdateDungeon() - Failed to find dungeon %u's object in DataMgr. This should never happen.", dungeonID);
    }
    SafeDelete(res);
}

void DungeonDataMgr::GetRandomDungeon(Dungeon::Dungeon& dungeon, uint8 archetype) {
    // Get the index for the archetype ID
    auto& archetypeIndex = m_dungeons.get<Dungeon::DungeonsByArchetype>();
    // Get the range of all dungeons with the specified archetype ID
    auto range = archetypeIndex.equal_range(archetype);
    // Calculate the number of dungeons in the range
    uint32 count = std::distance(range.first, range.second);
    // If there are no dungeons with the specified archetype, return
    if (count == 0) {
        return;
    }
    // Generate a random number within the range of the number of dungeons
    uint32 randomIndex = rand() % count;
    // Get the iterator to the random dungeon
    auto it = range.first;
    std::advance(it, randomIndex);
    // Assign the selected dungeon to the output parameter
    dungeon = *it;
}

void DungeonDataMgr::GetDungeon(Dungeon::Dungeon& dungeon, uint32 dungeonID) {
    // Multi-index view by dungeonID
    auto &byDungeonID = m_dungeons.get<Dungeon::DungeonsByID>();

    auto it = byDungeonID.find(dungeonID);
        if (it != byDungeonID.end())
        {
            dungeon = *it;
        } else {
            _log(DUNG__ERROR, "GetDungeon() - Failed to find dungeon with id %u", dungeonID);
        }
}

bool DungeonDataMgr::GetRandomMissionDungeon(Dungeon::Dungeon& dungeon, uint32 factionID)
{
    std::vector<Dungeon::Dungeon> candidates;
    std::vector<Dungeon::Dungeon> factionCandidates;

    // Archetype 20 is the static-data archetype for agent mission dungeons.
    // Templates retain all room objects. Select only rooms containing known NPC
    // groups so mining-only and scenery-only rooms cannot become combat pockets.
    for (const Dungeon::Dungeon& candidate : m_dungeons) {
        if (candidate.archetypeID != 20 || candidate.rooms.empty())
            continue;

        bool hasNPC = false;
        for (const auto& room : candidate.rooms) {
            for (const Dungeon::RoomObject& object : room.second.objects) {
                // dunRoomObjects.groupID is a dungeon-editor group ID, not
                // invGroups.groupID. Resolve the NPC category via typeID.
                Inv::TypeData type = {};
                Inv::GrpData group = {};
                sDataMgr.GetType(object.typeID, type);
                if (type.id == 0)
                    continue;
                sDataMgr.GetGroup(type.groupID, group);
                if (group.id != 0 && IsMissionNPCGroup(group)) {
                    hasNPC = true;
                    break;
                }
            }
            if (hasNPC)
                break;
        }
        if (!hasNPC)
            continue;

        candidates.push_back(candidate);
        if (factionID != 0 && candidate.factionID == factionID)
            factionCandidates.push_back(candidate);
    }

    const std::vector<Dungeon::Dungeon>& selection =
        factionCandidates.empty() ? candidates : factionCandidates;
    if (selection.empty()) {
        _log(DUNG__WARNING, "GetRandomMissionDungeon() - No mission templates with NPC room objects are loaded.");
        return false;
    }

    dungeon = selection[MakeRandomInt(0, static_cast<uint32>(selection.size() - 1))];
    return true;
}


bool DungeonDataMgr::GetRandomMissionDungeonForGroup(Dungeon::Dungeon& dungeon, const char* groupPrefix)
{
    std::vector<Dungeon::Dungeon> candidates;
    if (groupPrefix == nullptr || groupPrefix[0] == '\0')
        return false;

    uint32 missionTemplateCount = 0;
    uint32 firstRoomObjectCount = 0;
    uint32 validTypeCount = 0;
    uint32 entityGroupCount = 0;
    uint32 matchingGroupCount = 0;
    // Capture real static-data names/IDs from loaded room objects. This is
    // diagnostic only: selection remains strict until the actual data proves
    // which inventory groups represent the requested faction.
    std::string entityGroupSamples;
    uint32 entityGroupSampleCount = 0;

    // The Blockade test must use Angel Cartel room templates even when the
    // issuing security agent belongs to another empire faction. Require NPCs
    // in the first room because that is the room instantiated on acceptance.
    for (const Dungeon::Dungeon& candidate : m_dungeons) {
        if (candidate.archetypeID != 20 || candidate.rooms.empty())
            continue;

        ++missionTemplateCount;
        const Dungeon::Room& firstRoom = candidate.rooms.begin()->second;
        firstRoomObjectCount += static_cast<uint32>(firstRoom.objects.size());
        bool hasRequestedNPC = false;
        for (const Dungeon::RoomObject& object : firstRoom.objects) {
            // dunRoomObjects.groupID is the dungeon editor's room-object
            // grouping (dunGroups), not necessarily invGroups.groupID.
            // Resolve the inventory group through the object's typeID.
            Inv::TypeData type = {};
            Inv::GrpData group = {};
            sDataMgr.GetType(object.typeID, type);
            if (type.id == 0)
                continue;
            ++validTypeCount;
            sDataMgr.GetGroup(type.groupID, group);
            if (group.id == 0)
                continue;
            if (group.catID == EVEDB::invCategories::Entity) {
                ++entityGroupCount;
                if (entityGroupSampleCount < 30
                        && entityGroupSamples.find(group.name) == std::string::npos) {
                    if (!entityGroupSamples.empty())
                        entityGroupSamples += "; ";
                    entityGroupSamples += std::to_string(group.id) + ":" + group.name
                        + " [type " + std::to_string(type.id) + " " + type.name + "]";
                    ++entityGroupSampleCount;
                }
            }
            if (group.catID == EVEDB::invCategories::Entity
                    && group.name.compare(0, std::strlen(groupPrefix), groupPrefix) == 0) {
                ++matchingGroupCount;
                hasRequestedNPC = true;
                break;
            }
        }
        if (hasRequestedNPC)
            candidates.push_back(candidate);
    }

    if (candidates.empty()) {
        // Use the spawn error category so this actionable failure is visible
        // alongside the final mission-spawn error, even when DUNG logging is filtered.
        _log(SPAWN__ERROR,
            "Mission template selection failed for group '%s': loaded templates=%u, first-room objects=%u, valid types=%u, Entity groups=%u, matching groups=%u.",
            groupPrefix, missionTemplateCount, firstRoomObjectCount, validTypeCount,
            entityGroupCount, matchingGroupCount);
        _log(SPAWN__ERROR, "Mission Entity group samples (up to 30): %s",
            entityGroupSamples.empty() ? "<none>" : entityGroupSamples.c_str());
        return false;
    }

    _log(DUNG__MESSAGE, "GetRandomMissionDungeonForGroup() - Matched %u of %u mission templates for '%s'.",
        static_cast<uint32>(candidates.size()), missionTemplateCount, groupPrefix);
    dungeon = candidates[MakeRandomInt(0, static_cast<uint32>(candidates.size() - 1))];
    return true;
}

void DungeonDataMgr::Populate()
{
    // Populate dungeon datasets from DB
    double start = GetTimeMSeconds();
    DBQueryResult *res = new DBQueryResult();
    DBResultRow row;

    // Multi-index view by dungeonID
    auto &byDungeonID = m_dungeons.get<Dungeon::DungeonsByID>();

    DungeonDB::GetAllDungeonData(*res);
    while (res->GetRow(row))
    {
        CreateDungeon(row);
        FillObject(row);
    }

    // Mission content in this data set uses dunTemplates + dunRoomObjects,
    // not the legacy dunDungeons + dunRooms tables. Import all room objects
    // into the same in-memory representation; no SQL data is written.
    DBQueryResult* templateRes = new DBQueryResult();
    uint32 missionTemplateRowCount = 0;
    uint32 missionTemplateObjectCount = 0;
    uint32 missionTemplateCollisionCount = 0;
    DungeonDB::GetMissionTemplateData(*templateRes);
    while (templateRes->GetRow(row))
    {
        ++missionTemplateRowCount;
        const uint32 templateID = row.GetUInt(0);
        // A legacy dungeon with the same ID takes precedence. Count these
        // skipped rows explicitly: otherwise an ID collision can make an
        // entire mission template disappear without a useful runtime clue.
        if (byDungeonID.find(templateID) != byDungeonID.end()) {
            ++missionTemplateCollisionCount;
            continue;
        }
        CreateDungeon(row);
        FillObject(row);
        ++missionTemplateObjectCount;
    }
    _log(SPAWN__MESSAGE,
        "Mission template import: query rows=%u, imported room-object rows=%u, skipped legacy-ID collisions=%u.",
        missionTemplateRowCount, missionTemplateObjectCount, missionTemplateCollisionCount);
    if (missionTemplateCollisionCount != 0) {
        _log(SPAWN__WARNING,
            "Mission template import skipped %u rows because their template IDs already exist as legacy dungeon IDs.",
            missionTemplateCollisionCount);
    }
    SafeDelete(templateRes);

    sLog.Cyan("       DunDataMgr", "%lu Dungeon data sets loaded in %.3fms.",
              byDungeonID.size(), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
}

void DungeonDataMgr::CreateDungeon(DBResultRow row) {
    // Multi-index view by dungeonID
    auto &byDungeonID = m_dungeons.get<Dungeon::DungeonsByID>();

    // Check if dungeon already exists for this object
    auto it = byDungeonID.find(row.GetUInt(0));
    if (it != byDungeonID.end()) {
        Dungeon::Dungeon dData = *it;
        // Check if room already exists for this object
        if (dData.rooms.find(row.GetUInt(5)) == dData.rooms.end()) {
            // Create new room
            Dungeon::Room rData;
            rData.roomID = row.GetUInt(5);
            dData.rooms.insert({rData.roomID, rData});

            // Replace record in container
            byDungeonID.erase(dData.dungeonID);
            byDungeonID.insert(dData);
        }
    } else {
        // Create dungeon and room
        Dungeon::Dungeon dData;
        Dungeon::Room rData;
        rData.roomID = row.GetUInt(5);
        dData.rooms.insert({rData.roomID, rData});
        dData.dungeonID = row.GetUInt(0);
        byDungeonID.insert(dData);
    }
}

void DungeonDataMgr::FillObject(DBResultRow row) {
    /*    if (!sDatabase.RunQuery(res, "SELECT dunDungeons.dungeonID "
    "dungeonName, dungeonStatus, factionID, archetypeID, "
    "dunRooms.roomID, dunRooms.roomName, objectID, typeID, groupID, "
    "x, y, z, yaw, pitch, roll, radius "
    "FROM ((dunDungeons "
    "INNER JOIN dunRooms ON dunDungeons.dungeonID = dunRooms.dungeonID) "
    "INNER JOIN dunRoomObjects ON dunRooms.roomID = dunRoomObjects.roomID)"))*/

    // Multi-index view used for inserting
    auto &byDungeonID = m_dungeons.get<Dungeon::DungeonsByID>();

    auto it = byDungeonID.find(row.GetUInt(0));
    if (it != byDungeonID.end()) {
        Dungeon::Dungeon dData = *it;

        // Add the object to the room
        Dungeon::RoomObject oData;
        oData.objectID = row.GetUInt(7);
        oData.typeID = row.GetUInt(8);
        oData.groupID = row.GetUInt(9);
        oData.x = row.GetDouble(10);
        oData.y = row.GetDouble(11);
        oData.z = row.GetDouble(12);
        oData.yaw = row.GetDouble(13);
        oData.pitch = row.GetDouble(14);
        oData.roll = row.GetDouble(15);
        oData.radius = row.GetDouble(16);
        dData.rooms[row.GetUInt(5)].objects.push_back(oData);
        // Populate all data for the dungeon and room
        dData.name = row.GetText(1);
        dData.status = row.GetUInt(2);
        dData.factionID = row.GetUInt(3);
        dData.archetypeID = row.GetUInt(4);
        dData.rooms[row.GetUInt(5)].roomID = row.GetUInt(5);
        dData.rooms[row.GetUInt(5)].roomName = row.GetText(6);

        // Replace item in container
        byDungeonID.erase(dData.dungeonID);
        byDungeonID.insert(dData);
    } 
    else {
        _log(DUNG__ERROR, "FillObject() - Failed to find dungeon %u's object in DataMgr. This should never happen.", row.GetUInt(0));
    }
}

const char* DungeonDataMgr::GetDungeonType(int8 typeID)
{
    // Return the string representation of the given dungeon type ID
    switch (typeID) {
        case Dungeon::Type::Mission:        return "Mission";
        case Dungeon::Type::Gravimetric:    return "Gravimetric";
        case Dungeon::Type::Magnetometric:  return "Magnetometric";
        case Dungeon::Type::Radar:          return "Radar";
        case Dungeon::Type::Ladar:          return "Ladar";
        case Dungeon::Type::Rated:          return "Rated";
        case Dungeon::Type::Anomaly:        return "Anomaly";
        case Dungeon::Type::Unrated:        return "Unrated";
        case Dungeon::Type::Escalation:     return "Escalation";
        case Dungeon::Type::Wormhole:       return "Wormhole";
        default:                            return "Invalid";
    }
}


DungeonMgr::DungeonMgr(SystemManager* mgr, EVEServiceManager& svc)
: m_system(mgr),
m_services(svc),
m_anomMgr(nullptr),
m_spawnMgr(nullptr),
m_initalized(false)
{
}

DungeonMgr::~DungeonMgr()
{
    // TODO: clean up and free any resources
}

bool DungeonMgr::Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr)
{
    if (!m_initalized)
    {
        m_anomMgr = anomMgr;
        m_spawnMgr = spawnMgr;

        if (m_anomMgr == nullptr) {
            _log(COSMIC_MGR__ERROR, "System Init Fault. anomMgr == nullptr.  Not Initializing Dungeon Manager for %s(%u)", m_system->GetName(), m_system->GetID());
            return m_initalized;
        }

        if (m_spawnMgr == nullptr) {
            _log(COSMIC_MGR__ERROR, "System Init Fault. spawnMgr == nullptr.  Not Initializing Dungeon Manager for %s(%u)", m_system->GetName(), m_system->GetID());
            return m_initalized;
        }

        if (!sConfig.cosmic.DungeonEnabled){
            _log(COSMIC_MGR__INIT, "Dungeon System Disabled.  Not Initializing Dungeon Manager for %s(%u)", m_system->GetName(), m_system->GetID());
            return true;
        }

        m_spawnMgr->SetDungMgr(this);

        _log(COSMIC_MGR__INIT, "DungeonMgr Initialized for %s(%u)", m_system->GetName(), m_system->GetID());

        m_initalized = true;
        return true;
    }
    return false;
}

void DungeonMgr::Process()
{
    // TODO: process and update any active dungeons in the system
    if (!m_initalized)
        return;
}

bool DungeonMgr::MakeDungeon(CosmicSignature& sig, uint32 dungeonID)
{
    // TODO: create a new dungeon using the given signature

    Dungeon::Dungeon dData;

    // If we are given a dungeonID, use it otherwise pick a random dungeon based on archetype
    if (dungeonID == 0) {
        sDunDataMgr.GetRandomDungeon(dData, sig.dungeonType);
    } else {
        sDunDataMgr.GetDungeon(dData, dungeonID);
    }

    // Invalid templates must not create success-looking empty combat sites.
    // Gravimetric sites can use BeltMgr without a dungeon template.
    if (dData.dungeonID == 0 && sig.dungeonType != Dungeon::Type::Gravimetric) {
        _log(COSMIC_MGR__WARNING, "Missing anomaly dungeon template (type %u) in system %u.",
             sig.dungeonType, m_system->GetID());
        return false;
    }
    if ((sig.sigGroupID == EVEDB::invGroups::Cosmic_Signature) || (sig.sigGroupID == EVEDB::invGroups::Cosmic_Anomaly)) {
        // Create a new anomaly inventory item to track entire dungeon under
        ItemData iData(sig.sigTypeID, sig.ownerID, sig.systemID, flagNone, sig.sigName.c_str(), sig.position/*, info*/);

        InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
        if (iRef.get() == nullptr)
            return false;
        iRef->SetCustomInfo(std::string("livedungeon").c_str());
        iRef->SaveItem();

        CelestialSE* cSE = new CelestialSE(iRef, m_system->GetServiceMgr(), m_system);

        if (cSE == nullptr)
            return false;

        // dont add signal thru sysMgr.  signal is added when this returns to anomMgr
        m_system->AddEntity(cSE, false);
        sig.sigItemID = iRef->itemID();
        sig.bubbleID = cSE->SysBubble()->GetID();

        _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - %s using dungeonID %u", sig.sigName.c_str(), dData.dungeonID);

        // Create the new live dungeon
        Dungeon::LiveDungeon newDungeon;
        newDungeon.anomalyID = iRef->itemID();
        newDungeon.systemID = iRef->itemID();

        // Iterate through rooms and handle item spawning for each room
        uint16 roomCounter = 0;
        for (auto const& room : dData.rooms) {
            Dungeon::LiveRoom newRoom;
            // Set room position
            // Handle first room differently as it will be at the origin point of signature
            if (roomCounter == 0) {
                newRoom.position = sig.position;
            } else {
                // The following rooms shall be 100M kilometers in x direction from previous room.
                GPoint pos;
                pos.x = newDungeon.rooms[roomCounter - 1].position.x + NEXT_DUNGEON_ROOM_DIST;
                pos.y = newDungeon.rooms[roomCounter - 1].position.y;
                pos.z = newDungeon.rooms[roomCounter - 1].position.z;
                newRoom.position = pos;
            }

            for (auto object : room.second.objects ) {
                GPoint pos;
                // Set position for each object
                pos.x = newRoom.position.x + object.x;
                pos.y = newRoom.position.y + object.y;
                pos.z = newRoom.position.z + object.z;

                // If invGroup is an NPC, we must spawn it as such
                Inv::TypeData objType;
                Inv::GrpData objGroup;
                sDataMgr.GetType(object.typeID, objType);
                sDataMgr.GetGroup(objType.groupID, objGroup);
                if (objGroup.catID == EVEDB::invCategories::Ship || objGroup.catID == EVEDB::invCategories::Drone) {
                    m_spawnMgr->DoSpawnForAnomaly(sBubbleMgr.FindBubble(m_system->GetID(), pos), pos, GetRandLevel(), object.typeID);
                } 
                
                // Otherwise, spawn as a normal celestial object
                else {
                    // Define ItemData object for each RoomObject
                    ItemData dData(object.typeID, sig.ownerID, sig.systemID, flagNone, sDataMgr.GetTypeName(object.typeID), pos);

                    // Spawn the object (and persist it across system unloads)
                    iRef = sItemFactory.SpawnItem(dData);
                    if (iRef.get() == nullptr) {
                        _log(COSMIC_MGR__ERROR, "DungeonMgr::Create() - Unable to spawn item with type %s for room %u dungeon with anomaly itemID %u", sDataMgr.GetTypeName(object.typeID), roomCounter, newDungeon.anomalyID);
                        return false;
                    }
                    iRef->SetCustomInfo(("livedungeon_" + std::to_string(newDungeon.anomalyID)).c_str());
                    iRef->SaveItem();

                    cSE = new CelestialSE(iRef, m_system->GetServiceMgr(), m_system);
                    m_system->AddEntity(cSE, false);
                    newRoom.items.push_back(iRef->itemID());
                }
            }

            newDungeon.rooms.insert({roomCounter, newRoom});
            roomCounter++;
        }

        // Gravimetric sites need mineable AsteroidSE entities; the existing
        // BeltMgr creates those using this system's normal ore distribution.
        if (sig.dungeonType == Dungeon::Type::Gravimetric) {
            std::unordered_multimap<float, uint16> ore;
            sDataMgr.GetRoidDist(m_system->GetSystemSecurityClass(), ore);
            if (ore.empty() || m_system->GetBeltMgr() == nullptr
                || !m_system->GetBeltMgr()->Create(sig, ore)) {
                _log(COSMIC_MGR__WARNING, "Unable to create IHub mining site in system %u.",
                     m_system->GetID());
                return false;
            }
        }
        // Finally add the new dungeon to the system-wide list for tracking
        m_dungeonList.insert({newDungeon.anomalyID, newDungeon});
        return true; // Previously always returned false after successful spawn.
    }
    return false;
}

int8 DungeonMgr::GetFaction(uint32 factionID)
{
    switch (factionID) {
        case factionAngel:          return 2;
        case factionSanshas:        return 5;
        case factionGuristas:       return 4;
        case factionSerpentis:      return 1;
        case factionBloodRaider:    return 3;
        case factionRogueDrones:    return 6;
        case 0:                     return 7;
        // these are incomplete.  set to default (region rat)
        case factionAmarr:
        case factionAmmatar:
        case factionCaldari:
        case factionGallente:
        case factionMinmatar:
        default:
            return GetFaction(sDataMgr.GetRegionRatFaction(m_system->GetRegionID()));
    }
}

int8 DungeonMgr::GetRandLevel()
{
    double level = MakeRandomFloat();
    _log(DUNG__TRACE, "DungeonMgr::GetRandLevel() - level = %.2f", level);

    if (level < 0.15) {
        return 4;
    } else if (level < 0.25) {
        return 3;
    } else if (level < 0.50) {
        return 2;
    } else {
        return 1;
    }
}
