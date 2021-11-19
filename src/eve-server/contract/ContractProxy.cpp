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

#include <algorithm>    // Added to prevent std::find from freaking out
#include "eve-server.h"

#include "PyServiceCD.h"
#include "contract/ContractProxy.h"
#include "station/Station.h"
#include "packets/Contracts.h"
#include "contract/ContractUtils.h"

PyCallable_Make_InnerDispatcher(ContractProxy)

ContractProxy::ContractProxy( PyServiceMgr *mgr )
: PyService(mgr, "contractProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ContractProxy, GetContract);
    PyCallable_REG_CALL(ContractProxy, CreateContract);
    PyCallable_REG_CALL(ContractProxy, DeleteContract);
    PyCallable_REG_CALL(ContractProxy, AcceptContract);
    PyCallable_REG_CALL(ContractProxy, GetLoginInfo);
    PyCallable_REG_CALL(ContractProxy, SearchContracts);
    PyCallable_REG_CALL(ContractProxy, NumOutstandingContracts);
    PyCallable_REG_CALL(ContractProxy, CollectMyPageInfo);
    PyCallable_REG_CALL(ContractProxy, GetItemsInStation);
    PyCallable_REG_CALL(ContractProxy, GetContractListForOwner);
    PyCallable_REG_CALL(ContractProxy, GetMyExpiredContractList);
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

ContractProxy::~ContractProxy()
{
    delete m_dispatch;
}


PyResult ContractProxy::Handle_SearchContracts(PyCallArgs &call) {
    // We will not proceed, if contractType is not specified
    if (!call.byname.find("contractType")->second->IsNone()) {
        int contractType = call.byname.find("contractType")->second->AsInt()->value();

        /**
         * We're using sort of query constructor here - if request have certain value specified, we add it as another AND block.
         * For now, we will only filter by contract type, item type, item category, min/max price, min/max reward, location/end location and issuer
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
        // According to what i had during testing, locationID can only be system, constellation or region. Given that we only store system and region ID, we use OR clause for these
        if (!call.byname.find("locationID")->second->IsNone()) {
            int locationId = call.byname.find("locationID")->second->AsInt()->value();
            if (locationId >= 30000000 && locationId < 40000000) {
                // Solar system range
                query.append(" AND cC.startSolarSystemID = " + std::to_string(locationId));
            } else if (locationId >= 10000000 && locationId < 20000000) {
                // Region range
                query.append(" AND cC.startRegionID = " + std::to_string(locationId));
            }
        }
        // Same applies to endLocationID - it uses the same search::QuickQuery() call to get it
        if (!call.byname.find("endLocationID")->second->IsNone()) {
            int locationId = call.byname.find("endLocationID")->second->AsInt()->value();
            if (locationId >= 30000000 && locationId < 40000000) {
                // Solar system range
                query.append(" AND cC.endSolarSystemID = " + std::to_string(locationId));
            } else if (locationId >= 10000000 && locationId < 20000000) {
                // Region range
                query.append(" AND cC.endRegionID = " + std::to_string(locationId));
            }
        }
        // Once again, issuer can be either a character or a corporation. We use separate filters depending on value
        if (!call.byname.find("issuerID")->second->IsNone()) {
            int issuerId = call.byname.find("issuerID")->second->AsInt()->value();
            if (issuerId > 98000000) {
                // Corporation case
                query.append("AND cC.issuerCorpID = " + std::to_string(issuerId) + " AND cC.forCorp = true");
            } else {
                query.append("AND cC.issuerID = " + std::to_string(issuerId) + " AND cC.forCorp = false");
            }
        }
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
        response->SetItemString("searchTime", new PyInt(153));
        response->SetItemString("maxResults", new PyInt(1000));

        return new PyObject("util.KeyVal", response);
    } else {
        codelog(SERVICE__ERROR, "%s: ContractType was not specified. Aborting search", GetName());
        return nullptr;
    }
}

PyResult ContractProxy::Handle_CreateContract(PyCallArgs &call) {
    Call_CreateContract req;
    if (!req.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
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
    if (sDataMgr.IsStation(req.startStationId)) {
        startSystemId = sDataMgr.GetStationSystem(req.startStationId);
        startRegionId = sDataMgr.GetStationRegion(req.startStationId);
    } else {
        codelog(SERVICE__ERROR, "Specified start Station ID has no Station counterparts in DB");
        return nullptr;
    }
    if (req.endStationId != 0) {
        if (sDataMgr.IsStation(req.endStationId)) {
            endSystemId = sDataMgr.GetStationSystem(req.endStationId);
            endRegionId = sDataMgr.GetStationRegion(req.endStationId);
        } else {
            codelog(SERVICE__ERROR, "Specified end Station ID has no Station counterparts in DB");
            return nullptr;
        }
    } else {
        endSystemId = startSystemId;
        endRegionId = 0;
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
        "%li, %li, %u, %u, %u, "
        "%u, %u, %u, %u, %u, %u,"
        "%u, %u, %u, '%s', '%s', %u, %u)",
        req.contractType, call.client->GetCharacterID(), call.client->GetCorporationID(), forCorp, req.isPrivate?1:0, req.assigneeID,
        int64(GetFileTimeNow()), int64(GetRelativeFileTime(0, 0, req.expireTime)), req.expireTime, req.duration, req.expireTime / 1440,
        req.startStationId, startSystemId, startRegionId, req.endStationId, endSystemId, endRegionId,
        req.price, req.reward, req.collateral, req.title.c_str(), req.description.c_str(), call.client->GetAllianceID(), startStationDivision))
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
                     if (req.contractType == 3) {
                         totalVolume += sItemFactory.GetStationItem(req.startStationId)->GetMyInventory()->GetByID(itemID)->GetAttribute(161).get_float() * quantity;
                     }
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
        if (req.contractType == 3) {
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

    /*
        11:06:19 [Service] contractProxy::CreateContract()
        11:06:19 [SvcCallTrace]   Call Arguments:
        11:06:19 [SvcCallTrace]      Tuple: 12 elements
        11:06:19 [SvcCallTrace]       [ 0]    Integer: 1        Type (1 - exchange, 2 - auction, 3 - courier)
        11:06:19 [SvcCallTrace]       [ 1]    Boolean: false
        11:06:19 [SvcCallTrace]       [ 2]       None
        11:06:19 [SvcCallTrace]       [ 3]    Integer: 20160
        11:06:19 [SvcCallTrace]       [ 4]    Integer: 0
        11:06:19 [SvcCallTrace]       [ 5]    Integer: 60014719
        11:06:19 [SvcCallTrace]       [ 6]       None
        11:06:19 [SvcCallTrace]       [ 7]    Integer: 150000
        11:06:19 [SvcCallTrace]       [ 8]    Integer: 0
        11:06:19 [SvcCallTrace]       [ 9]    Integer: 0
        11:06:19 [SvcCallTrace]       [10]    WString: 'Just sum stuff'
        11:06:19 [SvcCallTrace]       [11]     String: ''
        11:06:19 [SvcCallTrace]  Named Arguments:
        11:06:19 [SvcCallTrace]   confirm
        11:06:19 [SvcCallTrace]           None
        11:06:19 [SvcCallTrace]   flag
        11:06:19 [SvcCallTrace]        Integer: 4
        11:06:19 [SvcCallTrace]   forCorp
        11:06:19 [SvcCallTrace]        Boolean: false
        11:06:19 [SvcCallTrace]   itemList
        16:17:17 [SvcCallTrace]       List: 3 elements
        16:17:17 [SvcCallTrace]       [ 0]   List: 2 elements
        16:17:17 [SvcCallTrace]       [ 0]   [ 0]    Integer: 140000032
        16:17:17 [SvcCallTrace]       [ 0]   [ 1]    Integer: 25
        16:17:17 [SvcCallTrace]       [ 1]   List: 2 elements
        16:17:17 [SvcCallTrace]       [ 1]   [ 0]    Integer: 140000029
        16:17:17 [SvcCallTrace]       [ 1]   [ 1]    Integer: 50
        16:17:17 [SvcCallTrace]       [ 2]   List: 2 elements
        16:17:17 [SvcCallTrace]       [ 2]   [ 0]    Integer: 140000031
        16:17:17 [SvcCallTrace]       [ 2]   [ 1]    Integer: 25
        11:06:19 [SvcCallTrace]   machoVersion
        11:06:19 [SvcCallTrace]        Integer: 1
        19:04:19 [SvcCallTrace]   requestItemTypeList
        19:04:19 [SvcCallTrace]       List: 2 elements
        19:04:19 [SvcCallTrace]       [ 0]   List: 2 elements
        19:04:19 [SvcCallTrace]       [ 0]   [ 0]    Integer: 482
        19:04:19 [SvcCallTrace]       [ 0]   [ 1]    Integer: 2
        19:04:19 [SvcCallTrace]       [ 1]   List: 2 elements
        19:04:19 [SvcCallTrace]       [ 1]   [ 0]    Integer: 27790
        19:04:19 [SvcCallTrace]       [ 1]   [ 1]    Integer: 1
     */


    // sLog.White( "ContractProxy::Handle_CreateContract()", "size=%li", call.tuple->size());
    // call.Dump(SERVICE__CALL_DUMP);

    // returns new contractID
    //return nullptr;
    return new PyInt((int) contractId);
}

PyResult ContractProxy::Handle_DeleteContract(PyCallArgs &call) {
    //  sends contractID to delete
    sLog.White( "ContractProxy::Handle_DeleteContract()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    /*
            [PyString "DeleteContract"]
            [PyTuple 1 items]
              [PyInt 41239648]


      [PySubStream 6 bytes]
        [PyBool True]
        */
    return nullptr;
}

PyResult ContractProxy::Handle_GetContract(PyCallArgs &call) {
    if (!call.tuple->empty()) {
        if (call.tuple->GetItem(0)->IsInt()) {
            return ContractUtils::GetContractEntry(call.tuple->GetItem(0)->AsInt()->value());
        } else {
            codelog(SERVICE__ERROR, "Invalid parameter in GetContract call");
            return nullptr;
        }
    } else {
        codelog(SERVICE__ERROR, "Empty GetContract call. Aborting");
        return nullptr;
    }
}

PyResult ContractProxy::Handle_AcceptContract(PyCallArgs &call) {
    /*
        [PyPackedRow 142 bytes]
          ["contractID" => <41239407> [I4]]
          ["type" => <1> [UI1]]
          ["issuerID" => <1661059544> [I4]]
          ["issuerCorpID" => <98038978> [I4]]
          ["forCorp" => <0> [Bool]]
          ["availability" => <1> [I4]]
          ["assigneeID" => <649670823> [I4]]
          ["acceptorID" => <0> [I4]]
          ["dateIssued" => <129494701600000000> [FileTime]]
          ["dateExpired" => <129495565600000000> [FileTime]]
          ["dateAccepted" => <129494701600000000> [FileTime]]
          ["numDays" => <0> [I4]]
          ["dateCompleted" => <129494701600000000> [FileTime]]
          ["startStationID" => <60006433> [I4]]
          ["startSolarSystemID" => <30000135> [I4]]
          ["startRegionID" => <10000002> [I4]]
          ["endStationID" => <60006433> [I4]]
          ["endSolarSystemID" => <0> [I4]]
          ["endRegionID" => <0> [I4]]
          ["price" => <0> [CY]]
          ["reward" => <100000000> [CY]]
          ["collateral" => <0> [CY]]
          ["title" => <this is a contract to Rhonin Caldera> [WStr]]
          ["description" => <empty string> [WStr]]
          ["status" => <0> [UI1]]
          ["crateID" => <1002309350211> [I8]]
          ["volume" => <6.01> [R8]]
          ["issuerAllianceID" => <0> [I4]]
          ["issuerWalletKey" => <0> [I4]]
          ["acceptorWalletKey" => <0> [I4]]


==================== Sent from Server 81 bytes

[PyObjectData Name: macho.Notification]
  [PyTuple 6 items]
    [PyInt 12]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 699185]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 4]
        [PyString "OnContractAccepted"]
        [PyList 0 items]
        [PyString "clientID"]
    [PyInt 5894042]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 15 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 1 items]
                [PyInt 41239473]
    [PyNone]


     */


    sLog.White( "ContractProxy::Handle_AcceptContract()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ContractProxy::Handle_GetMyExpiredContractList(PyCallArgs &call) {
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

PyResult ContractProxy::Handle_NumOutstandingContracts(PyCallArgs &call) {
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

PyResult ContractProxy::Handle_GetItemsInStation(PyCallArgs &call) {
    if (call.tuple->GetItem(0)->IsInt()) {
        uint32 station = call.tuple->GetItem(0)->AsInt()->value();

        if (sDataMgr.IsStation(station)) {
            CRowSet *rowSet = sItemFactory.GetStationItem(station)->GetMyInventory()->List(flagHangar);

            return rowSet;
        }
    }

    return nullptr;
}

PyResult ContractProxy::Handle_CollectMyPageInfo(PyCallArgs &call) {
    sLog.White( "ContractProxy::Handle_CollectMyPageInfo()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    /*
     *      [PySubStream 279 bytes]
     *        [PyObjectData Name: util.KeyVal]
     *          [PyDict 11 kvp]
     *            [PyString "numInProgressCorp"]
     *            [PyInt 0]
     *            [PyString "numRequiresAttention"]
     *            [PyInt 0]
     *            [PyString "numBiddingOn"]
     *            [PyInt 0]
     *            [PyString "numRequiresAttentionCorp"]
     *            [PyInt 0]
     *            [PyString "numInProgress"]
     *            [PyInt 0]
     *            [PyString "numBiddingOnCorp"]
     *            [PyInt 0]
     *            [PyString "numOutstandingContractsNonCorp"]
     *            [PyInt 0]
     *            [PyString "outstandingContracts"]
     *            [PyList 0 items]
     *            [PyString "numOutstandingContracts"]
     *            [PyInt 0]
     *            [PyString "numOutstandingContractsForCorp"]
     *            [PyInt 0]
     *            [PyString "numContractsLeftCorp"]
     *            [PyInt 0]
     */
    return nullptr;
}

PyResult ContractProxy::Handle_GetContractListForOwner(PyCallArgs &call) {
    sLog.White( "ContractProxy::Handle_GetContractListForOwner()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
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
              [PyInt 0]
              [PyNone]
              [PyNone]
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

PyResult ContractProxy::Handle_GetLoginInfo(PyCallArgs &call)
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
