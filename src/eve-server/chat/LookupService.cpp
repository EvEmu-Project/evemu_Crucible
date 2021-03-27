/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur, Aknor Jaden
*/


#include "eve-server.h"

#include "PyServiceCD.h"
#include "chat/LookupService.h"


PyCallable_Make_InnerDispatcher(LookupService)

LookupService::LookupService(PyServiceMgr *mgr)
: PyService(mgr, "lookupSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(LookupService, LookupCharacters);
    PyCallable_REG_CALL(LookupService, LookupOwners);
    PyCallable_REG_CALL(LookupService, LookupPlayerCharacters);
    PyCallable_REG_CALL(LookupService, LookupCorporations);
    PyCallable_REG_CALL(LookupService, LookupFactions);
    PyCallable_REG_CALL(LookupService, LookupCorporationTickers);
    PyCallable_REG_CALL(LookupService, LookupStations);
    PyCallable_REG_CALL(LookupService, LookupKnownLocationsByGroup);
    PyCallable_REG_CALL(LookupService, LookupEvePlayerCharacters);
    PyCallable_REG_CALL(LookupService, LookupPCOwners);
    PyCallable_REG_CALL(LookupService, LookupNoneNPCAccountOwners);
}

LookupService::~LookupService() {
    delete m_dispatch;
}

PyResult LookupService::Handle_LookupEvePlayerCharacters(PyCallArgs& call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupChars(args.searchString.c_str(), args.searchOption ? true : false);
}

PyResult LookupService::Handle_LookupCharacters(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupChars(args.searchString.c_str(), args.searchOption ? true : false);
}

// this may actually be a call to search for player corps by name.
PyResult LookupService::Handle_LookupPCOwners(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupChars(args.searchString.c_str(), args.searchOption ? true : false);
}
//LookupOwners
PyResult LookupService::Handle_LookupOwners(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupOwners(args.searchString.c_str(),  args.searchOption ? true : false );
}

PyResult LookupService::Handle_LookupNoneNPCAccountOwners(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupOwners(args.searchString.c_str(),  args.searchOption ? true : false );
}

PyResult LookupService::Handle_LookupPlayerCharacters(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupChars(args.searchString.c_str(), false);
}
PyResult LookupService::Handle_LookupCorporations(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupCorporations(args.searchString);
}
PyResult LookupService::Handle_LookupFactions(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupFactions(args.searchString);
}
PyResult LookupService::Handle_LookupCorporationTickers(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupCorporationTickers(args.searchString);
}
PyResult LookupService::Handle_LookupStations(PyCallArgs &call) {
    Call_LookupStringInt args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupStations(args.searchString);
}

PyResult LookupService::Handle_LookupKnownLocationsByGroup(PyCallArgs &call) {
    Call_LookupIntString args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return ServiceDB::LookupKnownLocationsByGroup(args.searchString, args.searchOption);
}
