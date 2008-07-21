/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __SPAWN_MANAGER_H__
#define __SPAWN_MANAGER_H__


#include <vector>
#include <string>
#include <set>
#include <map>
#include "SpawnDB.h"
#include "../common/gpoint.h"
#include "../common/timer.h"

class SystemManager;
class PyServiceMgr;

//TODO: add formation stuff....

class SpawnGroup {
public:
	SpawnGroup(uint32 id, const char *name, uint32 formation);
	//~SpawnGroup();
	
	class Entry {
	public:
		Entry(
			uint32 spawnGroupID,
			uint32 npcTypeID,
			uint8 quantity,
			float probability,
			uint32 ownerID,
			uint32 corporationID);

		//I really want these to be const, but it makes things so much 
		//more complicated, so I dropped it for now.
		uint32 spawnGroupID;
		uint32 npcTypeID;
		uint8 quantity;
		float probability;
		uint32 ownerID;
		uint32 corporationID;
	};
	
	const uint32 id;
	const std::string name;
	const uint32 formation;
	
	std::vector<Entry> entries;
};

class SpawnEntry {
public:
	typedef enum {
		boundsPoint = 0,
		boundsLine = 1,
//		boundsTriangle = 2,
//		boundsSquare = 3,
//		boundsTetrahedron = 4,	//3 sided pyramid
//		boundsPyramid = 5,		//4 sided pyramid
		boundsCube = 6,
//		boundsSphere = 7,
		_maxBoundsType
	} SpawnBoundsType;
	
	SpawnEntry(
		uint32 id, 
		SpawnGroup *group, 
		uint32 timerMin, 
		uint32 timerMax, 
		uint32 timerValue,
		SpawnBoundsType boundsType);
	
	inline uint32 GetID() const { return(m_id); }

	//passing these pointers so we dont have to store them in each entry.
	void Process(SystemManager *mgr, PyServiceMgr *svc);
	
	bool CheckBounds() const;

	void SpawnDepoped(uint32 npcID);

	//I really dont want this to be public, but it makes loading a lot
	//easier right now, so here it is.
	std::vector<GPoint> bounds;
protected:
	void _DoSpawn(SystemManager *mgr, PyServiceMgr *svc);

	//curently spawned information:
	std::set<uint32> m_spawnedIDs;
	
	//static info:
	const uint32 m_id;
	SpawnGroup *const m_group;
	
	//spawn timer:
	const uint32 m_timerMin;	//in seconds
	const uint32 m_timerMax;	//in seconds
	Timer m_timer;
	
	//bounds:
	const SpawnBoundsType m_boundsType;
};

class SpawnManager {
public:
	SpawnManager(DBcore *db, SystemManager *mgr, PyServiceMgr *svc);
	~SpawnManager();
	
	bool Load();
	bool DoInitialSpawn();
	void Process();
	
protected:
	SystemManager *const m_system;	//we do not own this
	PyServiceMgr *const m_services;	//we do not own this
	
	SpawnDB m_db;
	
	std::map<uint32, SpawnGroup *> m_groups;	//we own these
	std::map<uint32, SpawnEntry *> m_spawns;	//we own these.
};


#endif



