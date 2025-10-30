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
    Author:         Captnoord
    Rewrite:        Allan
    Implementation: AlTahir
*/

#include <boost/algorithm/string/replace.hpp>
#include <algorithm>    // Added to prevent std::find from freaking out
#include "eve-server.h"


#include "contract/ContractProxy.h"
#include "station/Station.h"
#include "inventory/Inventory.h"
#include "system/SolarSystem.h"

#include "contract/ContractUtils.h"
#include "account/AccountService.h"

ContractProxy::ContractProxy () :
    Service("contractProxy")
{
    this->Add("GetContract", &ContractProxy::GetContract);
    this->Add("CreateContract", static_cast <PyResult(ContractProxy::*)(PyCallArgs &,PyInt*, PyBool*, std::optional <PyNone*>, PyInt*, PyInt*, PyInt*, std::optional<PyNone*>, PyInt*, PyInt*, PyInt*, PyString*, PyString*)> (&ContractProxy::CreateContract));
    this->Add("CreateContract", static_cast <PyResult(ContractProxy::*)(PyCallArgs &,PyInt*, PyBool*, std::optional <PyInt*>, PyInt*, PyInt*, PyInt*, std::optional<PyNone*>, PyInt*, PyInt*, PyInt*, PyString*, PyString*)> (&ContractProxy::CreateContract));
    this->Add("CreateContract", static_cast <PyResult(ContractProxy::*)(PyCallArgs &,PyInt*, PyBool*, std::optional <PyInt*>, PyInt*, PyInt*, PyInt*, std::optional<PyNone*>, PyInt*, PyInt*, PyInt*, PyWString*, PyString*)> (&ContractProxy::CreateContract));
    this->Add("CreateContract", static_cast <PyResult(ContractProxy::*)(PyCallArgs &,PyInt*, PyBool*, std::optional <PyNone*>, PyInt*, PyInt*, PyInt*, std::optional<PyInt*>, PyInt*, PyInt*, PyInt*, PyString*, PyString*)> (&ContractProxy::CreateContract));
    this->Add("CreateContract", static_cast <PyResult(ContractProxy::*)(PyCallArgs &,PyInt*, PyBool*, std::optional <PyNone*>, PyInt*, PyInt*, PyInt*, std::optional <PyNone*>, PyInt*, PyInt*, PyInt*, PyWString*,PyString*)> (&ContractProxy::CreateContract));
    this->Add("CreateContract", static_cast <PyResult(ContractProxy::*)(PyCallArgs&, PyInt*, PyInt*, std::optional <PyInt*>, PyInt*, PyInt*, PyInt*, std::optional<PyInt*>, PyInt*, PyInt*, PyInt*, PyWString*, PyWString*)> (&ContractProxy::CreateContract));
    this->Add("DeleteContract", &ContractProxy::DeleteContract);
    this->Add("AcceptContract", &ContractProxy::AcceptContract);
    this->Add("CompleteContract", &ContractProxy::CompleteContract);
    this->Add("GetLoginInfo", &ContractProxy::GetLoginInfo);
    this->Add("SearchContracts", &ContractProxy::SearchContracts);
    this->Add("NumOutstandingContracts", &ContractProxy::NumOutstandingContracts);
    this->Add("CollectMyPageInfo", &ContractProxy::CollectMyPageInfo);
    this->Add("GetItemsInStation", &ContractProxy::GetItemsInStation);
    this->Add("GetContractListForOwner", &ContractProxy::GetContractListForOwner);
    this->Add("GetMyExpiredContractList", &ContractProxy::GetMyExpiredContractList);
    /*
     *
            ret = self.contractSvc.CompleteContract(contractID, const.conStatusFinished)
            return self.contractSvc.CompleteContract(contractID, const.conStatusRejected)
            return self.contractSvc.PlaceBid(contractID, bid, forCorp)
            return self.contractSvc.FinishAuction(contractID, isIssuer)
            ret = self.contractSvc.SplitStack(stationID, itemID, qty, forCorp, flag)
            return self.contractSvc.GetItemsInContainer(stationID, containerID, forCorp, flag)
            return self.contractSvc.GetItemsInStation(stationID, forCorp)
            return self.contractSvc.DeleteNotification(contractID, forCorp)
            info = self.contractSvc.GetCourierContractFromItemID(itemID)
     */
}

PyResult ContractProxy::SearchContracts(PyCallArgs &call) {
    // We will not proceed, if contractType is not specified
    if (!call.byname.find("contractType")->second->IsNone()) {
        int contractType = call.byname.find("contractType")->second->AsInt()->value();

        /**
         * We're using sort of query constructor here - if request have certain value specified, we add it as another AND block.
         * For now, we will only filter by contract type, item type, item category, min/max price, min/max reward, location/end location, issuer and availability
         */
        std::string query = "SELECT cC.contractId FROM ctrContracts cC "
                            "JOIN ctrItems cI on cC.contractId = cI.contractId "
                            "JOIN entity e on cI.itemID = e.itemID "
                            "JOIN invTypes iT on iT.typeID = e.typeID "
                            "JOIN invGroups iG on iG.groupID = iT.groupID "
                            "JOIN invCategories iC on iG.categoryID = iC.categoryID "
                            "WHERE cC.contractType IN " + std::string(contractType == 10 ? "(1,2)" : "(" + std::to_string(contractType) + ")");
                                                         // Type 10 is "All" and "Exclude WTB", for some reason. We'll assume it's "All", lol

        if (!call.byname.find("itemTypes")->second->IsNone()) {
            PyList* itemTypes = call.byname.find("itemTypes")->second->AsObjectEx()->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();
            std::string types;
            for (auto index = 0; index < itemTypes->size(); index++) {
                types.append(std::to_string(itemTypes->GetItem(index)->AsInt()->value()));
                if (index != itemTypes->size() - 1) {
                    types.append(",");
                }
            }

            if (!types.empty()) {
                query.append(" AND e.typeID IN (" + types + ")");
            }
        }

        if (!call.byname.find("itemGroupID")->second->IsNone()) {
            query.append(" AND iG.groupID = " + std::to_string(call.byname.find("itemGroupID")->second->AsInt()->value()));
        }
        if (!call.byname.find("itemCategoryID")->second->IsNone()) {
            query.append(" AND iC.categoryID = " + std::to_string(call.byname.find("itemCategoryID")->second->AsInt()->value()));
        }
        if (!call.byname.find("minPrice")->second->IsNone()) {
            query.append(" AND cC.price >= " + std::to_string(call.byname.find("minPrice")->second->AsInt()->value()));
        }
        if (!call.byname.find("maxPrice")->second->IsNone()) {
            query.append(" AND cC.price <= " + std::to_string(call.byname.find("maxPrice")->second->AsInt()->value()));
        }
        if (!call.byname.find("minReward")->second->IsNone()) {
            query.append(" AND cC.reward >= " + std::to_string(call.byname.find("minReward")->second->AsInt()->value()));
        }
        if (!call.byname.find("maxReward")->second->IsNone()) {
            query.append(" AND cC.reward <= " + std::to_string(call.byname.find("maxReward")->second->AsInt()->value()));
        }
        if (!call.byname.find("availability")->second->IsNone()) {
            int availability = call.byname.find("availability")->second->AsInt()->value();
            if (availability == 0) {
                // Public contracts
                query.append(" AND cC.isPrivate = 0");
            } else if (availability == 1) {
                // Private contracts, assigned to character
                query.append(" AND cC.isPrivate = 1 AND cC.assigneeId = " + std::to_string(call.client->GetCharacterID()));
            } else if(availability == 2) {
                // Private contracts, assigned to corp
                query.append(" AND cC.isPrivate = 1 AND cC.assigneeId = " + std::to_string(call.client->GetCorporationID()));
            }
        }
        // According to what i had during testing, locationID can only be system, constellation or region. Given that we only store system and region ID, we use OR clause for these
        if (!call.byname.find("locationID")->second->IsNone()) {
            int locationId = call.byname.find("locationID")->second->AsInt()->value();
            if (IsSolarSystemID(locationId)) {
                // Solar system range
                query.append(" AND cC.startSolarSystemID = " + std::to_string(locationId));
            } else if (IsRegionID(locationId)) {
                // Region range
                query.append(" AND cC.startRegionID = " + std::to_string(locationId));
            }
        }
        // Same applies to endLocationID - it uses the same search::QuickQuery() call to get it
        if (!call.byname.find("endLocationID")->second->IsNone()) {
            int locationId = call.byname.find("endLocationID")->second->AsInt()->value();
            if (IsSolarSystemID(locationId)) {
                // Solar system range
                query.append(" AND cC.endSolarSystemID = " + std::to_string(locationId));
            } else if (IsRegionID(locationId)) {
                // Region range
                query.append(" AND cC.endRegionID = " + std::to_string(locationId));
            }
        }
        // Once again, issuer can be either a character or a corporation. We use separate filters depending on value
        if (!call.byname.find("issuerID")->second->IsNone()) {
            int issuerId = call.byname.find("issuerID")->second->AsInt()->value();
            if (IsCorp(issuerId)) {
                // Corporation case
                query.append(" AND cC.issuerCorpID = " + std::to_string(issuerId) + " AND cC.forCorp = true");
            } else {
                query.append(" AND cC.issuerID = " + std::to_string(issuerId) + " AND cC.forCorp = false");
            }
        }
        query.append(" AND cC.status = 0");
        /**
         * Once query is constructed, we execute it and collect contractID's. Since we can have duplicate values, we first
         * collect it to std::vector, and then we pass it to GetContractEntries function
         */

        DBQueryResult contractRes;
        if (!sDatabase.RunQuery(contractRes, query.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", contractRes.error.c_str());
            return nullptr;
        }
        std::vector<int> contractIDs;
        DBResultRow contractRow;
        while (contractRes.GetRow(contractRow)) {
            int contractId = contractRow.GetInt(0);

            // To make sure we don't add duplicates, we check whether we already have said contractID in vector already.
            if (std::find(contractIDs.begin(), contractIDs.end(), contractId) == contractIDs.end()) {
                contractIDs.push_back(contractId);
            }
        }

        PyDict* response = new PyDict;
        PyList* contracts = ContractUtils::GetContractEntries(contractIDs);
        response->SetItemString("contracts", contracts ? contracts : new PyList);
        response->SetItemString("numFound", contracts ? new PyInt(contracts->size()) : new PyInt(0));
        response->SetItemString("searchTime", new PyInt(153));  // Since search time is of no relevance to the client, we simply hard-code it
        response->SetItemString("maxResults", new PyInt(1000)); // Same here - we do not limit the list of contracts queried, so we leave this value hard-coded

        return new PyObject("util.KeyVal", response);
    } else {
        codelog(SERVICE__ERROR, "%s: ContractType was not specified. Aborting search", GetName().c_str());
        return nullptr;
    }
}

PyResult ContractProxy::CreateContract(PyCallArgs &call,
    PyInt* contractType, PyBool* isPrivate, std::optional <PyNone*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID,
    PyInt* price, PyInt* reward, PyInt* collateral, PyString* title, PyString* description) {
    return CreateContract(call, contractType, new PyInt(isPrivate->value()), std::nullopt, expireTime, duration, startStationID, std::nullopt, price, reward, collateral, new PyWString(title->content()), new PyWString(description->content()));
}

PyResult ContractProxy::CreateContract(PyCallArgs &call,
    PyInt* contractType, PyBool* isPrivate, std::optional <PyInt*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID,
    PyInt* price, PyInt* reward, PyInt* collateral, PyString* title, PyString* description) {
    return CreateContract(call, contractType, new PyInt(isPrivate->value()), assigneeID, expireTime, duration, startStationID, std::nullopt, price, reward, collateral, new PyWString(title->content()), new PyWString(description->content()));
}

PyResult ContractProxy::CreateContract(PyCallArgs &call,
    PyInt* contractType, PyBool* isPrivate, std::optional <PyInt*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID,
    PyInt* price, PyInt* reward, PyInt* collateral, PyWString* title, PyString* description) {
    return CreateContract(call, contractType, new PyInt(isPrivate->value()), assigneeID, expireTime, duration, startStationID, std::nullopt, price, reward, collateral, title, new PyWString(description->content()));
}

PyResult ContractProxy::CreateContract(PyCallArgs &call,
    PyInt* contractType, PyBool* isPrivate, std::optional <PyNone*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyInt*> endStationID,
    PyInt* price, PyInt* reward, PyInt* collateral, PyString* title, PyString* description) {
    return CreateContract(call, contractType, new PyInt(isPrivate->value()), std::nullopt, expireTime, duration, startStationID, endStationID, price, reward, collateral, new PyWString(title->content()), new PyWString(description->content()));
}

PyResult ContractProxy::CreateContract(PyCallArgs &call,
    PyInt* contractType, PyBool* isPrivate, std::optional <PyNone*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID,
    PyInt* price, PyInt* reward, PyInt* collateral, PyWString* title, PyString* description) {
    return CreateContract(call, contractType, new PyInt(isPrivate->value()), std::nullopt, expireTime, duration, startStationID, std::nullopt, price, reward, collateral, title, new PyWString(description->content()));
}


PyResult ContractProxy::CreateContract(PyCallArgs &call, 
    PyInt* contractType, PyInt* isPrivate, std::optional <PyInt*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyInt*> endStationID,
    PyInt* price, PyInt* reward, PyInt* collateral, PyWString* title, PyWString* description) {
    int startStationDivision, startSystemId, startRegionId, endSystemId, endRegionId;
    bool forCorp;

    /**
     * Since named args (byname) aren't included in packet, we process them separately.
     */
    if (call.byname.find("flag")->second->IsInt()) {
        startStationDivision = call.byname.find("flag")->second->AsInt()->value();
    } else {
        codelog(SERVICE__ERROR, "startStationDivision value is of invalid type");
        return nullptr;
    }
    if (call.byname.find("forCorp")->second->IsBool()) {
        forCorp = call.byname.find("forCorp")->second->AsBool()->value();
    } else {
        codelog(SERVICE__ERROR, "forCorp value is of invalid type");
        return nullptr;
    }
    if (sDataMgr.IsStation(startStationID->value())) {
        startSystemId = sDataMgr.GetStationSystem(startStationID->value());
        startRegionId = sDataMgr.GetStationRegion(startStationID->value());
    } else {
        codelog(SERVICE__ERROR, "Specified start Station ID has no Station counterparts in DB");
        return nullptr;
    }
    if (endStationID.has_value()) {
        if (sDataMgr.IsStation(endStationID.value()->value())) {
            endSystemId = sDataMgr.GetStationSystem(endStationID.value()->value());
            endRegionId = sDataMgr.GetStationRegion(endStationID.value()->value());
        } else {
            codelog(SERVICE__ERROR, "Specified end Station ID has no Station counterparts in DB");
            return nullptr;
        }
    } else {
        endSystemId = startSystemId;
        endRegionId = 0;
    }

    // Courier-specific step - if we have a reward, we'd want to take it in advance and store it "in heap" to block no-funds scam
    if (contractType->value() == 3 && reward->value() > 0) {
        if (call.client->GetBalance() >= reward->value()) {
            call.client->AddBalance(-reward->value());
        } else {
            call.client->SendNotifyMsg("You do not have enough ISK to pay the reward");
            return nullptr;
        }
    }

    /**
     * Then, we go for actual entries creation.
     * Contract entry
     */
    uint32 contractId = 0;
    DBerror err;
    if (!sDatabase.RunQueryLID(err, contractId,
        "INSERT INTO ctrContracts "
        "(contractType, issuerID, issuerCorpID, forCorp, isPrivate, assigneeID, "
        "dateIssued, dateExpired, expireTimeInMinutes, duration, numDays, "
        "startStationID, startSolarSystemID, startRegionID, endStationID, endSolarSystemID, endRegionID, "
        "price, reward, collateral, title, description, issuerAllianceID, startStationDivision) "
        "VALUES "
        "(%u, %u, %u, %u, %u, %u, "
        "%lli, %lli, %u, %u, %u, "
        "%u, %u, %u, %u, %u, %u,"
        "%u, %u, %u, '%s', '%s', %u, %u)",
        contractType->value(), call.client->GetCharacterID(), call.client->GetCorporationID(), forCorp, isPrivate->value()?1:0, assigneeID.has_value() ? assigneeID.value()->value() : 0,
        int64(GetFileTimeNow()), int64(GetRelativeFileTime(0, 0, expireTime->value())), expireTime->value(), duration->value(), expireTime->value() / 1440,
        startStationID->value(), startSystemId, startRegionId, endStationID.has_value() ? endStationID.value()->value() : 0, endSystemId, endRegionId,
        price->value(), reward->value(), collateral->value(), title->content().c_str(), description->content().c_str(), call.client->GetAllianceID(), startStationDivision))
    {
        codelog(DATABASE__ERROR, "Failed to insert new entity: %s", err.c_str());
        return nullptr;
    }

    /**
     * Then, we insert the items.
     * First off, we gather the list of attributes for these items, using select query.
     * To save resources and reduce amount of DB hits, we compose the query by adding ID's first, then we execute it separately.
     */
    std::string itemsToInsert;
    float totalVolume = 0.00;
    if (call.byname.find("itemList")->second->IsList()) {
        PyList *tradedItems = call.byname.find("itemList")->second->AsList();
        if (!tradedItems->empty()) {
            //TODO: We need to account for items that can be packed in a container/ship/container inside the ship
            std::string query = "SELECT entity.itemID, entity.ownerID, entity.typeID, entity.quantity, entity.locationID, iB.pLevel, "
                                "       iB.mLevel, iB.copy, iB.runs, ea.valueInt as damage, entity.flag "
                                "FROM entity "
                                "LEFT JOIN invBlueprints iB on entity.itemID = iB.itemID "
                                "LEFT JOIN entity_attributes ea on entity.itemID = ea.itemID and ea.attributeID = 3 "
                                "WHERE entity.itemID IN (%s)";
            std::string queryIds;
            std::map<int, int> expectedQuantities;              // Key is itemID, value is quantity. We use map to save time on list iteration
            for (int index = 0; index < tradedItems->size(); index++) {
                PyList *tradedItem = tradedItems->GetItem(index)->AsList();
                int itemID = tradedItem->GetItem(0)->AsInt()->value();
                int quantity = tradedItem->GetItem(1)->AsInt()->value();

                queryIds.append(std::to_string(itemID));
                expectedQuantities[itemID] = quantity;

                // if it's not the last item - add a trailing comma
                if (index != tradedItems->size() - 1) {
                    queryIds.append(", ");
                }
            }

            DBQueryResult res;
            if (!sDatabase.RunQuery(res,query.c_str(), queryIds.c_str()))
            {
                codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
                return nullptr;
            }

            /**
             * I HATE the fact that i have to do the same bloody loop again, but we didn't have DB values prior to that.
             * We need to validate traded items exist, have correct owner and quantities. Any item that fails the check is excluded from the list.
             */
             DBResultRow row;
             int expectedOwnerID = call.client->GetCharacterID();
             // We work directly with DBQueryResult since resulting CRowSet does not fit ctrItems format - thus, it would be needless processing.
             while(res.GetRow(row)) {
                 int itemID = row.GetInt(0);
                 int ownerID = row.GetInt(1);
                 int quantity = row.GetInt(3);
                 int parentID = row.GetInt(4) >= 14000000 ? row.GetInt(4) : 0;
                 int pLevel = row.IsNull(5) ? 0 : row.GetInt(5);
                 int mLevel = row.IsNull(6) ? 0 : row.GetInt(6);
                 int isCopy = row.IsNull(7) ? 0 : (row.GetBool(7) ? 1 : 0);
                 int runs = row.IsNull(8) ? 0 : row.GetInt(8);
                 int damage = row.IsNull(9) ? 0 : row.GetInt(9);
                 int flag = row.IsNull(10) ? 0 : row.GetInt(10);

                 if (ownerID == expectedOwnerID && quantity == expectedQuantities.find(itemID)->second) {
                     itemsToInsert.append("(" + std::to_string(contractId) + ", " +
                        std::to_string(itemID) + ", " +
                        std::to_string(quantity) + ", " +
                        row.GetText(2) + ", " +
                        "1, " +
                        std::to_string(parentID) + ", " +
                        std::to_string(pLevel) + ", " +
                        std::to_string(mLevel) + ", " +
                        std::to_string(isCopy) + "," +
                        std::to_string(runs) + ", " +
                        std::to_string(damage) + ", " +
                        std::to_string(flag)+ "),");

                     // We only calculate volume for courier-type contracts - the other types don't use this value.
                     if (contractType->value() == 3) {
                         totalVolume += sItemFactory.GetStationRef(startStationID->value())->GetMyInventory()->GetByID(itemID)->GetAttribute(161).get_float() * quantity;
                     }

                     sItemFactory.GetItemRef(itemID)->ChangeOwner(1, true);
                 }
             }
        }
    }


    if (call.byname.find("requestItemTypeList")->second->IsList()) {
        PyList *requestedItems = call.byname.find("requestItemTypeList")->second->AsList();
        if (!requestedItems->empty()) {
            for (int index = 0; index < requestedItems->size(); index++) {
                PyList *requestedItem = requestedItems->GetItem(index)->AsList();
                itemsToInsert.append("(" + std::to_string(contractId) + ", " +
                                     "0, " +
                                     std::to_string(requestedItem->GetItem(1)->AsInt()->value()) + ", " +
                                     std::to_string(requestedItem->GetItem(0)->AsInt()->value()) + ", " +
                                     "0, 0, 0, 0, 0, 0, 0, 0),");
            }
        }
    }

    if(!itemsToInsert.empty()) {
        itemsToInsert.pop_back();
        std::string query = "INSERT INTO ctrItems (contractId, itemID, quantity, itemTypeID, inCrate, parentID, "
                            "productivityLevel, materialLevel, isCopy, licensedProductionRunsRemaining, damage, flagID) "
                            "VALUES " + itemsToInsert;
        uint32 last_insert;
        if (!sDatabase.RunQueryLID(err, last_insert, query.c_str()))
        {
            codelog(DATABASE__ERROR, "Failed to insert new entity: %s", err.c_str());
            return nullptr;
        }

        // We only insert volume for courier-type contracts - the other types don't use this value.
        if (contractType->value() == 3) {
            DBerror err;
            if (!sDatabase.RunQuery(err,
                                    "UPDATE ctrContracts SET volume = %f WHERE contractId = %u", totalVolume, contractId))
            {
                codelog(DATABASE__ERROR, "Failed to update contract volume: %s", err.c_str());
            }
        }

    } else {
        codelog(SERVICE__ERROR, "No traded or requested items was specified. Aborting");
        return nullptr;
    }

    return new PyInt((int) contractId);
}

PyResult ContractProxy::DeleteContract(PyCallArgs &call, PyInt* contractID) {
    sLog.White( "ContractProxy::Handle_DeleteContract()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    // In order to return items back to the owner, we need a full list of entityID's to return. We gather them using utils function
    std::vector<int> entityIds;
    ContractUtils::GetContractItemIDs(contractID->value(), &entityIds);
    for (auto entityID : entityIds) {
        // We have to put in couple layers of checks here. First one will cut requested item entries out of the equation
        if (entityID != 0) {
            // We have to check if we actually got the item ref - in case of invalid entity ID specified.
            InventoryItemRef ref = sItemFactory.GetItemRef(entityID);
            if (ref) {
                ref->ChangeOwner(call.client->GetCharacterID(), true);
            } else {
                continue;
            }
        }
    }

    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE ctrContracts SET status = 8 WHERE contractId = %u", contractID->value()))
    {
        codelog(DATABASE__ERROR, "Failed to update contract volume: %s", err.c_str());
    }

    return new PyBool(true);
}

PyResult ContractProxy::GetContract(PyCallArgs &call, PyInt* contractID) {
    return ContractUtils::GetContractEntry(contractID->value());
}

PyResult ContractProxy::AcceptContract(PyCallArgs &call, PyInt* contractID) {
    // For the time being - we ignore the second value in tuple (forCorp), since it's not yet functional.

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT contractType, status, price, reward, collateral, volume, startStationID, issuerID, forCorp, startSolarSystemID, endSolarSystemID FROM ctrContracts WHERE contractId = %u", contractID->value()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    if (res.GetRowCount() == 0) {
        return nullptr;
    }
    DBResultRow row;
    res.GetRow(row);
    int contractType = row.GetInt(0);
    int status = row.GetInt(1);
    int price = row.GetInt(2);
    int reward = row.GetInt(3);
    int collateral = row.GetInt(4);
    float volume = row.GetFloat(5);
    int startStationID = row.GetInt(6);
    int issuerID = row.GetInt(7);
    bool forCorp = row.GetBool(8);
    int startSolarSystemID = row.GetInt(9);
    int endSolarSystemID = row.GetInt(10);

    if (status == 0) {
        // We can only accept outstanding contracts. If it's not - we ignore the call.
        int64 timestamp = int64(GetFileTimeNow());
        switch (contractType) {
            case 1: {
                // Item Exchange
                // We start off by gathering traded and requested items
                std::vector<int> tradedItems;
                std::map<int, int> requestedItems;
                ContractUtils::GetContractItemIDs(contractID->value(), &tradedItems);
                ContractUtils::GetRequestedItems(contractID->value(), &requestedItems);

                // Next, we perform checks to make sure we fit all contract requirements. I won't do it by nesting if loops - i'll just use trigger booleans;
                bool iskRequirementMet(true), rewardRequirementMet(true), requestedItemsRequirementsMet(true);
                if (price > 0) {
                    if (call.client->GetBalance() < price) {
                        iskRequirementMet = false;
                    }
                }
                if (reward > 0) {
                    if (sItemFactory.GetCharacterRef(issuerID)->balance(Account::CreditType::ISK) < reward) {
                        rewardRequirementMet = false;
                    }
                }
                if (!requestedItems.empty()) {
                    for (const auto& entry : requestedItems) {
                        // Since we don't have a direct way to find items by typeID, we go over all items on current station and checking whether we have any of correct type and quantity
                        // TODO: Implement forCorp loop when corp contracts are unblocked.
                        if (sItemFactory.GetStationRef(startStationID)->GetMyInventory()->ContainsTypeStackQtyByFlag(entry.first, EVEItemFlags::flagHangar, entry.second) == 0) {
                            requestedItemsRequirementsMet = false;
                        }
                    }
                }

                // Then, we go for acceptance
                if (iskRequirementMet && rewardRequirementMet && requestedItemsRequirementsMet) {
                    // If we have a reward value - then contract's WTB
                    if (reward > 0) {
                        AccountService::TransferFunds(issuerID, call.client->GetCharacterID(), reward, "Payment for accepted contract", Journal::EntryType::ContractReward, contractID->value());
                    }

                    // If we have price value - then contract's WTS
                    if (price > 0) {
                        AccountService::TransferFunds(call.client->GetCharacterID(), issuerID, price, "Payment for accepted contract", Journal::EntryType::ContractPrice, contractID->value());
                    }

                    // Then, we go for requested items
                    if (!requestedItems.empty()) {
                        for (auto entry : requestedItems) {
                            int entityID = sItemFactory.GetStationRef(startStationID)->GetMyInventory()->ContainsTypeStackQtyByFlag(entry.first, flagHangar, entry.second);
                            if (sItemFactory.GetStationRef(startStationID)->GetMyInventory()->GetByID(entityID)->quantity() > entry.second) {
                                // If located stack contains more than we need, we split it and transfer the required amount.
                                sItemFactory.GetStationRef(startStationID)->GetMyInventory()->GetByID(entityID)->Split(entry.second)->ChangeOwner(issuerID, true);
                            } else {
                                // If not - we simply transfer it to issuer.
                                sItemFactory.GetItemRef(entityID)->ChangeOwner(issuerID, true);
                            }
                        }
                    }

                    // And finally, we go for traded items
                    if (!tradedItems.empty()) {
                        for (auto item : tradedItems) {
                            sItemFactory.GetItemRef(item)->ChangeOwner(call.client->GetCharacterID(), true);
                        }
                    }

                    // Once all manipulations are done, we update contract status. Response is sent outside the switch clause;
                    DBerror err;
                    if (!sDatabase.RunQuery(err,
                                            "UPDATE ctrContracts SET status = 4, dateAccepted = %lli, dateCompleted = %lli, acceptorID = %u WHERE contractId = %u",
                                            timestamp, timestamp, call.client->GetCharacterID(), contractID))
                    {
                        codelog(DATABASE__ERROR, "Failed to update contract : %s", err.c_str());
                    }
                } else {
                    if (!iskRequirementMet) {
                        call.client->SendNotifyMsg("You have insufficient funds");
                        return nullptr;
                    }
                    if (!rewardRequirementMet) {
                        call.client->SendNotifyMsg("Issuer have insufficient funds to pay for the contract");
                        return nullptr;
                    }
                    if (!requestedItemsRequirementsMet) {
                        call.client->SendNotifyMsg("You do not have required items to accept this contract");
                        return nullptr;
                    }
                }
                break;
            }
            case 3:
            {
                // Courier contract
                // First off, we validate if contract has a collateral. If yes - we check if player has enough ISK to pay. If not - we send a notification and quit
                if (collateral > 0) {
                    if (call.client->GetBalance() < collateral) {
                        call.client->SendNotifyMsg("You do not have enough ISK to pay collateral");
                        return nullptr;
                    }
                    // If we have enough - we take it
                    call.client->AddBalance(-collateral);
                }

                /**
                 * Courier contract acceptance includes following steps:
                 * - Create a plastic wrap container
                 * - Set capacity and volume attribute to the volume of the contracted items
                 * - Move all items inside this container
                 * -
                 * - Give that container to the character
                 * - Update contract entry - move it to In Progress and leave a time-stamp when it was accepted.
                 */
                // Create container and set capacity and volume attributes;
                std::string containerName = sItemFactory.GetSolarSystemRef(startSolarSystemID)->name();
                containerName = containerName + " -> " + sItemFactory.GetSolarSystemRef(endSolarSystemID)->name() + "(" + std::to_string(volume) + "m3)";

                ItemData itemData(itemPlasticWrap, call.client->GetCharacterID(), locTemp, flagNone);
                itemData.name = containerName;
                InventoryItemRef plasticWrap = sItemFactory.SpawnItem(itemData);
                if (plasticWrap.get() != nullptr) {
                    plasticWrap->SetAttribute(AttrVolume, volume);
                    plasticWrap->SetAttribute(AttrCapacity, volume);
                }
                plasticWrap->SaveItem();
                // Then, move all required items into it
                std::vector<int> items;
                ContractUtils::GetContractItemIDs(contractID->value(), &items);
                for (auto item : items) {
                    InventoryItemRef itm = sItemFactory.GetItemRef(item);
                    if (itm.get() != nullptr) {
                        itm->Move(plasticWrap->itemID(), flagNone, true);
                        itm->ChangeOwner(call.client->GetCharacterID());
                    }
                }
                // And we give the container to the player
                plasticWrap->Move(startStationID, flagHangar, true);

                // Finally, we update DB entry
                DBerror err;
                if (!sDatabase.RunQuery(err,
                                        "UPDATE ctrContracts SET status = 1, dateAccepted = %lli, acceptorID = %u, crateID = %u WHERE contractId = %u",
                                        timestamp, call.client->GetCharacterID(), plasticWrap->itemID(), contractID))
                {
                    codelog(DATABASE__ERROR, "Failed to update contract : %s", err.c_str());
                }
                break;
            }
            default:
                // We do not expect anything abnormal.
                return nullptr;
        }
        // Once type-specific manipulations are done, we query brief contract information (requested by client) and send it out.
        if (!sDatabase.RunQuery(res, "SELECT contractId, contractType, startStationID, endStationID, dateAccepted, numDays FROM ctrContracts WHERE contractId = %u", contractID))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }

        // DBResultToCRowset return doesn't work, for some reason (fails at unmarshalling on client's side), so making it a KeyVal dict
        res.GetRow(row);
        PyDict* ret = new PyDict;
        ret->SetItemString("contractID", new PyInt(row.GetInt(0)));
        ret->SetItemString("type", new PyInt(row.GetInt(1)));
        ret->SetItemString("startStationID", new PyInt(row.GetInt(2)));
        ret->SetItemString("endStationID", new PyInt(row.GetInt(3)));
        ret->SetItemString("dateAccepted", new PyLong(row.GetInt64(4)));
        ret->SetItemString("numDays", new PyInt(row.GetInt(5)));
        return new PyObject("util.KeyVal", ret);
    } else {
        return nullptr;
    }
}


PyResult ContractProxy::CompleteContract(PyCallArgs &call, PyInt* contractID, PyInt* completionStatus) {
    sLog.White( "ContractProxy::Handle_CompleteContract()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT contractType, status, price, reward, collateral, volume, startStationID, endStationID, issuerID, forCorp, crateID FROM ctrContracts WHERE contractId = %u", contractID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return new PyBool(false);
    }
    if (res.GetRowCount() == 0) {
        return new PyBool(false);
    }
    DBResultRow row;
    res.GetRow(row);
    int contractType = row.GetInt(0);
    int status = row.GetInt(1);
    int reward = row.GetInt(3);
    int collateral = row.GetInt(4);
    int startStationID = row.GetInt(6);
    int endStationID = row.GetInt(7);
    int issuerID = row.GetInt(8);
    bool forCorp = row.GetBool(9);
    int crateID = row.GetInt(10);

    int64 timestamp = int64(GetFileTimeNow());
    switch (completionStatus->value()) {
        case 4: {
            // Complete
            // First, we need to make sure that the container is indeed located in the end station
            if (call.client->GetStationID() != endStationID) {
                call.client->SendNotifyMsg("You have to deliver the package to %s", sItemFactory.GetStationRef(endStationID)->name());
                return new PyBool(false);
            }
            // Then, we validate the presence of all expected items
            std::map<int, int> expectedItems;
            ContractUtils::GetContractItemIDsAndQuantities(contractID->value(), &expectedItems);
            bool allItemsPresent(true);
            for (const auto& entry : expectedItems) {
                InventoryItemRef item = sItemFactory.GetItemRef(entry.first);
                if (item.get()) {
                    if (item->quantity() == entry.second && item->locationID() == crateID) {
                        continue;
                    }
                }
                allItemsPresent = false;
            }

            if (allItemsPresent) {
                // Once checks have passed, we extract the items into contractor's inventory
                for (const auto& entry : expectedItems) {
                    InventoryItemRef item = sItemFactory.GetItemRef(entry.first);
                    item->ChangeOwner(issuerID, true);
                    item->Move(endStationID, flagHangar, true);
                }
                // Plastic wrap seems to self-destruct after all the items are removed from it, so there's no need to delete it.

                // Then, we return the collateral (if any) and pay the reward (if any)
                if (collateral > 0) {
                    call.client->AddBalance(collateral);
                }
                if (reward > 0) {
                    call.client->AddBalance(reward);
                }

                // Then, we update the contract as Completed.
                DBerror err;
                if (!sDatabase.RunQuery(err,
                                        "UPDATE ctrContracts SET status = %u, dateCompleted = %lli WHERE contractId = %u",
                                        completionStatus->value(), timestamp, contractID->value()))
                {
                    codelog(DATABASE__ERROR, "Failed to update contract : %s", err.c_str());
                }
            } else {
                call.client->SendNotifyMsg("Not all required items are located in the container");
                return new PyBool(false);
            }
            break;
        }
        case 7: {
            // Fail
            // We pay the collateral to issuer and update the contract as failed.
            if (collateral > 0) {
                // Since we've taken the collateral prior to it and left it "hanging in the air" we put it back into acceptor's wallet and then issue a transfer
                call.client->AddBalance(collateral);
                AccountService::TransferFunds(call.client->GetCharacterID(), issuerID, collateral, "Collateral payment for failed contract", Journal::EntryType::ContractCollateral, contractID->value());
            }
            // Then, we update the contract as Афшдув.
            DBerror err;
            if (!sDatabase.RunQuery(err,
                                    "UPDATE ctrContracts SET status = %u, dateCompleted = %lli WHERE contractId = %u",
                                    completionStatus->value(), timestamp, contractID->value()))
            {
                codelog(DATABASE__ERROR, "Failed to update contract : %s", err.c_str());
            }
            break;
        }
        default:
            // We do not expect anything else
            return new PyBool(false);
    }

    return new PyBool(true);
}


PyResult ContractProxy::GetMyExpiredContractList(PyCallArgs &call) {
  sLog.White( "ContractProxy::Handle_GetMyExpiredContractList()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
/*
      [PySubStream 530 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 3 kvp]
            [PyString "contracts"]
            [PyObjectEx Type2]
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                  [PyString "header"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken blue.DBRowDescriptor]
                      [PyTuple 1 items]
                        [PyTuple 28 items]
                          [PyTuple 2 items]
                            [PyString "contractID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "type"]
                            [PyInt 17]
                          [PyTuple 2 items]
                            [PyString "issuerID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "issuerCorpID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "forCorp"]
                            [PyInt 11]
                          [PyTuple 2 items]
                            [PyString "availability"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "assigneeID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "acceptorID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "dateIssued"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "dateExpired"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "dateAccepted"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "numDays"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "dateCompleted"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "startStationID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "startSolarSystemID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "startRegionID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endStationID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endSolarSystemID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endRegionID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "price"]
                            [PyInt 6]
                          [PyTuple 2 items]
                            [PyString "reward"]
                            [PyInt 6]
                          [PyTuple 2 items]
                            [PyString "collateral"]
                            [PyInt 6]
                          [PyTuple 2 items]
                            [PyString "title"]
                            [PyInt 130]
                          [PyTuple 2 items]
                            [PyString "status"]
                            [PyInt 17]
                          [PyTuple 2 items]
                            [PyString "volume"]
                            [PyInt 5]
                          [PyTuple 2 items]
                            [PyString "issuerAllianceID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "issuerWalletKey"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "acceptorWalletKey"]
                            [PyInt 3]
            [PyString "items"]
            [PyDict 0 kvp]
            [PyString "bids"]
            [PyDict 0 kvp]
            */
    return nullptr;
}

PyResult ContractProxy::NumOutstandingContracts(PyCallArgs &call) {
    sLog.White( "ContractProxy::Handle_NumOutstandingContracts()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    /*
      [PySubStream 87 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 4 kvp]
            [PyString "nonCorpForMyChar"]
            [PyInt 0]
            [PyString "myCorpTotal"]
            [PyInt 0]
            [PyString "nonCorpForMyCorp"]
            [PyInt 0]
            [PyString "myCharTotal"]
            [PyInt 0]
            */
    return nullptr;
}

PyResult ContractProxy::GetItemsInStation(PyCallArgs &call, PyInt* stationID, std::optional<PyInt*> forCorp) {
    uint32 station = call.tuple->GetItem(0)->AsInt()->value();

    if (sDataMgr.IsStation(stationID->value()) == false)
        return nullptr;

    return sItemFactory.GetStationRef(station)->GetMyInventory()->List(flagHangar);
}

PyResult ContractProxy::CollectMyPageInfo(PyCallArgs &call) {
    sLog.White( "ContractProxy::Handle_CollectMyPageInfo()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    std::string mainQuery = "SELECT "
                            "IFNULL(SUM(CASE WHEN (cC.issuerID = {CHAR_ID} AND cC.status = 0) THEN 1 ELSE 0 END), 0) AS numOutstandingContracts, "
                            "IFNULL(SUM(CASE WHEN (cC.issuerID = {CHAR_ID} AND cC.status = 0 AND cC.forCorp = false) THEN 1 ELSE 0 END), 0) AS numOutstandingContractsNonCorp, "
                            "IFNULL(SUM(CASE WHEN (cC.issuerID = {CHAR_ID} AND cC.status = 0 AND cC.forCorp = true) THEN 1 ELSE 0 END), 0) AS numOutstandingContractsForCorp, "
                            "IFNULL(SUM(CASE WHEN (cC.assigneeID = {CHAR_ID} AND cC.forCorp = false AND cC.status = 1) THEN 1 ELSE 0 END), 0) AS numInProgress, "
                            "IFNULL(SUM(CASE WHEN (cC.issuerCorpID = {CORP_ID} AND cC.forCorp = true AND cC.status = 1) THEN 1 ELSE 0 END), 0) AS numInProgressCorp, "
                            "IFNULL(SUM(CASE WHEN (cC.assigneeID = {CHAR_ID} AND cC.status = 1) THEN 1 ELSE 0 END), 0) AS numRequiresAttention, "
                            "IFNULL(SUM(CASE WHEN (cC.issuerCorpID = {CORP_ID} AND cC.forCorp = true AND cC.status = 1) THEN 1 ELSE 0 END), 0) AS numRequiresAttentionCorp "
                            "FROM ctrContracts cC "
                            "LEFT JOIN ctrBids cB on cC.contractId = cB.contractId";
    std::string outstandingContractsQuery = "SELECT issuerID, issuerCorpID, assigneeID, contractType "
                                            "FROM ctrContracts "
                                            "WHERE assigneeID IN ({CHAR_ID},{CORP_ID}) and status = 0";
    // Since we already use boost - why not use some of it's libraries for string juggling? :P
    boost::replace_all(mainQuery, "{CHAR_ID}", std::to_string(call.client->GetCharacterID()));
    boost::replace_all(mainQuery, "{CORP_ID}", std::to_string(call.client->GetCorporationID()));
    boost::replace_all(outstandingContractsQuery, "{CHAR_ID}", std::to_string(call.client->GetCharacterID()));
    boost::replace_all(outstandingContractsQuery, "{CORP_ID}", std::to_string(call.client->GetCorporationID()));

    // First, we gather outstanding contracts - they will later be packed into response dict
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, outstandingContractsQuery.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in mainQuery: %s", res.error.c_str());
        return nullptr;
    }
    DBResultRow row;
    PyList* oustandingContractsList = new PyList;
    while (res.GetRow(row)) {
        PyList* list = new PyList(4);
        list->SetItem(0, new PyInt(row.GetInt(0)));
        list->SetItem(1, new PyInt(row.GetInt(1)));
        list->SetItem(2, new PyInt(row.GetInt(2)));
        list->SetItem(3, new PyInt(row.GetInt(3)));

        oustandingContractsList->AddItem(list);
    }

    // Then, we go for main message body data
    if (!sDatabase.RunQuery(res, mainQuery.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in mainQuery: %s", res.error.c_str());
        return nullptr;
    }
    // Because of outstandingContracts list in this response, we can't return DBResultToCRowset directly - so, we'll have to compose the dict manually.
    res.GetRow(row);
    PyDict* vals = new PyDict;
    vals->SetItemString("numOutstandingContracts", new PyInt(row.GetInt(0)));
    vals->SetItemString("numOutstandingContractsNonCorp", new PyInt(row.GetInt(1)));
    vals->SetItemString("numOutstandingContractsForCorp", new PyInt(row.GetInt(2)));
    vals->SetItemString("numInProgress", new PyInt(row.GetInt(3)));
    vals->SetItemString("numInProgressCorp", new PyInt(row.GetInt(4)));
    vals->SetItemString("outstandingContracts", oustandingContractsList);
    vals->SetItemString("numRequiresAttention", new PyInt(row.GetInt(5)));
    vals->SetItemString("numRequiresAttentionCorp", new PyInt(row.GetInt(6)));
    vals->SetItemString("numBiddingOn", new PyInt(0));      // Left hard-coded until bidding is implemented
    vals->SetItemString("numBiddingOnCorp", new PyInt(0));  // Left hard-coded until bidding is implemented

    return new PyObject("util.KeyVal", vals);
}

PyResult ContractProxy::GetContractListForOwner(PyCallArgs &call, PyInt* ownerID, PyInt* contractStatus, std::optional <PyInt*> contractType, std::optional <PyBool*> issuedToBy) {
    sLog.White( "ContractProxy::Handle_GetContractListForOwner()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return ContractUtils::GetContractListForOwner(ownerID, contractStatus, contractType, issuedToBy);
    /*
     *  client call....
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 73 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "GetContractListForOwner"]
            [PyTuple 4 items]
              [PyInt 649670823]
              [PyInt 0] (Contract status, 0 for outstanding, 1 for in progress, 4 for Finished)
              [PyInt] (Contract type. 1 for exchange, 2 for auction, 3 for courier, None for all)
              [PyBool] (false for Issued By, true for issued To, None for both)
            [PyDict 3 kvp]
              [PyString "num"]
              [PyInt 100]
              [PyString "machoVersion"]
              [PyInt 1]
              [PyString "startContractID"]
              [PyNone]

        server reply...
      [PySubStream 713 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 3 kvp]
            [PyString "contracts"]
            [PyObjectEx Type2]
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                  [PyString "header"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken blue.DBRowDescriptor]
                      [PyTuple 1 items]
                        [PyTuple 30 items]
                          [PyTuple 2 items]
                            [PyString "contractID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "type"]
                            [PyInt 17]
                          [PyTuple 2 items]
                            [PyString "issuerID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "issuerCorpID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "forCorp"]
                            [PyInt 11]
                          [PyTuple 2 items]
                            [PyString "availability"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "assigneeID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "acceptorID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "dateIssued"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "dateExpired"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "dateAccepted"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "numDays"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "dateCompleted"]
                            [PyInt 64]
                          [PyTuple 2 items]
                            [PyString "startStationID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "startSolarSystemID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "startRegionID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endStationID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endSolarSystemID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endRegionID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "price"]
                            [PyInt 6]
                          [PyTuple 2 items]
                            [PyString "reward"]
                            [PyInt 6]
                          [PyTuple 2 items]
                            [PyString "collateral"]
                            [PyInt 6]
                          [PyTuple 2 items]
                            [PyString "title"]
                            [PyInt 130]
                          [PyTuple 2 items]
                            [PyString "status"]
                            [PyInt 17]
                          [PyTuple 2 items]
                            [PyString "volume"]
                            [PyInt 5]
                          [PyTuple 2 items]
                            [PyString "issuerAllianceID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "issuerWalletKey"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "acceptorWalletKey"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "crateID"]
                            [PyInt 20]
                          [PyTuple 2 items]
                            [PyString "contractID"]
                            [PyInt 3]
              [PyPackedRow 146 bytes]
                ["contractID" => <41239648> [I4]]
                ["type" => <1> [UI1]]
                ["issuerID" => <649670823> [I4]]
                ["issuerCorpID" => <98038978> [I4]]
                ["forCorp" => <0> [Bool]]
                ["availability" => <1> [I4]]
                ["assigneeID" => <1661059544> [I4]]
                ["acceptorID" => <0> [I4]]
                ["dateIssued" => <129494707760000000> [FileTime]]
                ["dateExpired" => <129495571760000000> [FileTime]]
                ["dateAccepted" => <129494707760000000> [FileTime]]
                ["numDays" => <0> [I4]]
                ["dateCompleted" => <129494707760000000> [FileTime]]
                ["startStationID" => <60006433> [I4]]
                ["startSolarSystemID" => <30000135> [I4]]
                ["startRegionID" => <10000002> [I4]]
                ["endStationID" => <60006433> [I4]]
                ["endSolarSystemID" => <0> [I4]]
                ["endRegionID" => <0> [I4]]
                ["price" => <150000000> [CY]]
                ["reward" => <0> [CY]]
                ["collateral" => <0> [CY]]
                ["title" => <quafe!> [WStr]]
                ["status" => <0> [UI1]]
                ["volume" => <6> [R8]]
                ["issuerAllianceID" => <0> [I4]]
                ["issuerWalletKey" => <0> [I4]]
                ["acceptorWalletKey" => <0> [I4]]
                ["crateID" => <1002309425092> [I8]]
                ["contractID" => <41239648> [I4]]
            [PyString "items"]
            [PyDict 1 kvp]
              [PyInt 41239648]
              [PyList 1 items]
                [PyObjectData Name: util.Row]
                  [PyDict 2 kvp]
                    [PyString "header"]
                    [PyList 3 items]
                      [PyString "itemTypeID"]
                      [PyString "quantity"]
                      [PyString "inCrate"]
                    [PyString "line"]
                    [PyList 3 items]
                      [PyInt 3898]
                      [PyInt 6]
                      [PyBool True]
            [PyString "bids"]
            [PyDict 0 kvp]
     */
    return nullptr;
}

PyResult ContractProxy::GetLoginInfo(PyCallArgs &call)
{
    // currently a stub as I need to redesign or change some sub systems for this.

    /* create needsAttention row descriptor */
    DBRowDescriptor *needsAttentionHeader = new DBRowDescriptor();
        needsAttentionHeader->AddColumn( "contractID",   DBTYPE_I4);
        needsAttentionHeader->AddColumn( "",             DBTYPE_I4);

    /* create inProgress row descriptor */
    DBRowDescriptor *inProgressHeader = new DBRowDescriptor();
        inProgressHeader->AddColumn( "contractID",      DBTYPE_I4 );
        inProgressHeader->AddColumn( "startStationID",  DBTYPE_I4 );
        inProgressHeader->AddColumn( "endStationID",    DBTYPE_I4 );
        inProgressHeader->AddColumn( "expires",         DBTYPE_FILETIME );

    /* create assignedToMe row descriptor */
    DBRowDescriptor *assignedToMeHeader = new DBRowDescriptor();
        assignedToMeHeader->AddColumn( "contractID",    DBTYPE_I4);
        assignedToMeHeader->AddColumn( "issuerID",      DBTYPE_I4);

    CRowSet *needsAttention_rowset = new CRowSet( &needsAttentionHeader );
    CRowSet *inProgress_rowset = new CRowSet( &inProgressHeader );
    CRowSet *assignedToMe_rowset = new CRowSet( &assignedToMeHeader );

    PyDict* args = new PyDict;
        args->SetItemString( "needsAttention",          needsAttention_rowset );
        args->SetItemString( "inProgress",              inProgress_rowset );
        args->SetItemString( "assignedToMe",            assignedToMe_rowset );

    return new PyObject( "util.KeyVal", args );
}

     /**
     * NOTE: Contract statuses:
     * 0 - Outstanding
     * 1 - In Progress
     * 2 - Items not yet claimed
     * 3 - Unclaimed by seller
     * 4,5 - Finished
     * 6 - Rejected
     * 7 - Failed
     * 8 - Deleted
     * 9 - Reversal
     */

/* Description
 * GetLoginInfo sends back a util.KeyVal PyClass. This class contains 3 entries, those entries are
 * "needsAttention", "inProgress" and "assignedToMe".
 *
 * ------------------
 * assignedToMe
 * ------------------
 * assignedToMe is a pretty obvious packet, it contains rows of userid's and contractid's
 * that are belonging to the player.
 *
 * ------------------
 * inProgress
 * ------------------
 * inProgress is also pretty obvious, it contains rows of contractID, startStationID, endStationID and expires
 * that marks what contract is in progress.
 *
 * ------------------
 * needsAttention
 * ------------------
 * needsAttention contains contracts that need attention, its rather a way to make the journal blink. The
 * contracts off course are highlighted in the journal.
 *
      [PySubStream 273 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 3 kvp]
            [PyString "inProgress"]
            [PyObjectEx Type2]
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                  [PyString "header"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken blue.DBRowDescriptor]
                      [PyTuple 1 items]
                        [PyTuple 4 items]
                          [PyTuple 2 items]
                            [PyString "contractID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "startStationID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "endStationID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "expires"]
                            [PyInt 64]
            [PyString "needsAttention"]
            [PyObjectEx Type2]
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                  [PyString "header"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken blue.DBRowDescriptor]
                      [PyTuple 1 items]
                        [PyTuple 2 items]
                          [PyTuple 2 items]
                            [PyString "contractID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString ""]
                            [PyInt 3]
            [PyString "assignedToMe"]
            [PyObjectEx Type2]
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                  [PyString "header"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken blue.DBRowDescriptor]
                      [PyTuple 1 items]
                        [PyTuple 2 items]
                          [PyTuple 2 items]
                            [PyString "contractID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "issuerID"]
                            [PyInt 3]
*/
