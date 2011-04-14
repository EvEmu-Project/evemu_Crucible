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



PyCallable_Make_InnerDispatcher(BillMgrService)


BillMgrService::BillMgrService(PyServiceMgr *mgr)
: PyService(mgr, "billMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(BillMgrService, GetBillTypes)
	PyCallable_REG_CALL(BillMgrService, GetCorporationBills)
	PyCallable_REG_CALL(BillMgrService, GetCorporationBillsReceivable)
}

BillMgrService::~BillMgrService() {
	delete m_dispatch;
}


PyResult BillMgrService::Handle_GetBillTypes( PyCallArgs& call )
{
	PyRep* result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetBillTypes");

	//check to see if this method is in the cache already.
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetRefTypes();
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyNone();
		}
		m_manager->cache_service->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
	
	return result;
}

PyResult BillMgrService::Handle_GetCorporationBills(PyCallArgs &call) {
	// No incoming params
	uint32 corpID = call.client->GetCorporationID();

	return m_db.GetCorporationBills(corpID, true);
}
PyResult BillMgrService::Handle_GetCorporationBillsReceivable(PyCallArgs &call) {
	uint32 corpID = call.client->GetCorporationID();

	return m_db.GetCorporationBills(corpID, false);
}
























