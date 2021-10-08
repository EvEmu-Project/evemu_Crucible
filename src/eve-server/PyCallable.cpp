/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
: m_serviceDispatch(nullptr)
{
}

PyCallable::~PyCallable()
{
}

PyResult PyCallable::Call(const std::string &method, PyCallArgs &args) {
    //call the dispatcher, capturing the result.
    try {
        PyResult res(m_serviceDispatch->Dispatch(method, args));

        if (is_log_enabled(SERVICE__CALL_TRACE)) {
            _log(SERVICE__CALL_TRACE, "Call %s returned:", method.c_str());
            res.ssResult->Dump(SERVICE__CALL_TRACE, "      ");
        }
        return res;
    } catch(PyException &e) {
        if (is_log_enabled(SERVICE__CALL_ERROR)) {
            _log(SERVICE__CALL_ERROR, "Call %s threw exception:", method.c_str());
            e.ssException->Dump(SERVICE__CALL_ERROR, "      ");
        }
        throw;
    }
}


PyCallArgs::PyCallArgs(Client *c, PyTuple* tup, PyDict* dict)
: client(c),
  tuple(tup)
{
   // PyIncRef( tup );
    for (PyDict::const_iterator cur = dict->begin(); cur != dict->end(); cur++) {
        if (!cur->first->IsString()) {
            _log(SERVICE__ERROR, "Non-string key in call named arguments. Skipping.");
            cur->first->Dump(SERVICE__ERROR, "    ");
            continue;
        }
        PyIncRef( cur->second );
        byname[ cur->first->AsString()->content() ] = cur->second;
    }
}

PyCallArgs::~PyCallArgs() {
    PySafeDecRef( tuple );
    for (auto cur : byname)
        PySafeDecRef( cur.second );
}

void PyCallArgs::Dump(LogType type) const {
    if (!is_log_enabled(type))
        return;

    _log(type, "  Call Arguments:");
    tuple->Dump(type, "    ");
    if (!byname.empty()) {
        _log(type, " Named Arguments:");
        for (auto cur : byname) {
            _log(type, "  %s", cur.first.c_str());
            cur.second->Dump(type, "    ");
        }
    }
}

/* PyResult */
PyResult::PyResult() : ssResult( nullptr ), ssNamedResult( nullptr ) {}
PyResult::PyResult(PyRep* result)
: ssResult(result != nullptr ? result : PyStatic.NewNone()),
ssNamedResult( nullptr )
{}
PyResult::PyResult(PyRep* result, PyDict* namedResult)
: ssResult(result != nullptr ? result : PyStatic.NewNone()),
ssNamedResult(namedResult)
{}

PyResult::PyResult( const PyResult& oth ) : ssResult( nullptr ), ssNamedResult( nullptr ) { *this = oth; }
PyResult::~PyResult() { PySafeDecRef( ssResult ); PySafeDecRef( ssNamedResult ); }

PyResult& PyResult::operator=( const PyResult& oth )
{
    PySafeDecRef( ssResult );
    if (oth.ssResult != nullptr ) {
        ssResult = oth.ssResult;
    } else {
        ssResult = PyStatic.NewNone();
    }
    PySafeIncRef( ssResult );

    PySafeDecRef( ssNamedResult );
    ssNamedResult = oth.ssNamedResult;
    PySafeIncRef( ssNamedResult );

    return *this;
}
