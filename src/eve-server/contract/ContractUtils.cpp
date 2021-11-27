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
    Author: AlTahir
*/

#include <boost/algorithm/string/replace.hpp>
#include "eve-server.h"

#include "PyServiceCD.h"
#include "ContractUtils.h"

// Since these queries are used in multiple GET functions, we initialize them as constants and use at will.
const std::string getContractQueryBase = "SELECT contractId as contractID, contractType as type, issuerID, issuerCorpID, forCorp, isPrivate as availability, "
                               "assigneeID, acceptorID, dateIssued, dateExpired, dateAccepted, numDays, dateCompleted, startStationID, startSolarSystemID, "
                               "startRegionID, endStationID, endSolarSystemID, endRegionID, price, reward, collateral, title, description, status, "
                               "crateID, volume, issuerAllianceID, issuerWalletKey, acceptorWalletKey "
                               "FROM ctrContracts ";
const std::string getContractItemsQueryBase = "SELECT contractId as contractID, itemID, quantity, itemTypeID, inCrate, parentID, productivityLevel, materialLevel, isCopy as copy, "
                                    "licensedProductionRunsRemaining, damage, flagID "
                                    "FROM ctrItems "
                                    "WHERE contractId IN (%s)";
const std::string getContractItemsShortQueryBase = "SELECT itemID "
                                                   "FROM ctrItems "
                                                   "WHERE contractId IN (%s) AND inCrate = true";
const std::string getContractBidsQueryBase = "SELECT amount, bidderID "
                                   "FROM ctrBids "
                                   "WHERE contractId IN (%s) "
                                   "ORDER BY bidDateTime DESC";

/**
 * Gathers and packs contract data into a util.KeyVal PyObject.
 * @param contractId - contract ID to get
 * @return - util.KeyVal PyObject (or nullptr, if contract doesn't exist or there were errors during execution)
 */
PyResult ContractUtils::GetContractEntry(int contractId)
{
    std::string contractID = std::to_string(contractId);

    DBQueryResult contractRes;
    if (!sDatabase.RunQuery(contractRes, (getContractQueryBase + "WHERE contractId IN (%s)").c_str(), contractID.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", contractRes.error.c_str());
        return nullptr;
    }
    if (contractRes.GetRowCount() > 0) {
        DBResultRow contractRow;
        contractRes.GetRow(contractRow);

        // We only run queries for items and bids if we had something returned in contracts query - we don't need to waste time and resources on non-existent contracts.
        DBQueryResult itemsRes;
        DBQueryResult bidsRes;
        if (!sDatabase.RunQuery(itemsRes, getContractItemsQueryBase.c_str(), contractID.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", itemsRes.error.c_str());
            return nullptr;
        }
        if (!sDatabase.RunQuery(bidsRes, getContractBidsQueryBase.c_str(), contractID.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", bidsRes.error.c_str());
            return nullptr;
        }

        PyDict* response = new PyDict;
        response->SetItemString("contract", DBRowToPackedRow(contractRow));
        response->SetItemString("items", DBResultToCRowset(itemsRes));
        response->SetItemString("bids", DBResultToCRowset(bidsRes));

        return new PyObject( "util.KeyVal", response );
    } else {
        codelog(SERVICE__ERROR, "No contract with ID '%s' was found. Aborting", contractID.c_str());
        return nullptr;
    }
}

/**
 * Queries contracts and returns a PyList object, that contains data for each of them.
 * @param contractIds - Vector with all contractID's to search for
 * @return PyList with Contract KeyVal objects.
 */
PyList* ContractUtils::GetContractEntries(std::vector<int> contractIDList) {
    std::string contractIDs;
    if (!contractIDList.empty()) {
        for (auto contractID : contractIDList) {
            contractIDs.append(std::to_string(contractID) + ",");
        }
        contractIDs.pop_back(); // we do pop-back to remove trailing comma.
    } else {
        codelog(SERVICE__ERROR, "ContractUtils: contractIDList was empty. Aborting");
        return nullptr;
    }

    DBQueryResult contractRes;
    if (!sDatabase.RunQuery(contractRes, (getContractQueryBase + "WHERE contractId IN (%s)").c_str(), contractIDs.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", contractRes.error.c_str());
        return nullptr;
    }
    if (contractRes.GetRowCount() > 0) {
        // We only run queries for items and bids if we had something returned in contracts query - we don't need to waste time and resources on non-existent contracts.
        DBQueryResult itemsRes;
        DBQueryResult bidsRes;
        if (!sDatabase.RunQuery(itemsRes, getContractItemsQueryBase.c_str(), contractIDs.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", itemsRes.error.c_str());
            return nullptr;
        }
        if (!sDatabase.RunQuery(bidsRes, getContractBidsQueryBase.c_str(), contractIDs.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", bidsRes.error.c_str());
            return nullptr;
        }

        DBResultRow contractRow;
        /**
         * We use map as temp storage - we want to do a single iteration over each DBQueryResult.
         * Given that we'd like to initialize sub-contents of resulting PyList when we're done (we can't really
         * append rows to CRowSet inside PyDict - there's no CRowSet return), we:
         * - iterate over Items DBResultQuery and store values in temporary map (int <-> CRowSet)
         * - iterate over Bids DBResultQuery and store values in temporary map (int <-> CRowSet)
         * - We iterate over Contracts DBResultQuery, initialize PyDict for each, pull items and bids from previously
         *   saved maps and fill in the data.
         *
         * I think there might be a better way to do it, but that's a matter for further optimizations.
         */
        DBRowDescriptor* itemsHeader = new DBRowDescriptor(itemsRes);
        std::map<int, CRowSet*> itemsMap;
        DBResultRow itemRow;
        while (itemsRes.GetRow(itemRow)) {
            int contractID = itemRow.GetInt(0);

            auto pos = itemsMap.find(contractID);
            if (pos == itemsMap.end()) {
                CRowSet* rowset = new CRowSet(&itemsHeader);
                PyPackedRow* into = rowset->NewRow();
                FillItemData(&itemRow, into);

                itemsMap[contractID] = rowset;
            } else {
                CRowSet* rowset = pos->second;
                PyPackedRow* into = rowset->NewRow();
                FillItemData(&itemRow, into);
            }
        }

        DBRowDescriptor* bidsHeader = new DBRowDescriptor(bidsRes);
        std::map<int, CRowSet*> bidsMap;
        DBResultRow bidRow;
        while (bidsRes.GetRow(bidRow)) {
            int contractID = bidRow.GetInt(0);

            auto pos = bidsMap.find(contractID);
            if (pos == bidsMap.end()) {
                CRowSet* rowset = new CRowSet(&itemsHeader);
                PyPackedRow* into = rowset->NewRow();
                FillBidData(&bidRow, into);

                bidsMap[contractID] = rowset;
            } else {
                CRowSet* rowset = pos->second;
                PyPackedRow* into = rowset->NewRow();
                FillBidData(&bidRow, into);
            }
        }

        PyList* contractsList = new PyList;
        while (contractRes.GetRow(contractRow)) {
            int contractID = contractRow.GetInt(0);

            PyDict* contract = new PyDict;
            contract->SetItemString("contract", DBRowToPackedRow(contractRow));
            contract->SetItemString("items", itemsMap.find(contractID) == itemsMap.end() ? new CRowSet(&itemsHeader) : itemsMap.find(contractID)->second);
            contract->SetItemString("bids", bidsMap.find(contractID) == bidsMap.end() ? new CRowSet(&bidsHeader) : bidsMap.find(contractID)->second);

            contractsList->AddItem(new PyObject("util.KeyVal", contract));
        }

        return contractsList;
    } else {
        codelog(SERVICE__ERROR, "No contracts in range ('%s') was found. Aborting", contractIDs.c_str());
        return nullptr;
    }
}

/**
 * Queries and composes a response object for GetContractListForOwner call. Placed in utils class so that we can re-use query strings
 * @param call - Call instance
 * @return - Response obj
 */
PyResult ContractUtils::GetContractListForOwner(PyCallArgs& call) {
    /**
     * This call's tuple have 1 value, that might be either bool or None, depending on selected filter.
     * Since our XMLPKTGen don't have any adequate way to process that 3-way value, decoding packets is not an option.
     * So we're manually validating the value types here.
     */
    PyRep* ownerID = call.tuple->GetItem(0);
    PyRep* contractStatus = call.tuple->GetItem(1);
    PyRep* contractType = call.tuple->GetItem(2);
    PyRep* issuedToBy = call.tuple->GetItem(3);

    // OwnerID and contract status values are always present and must have a correct type.
    // First we validate if tuple had them in a first place
    if (ownerID && contractStatus) {
        // If they were - next we validate that their values are acceptable
        if (!ownerID->IsInt() && !contractStatus->IsInt()) {
            return nullptr;
        }
    } else {
        return nullptr;
    }

    std::string contracts_query = getContractQueryBase;
    std::string items_query = "SELECT contractId, itemTypeID, quantity, inCrate "
                              "FROM ctrItems "
                              "WHERE contractId IN (";
    std::vector<int> contractIDs;

    // First, we finish contracts query by adding filters
    if (issuedToBy->IsNone()) {
        contracts_query.append("WHERE (issuerID = {OWNER_ID} OR assigneeID = {OWNER_ID}) ");
    } else {
        bool issued = issuedToBy->AsBool()->value();
        if (issued) {
            contracts_query.append("WHERE assigneeID = {OWNER_ID} ");
        } else {
            contracts_query.append("WHERE issuerID = {OWNER_ID} ");
        }
    }
    if (contractType->IsNone()) {
        contracts_query.append("AND contractType IN (1,2,3) ");
    } else {
        if (contractType->IsInt()) {
            contracts_query.append("AND contractType = " + std::to_string(contractType->AsInt()->value()) + " ");
        } else {
            // Unacceptable case - we expect Int only
            return nullptr;
        }
    }
    contracts_query.append("AND status = " + std::to_string(contractStatus->AsInt()->value()));
    boost::replace_all(contracts_query, "{OWNER_ID}", std::to_string(ownerID->AsInt()->value()));

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, contracts_query.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObjectEx* contracts = DBResultToCRowset(res);
    for (auto contract : contracts->list()) {
        // To get items, we store contractID's in separate list
        contractIDs.push_back(contract->AsPackedRow()->GetField(0)->AsInt()->value());
    }

    PyDict* items = new PyDict;
    if (!contractIDs.empty()) {
        for (auto contractID : contractIDs) {
            items_query.append(std::to_string(contractID) + ",");
        }
        items_query.pop_back(); items_query.append(")");

        if (!sDatabase.RunQuery(res, items_query.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }

        // Given that we have items queried for every contract in the list, we sort them by contractID's first, then we put those in the dict.
        std::map<int, CRowSet*> itemsByContractID;
        DBResultRow row;
        while (res.GetRow(row)) {
            auto pos = itemsByContractID.find(row.GetInt(0));
            if(pos == itemsByContractID.end()) {
                DBRowDescriptor *header = new DBRowDescriptor(res);
                CRowSet *rowset = new CRowSet(&header);

                PyPackedRow* packedRow = rowset->NewRow();
                packedRow->SetField("contractId", new PyInt(row.GetInt(0)));
                packedRow->SetField("itemTypeID", new PyInt(row.GetInt(1)));
                packedRow->SetField("quantity", new PyInt(row.GetInt(2)));
                packedRow->SetField("inCrate", new PyBool(row.GetBool(3)));

                itemsByContractID[row.GetInt(0)] = rowset;
            } else {
                CRowSet* rowset = pos->second;

                PyPackedRow* packedRow = rowset->NewRow();
                packedRow->SetField("contractId", new PyInt(row.GetInt(0)));
                packedRow->SetField("itemTypeID", new PyInt(row.GetInt(1)));
                packedRow->SetField("quantity", new PyInt(row.GetInt(2)));
                packedRow->SetField("inCrate", new PyBool(row.GetBool(3)));
            }
        }
        if (!itemsByContractID.empty()) {
            for (auto entry : itemsByContractID) {
                items->SetItem(new PyInt(entry.first), entry.second);
            }
        }
    }

    PyDict* ret = new PyDict;
    ret->SetItemString("contracts", contracts);
    ret->SetItemString("items", items);

    return new PyObject("util.KeyVal", ret);
}

/**
 * Service function - populates PackedRow with values from DBResultRow for ctrItems
 * @param itemRow - DB ctrItems row
 * @param targetRow - target PyPackedRow
 */
void ContractUtils::FillItemData(DBResultRow *itemRow, PyPackedRow *targetRow) {
    targetRow->SetField("contractID", new PyInt(itemRow->GetInt(0)));
    targetRow->SetField("itemID", new PyInt(itemRow->GetInt(1)));
    targetRow->SetField("quantity", new PyInt(itemRow->GetInt(2)));
    targetRow->SetField("itemTypeID", new PyInt(itemRow->GetInt(3)));
    targetRow->SetField("inCrate", new PyBool(itemRow->GetBool(4)));
    targetRow->SetField("parentID", new PyInt(itemRow->GetInt(5)));
    targetRow->SetField("productivityLevel", new PyInt(itemRow->GetInt(6)));
    targetRow->SetField("materialLevel", new PyInt(itemRow->GetInt(7)));
    targetRow->SetField("copy", new PyBool(itemRow->GetBool(8)));
    targetRow->SetField("licensedProductionRunsRemaining", new PyInt(itemRow->GetInt(9)));
    targetRow->SetField("damage", new PyInt(itemRow->GetInt(10)));
    targetRow->SetField("flagID", new PyInt(itemRow->GetInt(11)));
}

/**
 * Service function - populates PackedRow with values from DBResultRow for ctrBids
 * @param itemRow - DB ctrBids row
 * @param targetRow - target PyPackedRow
 */
void ContractUtils::FillBidData(DBResultRow *bidRow, PyPackedRow *targetRow) {
    targetRow->SetField("contractId", new PyInt(bidRow->GetInt(0)));
    targetRow->SetField("amount", new PyInt(bidRow->GetInt(1)));
    targetRow->SetField("bidderID", new PyInt(bidRow->GetInt(2)));
    targetRow->SetField("bidDateTime", new PyLong(bidRow->GetInt64(3)));
}

/**
 * Queries the list of entityID's for a given contract. Primarily used to get the list of items to be returned to owner
 * upon contract's deletion.
 * @param contractId - Contract ID to search items in
 * @param into - Vector instance that will contain the entityID's
 */
void ContractUtils::GetContractItemIDs(int contractId, std::vector<int> *into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, getContractItemsShortQueryBase.c_str(), std::to_string(contractId).c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        into->push_back(row.GetInt(0));
    }
}

/**
 * Queries requested items for a contract and populates provided map with itemType and quantities
 * @param contractId
 * @param into
 */
void ContractUtils::GetRequestedItems(int contractId, std::map<int32, int32> *into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemTypeID, quantity FROM ctrItems WHERE contractID = %u AND inCrate = false", contractId))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        into->insert(std::pair<int, int> (row.GetInt(0), row.GetInt(1)));
    }
}

