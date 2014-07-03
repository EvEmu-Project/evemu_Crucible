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

#include "eve-server.h"

#include "PyServiceMgr.h"
#include "npc/NPC.h"
#include "npc/SpawnManager.h"
#include "ship/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/BubbleManager.h"
#include "system/SystemEntity.h"
#include "system/SystemEntities.h"

SpawnGroup::Entry::Entry(
    uint32 _spawnGroupID,
    uint32 _npcTypeID,
    uint8 _quantity,
    float _probability,
    uint32 _ownerID,
    uint32 _corporationID )
: spawnGroupID(_spawnGroupID),
  npcTypeID(_npcTypeID),
  quantity(_quantity),
  probability(_probability),
  ownerID(_ownerID),
  corporationID(_corporationID)
{
}

SpawnGroup::SpawnGroup(uint32 _id, const char *_name, uint32 _formation)
: id(_id),
  name(_name),
  formation(_formation)
{
}

SpawnEntry::SpawnEntry(
    uint32 id,
    SpawnGroup &group,
    uint32 timerMin,
    uint32 timerMax,
    uint32 timerValue,
    SpawnBoundsType boundsType )
: m_id(id),
  m_group(group),
  m_timerMin(timerMin),
  m_timerMax(timerMax),
  m_timer(timerValue),
  m_boundsType(boundsType)
{
	m_spawningNow = false;
}

SpawnManager::SpawnManager(SystemManager &mgr, PyServiceMgr &svc)
: m_system(mgr),
  m_services(svc)
{
}

SpawnManager::~SpawnManager() {
    std::map<uint32, SpawnGroup *>::iterator curs, ends;
    curs = m_groups.begin();
    ends = m_groups.end();
    for(; curs != ends; curs++) {
        delete curs->second;
    }

    std::map<uint32, SpawnEntry *>::iterator cure, ende;
    cure = m_spawns.begin();
    ende = m_spawns.end();
    for(; cure != ende; cure++) {
        delete cure->second;
    }
}

void SpawnEntry::Process(SystemManager &mgr, PyServiceMgr &svc) {
    if((m_timer.Check(false)) && (m_spawningNow == false)) {
		// Timer Expired - Process the Spawn!

        //if(!m_spawnedIDs.empty()) {
        //    _log(SPAWN__ERROR, "ERROR: spawn entry %u's timer went off when we have active spawn IDs!", m_id);
        //    m_spawnedIDs.clear();
        //}

        // Create / Manage Spawns for one or more of the following condtions:
		//   - last ship of this spawn has been destroyed and spawn timer is expired AND a new player ship has entered the bubble or is already here
		//   - new bubble created at this spawn AND a new player ship has entered the bubble
		std::set<SystemEntity *> entitiesInBubble;
		std::set<SystemEntity *>::iterator curEntity, endEntities;
		SystemBubble * currentBubble = NULL;
		currentBubble = mgr.bubbles.FindBubble( bounds.at(0) );
		uint32 curEntityTypeID = 0;
		bool npcFound = false;
		bool playerFound = false;

		// First check to see if we have a bubble at this spawn location:
		if( currentBubble != NULL )
		{
			// We have an existing bubble at this spawn:
			currentBubble->GetEntities( entitiesInBubble );		// Get a complete list of SystemEntity objects currently residing in this bubble
			if( !(entitiesInBubble.empty()) )
			{
				// We have one or more system entities in this bubble, let's see if at least one is a player ship,
				// BUT, if one is an NPC of typeID in our list, we abort trying to do the spawn:
				curEntity = entitiesInBubble.begin();
				endEntities = entitiesInBubble.end();
				for(; curEntity != endEntities; curEntity++)
				{
					if( !(m_spawnedIDs.empty()) )
					{
						//npcFound = true;
						// Make sure each entity checked is an ItemSystemEntity derivative object, otherwise, ignore it since it can't possibly be an NPC or player ship:
						if( (*curEntity)->Item() != 0 )
						{
							curEntityTypeID = (*curEntity)->Item()->itemID();
							if( m_spawnedIDs.find( curEntityTypeID ) != m_spawnedIDs.end() )
							{
								npcFound = true;
							}
						}
					}

					if( (*curEntity)->IsClient() )
					{
						// We have at least one player ship in this bubble
						playerFound = true;
					}
				}

				if( (playerFound == true) && (npcFound == false) )
				{
					// EXCELLENT!  We have a player ship and NONE of our NPC ships present in the bubble,
					// DO THE SPAWN!
					m_spawnedIDs.clear();
					_DoSpawn(mgr, svc);
				}
			}
		}
    }
}

void SpawnEntry::_DoSpawn(SystemManager &mgr, PyServiceMgr &svc) {
    _log(SPAWN__POP, "Spawning spawn entry %u with group %u", m_id, m_group.id);

	bool warpThisSpawnIn = false;

    //pick our spawn point...
    GPoint spawn_point;
    switch(m_boundsType) {    //safe to assume `bounds` are correct.

    case boundsPoint: {
        spawn_point = bounds[0];
    } break;

    case boundsLine: {
        GVector vec(bounds[0], bounds[1]);
        double len = vec.length();

        spawn_point = GPoint(bounds[0] + Ga::GaFloat(MakeRandomFloat(0, len))*Ga::GaVec3(vec));
    } break;

    default:
        _log(SPAWN__ERROR, "Invalid bounds type %u on spawn entry %u when spawning...", m_boundsType, GetID());
        return;
    }
    _log(SPAWN__POP, "    selected point (%.1f, %.1f, %.1f)", spawn_point.x, spawn_point.y, spawn_point.z);

    std::vector<NPC *> spawned;

	// Save Spawn point as Warp-In Point where NPCs will warp into from their "remote" spawn point off-grid:
	GPoint warp_in_point = spawn_point;
	if( warpThisSpawnIn )
	{
		spawn_point.x += 100000000;			// Put spawn point actually out away from intended spawn point by 100,000,000 meters in all three dimensions
		spawn_point.y += 100000000;
		spawn_point.z += 100000000;
	}

    // Spin through our spawn group of typeIDs and quantities and create all ships/structures in the spawn:
	m_spawningNow = true;	// Beginning spawning operation, enable internal marker
    std::vector<SpawnGroup::Entry>::const_iterator cur, end;
    cur = m_group.entries.begin();
    end = m_group.entries.end();
    for(; cur != end; cur++) {
        _log(SPAWN__POP, "    Evaluating spawn entry for NPC type %u in group %u", cur->npcTypeID, cur->spawnGroupID);
        int r;
        for(r = 0; r < cur->quantity; r++) {
            if(cur->probability < 1.0f) {
                if(MakeRandomFloat(0, 1.0f) > cur->probability) {
                    _log(SPAWN__POP, "        [%d] not spawning, p=%.4f failed.", r, cur->probability);
                    continue;
                }
                _log(SPAWN__POP, "        [%d] passed proability check of p=%.4f", r, cur->probability);
            }

            ItemData idata(
                cur->npcTypeID,
                cur->ownerID,    //owner
                mgr.GetID(),
                flagAutoFit
            );

            InventoryItemRef i = svc.item_factory.SpawnItem(idata);
            if( !i ) {
                _log(SPAWN__ERROR, "Failed to spawn item with type %u for group %u.", cur->npcTypeID, cur->spawnGroupID);
                continue;
            }

            _log(SPAWN__POP, "        [%d] spawning NPC id %u", r, i->itemID());

            //create them all at the same point to start with...
            //we will move them before they get added to the system
            NPC *npc = new NPC(&mgr, svc,
                i, cur->corporationID, 0, spawn_point, this);    //TODO: add allianceID
            spawned.push_back(npc);
        }
    }

    if(spawned.empty()) {
        int32 timer = static_cast<int32>(MakeRandomInt(m_timerMin, m_timerMax));
        _log(SPAWN__POP, "No NPCs produced by spawn entry %u. Resetting spawn timer to %d s.", m_id, timer);
        m_timer.Start(timer*1000);
        return;
    }

    //TODO: apply formation..
    //hacking it for now...
    std::vector<NPC *>::iterator curn, endn;
    curn = spawned.begin();
    endn = spawned.end();
    for(; curn != endn; curn++) {
        _log(SPAWN__POP, "Moving NPC %u to (%.1f, %.1f, %.1f) due to formation.", (*curn)->GetID(), spawn_point.x, spawn_point.y, spawn_point.z);
        (*curn)->ForcedSetPosition(spawn_point);
        spawn_point.x += 1000.0f;
        spawn_point.y += 1000.0f;
        spawn_point.z += 1000.0f;
    }


    //now the NPCs are all set up, lets drop them into the system.
    curn = spawned.begin();
    endn = spawned.end();
    for(; curn != endn; curn++) {
        //load up any NPC attributes...
        if(!(*curn)->Load(svc.serviceDB())) {
            _log(SPAWN__POP, "Failed to load NPC data for NPC %u with type %u, depoping.", (*curn)->GetID(), (*curn)->Item()->typeID());
            delete *curn;
            continue;
        }

        //record this NPC as something we spawned.
        m_spawnedIDs.insert((*curn)->GetID());

        mgr.AddNPC(*curn);

		if( warpThisSpawnIn )
			(*curn)->Destiny()->WarpTo(warp_in_point,0.0,true);
    }

    //timer is disabled while the spawn is up.
    m_timer.Disable();
	m_spawningNow = false;		// Spawning operation complete, turn off internal marker
}

void SpawnEntry::GetSpawnIDsList(std::set<uint32> &spawnIDsList)
{
	std::vector<SpawnGroup::Entry>::iterator cur, end;
    cur = m_group.entries.begin();
	end = m_group.entries.end();

	spawnIDsList.clear();
	for(; cur != end; cur++)
		spawnIDsList.insert((*cur).npcTypeID);
}

void SpawnEntry::MarkSpawned(uint32 npcID)
{
	m_spawnedIDs.insert(npcID);
}

void SpawnEntry::SpawnDepoped(uint32 npcID) {
    std::set<uint32>::iterator res;
    res = m_spawnedIDs.find(npcID);

    if(res == m_spawnedIDs.end()) {
        _log(SPAWN__ERROR, "SpawnDepopped() called for NPC %u on spawn entry %u, but that NPC is not in our spawned list.", npcID, m_id);
    } else {
        _log(SPAWN__DEPOP, "NPC %u depopped for spawn entry %u", npcID, m_id);
        m_spawnedIDs.erase(res);
    }

    if(m_spawnedIDs.empty()) {
        int32 timer = static_cast<int32>(MakeRandomInt(m_timerMin, m_timerMax));
		int32 timerMin = timer / 60;
		int32 timerSec = timer % 60;
        _log(SPAWN__DEPOP, "Spawn entry %u's entire spawn group has depopped, resetting timer to [%um %us]", m_id, timerMin, timerSec);
        m_timer.Start(timer*1000);
    }
}

bool SpawnEntry::CheckBounds() const {
    switch(m_boundsType) {

    case boundsPoint: {
        if(bounds.size() != 1) {
            _log(SPAWN__ERROR, "Invalid number of bounds points specified in spawn entry %u with type %u: %lu != 1", GetID(), m_boundsType, bounds.size());
            return false;
        }
    } break;

    case boundsLine: {
        if(bounds.size() != 2) {
            _log(SPAWN__ERROR, "Invalid number of bounds points specified in spawn entry %u with type %u: %lu != 2", GetID(), m_boundsType, bounds.size());
            return false;
        }
    } break;

    case boundsCube: {
        if(bounds.size() != 2) {
            _log(SPAWN__ERROR, "Invalid number of bounds points specified in spawn entry %u with type %u: %lu != 2", GetID(), m_boundsType, bounds.size());
            return false;
        }
    } break;

    default:
        _log(SPAWN__ERROR, "Invalid bounds type %u on spawn entry %u", m_boundsType, GetID());
        return false;
    }
    return true;
}



bool SpawnManager::Load() {
    //TODO: prune any invCategories::Entity items from the system. They
    // all get spawned by the spawn manager! Unless somebody changes
    // the spawn manager to be more persistent. This would require a table
    // mapping the entity table to the spawns table
    // (Drones too?)
    if(!m_db.LoadSpawnGroups(m_system.GetID(), m_groups)) {
        _log(SPAWN__ERROR, "Failed to load spawn groups for system %u!", m_system.GetID());
        return false;
    }

    if(!m_db.LoadSpawnEntries(m_system.GetID(), m_groups, m_spawns)) {
        _log(SPAWN__ERROR, "Failed to load spawn entries for system %u!", m_system.GetID());
        return false;
    }

    return true;
}

void SpawnManager::DoSpawnForBubble(SystemBubble &thisBubble)
{
	SpawnEntry * thisSpawn = NULL;

	thisSpawn = _FindSpawnForBubble(thisBubble);

	if( thisSpawn != NULL )
	{
		// We have a spawn entry for this bubble, process it:
		//thisSpawn->DoSpawnForBubble(m_system, thisBubble);
		thisSpawn->Process(m_system, m_services);
	}
}

bool SpawnManager::DoInitialSpawn() {
	// Loop through all spawn groups to verify if their areas are empty spawns and if not, DO NOT allow the inital spawn:
/*
	SystemBubble * currentBubble = NULL;
	uint32 npcTypeID;
	std::set<SystemEntity *>::iterator curEntity, endEntities;
	std::set<SystemEntity *> entitiesInBubble;
	std::set<uint32> spawnedIDsList;
	std::map<uint32, SpawnEntry *>::iterator cur, end;
    cur = m_spawns.begin();
    end = m_spawns.end();
    for(; cur != end; cur++) {
		// See if we can find a bubble in our system that would contain this spawn:
		if( cur->second->bounds.empty() )
			return false;

		currentBubble = m_system.bubbles.FindBubble( cur->second->bounds.at(0) );

		if( currentBubble != NULL )
		{
			// We found a bubble at our spawn!  Let's see if it has ships 
			entitiesInBubble.clear();
			currentBubble->GetEntities( entitiesInBubble );
			
			if( !(entitiesInBubble.empty()) )
			{
				// Bubble at our spawn has ships!  Let's see if one or more are from OUR spawn:
				cur->second->GetSpawnIDsList(spawnedIDsList);
				curEntity = entitiesInBubble.begin();
				endEntities = entitiesInBubble.end();
				for(; curEntity != endEntities; curEntity++)
				{
					if( (*curEntity)->Item() != 0 )
					{
						npcTypeID = (*curEntity)->Item()->typeID();
						if( spawnedIDsList.find( npcTypeID ) != spawnedIDsList.end() )
						{
							// We found one of our spawn items in this bubble!
							// Now we need to mark this SpawnEntry as 'spawned' so that it does not perform the inital spawn,
							// by inserting the ship itemIDs, not their typeIDs, into the SpawnEntry's spawnedIDs list:
							cur->second->MarkSpawned( (*curEntity)->Item()->itemID() );
							(*curEntity)->CastToNPC()->ForcedSetSpawner(cur->second);
						}
					}
				}
			}
		}
    }
*/
    // Right now, just running through Process will spawn what needs to be spawned.
    Process();
    return true;
}

void SpawnManager::Process() {
    std::map<uint32, SpawnEntry *>::iterator cur, end;
    cur = m_spawns.begin();
    end = m_spawns.end();
    for(; cur != end; cur++) {
        cur->second->Process(m_system, m_services);
    }
}

SpawnEntry * SpawnManager::_FindSpawnForBubble(SystemBubble &thisBubble)
{
	SystemBubble * bubble = NULL;
	std::map<uint32, SpawnEntry *>::iterator cur, end;
    cur = m_spawns.begin();
    end = m_spawns.end();
    for(; cur != end; cur++) {
		// Check each Spawn Entry to find a match for this bubble:
		if( !(cur->second->bounds.empty()) )
		{
			if( thisBubble.InBubble( cur->second->bounds.at(0) ) )
				// Found this SpawnEntry in this Bubble:
				return (cur->second);
		}
	}

	// Could not find a Spawn Entry for this bubble, return NULL:
	return NULL;
}
