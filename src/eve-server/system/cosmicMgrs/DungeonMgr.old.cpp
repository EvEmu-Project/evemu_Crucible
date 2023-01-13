
 /**
  * @name DungeonMgr.cpp
  *     Dungeon management system for EVEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  * @updated:       27 August 2017
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "math/Trig.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"

DungeonDataMgr::DungeonDataMgr()
: m_dungeonID(DUNGEON_ID)
{
}

int DungeonDataMgr::Initialize()
{
    // for now, we are deleting all saved dungeons on startup.  will fix this later as system matures.
    ManagerDB::ClearDungeons();

    Populate();
    sLog.Blue("   DungeonDataMgr", "Dungeon Data Manager Initialized.");
    return 1;
}

void DungeonDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    ManagerDB::GetDunTemplates(*res);
    while (res->GetRow(row)) {
        //SELECT dunTemplateID, dunTemplateName, dunEntryID, dunSpawnID, dunRoomID, dunTemplateDescription, dunTemplateFactionID FROM dunTemplates
        Dungeon::Template dtemplates = Dungeon::Template();
        dtemplates.dunName = row.GetText(1);
        dtemplates.dunEntryID = row.GetInt(2);
        dtemplates.dunSpawnClass = row.GetInt(3);
        dtemplates.dunRoomID = row.GetInt(4);
        dtemplates.dunDescription = row.GetText(5);
        dtemplates.dunFactionID = row.GetInt(6);
        templates.emplace(row.GetInt(0), dtemplates);
    }

    //res->Reset();
    ManagerDB::GetDunRoomData(*res);
    while (res->GetRow(row)) {
        // SELECT dunRoomID, dunGroupID, xpos, ypos, zpos FROM dunRoomData
        Dungeon::RoomData drooms = Dungeon::RoomData();
        drooms.dunGroupID = row.GetInt(1);
        drooms.x = row.GetInt(2);
        drooms.y = row.GetInt(3);
        drooms.z = row.GetInt(4);
       rooms.emplace(row.GetInt(0), drooms);
    }

    //res->Reset();
    ManagerDB::GetDunGroupData(*res);
    while (res->GetRow(row)) {
        // SELECT d.dunGroupID, d.itemTypeID, d.itemGroupID, t.typeName, t.groupID, g.categoryID, t.radius, d.xpos, d.ypos, d.zpos FROM dunGroupData
        Dungeon::GroupData dgroups = Dungeon::GroupData();
        dgroups.typeID = row.GetInt(1);
        dgroups.typeName = row.GetText(3);
        dgroups.typeGrpID = row.GetInt(4);
        dgroups.typeCatID = row.GetInt(5);
        dgroups.radius = row.GetInt(6);
        dgroups.x = row.GetInt(7);
        dgroups.y = row.GetInt(8);
        dgroups.z = row.GetInt(9);
        groups.emplace(row.GetInt(0), dgroups);
    }

    //res->Reset();
    ManagerDB::GetDunEntryData(*res);
    while (res->GetRow(row)) {
        //SELECT dunEntryID, xpos, ypos, zpos FROM dunEntryData
        Dungeon::EntryData dentry = Dungeon::EntryData();
        dentry.x = row.GetInt(1);
        dentry.y = row.GetInt(2);
        dentry.z = row.GetInt(3);
        entrys.emplace(row.GetInt(0), dentry);
    }

    /* not ready yet
    //res->Reset();
    ManagerDB::GetDunSpawnInfo(*res);
    while (res->GetRow(row)) {
        //SELECT dunRoomSpawnID, dunRoomSpawnType, xpos, ypos, zpos
        Dungeon::RoomSpawnInfo spawn = Dungeon::RoomSpawnInfo();
        spawn.dunRoomSpawnID = row.GetInt(0);
        spawn.dunRoomSpawnType = row.GetInt(1);
        spawn.x = row.GetInt(2);
        spawn.y = row.GetInt(3);
        spawn.z = row.GetInt(4);
        groups.emplace(spawn.dunRoomSpawnID, spawn);
    } */

    // sort/save template room/group data to avoid compilation later?

    //cleanup
    SafeDelete(res);

    //sLog.Cyan("   DungeonDataMgr", "%u rooms in %u buckets and %u groups in %u buckets for %u dungeon templates loaded in %.3fms.",\
              rooms.size(), rooms.bucket_count(), groups.size(), groups.bucket_count(), templates.size(), (GetTimeMSeconds() - start));

    sLog.Cyan("   DungeonDataMgr", "%u entrys, %u rooms and %u groups for %u dungeon templates loaded in %.3fms.",\
              entrys.size(), rooms.size(), groups.size(), templates.size(), (GetTimeMSeconds() - start));
}

void DungeonDataMgr::AddDungeon(Dungeon::ActiveData& dungeon)
{
    activeDungeons.emplace(dungeon.systemID, dungeon);
    _log(COSMIC_MGR__DEBUG, "Added Dungeon %u (%u) in systemID %u to active dungeon list.", dungeon.dunItemID, dungeon.dunTemplateID, dungeon.systemID);
    //ManagerDB::SaveActiveDungeon(dungeon);
}

void DungeonDataMgr::GetDungeons(std::vector<Dungeon::ActiveData>& dunList)
{
    for (auto cur : activeDungeons)
        dunList.push_back(cur.second);
}

bool DungeonDataMgr::GetTemplate(uint32 templateID, Dungeon::Template& dTemplate) {
    std::map<uint32, Dungeon::Template>::iterator itr = templates.find(templateID);
    if (itr == templates.end()) {
        _log(COSMIC_MGR__ERROR, "DungeonDataMgr - templateID %u not found.", templateID);
        return false;
    }
    dTemplate = itr->second;
    return true;
}

const char* DungeonDataMgr::GetDungeonType(int8 typeID)
{
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


DungeonMgr::DungeonMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
m_services(svc),
m_anomMgr(nullptr),
m_spawnMgr(nullptr),
m_initalized(false)
{
    m_anomalyItems.clear();
}

DungeonMgr::~DungeonMgr()
{
    //for now we're deleting everything till i can write proper item handling code

    /*  this is not needed as all items are temp at this time.
    for (auto cur : m_dungeonList)
        for (auto item : cur.second)
            InventoryDB::DeleteItem(item);
     */
}

bool DungeonMgr::Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr)
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
    Load();

    _log(COSMIC_MGR__INIT, "DungeonMgr Initialized for %s(%u)", m_system->GetName(), m_system->GetID());

    return (m_initalized = true);
}

// called from systemMgr
void DungeonMgr::Process() {
    if (!m_initalized)
        return;

    // this is used to remove empty/completed/timed-out dungeons....eventually
}

void DungeonMgr::Load()
{
    std::vector<Dungeon::ActiveData> dungeons;
    ManagerDB::GetSavedDungeons(m_system->GetID(), dungeons);
    /** @todo this will need more work as the system matures...
    for(auto dungeon : dungeons) {
        InventoryItemRef dungeonRef = sItemFactory.GetItemRef( dungeon.dungeonID );
        if ( !dungeonRef ) {
            _log(COSMIC_MGR__WARNING, "DungeonMgr::Load() -  Unable to spawn dungeon item #%u:'%s' of type %u.", dungeon.dungeonID, dungeon.typeID);
            continue;
        }
        AsteroidSE* asteroidObj = new AsteroidSE( dungeonRef, *(m_system->GetServiceMgr()), m_system );
        if ( !asteroidObj ) {
            _log(COSMIC_MGR__WARNING, "DungeonMgr::Load() -  Unable to spawn dungeon entity #%u:'%s' of type %u.", dungeon.dungeonID, dungeon.typeID);
            continue;
        }
        _log(COSMIC_MGR__TRACE, "DungeonMgr::Load() - Loaded dungeon %u, type %u for %s(%u)", dungeon.dungeonID, dungeon.typeID, m_system->GetName(), m_systemID );
        sBubbleMgr.Add( asteroidObj );
        sDunDataMgr.AddDungeon(std::pair<uint32, Dungeon::ActiveData*>(m_system->GetID(), dungeon));
    } */
}

// Create dungeon (dynamically)
bool DungeonMgr::Create(uint32 templateID, CosmicSignature& sig)
{
    Dungeon::Template dTemplate = Dungeon::Template();
    if (!sDunDataMgr.GetTemplate(templateID, dTemplate))
        return false;

    if (dTemplate.dunRoomID == 0) {
        _log(COSMIC_MGR__ERROR, "DungeonMgr::Create() - roomID is 0 for template %u.", templateID);
        return false;
    }

    if ((sig.dungeonType < Dungeon::Type::Wormhole) // 1 - 5
    or  (sig.ownerID == factionRogueDrones)) {
        sig.sigName = dTemplate.dunName;
    } else {
        sig.sigName = sDataMgr.GetFactionName(sig.ownerID);
        sig.sigName += dTemplate.dunName;
    }

    // spawn and save actual anomaly item  // typeID, ownerID, locationID, flag, name, &_position
    /** @todo make specific table for dungeon items:  dungeonID, systemID, entity shit if we decide to keep them. */
    /*
    std::string info = "Dungeon: ";
    info += sig.sigName;
    info += " in ";
    info += m_system->GetName();
    */
    ItemData iData(sig.sigTypeID, sig.ownerID, sig.systemID, flagNone, sig.sigName.c_str(), sig.position/*, info*/);
    InventoryItemRef iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), iData);
    if (iRef.get() == nullptr) // make error and exit
        return false;
    CelestialSE* cSE = new CelestialSE(iRef, *(m_system->GetServiceMgr()), m_system);
    if (cSE == nullptr)
        return false;   //  we'll get over it.
    // dont add signal thru sysMgr.  signal is added when this returns to anomMgr
    m_system->AddEntity(cSE, false);
    sig.sigItemID = iRef->itemID();
    sig.bubbleID = cSE->SysBubble()->GetID();

    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - %s using templateID %u and roomID %i", sig.sigName.c_str(), templateID, dTemplate.dunRoomID);

    /* do we need this?  persistent dungeons?
    if ((typeID == 1) or (typeID == 8) or (typeID == 9) or (typeID == 10)) {
        // setup data to save active dungeon
        ActiveDungeon dungeon = ActiveDungeon();
            dungeon.dunExpiryTime = Win32TimeNow() + (EvE::Time::Day * 3);       // 3 days - i know this isnt right. just for testing.
            dungeon.dunTemplateID = templateID;
            dungeon.dunItemID = sig.sigItemID;
            dungeon.state = 0;  //dunType here.
            dungeon.systemID = sig.systemID;
            dungeon.x = sig.x;
            dungeon.y = sig.y;
            dungeon.z = sig.z;
        sDunDataMgr.AddDungeon(dungeon);
    } */

    /* roomID format.  ABCD
     *      A = roomtype - 1:combat, 2:rescue, 3:capture, 4:salvage, 5:relic, 6:hacking, 7:roids, 8:clouds, 9:recon
     *      B = level -  0:none, 1:f, 2:d, 3:c, 4:af/ad, 5:bc, 6:ac, 7:bs, 8:abs, 9:hard
     *      C = amount/size - 0:code defined 1:small(1-5), 2:medium(2-10), 3:large(5-25), 4:enormous(10-50), 5:colossal(20-100), 6-9:ice
     *      D = faction - 0=drone, 1:Serpentis, 2:Angel, 3:Blood, 4:Guristas, 5:Sansha, 6:Amarr, 7:Caldari, 8:Gallente, 9:Minmatar
     *      D = sublevel - 0:gas, 1-5:ore, 6-9:ice
     */

    int16 x=0, y=0, z=0;
    Dungeon::GroupData grp;
    auto roomRange = sDunDataMgr.rooms.equal_range(dTemplate.dunRoomID);
    for (auto it = roomRange.first; it != roomRange.second; ++it) {
        x = it->second.x;
        y = it->second.y;
        z = it->second.z;
        auto groupRange = sDunDataMgr.groups.equal_range(it->second.dunGroupID);
        for (auto it2 = groupRange.first; it2 != groupRange.second; ++it2) {
            grp.typeCatID = it2->second.typeCatID;
            grp.typeGrpID = it2->second.typeGrpID;
            grp.typeName = it2->second.typeName;
            grp.typeID = it2->second.typeID;
            grp.x = (x + it2->second.x);
            grp.y = (y + it2->second.y);
            grp.z = (z + it2->second.z);
            m_anomalyItems.push_back(grp);
        }
    }
    if (sig.dungeonType == Dungeon::Type::Gravimetric) {
        // dungeon template for grav sites just give 'extra' roid data
        int16 size = m_anomalyItems.size();
        uint8 divisor = 10;
        if (size < 100)
            divisor = 100;
        float chance = (100.0 /size) /divisor;
        if (chance < 0.01)
            chance = 0.01;
        std::unordered_multimap<float, uint16> roidTypes;
        for (auto cur : m_anomalyItems)
            roidTypes.emplace(chance, cur.typeID);

        m_system->GetBeltMgr()->Create(sig, roidTypes);
        // clear out extra roids data to continue with room deco
        m_anomalyItems.clear();
    } else {
        // other types have a chance for roids.
    }

    // create deco items for this dungeon
    CreateDeco(templateID, sig);

    // item spawning method
    uint32 systemID = m_system->GetID();
    std::vector<uint32> items;
    GPoint pos2(NULL_ORIGIN);
    std::vector<Dungeon::GroupData>::iterator itr = m_anomalyItems.begin(), end = m_anomalyItems.end();
    while (itr != end) {
        pos2.x = sig.position.x + itr->x;
        pos2.y = sig.position.y + itr->y;
        pos2.z = sig.position.z + itr->z;
        // typeID, ownerID, locationID, flag, name, &_position
        ItemData dData(itr->typeID, sig.ownerID, systemID, flagNone, itr->typeName.c_str(), pos2);
        iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), dData);
        if (iRef.get() == nullptr) // we'll survive...
            continue;
        // should ALL of these be CelestialSEs?
        cSE = new CelestialSE(iRef, *(m_system->GetServiceMgr()), m_system);
        m_system->AddEntity(cSE, false);
        items.push_back(iRef->itemID());
        ++itr;
    }

    if (dTemplate.dunSpawnClass > 0)
        m_spawnMgr->DoSpawnForAnomaly(sBubbleMgr.FindBubble(m_system->GetID(), sig.position), dTemplate.dunSpawnClass);

    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - dungeonID %u created for %s with %u items.", \
              sig.sigItemID, sig.sigName.c_str(), m_anomalyItems.size());

    m_anomalyItems.clear();
    if (!items.empty())
        m_dungeonList.emplace(sig.sigItemID, items);

    return true;
}

/*
 * Band        1/5     1/10    1/15    1/20    1/25    1/40    1/45    1/60    1/80
 * Percentage  20.0%   10.0%   6.67%   5.0%    4.0%    2.5%    2.22%   1.67%   1.25%
 */

/* templateID format.  ABCDE
 *       A = site - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 6:ded, 7:anomaly, 8:unrated, 9:escalation
 *       B = sec - mission: 1-9 (incomplete); others - sysSec: 1=hi, 2=lo, 3=null, 4=mid;
 *       C = type - grav: ore 0-5, ice 6-9; anomaly: 1-5; mission: 1-9; mag: *see below*; ded: 1-8; ladar/radar: 1-8
 *       D = level - mission: 1-9; grav: ore 1-3, ice 0; mag: *see below*; radar: 1-norm; 2-digital(nullsec); ladar: 1; anomaly: 1-5
 *       E = faction - 0=code defined, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones, 7=region sov, 8=region rat, 9=other
 *
 * NOTE:  mag sites have multiple types and levels based on other variables.
 *      types are defined as relic(1), salvage(2), and drone(3), with salvage being dominant.
 *      for hisec and losec, levels are 1-8 for relic and salvage.  there are no drone mag sites here
 *      for nullsec, relic site levels are 1-8, salvage site levels are 1-4, and drone site levels are 1-7
 *
 * NOTE:  faction can only be 8 for grav and anomaly sites, unless drones (6). all others MUST use 1-6
 */

bool DungeonMgr::MakeDungeon(CosmicSignature& sig)
{
    float secRating = m_system->GetSystemSecurityRating();
    int8 sec = 1; // > 0.6
    if (secRating < -0.2) {
        sec = 3;
    } else if (secRating < 0.2) {
        sec = 4;
    } else if (secRating < 0.6) {
        sec = 2;
    }

    float level(1.0f);
    int8 type(1);
    // need to determine region sov, region rat or other here also
    int8 faction(GetFaction(sig.ownerID));

    using namespace Dungeon::Type;
    switch (sig.dungeonType) {
        case Gravimetric: {       // 2
            faction = 8;  // region rat
            // all roid types can spawn in grav sites.
            level = MakeRandomFloat();
            if (level < 0.1) {
                level = 3;
            } else if (level < 0.3) {
                level = 2;
            } else {
                level = 1;
            }
            // sigStrength depends on roid types as well as trueSec
            if (sec == 1) {             // hi sec
                sig.sigStrength = 0.2 /level;  // 1/5 base
                //sig.sigStrength = 0.0125;        // testing 1/80
                type = MakeRandomInt(0,5);
            } else if (sec == 2) {      // lo sec
                sig.sigStrength = 0.1 /level; // 1/10 base
                type = MakeRandomInt(0,3);
            } else if (sec == 4) {      // mid sec
                sig.sigStrength = 0.0667 /level; // 1/15 base
                type = MakeRandomInt(0,2);
            } else {                    // null sec
                sig.sigStrength = 0.05 /level; // 1/20 base
                type = MakeRandomInt(0,2);
            }
        } break;
        case Magnetometric: {     // 3
            level = MakeRandomFloat();
            if (sec == 3) { // nullsec
                if (level < 0.1) { // 10% to be drone site
                    level = 3;
                    type = MakeRandomInt(1,7);
                    sig.sigStrength = 0.0125; // 1/80
                } else if (level < 0.3) {   // 20% to be relic site
                    level = 1;
                    type = MakeRandomInt(1,8);
                    sig.sigStrength = 0.025; // 1/40
                } else {    // else salvage site
                    level = 1;
                    type = MakeRandomInt(1,4);
                    sig.sigStrength = 0.05; // 1/20
                }
            } else {    // hi, mid and lo sec
                type = MakeRandomInt(1,8);
                if (level < 0.3) {   // 20% to be relic site
                    level = 1;
                    sig.sigStrength = 0.05; // 1/20
                } else {
                    level = 2;
                    sig.sigStrength = 0.1; // 1/10
                }
            }
            if (level == 3) {
                faction = 6;  // drone
            } else if (faction == 6) {
                // lvls 1&2 cannot be drone.  set to region pirate
                faction = GetFaction(sDataMgr.GetRegionRatFaction(m_system->GetRegionID()));
            }
        } break;
        case Radar: {             // 4
            // type 1, level 1 are covert research (ghost sites)
            // level 2 are digital sites and region-specific (only in nullsec)
            // both are incomplete and will be harder than reg sites.
            type = MakeRandomInt(1,8);
            if (sec == 1) {
                sig.sigStrength = 0.1; // 1/10
                if (type == 1) { // Covert Research
                    level = 1;
                    sig.sigStrength = 0.05; // 1/20
                }
            } else if (sec == 2) {
                sig.sigStrength = 0.05; // 1/20
                if (type == 1) { // Covert Research
                    level = 1;
                    sig.sigStrength = 0.025; // 1/40
                }
            } else if (sec == 3) {
                sig.sigStrength = 0.025; // 1/40
                if (faction == 0) {   // this should not hit
                    level = 2;
                    sig.sigStrength = 0.0222; // 1/45
                } else if (type == 1) { // Covert Research
                    level = 1;
                    sig.sigStrength = 0.0167; // 1/60
                }
            }
        } break;
        case Ladar: {             // 5
            faction = 0;
            type = MakeRandomInt(1,8);
            if (sec == 1) {
                sig.sigStrength = 0.1; // 1/10
            } else if (sec == 2) {
                sig.sigStrength = 0.05; // 1/20
            } else if (sec == 3) {
                sig.sigStrength = 0.025; // 1/40
            }
        } break;
        case Anomaly: {           // 7
            type = MakeRandomInt(1,5);
            // if anomaly is non-drone, set template variables for types.
            //   looking over this again (years later) it dont make much sense.
            //   will have to look deeper when i have time.
            if (faction != 6) {
                faction = 8;
                if (sec == 1) {
                    if (type == 1) {
                        level = GetRandLevel();
                    }
                } else if (sec == 2) {
                    if (type == 2) {
                        level = GetRandLevel();
                    } else if (type == 4) {
                        level = GetRandLevel();
                    }
                } else if (sec == 3) {
                    if (type == 1) {
                        level = GetRandLevel();
                    } else if (type == 3) {
                        level = GetRandLevel();
                    }
                }
            }
        } break;
        // yes, these will 'fall thru' to 'Unrated' here.  this is on purpose
        case Escalation:  // 9
        case Rated: {  // 10
            //sig.dungeonType = 9;
        };
        case Mission: {   // 1
            // not sure how im gonna do this one yet...make it unrated for now
            sig.dungeonType = 8;
        };
        case Unrated: {           // 8
            if (faction == 6) {
                type = MakeRandomInt(1,3);
            } else {
                faction = 0;
                type = MakeRandomInt(1,5);
            }
        } break;
        case 0:
        default: {
            sig.dungeonType = 7;
            MakeDungeon(sig);
        } break;
    }

    if (faction == 7) {
        // faction is defined to be region sovereign holder.
        //   this hasnt been written yet, so default to region rats
        faction = GetFaction(sDataMgr.GetRegionRatFaction(m_system->GetRegionID()));
    }

    uint32 templateID = (sig.dungeonType * 10000) + (sec * 1000) + (type * 100) + (level * 10) + faction;

    _log(COSMIC_MGR__TRACE, "DungeonMgr::MakeDungeon() - Calling Create for type %s(%u) using templateID %u", \
            sDunDataMgr.GetDungeonType(sig.dungeonType), sig.dungeonType, templateID);

    return Create(templateID, sig);
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
    _log(COSMIC_MGR__TRACE, "DungeonMgr::GetRandLevel() - level = %.2f", level);

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

/*
struct CosmicSignature {
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    GPoint position;
};
*/
void DungeonMgr::CreateDeco(uint32 templateID, CosmicSignature& sig)
{
    /** @todo this needs work for proper sizing of deco.  */
    /* templateID format.  ABCDE
     *       A = site - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 6:ded, 7:anomaly, 8:unrated, 9:escalation
     *       B = sec - mission: 1-9 (incomplete); others - sysSec: 1=hi, 2=lo, 3=null, 4=mid;
     *       C = type - grav: ore 0-5, ice 6-9; anomaly: 1-5; mission: 1-9; mag: *see below*; ded: 1-8; ladar/radar: 1-8
     *       D = level - mission: 1-9; grav: ore 1-3, ice 0; mag: *see below*; radar: 1-norm; 2-digital(nullsec); ladar: 1; anomaly: 1-5
     *       E = faction - 0=code defined, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones, 7=region sov, 8=region rat, 9=other
     *
     * NOTE:  mag sites have multiple types and levels based on other variables.
     *      levels are defined as relic(1), salvage(2), and drone(3), with salvage being dominant.
     *      for hisec and losec, types are 1-8 for relic and salvage.  there are no drone mag sites here
     *      for nullsec, relic site types are 1-8, salvage site types are 1-4, and drone site types are 1-7
     *
     * NOTE:  faction can only be 8 for grav and anomaly sites, unless drones (6). all others MUST use 1-6
     */

    // templateID = (sig.dungeonType *10000) + (sec *1000) + (type *100) + (level *10) + factionID;
    uint8 factionID = templateID % 10;
    uint8 level = templateID / 10 % 10;
    uint8 type = templateID / 100 % 10;
    //uint8 sec = templateID / 1000 % 10;

    // create groupIDs for this dungeon, and add to vector
    //  NOTE:  these are NOT invGroups here....
    std::vector<uint16> groupVec;
    groupVec.clear();
    // all groups get the worthless mining types and misc deco items
    groupVec.push_back(131);    //misc roids
    groupVec.push_back(132);    //worthless mining types
    groupVec.push_back(691);    // misc
    // add worthless shit to vector
    AddDecoToVector(sig.dungeonType, templateID, groupVec);
    // clear out vector before adding specific types
    groupVec.clear();

    using namespace Dungeon::Type;
    switch (sig.dungeonType) {
        case Mission: {    //1
        } break;
        case Gravimetric: {    //2
            groupVec.push_back(130);    //named roids
            groupVec.push_back(160);    //asteroid colony items
            groupVec.push_back(620);    // Starbase
            groupVec.push_back(630);    // Habitation
        } break;
        case Magnetometric: {  //3
            groupVec.push_back(620);    // Starbase
            groupVec.push_back(630);    // Habitation
            groupVec.push_back(640);    // Stationary
            groupVec.push_back(650);    // Indestructible
            groupVec.push_back(660);    // Forcefield
            groupVec.push_back(670);    // Shipyard
            groupVec.push_back(680);    // Construction
            groupVec.push_back(690);    // Storage
        } break;
        case Radar: {  //4
            groupVec.push_back(130);    //named roids
            groupVec.push_back(160);    //asteroid colony items
            groupVec.push_back(630);    // Habitation
            groupVec.push_back(640);    // Stationary
        } break;
        case Ladar: {  //5
            groupVec.push_back(160);    //asteroid colony items
            groupVec.push_back(670);    // Shipyard
            groupVec.push_back(680);    // Construction
            groupVec.push_back(690);    // Storage
        } break;
        case Anomaly:  //7
        case Rated: {  //10
            groupVec.push_back(430);    //lco misc
            groupVec.push_back(431);    //lco Habitation
            groupVec.push_back(432);    //lco drug labs
            groupVec.push_back(433);    //lco Starbase
            groupVec.push_back(630);    // Habitation
            groupVec.push_back(640);    // Stationary
            groupVec.push_back(650);    // Indestructible
        } break;
        case Unrated: {    //8
            groupVec.push_back(430);    //lco misc
            groupVec.push_back(431);    //lco Habitation
            groupVec.push_back(432);    //lco drug labs
            groupVec.push_back(433);    //lco Starbase
        } break;
        case Escalation: { //9
            groupVec.push_back(640);    // Stationary
            groupVec.push_back(650);    // Indestructible
        } break;
    }
    // add misc shit to vector
    AddDecoToVector(sig.dungeonType, templateID, groupVec);
    // clear out vector before adding specific faction types
    groupVec.clear();
    switch (sig.dungeonType) {
        case Anomaly:     //7
        case Unrated:     //8
        case Escalation:  //9
        case Rated: {    //10
            switch (factionID) {
                case 1: { //Serpentis
                    groupVec.push_back(401); //faction lco
                    groupVec.push_back(601); //faction base
                } break;
                case 2: { //Angel
                    groupVec.push_back(402);  //faction lco
                    groupVec.push_back(602);  //faction base
                } break;
                case 3: { //Blood
                    groupVec.push_back(403);  //faction lco
                    groupVec.push_back(603);  //faction base
                } break;
                case 4: { //Guristas
                    groupVec.push_back(404);  //faction lco
                    groupVec.push_back(604);  //faction base
                } break;
                case 5: { //Sansha
                    groupVec.push_back(405);  //faction lco
                    groupVec.push_back(605);  //faction base
                } break;
                case 6: { //Drones
                    groupVec.push_back(406);  //faction lco
                    groupVec.push_back(606);  //faction base
                } break;
                // make error for default or 0 here?
            }
        }
    }
    AddDecoToVector(sig.dungeonType, templateID, groupVec);
}

void DungeonMgr::AddDecoToVector(uint8 dunType, uint32 templateID, std::vector<uint16>& groupVec)
{
    // templateID = (sig.dungeonType *10000) + (sec *1000) + (type *100) + (level *10) + factionID;
    uint8 factionID = templateID % 10;
    uint8 level = templateID / 10 % 10;
    uint8 type = templateID / 100 % 10;
    uint8 sec = templateID / 1000 % 10;

    int8 step = 0;
    uint16 count = 0, radius = 0, pos = 10000 * level;
    double theta = 0;

    // level is 0 to 9, system multiplier is 0.1 to 2.0 (x10 is 1-20)
    level *= (m_system->GetSecValue() *10);  // config variable here?
    // set origLevel 0 to 18, rounding up
    uint8 origLevel = ceil(level /10);
    if (origLevel < 1)
        origLevel = 1;
    for (auto cur : groupVec) {
        level = origLevel;
        count = sDunDataMgr.groups.count(cur);
        if (count < 1)
            continue;

        _log(COSMIC_MGR__DEBUG, "DungeonMgr::AddDecoToVector() - %s(%u):  faction %u, group %u, type %u, level %u, count %u, baseLvl %u",\
                    sDunDataMgr.GetDungeonType(dunType), dunType, factionID, \
                    cur, type, level, count, origLevel);

        auto groupRange = sDunDataMgr.groups.equal_range(cur);
        auto it = groupRange.first;
        double degreeSeparation = (250/level);
        // make 1-20 random items in the anomaly based on system trusec
        for (uint8 i=0; i < level; ++i) {
            Dungeon::GroupData grp = Dungeon::GroupData();
            step = MakeRandomInt(1,count);
            std::advance(it,step);      // this is some fancy shit here
            grp.typeID = it->second.typeID;
            grp.typeName = it->second.typeName;
            grp.typeGrpID = it->second.typeGrpID;
            grp.typeCatID = it->second.typeCatID;
            // site size and item radius determine position
            radius = it->second.radius;
            //if (sig.dungeonType == Gravimetric) {
                theta =  EvE::Trig::Deg2Rad(degreeSeparation * i);
                //theta = MakeRandomFloat(0,  EvE::Trig::Pi);
                grp.x = (radius + pos * std::cos(theta)) * (IsEven(MakeRandomInt(0,10)) ? -1 : 1);
                grp.z = (radius + pos * std::sin(theta)) * -1;
                /*
                grp.y = MakeRandomFloat(-radius, radius);
            } else if (IsEven(MakeRandomInt(0,10))) {
                grp.x = (pos + it->second.x + (radius*2)) * (IsEven(MakeRandomInt(0,10)) ? -1 : 1);
                grp.z = pos + it->second.z + radius;
            } else {
                grp.x = (pos + it->second.x + radius) * -1;
                grp.z = (pos + it->second.z + (radius*2)) * -1;
            } */
            grp.y = it->second.y + MakeRandomInt(-5000, radius * 2);
            m_anomalyItems.push_back(grp);
            it = groupRange.first;
        }
    }
}


/* groupID format
      ABB - item def groups
   A = group type - 1:deco, 2:system effect beacon, 3:mining, 4:lco, 5:ships, 6:base, 7:station gun, 8:station wrecks, 9:misc
   B =  1:space objects, 2:effect beacons, 3:roid types, 4:ice types, 5:, 6:asteroid colony, 7:, 8:, 9:misc
   B = ship/gun faction:  1:Amarr, 2:Caldari, 3:Gallente, 4:Minmatar, 5:Sentinel, 6:Guardian
   B =  faction:  00:none, 01:Serpentis, 02:Angel, 03:Blood, 04:Guristas, 05:Sansha, 06:Drones, 07:Amarr, 08:Caldari, 09:Gallente,
                  10:Minmatar, 11:Sleeper, 12:Talocan, 13:Ammatar

1xx  deco items
110  wormholes
13x  mining types
130  named roids
131  misc roids
132  worthless mining types
140  infested items
160  Asteroid Colony
191  Monument

2xx  effect beacons **incomplete
211 Electronic
212 omni
22x Incursion
23x Black Hole
24x Magnetar
25x Pulsar
26x Red Giant
27x Wolf Rayet
28x Cataclysmic Variable

3xx  mining objects
30x  ore
34x  ice
36x  clouds

4xx lco
401 Serpentis
402 Angel
403 Blood
404 Guristas
405 Sansha
406 drone
407 Amarr
408 Caldari
409 Gallente
410 Minmatar
42x lco ships
43x lco structures
430 lco misc
431 lco Habitation
432 lco drug labs
433 lco Starbase

5xx ships
51x Amarr
52x Caldari
53x Gallente
54x Minmatar

6xx base
601 Serpentis
602 Angel
603 Blood
604 Guristas
605 Sansha
606 drone
607 Amarr
608 Caldari
609 Gallente
610 Minmatar
611 Sleeper
612 Talocan
613 Ammatar

620 Starbase
630 Habitation
640 Stationary
650 Indestructible
660 Forcefield
670 Shipyard
680 Construction
690 Storage
691 misc

7xx station guns  **incomplete

8xx station and structure ruins **incomplete
80x  Sansha
81x  Amarr
82x  Caldari
83x  Gallente
84x  Minmatar
85x  misc ruined parts
86x  misc debris

9xx misc **incomplete
91x comets
92x clouds
93x environment
96x event lco/lcs
*/

    /*
    In player-owned sovereign nullsec, using Ore Prospecting Arrays,
    (23510,'Small Asteroid Cluster',0,2,0,1,0,0,0),
    (23520,'Moderate Asteroid Cluster',0,2,0,15,0,0,0),
    (23530,'Large Asteroid Cluster',0,2,0,29,0,0,0),
    (23540,' Enormous Asteroid Cluster ',0,2,0,29,0,0,0),
    (23550,'Colossal Asteroid Cluster',0,2,0,29,0,0,0),
    */