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



#include "AuthService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/EVEUtils.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"

class AuthService::Dispatcher
: public PyCallableDispatcher<AuthService> {
public:
	Dispatcher(AuthService *c)
	: PyCallableDispatcher<AuthService>(c) {}
};


AuthService::AuthService(PyServiceMgr *mgr)
: PyService(mgr, "authentication"),
m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(AuthService, Ping)
	PyCallable_REG_CALL(AuthService, GetPostAuthenticationMessage)
}

AuthService::~AuthService() {
	delete m_dispatch;
}


PyCallResult AuthService::Handle_Ping(PyCallArgs &call) {
	PyRep *result = NULL;
	
	result = new PyRepInteger(Win32TimeNow());
	
	return(result);
}


PyCallResult AuthService::Handle_GetPostAuthenticationMessage(PyCallArgs &call) {
	PyRep *result = NULL;
/*
	PyRepObject *o = new PyRepObject();
	result = o;
		o->type = "util.KeyVal";
		PyRepDict *obj_args = new PyRepDict();
		o->arguments = obj_args;
			obj_args->items[ new PyRepString("message") ] 
				= new PyRepString("BrowseIGB");
			PyRepDict *args = new PyRepDict();
			obj_args->items[ new PyRepString("args") ] = args;
				args->items[ new PyRepString("showStatusBar") ] = new PyRepInteger(0);
				args->items[ new PyRepString("center") ] = new PyRepInteger(1);
				args->items[ new PyRepString("showAddressBar") ] = new PyRepInteger(0);
				args->items[ new PyRepString("url") ] = new PyRepString(
					"http://www.eve-online.com/igb/login/?username=aaa&total"
					"Trial=1858&trialMax=3000&createDate=2006.07.04&daysLeft=8&"
					"trialLen=14&login=yes&totalPlayers=18034&");
				args->items[ new PyRepString("showOptions") ] = new PyRepInteger(0);
				args->items[ new PyRepString("showButtons") ] = new PyRepInteger(0);
				args->items[ new PyRepString("showModal") ] = new PyRepInteger(1);
*/
	result = new PyRepNone();
	return(result);
}


















