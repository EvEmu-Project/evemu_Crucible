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


#ifndef __PYSERVICEMGR_H_INCL__
#define __PYSERVICEMGR_H_INCL__

#include "../common/common.h"
#include "../common/CachedObjectMgr.h"
#include "../common/timer.h"
#include "inventory/ItemFactory.h"

#include <set>
#include <map>
#include <string>

class PyService;
class PyCallable;
class PyBoundObject;
class PyPacket;
class ServiceDB;
class Client;
class PyRep;
class PyRepSubStruct;
class EntityList;
class ObjCacheService;
class DBcore;
class ItemFactory;

class LSCService;

class PyServiceMgr {
public:
	PyServiceMgr(uint32 nodeID, DBcore *db, EntityList *elist, ItemFactory *ifactory, const std::string &CacheDirectory);
	~PyServiceMgr();
	
	void Process();
		
	void RegisterService(PyService *d);
	PyService *LookupService(const PyPacket *p);
	
	ObjCacheService *GetCache() { return(m_cache); }
	uint32 GetNodeID() const { return(m_nodeID); }
	
	//object binding, not fully understood yet.
	PyRepSubStruct *BindObject(Client *who, PyBoundObject *obj, PyRepDict **dict = NULL);
	void ClearBoundObjects(Client *who);
	PyBoundObject *FindBoundObject(const char *bindID);
	void ClearBoundObject(const char *bindID);

	//this is a hack and needs to die:
	ServiceDB *GetServiceDB() const { return(m_svcDB); }

	ItemFactory *const item_factory;	//here for anybody to use. we do not own this.
	EntityList *const entity_list;	//here for anybody to use. we do not own this.
	
	//Area to access services by name. This isnt ideal, but it avoids casting.
	//these may be NULL during service init, but should never be after that.
	//we do not own these pointers (we do in their PyService * form though)
	LSCService *lsc_service; 
	
protected:
	std::set<PyService *> m_services;	//we own these pointers.

	ObjCacheService *const m_cache;
	
	//this is getting messy:
	uint32 _AllocateBindID();
	uint32 m_nodeID;
	uint32 m_nextBindID;
	class BoundObject {
	public:
		Client *client;	//we do not own this.
		PyBoundObject *destination;	//we own this. PyServiceMgr deletes it
	};
	std::map<std::string, BoundObject> m_boundObjects;
	class BoundCaller;
	BoundCaller *m_BoundCallDispatcher;

	ServiceDB *const m_svcDB;	//this is crap, get rid of this
};





#endif


