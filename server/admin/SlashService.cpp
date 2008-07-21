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


//note, using /slashes from LSC requires ROLE_SLASH



#include "SlashService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "CommandDispatcher.h"
//#include "../PyBoundObject.h"

#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(SlashService)



/*
class SlashBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(SlashBound)
	
	SlashBound(PyServiceMgr *mgr, SlashDB *db)
	: PyBoundObject(mgr, "SlashBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(SlashBound, )
		PyCallable_REG_CALL(SlashBound, )
	}
	virtual ~SlashBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	SlashDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


SlashService::SlashService(PyServiceMgr *mgr, CommandDispatcher *cd)
: PyService(mgr, "slash"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(SlashService, SlashCmd)
}

SlashService::~SlashService() {
	delete m_dispatch;
}


/*
PyBoundObject *SlashService::_CreateBoundObject(Client *c, PyRepTuple *bind_args) {
	_log(CLIENT__MESSAGE, "SlashService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new SlashBound(m_manager, &m_db));
}*/


PyCallResult SlashService::Handle_SlashCmd(PyCallArgs &call) {
	Call_SingleStringArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		return(NULL);
	}

	if(!(call.client->GetRole() & ROLE_SLASH)) {
		_log(SERVICE__ERROR, "%s: Client '%s' used a slash command but does not have ROLE_SLASH. Modified client?", GetName(), call.client->GetName());
		return(NULL);
	}

	m_commandDispatch->Execute(call.client, args.arg.c_str());

	//this string appears in the character's LOG window.
	return(new PyRepString("Slash Result Test"));
}

























