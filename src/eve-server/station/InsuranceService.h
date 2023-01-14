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


#ifndef __INSURANCE_SERVICE_H_INCL__
#define __INSURANCE_SERVICE_H_INCL__

#include "ship/ShipDB.h"
#include "services/BoundService.h"

class InsuranceService : public BindableService <InsuranceService> {
public:
    InsuranceService(EVEServiceManager& mgr);

protected:
    ShipDB m_db;

    PyResult GetContractForShip(PyCallArgs& call, PyInt* shipID);
    PyResult GetInsurancePrice(PyCallArgs& call, PyInt* typeID);

    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters);
};

class InsuranceBound : public EVEBoundObject <InsuranceBound>
{
public:
    InsuranceBound(EVEServiceManager& mgr, PyRep* bindData, ShipDB* db);

protected:
    bool CanClientCall(Client* client) override;

    PyResult InsureShip(PyCallArgs& call, PyInt* shipID, PyFloat* amount, std::optional<PyInt*> isCorporation);
    PyResult UnInsureShip(PyCallArgs& call, PyInt* shipID);
    PyResult GetContracts(PyCallArgs& call, std::optional<PyRep*> isCorporation);
    PyResult GetInsurancePrice(PyCallArgs& call, PyInt* typeID);

protected:
    ShipDB* m_db;
    LSCService* m_lsc;
};

#endif


