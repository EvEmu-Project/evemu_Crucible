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



#include "BeyonceService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/gpoint.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../cache/ObjCacheService.h"
#include "../ship/DestinyManager.h"
#include "../system/SystemManager.h"

#include "../packets/General.h"
#include "../packets/Destiny.h"

PyCallable_Make_InnerDispatcher(BeyonceService)



class BeyonceBound
: public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(BeyonceBound)
	
	BeyonceBound(PyServiceMgr *mgr, ShipDB *db)
	: PyBoundObject(mgr, "BeyonceBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(BeyonceBound, FollowBall)
		PyCallable_REG_CALL(BeyonceBound, Orbit)
		PyCallable_REG_CALL(BeyonceBound, GotoDirection)
		PyCallable_REG_CALL(BeyonceBound, SetSpeedFraction)
		PyCallable_REG_CALL(BeyonceBound, Stop)
		PyCallable_REG_CALL(BeyonceBound, WarpToStuff)
		PyCallable_REG_CALL(BeyonceBound, Dock)
		PyCallable_REG_CALL(BeyonceBound, StargateJump)
		PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest)
	}
	virtual ~BeyonceBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(FollowBall)
	PyCallable_DECL_CALL(Orbit)
	PyCallable_DECL_CALL(GotoDirection)
	PyCallable_DECL_CALL(SetSpeedFraction)
	PyCallable_DECL_CALL(Stop)
	PyCallable_DECL_CALL(WarpToStuff)
	PyCallable_DECL_CALL(Dock)
	PyCallable_DECL_CALL(StargateJump)
	PyCallable_DECL_CALL(UpdateStateRequest)

protected:
	ShipDB *const m_db;
	Dispatcher *const m_dispatch;
};


BeyonceService::BeyonceService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "beyonce"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	//PyCallable_REG_CALL(BeyonceService, )
	PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
	delete m_dispatch;
}


PyBoundObject *BeyonceService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "BeyonceService bind request for:");
	bind_args->Dump(stdout, "    ");

	return(new BeyonceBound(m_manager, &m_db));
}


PyCallResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetRefTypes");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.

		//vicious crap... but this is gunna be a bit of work to load from the DB (nested tuples)
		PyRepTuple *forms = new PyRepTuple(2);
		result = forms;
		
		PyRepTuple *formation_t;
		PyRepTuple *list_t;
		PyRepTuple *item_t;
	
		//Diamond formation
		list_t = new PyRepTuple(4);
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(100);
		item_t->items[1] = new PyRepReal(0);
		item_t->items[2] = new PyRepReal(0);
		list_t->items[0] = item_t;
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(0);
		item_t->items[1] = new PyRepReal(100);
		item_t->items[2] = new PyRepReal(0);
		list_t->items[1] = item_t;
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(-100);
		item_t->items[1] = new PyRepReal(0);
		item_t->items[2] = new PyRepReal(0);
		list_t->items[2] = item_t;
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(0);
		item_t->items[1] = new PyRepReal(-100);
		item_t->items[2] = new PyRepReal(0);
		list_t->items[3] = item_t;
		
		formation_t = new PyRepTuple(2);
		formation_t->items[0] = new PyRepString("Diamond");
		formation_t->items[1] = list_t;
	
		forms->items[0] = formation_t;
	
	
		//Arrow formation
		list_t = new PyRepTuple(4);
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(100);
		item_t->items[1] = new PyRepReal(0);
		item_t->items[2] = new PyRepReal(-50);
		list_t->items[0] = item_t;
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(50);
		item_t->items[1] = new PyRepReal(0);
		item_t->items[2] = new PyRepReal(0);
		list_t->items[1] = item_t;
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(-100);
		item_t->items[1] = new PyRepReal(0);
		item_t->items[2] = new PyRepReal(-50);
		list_t->items[2] = item_t;
	
		item_t = new PyRepTuple(3);
		item_t->items[0] = new PyRepReal(-50);
		item_t->items[1] = new PyRepReal(0);
		item_t->items[2] = new PyRepReal(0);
		list_t->items[3] = item_t;
		
		formation_t = new PyRepTuple(2);
		formation_t->items[0] = new PyRepString("Arrow");
		formation_t->items[1] = list_t;
	
		forms->items[1] = formation_t;
		
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


/*
PyCallResult BeyonceService::Handle_(PyCallArgs &call) {
	PyRep *result = NULL;

	return(result);
}
*/



PyCallResult BeyonceBound::Handle_FollowBall(PyCallArgs &call) {
	//freakin python... the second arg is coming in as a real some times..
	PyRepTuple *iargs = call.tuple;

	//argument is a single integer, the ship id
	if(iargs->items.size() != 2) {
		codelog(CLIENT__ERROR, "Invalid arg count: %d", iargs->items.size());
		//TODO: throw exception
	} else if(!iargs->items[0]->CheckType(PyRep::Integer)) {
		codelog(CLIENT__ERROR, "Invalid argument: got %s, int expected", iargs->items[0]->TypeString());
	} else if(!iargs->items[1]->CheckType(PyRep::Integer) && !iargs->items[1]->CheckType(PyRep::Real)) {
		codelog(CLIENT__ERROR, "Invalid argument: got %s, int or real expected", iargs->items[0]->TypeString());
	} else {
		PyRepInteger *ball_id = (PyRepInteger *) iargs->items[0];
		
		uint32 distance = 0;
		if(iargs->items[1]->CheckType(PyRep::Integer)) {
			PyRepInteger *unknown = (PyRepInteger *) iargs->items[1];
			distance= unknown->value;
		} else {
			PyRepReal *unknown = (PyRepReal *) iargs->items[1];
			distance = (uint32) unknown->value;
		}

		SystemManager *system = call.client->GetSystem();
		if(system == NULL) {
			codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
			return(NULL);
		}
		
		SystemEntity *entity = system->get(ball_id->value);
		if(entity == NULL) {
			_log(CLIENT__ERROR, "%s: Unable to find entity %lu to Orbit.", call.client->GetName(), ball_id->value);
			return(NULL);
		}
		
		DestinyManager *destiny = call.client->Destiny();
		if(destiny == NULL) {
			codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
			return(NULL);
		}
		
		destiny->Follow(entity, distance);
	}
	

	return(NULL);
}

PyCallResult BeyonceBound::Handle_SetSpeedFraction(PyCallArgs &call) {
	Call_SingleRealArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return(NULL);
	}
		
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return(NULL);
	}
	
	destiny->SetSpeedFraction(arg.arg);

	return(NULL);
}

PyCallResult BeyonceBound::Handle_GotoDirection(PyCallArgs &call) {
	Call_PointArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return(NULL);
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return(NULL);
	}

	destiny->GotoDirection( GPoint( arg.x, arg.y, arg.z ) );

	return(NULL);
}

PyCallResult BeyonceBound::Handle_Orbit(PyCallArgs &call) {
	Call_TwoIntegerArgs arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return(NULL);
	}

	SystemManager *system = call.client->GetSystem();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return(NULL);
	}
	
	SystemEntity *entity = system->get(arg.arg1);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %lu to Orbit.", call.client->GetName(), arg.arg1);
		return(NULL);
	}
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return(NULL);
	}

	destiny->Orbit(entity, arg.arg2);

	return(NULL);
}

PyCallResult BeyonceBound::Handle_WarpToStuff(PyCallArgs &call) {
	CallWarpToStuff arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return(NULL);
	}

	//we need to delay the destiny updates until after we return

	GPoint pos;
	if(!m_db->GetStaticPosition(arg.item, pos.x, pos.y, pos.z)) {
		codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), arg.item);
		return(NULL);
	}

	call.client->WarpTo(pos);
	
	return(NULL);
}

PyCallResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
	//no arguments.
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return(NULL);
	}
	
	codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest! that means we messed up pretty bad.", call.client->GetName());
	destiny->SendSetState(call.client->Bubble());
	
	return(NULL);
}

PyCallResult BeyonceBound::Handle_Stop(PyCallArgs &call) {

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return(NULL);
	}

	destiny->Stop();

	return(NULL);
}

PyCallResult BeyonceBound::Handle_Dock(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return(NULL);
	}
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return(NULL);
	}
	
	double x,y,z;
	if(!m_db->GetStaticPosition(arg.arg, x, y, z)) {
		codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), arg.arg);
		return(NULL);
	}

	OnDockingAccepted da;
	da.start_x = x;
	da.start_y = y;
	da.start_z = z;
	da.end_x = x;
	da.end_y = y;
	da.end_z = z;
	da.stationID = arg.arg;

	GPoint start(da.start_x, da.start_y, da.start_z);
	GPoint end(da.end_x, da.end_y, da.end_z);
	GVector direction(start, end);
	direction.normalize();
	
	destiny->GotoDirection(direction);

	GPoint p(0, 0, 0);	//when docking, xyz dosent matter...
	call.client->MoveToLocation(arg.arg, p);

	return(NULL);
}

PyCallResult BeyonceBound::Handle_StargateJump(PyCallArgs &call) {
	Call_TwoIntegerArgs arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return(NULL);
	}

	call.client->StargateJump(arg.arg1, arg.arg2);
	
	return(NULL);
}























