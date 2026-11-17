
 /**
  * @name SpawnMgr.cpp
  *     NPC Spawn management system for EVEmu
  *
  * @Author:         Allan
  * @date:          15 July 2015
  *
  */

#include "EVEServerConfig.h"
#include <algorithm>

#include "StaticDataMgr.h"
#include "EVE_Missions.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "system/DestinyManager.h"
#include "system/Celestial.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "agents/Agent.h"

// NPC ship types in this Crucible data set use Entity category (11) and
// faction rat groups named "Asteroid ...". Retain non-NPC room objects as
// celestials instead of silently discarding them.
static bool IsMissionNPCGroup(const Inv::GrpData& group)
{
    return group.catID == EVEDB::invCategories::Ship
        || group.catID == EVEDB::invCategories::Drone
        || (group.catID == EVEDB::invCategories::Entity
            && group.name.compare(0, 9, "Asteroid ") == 0);
}

// Synthetic Blockade mission 65535 uses 12 Angel Cartel cruisers and battlecruisers.
// These are the verified Angel Cartel NPC group IDs. In particular, group
// 552 is Battleships (not Cruisers), while group 576 is BattleCruisers.
// Keep the expected group name alongside each ID so a mismatched static-data
// pack cannot silently introduce a higher hull class into this encounter.
struct BlockadeTestShipClass
{
    uint16 groupID;
    const char* expectedGroupName;
    uint8 quantity;
};

struct BlockadeTestShip
{
    uint16 typeID;
    uint16 groupID;
    const char* expectedGroupName;
};

static const BlockadeTestShipClass kAngelCartelBlockadeTestMix[] = {
    {551, "Asteroid Angel Cartel Cruiser", 8},
    {576, "Asteroid Angel Cartel BattleCruiser", 4}
};

// Mission 65534 retains 18 valid Crucible NPC candidate groups, but selects
// only 8 ships per new pocket: 2 DED/deadspace, 2 commanders, 2 officers
// and 2 overseers. Factions and hull classes vary between offers.
// The original 12-NPC Blockade is unchanged.
struct EliteSecurityShipSpec {
    uint16 groupID;
    uint32 factionID;
    const char* groupNameToken;
};

static const EliteSecurityShipSpec kEliteSecurityFleet[] = {
    // Deadspace: frigate, destroyer, cruiser, battlecruiser, battleship.
    {597, factionAngel,       "Deadspace"},
    {605, factionBloodRaider, "Deadspace"},
    {613, factionGuristas,    "Deadspace"},
    {620, factionSanshas,     "Deadspace"},
    {630, factionSerpentis,   "Deadspace"},
    // Faction commanders: one of each hull size across the five factions.
    {789, factionAngel,       "Commander"},
    {796, factionBloodRaider, "Commander"},
    {798, factionGuristas,    "Commander"},
    {807, factionSanshas,     "Commander"},
    {852, factionSerpentis,   "Commander"},
    // Officers: one rare boss representing each pirate faction.
    {553, factionAngel,       "Officer"},
    {559, factionBloodRaider, "Officer"},
    {564, factionGuristas,    "Officer"},
    {569, factionSanshas,     "Officer"},
    {574, factionSerpentis,   "Officer"},
    // DED overseer frigate, cruiser and battleship (Angel-backed faction).
    {819, factionAngel,       "Overseer"},
    {820, factionAngel,       "Overseer"},
    {821, factionAngel,       "Overseer"}
};
// Four category buckets in the 18-element candidate roster:
 // Deadspace, Commander, Officer, Overseer.
static constexpr std::size_t kEliteSecurityCategoryOffsets[] = {0, 5, 10, 15, 18};
static constexpr uint8 kEliteSecurityShipsPerCategory = 2;
static_assert(sizeof(kEliteSecurityFleet) / sizeof(kEliteSecurityFleet[0]) == 18,
    "Keep all 18 validated Crucible NPC candidate groups");
static_assert(Mission::kEliteSecurityRequiredKills
    == kEliteSecurityShipsPerCategory
        * (sizeof(kEliteSecurityCategoryOffsets) / sizeof(kEliteSecurityCategoryOffsets[0]) - 1),
    "Elite mission kill target must match its 8 selected ships");

/** @todo  this can be updated to spawn mission, anomaly and deadspace rats.
 *   change all *roidRat* to *somethingelse* to better explain/describe the maps and how they're used.
 *  add objects for mission, anomaly and deadspace rats as needed to aid in tracking/deleting.
 *
 * add booleans for checking roam/static spawns and add anomaly checks to enable spawn mgr.
 *
 */

/*
 SPAWN__ERROR
 SPAWN__WARNING
 SPAWN__MESSAGE
 SPAWN__POP
 SPAWN__DEPOP
 SPAWN__TRACE
 */

/** @todo this class needs a bit more tweaking to work as designed...may/may not spawn all types correctly at this time.  */
SpawnMgr::SpawnMgr(SystemManager* mgr, EVEServiceManager& svc)
: m_system(mgr),
  m_services(svc),
  m_dungMgr(nullptr),
  m_ratTimer(0),
  m_ratGroupTimer(0),
  m_missionTimer(0),
  m_incursionTimer(0),
  m_deadspaceTimer(0),
  m_groupTimerSetTime(0),
  m_spawnID(1),
  m_initalized(false)
{
    m_spawns.clear();
    m_bubbles.clear();
    m_toSpawn.clear();
    m_ratSpawns.clear();
    m_ratSpawnClass.clear();        // not used
    m_factionGroups.clear();
}

bool SpawnMgr::Init()
{
    // even if belt spawns arent activated, still allow anomaly/dungeon spawning
    m_initalized = true;

    if (m_system->BeltCount() < 1) {
        _log(COSMIC_MGR__INIT, "Belt Spawns Disabled for %s(%u) - no belts.", m_system->GetName(), m_system->GetID());
        return true;
    }

    m_groupTimerSetTime = 150;  // (in seconds) 2.5m default check time. this will allow a max wait time of 7.5m for respawn

    _log(COSMIC_MGR__INIT, "SpawnMgr Initialized for %s(%u)", m_system->GetName(), m_system->GetID());
    _log(COSMIC_MGR__INIT, "Roaming Belt Spawns are %s", sConfig.npc.RoamingSpawns ? "enabled" : "disabled");
    _log(COSMIC_MGR__INIT, "Static Gate Spawns are %s", sConfig.npc.StaticSpawns ? "enabled" : "disabled");

    return m_initalized;
}

// this is only for rats.
void SpawnMgr::Process() {
    if (!m_initalized)
        return;

    double profileStartTime(GetTimeUSeconds());
    // called by SystemManager::Process() for each system.  this will need to be fast.
    //  check timers and call appropriate functions as needed.

    // will have to think about this one a bit to implement properly (and quickly)
    /*  current implementation uses a single timer for entire system
	 * timer goes off, do the following...
	 *   loop thru all spawned bubbles in system
	 *   spawn rats as needed to fill spawn groups
	 *   kill timer after all spawns are full
	 * this should be a single check for each iteration.  if multiple rats are killed, timer remains counting down from initial setting.
	 * if all rats are respawned, then a rat is killed again (in this system), the timer is reset and the system begins again.
	 *   each time the group timer is hit, it will check for missing rats in each spawn group.  if there is a missing rat, that rat is
	 *   respawned and the timer is reset, to check again.  this *should* enable chain-ratting.
     *
     *  this will need work for correct operation as intended.
     * process should look at all SpawnEntryDefs for each bubble, and spawn according to number/total for that bubble's spawnID
     * right now, that number isnt updated when rats are killed.  will have to work on coding that correctly
	 */
    if (m_ratGroupTimer.Enabled())
        if (m_ratGroupTimer.Check()) {
            bool killTimer = true;
            SpawnEntryDef::iterator itr = m_spawns.begin(), end = m_spawns.end();
            while (itr != end) {
                if (itr->second.enabled) {
                    killTimer = false;
                    // stamp is the earliest permitted respawn time, not
                    // an expiry; retain entries until their delay has elapsed.
                    if (itr->second.stamp > sEntityList.GetStamp()) {
                        ++itr;
                        continue;
                    }
                    SystemBubble* spawnBubble = sBubbleMgr.FindBubbleByID(itr->first);
                    if (spawnBubble == nullptr) {
                        itr->second.enabled = false;
                        ++itr;
                        continue;
                    }
                    _log(SPAWN__TRACE, "Process() calling Respawn for SpawnEntryID %u (0x%X)", \
                            itr->second.spawnID, &itr->second);
                    // this means check SpawnEntry for 'missing' SpawnGroup members and respawn as needed.
                    ReSpawn(spawnBubble, itr->second);
                    itr->second.enabled = false;
                }
                ++itr;
            }

            if (killTimer) {
                m_ratGroupTimer.Disable();
                _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Rat Spawn Groups full (or no spawns) for %s(%u).  RatGroup Timer disabled.", \
                            m_system->GetName(), m_system->GetID());
            }
        }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::spawn, GetTimeUSeconds() - profileStartTime);
}

void SpawnMgr::MoveSpawn(NPC* pNPC, SystemBubble* pBubble)
{
    if (pNPC == nullptr)
        return;
    if (pBubble == nullptr)
        return;
    uint32 npcID =  pNPC->GetID();
    _log(SPAWN__TRACE, "MoveSpawn() called by %s(%u) from bubbleID %u to bubbleID %u", pNPC->GetName(), npcID, pNPC->SysBubble()->GetID(), pBubble->GetID() );

    // get npc spawn data and add to new location
    auto range = m_spawns.equal_range(pNPC->SysBubble()->GetID());
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.itemID == npcID) {
            m_spawns.emplace(pBubble->GetID(), itr->second);
            m_spawns.erase(itr);
            break;
        }
        ++itr;
    }

    //find out if any rats remain before setting bubble spawned to false
    if (pNPC->SysBubble()->CountNPCs() < 2)
        pNPC->SysBubble()->SetSpawned(false);
    // this will NOT count as new bubble being spawned.
    //pBubble->SetSpawned(true);
}

void SpawnMgr::WarpOutSpawn(NPC* pNPC, SystemBubble* pBubble)
{
    if (pNPC == nullptr)
        return;
    if (pBubble == nullptr)
        return;
    _log(SPAWN__TRACE, "WarpOutSpawn() called by %s(%u) from bubbleID %u to bubbleID %u", pNPC->GetName(), pNPC->GetID(), pNPC->SysBubble()->GetID(), pBubble->GetID() );
    NPC* rNPC(nullptr);
    auto range = m_spawns.equal_range(pNPC->SysBubble()->GetID());
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.enabled) {
            ++itr;
            continue;
        }
        rNPC = m_system->GetNPCSE(itr->second.itemID);
        if (rNPC == nullptr) {
            ++itr;
            continue;
        }
        rNPC->DestinyMgr()->WarpTo(pBubble->GetCenter(), MakeRandomFloat(10, 30) *100);
        rNPC->GetAIMgr()->DisableWarpOutTimer();
        m_spawns.emplace(pBubble->GetID(), itr->second);
        m_spawns.erase(itr);
        ++itr;
    }

    pNPC->SysBubble()->SetSpawned(false);
    pBubble->SetSpawned(true);
}


void SpawnMgr::StartRatTimer()
{
    if (m_ratTimer.Enabled())
        return;
    uint16 time = sConfig.npc.RoamingTimer *1000;  //  s to ms
    if (sConfig.debug.SpawnTest)
        time = 5000; /* 5s for npc spawn testing */
    m_ratTimer.Start(time);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::StartRatTimer() - Main Spawn Timer started for %s(%u) at %u ms.", \
            m_system->GetName(), m_system->GetID(), time);
}

void SpawnMgr::StartRatGroupTimer()
{
    if (m_ratGroupTimer.Enabled()) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::StartRatGroupTimer() - Group Spawn Timer currently running.  Time left: %us", m_ratGroupTimer.GetRemainingTime() /1000);
        return;
    }
    m_ratGroupTimer.Start(m_groupTimerSetTime *1000);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::StartRatGroupTimer() - Group Spawn Timer started for %s(%u) at %us.", \
            m_system->GetName(), m_system->GetID(), m_groupTimerSetTime);
}

void SpawnMgr::SpawnKilled(SystemBubble* pBubble, uint32 itemID, uint32 killerID)
{
    if (pBubble == nullptr)
        return;
    if (pBubble->IsBelt()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Belt - called by %u.", itemID);
        // if any SpawnEntry still exists for this bubble, reset group timer.
        // this enables chain ratting
        bool killed = true;
        bool foundKilledNPC = false;
        auto range = m_spawns.equal_range(pBubble->GetID());
        auto itr = range.first;
        while (itr != range.second) {
            if (itr->second.itemID == itemID) {
                foundKilledNPC = true;
                itr->second.stamp = sEntityList.GetStamp() + sConfig.npc.RespawnTimer; // seconds
                itr->second.enabled = true;
            }
            if (!itr->second.enabled)
                killed = false;     // at least one rat left.
            ++itr;
        }
        if (!foundKilledNPC)
            return;
        if (killed) {
            _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled - Belt Spawn has been destoyed.  Resetting spawn checks for bubble %u.", pBubble->GetID());
            // spawn destroyed.  delete from list and reset bubble checks.
            m_spawns.erase(pBubble->GetID());
            auto bubbleItr = std::find(m_bubbles.begin(), m_bubbles.end(), pBubble);
            if (bubbleItr != m_bubbles.end())
                m_bubbles.erase(bubbleItr);
            pBubble->ResetBubbleRatSpawn();
            m_system->RemoveSpawnBubble(pBubble);
            return;
        } else {
            StartRatGroupTimer();
        }
    } else if (pBubble->IsGate()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Gate - called by %u.", itemID);
        // we are not enabling rat chaining on gates.
        RemoveSpawn(pBubble->GetID(), itemID);
        if (pBubble->CountNPCs() < 2)
            pBubble->SetSpawned(false);
    } else if (pBubble->IsAnomaly()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Anomaly - called by %u.", itemID);
        if (m_spawns.count(pBubble->GetID()) == 1) {
            // last npc in this wave.  get data needed for next wave, if applicable.
            SpawnEntryDef::iterator itr = m_spawns.find(pBubble->GetID());
            if (itr == m_spawns.end())
                return; // this is an error.
            MakeSpawn(pBubble, itr->second.factionID, itr->second.spawnClass, itr->second.level);
            // now remove this spawn from map.
            m_spawns.erase(itr);
            // unlock warp gate if applicable
        } else if (m_spawns.count(pBubble->GetID()) < 1) {
            // this is an error...
        } else {
            // there are still npcs in this wave....continue.
        }

        /*  this needs to deal with multiple things.
         * 1- unlocking warp gates when needed per wave
         * 2- dropping loot according to (wave/dungeon/template)?
         * 3- after last spawn, possible escelation per dungeon type?   this should signal anomaly mgr to create the escelation
         * 4- spawn next wave, if applicable  (code above...currently testing)
         * 5- more/others?
         */
    } else if (pBubble->IsMission()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Mission - NPC %u killed by character %u.", itemID, killerID);
        RemoveSpawn(pBubble->GetID(), itemID);

        const bool missionKillRecorded = pBubble->RecordMissionKill();
        const uint32 liveNPCsIncludingDyingShip = pBubble->CountNPCs();
        // NPC::Killed runs while the dying NPC may still be in the bubble.
        // <= 1 therefore means this was the final live hostile in this
        // dedicated mission pocket, even if the spawn registry retained rows.
        const bool allMissionNPCsDestroyed =
            (m_spawns.count(pBubble->GetID()) == 0 || liveNPCsIncludingDyingShip <= 1);

        _log(SPAWN__ERROR,
            "Mission kill event: offer %u, NPC %u, bubble %u, live NPCs (including dying ship) %u, registry entries %u, kill counter %u/%u.",
            pBubble->GetMissionOfferID(), itemID, pBubble->GetID(),
            liveNPCsIncludingDyingShip,
            static_cast<uint32>(m_spawns.count(pBubble->GetID())),
            pBubble->GetMissionKills(), pBubble->GetMissionKillsRequired());

        // The final live NPC closes the objective even if stale spawn rows remain.
        if (allMissionNPCsDestroyed)
            m_spawns.erase(pBubble->GetID());

        if (missionKillRecorded || allMissionNPCsDestroyed) {
            Agent* pAgent = sEntityList.GetAgent(pBubble->GetMissionAgentID());
            MissionOffer offer = MissionOffer();
            if (pAgent != nullptr
                && pAgent->HasMission(pBubble->GetMissionCharacterID(), offer)
                && offer.offerID == pBubble->GetMissionOfferID()
                && offer.stateID == Mission::State::Accepted
                && offer.courierAmount > 0) {
                offer.courierTypeID = (allMissionNPCsDestroyed
                    || pBubble->GetMissionKills() >= offer.courierAmount
                    ? offer.courierAmount : pBubble->GetMissionKills());
                pAgent->UpdateOffer(pBubble->GetMissionCharacterID(), offer);
                Client* pClient = sEntityList.FindClientByCharID(pBubble->GetMissionCharacterID());
                if (pClient != nullptr)
                    pAgent->SendMissionUpdate(pClient, "modified");
                _log(SPAWN__MESSAGE,
                    "Mission offer %u progress: %u/%u hostile ships destroyed%s.",
                    offer.offerID, offer.courierTypeID, offer.courierAmount,
                    allMissionNPCsDestroyed ? " (all mission spawns cleared)" : "");
            } else {
                _log(SPAWN__WARNING,
                    "Mission pocket %u has no remaining NPC spawns, but its accepted offer %u could not be updated.",
                    pBubble->GetID(), pBubble->GetMissionOfferID());
            }
        }

        if (allMissionNPCsDestroyed) {
            _log(SPAWN__ERROR,
                "Mission offer %u marked ready for turn-in: all live NPCs cleared; objective is %u/%u.",
                pBubble->GetMissionOfferID(), pBubble->GetMissionKillsRequired(),
                pBubble->GetMissionKillsRequired());
            auto bubbleItr = std::find(m_bubbles.begin(), m_bubbles.end(), pBubble);
            if (bubbleItr != m_bubbles.end())
                m_bubbles.erase(bubbleItr);
            pBubble->SetSpawned(false);
            _log(SPAWN__MESSAGE, "Mission pocket %u has no remaining NPC spawn entries.", pBubble->GetID());
        }
    } else if (pBubble->IsIncursion()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Incursion - called by %u.", itemID);
        // placeholder - not coded yet.
    } else {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Other - called by %u.", itemID);
        RemoveSpawn(pBubble->GetID(), itemID);
    }
}

// Spawn an individual enemy inside a dungeon (called by dungeonMgr)
void SpawnMgr::DoSpawnForAnomaly(SystemBubble* pBubble, GPoint pos, uint8 level, uint16 typeID)
{
    if (pBubble == nullptr)
        return;
    pBubble->SetAnomaly();

    float secRating = m_system->GetSecValue();

    // Get faction of enemy based on item type
    Inv::TypeData objType;
    sDataMgr.GetType(typeID, objType);
    uint32 factionID = sDataMgr.GetRaceFaction(objType.race);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::DoSpawnForAnomaly() - faction: %s, region %u.", \
                sDataMgr.GetFactionName(factionID).c_str(), m_system->GetRegionID());

    SpawnGroup group;
    group.quantity = 1;
    group.typeID = typeID;

    // A missing faction/class mapping yields typeID 0. Do not create
    // invalid NPCs or mark an empty belt bubble as successfully spawned.
    m_toSpawn.erase(std::remove_if(m_toSpawn.begin(), m_toSpawn.end(),
                     [](const SpawnGroup& group) {
                         return group.typeID == 0 || group.quantity == 0;
                     }), m_toSpawn.end());

    if (!m_toSpawn.empty()) {
        if (is_log_enabled(SPAWN__MESSAGE)) {
            _log(SPAWN__MESSAGE, "SpawnMgr::DoSpawnForAnomaly() - toSpawn size is %u.", m_toSpawn.size());    //variable
            _log(SPAWN__MESSAGE, "SpawnMgr::DoSpawnForAnomaly() - Creating spawn for %s in bubbleID %u.", \
            sDataMgr.GetFactionName(factionID).c_str(), pBubble->GetID());
        }

        GPoint startPos(pos);
        GPoint warpToPoint(startPos);

        std::string name = objType.name;

        uint32 corpID = sDataMgr.GetFactionCorp(factionID);
        FactionData data = FactionData();
            data.allianceID = factionID;    // this is to set wreck salvage correctly (tests for faction)
            data.corporationID = corpID;
            data.factionID = (factionID == factionRogueDrones ? 0 : factionID); // the faction of rogue drones is wrong....should be "0" for client to use it right.
            data.ownerID = corpID;

        NPC* pNPC(nullptr);
        InventoryItemRef iRef(nullptr);
        for (auto cur : m_toSpawn) {
            ItemData idata(cur.typeID, corpID, m_system->GetID(), flagNone, "", startPos, name.c_str());
            for (uint8 x=0; x < cur.quantity; ++x) {
                iRef = sItemFactory.SpawnItem(idata);
                if (iRef.get() == nullptr) {
                    _log(SPAWN__ERROR, "Failed to spawn item type %u.", cur.typeID);
                    continue;
                }

                _log(SPAWN__POP, "SpawnMgr::MakeSpawn - Spawning NPC type %u (%u)", cur.typeID, iRef->itemID());

                pNPC = new NPC(iRef, m_services, m_system, data, this);
                if (pNPC == nullptr)
                    continue;

                if (!pNPC->Load()) {
                    _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
                    pNPC->Delete();
                    continue;
                }

                m_system->AddNPC(pNPC);

                pNPC->DestinyMgr()->SetPosition(startPos);

                // For large ships, warp them in from a distance
                if (iRef->GetAttribute(AttrMass) > 10000000) {
                    // adjust warpIn point so show some variation instead of a straight line.
                    GPoint warpTo(warpToPoint);
                    warpTo.MakeRandomPointOnSphere(rand()%(12) *1000);  // random point (1-12) x 1k from center
                    pNPC->DestinyMgr()->WarpTo(warpTo, (MakeRandomInt(-5, 10) *1000));
                }

                // Temporary: generate random spawn class
                uint8 sClass = rand()%(12) + 1;

                SpawnEntry se = SpawnEntry();
                se.enabled = false;
                se.groupID = iRef->type().groupID();
                se.itemID = iRef->itemID();
                se.total = cur.quantity;
                se.number = x+1;
                se.typeID = cur.typeID;
                se.spawnID = m_spawnID;
                se.corpID = corpID;
                se.factionID = factionID;
                se.spawnClass = sClass;
                se.spawnGroup = GetSpawnGroup(sClass);
                se.level = level;
                if (sClass <= Spawn::Class::Officer) {  // this spawn is for rat.
                    se.stamp = 0;   // this is for respawn time...do not set here.
                } else {
                    se.stamp = sEntityList.GetStamp(); // set time of this spawn for ??
                }
                m_spawns.emplace(pBubble->GetID(), se);
                _log(SPAWN__TRACE, "MakeSpawn() adding SpawnEntry with ID %u to m_spawns. Class: %s, Group:%s, Level: %u.", \
                            se.spawnID, GetSpawnClassName(se.spawnClass).c_str(), GetSpawnGroupName(se.spawnGroup).c_str(), level);
            }
        }
        
    ++m_spawnID;
    m_bubbles.push_back(pBubble);

    //cleanup
    m_toSpawn.clear();
    m_ratSpawns.clear();

    _log(SPAWN__TRACE, "MakeSpawn() completed in %s(%u) with %u bubbles in m_bubbles and %u entities in m_spawns.", \
                m_system->GetName(), m_system->GetID(), m_bubbles.size(), m_spawns.size());
        return;
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - Nothing to spawn.");
    }

    return;
}

void SpawnMgr::DoSpawnForIncursion(SystemBubble* pBubble, uint32 regionID)
{
    if (pBubble == nullptr)
        return;
    if (!IsRegionID(regionID))
        return;
    pBubble->SetIncursion();
    // unknown parameters at this time
}

bool SpawnMgr::DoSpawnForMission(SystemBubble* pBubble, uint32 regionID, uint8 missionLevel,
                                  uint32 characterID, uint32 agentID, uint32 offerID, uint16 missionID,
                                  uint32 beaconID, uint32 factionID, uint32 dungeonID)
{
    _log(SPAWN__MESSAGE,
        "SpawnMgr::DoSpawnForMission() - offer %u mission %u level %u character %u agent %u beacon %u faction %u dungeon %u bubble %u.",
        offerID, missionID, missionLevel, characterID, agentID, beaconID, factionID,
        dungeonID, (pBubble != nullptr ? pBubble->GetID() : 0));
    if (pBubble == nullptr || !IsRegionID(regionID)
        || characterID == 0 || agentID == 0 || offerID == 0) {
        _log(SPAWN__ERROR,
            "SpawnMgr::DoSpawnForMission() - rejected invalid arguments for offer %u mission %u (region %u).",
            offerID, missionID, regionID);
        return false;
    }

    uint8 spawnClass = Spawn::Class::Easy;
    switch (missionLevel) {
        case 0: spawnClass = Spawn::Class::Easy; break;
        case 1: spawnClass = Spawn::Class::Fair; break;
        case 2: spawnClass = Spawn::Class::Average; break;
        case 3: spawnClass = Spawn::Class::Medium; break;
        case 4: spawnClass = Spawn::Class::Hard; break;
        default: spawnClass = Spawn::Class::Crazy; break;
    }

    pBubble->SetMission();
    pBubble->SetMissionTypeID(missionID);

    if (missionID == Mission::kEliteSecurityMissionID) {
        // Fail closed if the selected Crucible SDE cannot supply one of the
        // named NPC classes or any of the three required real loot tiers.
        uint16 previewLoot = 0;
        for (uint8 tier = 4; tier <= 6; ++tier) {
            if (!sDataMgr.GetEliteMissionLootType(tier, previewLoot)) {
                _log(SPAWN__ERROR,
                    "DED/Officer mission %u has no valid Module-category loot for meta group %u in installed static data.",
                    offerID, tier);
                pBubble->SetMission(false);
                pBubble->SetMissionTypeID(0);
                return false;
            }
        }

        std::map<uint16, Inv::TypeData> allTypes;
        sDataMgr.GetTypes(allTypes);
        struct SelectedEliteShip {
            uint16 typeID;
            uint16 groupID;
            uint32 factionID;
        };
        // Choose two distinct NPC groups in each rarity category; the wider
        // original roster remains available for variation between missions.
        std::vector<const EliteSecurityShipSpec*> chosenSpecs;
        for (std::size_t category = 0; category + 1
                < sizeof(kEliteSecurityCategoryOffsets) / sizeof(kEliteSecurityCategoryOffsets[0]);
                ++category) {
            std::vector<const EliteSecurityShipSpec*> available;
            for (std::size_t i = kEliteSecurityCategoryOffsets[category];
                    i < kEliteSecurityCategoryOffsets[category + 1]; ++i)
                available.push_back(&kEliteSecurityFleet[i]);
            for (uint8 i = 0; i < kEliteSecurityShipsPerCategory; ++i) {
                const std::size_t index = Mission::RandomCandidateIndex(available.size());
                chosenSpecs.push_back(available[index]);
                available[index] = available.back();
                available.pop_back();
            }
        }

        std::vector<SelectedEliteShip> selectedFleet;
        for (const EliteSecurityShipSpec* spec : chosenSpecs) {
            Inv::GrpData group = {};
            sDataMgr.GetGroup(spec->groupID, group);
            if (group.id == 0 || group.catID != EVEDB::invCategories::Entity
                || group.name.find(spec->groupNameToken) == std::string::npos) {
                _log(SPAWN__ERROR,
                    "DED/Officer mission %u missing expected entity group %u (%s).",
                    offerID, spec->groupID, spec->groupNameToken);
                pBubble->SetMission(false);
                pBubble->SetMissionTypeID(0);
                return false;
            }
            std::vector<uint16> candidates;
            for (const auto& candidate : allTypes) {
                if (candidate.second.id != 0 && candidate.second.groupID == spec->groupID)
                    candidates.push_back(candidate.first);
            }
            if (candidates.empty()) {
                _log(SPAWN__ERROR,
                    "DED/Officer mission %u has no NPC types in group %u (%s).",
                    offerID, spec->groupID, group.name.c_str());
                pBubble->SetMission(false);
                pBubble->SetMissionTypeID(0);
                return false;
            }
            const uint16 typeID = candidates[Mission::RandomCandidateIndex(candidates.size())];
            selectedFleet.push_back({typeID, spec->groupID, spec->factionID});
        }

        uint16 spawnedCount = 0;
        uint16 failedCount = 0;
        uint16 fleetNumber = 0;
        const GPoint center = pBubble->GetCenter();
        for (const SelectedEliteShip& ship : selectedFleet) {
            Inv::TypeData type = {};
            sDataMgr.GetType(ship.typeID, type);
            if (type.id == 0 || type.groupID != ship.groupID) {
                ++failedCount;
                continue;
            }
            const uint32 corpID = sDataMgr.GetFactionCorp(ship.factionID);
            if (corpID == 0) {
                _log(SPAWN__ERROR, "DED/Officer mission %u invalid faction %u.",
                    offerID, ship.factionID);
                ++failedCount;
                continue;
            }

            GPoint position(center);
            // Separate hostile ships across the mission pocket, not on the
            // warp-in point. Range includes battleship and officer spawns.
            position.x += MakeRandomFloat(-24000.0f, 24000.0f);
            position.y += MakeRandomFloat(-24000.0f, 24000.0f);
            position.z += MakeRandomFloat(-12000.0f, 12000.0f);

            ItemData itemData(ship.typeID, corpID, m_system->GetID(),
                flagNone, "", position, type.name.c_str());
            InventoryItemRef item = sItemFactory.SpawnItem(itemData);
            if (item.get() == nullptr) {
                ++failedCount;
                _log(SPAWN__ERROR, "DED/Officer mission %u failed spawning NPC type %u.",
                    offerID, ship.typeID);
                continue;
            }

            FactionData factionData = FactionData();
            factionData.allianceID = ship.factionID;
            factionData.corporationID = corpID;
            factionData.factionID = ship.factionID;
            factionData.ownerID = corpID;
            NPC* pNPC = new NPC(item, m_services, m_system, factionData, this);
            if (pNPC == nullptr || !pNPC->Load()) {
                ++failedCount;
                _log(SPAWN__ERROR, "DED/Officer mission %u NPC load failed for type %u.",
                    offerID, ship.typeID);
                if (pNPC != nullptr)
                    pNPC->Delete();
                continue;
            }

            m_system->AddNPC(pNPC);
            pNPC->DestinyMgr()->SetPosition(position);
            SpawnEntry entry = SpawnEntry();
            entry.enabled = false;  // do not respawn completed mission kills
            entry.groupID = ship.groupID;
            entry.itemID = item->itemID();
            entry.total = 1;
            entry.number = ++fleetNumber;
            entry.typeID = ship.typeID;
            entry.spawnID = m_spawnID;
            entry.corpID = corpID;
            entry.factionID = ship.factionID;
            entry.spawnClass = spawnClass;
            entry.spawnGroup = GetSpawnGroup(spawnClass);
            entry.level = missionLevel;
            entry.stamp = 0;
            m_spawns.emplace(pBubble->GetID(), entry);
            ++spawnedCount;
        }

        if (spawnedCount == 0) {
            _log(SPAWN__ERROR, "DED/Officer mission %u spawned zero valid NPCs.",
                offerID);
            pBubble->SetMission(false);
            pBubble->SetMissionTypeID(0);
            return false;
        }
        ++m_spawnID;
        m_bubbles.push_back(pBubble);
        const uint32 npcCount = pBubble->CountNPCs();
        if (npcCount == 0 || npcCount > 65535) {
            m_spawns.erase(pBubble->GetID());
            m_bubbles.pop_back();
            pBubble->SetMission(false);
            pBubble->SetMissionTypeID(0);
            pBubble->SetSpawned(false);
            return false;
        }
        // Accepted offer's required kills are updated from this actual pocket
        // by Client::StartEncounterMission; no unwinnable stale target.
        pBubble->SetMissionData(characterID, agentID, offerID, beaconID,
            static_cast<uint16>(npcCount));
        pBubble->SetSpawned(true);
        _log(SPAWN__MESSAGE,
            "DED/Officer mission %u spawned %u/%u NPCs (%u failures), two from each elite category selected.",
            offerID, spawnedCount, Mission::kEliteSecurityRequiredKills, failedCount);
        return true;
    }

    // Mission 65535 is a synthetic test encounter. Spawn the supplied Angel
    // Cartel inventory types directly so it is independent of incomplete or
    // unrelated imported mission-dungeon room data.
    if (missionID == Mission::kBlockadeSecurityTestMissionID) {
        const uint32 angelCartelFactionID = 500011;
        const uint32 angelCartelCorporationID = 1000138;
        FactionData factionData = FactionData();
        factionData.allianceID = angelCartelFactionID;
        factionData.corporationID = angelCartelCorporationID;
        factionData.factionID = angelCartelFactionID;
        factionData.ownerID = angelCartelCorporationID;

        uint16 spawnedCount = 0;
        uint16 failedCount = 0;
        uint16 fleetNumber = 0;
        const GPoint center = pBubble->GetCenter();

        std::map<uint16, Inv::TypeData> allTypes;
        sDataMgr.GetTypes(allTypes);
        std::vector<BlockadeTestShip> selectedFleet;
        for (const BlockadeTestShipClass& shipClass : kAngelCartelBlockadeTestMix) {
            Inv::GrpData group = {};
            sDataMgr.GetGroup(shipClass.groupID, group);
            if (group.id == 0 || group.catID != EVEDB::invCategories::Entity
                || group.name != shipClass.expectedGroupName) {
                _log(SPAWN__ERROR,
                    "Blockade test fleet group %u is missing or is not the expected NPC class (%s).",
                    shipClass.groupID, shipClass.expectedGroupName);
                ++failedCount;
                continue;
            }

            std::vector<uint16> candidates;
            for (const auto& typeEntry : allTypes) {
                if (typeEntry.second.groupID == shipClass.groupID
                    && typeEntry.second.id != 0)
                    candidates.push_back(typeEntry.first);
            }
            if (candidates.empty()) {
                _log(SPAWN__ERROR,
                    "Blockade test fleet group %u (%s) has no available NPC ship types.",
                    shipClass.groupID, group.name.c_str());
                ++failedCount;
                continue;
            }

            for (uint8 i = 0; i < shipClass.quantity; ++i) {
                const uint16 selectedTypeID =
                    candidates[static_cast<size_t>(rand()) % candidates.size()];
                selectedFleet.push_back({selectedTypeID, shipClass.groupID, shipClass.expectedGroupName});
            }
        }

        const uint32 fleetSize = Mission::kBlockadeSecurityTestRequiredKills;
        if (selectedFleet.size() != fleetSize) {
            _log(SPAWN__ERROR,
                "Blockade test fleet composition incomplete: selected %u of %u ships (%u group errors).",
                static_cast<uint32>(selectedFleet.size()), fleetSize, failedCount);
            pBubble->SetMission(false);
            return false;
        }

        for (const BlockadeTestShip& ship : selectedFleet) {
            const uint16 typeID = ship.typeID;
            Inv::TypeData type = {};
            Inv::GrpData group = {};
            sDataMgr.GetType(typeID, type);
            if (type.id == 0) {
                _log(SPAWN__ERROR, "Blockade test fleet selected missing NPC type %u.", typeID);
                ++failedCount;
                continue;
            }
            sDataMgr.GetGroup(type.groupID, group);
            if (group.id == 0 || group.catID != EVEDB::invCategories::Entity
                || group.name != ship.expectedGroupName) {
                _log(SPAWN__ERROR,
                    "Blockade test fleet type %u has invalid NPC group %u.",
                    typeID, type.groupID);
                ++failedCount;
                continue;
            }

            ++fleetNumber;
            GPoint spawnPosition(center);
            // Spread the test fleet around the pocket centre to avoid stacking
            // every ship at precisely the same coordinates.
            spawnPosition.x += MakeRandomFloat(-5000.0f, 5000.0f);
            spawnPosition.y += MakeRandomFloat(-5000.0f, 5000.0f);
            spawnPosition.z += MakeRandomFloat(-2500.0f, 2500.0f);

            ItemData itemData(typeID, angelCartelCorporationID,
                m_system->GetID(), flagNone, "", spawnPosition, type.name.c_str());
            InventoryItemRef item = sItemFactory.SpawnItem(itemData);
            if (item.get() == nullptr) {
                _log(SPAWN__ERROR, "Blockade test fleet failed to create type %u.", typeID);
                ++failedCount;
                continue;
            }

            NPC* pNPC = new NPC(item, m_services, m_system, factionData, this);
            if (pNPC == nullptr || !pNPC->Load()) {
                _log(SPAWN__ERROR, "Blockade test fleet failed to load NPC type %u.", typeID);
                if (pNPC != nullptr)
                    pNPC->Delete();
                ++failedCount;
                continue;
            }

            m_system->AddNPC(pNPC);
            pNPC->DestinyMgr()->SetPosition(spawnPosition);

            SpawnEntry entry = SpawnEntry();
            entry.enabled = false;
            entry.groupID = type.groupID;
            entry.itemID = item->itemID();
            entry.total = 1;
            entry.number = fleetNumber;
            entry.typeID = typeID;
            entry.spawnID = m_spawnID;
            entry.corpID = angelCartelCorporationID;
            entry.factionID = angelCartelFactionID;
            entry.spawnClass = spawnClass;
            entry.spawnGroup = GetSpawnGroup(spawnClass);
            entry.level = missionLevel;
            entry.stamp = 0;
            m_spawns.emplace(pBubble->GetID(), entry);
            ++spawnedCount;
        }

        if (spawnedCount == 0) {
            _log(SPAWN__ERROR, "Blockade test mission %u spawned no Angel Cartel NPCs (%u failed).",
                offerID, failedCount);
            pBubble->SetMission(false);
            return false;
        }

        ++m_spawnID;
        m_bubbles.push_back(pBubble);
        const uint32 npcCount = pBubble->CountNPCs();
        if (npcCount == 0 || npcCount > 65535) {
            m_spawns.erase(pBubble->GetID());
            m_bubbles.pop_back();
            pBubble->SetMission(false);
            pBubble->SetSpawned(false);
            return false;
        }

        pBubble->SetMissionData(characterID, agentID, offerID, beaconID,
            static_cast<uint16>(npcCount));
        pBubble->SetSpawned(true);
        _log(SPAWN__MESSAGE,
            "Blockade test mission %u spawned %u/%u Angel Cartel NPCs (faction %u, corporation %u; %u failed).",
            offerID, spawnedCount, fleetSize, angelCartelFactionID,
            angelCartelCorporationID, failedCount);
        return true;
    }

    const bool blockadeTestMission = (missionID == Mission::kBlockadeSecurityTestMissionID);

    // Mission dungeon metadata is more authoritative than the agent's faction:
    // the dungeon template identifies the actual opposing NPC faction and its
    // first-room NPC type IDs. Later room/gate progression is handled separately.
    bool spawnedFromTemplate = false;
    {
        Dungeon::Dungeon missionDungeon = {};
        if (dungeonID != 0)
            sDunDataMgr.GetDungeon(missionDungeon, dungeonID);

        // The imported mission rows in some Crucible data sets have dungeonID=0.
        // Resolve a real NPC-bearing agent-mission template by faction instead
        // of requiring the empty legacy dunDungeons/dunRooms tables.
        if (missionDungeon.rooms.empty()) {
            if (blockadeTestMission) {
                // The synthetic Blockade test is Angel Cartel content and must
                // not inherit the issuing agent's empire faction.
                sDunDataMgr.GetRandomMissionDungeonForGroup(missionDungeon, "Asteroid Angel Cartel");
            } else {
                sDunDataMgr.GetRandomMissionDungeon(missionDungeon, factionID);
            }
        }

        if (blockadeTestMission && missionDungeon.rooms.empty()) {
            _log(SPAWN__ERROR, "Blockade test mission %u has no Angel Cartel first-room template; refusing to spawn a different faction.", offerID);
            pBubble->SetMission(false);
            return false;
        }

        if (missionDungeon.factionID != 0)
            factionID = missionDungeon.factionID;
        if (factionID == 0)
            factionID = sDataMgr.GetRegionRatFaction(m_system->GetRegionID());

        const uint32 selectedDungeonID = missionDungeon.dungeonID;
        _log(SPAWN__MESSAGE, "Mission %u uses dungeon template %u and faction %u.",
                offerID, selectedDungeonID, factionID);

        if (!missionDungeon.rooms.empty()) {
            const Dungeon::Room& firstRoom = missionDungeon.rooms.begin()->second;
            const uint32 corpID = sDataMgr.GetFactionCorp(factionID);
            FactionData factionData = FactionData();
            factionData.allianceID = factionID;
            factionData.corporationID = corpID;
            factionData.factionID = (factionID == factionRogueDrones ? 0 : factionID);
            factionData.ownerID = corpID;

            uint16 spawnNumber = 0;
            uint16 staticObjectCount = 0;
            for (const Dungeon::RoomObject& object : firstRoom.objects) {
                // Static-data lookups are void-returning and leave the output
                // untouched when an ID is missing. Zero-initialize both records
                // so malformed dungeon references cannot expose uninitialized IDs.
                Inv::TypeData objectType = {};
                Inv::GrpData objectGroup = {};
                sDataMgr.GetType(object.typeID, objectType);
                if (objectType.id == 0) {
                    _log(SPAWN__WARNING, "Mission %u dungeon %u references missing type %u.",
                            offerID, selectedDungeonID, object.typeID);
                    continue;
                }
                sDataMgr.GetGroup(objectType.groupID, objectGroup);
                if (objectGroup.id == 0) {
                    _log(SPAWN__WARNING, "Mission %u dungeon %u type %u references missing group %u.",
                            offerID, selectedDungeonID, object.typeID, objectType.groupID);
                    continue;
                }

                GPoint spawnPosition = pBubble->GetCenter();
                spawnPosition.x += object.x;
                spawnPosition.y += object.y;
                spawnPosition.z += object.z;

                if (IsMissionNPCGroup(objectGroup)) {
                    ItemData itemData(object.typeID, corpID, m_system->GetID(), flagNone,
                                      "", spawnPosition, objectType.name.c_str());
                    InventoryItemRef item = sItemFactory.SpawnItem(itemData);
                    if (item.get() == nullptr) {
                        _log(SPAWN__ERROR, "Mission %u failed to create template NPC type %u.",
                                offerID, object.typeID);
                        continue;
                    }

                    NPC* pNPC = new NPC(item, m_services, m_system, factionData, this);
                    if (pNPC == nullptr)
                        continue;
                    if (!pNPC->Load()) {
                        _log(SPAWN__ERROR, "Mission %u failed to load template NPC %u (type %u).",
                                offerID, pNPC->GetID(), object.typeID);
                        pNPC->Delete();
                        continue;
                    }

                    m_system->AddNPC(pNPC);
                    pNPC->DestinyMgr()->SetPosition(spawnPosition);

                    SpawnEntry entry = SpawnEntry();
                    entry.enabled = false;
                    entry.groupID = item->type().groupID();
                    entry.itemID = item->itemID();
                    entry.total = 1;
                    entry.number = ++spawnNumber;
                    entry.typeID = object.typeID;
                    entry.spawnID = m_spawnID;
                    entry.corpID = corpID;
                    entry.factionID = factionID;
                    entry.spawnClass = spawnClass;
                    entry.spawnGroup = GetSpawnGroup(spawnClass);
                    entry.level = missionLevel;
                    entry.stamp = 0;
                    m_spawns.emplace(pBubble->GetID(), entry);
                } else {
                    // Non-NPC room objects (structures, gates, containers and
                    // scenery) remain part of the pocket as ordinary celestials.
                    ItemData objectData(object.typeID, corpID, m_system->GetID(), flagNone,
                                        "", spawnPosition, objectType.name.c_str());
                    InventoryItemRef objectItem = sItemFactory.SpawnItem(objectData);
                    if (objectItem.get() == nullptr) {
                        _log(SPAWN__WARNING, "Mission %u failed to create room object type %u.",
                                offerID, object.typeID);
                        continue;
                    }

                    CelestialSE* pCelestial = new CelestialSE(objectItem, m_services, m_system);
                    m_system->AddEntity(pCelestial, false);
                    ++staticObjectCount;
                }
            }

            if (spawnNumber > 0) {
                ++m_spawnID;
                m_bubbles.push_back(pBubble);
                spawnedFromTemplate = true;
                _log(SPAWN__MESSAGE, "Mission %u spawned %u NPCs and %u room objects from dungeon %u room %u.",
                        offerID, spawnNumber, staticObjectCount, selectedDungeonID, firstRoom.roomID);
            }
        }
    }

    if (!spawnedFromTemplate && blockadeTestMission) {
        _log(SPAWN__ERROR, "Blockade test mission %u did not spawn from its Angel Cartel room template.", offerID);
        pBubble->SetMission(false);
        return false;
    }

    if (!spawnedFromTemplate && !PrepSpawn(pBubble, spawnClass, 0, factionID)) {
        pBubble->SetMission(false);
        return false;
    }

    const uint32 npcCount = pBubble->CountNPCs();
    if (npcCount == 0 || npcCount > 65535) {
        m_spawns.erase(pBubble->GetID());
        auto bubbleItr = std::find(m_bubbles.begin(), m_bubbles.end(), pBubble);
        if (bubbleItr != m_bubbles.end())
            m_bubbles.erase(bubbleItr);
        pBubble->SetMission(false);
        pBubble->SetSpawned(false);
        return false;
    }

    pBubble->SetMissionData(characterID, agentID, offerID, beaconID, static_cast<uint16>(npcCount));
    pBubble->SetSpawned(true);
    _log(SPAWN__MESSAGE, "Created Level %u mission pocket %u for character %u with %u NPCs.",
            missionLevel, pBubble->GetID(), characterID, npcCount);
    return true;
}

bool SpawnMgr::DoSpawnForBubble(SystemBubble* pBubble)
{
    if (pBubble == nullptr)
        return false;

    if (pBubble->IsBelt()) {
        if (!sConfig.cosmic.BeltEnabled)
            return false;
        if (!sConfig.npc.RoamingSpawns)
            return false;
    }

    if (pBubble->IsGate())
        if (!sConfig.npc.StaticSpawns)
            return false;

    if (FindSpawnForBubble(pBubble->GetID())) {
        _log(SPAWN__TRACE, "SpawnMgr::FindSpawnForBubble() returned true for bubble %u.", pBubble->GetID());
        pBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        return false;
    } else {
        if (PrepSpawn(pBubble)) {
            pBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        } else {
            _log(SPAWN__TRACE, "SpawnMgr::PrepSpawn() returned false for bubble %u.", pBubble->GetID());
            return false;
        }
    }

    if (pBubble->IsBelt())
        m_system->IncRatSpawnCount();
    if (pBubble->IsGate())
        m_system->IncGateSpawnCount();

    return true;
}

bool SpawnMgr::PrepSpawn(SystemBubble* pBubble, uint8 sClass/*Spawn::Class::None*/, uint8 level/*0*/, uint32 factionOverride)
{
    if (pBubble == nullptr)
        return false;
    // Use the actual system security rating (-1.0 to 1.0). GetSecValue()
    // stores 1.1 - trueSec and the old 1.0 - GetSecValue() was 0.1 too low.
    const float secRating = m_system->GetSystemSecurityRating();
    m_toSpawn.clear();
    m_factionGroups.clear();

    bool anomaly = false;
    // get faction for this region
    uint32 factionID = factionRogueDrones;  // default to rogue drones.  this is my internal rogue drone factionID.
    if (factionOverride != 0) {
        // Mission encounters are tied to the issuing agent, not regional rat RNG.
        factionID = factionOverride;
    } else if (sConfig.npc.RatFaction) {        // is RatFaction set in config?
        factionID = sConfig.npc.RatFaction;
    } else if (MakeRandomFloat() > 0.05) {      // 5% chance for ANY ordinary spawn to be rogue drone.
        factionID = sDataMgr.GetRegionRatFaction(m_system->GetRegionID());
        if (!factionID) {
            _log(SPAWN__WARNING, "No NPC faction configured for region %u; using rogue drones.", m_system->GetRegionID());
            factionID = factionRogueDrones;
        }
    }

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - faction: %s, region %u. (config set %s)", \
                sDataMgr.GetFactionName(factionID).c_str(), m_system->GetRegionID(), (sConfig.npc.RatFaction?"true":"false"));

    // if rat, get possible spawn groups for this secRating.
    if (sClass == Spawn::Class::None) {
        if ((secRating < 0.2) and pBubble->IsBelt()) {   // check for hauler, commander, officer spawn, but ONLY in a belt
            //NOTE  random checks here are for TESTING only....all rates are high.  make config option later?
            double rand = MakeRandomFloat();
            if (rand < 0.1) { // officer spawn
                if (factionID != factionRogueDrones) {  //but not for drones.  they dont have officers..make this the rare drone hauler spawn (which isnt written yet)
                    sClass = Spawn::Class::Officer;
                } else {
                    sClass = Spawn::Class::Hauler;
                }
            } else if (rand < 0.15) { // commander spawn
                sClass = Spawn::Class::Commander;
            } else if (rand < 0.25) { // hauler spawn
                if (factionID != factionRogueDrones)
                    sClass = Spawn::Class::Hauler;
            }
        }
    } else {
        // we were called from anomaly, incursion or mission to spawn npcs
        anomaly = true;
    }

    if (sClass == Spawn::Class::None) {
        if (pBubble->IsBelt()) {
            if ((factionID != factionRogueDrones) and (MakeRandomFloat() < 0.08)) { // second chance for hauler spawn, but include ALL secRating in this one
                sClass = Spawn::Class::Hauler;
            } else { // gonna be a 'regular' trusec-based spawn in a belt.
                if (secRating < -0.7) {
                    sClass = Spawn::Class::Insane;
                } else if (secRating < -0.4) {
                    sClass = Spawn::Class::Crazy;
                } else if (secRating < -0.1) {
                    sClass = Spawn::Class::Hard;
                } else if (secRating < 0.3) {
                    sClass = Spawn::Class::Medium;
                } else if (secRating < 0.6) {
                    sClass = Spawn::Class::Average;
                } else if (secRating < 0.85) {
                    sClass = Spawn::Class::Fair;
                } else {
                    sClass = Spawn::Class::Easy;
                }
            }
            if ((secRating < 0) and  (sClass < Spawn::Class::Hauler))
                if (MakeRandomFloat() < 0.1)
                    sClass = Spawn::Class::Hell;
        } else if (pBubble->IsGate()) { // gate spawns are smaller/easier than roid spawns
            if (secRating < -0.7) {
                sClass = Spawn::Class::Hard;
            } else if (secRating < -0.4) {
                sClass = Spawn::Class::Medium;
            } else if (secRating < -0.1) {
                sClass = Spawn::Class::Average;
            } else if (secRating < 0.3) {
                sClass = Spawn::Class::Fair;
            } else {
                sClass = Spawn::Class::Easy;
            }
        } else {
            // make error here?
        }
    }

    // get faction's shipClass and groupID map...is this feasible?  it's fine...there's only 21 at this time.
    if (sDataMgr.GetNPCGroups(factionID, m_factionGroups)) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - m_factionGroups size is %u.", m_factionGroups.size());    //should be 21 (22 for drones)
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No RatFaction data for faction %u.  Cancelling spawn.", factionID);
        return false;
    }

    RatSpawnClassVec spawnEntry;
    if (sDataMgr.GetNPCClasses(sClass, spawnEntry)) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - spawnEntry - size: %u, class: %s(%u).", spawnEntry.size(), GetSpawnClassName(sClass).c_str(), sClass);
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No NPC Class data for %u (%s).  Cancelling spawn.", sClass, GetSpawnClassName(sClass).c_str());
        return false;
    }

    if (anomaly) {
        // anomaly, incursion or mission
        ++level;    // increment wave
        // check wave # vs possible waves.  (oob check)
        if (spawnEntry.size() <= level) {
            _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - spawnEntry.size (%u) < level (%u) for anomaly class %s.  Cancelling spawn.", spawnEntry.size(), level, GetSpawnClassName(sClass).c_str());
            return false;
        }
        /** @todo  test for overseer wave and spawn correct overseer for this anomaly  */
        /** @todo  make templates/functions/whatever for sending msgs to players local for waves */
    } else if (sClass == Spawn::Class::Hauler) {
        // split hauler spawns based on trusec
             if (secRating < -0.8)  { level = MakeRandomInt(4, 7); }
        else if (secRating < -0.5)  { level = MakeRandomInt(3, 6); }
        else if (secRating < -0.2)  { level = MakeRandomInt(2, 5); }
        else if (secRating < 0.1)   { level = MakeRandomInt(1, 4); }
        else if (secRating < 0.4)   { level = MakeRandomInt(1, 3); }
        else if (secRating < 0.7)   { level = 2; }
        else                        { level = 1; }
    } else if (sClass <= Spawn::Class::Hell) {
        level = MakeRandomInt(0, spawnEntry.size() - 1);  // highest valid index is size()-1.
    } else {
        // do we need anything else here?
    }

    if (level >= spawnEntry.size()) {
        // Some static-data packs have fewer hauler variants than the
        // security-based range above. Never index past the available rows.
        _log(SPAWN__WARNING, "NPC spawn class %u has %u variants; clamping requested level %u.",
             sClass, static_cast<uint32>(spawnEntry.size()), level);
        level = static_cast<uint8>(spawnEntry.size() - 1);
    }

    // get ship class data from spawnEntry.at(subtype)
    // and put this spawn's group information in class designators
    uint8 f = spawnEntry.at(level).f;
    uint8 af = spawnEntry.at(level).af;
    uint8 d = spawnEntry.at(level).d;
    uint8 c = spawnEntry.at(level).c;
    uint8 ac = spawnEntry.at(level).ac;
    uint8 bc = spawnEntry.at(level).bc;
    uint8 bs = spawnEntry.at(level).bs;
    uint8 h = spawnEntry.at(level).h;
    uint8 o = spawnEntry.at(level).o;
    uint8 cf = spawnEntry.at(level).cf;
    uint8 cd = spawnEntry.at(level).cd;
    uint8 cc = spawnEntry.at(level).cc;
    uint8 cbc = spawnEntry.at(level).cbc;
    uint8 cbs = spawnEntry.at(level).cbs;
    spawnEntry.clear();

    // get typeIDs to spawn based on info in m_factionGroups and ship designators and put into Spawn Vector
    // figure out how to distuinguish between roid, anomaly, incursion and mission defs for this....
    uint8 shipClass = 0;
    if (sClass > Spawn::Class::BeltSpawn)
        shipClass = 14;
    SpawnGroup toSpawn = SpawnGroup();
    // these types are for ALL spawn types.
    if (f > 0) {
        toSpawn.typeID = GetRandTypeID(1 + shipClass);
        toSpawn.quantity = f;
        m_toSpawn.push_back(toSpawn);
    }
    if (af > 0) {
        toSpawn.typeID = GetRandTypeID(2 + shipClass);
        toSpawn.quantity = af;
        m_toSpawn.push_back(toSpawn);
    }
    if (d > 0) {
        toSpawn.typeID = GetRandTypeID(3 + shipClass);
        toSpawn.quantity = d;
        m_toSpawn.push_back(toSpawn);
    }
    if (c > 0) {
        toSpawn.typeID = GetRandTypeID(4 + shipClass);
        toSpawn.quantity = c;
        m_toSpawn.push_back(toSpawn);
    }
    if (ac > 0) {
        toSpawn.typeID = GetRandTypeID(5 + shipClass);
        toSpawn.quantity = ac;
        m_toSpawn.push_back(toSpawn);
    }
    if (bc > 0) {
        toSpawn.typeID = GetRandTypeID(6 + shipClass);
        toSpawn.quantity = bc;
        m_toSpawn.push_back(toSpawn);
    }
    if (bs > 0) {
        toSpawn.typeID = GetRandTypeID(7 + shipClass);
        toSpawn.quantity = bs;
        m_toSpawn.push_back(toSpawn);
    }
    // end of possible non-roid rat types.  following are for belt/gate only
    if (sClass < Spawn::Class::BeltSpawn) {
        if (h > 0) {
            toSpawn.typeID = GetRandTypeID(8);
            toSpawn.quantity = h;
            m_toSpawn.push_back(toSpawn);
        }
        if ((o > 0) and (factionID != factionRogueDrones)) {    //drones do NOT have officers (internal type 9).
            toSpawn.typeID = GetRandTypeID(9);
            toSpawn.quantity = o;
            m_toSpawn.push_back(toSpawn);
        }
        if (cf > 0) {
            toSpawn.typeID = GetRandTypeID(10);
            toSpawn.quantity = cf;
            m_toSpawn.push_back(toSpawn);
        }
        if (cd > 0) {
            toSpawn.typeID = GetRandTypeID(11);
            toSpawn.quantity = cd;
            m_toSpawn.push_back(toSpawn);
        }
        if (cc > 0) {
            toSpawn.typeID = GetRandTypeID(12);
            toSpawn.quantity = cc;
            m_toSpawn.push_back(toSpawn);
        }
        if (cbc > 0) {
            toSpawn.typeID = GetRandTypeID(13);
            toSpawn.quantity = cbc;
            m_toSpawn.push_back(toSpawn);
        }
        if (cbs > 0) {
            toSpawn.typeID = GetRandTypeID(14);
            toSpawn.quantity = cbs;
            m_toSpawn.push_back(toSpawn);
        }
    }

    if (factionID == factionRogueDrones) {
        if ((bc > 0) or (bs > 0)) {
            if (sClass < Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(9);
            } else if (sClass > Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(22);
            }
            // spawn 4 swarm ships for each bc/bs
            toSpawn.quantity = ((bs > 0 ? bs : bc) *4);
            m_toSpawn.push_back(toSpawn);
        } else if (o > 0) {
            // drones dont have officers.  spawn swarm x10
            if (sClass < Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(9);
            } else if (sClass > Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(22);
            }
            toSpawn.quantity = o *10;
            m_toSpawn.push_back(toSpawn);
        }
    }

    //cleanup
    m_factionGroups.clear();

    if (m_toSpawn.size() > 0) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - toSpawn size is %u.", m_toSpawn.size());    //variable
        MakeSpawn(pBubble, factionID, sClass, level, anomaly);
        return true;
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - Nothing to spawn.");
    }

    return false;
}

/*
struct SpawnEntry {     // notes for me while creating/writing/testing
    bool enabled;       // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
    uint8 spawnClass;   // spawn class.  0 = none, 1-7 = easy to insane based on sysSec, 8 = hauler, 9 = commander, 10 = officer
    uint8 spawnGroup;   // spawn group.   1 = roid rat, 2 = roaming, 3 = static, 4 = anomaly, 5 = mission, 6 = incursion, 7 = deadspace, 8 = sleeper
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rat's number in group (to match up with above total)
    uint8 level;        // spawn data subtype/wave
    uint8 classID;      // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint16 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint16 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint32 itemID;      // rat entity id
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
    uint16 stamp;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
};
*/
void SpawnMgr::MakeSpawn(SystemBubble* pBubble, uint32 factionID, uint8 sClass, uint8 level, bool anomaly/*false*/)
{
    _log(SPAWN__MESSAGE, "SpawnMgr::MakeSpawn() - Creating spawn class %s for %s in bubbleID %u (anomaly = %s).", \
            GetSpawnClassName(sClass).c_str(), sDataMgr.GetFactionName(factionID).c_str(), pBubble->GetID(), anomaly?"true":"false");

    /*  the point here is to have all belt rats spawn outside their belt's bubble.
     * to make it 'realistic', they will need the appearance of warping in from some random point,
     *  to somewhere around bubble center.  this will make their origin appear elsewhere,
     * but not from same place every time.  they're pirates, they got other shit to do, too.
     *  eventually, when other systems are working, npcs will appear to 'warp in' from a hideout in the current system.
     *  this particular bit is not general knowledge and will have to be thought out a bit more before coding.
     *
     * for non-rat spawns, this is the intial spawn which and they will already be in pocket.
     *  waves will be spawned at structure (template positioning data), OR will warp in if no structure in pocket
     */
    GPoint startPos(pBubble->GetCenter());
    GPoint warpToPoint(startPos);

    std::string name = "BeltRat";
    if (anomaly) {
        name = GetSpawnClassName(sClass);
    } else {
        /** @todo  make method to get/use template positioning data for spawns here */
        // ratspawn will warp in, others will not.
        startPos.MakeRandomPointOnSphere(MakeRandomInt(10, 15) *100000); //1-1m5 km from current bubble center
    }

    uint32 corpID = sDataMgr.GetFactionCorp(factionID);
    FactionData data = FactionData();
        data.allianceID = factionID;    // this is to set wreck salvage correctly (tests for faction)
        data.corporationID = corpID;
        data.factionID = (factionID == factionRogueDrones ? 0 : factionID); // the faction of rogue drones is wrong....should be "0" for client to use it right.
        data.ownerID = corpID;

    NPC* pNPC(nullptr);
    InventoryItemRef iRef(nullptr);
    for (auto cur : m_toSpawn) {
        /*
         *        ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
         *                  const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
         */
        ItemData idata(cur.typeID, corpID, m_system->GetID(), flagNone, "", startPos, name.c_str());
        for (uint8 x=0; x < cur.quantity; ++x) {
            iRef = sItemFactory.SpawnItem(idata);
            if (iRef.get() == nullptr) {
                _log(SPAWN__ERROR, "Failed to spawn item type %u.", cur.typeID);
                continue;
            }

            _log(SPAWN__POP, "SpawnMgr::MakeSpawn - Spawning NPC type %u (%u)", cur.typeID, iRef->itemID());

            pNPC = new NPC(iRef, m_services, m_system, data, this);
            if (pNPC == nullptr)
                continue;

            if (!pNPC->Load()) {
                _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
                pNPC->Delete();
                continue;
            }

            m_system->AddNPC(pNPC);

            pNPC->DestinyMgr()->SetPosition(startPos);
            //  begin warp.  this may have to be looked into later for timing of large spawns (>6)
            //  actually looks kinda cool when larger ships come in later...
            if (sClass <= Spawn::Class::Officer) {   // ratspawn will warp in, others will not.
                // adjust warpIn point so show some variation instead of a straight line.
                GPoint warpTo(warpToPoint);
                warpTo.MakeRandomPointOnSphere(sClass *1000);  // random point <class (1-12)> x 1k from center
                pNPC->DestinyMgr()->WarpTo(warpTo, (MakeRandomInt(-5, 10) *1000));
            }

            SpawnEntry se = SpawnEntry();
            se.enabled = false;
            se.groupID = iRef->type().groupID();
            se.itemID = iRef->itemID();
            se.total = cur.quantity;
            se.number = x+1;
            se.typeID = cur.typeID;
            se.spawnID = m_spawnID;
            se.corpID = corpID;
            se.factionID = factionID;
            se.spawnClass = sClass;
            se.spawnGroup = GetSpawnGroup(sClass);
            se.level = level;
            if (sClass <= Spawn::Class::Officer) {  // this spawn is for rat.
                se.stamp = 0;   // this is for respawn time...do not set here.
            } else {
                se.stamp = sEntityList.GetStamp(); // set time of this spawn for ??
            }
            m_spawns.emplace(pBubble->GetID(), se);
            _log(SPAWN__TRACE, "MakeSpawn() adding SpawnEntry with ID %u to m_spawns. Class: %s, Group:%s, Level: %u.", \
                        se.spawnID, GetSpawnClassName(se.spawnClass).c_str(), GetSpawnGroupName(se.spawnGroup).c_str(), level);
        }
    }

    ++m_spawnID;
    m_bubbles.push_back(pBubble);

    //cleanup
    m_toSpawn.clear();
    m_ratSpawns.clear();

    _log(SPAWN__TRACE, "MakeSpawn() completed in %s(%u) with %u bubbles in m_bubbles and %u entities in m_spawns.", \
                m_system->GetName(), m_system->GetID(), m_bubbles.size(), m_spawns.size());
}

void SpawnMgr::ReSpawn(SystemBubble* pBubble, SpawnEntry& spawnEntry)
{
    if (pBubble == nullptr)
        return;

    //  we are NOT enabling spawn chaining for officer, hauler, or commander spawns.
    if (spawnEntry.spawnClass > Spawn::Class::Insane)
        return;
    GPoint startPos(pBubble->GetCenter());
    GPoint warpToPoint(startPos);
    startPos.MakeRandomPointOnSphere(MakeRandomInt(10, 15) *100000); //1-1m5 km from bubble center
    _log(SPAWN__TRACE, "ReSpawn()  data for spawnEntryID %u  0x%X is type:%u, corp:%u, faction:%u, #:%u of %u", \
            spawnEntry.spawnID, &spawnEntry, spawnEntry.typeID, spawnEntry.corpID, \
            spawnEntry.factionID, spawnEntry.number, spawnEntry.total);
    /* ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
     *           const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
     */
    ItemData idata(spawnEntry.typeID, spawnEntry.corpID, m_system->GetID(), flagNone, "", startPos, "BeltRat");
    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);      // will have to work on this to NOT save npc to db.
    if (iRef.get() == nullptr) {
        _log(SPAWN__ERROR, "Failed to spawn item type %u.", spawnEntry.typeID);
        return;
    }

    _log(SPAWN__POP, "SpawnMgr::ReSpawn - Spawning NPC %s(%u)", iRef->name(), iRef->itemID());

    FactionData data = FactionData();
        data.allianceID = spawnEntry.factionID;   // this is to set wreck salvage correctly (tests for faction)
        data.corporationID = spawnEntry.corpID;
        data.factionID = spawnEntry.factionID;
        data.ownerID = spawnEntry.corpID;
    NPC* pNPC = new NPC(iRef, m_services, m_system, data, this);
    if (pNPC == nullptr) {
        _log(SPAWN__ERROR, "Failed to create NPC SE for item type %u.", spawnEntry.typeID);
        return;
    }

    if (!pNPC->Load()) {
        _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
        SafeDelete(pNPC);
        return;
    }

    m_system->AddNPC(pNPC);
    pNPC->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(-5, 10) *1000));

    spawnEntry.stamp = 0;
    spawnEntry.enabled = false;
    _log(SPAWN__TRACE, "ReSpawn() completed for spawnEntryID %u 0x%X in bubble %u.", spawnEntry.spawnID, &spawnEntry, pBubble->GetID());
}

bool SpawnMgr::FindSpawnForBubble(uint16 bubbleID) {
    if (m_spawns.find(bubbleID) != m_spawns.end())
        return true;

    return false;
}

uint16 SpawnMgr::GetRandTypeID(uint8 sClass)
{
    uint16 groupID = 0;
    RatFactionGroupsMap::iterator itr = m_factionGroups.find(sClass);
    if (itr != m_factionGroups.end())
        groupID = itr->second;
    return sDataMgr.GetRandRatType(sClass, groupID);
}

bool SpawnMgr::IsChaining(uint16 bubbleID)
{
    bool rsp = false;
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.enabled)
            rsp = true;
        ++itr;
    }

    return rsp;
}

void SpawnMgr::RemoveSpawn(uint16 bubbleID, uint32 itemID)
{
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.itemID == itemID) {
            _log(SPAWN__TRACE, "RemoveSpawn() found item %u in spawnID %u and removed it.", itemID, itr->second.spawnID);
            m_spawns.erase(itr);
            return;
        }
        ++itr;
    }

    _log(SPAWN__TRACE, "RemoveSpawn() did not find item %u in bubble %u, out of %u total spawns in the map.", itemID, bubbleID, m_spawns.size());
    return;
}

uint8 SpawnMgr::GetSpawnGroup(uint8 sClass)
{
    switch (sClass) {
        case Spawn::Class::Extra:    // placeholder - not used yet
        case Spawn::Class::None:
            return Spawn::Group::None;
        case Spawn::Class::Easy:
        case Spawn::Class::Fair:
        case Spawn::Class::Average:
        case Spawn::Class::Medium:
        case Spawn::Class::Hard:
        case Spawn::Class::Crazy:
        case Spawn::Class::Insane:
        case Spawn::Class::Hell:
        case Spawn::Class::Hauler:
        case Spawn::Class::Commander:
        case Spawn::Class::Officer:
            return Spawn::Group::Roaming;
        case Spawn::Class::Hideaway:
        case Spawn::Class::Burrow:
        case Spawn::Class::Refuge:
        case Spawn::Class::Den:
        case Spawn::Class::Yard:
        case Spawn::Class::RallyPoint:
        case Spawn::Class::Port:
        case Spawn::Class::Hub:
        case Spawn::Class::Haven:
        case Spawn::Class::Sanctum:
        case Spawn::Class::Cluster:
        case Spawn::Class::Collection:
        case Spawn::Class::Assembly:
        case Spawn::Class::Gathering:
        case Spawn::Class::Surveillance:
        case Spawn::Class::Menagerie:
        case Spawn::Class::Herd:
        case Spawn::Class::Squad:
        case Spawn::Class::Patrol:
        case Spawn::Class::Horde:
          return Spawn::Group::Anomaly;
        case Spawn::Class::Hideout:
        case Spawn::Class::Lookout:
        case Spawn::Class::Watch:
        case Spawn::Class::Vigil:
        case Spawn::Class::Outpost:
        case Spawn::Class::Annex:
        case Spawn::Class::Base:
        case Spawn::Class::Fortress:
        case Spawn::Class::Complex:
        case Spawn::Class::StagingPoint:
        case Spawn::Class::HauntedYard:
        case Spawn::Class::DesolateSite:
        case Spawn::Class::ChemicalYard:
        case Spawn::Class::TrialYard:
        case Spawn::Class::DirtySite:
        case Spawn::Class::Ruins:
        case Spawn::Class::Independence:
        case Spawn::Class::Radiance:
        case Spawn::Class::Hierarchy:
            return Spawn::Group::Combat;
        case Spawn::Class::Crumbling:
        case Spawn::Class::Decayed:
        case Spawn::Class::Ruined:
        case Spawn::Class::Looted:
        case Spawn::Class::Ransacked:
        case Spawn::Class::Pristine:
        case Spawn::Class::Shard:
        case Spawn::Class::Tower:
        case Spawn::Class::Mainframe:
        case Spawn::Class::Center:
        case Spawn::Class::Server:
            return Spawn::Group::Deadspace;
    }
    // default
    return Spawn::Group::None;
}

std::string SpawnMgr::GetSpawnGroupName(int8 sGroup)
{
    switch(sGroup) {
        case Spawn::Group::None:            return "None";
        case Spawn::Group::Roaming:         return "Roaming";
        case Spawn::Group::Static:          return "Static";
        case Spawn::Group::Anomaly:         return "Anomaly";
        case Spawn::Group::Combat:          return "Combat";
        case Spawn::Group::Deadspace:       return "Deadspace";
        case Spawn::Group::Mission:         return "Mission";
        case Spawn::Group::Incursion:       return "Incursion";
        case Spawn::Group::Sleeper:         return "Sleeper";
        case Spawn::Group::Escalation:      return "Escalation";
        default:                            return "Invalid";
    }
    return "Undefined";
}

std::string SpawnMgr::GetSpawnClassName(int8 sClass)
{
    switch(sClass) {
        case Spawn::Class::None:            return "None";
        case Spawn::Class::Easy:            return "Easy";
        case Spawn::Class::Fair:            return "Fair";
        case Spawn::Class::Average:         return "Average";
        case Spawn::Class::Medium:          return "Medium";
        case Spawn::Class::Hard:            return "Hard";
        case Spawn::Class::Crazy:           return "Crazy";
        case Spawn::Class::Insane:          return "Insane";
        case Spawn::Class::Hauler:          return "Hauler";
        case Spawn::Class::Commander:       return "Commander";
        case Spawn::Class::Officer:         return "Officer";
        case Spawn::Class::Hideaway:        return "Hideaway";
        case Spawn::Class::Burrow:          return "Burrow";
        case Spawn::Class::Refuge:          return "Refuge";
        case Spawn::Class::Den:             return "Den";
        case Spawn::Class::Yard:            return "Yard";
        case Spawn::Class::RallyPoint:      return "RallyPoint";
        case Spawn::Class::Port:            return "Port";
        case Spawn::Class::Hub:             return "Hub";
        case Spawn::Class::Haven:           return "Haven";
        case Spawn::Class::Sanctum:         return "Sanctum";
        case Spawn::Class::Cluster:         return "Cluster";
        case Spawn::Class::Collection:      return "Collection";
        case Spawn::Class::Assembly:        return "Assembly";
        case Spawn::Class::Gathering:       return "Gathering";
        case Spawn::Class::Surveillance:    return "Surveillance";
        case Spawn::Class::Menagerie:       return "Menagerie";
        case Spawn::Class::Herd:            return "Herd";
        case Spawn::Class::Squad:           return "Squad";
        case Spawn::Class::Patrol:          return "Patrol";
        case Spawn::Class::Horde:           return "Horde";
        case Spawn::Class::Hideout:         return "Hideout";
        case Spawn::Class::Lookout:         return "Lookout";
        case Spawn::Class::Watch:           return "Watch";
        case Spawn::Class::Vigil:           return "Vigil";
        case Spawn::Class::Outpost:         return "Outpost";
        case Spawn::Class::Annex:           return "Annex";
        case Spawn::Class::Base:            return "Base";
        case Spawn::Class::Fortress:        return "Fortress";
        case Spawn::Class::Complex:         return "Complex";
        case Spawn::Class::StagingPoint:    return "StagingPoint";
        case Spawn::Class::HauntedYard:     return "HauntedYard";
        case Spawn::Class::DesolateSite:    return "DesolateSite";
        case Spawn::Class::ChemicalYard:    return "ChemicalYard";
        case Spawn::Class::TrialYard:       return "TrialYard";
        case Spawn::Class::DirtySite:       return "DirtySite";
        case Spawn::Class::Ruins:           return "Ruins";
        case Spawn::Class::Independence:    return "Independence";
        case Spawn::Class::Radiance:        return "Radiance";
        case Spawn::Class::Hierarchy:       return "Hierarchy";
        case Spawn::Class::Crumbling:       return "Crumbling";
        case Spawn::Class::Decayed:         return "Decayed";
        case Spawn::Class::Ruined:          return "Ruined";
        case Spawn::Class::Looted:          return "Looted";
        case Spawn::Class::Ransacked:       return "Ransacked";
        case Spawn::Class::Pristine:        return "Pristine";
        case Spawn::Class::Shard:           return "Shard";
        case Spawn::Class::Tower:           return "Tower";
        case Spawn::Class::Mainframe:       return "Mainframe";
        case Spawn::Class::Center:          return "Center";
        case Spawn::Class::Server:          return "Server";
        default:                            return "Invalid";
    }
    return "Undefined";
}
