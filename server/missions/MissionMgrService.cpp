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
#include "EvemuPCH.h"


PyCallable_Make_InnerDispatcher(MissionMgrService)



/*
class MissionMgrBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(MissionMgrBound)
	
	MissionMgrBound(PyServiceMgr *mgr, MissionMgrDB *db)
	: PyBoundObject(mgr, "MissionMgrBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(MissionMgrBound, )
		PyCallable_REG_CALL(MissionMgrBound, )
	}
	virtual ~MissionMgrBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	MissionMgrDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


MissionMgrService::MissionMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "missionMgr"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(MissionMgrService, GetMyCourierMissions)
}

MissionMgrService::~MissionMgrService() {
	delete m_dispatch;
}


/*
PyBoundObject *MissionMgrService::_CreateBoundObject(Client *c, PyRepTuple *bind_args) {
	_log(CLIENT__MESSAGE, "MissionMgrService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new MissionMgrBound(m_manager, &m_db));
}*/


PyResult MissionMgrService::Handle_GetMyCourierMissions(PyCallArgs &call) {
	codelog(SERVICE__ERROR, "GetMyCourierMissions unimplemented");
	//SELECT * FROM courierMissions
	return NULL;
}





























