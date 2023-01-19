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
    Updates:    Allan
*/

#include "eve-server.h"

#include "NetService.h"
#include "cache/ObjCacheService.h"

NetService::NetService(EVEServiceManager& mgr) :
    Service("machoNet"),
    m_manager (mgr)
{
    this->Add("GetTime", &NetService::GetTime);
    this->Add("GetClusterSessionStatistics", &NetService::GetClusterSessionStatistics);
    this->Add("GetInitVals", &NetService::GetInitVals);

    this->m_cache = this->m_manager.Lookup <ObjCacheService>("objectCaching");
}

PyResult NetService::GetTime(PyCallArgs &call) {
    return new PyLong(GetFileTimeNow());
}

PyResult NetService::GetClusterSessionStatistics(PyCallArgs &call)
{
    // got this shit working once i understood what the client wanted....only took 4 years
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT solarSystemID, pilotsInSpace, pilotsDocked FROM mapDynamicData"); // WHERE active = 1");
    /** @todo  instead of hitting db, call solarsystem to get docked/inspace and NOT afk
     *   client already has IsAFK()
     */

    uint16 system(0);
    PyDict* sol = new PyDict();
    PyDict* sta = new PyDict();

    DBResultRow row;
    while (res.GetRow(row)) {
        system = row.GetUInt(0) - 30000000;
        sol->SetItem(new PyInt(system), new PyInt(row.GetUInt(1) + row.GetUInt(2)));    // inspace + docked = total
        sta->SetItem(new PyInt(system), new PyInt(row.GetUInt(2)));                     // total - docked
    }

    PyTuple *result = new PyTuple(3);
    result->SetItem(0, sol);
    result->SetItem(1, sta);
    result->SetItem(2, new PyFloat(1)); //statDivisor

    return result;
}

/** @note:  wtf is this used for???  */
PyResult NetService::GetInitVals(PyCallArgs &call) {
    PyString* str(new PyString("machoNet.serviceInfo"));

    //  look into this.  what's it for?  are we using it right?  missing anything?
    // client calls this, then loads cached data upon return.  not sure how this is used yet
    if (!this->m_cache->IsCacheLoaded(str)) {
        PyDict* dict = new PyDict();
        // build the service info for the client to know where to direct calls
        for (auto svc : this->m_manager.GetServices()) {
            PyRep* value = nullptr;

            switch (svc.second->GetAccessLevel()) {
                case eAccessLevel_None:
                    value = PyStatic.NewNone();
                    break;
                case eAccessLevel_Location:
                    value = new PyString("location");
                    break;
                case eAccessLevel_LocationPreferred:
                    value = new PyString("locationPreferred");
                    break;
                case eAccessLevel_SolarSystem:
                    value = new PyString("solarsystem");
                    break;
                case eAccessLevel_SolarSystem2:
                    value = new PyString("solarsystem2");
                    break;
                case eAccessLevel_Station:
                    value = new PyString("station");
                    break;
                case eAccessLevel_Character:
                    value = new PyString("character");
                    break;
                case eAccessLevel_Corporation:
                    value = new PyString("corporation");
                    break;
                case eAccessLevel_User:
                    value = new PyString("bulk");
                    break;
            }

            dict->SetItemString(svc.second->GetName().c_str(), value);
        }
        /* ServiceCallGPCS.py:197
         *        where = self.machoNet.serviceInfo[service]
         *        if where:
         *            for (k, v,) in self.machoNet.serviceInfo.iteritems():
         *                if ((k != service) and (v and (v.startswith(where) or where.startswith(v)))):
         *                    nodeID = self.services.get(k, None)
         *                    break
         */

        //register it
        this->m_cache->GiveCache(str, (PyRep**)&dict);
    }

    PyRep* serverinfo(this->m_cache->GetCacheHint(str));
    PyDecRef( str );

    PyDict* initvals = new PyDict();
    PyTuple* result = new PyTuple( 2 );
        result->SetItem( 0, serverinfo );
        result->SetItem( 1, initvals );
    return result;
}
