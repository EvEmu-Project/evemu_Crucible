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


#include "DungeonService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"

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


DungeonService::DungeonService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "dungeon"),
m_dispatch(new Dispatcher(this)),
m_db(db)
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


PyCallResult DungeonService::Handle_DEGetFactions(PyCallArgs &call) {
	//PyRep *result = NULL;

	_log(SERVICE__ERROR, "Unhandled DEGetFactions");

	return(NULL);
}


PyCallResult DungeonService::Handle_DEGetDungeons(PyCallArgs &call) {
	//PyRep *result = NULL;
	//dict args:
	// factionID
	// or dungeonVID

	// rows: status (1=RELEASE,2=TESTING,else Working Copy),
	//       dungeonVName
	//       dungeonVID 

	_log(SERVICE__ERROR, "Unhandled DEGetDungeons");
	
	return(NULL);
}


PyCallResult DungeonService::Handle_DEGetRooms(PyCallArgs &call) {
	//dict arg: dungeonVID
	//PyRep *result = NULL;

	//rows: roomName

	_log(SERVICE__ERROR, "Unhandled DEGetRooms");

	return(NULL);
}























