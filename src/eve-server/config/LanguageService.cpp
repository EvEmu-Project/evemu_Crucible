/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

#include "EVEServerPCH.h"

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


LanguageService::LanguageService(PyServiceMgr *mgr)
: PyService(mgr, "languageSvc"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(LanguageService, GetLanguages)
	PyCallable_REG_CALL(LanguageService, GetTextsForGroup)
}

LanguageService::~LanguageService() {
	delete m_dispatch;
}


/*
PyBoundObject *LanguageService::_CreateBoundObject(Client *c, PyTuple *bind_args) {
	_log(CLIENT__MESSAGE, "LanguageService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new LanguageBound(m_manager, &m_db));
}*/


PyResult LanguageService::Handle_GetLanguages(PyCallArgs &call) {
	PyRep *result = NULL;

	result = m_db.ListLanguages();
	
	return result;
}
PyResult LanguageService::Handle_GetTextsForGroup(PyCallArgs &call) {
	Call_GetTextsForGroup args;

	if (!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	return m_db.GetTextsForGroup(args.languageID, args.textgroup);
}


























