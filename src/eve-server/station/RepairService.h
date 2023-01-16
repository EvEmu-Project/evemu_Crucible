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
*/

#ifndef __REPAIRSERVICE_SERVICE_INCL_H__
#define __REPAIRSERVICE_SERVICE_INCL_H__

#include "services/BoundService.h"

class Client;
class Inventory;
class RepairServiceBound;

class RepairService : public BindableService <RepairService, RepairServiceBound> {
public:
    RepairService(EVEServiceManager& mgr);

    void BoundReleased (RepairServiceBound* bound) override;

    static void GetDamageReports(uint32 itemID, Inventory* pInv, PyList* list);

protected:
    PyResult UnasembleItems(PyCallArgs& call, PyDict* validIDsByStationID, PyList* skipChecks);
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters) override;

private:
    std::map<uint32, RepairServiceBound*> m_instances;
};

class RepairServiceBound : public EVEBoundObject <RepairServiceBound> {
public:
    RepairServiceBound(EVEServiceManager& mgr, RepairService& parent, uint32 locationID);

    uint32 GetLocationID () { return this->m_locationID; }
protected:
    PyResult DamageModules(PyCallArgs& call, PyList* itemIDAndAmountOfDamage);
    PyResult RepairItems(PyCallArgs& call, PyList* itemIDs, PyFloat* iskAmount);
    PyResult GetDamageReports(PyCallArgs& call, PyList* itemIDs);

    bool CanClientCall(Client* client) override;

    uint32 m_locationID;
};

#endif