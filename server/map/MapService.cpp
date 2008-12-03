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


#include "EvemuPCH.h"


PyCallable_Make_InnerDispatcher(MapService)

MapService::MapService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "map"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(MapService, GetStationExtraInfo)
	PyCallable_REG_CALL(MapService, GetSolarSystemPseudoSecurities)
	PyCallable_REG_CALL(MapService, GetStuckSystems)
}

MapService::~MapService() {
	delete m_dispatch;
}


PyResult MapService::Handle_GetStationExtraInfo(PyCallArgs &call) {
	//takes no arguments
	//returns tuple(
	// 	stations: rowset stationID,solarSystemID,operationID,stationTypeID,ownerID
	// 	opservices: rowset: (operationID, serviceID) (from staOperationServices)
	// 	services: rowset: (serviceID,serviceName) (from staServices)
	// )

	PyRep *result = NULL;
	
	ObjectCachedMethodID method_id(GetName(), "GetStationExtraInfo");
	
	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		
		PyRepTuple *resultt = new PyRepTuple(3);
		
		resultt->items[0] = m_db.GetStationExtraInfo();
		if(resultt->items[0] == NULL) {
			codelog(SERVICE__ERROR, "Failed to query station info");
			return NULL;
		}
		
		resultt->items[1] = m_db.GetStationOpServices();
		if(resultt->items[1] == NULL) {
			codelog(SERVICE__ERROR, "Failed to query op services");
			return NULL;
		}
		
		resultt->items[2] = m_db.GetStationServiceInfo();
		if(resultt->items[2] == NULL) {
			codelog(SERVICE__ERROR, "Failed to query service info");
			return NULL;
		}

		result = resultt;
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}


PyResult MapService::Handle_GetSolarSystemPseudoSecurities(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetSolarSystemPseudoSecurities");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetPseudoSecurities();
		if(result == NULL)
			result = new PyRepNone();
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}


PyResult MapService::Handle_GetStuckSystems(PyCallArgs &call) {
	PyRep *result = NULL;

	result = new PyRepDict();
	
	return(result);
}

























