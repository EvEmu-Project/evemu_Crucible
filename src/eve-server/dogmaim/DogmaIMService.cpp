/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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

PyCallable_Make_InnerDispatcher(DogmaIMService)

class DogmaIMBound : public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(DogmaIMBound)
	
	DogmaIMBound(PyServiceMgr *mgr, DogmaIMDB *db)
	: PyBoundObject(mgr),
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
		PyCallable_REG_CALL(DogmaIMBound, GetWeaponBankInfoForShip)
	}
	virtual ~DogmaIMBound() {delete m_dispatch;}
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
	PyCallable_DECL_CALL(GetWeaponBankInfoForShip)

protected:
	
	DogmaIMDB *const m_db;
	Dispatcher *const m_dispatch;
};


DogmaIMService::DogmaIMService(PyServiceMgr *mgr)
: PyService(mgr, "dogmaIM"),
  m_dispatch(new Dispatcher(this))
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

PyResult DogmaIMService::Handle_GetAttributeTypes(PyCallArgs &call) {
    PyString* str = new PyString( "dogmaIM.attributesByName" );
	PyRep* result = m_manager->cache_service->GetCacheHint( str );
    PyDecRef( str );

	return result;
}

PyResult DogmaIMBound::Handle_ShipGetInfo(PyCallArgs &call) {
	//takes no arguments
	
	PyObject *result = call.client->GetShip()->ShipGetInfo();
	if(result == NULL) {
		codelog(SERVICE__ERROR, "Unable to build ship info for ship %u", call.client->GetShipID());
		return NULL;
	}
	
    return result;
}

PyResult DogmaIMBound::Handle_ItemGetInfo(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		return NULL;
	}
	
	InventoryItemRef item = m_manager->item_factory.GetItem( args.arg );
	if( !item ) {
		codelog(SERVICE__ERROR, "Unable to load item %u", args.arg);
		return NULL;
	}

    return item->ItemGetInfo();
}

PyResult DogmaIMBound::Handle_CharGetInfo(PyCallArgs &call) {
	//no arguments
	
	PyObject *result = call.client->GetChar()->CharGetInfo();
	if(result == NULL) {
		codelog(SERVICE__ERROR, "Unable to build char info for char %u", call.client->GetCharacterID());
		return NULL;
	}

	return result;
}

PyResult DogmaIMBound::Handle_CheckSendLocationInfo( PyCallArgs& call )
{
	//no arguments

    sLog.Debug( "DogmaIMBound", "Called CheckSendLocationInfo stub." );

	return new PyNone;
}

PyResult DogmaIMBound::Handle_GetTargets(PyCallArgs &call) {
	//no arguments
	return call.client->targets.GetTargets();
}

PyResult DogmaIMBound::Handle_GetTargeters(PyCallArgs &call) {
	//no arguments
	return call.client->targets.GetTargeters();
}

PyResult DogmaIMBound::Handle_Activate( PyCallArgs& call )
{
	Call_Dogma_Activate args;
	if( !args.Decode( &call.tuple ) )
    {
		codelog( SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName() );
		return NULL;
	}

    //TODO: make sure we are allowed to do this.
	return new PyInt( call.client->modules.Activate( args.itemID, args.effectName, args.target, args.repeat ) );
}

PyResult DogmaIMBound::Handle_Deactivate( PyCallArgs& call )
{
	Call_Dogma_Deactivate args;
	if( !args.Decode( &call.tuple ) )
    {
		codelog( SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName() );
		return NULL;
	}

    //TODO: make sure we are allowed to do this.
    call.client->modules.Deactivate( args.itemID, args.effectName );

	return NULL;
}

PyResult DogmaIMBound::Handle_AddTarget(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return NULL;
	}
	
	SystemManager *smgr = call.client->System();
	if(smgr == NULL) {
		codelog(SERVICE__ERROR, "Unable to find system manager from '%s'", call.client->GetName());
		return NULL;
	}
	SystemEntity *target = smgr->get(args.arg);
	if(target == NULL) {
		codelog(SERVICE__ERROR, "Unable to find entity %u in system %u from '%s'", args.arg, smgr->GetID(), call.client->GetName());
		return NULL;
	}

	codelog(SERVICE__ERROR, "AddTarget timer not implemented.");
	call.client->targets.StartTargeting(target, 2000);


	Rsp_Dogma_AddTarget rsp;
	rsp.success = true;
	rsp.targets.push_back(target->GetID());
	
	return(rsp.Encode());
}

PyResult DogmaIMBound::Handle_RemoveTarget(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return NULL;
	}
	
	SystemManager *smgr = call.client->System();
	if(smgr == NULL) {
		codelog(SERVICE__ERROR, "Unable to find system manager from '%s'", call.client->GetName());
		return NULL;
	}
	SystemEntity *target = smgr->get(args.arg);
	if(target == NULL) {
		codelog(SERVICE__ERROR, "Unable to find entity %u in system %u from '%s'", args.arg, smgr->GetID(), call.client->GetName());
		return NULL;
	}
	
	call.client->targets.ClearTarget(target);
	
	return NULL;
}

PyResult DogmaIMBound::Handle_ClearTargets(PyCallArgs &call) {
	//no arguments.
	
	call.client->targets.ClearTargets();
	
	return NULL;
}

PyResult DogmaIMBound::Handle_GetWeaponBankInfoForShip( PyCallArgs& call )
{
    sLog.Debug( "DogmaIMBound", "Called GetWeaponBankInfoForShip stub." );

    return new PyDict;
}
