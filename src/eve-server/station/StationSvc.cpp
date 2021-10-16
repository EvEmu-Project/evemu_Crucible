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

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "station/StationDataMgr.h"
#include "station/StationSvc.h"

#include "system/sov/SovereigntyDataMgr.h"
#include "Client.h"


PyCallable_Make_InnerDispatcher(StationSvc)

StationSvc::StationSvc(PyServiceMgr *mgr)
: PyService(mgr, "stationSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(StationSvc, GetStationItemBits);
    PyCallable_REG_CALL(StationSvc, GetSolarSystem);
    PyCallable_REG_CALL(StationSvc, GetStation);
    PyCallable_REG_CALL(StationSvc, GetAllianceSystems);
    PyCallable_REG_CALL(StationSvc, GetSystemsForAlliance);
}

StationSvc::~StationSvc() {
    delete m_dispatch;
}

PyResult StationSvc::Handle_GetStationItemBits(PyCallArgs &call) {
    return stDataMgr.GetStationItemBits(call.client->GetStationID());
}

PyResult StationSvc::Handle_GetSolarSystem(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    ObjectCachedMethodID method_id(GetName(), "GetSolarSystem");

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyPackedRow *t = SystemDB::GetSolarSystem(arg.arg);

        m_manager->cache_service->GiveCache(method_id, (PyRep **)&t);
    }

    return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
}

PyResult StationSvc::Handle_GetStation(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return stDataMgr.GetStationPyData(arg.arg);
}

//This is called when opening up the sov dashboard
PyResult StationSvc::Handle_GetAllianceSystems(PyCallArgs &call) {
  sLog.White( "StationSvc::Handle_GetAllianceSystems()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return svDataMgr.GetAllianceSystems();
}

//This call is made by client when player opens 'Settled Systems' dropdown in alliance details ui
PyResult StationSvc::Handle_GetSystemsForAlliance(PyCallArgs &call) {
  sLog.White( "StationSvc::Handle_GetSystemsForAlliance()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}