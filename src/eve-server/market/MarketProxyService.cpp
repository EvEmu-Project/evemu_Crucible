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
    Updates:    Allan
*/

// this class isnt a singleton, but is instantiated once on server start.

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "market/MarketMgr.h"
#include "market/MarketProxyService.h"
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"
#include "standing/StandingDB.h"


/*
 * MARKET__ERROR
 * MARKET__WARNING
 * MARKET__MESSAGE
 * MARKET__DEBUG
 * MARKET__TRACE
 * MARKET__DB_ERROR
 * MARKET__DB_TRACE
 */

PyCallable_Make_InnerDispatcher(MarketProxyService)

MarketProxyService::MarketProxyService(PyServiceMgr *mgr)
: PyService(mgr, "marketProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MarketProxyService, StartupCheck);
    PyCallable_REG_CALL(MarketProxyService, GetStationAsks);
    PyCallable_REG_CALL(MarketProxyService, GetSystemAsks);
    PyCallable_REG_CALL(MarketProxyService, GetRegionBest);
    PyCallable_REG_CALL(MarketProxyService, GetMarketGroups);
    PyCallable_REG_CALL(MarketProxyService, GetOrders);
    PyCallable_REG_CALL(MarketProxyService, GetOldPriceHistory);
    PyCallable_REG_CALL(MarketProxyService, GetNewPriceHistory);
    PyCallable_REG_CALL(MarketProxyService, PlaceCharOrder);
    PyCallable_REG_CALL(MarketProxyService, GetCharOrders);
    PyCallable_REG_CALL(MarketProxyService, ModifyCharOrder);
    PyCallable_REG_CALL(MarketProxyService, CancelCharOrder);
    PyCallable_REG_CALL(MarketProxyService, CharGetNewTransactions);
    PyCallable_REG_CALL(MarketProxyService, CorpGetNewTransactions);
    PyCallable_REG_CALL(MarketProxyService, GetCorporationOrders);
}

MarketProxyService::~MarketProxyService() {
    delete m_dispatch;
}

PyResult MarketProxyService::Handle_GetMarketGroups(PyCallArgs &call) {
    return sMktMgr.GetMarketGroups();
}

PyResult MarketProxyService::Handle_StartupCheck(PyCallArgs &call) {
    //if (sMktMgr.NeedsUpdate())
    //    sMktMgr.UpdatePriceHistory();
    return nullptr;
}

PyResult MarketProxyService::Handle_GetCharOrders(PyCallArgs &call) {
    return m_db.GetOrdersForOwner(call.client->GetCharacterID());
}

PyResult MarketProxyService::Handle_GetCorporationOrders(PyCallArgs &call) {
    return m_db.GetOrdersForOwner(call.client->GetCorporationID());
}

/** @todo update these to use market manager and cache instead of hitting db? */
// station, system, region based on selection in market window
PyResult MarketProxyService::Handle_GetStationAsks(PyCallArgs &call) {
    return m_db.GetStationAsks(call.client->GetStationID());
}

PyResult MarketProxyService::Handle_GetSystemAsks(PyCallArgs &call) {
    return m_db.GetSystemAsks(call.client->GetSystemID());
}

PyResult MarketProxyService::Handle_GetRegionBest(PyCallArgs &call) {
    return m_db.GetRegionBest(call.client->GetRegionID());
}

// this is called 3x on every market transaction
PyResult MarketProxyService::Handle_GetOldPriceHistory(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return sMktMgr.GetOldPriceHistory(call.client->GetRegionID(), args.arg);
}

PyResult MarketProxyService::Handle_GetNewPriceHistory(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return sMktMgr.GetNewPriceHistory(call.client->GetRegionID(), args.arg);
}

PyResult MarketProxyService::Handle_CharGetNewTransactions(PyCallArgs &call)
{
    Call_GetNewCharTransactions args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Market::TxData data = Market::TxData();
        data.clientID = args.clientID;
        data.isBuy = args.sellBuy;
        data.price = args.minPrice;
        data.quantity = args.quantity;
        data.typeID = args.typeID;
        data.time = args.fromDate;
        data.accountKey = Account::KeyType::Cash;
    return m_db.GetTransactions(call.client->GetCharacterID(), data);
}

PyResult MarketProxyService::Handle_CorpGetNewTransactions(PyCallArgs &call)
{
    Call_GetNewCorpTransactions args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Market::TxData data = Market::TxData();
        data.clientID = args.clientID;
        data.isBuy = args.sellBuy;
        data.price = args.minPrice;
        data.quantity = args.quantity;
        data.typeID = args.typeID;
        data.time = args.fromDate;
        data.accountKey = args.accountKey;
        data.memberID = args.memberID;
    return m_db.GetTransactions(call.client->GetCorporationID(), data);
}

PyResult MarketProxyService::Handle_GetOrders(PyCallArgs &call) {
    Call_SingleIntegerArg args; //typeID
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyRep* result(nullptr);
    std::string method_name ("GetOrders_");
    method_name += std::to_string(call.client->GetRegionID());
    method_name += "_";
    method_name += std::to_string(args.arg);
    ObjectCachedMethodID method_id(GetName(), method_name.c_str());
    //check to see if this method is in the cache already.
    if (!m_manager->cache_service->IsCacheLoaded(method_id))
    {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetOrders(call.client->GetRegionID(), args.arg);
        if (result == nullptr) {
            _log(MARKET__DB_ERROR, "Failed to load cache, generating empty contents.");
            result = PyStatic.NewNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    /*{'FullPath': u'UI/Messages', 'messageID': 258616, 'label': u'MktMarketOpeningTitle'}(u'Market not open yet', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258617, 'label': u'MktMarketOpeningBody'}
     * (u'{region} market region is currently opening up for business and is not yet ready.
     * Please try again in a few minutes, or refer to another market region for your trading needs.',
     * None, {u'{region}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'region'}})
     */
    return result;
}

PyResult MarketProxyService::Handle_PlaceCharOrder(PyCallArgs &call) {
    //self.GetMarketProxy().PlaceCharOrder(int(stationID), int(typeID), round(float(price), 2), int(quantity), int(bid), int(orderRange),
    //   itemID = None, int(minVolume = 1), int(duration = 14), useCorp = False, located = None)
    // located = [officeFolderID, officeID] or None

  /*
21:47:34 [MktDump] Mkt::PlaceCharOrder()
21:47:34 [MktDump]     Call_PlaceCharOrder
21:47:34 [MktDump]     stationID=60014137
21:47:34 [MktDump]     typeID=20327
21:47:34 [MktDump]     price=100.0000000000000
21:47:34 [MktDump]     quantity=1
21:47:34 [MktDump]     bid=1
21:47:34 [MktDump]     orderRange=4294967295
21:47:34 [MktDump]     itemID=0
21:47:34 [MktDump]     minVolume=1
21:47:34 [MktDump]     duration=0
21:47:34 [MktDump]     useCorp=0
21:47:34 [MktDump]     located:
21:47:34 [MktDump]               None
*/
    Call_PlaceCharOrder args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // not sent from call, but used in transactions
    uint16 accountKey(Account::KeyType::Cash);

    _log(MARKET__DUMP, "Mkt::PlaceCharOrder()");
    args.Dump(MARKET__DUMP, "    ");

    //TODO: verify the validity of args.stationID (range vs. skill)

    if (args.useCorp) {
        _log(MARKET__MESSAGE, "Denying Corp Market use for %s", call.client->GetName());
        call.client->SendErrorMsg("Corporation Market transactions are not available at this time.");
        return nullptr;
    }

    /** @todo  update for corporate use
     * check corp has office in target station for item delivery
     * check player can use given corp acct (get current division from char data)
     * check corp wallet division has funds (get current division from char data)
     * code to handle 'located' as sent
     *   ...more?
     * corp checks coded for ram/reproc.  use as template
     */

    if (args.bid and (args.itemID == 0)) {  //buy
        // check for corp usage and get standings with station owners
        float fStanding(0), cStanding(0);
        if (args.useCorp) {
            // it is.  perform checks and set needed variables for corp use
            if (!IsPlayerCorp(call.client->GetCorporationID())) {
                // cant buy items for npc corp...
                call.client->SendErrorMsg("You cannot buy items for an NPC corp.");
                return nullptr;
            }
            // this is a corp transaction.  verify char can buy shit for corp...
            // some corp error msgs in inventory.h, corp.h and market.h
            //   will need corp methods to determine member access rights for item location and roles....
            //   these may be written already.  will have to check

            accountKey = call.client->GetCorpAccountKey();

        } else {

        }

        //is this standing order or immediate?
        if (args.duration == 0) {
            // immediate.  look for open sell order that matches all reqs (price, qty, distance, etc)
            // check distance shit, set order range and make station list.  this shit will be nuts.
            uint32 orderID = m_db.FindSellOrder(args);
            if (orderID) {
                // found one.
                _log(MARKET__TRACE, "PlaceCharOrder - Found sell order #%u in %s for %s. (type %i, price %.2f, qty %i, range %i)", \
                        orderID, stDataMgr.GetStationName(args.stationID).c_str(), call.client->GetName(), args.typeID, args.price, args.quantity, args.orderRange);

                sMktMgr.ExecuteSellOrder(call.client, orderID, args);
                return nullptr;
            }

            _log(MARKET__TRACE, "PlaceCharOrder - Failed to satisfy buy order for %i of type %i at %.2f ISK.", \
                    args.quantity, args.typeID, args.price);
            call.client->SendErrorMsg("No sell order found.");  // find/implement type name here
            return nullptr;
        }

        // determine escrow amount
        float money(args.price * args.quantity);

        // set save data
        Market::SaveData data = Market::SaveData();
        data.accountKey         = accountKey;
        data.minVolume          = 1; // verify this
        data.issued             = GetFileTimeNow();
        // fill items from sent data
        data.bid                = args.bid;
        data.isCorp             = args.useCorp;
        data.typeID             = args.typeID;
        data.orderRange         = args.orderRange;
        data.ownerID            = args.useCorp?call.client->GetCorporationID():call.client->GetCharacterID();
        data.solarSystemID      = sDataMgr.GetStationSystem(args.stationID);
        data.regionID           = sDataMgr.GetStationRegion(args.stationID);
        data.stationID          = args.stationID;
        data.price              = args.price;
        data.volEntered         = args.quantity;
        data.volRemaining       = args.quantity;
        data.duration           = args.duration;
        data.memberID           = args.useCorp?call.client->GetCharacterID():0;
        data.escrow             = money;

        // these need a bit more data
        data.contraband = false;   // does this need to check region/system?  yes
        data.jumps = 1;     // not sure if this is used....

        // create buy order
        uint32 orderID(m_db.StoreOrder(data));
        if (orderID == 0) {
            _log(MARKET__ERROR, "PlaceCharOrder - Failed to record buy order in the DB.");
            call.client->SendErrorMsg("Failed to record the order.");
            return nullptr;
        }

        std::string reason = "DESC:  Setting up buy order in ";
        reason += stDataMgr.GetStationName(args.stationID).c_str();
        // get data for computing broker fees
        uint8 lvl(call.client->GetChar()->GetSkillLevel(EvESkill::BrokerRelations));
        //call.client->GetChar()->GetStandingModified();
        uint32 stationOwnerID = stDataMgr.GetOwnerID (call.client->GetStationID ());
        float ownerStanding = StandingDB::GetStanding (stationOwnerID, call.client->GetCharacterID ());
        float factionStanding = 0.0f;

        if (IsNPCCorp (stationOwnerID))
            factionStanding = StandingDB::GetStanding(sDataMgr.GetCorpFaction (stationOwnerID), call.client->GetCharacterID());

        float fee = EvEMath::Market::BrokerFee(lvl, factionStanding, ownerStanding, money);
        _log(MARKET__DEBUG, "PlaceCharOrder(buy) - %s: Escrow: %.2f, Fee: %.2f", args.useCorp?"Corp":"Player", money, fee);
        // take monies and record actions
        if (args.useCorp) {
            AccountService::TranserFunds(call.client->GetCorporationID(), stDataMgr.GetOwnerID(args.stationID), fee, \
                reason.c_str(), Journal::EntryType::Brokerfee, orderID, accountKey, Account::KeyType::Cash, call.client);
            AccountService::TranserFunds(call.client->GetCorporationID(), stDataMgr.GetOwnerID(args.stationID), money, \
                reason.c_str(), Journal::EntryType::MarketEscrow, orderID, accountKey, Account::KeyType::Escrow, call.client);
        } else {
            AccountService::TranserFunds(call.client->GetCharacterID(), stDataMgr.GetOwnerID(args.stationID), fee, \
                reason.c_str(), Journal::EntryType::Brokerfee, orderID, accountKey);
            AccountService::TranserFunds(call.client->GetCharacterID(), stDataMgr.GetOwnerID(args.stationID), money, \
                reason.c_str(), Journal::EntryType::MarketEscrow, orderID, accountKey, Account::KeyType::Escrow);
        }

        //send notification of new order...
        sMktMgr.InvalidateOrdersCache(call.client->GetRegionID(), args.typeID);
        sMktMgr.SendOnOwnOrderChanged(call.client, orderID, Market::Action::Add, args.useCorp);
    } else {
        //sell order
        if (!args.located->IsNone()) {
            //  corp item in corp hangar
            // located = [officeFolderID, officeID] or None
            PyTuple* located = args.located->AsTuple();


        }

        //verify that they actually have the item in the quantity specified...
        InventoryItemRef iRef = sItemFactory.GetItem( args.itemID );
        if (iRef.get() == nullptr) {
            _log(ITEM__ERROR, "PlaceCharOrder - Failed to find item %i for sell order.", args.itemID);
            call.client->SendErrorMsg("Unable to find item to sell.");
            return nullptr;
        }

        if (iRef->typeID() != args.typeID) {
            _log(MARKET__MESSAGE, "PlaceCharOrder - Denying Sell of typeID %u using typeID %i.", call.client->GetName(), iRef->itemID(), iRef->typeID(), args.typeID);
            call.client->SendErrorMsg("Invalid sell order item type.");
            return nullptr;
        }

        if (iRef->quantity() < args.quantity) {
            // trying to sell more than they have
            _log(MARKET__MESSAGE, "PlaceCharOrder - Denying inflated qty for %s", call.client->GetName());
            call.client->SendErrorMsg("You cannot sell %i %s when you only have %i.  If applicable, merge stacks and try again.", \
                args.quantity, iRef->name(), iRef->quantity());
            return nullptr;
        }
        //verify right to sell this thing..
        if (args.useCorp) {
            if (!IsPlayerCorp(call.client->GetCorporationID())) {
                // cant sell npc corp items...
                call.client->SendErrorMsg("You cannot sell items for an NPC corp.");
                return nullptr;
            }
            // this is a corp transaction.  verify char can sell corp shit...
            // some corp error msgs in inventory.h, corp.h and market.h
            //   will need corp methods to determine member access rights for item location and roles....
            //   these may be written already.  will have to check
            accountKey = call.client->GetCorpAccountKey();
        } else {
            if ( iRef->ownerID() != call.client->GetCharacterID()) {
                _log(MARKET__WARNING, "%s(%u) Tried to sell %i %s owned by %u in %s.", \
                        call.client->GetName(), call.client->GetCharID(), iRef->quantity(), \
                        iRef->name(), iRef->ownerID(), stDataMgr.GetStationName(args.stationID).c_str());
                call.client->SendErrorMsg("You cannot sell items you do not own.");
                return nullptr;
            }
        }

        //verify valid location
        if (( iRef->locationID() != args.stationID)   //item in station hanger
        and !(call.client->GetShip()->GetMyInventory()->ContainsItem( iRef->itemID() )  //item is in our ship
        and call.client->GetStationID() == args.stationID ))   //and our ship is in the station
        {
            std::string itemLoc;
            if (IsStation(iRef->locationID())) {
                itemLoc = stDataMgr.GetStationName(iRef->locationID());
            } else if (IsSolarSystem(iRef->locationID())) {
                itemLoc = sDataMgr.GetSystemName(iRef->locationID());
            } else {
                itemLoc = "an Invalid Location";
            }
            _log(MARKET__ERROR, "%s Trying to sell %s(%u) in %s through %s while in %s", \
                    call.client->GetName(), iRef->name(), iRef->itemID(), itemLoc.c_str(), \
                    stDataMgr.GetStationName(args.stationID).c_str(), stDataMgr.GetStationName(call.client->GetStationID()).c_str());
            call.client->SendErrorMsg("You cannot sell %s from %s while you are in %s.  Locations mismatch", \
                    iRef->name(), stDataMgr.GetStationName(args.stationID).c_str(), stDataMgr.GetStationName(call.client->GetStationID()).c_str());
            return nullptr;
        }

        //TODO: verify orderRange against their skills.   client may do this...verify

        // they are allowed to sell this thing...

        //is this standing order or immediate?
        if (args.duration == 0) {
            // immediate - loop to search and fill buy orders at or above asking price until qty depleted or no orders found
            bool search(true);
            uint32 orderID(0), origQty(args.quantity);
            while (args.quantity and search) {
                orderID = m_db.FindBuyOrder(args);
                if (orderID) {
                    _log(MARKET__TRACE, "PlaceCharOrder - Found buy order #%u in %s for %s.", \
                            orderID, stDataMgr.GetStationName(args.stationID).c_str(), call.client->GetName());
                    search = sMktMgr.ExecuteBuyOrder(call.client, orderID, iRef, args);
                }
            }

            // test for qty change for correct msg.
            if (args.quantity == 0) {
                // completely fulfilled.  msgs sent from ExecuteBuyOrder()
                return nullptr;
            } else if (args.quantity == origQty) {
                //unable to find any order for this item using client parameters
                // find/implement type name here?
                _log(MARKET__TRACE, "PlaceCharOrder - Failed to find any buy order for type %i at %.2f ISK.", \
                        args.typeID, args.price);
                call.client->SendErrorMsg("No buy order found.");
                return nullptr;
            } else {
                // partially filled
                _log(MARKET__TRACE, "PlaceCharOrder - Failed to find buy orders for remaining %i of type %i at %.2f ISK.", \
                        args.quantity, args.typeID, args.price);
                call.client->SendErrorMsg("There were only buyers for %u of the %i items you wanted to sell.", args.quantity, origQty);
                return nullptr;
            }

            _log(MARKET__ERROR, "PlaceCharOrder - immediate order qty hit end of conditional.");\
            return nullptr;
        }

        // they will be placing a sell order.

        // set save data
        Market::SaveData data = Market::SaveData();
        data.accountKey         = accountKey;
        data.minVolume          = 1; // verify this
        data.issued             = GetFileTimeNow();
        // fill items from sent data
        data.bid                = args.bid;
        data.isCorp             = args.useCorp;
        data.typeID             = args.typeID;
        data.orderRange         = args.orderRange;
        data.ownerID            = iRef->ownerID();
        data.solarSystemID      = sDataMgr.GetStationSystem(args.stationID);
        data.regionID           = sDataMgr.GetStationRegion(args.stationID);
        data.stationID          = args.stationID;
        data.price              = args.price;
        data.volEntered         = args.quantity;
        data.volRemaining       = args.quantity;
        data.duration           = args.duration;
        data.memberID           = args.useCorp?call.client->GetCharacterID():0;

        // check seller's permissions on the wallet if it's for corp
        if (data.isCorp) {
            int64 corpRole = call.client->GetCorpRole ();

            // make sure the user has permissions to take money from the corporation account
            if (
                    (accountKey == 1000 && (corpRole & Corp::Role::AccountCanTake1) == 0) ||
                    (accountKey == 1001 && (corpRole & Corp::Role::AccountCanTake2) == 0) ||
                    (accountKey == 1002 && (corpRole & Corp::Role::AccountCanTake3) == 0) ||
                    (accountKey == 1003 && (corpRole & Corp::Role::AccountCanTake4) == 0) ||
                    (accountKey == 1004 && (corpRole & Corp::Role::AccountCanTake5) == 0) ||
                    (accountKey == 1005 && (corpRole & Corp::Role::AccountCanTake6) == 0) ||
                    (accountKey == 1006 && (corpRole & Corp::Role::AccountCanTake7) == 0)
                    )
                throw UserError("CrpAccessDenied").AddFormatValue ("reason", new PyString ("You do not have access to that wallet"));
        }

        // these need a bit more data
        data.contraband = iRef->contraband();   // does this need to check region/system?
        data.jumps = 1;     // not sure if this is used....

        // calculate total for broker fees
        float total = args.price * args.quantity;
        std::string reason = "DESC:  Setting up sell order in ";
        reason += stDataMgr.GetStationName(args.stationID).c_str();
        // get data for computing broker fees
        uint8 lvl = call.client->GetChar()->GetSkillLevel(EvESkill::BrokerRelations);
        //call.client->GetChar()->GetStandingModified();
        uint32 stationOwnerID = stDataMgr.GetOwnerID (call.client->GetStationID ());
        float ownerStanding = StandingDB::GetStanding (stationOwnerID, call.client->GetCharacterID ());
        float factionStanding = 0.0f;

        if (IsNPCCorp (stationOwnerID))
            factionStanding = StandingDB::GetStanding(sDataMgr.GetCorpFaction (stationOwnerID), call.client->GetCharacterID());

        float fee = EvEMath::Market::BrokerFee(lvl, factionStanding, ownerStanding, total);
        _log(MARKET__DEBUG, "PlaceCharOrder(sell) - %s: Total: %.2f, Fee: %.2f", args.useCorp?"Corp":"Player", total, fee);

        // take monies and record actions (taxes are paid when item sells)
        if (args.useCorp) {
            AccountService::TranserFunds(call.client->GetCorporationID(), stDataMgr.GetOwnerID(args.stationID), fee, \
                    reason.c_str(), Journal::EntryType::Brokerfee, 0, accountKey, Account::KeyType::Cash, call.client);
        } else {
            AccountService::TranserFunds(call.client->GetCharacterID(), stDataMgr.GetOwnerID(args.stationID), fee, \
                    reason.c_str(), Journal::EntryType::Brokerfee, 0, accountKey);
        }

        //store the order in the DB.
        uint32 orderID = m_db.StoreOrder(data);
        if (orderID == 0) {
            _log(MARKET__ERROR, "PlaceCharOrder - Failed to record sell order in the DB.");
            call.client->SendErrorMsg("Failed to record the order in the DB!");
            return nullptr;
        }

        if (iRef->quantity() == args.quantity) {
            //take item from seller
            call.client->SystemMgr()->RemoveItemFromInventory(iRef);
            iRef->Delete();
        } else {
            //update the item.
            if (!iRef->AlterQuantity(-args.quantity, true)) {
                _log(MARKET__ERROR, "PlaceCharOrder - Failed to consume %i units from %s", args.quantity, iRef->name());
                return nullptr;
            }
        }

        //notify client about new order.
        sMktMgr.InvalidateOrdersCache(call.client->GetRegionID(), args.typeID);
        sMktMgr.SendOnOwnOrderChanged(call.client, orderID, Market::Action::Add, args.useCorp);
    }

    //returns nothing.
    return nullptr;
}

PyResult MarketProxyService::Handle_ModifyCharOrder(PyCallArgs &call) {
    Call_ModifyCharOrder args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // client coded to throw error if price > 9223372036854.0
    // we need to pull data from db for typeID and isCorp...
    Market::OrderInfo oInfo = Market::OrderInfo();
    if (!m_db.GetOrderInfo(args.orderID, oInfo)) {
        _log(MARKET__ERROR, "ModifyCharOrder - Failed to get info about order #%u.", args.orderID);
        return nullptr;
    }

    // there is no refund in broker fees.

    // adjust balance for price change
    float money = (args.price - args.newPrice) * args.volRemaining;
    std::string reason = "DESC:  Altering Market Order #";
    reason += std::to_string(args.orderID);
    AccountService::TranserFunds(call.client->GetCharID(), stDataMgr.GetOwnerID(args.stationID), money,
                        reason.c_str(), Journal::EntryType::MarketEscrow, args.orderID,
                        Account::KeyType::Cash, Account::KeyType::Escrow);

    if (!m_db.AlterOrderPrice(args.orderID, args.newPrice)) {
        _log(MARKET__ERROR, "ModifyCharOrder - Failed to modify price for order #%u.", call.client->GetName(), args.orderID);
        return nullptr;
    }

    sMktMgr.InvalidateOrdersCache(call.client->GetRegionID(), oInfo.typeID);
    sMktMgr.SendOnOwnOrderChanged(call.client, args.orderID, Market::Action::Modify, oInfo.isCorp);

    return nullptr;
}

PyResult MarketProxyService::Handle_CancelCharOrder(PyCallArgs &call) {
    Call_CancelCharOrder args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Market::OrderInfo oInfo = Market::OrderInfo();
    if (!m_db.GetOrderInfo(args.orderID, oInfo)) {
        _log(MARKET__ERROR, "CancelCharOrder - Failed to get info about order #%u.", call.client->GetName(), args.orderID);
        return nullptr;
    }

    if (oInfo.isBuy) {
        // buy order only refunds escrow
        float money = oInfo.price * oInfo.quantity;
        // send wallet blink event and record the transaction in their journal.
        std::string reason = "DESC:  Canceling Market Order #";
        reason += std::to_string(args.orderID);
        AccountService::TranserFunds(stDataMgr.GetOwnerID(oInfo.stationID), call.client->GetCharID(), money,
                                     reason.c_str(), Journal::EntryType::MarketEscrow, args.orderID,
                                     Account::KeyType::Escrow, Account::KeyType::Cash);
    } else {
        ItemData idata(oInfo.typeID, ownerStation, locTemp, flagHangar, oInfo.quantity);
        InventoryItemRef iRef = sItemFactory.SpawnItem(idata);
        if (iRef.get() != nullptr)
            iRef->Donate(call.client->GetCharacterID(), oInfo.stationID, flagHangar, true);
    }

    PyRep* order(m_db.GetOrderRow(args.orderID));
    if (!m_db.DeleteOrder(args.orderID)) {
        _log(MARKET__ERROR, "CancelCharOrder - Failed to delete order #%u.", args.orderID);
        return nullptr;
    }

    sMktMgr.InvalidateOrdersCache(call.client->GetRegionID(), oInfo.typeID);
    sMktMgr.SendOnOwnOrderChanged(call.client, args.orderID, Market::Action::Expiry, oInfo.isCorp, order);

    return nullptr;
}

