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


#ifndef __BEYONCE_SERVICE_H_INCL__
#define __BEYONCE_SERVICE_H_INCL__

#include "ship/ShipDB.h"
#include "services/BoundService.h"
#include "system/BookmarkService.h"

class BeyonceService : public BindableService <BeyonceService> {
public:
    BeyonceService(EVEServiceManager& mgr);

protected:
    ShipDB m_db;

    PyResult GetFormations(PyCallArgs& call);

    //overloaded in order to support bound objects:
    BoundDispatcher *BindObject(Client *client, PyRep* bindParameters) override;
};


class BeyonceBound : public EVEBoundObject <BeyonceBound>
{
public:
    BeyonceBound(EVEServiceManager& mgr, PyRep* bindData, Client* client);

protected:
    bool CanClientCall(Client* client) override;

    PyResult CmdFollowBall(PyCallArgs& call, PyInt* ballID, PyRep* distance);
    PyResult CmdSetSpeedFraction(PyCallArgs& call, PyFloat* speedFraction);
    PyResult CmdAlignTo(PyCallArgs& call, PyInt* entityID);
    PyResult CmdGotoDirection(PyCallArgs& call, PyFloat* x, PyFloat* y, PyFloat* z);
    PyResult CmdGotoBookmark(PyCallArgs& call, PyInt* bookmarkID);
    PyResult CmdOrbit(PyCallArgs& call, PyInt* entityID, PyRep* rangeValue);
    PyResult CmdWarpToStuff(PyCallArgs& call, PyString* type, PyRep* id);
    PyResult CmdWarpToStuffAutopilot(PyCallArgs& call, PyInt* destID);
    PyResult CmdStop(PyCallArgs& call);
    PyResult CmdDock(PyCallArgs& call, PyInt* celestialID, PyInt* shipID, std::optional<PyRep*> paymentRequired);
    PyResult CmdStargateJump(PyCallArgs& call, PyInt* fromStargateID, PyInt* toStargateID, PyInt* shipID);
    PyResult CmdAbandonLoot(PyCallArgs& call, PyList* wreckIDs);
    PyResult UpdateStateRequest(PyCallArgs& call);
    PyResult CmdJumpThroughFleet(PyCallArgs& call, PyInt* otherCharID, PyInt* otherShipID, PyInt* beaconID, PyInt* solarSystemID);
    PyResult CmdJumpThroughAlliance(PyCallArgs& call, PyInt* otherShipID, PyInt* beaconID, PyInt* solarSystemID);
    PyResult CmdJumpThroughCorporationStructure(PyCallArgs& call, PyInt* itemID, PyInt* remoteStructureID, PyInt* remoteSystemID);
    PyResult CmdBeaconJumpFleet(PyCallArgs& call, PyInt* characterID, PyInt* beaconID, PyInt* solarSystemID);
    PyResult CmdBeaconJumpAlliance(PyCallArgs& call, PyInt* beaconID, PyInt* solarSystemID);
    PyResult CmdFleetRegroup(PyCallArgs& call);
    PyResult CmdFleetTagTarget(PyCallArgs& call, PyInt* itemID, PyString* tag);
    
private:
    BookmarkService* m_bookmark;
};

#endif
