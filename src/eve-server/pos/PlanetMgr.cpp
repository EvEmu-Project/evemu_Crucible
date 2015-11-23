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
    Author:        Reve
*/

//work in progress

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "pos/PlanetMgr.h"

class PlanetMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetMgrBound)

    PlanetMgrBound(PyServiceMgr *mgr, uint32 planetID)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)), m_planetID(planetID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PlanetMgrBound";

        PyCallable_REG_CALL(PlanetMgrBound, GetCommandPinsForPlanet)
        PyCallable_REG_CALL(PlanetMgrBound, GetExtractorsForPlanet)
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetInfo)
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetResourceInfo)
        PyCallable_REG_CALL(PlanetMgrBound, GetProgramResultInfo)
        PyCallable_REG_CALL(PlanetMgrBound, GetResourceData)
        PyCallable_REG_CALL(PlanetMgrBound, GMAddCommodity)
        PyCallable_REG_CALL(PlanetMgrBound, GMConvertCommandCenter)
        PyCallable_REG_CALL(PlanetMgrBound, GMForceInstallProgram)
        PyCallable_REG_CALL(PlanetMgrBound, GMGetLocalDistributionReport)
        PyCallable_REG_CALL(PlanetMgrBound, GMGetSynchedServerState)
        PyCallable_REG_CALL(PlanetMgrBound, GMRunDepletionSim)
        PyCallable_REG_CALL(PlanetMgrBound, UserAbandonPlanet)
        PyCallable_REG_CALL(PlanetMgrBound, UserLaunchCommodities)
        PyCallable_REG_CALL(PlanetMgrBound, UserTransferCommodities)
        PyCallable_REG_CALL(PlanetMgrBound, UserUpdateNetwork)
    }
    virtual ~PlanetMgrBound() { delete m_dispatch; }
    virtual void Release() {
        //He hates this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetCommandPinsForPlanet)
    PyCallable_DECL_CALL(GetExtractorsForPlanet)
    PyCallable_DECL_CALL(GetPlanetInfo)
    PyCallable_DECL_CALL(GetPlanetResourceInfo)
    PyCallable_DECL_CALL(GetProgramResultInfo)
    PyCallable_DECL_CALL(GetResourceData)
    PyCallable_DECL_CALL(GMAddCommodity)
    PyCallable_DECL_CALL(GMConvertCommandCenter)
    PyCallable_DECL_CALL(GMForceInstallProgram)
    PyCallable_DECL_CALL(GMGetLocalDistributionReport)
    PyCallable_DECL_CALL(GMGetSynchedServerState)
    PyCallable_DECL_CALL(GMRunDepletionSim)
    PyCallable_DECL_CALL(UserAbandonPlanet)
    PyCallable_DECL_CALL(UserLaunchCommodities)
    PyCallable_DECL_CALL(UserTransferCommodities)
    PyCallable_DECL_CALL(UserUpdateNetwork)

protected:
    Dispatcher *const m_dispatch;
    const uint32 m_planetID;
};

PyCallable_Make_InnerDispatcher(PlanetMgrService)

PlanetMgrService::PlanetMgrService(PyServiceMgr *mgr)
: PyService(mgr, "planetMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PlanetMgrService, GetPlanetsForChar)
    PyCallable_REG_CALL(PlanetMgrService, GetMyLaunchesDetails)
}

PlanetMgrService::~PlanetMgrService() {
    delete m_dispatch;
}

PyBoundObject *PlanetMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return NULL;
    }
    return new PlanetMgrBound(m_manager, bind_args->AsInt()->value());
}

PyResult PlanetMgrService::Handle_GetPlanetsForChar(PyCallArgs &call) {
    sLog.Debug("Server", "Called GetPlanetsForChar Stub.");

    return NULL;
}

PyResult PlanetMgrService::Handle_GetMyLaunchesDetails(PyCallArgs &call) {
    sLog.Debug("Server", "Called GetMyLaunchesDetails Stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetCommandPinsForPlanet(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetCommandPinsForPlanet stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetExtractorsForPlanet(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetExtractorsForPlanet stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetPlanetInfo(PyCallArgs &call) {
    sLog.Debug("Server", "Called GetPlanetInfo Incomplete.");
    /* Incomplete, needs to check if planet is colonised by char, if so, return full colony + planet data.
     * Right now every planet is un-colonised.
     */

    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `typeID` AS `planetTypeID`, `itemID` AS `planetID`, `radius` FROM mapdenormalize WHERE `itemID` = %u", m_planetID)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetInfo query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if(res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetInfo query, failed to get row");
        return NULL;
    }
    return DBRowToKeyVal(row);
}

PyResult PlanetMgrBound::Handle_GetPlanetResourceInfo(PyCallArgs &call) {
    sLog.Debug("Server", "Called GetPlanetResourceInfo Stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetProgramResultInfo(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetProgramResultInfo stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetResourceData(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetResourceData stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMAddCommodity(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMAddCommodity stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMConvertCommandCenter(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMConvertCommandCenter stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMForceInstallProgram(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMForceInstallProgram stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMGetLocalDistributionReport(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMGetLocalDistributionReport stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMGetSynchedServerState(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMGetSynchedServerState stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMRunDepletionSim(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMRunDepletionSim stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserAbandonPlanet(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserAbandonPlanet stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserLaunchCommodities(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserLaunchCommodities stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserTransferCommodities(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserTransferCommodities stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserUpdateNetwork(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserUpdateNetwork stub.");

    return NULL;
}
