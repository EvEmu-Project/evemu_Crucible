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

#ifndef __SYSTEMMANAGER_H_INCL__
#define __SYSTEMMANAGER_H_INCL__

#include "SystemDB.h"
#include "BubbleManager.h"
#include <map>
#include <set>


class PyRep;
class PyRepDict;
class PyRepTuple;
class PyRepList;
class Client;
class NPC;
class InventoryItem;
class SystemEntity;
class SystemBubble;
class DoDestiny_SetState;


class SpawnManager;
class PyServiceMgr;

class SystemManager {
public:
	SystemManager(const char *systemName, uint32 systemID, DBcore *db, PyServiceMgr *svc);
	virtual ~SystemManager();
	
	//bubble stuff:
	BubbleManager bubbles;
	
	void RangecastDestiny(const GPoint &pt, double range, std::vector<PyRepTuple *> &updates, std::vector<PyRepTuple *> &events) const;
	void RangecastDestinyUpdate(const GPoint &pt, double range, PyRepTuple **payload) const;
	void RangecastDestinyEvent(const GPoint &pt, double range, PyRepTuple **payload) const;
	
	uint32 GetID() const { return(m_systemID); }
	const std::string &GetName() const { return(m_systemName); }
	double GetWarpSpeed() const;
	
	bool BootSystem();
	
	bool Process();
	void ProcessDestiny();	//called once for each destiny second.

	void AddClient(Client *who);
	void RemoveClient(Client *who);
	void AddNPC(NPC *who);
	void RemoveNPC(NPC *who);
	void AddEntity(SystemEntity *who);
	void RemoveEntity(SystemEntity *who);
	
	SystemEntity *get(uint32 entityID) const;
	
	void MakeSetState(const SystemBubble *bubble, DoDestiny_SetState &into) const;

	uint32 AllocateNPCID() { return(m_nextNPCID++); }
	
	SystemDB *GetSystemDB() { return(&m_db); }
	const char * GetSystemSecurity() { return m_systemSecurity.c_str(); }

	inline ItemFactory *itemFactory() const;
	
protected:
	bool _LoadSystemCelestials();
	bool _LoadSystemDynamics();
	
	const std::string m_systemName;
	const uint32 m_systemID;
	std::string m_systemSecurity;
	SystemDB m_db;
	PyServiceMgr *const m_services;	//we do not own this
	SpawnManager *m_spawnManager;	//we own this, never NULL, dynamic to keep the knowledge down.
	
	uint32 m_nextNPCID;
	
	//overall system entity lists:
	bool m_clientChanged;
	bool m_entityChanged;
	std::set<Client *> m_clients;	//we do not own these
	std::map<uint32, SystemEntity *> m_entities;	//we own these, but they are also referenced in m_bubbles
};





#endif


