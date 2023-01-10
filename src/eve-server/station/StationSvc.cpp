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

#include "Client.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "station/StationDataMgr.h"
#include "station/StationSvc.h"
#include "system/sov/SovereigntyDataMgr.h"

StationSvc::StationSvc(EVEServiceManager* mgr) :
    Service("stationSvc"),
    m_manager (mgr)
{
    this->Add("GetStationItemBits", &StationSvc::GetStationItemBits);
    this->Add("GetSolarSystem", &StationSvc::GetSolarSystem);
    this->Add("GetStation", &StationSvc::GetStation);
    this->Add("GetAllianceSystems", &StationSvc::GetAllianceSystems);
    this->Add("GetSystemsForAlliance", &StationSvc::GetSystemsForAlliance);

    this->m_cache = m_manager->Lookup <ObjCacheService>("objectCaching");
}

PyResult StationSvc::GetStationItemBits(PyCallArgs &call) {
    return stDataMgr.GetStationItemBits(call.client->GetStationID());
}

PyResult StationSvc::GetSolarSystem(PyCallArgs &call, PyInt* solarSystemID) {
    ObjectCachedMethodID method_id(GetName().c_str(), "GetSolarSystem");

    if (!this->m_cache->IsCacheLoaded(method_id)) {
        PyPackedRow *t = SystemDB::GetSolarSystemPackedRow(solarSystemID->value());

        this->m_cache->GiveCache(method_id, (PyRep **)&t);
    }

    return(this->m_cache->MakeObjectCachedMethodCallResult(method_id));
}

PyResult StationSvc::GetStation(PyCallArgs &call, PyInt* stationID) {
    return stDataMgr.GetStationPyData(stationID->value());
}

//This is called when opening up the sov dashboard
PyResult StationSvc::GetAllianceSystems(PyCallArgs &call) {
  sLog.White( "StationSvc::Handle_GetAllianceSystems()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return svDataMgr.GetAllianceSystems();
}

//This call is made by client when player opens 'Settled Systems' dropdown in alliance details ui
PyResult StationSvc::GetSystemsForAlliance(PyCallArgs &call, PyInt* allianceID) {
    // systems = sm.RemoteSvc('stationSvc').GetSystemsForAlliance(session.allianceid)
  sLog.White( "StationSvc::Handle_GetSystemsForAlliance()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}