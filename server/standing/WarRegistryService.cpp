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



PyCallable_Make_InnerDispatcher(WarRegistryService)

class WarRegistryBound : public PyBoundObject {
public:
	PyCallable_Make_Dispatcher(WarRegistryBound)

	WarRegistryBound(PyServiceMgr *mgr, uint32 corporationID)
	: PyBoundObject(mgr, "warRegistryBound"),
	  m_dispatch(new Dispatcher(this)),
	  m_corporationID(corporationID)
	{
		_SetCallDispatcher(m_dispatch);

		PyCallable_REG_CALL(WarRegistryBound, GetWars);
	}
	void Release() {
		//I hate this statement
		delete this;
	}

	PyCallable_DECL_CALL(GetWars)

private:
	Dispatcher *const m_dispatch;

	uint32 m_corporationID;
};

WarRegistryService::WarRegistryService(PyServiceMgr *mgr)
: PyService(mgr, "warRegistry"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);
}

PyBoundObject *WarRegistryService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	Call_TwoIntegerArgs args;
	//crap
	PyRep *t = bind_args->Clone();
	if(!args.Decode(&t)) {
		_log(SERVICE__ERROR, "%s: Failed to decode bind object params.", GetName());
		return NULL;
	}
	//arg1 = corporationID
	//arg2 = ???
	return(new WarRegistryBound(m_manager, args.arg1));
}

PyResult WarRegistryBound::Handle_GetWars(PyCallArgs &call) {
	_log(SERVICE__ERROR, "GetWars unimplemented.");

	util_IndexRowset irowset;

	irowset.header.push_back("warID");
	irowset.header.push_back("declaredByID");
	irowset.header.push_back("againstID");
	irowset.header.push_back("timeDeclared");
	irowset.header.push_back("timeFinished");
	irowset.header.push_back("retracted");
	irowset.header.push_back("retractedBy");
	irowset.header.push_back("billID");
	irowset.header.push_back("mutual");

	irowset.idName = "warID";

	return(irowset.Encode());
}


