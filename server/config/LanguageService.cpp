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

PyCallable_Make_InnerDispatcher(LanguageService)



/*
class LanguageBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(LanguageBound)
	
	LanguageBound(PyServiceMgr *mgr, LanguageDB *db)
	: PyBoundObject(mgr, "LanguageBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(LanguageBound, )
		PyCallable_REG_CALL(LanguageBound, )
	}
	virtual ~LanguageBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	LanguageDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


LanguageService::LanguageService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "languageSvc"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(LanguageService, GetLanguages)
	PyCallable_REG_CALL(LanguageService, GetTextsForGroup)
}

LanguageService::~LanguageService() {
	delete m_dispatch;
}


/*
PyBoundObject *LanguageService::_CreateBoundObject(Client *c, PyRepTuple *bind_args) {
	_log(CLIENT__MESSAGE, "LanguageService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new LanguageBound(m_manager, &m_db));
}*/


PyResult LanguageService::Handle_GetLanguages(PyCallArgs &call) {
	PyRep *result = NULL;

	result = m_db.ListLanguages();
	
	return(result);
}
PyResult LanguageService::Handle_GetTextsForGroup(PyCallArgs &call) {
	Call_GetTextsForGroup args;

	if (!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	return m_db.GetTextsForGroup(args.languageID, args.textgroup);
}


























