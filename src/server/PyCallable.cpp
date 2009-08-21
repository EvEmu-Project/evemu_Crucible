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

PyCallable::PyCallable()
: m_serviceDispatch(NULL)
{
}

PyCallable::~PyCallable()
{
}

PyResult PyCallable::Call(const std::string &method, PyCallArgs &args) {
	//call the dispatcher, capturing the result.
	try {
		PyResult res = m_serviceDispatch->Dispatch(method, args);

		_log(SERVICE__CALL_TRACE, "Call %s returned:", method.c_str());
		res.ssResult->Dump(SERVICE__CALL_TRACE, "      ");

		return res;
	} catch(PyException &e) {
		_log(SERVICE__CALL_TRACE, "Call %s threw exception:", method.c_str());
		e.ssException->Dump(SERVICE__CALL_TRACE, "      ");

		throw;
	}
}


PyCallArgs::PyCallArgs(Client *c, PyTuple **tup, PyDict **dict)
: client(c),
  tuple(*tup)
{
	*tup = NULL;
	
	PyDict *d = *dict;
	*dict = NULL;
	PyDict::iterator cur, end;
	cur = d->begin();
	end = d->end();
	for(; cur != end; cur++) {
		if(!cur->first->IsString()) {
			_log(SERVICE__ERROR, "Non-string key in call named arguments. Skipping.");
			cur->first->Dump(SERVICE__ERROR, "    ");
			continue;
		}
		PyString *s = (PyString *) cur->first;
		
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

/* default PyResult */
PyResult::PyResult()                : ssResult(NULL) {}
PyResult::PyResult(PyRep *result)   : ssResult((result==NULL) ? new PyNone() : result ) {}
PyResult::~PyResult() {}

/* default PyException */
PyException::PyException() : ssException(NULL) {}
PyException::PyException(PyRep *except) : ssException((except==NULL) ? new PyNone : except) {}
PyException::~PyException() {}