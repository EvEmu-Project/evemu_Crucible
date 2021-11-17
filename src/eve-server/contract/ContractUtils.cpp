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

#include "eve-server.h"

#include "PyServiceCD.h"
#include "ContractUtils.h"

// Since these queries are used in all GET functions, we initialize them as constants and use at will.
const std::string getContractQueryBase = "SELECT contractId as contractID, contractType as type, issuerID, issuerCorpID, forCorp, isPrivate as availability, "
                               "assigneeID, acceptorID, dateIssued, dateExpired, dateAccepted, numDays, dateCompleted, startStationID, startSolarSystemID, "
                               "startRegionID, endStationID, endSolarSystemID, endRegionID, price, reward, collateral, title, description, status, "
                               "crateID, volume, issuerAllianceID, issuerWalletKey, acceptorWalletKey "
                               "FROM ctrContracts "
                               "WHERE contractId IN (%s)";
const std::string getContractItemsQueryBase = "SELECT contractId as contractID, itemID, quantity, itemTypeID, inCrate, parentID, productivityLevel, materialLevel, isCopy as copy, "
                                    "licensedProductionRunsRemaining, damage, flagID "
                                    "FROM ctrItems "
                                    "WHERE contractId IN (%s)";
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
    if (!sDatabase.RunQuery(contractRes, getContractQueryBase.c_str(), contractID.c_str()))
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
 * @param contractIds - String with comma-separated list of ID's to gather (143,144,145).
 *                      NOTE: We use string here, since we already iterate over DB results prior to this function - we don't need another iterator here.
 * @return PyList with Contract KeyVal objects.
 */
PyList* ContractUtils::GetContractEntries(const std::string& contractIDs) {
    DBQueryResult contractRes;
    if (!sDatabase.RunQuery(contractRes, getContractQueryBase.c_str(), contractIDs.c_str()))
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
