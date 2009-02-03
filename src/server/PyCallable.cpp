/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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

#include "EvemuPCH.h"

PyCallable::PyCallable(PyServiceMgr &mgr, const char *callableName)
: m_manager(&mgr),
  m_callableName(callableName),
  m_serviceDispatch(NULL)
{
}

PyCallable::~PyCallable()
{
}

PyResult PyCallable::Call(PyCallStream &call, PyCallArgs &args) {

	_log(SERVICE__CALLS, "%s Service: calling %s", m_callableName.c_str(), call.method.c_str());
	args.Dump(SERVICE__CALL_TRACE);
	
	//call the dispatcher, capturing the result.
	try {
		PyResult res = m_serviceDispatch->Dispatch(call.method, args);

		_log(SERVICE__CALL_TRACE, "%s Service: Call %s returned:", m_callableName.c_str(), call.method.c_str());
		res.ssResult->Dump(SERVICE__CALL_TRACE, "      ");

		return(res);
	} catch(PyException &e) {
		_log(SERVICE__CALL_TRACE, "%s Service: Call %s threw exception:", m_callableName.c_str(), call.method.c_str());
		e.ssException->Dump(SERVICE__CALL_TRACE, "      ");

		throw;
	}
}


PyCallArgs::PyCallArgs(Client *c, PyRepTuple **tup, PyRepDict **dict)
: client(c),
  tuple(*tup)
{
	*tup = NULL;
	
	PyRepDict *d = *dict;
	*dict = NULL;
	PyRepDict::iterator cur, end;
	cur = d->begin();
	end = d->end();
	for(; cur != end; cur++) {
		if(!cur->first->IsString()) {
			_log(SERVICE__ERROR, "Non-string key in call named arguments. Skipping.");
			cur->first->Dump(SERVICE__ERROR, "    ");
			continue;
		}
		PyRepString *s = (PyRepString *) cur->first;
		
		byname[s->value] = cur->second;
		cur->second = NULL;
	}
	//do not do anything with d except delete it, it has NULL pointers in it.
	delete d;
}

PyCallArgs::~PyCallArgs() {
	delete tuple;
	std::map<std::string, PyRep *>::iterator cur, end;
	cur = byname.begin();
	end = byname.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

void PyCallArgs::Dump(LogType type) const {
	if(!is_log_enabled(type))
		return;
	
	_log(type, "  Call Arguments:");
	tuple->Dump(type, "      ");
	if(!byname.empty()) {
		_log(type, "  Call Named Arguments:");
		std::map<std::string, PyRep *>::const_iterator cur, end;
		cur = byname.begin();
		end = byname.end();
		for(; cur != end; cur++) {
			_log(type, "    Argument '%s':", cur->first.c_str());
			cur->second->Dump(type, "        ");
		}
	}
}

PyResult::PyResult()
: ssResult(NULL)
{
	//_log(SERVER__INIT, "Constructing Regular NULL");
}

PyResult::PyResult(PyRep *result)
: ssResult(
	  (result==NULL)
	  ? new PyRepNone()
	  : result
	  )
{
	//_log(SERVER__INIT, "Constructing Regular %p", &(*ssResult));
}

PyResult::~PyResult() {
	//_log(SERVER__INIT, "Destroying Regular %p", &(*ssResult));
}

PyException::PyException()
: ssException(NULL)
{
	_log(SERVER__INIT, "Constructing Exception NULL");
}

PyException::PyException(PyRep *except)
: ssException(
		(except==NULL)
		? new PyRepNone
		: except
		)
{
	_log(SERVER__INIT, "Constructing Exception %p", &(*ssException));
}

PyException::~PyException() {
	_log(SERVER__INIT, "Destroying Exception %p", &(*ssException));
}


























