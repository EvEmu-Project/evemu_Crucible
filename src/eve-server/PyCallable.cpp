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
    Author:        Zhur
*/

#include "eve-server.h"

#include "PyCallable.h"

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


PyCallArgs::PyCallArgs(Client *c, PyTuple* tup, PyDict* dict)
: client(c),
  tuple(tup)
{
    PyIncRef( tup );

    PyDict::const_iterator cur, end;
    cur = dict->begin();
    end = dict->end();
    for(; cur != end; cur++) {
        if(!cur->first->IsString()) {
            _log(SERVICE__ERROR, "Non-string key in call named arguments. Skipping.");
            cur->first->Dump(SERVICE__ERROR, "    ");
            continue;
        }
        byname[ cur->first->AsString()->content() ] = cur->second;
        PyIncRef( cur->second );
    }
}

PyCallArgs::~PyCallArgs() {
    PySafeDecRef( tuple );

    std::map<std::string, PyRep *>::iterator cur, end;
    cur = byname.begin();
    end = byname.end();
    for(; cur != end; cur++)
        PySafeDecRef( cur->second );
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

/* PyResult */
PyResult::PyResult( PyRep* result ) : ssResult( NULL == result ? new PyNone : result ) {}
PyResult::PyResult( const PyResult& oth ) : ssResult( NULL ) { *this = oth; }
PyResult::~PyResult() { PySafeDecRef( ssResult ); }

PyResult& PyResult::operator=( const PyResult& oth )
{
    PySafeDecRef( ssResult );
    ssResult = oth.ssResult;

    if( NULL != ssResult )
        PyIncRef( ssResult );

    return *this;
}

/* PyException */
PyException::PyException( PyRep* except ) : ssException( NULL == except ? new PyNone : except ) {}
PyException::PyException( const PyException& oth ) : ssException( NULL ) { *this = oth; }
PyException::~PyException() { PySafeDecRef( ssException ); }

PyException& PyException::operator=( const PyException& oth )
{
    PySafeDecRef( ssException );
    ssException = oth.ssException;

    if( NULL != ssException )
        PyIncRef( ssException );

    return *this;
}

