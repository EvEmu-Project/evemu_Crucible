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

/**
 * Gathers and packs contract data into a util.KeyVal PyObject.
 * @param contractId - contract ID to get
 * @return - util.KeyVal PyObject (or nullptr, if contract doesn't exist or there were errors during execution_
 */
PyResult ContractUtils::GetContractEntry(int contractId)
{
    std::string contractID = std::to_string(contractId);
    std::string getContractQuery = "SELECT contractId as contractID, contractType as type, issuerID, issuerCorpID, forCorp, isPrivate as availability,\n"
                                   "       assigneeID, acceptorID, dateIssued, dateExpired, dateAccepted, numDays, dateCompleted, startStationID, startSolarSystemID,\n"
                                   "       startRegionID, endStationID, endSolarSystemID, endRegionID, price, reward, collateral, title, description, status,\n"
                                   "       crateID, volume, issuerAllianceID, issuerWalletKey, acceptorWalletKey "
                                   "FROM ctrContracts "
                                   "WHERE contractId = " + contractID;
    std::string getContractItemsQuery = "SELECT contractId as contractID, itemID, quantity, itemTypeID, inCrate, parentID, productivityLevel, materialLevel, isCopy as copy,\n"
                                        "       licensedProductionRunsRemaining, damage, flagID "
                                        "FROM ctrItems "
                                        "WHERE contractId = " + contractID;
    std::string getContractBidsQuery = "SELECT amount, bidderID "
                                       "FROM ctrBids "
                                       "WHERE contractId = " + contractID +
                                       " ORDER BY bidDateTime DESC";

    DBQueryResult contractRes;
    if (!sDatabase.RunQuery(contractRes, getContractQuery.c_str()))
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
        if (!sDatabase.RunQuery(itemsRes, getContractItemsQuery.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", itemsRes.error.c_str());
            return nullptr;
        }
        if (!sDatabase.RunQuery(bidsRes, getContractBidsQuery.c_str()))
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