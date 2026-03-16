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
    Author:     Zhur
*/

#ifndef __BOUNTYPROXYSERVICE_H_INCL__
#define __BOUNTYPROXYSERVICE_H_INCL__

#include "eve-server.h"

#include "services/Service.h"

class BountyProxyService : public Service<BountyProxyService> {
public:
    BountyProxyService();
    virtual ~BountyProxyService();

protected:
    PyResult GetBounties(PyCallArgs& call, PyRep* ownerIDs);
    PyResult GetMyBounties(PyCallArgs& call);
    PyResult GetTopPilotBounties(PyCallArgs& call, PyInt* page);
    PyResult GetTopCorpBounties(PyCallArgs& call, PyInt* page);
    PyResult GetTopAllianceBounties(PyCallArgs& call, PyInt* page);
    PyResult GetTopPilotBountyHunters(PyCallArgs& call, PyInt* page);
    PyResult GetTopCorporationBountyHunters(PyCallArgs& call, PyInt* page);
    PyResult GetTopAllianceBountyHunters(PyCallArgs& call, PyInt* page);
    PyResult AddToBounty(PyCallArgs& call, PyInt* ownerID, PyInt* amount);
    PyResult SearchCharBounties(PyCallArgs& call, PyInt* charID);
    PyResult SearchCorpBounties(PyCallArgs& call, PyInt* corpID);
    PyResult SearchAllianceBounties(PyCallArgs& call, PyInt* allianceID);
    PyResult GetBountiesAndKillRights(PyCallArgs& call, PyList* bountiesToFetch, PyList* killRightsToFetch);
    PyResult SellKillRight(PyCallArgs& call, PyInt* killRightID, PyInt* price, PyRep* restrictedTo);
    PyResult CancelSellKillRight(PyCallArgs& call, PyInt* killRightID, PyInt* toID);
    PyResult GMReimburseBounties(PyCallArgs& call);
    PyResult GMClearBountyCache(PyCallArgs& call);
};

#endif
