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



#include "PyServiceMgr.h"

#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/EVEUtils.h"

#include "PyService.h"
#include "PyCallable.h"
#include "PyBoundObject.h"
#include "EntityList.h"
#include "cache/ObjCacheService.h"

/*                                                                              
 * This is a quick proxy object which receives any calls with no service in
 * their destination, assumes they are calls on bound objects, and tries to find
 * that bound object to execute the call.
 *
*/
class PyServiceMgr::BoundCaller
: public PyService
{
public:
	BoundCaller(PyServiceMgr *mgr)
	: PyService(mgr, "BoundCallFakeService") {
	}
	virtual PyCallResult Call(PyCallStream &call, PyCallArgs &args) {
		if(call.remoteObject != 0) {
			_log(SERVICE__ERROR, "Service-less message received with an integer remote object ID %lu!", call.remoteObject);
			return(NULL);
		}

		PyBoundObject *called = m_manager->FindBoundObject(call.remoteObjectStr.c_str());
		if(called == NULL) {
			_log(SERVICE__ERROR, "Service-less message received for unknown bound object %s", call.remoteObjectStr.c_str());
			return(NULL);
		}

		return(called->Call(call, args));
	}
};


PyServiceMgr::PyServiceMgr(
	uint32 global_node_ID, 
	DBcore *db, 
	EntityList *elist, 
	ItemFactory *ifactory, 
	const std::string &CacheDirectory
)
: item_factory(ifactory),
  entity_list(elist),
  lsc_service(NULL),
  m_cache(new ObjCacheService(this, db, CacheDirectory)),
  m_globalNodeID(global_node_ID),
  m_nextBindID(100),
  m_svcDB(new ServiceDB(db))
{
	entity_list->UseServices(this);
	//cannot call this on the init list since it needs us to be fully constructed.
	m_BoundCallDispatcher = new BoundCaller(this);
	RegisterService(m_cache);
}

PyServiceMgr::~PyServiceMgr() {
	{
		std::set<PyService *>::iterator cur, end;
		cur = m_services.begin();
		end = m_services.end();
		for(; cur != end; cur++) {
			delete *cur;
		}
	}

	{
		std::map<std::string, BoundObject>::iterator cur, end;
		cur = m_boundObjects.begin();
		end = m_boundObjects.end();
		for(; cur != end; cur++) {
			delete cur->second.destination;
		}
	}

	delete m_BoundCallDispatcher;
	delete m_svcDB;
	//m_cache was in m_services
}

void PyServiceMgr::Process() {
	//well... we used to have something to do, but not right now...
}

void PyServiceMgr::RegisterService(PyService *d) {
	m_services.insert(d);
}

PyService *PyServiceMgr::LookupService(const PyPacket *p) {
	//first see if it is a bound call.
	if(p->dest.service == "")
		return(m_BoundCallDispatcher);
	
	std::set<PyService *>::iterator cur, end;
	cur = m_services.begin();
	end = m_services.end();
	for(; cur != end; cur++) {
		if((*cur)->IsPacketFor(p))
			return(*cur);
	}
	return(NULL);
}

uint32 PyServiceMgr::_AllocateBindID() {
	return(m_nextBindID++);
}

PyRepSubStruct *PyServiceMgr::BindObject(Client *c, PyBoundObject *cb, PyRepDict **dict) {
	if(cb == NULL) {
		_log(SERVICE__ERROR, "Tried to bind a NULL object!");
		return(new PyRepSubStruct(new PyRepNone()));
	}
	
	//generate a nice bind string:
	char bind_str[128];
	snprintf(bind_str, sizeof(bind_str), "N=%lu:%lu", GetNodeID(), _AllocateBindID());
	
	//not sure what this really is...
	uint64 expiration = Win32TimeNow();
	expiration += 10000000LL * 1000;

	BoundObject obj;
	obj.client = c;
	obj.destination = cb;
	
	m_boundObjects[bind_str] = obj;
	cb->m_bindID = bind_str;	//tell the object what its bind ID is.
	
	_log(SERVICE__MESSAGE, "Binding %s to service %s", bind_str, cb->GetName());
	
	PyRepTuple *objt;
	if(dict == NULL || *dict == NULL) {
		objt = new PyRepTuple(2);
		objt->items[0] = new PyRepString(bind_str);
		objt->items[1] = new PyRepInteger(expiration);	//expiration?
	} else {
		objt = new PyRepTuple(3);
		objt->items[0] = new PyRepString(bind_str);
		objt->items[1] = *dict; 
		*dict = NULL;	//consumed
		objt->items[2] = new PyRepInteger(expiration);	//expiration?
	}
	return(new PyRepSubStruct(new PyRepSubStream(objt)));
}

void PyServiceMgr::ClearBoundObjects(Client *who) {
	std::map<std::string, BoundObject>::iterator cur, end, tmp;
	cur = m_boundObjects.begin();
	end = m_boundObjects.end();
	while(cur != end) {
		if(cur->second.client == who) {
			tmp = cur;
			cur++;
			PyBoundObject *bo = tmp->second.destination;
			_log(SERVICE__MESSAGE, "Clearing bound object %s", tmp->first.c_str());
			m_boundObjects.erase(tmp);
			bo->Release();
		} else {
			cur++;
		}
	}
}

PyBoundObject *PyServiceMgr::FindBoundObject(const char *bindID) {
	std::map<std::string, BoundObject>::iterator res;
	res = m_boundObjects.find(bindID);
	if(res == m_boundObjects.end())
		return(NULL);
	return(res->second.destination);
}

void PyServiceMgr::ClientHasReleasedTheseObjects(const char *bindID) {
	
	std::map<std::string, BoundObject>::iterator res;
	res = m_boundObjects.find(bindID);
	if(res == m_boundObjects.end()) {
		_log(SERVICE__ERROR, "Unable to find bound object '%s' to release.", bindID);
		return;
	}
	
	PyBoundObject *bo = res->second.destination;
	_log(SERVICE__MESSAGE, "Clearing bound object %s (released)", res->first.c_str());
	m_boundObjects.erase(res);
	bo->Release();
}
















