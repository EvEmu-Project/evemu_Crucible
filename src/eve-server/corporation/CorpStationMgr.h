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


#ifndef __CORPSTATIONMGR_SERVICE_H_INCL__
#define __CORPSTATIONMGR_SERVICE_H_INCL__

#include "corporation/CorporationDB.h"
#include "services/BoundService.h"

class PyRep;

class CorpStationMgr : public BindableService <CorpStationMgr>
{
public:
    CorpStationMgr(EVEServiceManager& mgr);

protected:
    CorporationDB m_db;

    PyResult GetStationServiceStates(PyCallArgs& call);
    PyResult GetImprovementStaticData(PyCallArgs& call);

    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters) override;
};

class CorpStationMgrIMBound : public EVEBoundObject <CorpStationMgrIMBound>
{
public:
    CorpStationMgrIMBound(EVEServiceManager& mgr, PyRep* bindData, CorporationDB& db, uint32 station_id);

protected:
    bool CanClientCall(Client* client) override;

    PyResult GetCorporateStationInfo(PyCallArgs& call);
    PyResult DoStandingCheckForStationService(PyCallArgs& call, PyInt* stationServiceID);
    PyResult GetPotentialHomeStations(PyCallArgs& call);
    PyResult SetHomeStation(PyCallArgs& call, PyInt* newHomeStationID);
    PyResult SetCloneTypeID(PyCallArgs& call, PyInt* cloneTypeID);
    PyResult GetQuoteForRentingAnOffice(PyCallArgs& call);
    PyResult RentOffice(PyCallArgs& call, PyInt* amount);
    PyResult CancelRentOfOffice(PyCallArgs& call);
    PyResult GetStationOffices(PyCallArgs& call);
    PyResult GetNumberOfUnrentedOffices(PyCallArgs& call);
    PyResult MoveCorpHQHere(PyCallArgs& call);
    //testing
    PyResult GetCorporateStationOffice(PyCallArgs& call);
    PyResult DoesPlayersCorpHaveJunkAtStation(PyCallArgs& call);
    PyResult GetQuoteForGettingCorpJunkBack(PyCallArgs& call);
    PyResult PayForReturnOfCorpJunk(PyCallArgs& call, PyFloat* cost);
    PyResult GetStationServiceIdentifiers(PyCallArgs& call);
    PyResult GetStationDetails(PyCallArgs& call, PyInt* stationID);
    PyResult GetStationServiceAccessRule(PyCallArgs& call, PyInt* stationID, PyInt* serviceID);
    PyResult GetStationManagementServiceCostModifiers(PyCallArgs& call, PyInt* stationID);
    PyResult GetRentableItems(PyCallArgs& call);
    PyResult GetOwnerIDsOfClonesAtStation(PyCallArgs& call, PyInt* corporationID);
    PyResult GetStationImprovements(PyCallArgs& call);

protected:
    StationItem* pStationItem;    //we do not own this

    CorporationDB& m_db;
    const uint32 m_stationID;
};

#endif

/*
 * RentingOfficeRequestDenied
 * RentingAnOfficeCostsMore
 * NoOfficeAtStation
 * NoOfficesAreAvailableForRenting
 * OfficeEstablishmentItemNotAtStation
 * OfficeRentalCostMustBePositive
 */