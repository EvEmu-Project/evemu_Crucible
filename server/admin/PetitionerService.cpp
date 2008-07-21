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

#include "PetitionerService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"

PyCallable_Make_InnerDispatcher(PetitionerService)



/*
class PetitionerBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(PetitionerBound)
	
	PetitionerBound(PyServiceMgr *mgr, PetitionerDB *db)
	: PyBoundObject(mgr, "PetitionerBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(PetitionerBound, )
		PyCallable_REG_CALL(PetitionerBound, )
	}
	virtual ~PetitionerBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	PetitionerDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


PetitionerService::PetitionerService(PyServiceMgr *mgr)
: PyService(mgr, "petitioner"),
m_dispatch(new Dispatcher(this))
//m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(PetitionerService, GetCategories)
	PyCallable_REG_CALL(PetitionerService, GetUnreadMessages)
}

PetitionerService::~PetitionerService() {
	delete m_dispatch;
}


/*
PyBoundObject *PetitionerService::_CreateBoundObject(Client *c, PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "PetitionerService bind request for:");
	bind_args->Dump(stdout, "    ");

	return(new PetitionerBound(m_manager, &m_db));
}*/


PyCallResult PetitionerService::Handle_GetCategories(PyCallArgs &call) {
	//takes no arguments
	
	PyRepList *result = new PyRepList();
	result->add("Test Cat");
	result->add("Test Cat2");
	_log(SERVICE__ERROR, "Unhandled %s::GetCategories", GetName());

	return(result);
}


PyCallResult PetitionerService::Handle_GetUnreadMessages(PyCallArgs &call) {
	//takes no arguments

	//unknown...
	PyRepList *result = new PyRepList();
	
	_log(SERVICE__ERROR, "Unhandled %s::GetUnreadMessages", GetName());

	return(result);
}























