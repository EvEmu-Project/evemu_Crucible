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
    Author:        Reve
    Updates:    Allan
*/

#ifndef EVEMU_PLANET_PLANETMGR_BOUND_H_
#define EVEMU_PLANET_PLANETMGR_BOUND_H_

#include "services/BoundService.h"
#include "planet/PlanetDB.h"
#include "planet/PlanetMgr.h"

class PlanetMgrService : public BindableService <PlanetMgrService>
{
public:
    PlanetMgrService(EVEServiceManager& mgr);

protected:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters);

    PyResult GetPlanetsForChar(PyCallArgs& call);
    PyResult GetMyLaunchesDetails(PyCallArgs& call);
    PyResult GetPlanet(PyCallArgs& call, PyInt* planetID);
    PyResult DeleteLaunch(PyCallArgs& call, PyInt* launchID);
};


class PlanetMgrBound : public EVEBoundObject <PlanetMgrBound>
{
public:
    PlanetMgrBound(EVEServiceManager& mgr, PyRep* bindData, Client* client, PlanetSE* planet);
    
protected:
    bool CanClientCall(Client* client) override;

    PyResult GetPlanetResourceInfo(PyCallArgs& call);
    PyResult GetPlanetInfo(PyCallArgs& call);
    PyResult GetExtractorsForPlanet(PyCallArgs& call, PyInt* planetID);
    PyResult UserUpdateNetwork(PyCallArgs& call, PyList* commandList);
    PyResult GetProgramResultInfo(PyCallArgs& call, PyInt* ecuID, PyInt* typeID, PyList* heads, PyFloat* headRadius);
    PyResult GetResourceData(PyCallArgs& call, PyObject* info);
    PyResult UserAbandonPlanet(PyCallArgs& call);
    PyResult UserLaunchCommodities(PyCallArgs& call, PyInt* commandPinID, PyDict* commoditiesToLaunch);
    PyResult UserTransferCommodities(PyCallArgs& call, PyList* path, PyDict* commodities);
    PyResult GetCommandPinsForPlanet(PyCallArgs& call, PyInt* planetID);
    PyResult GetFullNetworkForOwner(PyCallArgs& call, PyInt* planetID, PyInt* characterID);
    PyResult GMAddCommodity(PyCallArgs& call, PyInt* pinID, PyInt* typeID, PyInt* quantity);
    PyResult GMConvertCommandCenter(PyCallArgs& call, PyInt* pinID);
    PyResult GMForceInstallProgram(PyCallArgs& call, PyInt* pinID, PyInt* typeID, PyInt* cycleTime, PyInt* lifetimeHours, PyInt* qtyPerCycle, PyFloat* radius);
    PyResult GMGetLocalDistributionReport(PyCallArgs& call, PyInt* planetID, PyTuple* surfacePoint);
    PyResult GMGetSynchedServerState(PyCallArgs& call, PyInt* characterID);
    PyResult GMRunDepletionSim(PyCallArgs& call);

    /*

    data = planet.remoteHandler.GMGetCompleteResource(resourceTypeID, layer)
        sh = builder.CreateSHFromBuffer(data.data, data.numBands)

    self.planet.remoteHandler.GMCreateNuggetLayer(self.planetID, typeID)
        self.GMShowResource(typeID, 'nuggets')      {{ 'nuggets' = layer here }}

        */

protected:
    Colony* m_colony;
    PlanetSE* m_planet;
    PlanetMgr* m_planetMgr;
};
#endif  // EVEMU_PLANET_PLANETMGR_BOUND_H_