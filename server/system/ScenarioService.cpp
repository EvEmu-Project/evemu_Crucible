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


PyCallable_Make_InnerDispatcher(ScenarioService)



/*
class ScenarioBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(ScenarioBound)
	
	ScenarioBound(PyServiceMgr *mgr, ScenarioDB *db)
	: PyBoundObject(mgr, "ScenarioBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(ScenarioBound, )
		PyCallable_REG_CALL(ScenarioBound, )
	}
	virtual ~ScenarioBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	ScenarioDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


ScenarioService::ScenarioService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "scenario"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ScenarioService, ResetD)
}

ScenarioService::~ScenarioService() {
	delete m_dispatch;
}


/*
PyBoundObject *ScenarioService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "ScenarioService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new ScenarioBound(m_manager, &m_db));
}*/


PyResult ScenarioService::Handle_ResetD(PyCallArgs &call) {
	PyRep *result = NULL;

	codelog(SERVICE__ERROR, "Unhandled ResetD");

	return(result);
}

























