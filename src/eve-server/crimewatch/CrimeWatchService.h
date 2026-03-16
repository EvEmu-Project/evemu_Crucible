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
    Author: Zhur
*/

#ifndef __CRIMEWATCH_SERVICE_H_INCL__
#define __CRIMEWATCH_SERVICE_H_INCL__

#include "services/BoundService.h"
#include "services/ServiceManager.h"
#include "packets/Crimewatch.h"

class CrimewatchBound;

class CrimewatchService : public BindableService<CrimewatchService, CrimewatchBound>
{
public:
    CrimewatchService(EVEServiceManager& mgr);

    void BoundReleased(CrimewatchBound* bound) override;

protected:
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters);

private:
    friend class CrimewatchBound;
};

class CrimewatchBound : public EVEBoundObject<CrimewatchBound>
{
public:
    CrimewatchBound(EVEServiceManager& mgr, CrimewatchService& parent, uint32 locationID, uint32 groupID);
    
    PyResult GetClientStates(PyCallArgs& call);
    PyResult SetSafetyLevel(PyCallArgs& call, PyInt* safetyLevel);
    PyResult GetMySecurityStatus(PyCallArgs& call);
    PyResult GetCharacterSecurityStatus(PyCallArgs& call, PyInt* charID);
    PyResult StartDuelChallenge(PyCallArgs& call, PyInt* charID);
    PyResult RespondToDuelChallenge(PyCallArgs& call, PyInt* fromCharID, PyInt* expiryTime, PyBool* accept);
    PyResult GetMyEngagements(PyCallArgs& call);
    
private:
    uint32 m_locationID;
    uint32 m_groupID;
};

#endif
