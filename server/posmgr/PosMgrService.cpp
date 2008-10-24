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


PyCallable_Make_InnerDispatcher(PosMgrService)

PosMgrService::PosMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "posMgr"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(PosMgrService, GetControlTowerFuelRequirements)
	//PyCallable_REG_CALL(PosMgrService, )
}

PosMgrService::~PosMgrService() {
	delete m_dispatch;
}

/*
PyBoundObject *PosMgrService::_CreateBoundObject(Client *c, PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "PosMgrService bind request for:");
	bind_args->Dump(stdout, "    ");

	return(new PosMgrServiceBound(m_manager, &m_db));
}*/

PyResult PosMgrService::Handle_GetControlTowerFuelRequirements(PyCallArgs &args) {
	return m_db.GetControlTowerFuelRequirements();
}

