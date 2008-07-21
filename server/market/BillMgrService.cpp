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



#include "BillMgrService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../cache/ObjCacheService.h"

PyCallable_Make_InnerDispatcher(BillMgrService)


BillMgrService::BillMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "billMgr"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(BillMgrService, GetBillTypes)
	PyCallable_REG_CALL(BillMgrService, GetCorporationBills)
	PyCallable_REG_CALL(BillMgrService, GetCorporationBillsReceivable)
}

BillMgrService::~BillMgrService() {
	delete m_dispatch;
}


PyCallResult BillMgrService::Handle_GetBillTypes(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetRefTypes");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.

#ifndef WIN32
#warning can pull these from billTypes now.
#endif

		//start building the Rowset
		PyRepObject *rowset = new PyRepObject();
		result = rowset;
	
		rowset->type = "util.Rowset";
		PyRepDict *args = new PyRepDict();
		rowset->arguments = args;
	
		//header:
		PyRepList *header = new PyRepList();
		args->add("header", header);
		header->add("billTypeID");
		header->add("billTypeName");
		header->add("description");
	
		//RowClass:
		args->add("RowClass", new PyRepString("util.Row", true));
	
		//lines:
		PyRepList *alist = new PyRepList();
		args->add("lines", alist);
	
		PyRepList *idata = new PyRepList();
		alist->items.push_back(idata);
		idata->add(new PyRepInteger(1));
		idata->add(new PyRepString("Market Fine"));
		idata->add(new PyRepString(""));
	
		idata = new PyRepList();
		alist->items.push_back(idata);
		idata->add(new PyRepInteger(2));
		idata->add(new PyRepString("Rental Bill"));
		idata->add(new PyRepString(""));
	
		idata = new PyRepList();
		alist->items.push_back(idata);
		idata->add(new PyRepInteger(3));
		idata->add(new PyRepString("Broker Bill"));
		idata->add(new PyRepString(""));
	
		idata = new PyRepList();
		alist->items.push_back(idata);
		idata->add(new PyRepInteger(4));
		idata->add(new PyRepString("War Bill"));
		idata->add(new PyRepString(""));
	
		idata = new PyRepList();
		alist->items.push_back(idata);
		idata->add(new PyRepInteger(5));
		idata->add(new PyRepString("Alliance Maintainance Bill"));
		idata->add(new PyRepString(""));
		
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyRepNone();
		}
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}

PyCallResult BillMgrService::Handle_GetCorporationBills(PyCallArgs &call) {
	// No incoming params
	uint32 corpID = call.client->GetCorporationID();

	return m_db.GetCorporationBills(corpID, true);
}
PyCallResult BillMgrService::Handle_GetCorporationBillsReceivable(PyCallArgs &call) {
	uint32 corpID = call.client->GetCorporationID();

	return m_db.GetCorporationBills(corpID, false);
}
























