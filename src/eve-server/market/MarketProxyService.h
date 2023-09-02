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

#ifndef __MARKETPROXY_SERVICE_H_INCL__
#define __MARKETPROXY_SERVICE_H_INCL__

#include "market/MarketDB.h"
#include "services/Service.h"
#include "services/ServiceManager.h"

#include "cache/ObjCacheService.h"

class MarketProxyService : public Service <MarketProxyService>
{
public:
    MarketProxyService(EVEServiceManager& mgr);

protected:
    MarketDB m_db;

    PyResult GetMarketGroups(PyCallArgs& call);
    PyResult StartupCheck(PyCallArgs& call);
    PyResult GetCharOrders(PyCallArgs& call);
    PyResult GetCorporationOrders(PyCallArgs& call);
    PyResult GetStationAsks(PyCallArgs& call);
    PyResult GetSystemAsks(PyCallArgs& call);
    PyResult GetRegionBest(PyCallArgs& call);
    PyResult GetOldPriceHistory(PyCallArgs& call, PyInt* typeID);
    PyResult GetNewPriceHistory(PyCallArgs& call, PyInt* typeID);
    PyResult CorpGetNewTransactions(PyCallArgs& call, PyRep* sellBuy, PyRep* typeID, PyRep* clientID, PyRep* quantity, PyRep* fromDate, PyRep* maxPrice, PyRep* minPrice, PyRep* accountKey, PyRep* memberID);
    PyResult CharGetNewTransactions(PyCallArgs& call, PyRep* sellBuy, PyRep* typeID, PyRep* clientID, PyRep* quantity, PyRep* fromDate, PyRep* maxPrice, PyRep* minPrice);
    PyResult GetOrders(PyCallArgs& call, PyInt* typeID);
    PyResult PlaceCharOrder(PyCallArgs& call, PyInt* stationID, PyInt* typeID, PyFloat* price, PyInt* quantity, PyInt* bid, PyInt* orderRange, std::optional <PyInt*> itemID, PyInt* minVolume, PyInt* duration, PyBool* useCorp, std::optional<PyRep*> located);
    PyResult PlaceCharOrder(PyCallArgs &call, PyInt* stationID, PyInt* typeID, PyFloat* price, PyInt* quantity, PyInt* bid, PyInt* orderRange, std::optional <PyInt*> itemID, PyInt* minVolume, PyInt* duration, PyInt* useCorp, std::optional<PyRep*> located);
    PyResult ModifyCharOrder(PyCallArgs& call, PyInt* orderID, PyFloat* newPrice, PyInt* bid, PyInt* stationID, PyInt* solarSystemID, PyFloat* price, PyInt* range, PyInt* volRemaining, PyLong* issueDate);
    PyResult CancelCharOrder(PyCallArgs& call, PyInt* orderID, PyInt* regionID);

private:
    EVEServiceManager& m_manager;
    ObjCacheService* m_cache;
};

#endif
