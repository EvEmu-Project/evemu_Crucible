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


#ifndef EVEMU_PLANET_PLANETORB_BOUND_H_
#define EVEMU_PLANET_PLANETORB_BOUND_H_

#include "services/BoundService.h"
#include "planet/PlanetDB.h"

class PlanetORBBound;

class PlanetORB : public BindableService <PlanetORB, PlanetORBBound> {
public:
    PlanetORB(EVEServiceManager& mgr);

    void BoundReleased (PlanetORBBound* bound) override;
protected:
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters) override;

private:
    std::map<uint32, PlanetORBBound*> m_instances;
};

class PlanetORBBound : public EVEBoundObject <PlanetORBBound> {
public:
    PlanetORBBound(EVEServiceManager& mgr, PlanetORB& parent, uint32 systemID);

    uint32 GetSystemID () { return this->m_systemID; }
protected:
    PyResult GetTaxRate(PyCallArgs& call, PyInt* itemID);
    PyResult GetSettingsInfo(PyCallArgs& call, PyInt* orbitalID);
    PyResult UpdateSettings(PyCallArgs& call, PyInt* orbitalID, PyInt* reinforceValue, PyObject* taxRateValues, PyFloat* standingValue, PyBool* allowAllianceValue, PyBool* allowStandingsValue);
    PyResult GMChangeSpaceObjectOwner(PyCallArgs& call, PyInt* itemID, PyInt* corpID);

private:
    uint32 m_systemID;
};

#endif  // EVEMU_PLANET_PLANETORB_BOUND_H_