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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

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
	if(m_timer.Check()) {
		if(!m_spawnedIDs.empty()) {
			_log(SPAWN__ERROR, "ERROR: spawn entry %u's timer went off when we have active spawn IDs!", m_id);
			m_spawnedIDs.clear();
		}
		
		//time to spawn...
		_DoSpawn(mgr, svc);
	}
}

void SpawnEntry::_DoSpawn(SystemManager &mgr, PyServiceMgr &svc) {
	_log(SPAWN__POP, "Spawning spawn entry %u with group %u", m_id, m_group.id);
	
	//pick our spawn point...
	GPoint spawn_point;
	switch(m_boundsType) {	//safe to assume `bounds` are correct.
	
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
	
	//now see what is gunna spawn...
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

			//NOTE: this is currently creating an entry in the DB...
			//which is terrible... we need to make an "in-memory only"
			// item concept.
			ItemData idata(
				cur->npcTypeID,
				cur->ownerID,	//owner
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
				i, cur->corporationID, 0, spawn_point, this);	//TODO: add allianceID
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
		spawn_point.y += 1000.0f;
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
	}
	
	//timer is disabled while the spawn is up.
	m_timer.Disable();
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
		_log(SPAWN__DEPOP, "Spawn entry %u's entire spawn group has depopped, resetting timer to %d s.", m_id, timer);
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

bool SpawnManager::DoInitialSpawn() {
	//right now, just running through Process will spawn what needs to be spawned.
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
























