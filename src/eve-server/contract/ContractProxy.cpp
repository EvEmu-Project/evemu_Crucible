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
    Author:        Captnoord
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "contract/ContractProxy.h"
#include "station/Station.h"

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

/** @todo  finally found good packet data for contract system.
 * now just need to write it....
 */

PyResult ContractProxy::Handle_SearchContracts(PyCallArgs &call) {
    /*
     * ret = sm.ProxySvc('contractProxy').SearchContracts(itemTypes=itemTypes, itemTypeName=itemTypeName, itemCategoryID=itemCategoryID, itemGroupID=itemGroupID, contractType=contractType, securityClasses=securityClasses, locationID=locationID, endLocationID=endLocationID, issuerID=issuerID, minPrice=minPrice, maxPrice=maxPrice, minReward=minReward, maxReward=maxReward, minCollateral=minCollateral, maxCollateral=maxCollateral, minVolume=minVolume, maxVolume=maxVolume, excludeTrade=excludeTrade, excludeMultiple=excludeMultiple, excludeNoBuyout=excludeNoBuyout, availability=availability, description=description, searchHint=searchHint, sortBy=sortBy, sortDir=sortDir, startNum=startNum)
     * contracts = ret.contracts
     * numFound = ret.numFound
     * searchTime = ret.searchTime
     * maxResults = ret.maxResults
     *
     *
     * AttributeError: 'NoneType' object has no attribute 'contracts'
     */

    /*  client call....
     *
        [PySubStream 359 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "SearchContracts"]
            [PyTuple 0 items]
            [PyDict 27 kvp]
              [PyString "itemTypeName"]
              [PyNone]
              [PyString "itemCategoryID"]
              [PyNone]
              [PyString "description"]
              [PyNone]
              [PyString "issuerID"]
              [PyNone]
              [PyString "excludeMultiple"]
              [PyInt 0]
              [PyString "minCollateral"]
              [PyNone]
              [PyString "excludeNoBuyout"]
              [PyNone]
              [PyString "securityClasses"]
              [PyNone]
              [PyString "maxVolume"]
              [PyNone]
              [PyString "endLocationID"]
              [PyNone]
              [PyString "maxCollateral"]
              [PyNone]
              [PyString "contractType"]
              [PyInt 10]
              [PyString "minPrice"]
              [PyNone]
              [PyString "availability"]
              [PyInt 2]
              [PyString "machoVersion"]
              [PyInt 1]
              [PyString "minReward"]
              [PyNone]
              [PyString "maxReward"]
              [PyNone]
              [PyString "minVolume"]
              [PyNone]
              [PyString "sortDir"]
              [PyInt 0]
              [PyString "searchHint"]
              [PyNone]
              [PyString "startNum"]
              [PyInt 0]
              [PyString "maxPrice"]
              [PyNone]
              [PyString "itemTypes"]
              [PyNone]
              [PyString "itemGroupID"]
              [PyNone]
              [PyString "locationID"]
              [PyNone]
              [PyString "excludeTrade"]
              [PyNone]
              [PyString "sortBy"]
              [PyInt 0]

    server return...
        -- for zero results
      [PySubStream 76 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 4 kvp]
            [PyString "contracts"]
            [PyList 0 items]
            [PyString "numFound"]
            [PyInt 0]
            [PyString "searchTime"]
            [PyIntegerVar 583232]
            [PyString "maxResults"]
            [PyInt 1000]
        -- for a hit
      [PySubStream 1176 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 4 kvp]
            [PyString "contracts"]
            [PyList 1 items]
              [PyObjectData Name: util.KeyVal]
                [PyDict 12 kvp]
                  [PyString "securityClassEnd"]
                  [PyNone]
                  [PyString "startSolarSystemName"]
                  [PyString "Outuni"]
                  [PyString "items"]
                  [PyList 2 items]
                    [PyPackedRow 52 bytes]
                      ["contractID" => <41239473> [I4]]
                      ["itemID" => <1002299726681> [I8]]
                      ["quantity" => <1> [I4]]
                      ["itemTypeID" => <3732> [I4]]
                      ["inCrate" => <1> [Bool]]
                      ["parentID" => <0> [I8]]
                      ["productivityLevel" => <0> [I4]]
                      ["materialLevel" => <0> [I4]]
                      ["copy" => <0> [Bool]]
                      ["licensedProductionRunsRemaining" => <0> [I4]]
                      ["damage" => <0> [R8]]
                      ["flagID" => <0> [I2]]
                    [PyPackedRow 52 bytes]
                      ["contractID" => <41239473> [I4]]
                      ["itemID" => <1002272878889> [I8]]
                      ["quantity" => <6> [I4]]
                      ["itemTypeID" => <3898> [I4]]
                      ["inCrate" => <1> [Bool]]
                      ["parentID" => <0> [I8]]
                      ["productivityLevel" => <0> [I4]]
                      ["materialLevel" => <0> [I4]]
                      ["copy" => <0> [Bool]]
                      ["licensedProductionRunsRemaining" => <0> [I4]]
                      ["damage" => <0> [R8]]
                      ["flagID" => <0> [I2]]
                  [PyString "bids"]
                  [PyList 0 items]
                  [PyString "contract"]
                  [PyPackedRow 142 bytes]
                    ["contractID" => <41239473> [I4]]
                    ["type" => <1> [UI1]]
                    ["issuerID" => <649670823> [I4]]
                    ["issuerCorpID" => <98038978> [I4]]
                    ["forCorp" => <0> [Bool]]
                    ["availability" => <1> [I4]]
                    ["assigneeID" => <98038978> [I4]]
                    ["acceptorID" => <0> [I4]]
                    ["dateIssued" => <129494703690000000> [FileTime]]
                    ["dateExpired" => <129495567690000000> [FileTime]]
                    ["dateAccepted" => <129494703690000000> [FileTime]]
                    ["numDays" => <0> [I4]]
                    ["dateCompleted" => <129494703690000000> [FileTime]]
                    ["startStationID" => <60006433> [I4]]
                    ["startSolarSystemID" => <30000135> [I4]]
                    ["startRegionID" => <10000002> [I4]]
                    ["endStationID" => <60006433> [I4]]
                    ["endSolarSystemID" => <0> [I4]]
                    ["endRegionID" => <0> [I4]]
                    ["price" => <150000000> [CY]]
                    ["reward" => <0> [CY]]
                    ["collateral" => <0> [CY]]
                    ["title" => <my contract to Munich Lumberjacks> [WStr]]
                    ["description" => <empty string> [WStr]]
                    ["status" => <0> [UI1]]
                    ["crateID" => <1002309375656> [I8]]
                    ["volume" => <6.01> [R8]]
                    ["issuerAllianceID" => <0> [I4]]
                    ["issuerWalletKey" => <0> [I4]]
                    ["acceptorWalletKey" => <0> [I4]]
                  [PyString "itemGroups"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken __builtin__.set]
                      [PyTuple 1 items]
                        [PyList 2 items]
                          [PyInt 266]
                          [PyInt 303]
                  [PyString "securityClassStart"]
                  [PyInt 2]
                  [PyString "startConstellationID"]
                  [PyInt 20000019]
                  [PyString "endConstellationID"]
                  [PyNone]
                  [PyString "itemCategories"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken __builtin__.set]
                      [PyTuple 1 items]
                        [PyList 2 items]
                          [PyInt 16]
                          [PyInt 20]
                  [PyString "itemTypes"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken __builtin__.set]
                      [PyTuple 1 items]
                        [PyList 2 items]
                          [PyInt 3898]
                          [PyInt 3732]
                  [PyString "numBids"]
                  [PyInt 0]
            [PyString "numFound"]
            [PyInt 1]
            [PyString "searchTime"]
            [PyIntegerVar 552645]
            [PyString "maxResults"]
            [PyInt 1000]
              */

    sLog.White( "ContractProxy::Handle_SearchContracts()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ContractProxy::Handle_CreateContract(PyCallArgs &call) {
    /*
        [PySubStream 171 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "CreateContract"]
            [PyTuple 12 items]
              [PyInt 1]
              [PyBool True]
              [PyInt 98038978]
              [PyInt 1440]
              [PyInt 0]
              [PyIntegerVar 60006433]
              [PyNone]
              [PyInt 15000]
              [PyInt 0]
              [PyInt 0]
              [PyString "my contract to Munich Lumberjacks"]
              [PyString ""]
            [PyDict 6 kvp]
              [PyString "requestItemTypeList"]
              [PyList 0 items]
              [PyString "confirm"]
              [PyNone]
              [PyString "forCorp"]
              [PyBool False]
              [PyString "flag"]
              [PyInt 4]
              [PyString "itemList"]
              [PyList 2 items]
                [PyList 2 items]
                  [PyIntegerVar 1002272878889]
                  [PyInt 6]
                [PyList 2 items]
                  [PyIntegerVar 1002299726681]
                  [PyInt 1]
              [PyString "machoVersion"]
              [PyInt 1]
     */


    sLog.White( "ContractProxy::Handle_CreateContract()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    // returns new contractID
    return nullptr;
}

PyResult ContractProxy::Handle_DeleteContract(PyCallArgs &call) {
    //  sends contractID to delete
    sLog.White( "ContractProxy::Handle_DeleteContract()", "size=%li", call.tuple->size());
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
    /*
      [PySubStream 1103 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 11 kvp]
            [PyString "securityClassEnd"]
            [PyNone]
            [PyString "startSolarSystemName"]
            [PyString "Outuni"]
            [PyString "items"]
            [PyList 2 items]
              [PyPackedRow 52 bytes]
                ["contractID" => <41239407> [I4]]
                ["itemID" => <1002299726681> [I8]]
                ["quantity" => <1> [I4]]
                ["itemTypeID" => <3732> [I4]]
                ["inCrate" => <1> [Bool]]
                ["parentID" => <0> [I8]]
                ["productivityLevel" => <0> [I4]]
                ["materialLevel" => <0> [I4]]
                ["copy" => <0> [Bool]]
                ["licensedProductionRunsRemaining" => <0> [I4]]
                ["damage" => <0> [R8]]
                ["flagID" => <0> [I2]]
              [PyPackedRow 52 bytes]
                ["contractID" => <41239407> [I4]]
                ["itemID" => <1002261932864> [I8]]
                ["quantity" => <6> [I4]]
                ["itemTypeID" => <3898> [I4]]
                ["inCrate" => <1> [Bool]]
                ["parentID" => <0> [I8]]
                ["productivityLevel" => <0> [I4]]
                ["materialLevel" => <0> [I4]]
                ["copy" => <0> [Bool]]
                ["licensedProductionRunsRemaining" => <0> [I4]]
                ["damage" => <0> [R8]]
                ["flagID" => <0> [I2]]
            [PyString "bids"]
            [PyList 0 items]
            [PyString "contract"]
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
            [PyString "itemGroups"]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.set]
                [PyTuple 1 items]
                  [PyList 2 items]
                    [PyInt 266]
                    [PyInt 303]
            [PyString "securityClassStart"]
            [PyInt 2]
            [PyString "startConstellationID"]
            [PyInt 20000019]
            [PyString "endConstellationID"]
            [PyNone]
            [PyString "itemCategories"]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.set]
                [PyTuple 1 items]
                  [PyList 2 items]
                    [PyInt 16]
                    [PyInt 20]
            [PyString "itemTypes"]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.set]
                [PyTuple 1 items]
                  [PyList 2 items]
                    [PyInt 3898]
                    [PyInt 3732]
    [PyNone]
    */

    sLog.White( "ContractProxy::Handle_GetContract()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
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


    sLog.White( "ContractProxy::Handle_AcceptContract()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}


PyResult ContractProxy::Handle_GetMyExpiredContractList(PyCallArgs &call) {
  sLog.White( "ContractProxy::Handle_GetMyExpiredContractList()", "size=%li", call.tuple->size());
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
    sLog.White( "ContractProxy::Handle_NumOutstandingContracts()", "size=%li", call.tuple->size());
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
    sLog.White( "ContractProxy::Handle_CollectMyPageInfo()", "size=%li", call.tuple->size());
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
    sLog.White( "ContractProxy::Handle_GetContractListForOwner()", "size=%li", call.tuple->size());
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
