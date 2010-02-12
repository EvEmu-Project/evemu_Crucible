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

PyCallable_Make_InnerDispatcher(BeyonceService)

class BeyonceBound
: public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(BeyonceBound)
	
	BeyonceBound(PyServiceMgr *mgr, Client *c)
	: PyBoundObject(mgr),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(BeyonceBound, FollowBall)
		PyCallable_REG_CALL(BeyonceBound, Orbit)
		PyCallable_REG_CALL(BeyonceBound, AlignTo)
		PyCallable_REG_CALL(BeyonceBound, GotoDirection)
		PyCallable_REG_CALL(BeyonceBound, SetSpeedFraction)
		PyCallable_REG_CALL(BeyonceBound, Stop)
		PyCallable_REG_CALL(BeyonceBound, WarpToStuff)
		PyCallable_REG_CALL(BeyonceBound, Dock)
		PyCallable_REG_CALL(BeyonceBound, StargateJump)
		PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest)

		if(c->Destiny() != NULL)
			c->Destiny()->SendSetState(c->Bubble());
	}
	virtual ~BeyonceBound() {delete m_dispatch;}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(FollowBall)
	PyCallable_DECL_CALL(Orbit)
	PyCallable_DECL_CALL(AlignTo)
	PyCallable_DECL_CALL(GotoDirection)
	PyCallable_DECL_CALL(SetSpeedFraction)
	PyCallable_DECL_CALL(Stop)
	PyCallable_DECL_CALL(WarpToStuff)
	PyCallable_DECL_CALL(Dock)
	PyCallable_DECL_CALL(StargateJump)
	PyCallable_DECL_CALL(UpdateStateRequest)

protected:
	Dispatcher *const m_dispatch;
};


BeyonceService::BeyonceService(PyServiceMgr *mgr)
: PyService(mgr, "beyonce"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	//PyCallable_REG_CALL(BeyonceService, )
	PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
	delete m_dispatch;
}


PyBoundObject* BeyonceService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
	_log( CLIENT__MESSAGE, "BeyonceService bind request for:" );
	bind_args->Dump( CLIENT__MESSAGE, "    " );

	return new BeyonceBound( m_manager, c );
}


PyResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
	ObjectCachedMethodID method_id(GetName(), "GetFormations");

	//check to see if this method is in the cache already.
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		PyRep *res = m_db.GetFormations();
		if(res == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			res = new PyNone();
		}

		m_manager->cache_service->GiveCache(method_id, &res);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
}

/*
PyResult BeyonceService::Handle_(PyCallArgs &call) {
	PyRep *result = NULL;

	return result;
}
*/

PyResult BeyonceBound::Handle_FollowBall(PyCallArgs &call) {
	Call_FollowBall args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
		return NULL;
	}

	double distance;
	if( args.distance->IsInt() )
		distance = args.distance->AsInt()->value();
    else if( args.distance->IsFloat() )
		distance = args.distance->AsFloat()->value();
    else
    {
		codelog(CLIENT__ERROR, "%s: Invalid type %s for distance argument received.", call.client->GetName(), args.distance->TypeString());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	
	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return NULL;
	}
	SystemEntity *entity = system->get(args.ballID);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.ballID);
		return NULL;
	}
	
	destiny->Follow(entity, distance);

	return NULL;
}

PyResult BeyonceBound::Handle_SetSpeedFraction(PyCallArgs &call) {
	Call_SingleRealArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}
		
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	
	destiny->SetSpeedFraction(arg.arg);

	return NULL;
}

/* AlignTo
 * This will look up the entityID to get it's position in space, then call
 * AlignTo to have it respond with gotopoint.
 * @author Xanarox
*/
PyResult BeyonceBound::Handle_AlignTo(PyCallArgs &call) {
	CallAlignTo arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return NULL;
	}

	SystemEntity *entity = system->get(arg.entityID);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
		return NULL;
	}

	const GPoint &position = entity->GetPosition();
	destiny->AlignTo( position );

	return NULL;
}

PyResult BeyonceBound::Handle_GotoDirection(PyCallArgs &call) {
	Call_PointArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	destiny->GotoDirection( GPoint( arg.x, arg.y, arg.z ) );

	return NULL;
}

PyResult BeyonceBound::Handle_Orbit(PyCallArgs &call) {
	Call_Orbit arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	double distance;
	if( arg.distance->IsInt() )
		distance = arg.distance->AsInt()->value();
    else if( arg.distance->IsFloat() )
		distance = arg.distance->AsFloat()->value();
    else
    {
		codelog(CLIENT__ERROR, "%s: Invalid type %s for distance argument received.", call.client->GetName(), arg.distance->TypeString());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return NULL;
	}
	SystemEntity *entity = system->get(arg.entityID);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), arg.entityID);
		return NULL;
	}

	destiny->Orbit(entity, distance);

	return NULL;
}

PyResult BeyonceBound::Handle_WarpToStuff(PyCallArgs &call) {
	CallWarpToStuff arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	double distance;
	std::map<std::string, PyRep *>::const_iterator res = call.byname.find("minRange");
	if(res == call.byname.end()) {
		codelog(CLIENT__ERROR, "%s: range not found, using 15 km.", call.client->GetName());
		distance = 15000.0;
	} else if(!res->second->IsInt() && !res->second->IsFloat()) {
		codelog(CLIENT__ERROR, "%s: range of invalid type %s, expected Integer or Real; using 15 km.", call.client->GetName(), res->second->TypeString());
		distance = 15000.0;
	} else {
		distance =
			res->second->IsInt()
			? res->second->AsInt()->value()
			: res->second->AsFloat()->value();
	}

	//we need to delay the destiny updates until after we return

	SystemManager *sm = call.client->System();
	if(sm == NULL) {
		codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
		return NULL;
	}
	SystemEntity *se = sm->get(arg.item);
	if(se ==  NULL) {
		codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), arg.item);
		return NULL;
	}
	// don't forget to add radiuses
	distance += call.client->GetRadius() + se->GetRadius();

	call.client->WarpTo(se->GetPosition(), distance);
	
	return NULL;
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
	codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest! that means we messed up pretty bad.", call.client->GetName());

	//no arguments.
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	
	destiny->SendSetState(call.client->Bubble());
	
	return NULL;
}

PyResult BeyonceBound::Handle_Stop(PyCallArgs &call) {
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	destiny->Stop();

	return NULL;
}

PyResult BeyonceBound::Handle_Dock(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	SystemManager *sm = call.client->System();
	if(sm == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager.", call.client->GetName());
		return NULL;
	}
	SystemEntity *station = sm->get(arg.arg);
	if(station == NULL) {
		codelog(CLIENT__ERROR, "%s: Station %u not found.", call.client->GetName(), arg.arg);
		return NULL;
	}
	
	const GPoint &position = station->GetPosition();

	OnDockingAccepted da;
	da.start_x = da.end_x = position.x;
	da.start_y = da.end_y = position.y;
	da.start_z = da.end_z = position.z;
	da.stationID = arg.arg;

	GPoint start(da.start_x, da.start_y, da.start_z);
	GPoint end(da.end_x, da.end_y, da.end_z);
	GVector direction(start, end);
	direction.normalize();
	
	destiny->GotoDirection(direction);

	//when docking, xyz doesn't matter...
	call.client->MoveToLocation(arg.arg, GPoint(0, 0, 0));

	return NULL;
}

PyResult BeyonceBound::Handle_StargateJump(PyCallArgs &call) {
	Call_TwoIntegerArgs arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	call.client->StargateJump(arg.arg1, arg.arg2);
	
	return NULL;
}
