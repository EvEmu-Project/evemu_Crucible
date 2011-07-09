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


PyCallable_Make_InnerDispatcher(StationSvcService)


/*
class StationSvcBound
: public PyBoundObject {
public:
	
	class Dispatcher
	: public PyCallableDispatcher<StationSvcBound> {
	public:
		Dispatcher(StationSvcBound *c)
		: PyCallableDispatcher<StationSvcBound>(c) {}
	};
	
	StationSvcBound(PyServiceMgr *mgr, StationSvcDB *db)
	: PyBoundObject(mgr, "StationSvcBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(StationSvcBound, )
		PyCallable_REG_CALL(StationSvcBound, )
	}
	virtual ~StationSvcBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	StationSvcDB *const m_db;
	Dispatcher *const m_dispatch;
};
*/


StationSvcService::StationSvcService(PyServiceMgr *mgr)
: PyService(mgr, "stationSvc"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(StationSvcService, GetStationItemBits)
	PyCallable_REG_CALL(StationSvcService, GetSolarSystem)
	PyCallable_REG_CALL(StationSvcService, GetStation)
}

StationSvcService::~StationSvcService() {
	delete m_dispatch;
}

/*
PyBoundObject* StationSvcService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
	_log( CLIENT__MESSAGE, "StationSvcService bind request for:" );
	bind_args->Dump( CLIENT__MESSAGE, "    " );

	return new StationSvcBound( m_manager, &m_db );
}*/



PyResult StationSvcService::Handle_GetStationItemBits(PyCallArgs &call) {
	return m_db.GetStationItemBits(call.client->GetStationID());
}

PyResult StationSvcService::Handle_GetSolarSystem(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return NULL;
	}
	return(m_db.GetSolarSystem(arg.arg));
}

PyResult StationSvcService::Handle_GetStation(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return (new PyInt(0));
	}

	return m_db.DoGetStation(arg.arg);
}
