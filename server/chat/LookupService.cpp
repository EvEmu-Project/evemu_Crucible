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


PyCallable_Make_InnerDispatcher(LookupService)


/*
class LookupSvcBound
: public LookupSvcObject {
public:
	
	class Dispatcher
	: public PyCallableDispatcher<LookupSvcBound> {
	public:
		Dispatcher(LookupSvcBound *c)
		: PyCallableDispatcher<LookupSvcBound>(c) {}
	};
	
	LookupSvcBound(PyServiceMgr *mgr, LookupSvcDB *db)
	: PyBoundObject(mgr, "LookupSvcBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(LookupSvcBound, )
		PyCallable_REG_CALL(LookupSvcBound, )
	}
	virtual ~LookupSvcBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	LookupSvcDB *const m_db;
	Dispatcher *const m_dispatch;
};
*/


LookupService::LookupService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "lookupSvc"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(LookupService, LookupCharacters)
	PyCallable_REG_CALL(LookupService, LookupPlayerCharacters)
	PyCallable_REG_CALL(LookupService, LookupCorporations)
	PyCallable_REG_CALL(LookupService, LookupFactions)
	PyCallable_REG_CALL(LookupService, LookupCorporationTickers)
	PyCallable_REG_CALL(LookupService, LookupStations)
	PyCallable_REG_CALL(LookupService, LookupLocationsByGroup)
	//PyCallable_REG_CALL(LookupService, )
}

LookupService::~LookupService() {
	delete m_dispatch;
}


/*
PyBoundObject *LookupService::_CreateBoundObject(Client *c, PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "LookupService bind request for:");
	bind_args->Dump(stdout, "    ");

	return(new LookupSvcBound(m_manager, &m_db));
}*/


PyResult LookupService::Handle_LookupCharacters(PyCallArgs &call) {
	Call_LookupStringInt args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupCharacters");
		return false;
	}
	
	return m_db.LookupChars(args.searchString.c_str(), args.searchOption);
}
PyResult LookupService::Handle_LookupPlayerCharacters(PyCallArgs &call) {
	Call_LookupStringInt args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupPlayerCharacters");
		return false;
	}

	return m_db.LookupPlayerChars(args.searchString.c_str(), args.searchOption);
}
PyResult LookupService::Handle_LookupCorporations(PyCallArgs &call) {
	Call_LookupStringInt args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupCorporations");
		return false;
	}
	
	return m_db.LookupCorporations(args.searchString);
}
PyResult LookupService::Handle_LookupFactions(PyCallArgs &call) {
	Call_LookupStringInt args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupCorporations");
		return false;
	}
	
	return m_db.LookupFactions(args.searchString);
}
PyResult LookupService::Handle_LookupCorporationTickers(PyCallArgs &call) {
	Call_LookupStringInt args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupCorporations");
		return false;
	}
	
	return m_db.LookupCorporationTickers(args.searchString);
}
PyResult LookupService::Handle_LookupStations(PyCallArgs &call) {
	Call_LookupStringInt args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupCorporations");
		return false;
	}
	
	return m_db.LookupStations(args.searchString);
}
// Asteroids, constellations and regions should be injected into the entity table...
PyResult LookupService::Handle_LookupLocationsByGroup(PyCallArgs &call) {
	Call_LookupIntString args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in LookupCorporations");
		return false;
	}
	
	return m_db.LookupLocationsByGroup(args.searchString, args.searchOption);
}
























