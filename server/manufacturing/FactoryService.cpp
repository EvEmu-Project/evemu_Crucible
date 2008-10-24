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


PyCallable_Make_InnerDispatcher(FactoryService)

FactoryService::FactoryService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "factory"),
  m_db(db),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(FactoryService, GetBlueprintAttributes);
	PyCallable_REG_CALL(FactoryService, GetMaterialsForTypeWithActivity);
	PyCallable_REG_CALL(FactoryService, GetMaterialCompositionOfItemType);
}

FactoryService::~FactoryService() {
	delete m_dispatch;
}

PyResult FactoryService::Handle_GetBlueprintAttributes(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to decode args.");
		return(NULL);
	}

	Rsp_GetBlueprintAttributes rsp;

	if(!m_db.GetBlueprintAttributes(args.arg, rsp))
		return(NULL);

	return(rsp.Encode());
}

PyResult FactoryService::Handle_GetMaterialsForTypeWithActivity(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_TwoIntegerArgs call_args;
	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__MESSAGE, "Failed to decode args.");
		return(NULL);
	}

	result = m_db.GetMaterialsForTypeWithActivity(call_args.arg1);

	if(result == NULL)
		return(NULL);

	return(result);
}

PyResult FactoryService::Handle_GetMaterialCompositionOfItemType(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_SingleIntegerArg call_args;
	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__MESSAGE, "Failed to decode args.");
		return(NULL);
	}

	result = m_db.GetMaterialCompositionOfItemType(call_args.arg);

	if(result == NULL)
		return(NULL);

	return(result);
}

