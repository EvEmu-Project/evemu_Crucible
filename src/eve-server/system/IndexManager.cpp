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
    Author:        Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/IndexManager.h"
#include "system/sov/SovereigntyDataMgr.h"

PyCallable_Make_InnerDispatcher(IndexManager)

IndexManager::IndexManager(PyServiceMgr *mgr)
: PyService(mgr, "devIndexManager"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(IndexManager, GetAllDevelopmentIndices);
    PyCallable_REG_CALL(IndexManager, GetDevelopmentIndicesForSystem);
}

IndexManager::~IndexManager() {
    delete m_dispatch;
}

PyResult IndexManager::Handle_GetAllDevelopmentIndices( PyCallArgs& call ) {
    /*
22:49:13 L IndexManager::Handle_GetAllDevelopmentIndices(): size= 0
        for indexInfo in sm.RemoteSvc('devIndexManager').GetAllDevelopmentIndices():
            systemToIndexMap[indexInfo.solarSystemID] = {const.attributeDevIndexMilitary: indexInfo.militaryPoints,
             const.attributeDevIndexIndustrial: indexInfo.industrialPoints,
             const.attributeDevIndexSovereignty: indexInfo.claimedFor * CLAIM_DAYS_TO_SECONDS}
  */
    return nullptr;
}

PyResult IndexManager::Handle_GetDevelopmentIndicesForSystem( PyCallArgs& call ) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!IsSolarSystem(args.arg)) {
        codelog(SERVICE__ERROR, "%s: %d is not a solar system.", args.arg, GetName());
        return nullptr;
    }

    SovereigntyData const data = svDataMgr.GetSovereigntyData(args.arg);
    auto *props = new PyDict;
    props->SetItemString("points", new PyInt(data.militaryPoints));
    auto *mp = new PyObject("util.KeyVal", props);
    props = new PyDict;
    props->SetItemString("points", new PyInt(data.industrialPoints));
    auto *ip = new PyObject("util.KeyVal", props);
    auto *result = new PyDict;
    result->SetItemString("militaryPoints", mp);
    result->SetItemString("industrialPoints", ip);
    return result;
}

/*
developmentIndices = [attributeDevIndexMilitary, attributeDevIndexIndustrial, attributeDevIndexSovereignty]
attributeDevIndexMilitary = 1583
attributeDevIndexIndustrial = 1584
attributeDevIndexSovereignty = 1615



*/