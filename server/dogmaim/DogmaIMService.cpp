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



#include "DogmaIMService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/EVEUtils.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyBoundObject.h"
#include "../PyServiceMgr.h"
#include "../cache/ObjCacheService.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"
#include "../system/SystemManager.h"

#include "../packets/General.h"
#include "../packets/DogmaIM.h"

PyCallable_Make_InnerDispatcher(DogmaIMService)

class DogmaIMBound
: public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(DogmaIMBound)
	
	DogmaIMBound(PyServiceMgr *mgr, DogmaIMDB *db)
	: PyBoundObject(mgr, "DogmaIMBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(DogmaIMBound, ShipGetInfo)
		PyCallable_REG_CALL(DogmaIMBound, CharGetInfo)
		PyCallable_REG_CALL(DogmaIMBound, ItemGetInfo)
		PyCallable_REG_CALL(DogmaIMBound, CheckSendLocationInfo)
		PyCallable_REG_CALL(DogmaIMBound, GetTargets)
		PyCallable_REG_CALL(DogmaIMBound, GetTargeters)
		PyCallable_REG_CALL(DogmaIMBound, Activate)
		PyCallable_REG_CALL(DogmaIMBound, Deactivate)
		PyCallable_REG_CALL(DogmaIMBound, AddTarget)
		PyCallable_REG_CALL(DogmaIMBound, RemoveTarget)
		PyCallable_REG_CALL(DogmaIMBound, ClearTargets)
	}
	virtual ~DogmaIMBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(ShipGetInfo)
	PyCallable_DECL_CALL(CharGetInfo)
	PyCallable_DECL_CALL(ItemGetInfo)
	PyCallable_DECL_CALL(CheckSendLocationInfo)
	PyCallable_DECL_CALL(GetTargets)
	PyCallable_DECL_CALL(GetTargeters)
	PyCallable_DECL_CALL(Activate)
	PyCallable_DECL_CALL(Deactivate)
	PyCallable_DECL_CALL(AddTarget)
	PyCallable_DECL_CALL(RemoveTarget)
	PyCallable_DECL_CALL(ClearTargets)

protected:
	
	DogmaIMDB *const m_db;
	Dispatcher *const m_dispatch;
};


DogmaIMService::DogmaIMService(PyServiceMgr *mgr, DBcore *dbc)
: PyService(mgr, "dogmaIM"),
  m_dispatch(new Dispatcher(this)),
  m_db(dbc)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(DogmaIMService, GetAttributeTypes)
}

DogmaIMService::~DogmaIMService() {
	delete m_dispatch;
}


PyBoundObject *DogmaIMService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "DogmaIMService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new DogmaIMBound(m_manager, &m_db));
}


PyCallResult DogmaIMService::Handle_GetAttributeTypes(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRep *hint = call.client->GetServices()->GetCache()->GetCacheHint("dogmaIM.attributesByName");
	if(hint == NULL) {
		_log(CLIENT__ERROR, "Unable to load cache hint for dogmaIM.attributesByName");
		return(new PyRepNone());
	}
	result = hint;


	_log(CLIENT__MESSAGE, "Sending attributes type reply");

	return(result);
}

PyCallResult DogmaIMBound::Handle_ShipGetInfo(PyCallArgs &call) {
	//takes no arguments
	
	PyRepObject *result = call.client->Ship()->ShipGetInfo();
	if(result == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to build ship info for ship %lu", GetName(), call.client->Ship()->itemID());
		return(NULL);
	}
	
    return(result);
}

PyCallResult DogmaIMBound::Handle_ItemGetInfo(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		return(NULL);
	}
	
	InventoryItem *item = m_manager->item_factory->Load(args.arg, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", GetName(), args.arg);
		return(NULL);
	}

	PyRepObject *result = item->ItemGetInfo();
	item->Release();
	if(result == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to build item info for item %lu", GetName(), args.arg);
		return(NULL);
	}
	
    return(result);
}

PyCallResult DogmaIMBound::Handle_CharGetInfo(PyCallArgs &call) {
	//no arguments
	
	PyRepObject *result = call.client->Item()->CharGetInfo();
	if(result == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to build char info for char %lu", GetName(), call.client->Item()->itemID());
		return(NULL);
	}

	return(result);
}

PyCallResult DogmaIMBound::Handle_CheckSendLocationInfo(PyCallArgs &call) {
	//no arguments
	PyRep *result = NULL;

	result = new PyRepNone();
	_log(SERVICE__ERROR, "Unhandled DogmaIMBound::CheckSendLocationInfo");

	return(result);
}

PyCallResult DogmaIMBound::Handle_GetTargets(PyCallArgs &call) {
	//no arguments
	PyRep *result = NULL;
	
	result = call.client->targets.Encode_GetTargets();

	return(result);
}

PyCallResult DogmaIMBound::Handle_GetTargeters(PyCallArgs &call) {
	//no arguments
	PyRep *result = NULL;
	
	result = call.client->targets.Encode_GetTargeters();
	
	return(result);
}

PyCallResult DogmaIMBound::Handle_Activate(PyCallArgs &call) {
	Call_Dogma_Activate args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return(NULL);
	}
	
	codelog(SERVICE__ERROR, "Unimplemented (mostly)");
	
	//TODO: make sure we are allowed to do this.
	
	int res = call.client->modules.Activate(args.itemID, args.effectName, args.target, args.repeat);
	
	return(new PyRepInteger(res));
}

PyCallResult DogmaIMBound::Handle_Deactivate(PyCallArgs &call) {
	Call_Dogma_Deactivate args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return(NULL);
	}
	
	codelog(SERVICE__ERROR, "Mostly Unimplemented");
	
	//TODO: make sure we are allowed to do this.
	
	call.client->modules.Deactivate(args.itemID, args.effectName);

	/*InventoryItem *item = m_manager->item_factory->Load(args.itemID, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), args.itemID);
		return (NULL);
	}
	
	if(args.effectName == "online") {
		item->PutOffline();
	}*/
	
	return(NULL);
}

PyCallResult DogmaIMBound::Handle_AddTarget(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return(NULL);
	}
	
	SystemManager *smgr = call.client->GetSystem();
	if(smgr == NULL) {
		codelog(SERVICE__ERROR, "Unable to find system manager from '%s'", call.client->GetName());
		return(NULL);
	}
	SystemEntity *target = smgr->get(args.arg);
	if(target == NULL) {
		codelog(SERVICE__ERROR, "Unable to find entity %lu in system %lu from '%s'", args.arg, smgr->GetID(), call.client->GetName());
		return(NULL);
	}

	codelog(SERVICE__ERROR, "AddTarget timer not implemented.");
	call.client->targets.StartTargeting(target, 2000);


	Rsp_Dogma_AddTarget rsp;
	rsp.success = true;
	rsp.targets.push_back(target->GetID());
	
	return(rsp.FastEncode());
}

PyCallResult DogmaIMBound::Handle_RemoveTarget(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return(NULL);
	}
	
	SystemManager *smgr = call.client->GetSystem();
	if(smgr == NULL) {
		codelog(SERVICE__ERROR, "Unable to find system manager from '%s'", call.client->GetName());
		return(NULL);
	}
	SystemEntity *target = smgr->get(args.arg);
	if(target == NULL) {
		codelog(SERVICE__ERROR, "Unable to find entity %lu in system %lu from '%s'", args.arg, smgr->GetID(), call.client->GetName());
		return(NULL);
	}
	
	call.client->targets.ClearTarget(target);
	
	return(NULL);
}

PyCallResult DogmaIMBound::Handle_ClearTargets(PyCallArgs &call) {
	//no arguments.
	
	call.client->targets.ClearTargets();
	
	return(NULL);
}























