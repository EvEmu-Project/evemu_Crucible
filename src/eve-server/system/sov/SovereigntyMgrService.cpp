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
    Author:        Reve,
    Rewrite:    Allan
    Updates:    James
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "system/sov/SovereigntyDB.h"
#include "system/sov/SovereigntyMgrService.h"

PyCallable_Make_InnerDispatcher(SovereigntyMgrService)

SovereigntyMgrService::SovereigntyMgrService(PyServiceMgr *mgr)
: PyService(mgr, "sovMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(SovereigntyMgrService, GetSystemSovereigntyInfo);
}

SovereigntyMgrService::~SovereigntyMgrService() {
    delete m_dispatch;
}

// this is only call to this service
PyResult SovereigntyMgrService::Handle_GetSystemSovereigntyInfo(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return svDataMgr.GetSystemSovereignty(args.arg);
}
