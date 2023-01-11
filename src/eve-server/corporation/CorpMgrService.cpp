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
    Rewrite:    Allan
*/

#include <string>
#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/CorpMgrService.h"

CorpMgrService::CorpMgrService() :
    Service("corpmgr")
{
    this->Add("GetPublicInfo", &CorpMgrService::GetPublicInfo);
    this->Add("GetCorporations", &CorpMgrService::GetCorporations);
    this->Add("GetAssetInventory", &CorpMgrService::GetAssetInventory);
    this->Add("GetCorporationStations", &CorpMgrService::GetCorporationStations);
    this->Add("GetCorporationIDForCharacter", &CorpMgrService::GetCorporationIDForCharacter);
    this->Add("GetAssetInventoryForLocation", &CorpMgrService::GetAssetInventoryForLocation);
    this->Add("SearchAssets", &CorpMgrService::SearchAssets);
    this->Add("AuditMember", &CorpMgrService::AuditMember);
}

/*
 * CORP__ERROR
 * CORP__WARNING
 * CORP__INFO
 * CORP__MESSAGE
 * CORP__TRACE
 * CORP__CALL
 * CORP__CALL_DUMP
 * CORP__RSP_DUMP
 * CORP__DB_ERROR
 * CORP__DB_WARNING
 * CORP__DB_INFO
 * CORP__DB_MESSAGE
 */

PyResult CorpMgrService::GetPublicInfo(PyCallArgs &call, PyInt* corporationID) {
    return m_db.GetCorpInfo(corporationID->value());
}

PyResult CorpMgrService::GetCorporations(PyCallArgs &call, PyInt* corporationID) {
    return m_db.GetCorporations(corporationID->value());
}

PyResult CorpMgrService::GetCorporationIDForCharacter(PyCallArgs &call, PyInt* characterID) {
    return new PyInt(m_db.GetCorpIDforChar(characterID->value()));
}

PyResult CorpMgrService::AuditMember(PyCallArgs &call, PyInt* memberID, PyLong* fromDate, PyLong* toDate, std::optional<PyInt*> rowsPerPage) {
    // logItemEventRows, crpRoleHistroyRows = sm.RemoteSvc('corpmgr').AuditMember(memberID, fromDate, toDate, rowsPerPage)

    _log(CORP__CALL, "CorpMgrService::Handle_AuditMember()");
    call.Dump(CORP__CALL_DUMP);

    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, m_db.GetItemEvents(call.client->GetCorporationID(), memberID->value(), fromDate->value(), toDate->value(), rowsPerPage.has_value() ? rowsPerPage.value()->value() : 0));
    tuple->SetItem(1, m_db.GetRoleHistroy(call.client->GetCorporationID(), memberID->value(), fromDate->value(), toDate->value(), rowsPerPage.has_value() ? rowsPerPage.value()->value() : 0));

    if (is_log_enabled(CORP__RSP_DUMP))
        tuple->Dump(CORP__RSP_DUMP, "    ");

    return tuple;
}

PyResult CorpMgrService::GetAssetInventory(PyCallArgs &call, PyInt* corporationID, PyString* which) {
    // rows = sm.RemoteSvc('corpmgr').GetAssetInventory(eve.session.corpid, which)
    // this is called from corp asset screen.  wants a return of locationIDs of stations where corp hangers have items
    _log(CORP__CALL, "CorpMgrService::Handle_GetAssetInventory()");
    call.Dump(CORP__CALL_DUMP);

    EVEItemFlags locFlag = flagNone;
    std::ostringstream flags;
    flags << "(";
    if (which->content().compare("offices") == 0) {
        locFlag = flagOffice;
        flags << flagHangar << "," << flagCorpHangar2 << "," << flagCorpHangar3 << "," << flagCorpHangar4 << "," << flagCorpHangar5;
        flags  << "," << flagCorpHangar6 << "," << flagCorpHangar7;
    } else if (which->content().compare("junk") == 0) {        // this is 'impounded' tab for all locations
        locFlag = flagImpounded;
        flags << flagImpounded;
    } else if (which->content().compare("property") == 0) {    // this is 'inSpace' tab...LSC, ALSC, POS, etc
        locFlag = flagProperty;
        flags << flagHangar << "," << flagCorpHangar2 << "," << flagCorpHangar3 << "," << flagCorpHangar4 << "," << flagCorpHangar5;
        flags  << "," << flagCorpHangar6 << "," << flagCorpHangar7;
    } else if (which->content().compare("deliveries") == 0) {
        locFlag = flagCorpMarket;
        flags << flagCorpMarket;
    } else {
        _log(CORP__ERROR, "CorpMgrService::Handle_GetAssetInventory: flag is %s", which->content().c_str());
        return nullptr;
    }

    flags << ")";

    PyRep* rsp = m_db.GetAssetInventory(corporationID->value(), locFlag, flags.str().c_str());
    if (is_log_enabled(CORP__RSP_DUMP))
        rsp->Dump(CORP__RSP_DUMP, "    ");

    //  returns a CRowSet
    return rsp;
}

PyResult CorpMgrService::GetAssetInventoryForLocation(PyCallArgs &call, PyInt* corporationID, PyInt* stationID, PyString* which) {
    //  items = sm.RemoteSvc('corpmgr').GetAssetInventoryForLocation(eve.session.corpid, stationID, which)
    _log(CORP__CALL, "CorpMgrService::Handle_GetAssetInventoryForLocation()");
    call.Dump(CORP__CALL_DUMP);

    EVEItemFlags locFlag = flagNone;
    std::ostringstream flags;
    flags << "(";
    if (which->content().compare("offices") == 0) {
        locFlag = flagOffice;
        flags << flagHangar << "," << flagCorpHangar2 << "," << flagCorpHangar3 << "," << flagCorpHangar4 << "," << flagCorpHangar5;
        flags  << "," << flagCorpHangar6 << "," << flagCorpHangar7;
    } else if (which->content().compare("junk") == 0) {
        locFlag = flagImpounded;
        flags << flagImpounded;
    } else if (which->content().compare("property") == 0) {    // this is 'inSpace' tab...LSC, POS, etc
        locFlag = flagProperty;
        flags << flagHangar << "," << flagCorpHangar2 << "," << flagCorpHangar3 << "," << flagCorpHangar4 << "," << flagCorpHangar5;
        flags  << "," << flagCorpHangar6 << "," << flagCorpHangar7;
    } else if (which->content().compare("deliveries") == 0) {
        locFlag = flagCorpMarket;
        flags << flagCorpMarket;
    } else {
        _log(CORP__ERROR, "CorpMgrService::Handle_GetAssetInventory: flag is %s", which->content().c_str());
        return nullptr;
    }

    flags << ")";
    //  returns a CRowSet
    //  this can be called on system/station/office that isnt loaded, so must hit db for info.
    return m_db.GetAssetInventoryForLocation(corporationID->value(), stationID->value(), flags.str().c_str());
}

// this is called for corp trade
PyResult CorpMgrService::GetCorporationStations(PyCallArgs &call) {
  /**           this is called from trademgr.py (and others)
        stations = sm.RemoteSvc('corpmgr').GetCorporationStations()
        for station in stations:
            if station.itemID in self.shell.GetStationIDs():
                continue
            stationListing.append([localization.GetByLabel('UI/PVPTrade/StationInSolarsystem', station=station.itemID, solarsystem=station.locationID), station.itemID, station.typeID])
*/

    _log(CORP__CALL, "CorpMgrService::Handle_GetCorporationStations()");
    call.Dump(CORP__CALL_DUMP);

    if (IsPlayerCorp(call.client->GetCorporationID()))
        return m_db.Fetch(call.client->GetCorporationID(), 0, 50);  // arbitrary values

    _log(CORP__WARNING, "CorpMgrService::Handle_GetCorporationStations() calling for NPC Corp");
    return nullptr;
}


PyResult CorpMgrService::SearchAssets(PyCallArgs &call, PyString* which, std::optional<PyInt*> itemCategoryID, std::optional<PyInt*> itemGroupID, std::optional<PyInt*> itemTypeID, std::optional<PyInt*> quantity) {
    //   rows = sm.RemoteSvc('corpmgr').SearchAssets(which, itemCategoryID, itemGroupID, itemTypeID, qty)
    // 'which' is a filter type or None
    _log(CORP__CALL, "CorpMgrService::Handle_SearchAssets()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}
