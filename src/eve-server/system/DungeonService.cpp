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


PyCallable_Make_InnerDispatcher(DungeonService)



/*
class DungeonBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(DungeonBound)
	
	DungeonBound(PyServiceMgr *mgr, DungeonDB *db)
	: PyBoundObject(mgr, "DungeonBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(DungeonBound, )
		PyCallable_REG_CALL(DungeonBound, )
	}
	virtual ~DungeonBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	DungeonDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


DungeonService::DungeonService(PyServiceMgr *mgr)
: PyService(mgr, "dungeon"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(DungeonService, DEGetFactions)
	PyCallable_REG_CALL(DungeonService, DEGetDungeons)
	PyCallable_REG_CALL(DungeonService, DEGetRooms)
}

DungeonService::~DungeonService() {
	delete m_dispatch;
}


/*
PyBoundObject *DungeonService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "DungeonService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new DungeonBound(m_manager, &m_db));
}*/


PyResult DungeonService::Handle_DEGetFactions( PyCallArgs& call )
{
	//PyRep *result = NULL;

    sLog.Debug( "DungeonService", "Called DEGetFactions stub." );

	return NULL;
}


PyResult DungeonService::Handle_DEGetDungeons( PyCallArgs& call )
{
	//PyRep *result = NULL;
	//dict args:
	// factionID
	// or dungeonVID

	// rows: status (1=RELEASE,2=TESTING,else Working Copy),
	//       dungeonVName
	//       dungeonVID 

    sLog.Debug( "DungeonService", "Called DEGetDungeons stub." );
	
	return NULL;
}


PyResult DungeonService::Handle_DEGetRooms( PyCallArgs& call )
{
	//dict arg: dungeonVID
	//PyRep *result = NULL;

	//rows: roomName

    sLog.Debug( "DungeonService", "Called DEGetRooms stub." );

	return NULL;
}























