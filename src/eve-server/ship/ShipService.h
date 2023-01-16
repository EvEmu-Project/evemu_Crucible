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


#ifndef __SHIP_SERVICE_H_INCL__
#define __SHIP_SERVICE_H_INCL__

#include "services/BoundService.h"

class ShipBound;

class ShipService : public BindableService <ShipService, ShipBound>
{
public:
    ShipService(EVEServiceManager& mgr);

    void BoundReleased (ShipBound* bound) override;

protected:
    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters) override;

private:
    std::map<uint32, ShipBound*> m_instances;
};


class ShipBound : public EVEBoundObject <ShipBound>
{
public:
    ShipBound(EVEServiceManager& mgr, ShipService& parent, ShipItem* ship);

    uint32 GetShipID () { return this->pShip->itemID (); }

protected:
    PyResult Board(PyCallArgs& call, PyInt* newShipID, std::optional<PyInt*> oldShipID);
    PyResult Eject(PyCallArgs& call);
    PyResult LeaveShip(PyCallArgs& call, PyInt* shipID);
    PyResult ActivateShip(PyCallArgs& call, PyInt* newShipID, std::optional<PyInt*> oldShipID);
    PyResult Undock(PyCallArgs& call, PyInt* shipID, PyBool* ignoreContraband);
    PyResult Drop(PyCallArgs& call, PyList* PyToDropList, std::optional <PyInt*> ownerID, PyBool* ignoreWarning);
    PyResult Scoop(PyCallArgs& call, PyInt* itemID);
    PyResult ScoopDrone(PyCallArgs& call, PyList* itemIDs);
    PyResult Jettison(PyCallArgs& call, PyList* itemIDs);
    PyResult AssembleShip(PyCallArgs& args, PyInt* shipID);
    PyResult AssembleShip(PyCallArgs& call, PyList* itemIDs);
    PyResult GetShipConfiguration(PyCallArgs& call);
    PyResult ConfigureShip(PyCallArgs& call, PyDict* configuration);
    PyResult LaunchFromContainer(PyCallArgs& call, PyInt* structureID, PyList* ids);
    PyResult ScoopToSMA(PyCallArgs& call, PyInt* objectID);
    PyResult BoardStoredShip(PyCallArgs& call, PyInt* structureID, PyInt* shipID);
    PyResult StoreVessel(PyCallArgs& call, PyInt* destID);
    PyResult SelfDestruct(PyCallArgs& call, PyInt* shipID);

private:
    ShipItem* pShip;
};

#endif

/*{'FullPath': u'UI/Messages', 'messageID': 258809, 'label': u'ShipInvolvedInPoliceCrimeBody'}(u'That ship, or its owner, are currently engaged with local police forces. CONCORD prohibits you from boarding it until the engagement is over.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258810, 'label': u'ShipNotInHangarBody'}(u'Your ship is stuck, as server does not think that it is in a hangar. Please contact customer support to resolve this problem.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258811, 'label': u'ShipNotYoursTitle'}(u'Wrong Key', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258812, 'label': u'ShipNotYoursBody'}(u"You don't seem to have the right keys for the ship's ignition switch. Perhaps you left them in your other jacket?", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258813, 'label': u'ShipTooDamagedToBoardBody'}(u'That ship is too damaged to board. Somehow it is still holding together though.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258814, 'label': u'ShipTooDamagedToUndockBody'}(u'The ship you are piloting is too damaged to be handled in space. You do not have the skill to manage it in its current state.', None, None)
 */

//{'FullPath': u'UI/Messages', 'messageID': 259631, 'label': u'DropLocationUnsuitableBody'}(u'You cannot safely launch {dropitem} within {distance} of {items} in space.', None, {u'{dropitem}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'dropitem'}, u'{distance}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{items}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'items'}})
