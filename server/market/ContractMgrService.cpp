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
//#include "ContractMgrService.h"



PyCallable_Make_InnerDispatcher(ContractMgrService)

ContractMgrService::ContractMgrService(PyServiceMgr *mgr)
: PyService(mgr, "contractMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ContractMgrService, NumRequiringAttention);
}

PyResult ContractMgrService::Handle_NumRequiringAttention(PyCallArgs &call) {
	_log(SERVICE__WARNING, "%s: NumReqiringAttention unimplemented.", GetName());

	PyRepDict *args = new PyRepDict;
	args->add("n", new PyRepInteger(0));
	args->add("ncorp", new PyRepInteger(0));

	return(new PyRepObject("util.KeyVal", args));
}


