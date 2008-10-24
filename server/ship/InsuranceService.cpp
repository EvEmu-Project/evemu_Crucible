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


PyCallable_Make_InnerDispatcher(InsuranceService)


/*
class InsuranceBound
: public PyBoundObject {
public:
	
	class Dispatcher
	: public PyCallableDispatcher<InsuranceBound> {
	public:
		Dispatcher(InsuranceBound *c)
		: PyCallableDispatcher<InsuranceBound>(c) {}
	};
	
	InsuranceBound(PyServiceMgr *mgr, InsuranceDB *db)
	: PyBoundObject(mgr, "InsuranceBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(InsuranceBound, )
		PyCallable_REG_CALL(InsuranceBound, )
	}
	virtual ~InsuranceBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	InsuranceDB *const m_db;
	Dispatcher *const m_dispatch;
};
*/


InsuranceService::InsuranceService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "insuranceSvc"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(InsuranceService, GetContractForShip)
	//PyCallable_REG_CALL(InsuranceService, )
}

InsuranceService::~InsuranceService() {
	delete m_dispatch;
}


/*
PyBoundObject *InsuranceService::_CreateBoundObject(Client *c, PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "InsuranceService bind request for:");
	bind_args->Dump(stdout, "    ");

	return(new InsuranceBound(m_manager, &m_db));
}*/


PyResult InsuranceService::Handle_GetContractForShip(PyCallArgs &call) {
	PyRep *result = NULL;

	result = new PyRepNone();

	_log(CLIENT__MESSAGE, "GetContractForShip not implemented!");
	
	return(result);
}


























