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

PyCallable_Make_InnerDispatcher(MapService)

MapService::MapService(PyServiceMgr *mgr)
: PyService(mgr, "map"),
  m_dispatch(new Dispatcher(this))
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
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		
		PyTuple *resultt = new PyTuple(3);
		
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
		m_manager->cache_service->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
	
	return result;
}


PyResult MapService::Handle_GetSolarSystemPseudoSecurities(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetSolarSystemPseudoSecurities");

	//check to see if this method is in the cache already.
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetPseudoSecurities();
		if(result == NULL)
			result = new PyNone();
		m_manager->cache_service->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
	
	return result;
}

/* not handled */
PyResult MapService::Handle_GetStuckSystems(PyCallArgs &call) {
	PyRep *result = NULL;

	result = new PyDict();
	
	return result;
}
