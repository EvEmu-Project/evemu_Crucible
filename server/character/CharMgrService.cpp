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

PyCallable_Make_InnerDispatcher(CharMgrService)

CharMgrService::CharMgrService(PyServiceMgr *mgr, DBcore *dbc)
: PyService(mgr, "charmgr"),
  m_dispatch(new Dispatcher(this)),
  m_db(dbc)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CharMgrService, GetPublicInfo)
	PyCallable_REG_CALL(CharMgrService, GetPublicInfo3)
}

CharMgrService::~CharMgrService() {
	delete m_dispatch;
}

PyResult CharMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
	//takes a single int arg: char id
	// or corp id
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
		return NULL;
	}

	/*if(IsAgent(args.arg)) {
		//handle agents special right now...
		PyRep *result = m_db.GetAgentPublicInfo(args.arg);
		if(result == NULL) {
			codelog(CLIENT__ERROR, "%s: Failed to find agent %lu", call.client->GetName(), args.arg);
			return NULL;
		}
		return(result);
	}*/

	PyRep *result = m_db.GetCharPublicInfo(args.arg);
	if(result == NULL) {
		codelog(CLIENT__ERROR, "%s: Failed to find char %lu", call.client->GetName(), args.arg);
		return NULL;
	}
	
	return(result);
}

PyResult CharMgrService::Handle_GetPublicInfo3(PyCallArgs &call) {
	//takes a single int arg: char id
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
		return NULL;
	}
	
	PyRep *result = m_db.GetCharPublicInfo3(args.arg);
	if(result == NULL) {
		codelog(CLIENT__ERROR, "%s: Failed to find char %lu", call.client->GetName(), args.arg);
		return NULL;
	}
	
	return(result);
}
























