/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
*/

#include "EVEServerPCH.h"


PyCallable_Make_InnerDispatcher(MarketProxyService)



/*
class MarketProxyBound
: public PyBoundObject {
public:

    PyCallable_Make_Dispatcher(MarketProxyBound)

    MarketProxyBound(PyServiceMgr *mgr, MarketProxyDB *db)
    : PyBoundObject(mgr, "MarketProxyBound"),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(MarketProxyBound, )
        PyCallable_REG_CALL(MarketProxyBound, )
    }
    virtual ~MarketProxyBound() { delete m_dispatch; }
    virtual void DecRef() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL()
    PyCallable_DECL_CALL()

protected:
    MarketProxyDB *const m_db;
    Dispatcher *const m_dispatch;   //we own this
};
*/


MarketProxyService::MarketProxyService(PyServiceMgr *mgr)
: PyService(mgr, "marketProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MarketProxyService, GetStationAsks)
    PyCallable_REG_CALL(MarketProxyService, GetSystemAsks)
    PyCallable_REG_CALL(MarketProxyService, GetRegionBest)
    PyCallable_REG_CALL(MarketProxyService, GetMarketGroups)
    PyCallable_REG_CALL(MarketProxyService, GetOrders)
    PyCallable_REG_CALL(MarketProxyService, GetOldPriceHistory)
    PyCallable_REG_CALL(MarketProxyService, GetNewPriceHistory)
    PyCallable_REG_CALL(MarketProxyService, PlaceCharOrder)
    PyCallable_REG_CALL(MarketProxyService, GetCharOrders)
    PyCallable_REG_CALL(MarketProxyService, ModifyCharOrder)
    PyCallable_REG_CALL(MarketProxyService, CancelCharOrder)
    PyCallable_REG_CALL(MarketProxyService, CharGetNewTransactions)
    PyCallable_REG_CALL(MarketProxyService, StartupCheck)
}

MarketProxyService::~MarketProxyService() {
    delete m_dispatch;
}


/*
PyBoundObject *MarketProxyService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    _log(MARKET__MESSAGE, "MarketProxyService bind request for:");
    bind_args->Dump(MARKET__MESSAGE, "    ");

    return(new MarketProxyBound(m_manager, &m_db));
}*/


PyResult MarketProxyService::Handle_GetStationAsks(PyCallArgs &call) {
    PyRep *result = NULL;

    uint32 locid = call.client->GetLocationID();
    if(!IsStation(locid)) {
        _log(SERVICE__ERROR, "%s: Requested StationAsks when in non-station location %u", call.client->GetName(), locid);
        return NULL;
    }
    result = m_db.GetStationAsks(locid);
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load StationAsks for location %u", call.client->GetName(), locid);
        return NULL;
    }

    return result;
}


PyResult MarketProxyService::Handle_GetSystemAsks(PyCallArgs &call) {
    PyRep *result = NULL;

    uint32 locid = call.client->GetSystemID();
    if(!IsSolarSystem(locid)) {
        codelog(SERVICE__ERROR, "%s: GetSystemID() returned a non-system %u!", call.client->GetName(), locid);
        return NULL;
    }
    result = m_db.GetSystemAsks(locid);
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load SystemAsks for location %u", call.client->GetName(), locid);
        return NULL;
    }

    return result;
}


PyResult MarketProxyService::Handle_GetRegionBest(PyCallArgs &call) {
    PyRep *result = NULL;

    uint32 locid = call.client->GetSystemID();
    if(!IsSolarSystem(locid)) {
        codelog(SERVICE__ERROR, "%s: GetSystemID() returned a non-system %u!", call.client->GetName(), locid);
        return NULL;
    }

    uint32 regionID;
    if(!m_db.GetSystemInfo(locid, NULL, &regionID, NULL, NULL)) {
        codelog(SERVICE__ERROR, "%s: Failed to find parents of system %u!", call.client->GetName(), locid);
        return NULL;
    }

    result = m_db.GetRegionBest(regionID);
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load GetRegionBest for region %u", call.client->GetName(), regionID);
        return NULL;
    }

    return result;
}

PyResult MarketProxyService::Handle_GetMarketGroups(PyCallArgs &call) {
    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetMarketGroups");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetMarketGroups();
        if(result == NULL) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            result = new PyNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

PyResult MarketProxyService::Handle_GetOrders(PyCallArgs &call) {
    Call_SingleIntegerArg args; //itemID
    if(!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return NULL;
    }

    /*PyRep *result = NULL;

    uint32 locid = call.client->GetSystemID();
    if(!IsSolarSystem(locid)) {
        codelog(SERVICE__ERROR, "%s: GetSystemID() returned a non-system %u!", call.client->GetName(), locid);
        return NULL;
    }

    uint32 regionID;
    if(!m_db.GetSystemInfo(locid, NULL, &regionID, NULL, NULL)) {
        codelog(SERVICE__ERROR, "%s: Failed to find parents of system %u!", call.client->GetName(), locid);
        return NULL;
    }

    result = m_db.GetOrders(regionID, args.arg);
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load GetOrders for item %u of region %u", call.client->GetName(), args.arg, regionID);
        return NULL;
    }

    return result;*/
    PyRep *result = NULL;

    std::string method_name ("GetOrders_");
    method_name += itoa(args.arg);
    ObjectCachedMethodID method_id(GetName(), method_name.c_str());
    #ifndef WIN32
    #warning TODO: temporary solution, make cache objects with arguments
    #endif


    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id))
    {
        //this method is not in cache yet, load up the contents and cache it.
        uint32 locid = call.client->GetSystemID();
        if(!IsSolarSystem(locid))
        {
            codelog(SERVICE__ERROR, "%s: GetSystemID() returned a non-system %u!", call.client->GetName(), locid);
            return NULL;
        }

        uint32 regionID;
        if(!m_db.GetSystemInfo(locid, NULL, &regionID, NULL, NULL))
        {
            codelog(SERVICE__ERROR, "%s: Failed to find parents of system %u!", call.client->GetName(), locid);
            return NULL;
        }

        result = m_db.GetOrders(regionID, args.arg);
        if(result == NULL) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            result = new PyNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

PyResult MarketProxyService::Handle_GetCharOrders(PyCallArgs &call) {
    //no arguments
    PyRep *result = NULL;

    result = m_db.GetCharOrders(call.client->GetCharacterID());
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load GetCharOrders", call.client->GetName());
        return NULL;
    }

    return result;
}

PyResult MarketProxyService::Handle_GetOldPriceHistory(PyCallArgs &call) {
    Call_SingleIntegerArg args; //itemID
    if(!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return NULL;
    }

    PyRep *result = NULL;

    uint32 locid = call.client->GetSystemID();
    if(!IsSolarSystem(locid)) {
        codelog(SERVICE__ERROR, "%s: GetSystemID() returned a non-system %u!", call.client->GetName(), locid);
        return NULL;
    }

    uint32 regionID;
    if(!m_db.GetSystemInfo(locid, NULL, &regionID, NULL, NULL)) {
        codelog(SERVICE__ERROR, "%s: Failed to find parents of system %u!", call.client->GetName(), locid);
        return NULL;
    }

    result = m_db.GetOldPriceHistory(regionID, args.arg);
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load Old Price History for item %u of region %u", call.client->GetName(), args.arg, regionID);
        return NULL;
    }

    return result;
}

PyResult MarketProxyService::Handle_GetNewPriceHistory(PyCallArgs &call) {
    Call_SingleIntegerArg args; //itemID
    if(!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return NULL;
    }

    PyRep *result = NULL;

    uint32 locid = call.client->GetSystemID();
    if(!IsSolarSystem(locid)) {
        codelog(SERVICE__ERROR, "%s: GetSystemID() returned a non-system %u!", call.client->GetName(), locid);
        return NULL;
    }

    uint32 regionID;
    if(!m_db.GetSystemInfo(locid, NULL, &regionID, NULL, NULL)) {
        codelog(SERVICE__ERROR, "%s: Failed to find parents of system %u!", call.client->GetName(), locid);
        return NULL;
    }

    result = m_db.GetNewPriceHistory(regionID, args.arg);
    if(result == NULL) {
        _log(SERVICE__ERROR, "%s: Failed to load New Price History for item %u of region %u", call.client->GetName(), args.arg, regionID);
        return NULL;
    }

    return result;
}

PyResult MarketProxyService::Handle_PlaceCharOrder(PyCallArgs &call) {
    Call_PlaceCharOrder args;
    if(!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return NULL;
    }

    if(args.bid) {
        //buy order

        //TODO: do something with args.itemID

        //try to satisfy immediately...
        uint32 order_id = m_db.FindSellOrder(
            args.stationID,
            args.typeID,
            args.price,
            args.quantity,
            args.orderRange);
        if(order_id != 0) {
            _log(MARKET__TRACE, "%s: Found sell order %u to satisfy (type %u, station %u, price %f, qty %u, range %u)", call.client->GetName(), order_id, args.stationID, args.typeID, args.price, args.quantity, args.orderRange);

            _ExecuteSellOrder(order_id, args.stationID, args.quantity, call.client, args.useCorp);
            return NULL;
        }

        //unable to satisfy immediately...
        if(args.duration == 0) {
            _log(MARKET__ERROR, "%s: Failed to satisfy order for %d of %d at %f ISK.", call.client->GetName(), args.typeID, args.quantity, args.price);
            call.client->SendErrorMsg("No such order found.");
            return NULL;
        }

        //TODO: verify the validity of args.stationID (range vs. skill)
        //TODO: handle located?
        //NOTE: I am not sure that useCorp is as simple as it is currently implemented...

        //make sure they can afford this, and take the money if they can.
        double money = args.price * args.quantity;
        //TODO: add broker fees...
        if(!call.client->AddBalance(-money)) {
            _log(MARKET__ERROR, "%s: Client requested buy order exceeding their balance (%f ISK total).", call.client->GetName(), money);
            call.client->SendErrorMsg("No such order found.");
            return NULL;
        }

        //store the order in the DB.
        uint32 orderID = m_db.StoreBuyOrder(
            call.client->GetCharacterID(),
            call.client->GetAccountID(),
            args.stationID,
            args.typeID,
            args.price,
            args.quantity,
            args.orderRange,
            args.minVolume,
            args.duration,
            args.useCorp);
        if(orderID == 0) {
            codelog(MARKET__ERROR, "%s: Failed to record order in the DB.", call.client->GetName());
            call.client->SendErrorMsg("Failed to record the order in the DB!");
            return NULL;
        }

        //send notification of new order...
        _InvalidateOrdersCache(args.typeID);
        _BroadcastOnOwnOrderChanged(call.client->GetRegionID(), orderID, "Add", args.useCorp);
    } else {
        //sell order

        //verify that they actually have the item in the quantity specified...
        InventoryItemRef item = m_manager->item_factory.GetItem( args.itemID );
        if( !item ) {
            codelog(MARKET__ERROR, "%s: Failed to find item %d for sell order.", call.client->GetName(), args.itemID);
            call.client->SendErrorMsg("Unable to find items %d to sell!", args.itemID);
            return NULL;
        }
        //verify right to sell this thing..
        //TODO: this should be a much more complicated check with corp stuff....
        if(item->ownerID() != call.client->GetCharacterID()) {
            codelog(MARKET__ERROR, "%s: Char %d Tried to sell item %d with owner %d.", call.client->GetName(), call.client->GetCharacterID(), item->itemID(), item->ownerID());
            call.client->SendErrorMsg("You cannot sell items you do not own.");
            return NULL;
        }

        //verify that they specified a valid station ID to sell from.
        if(
           (item->locationID() != (uint32)args.stationID)   //item in station hanger
           && !(
                call.client->GetShip()->Contains( item->itemID() )  //item is in our ship
                && call.client->GetStationID() == (uint32)args.stationID    //and our ship is in the station
                )
        ) {
            codelog(MARKET__ERROR, "%s: Tried to sell item %d which is in location %d through station %d while in station %d", call.client->GetName(), item->itemID(), item->locationID(), args.stationID, call.client->GetStationID());
            call.client->SendErrorMsg("You cannot sell that item in that station.");
            return NULL;
        }

        if((item->singleton() && args.quantity != 1)
           || item->quantity() < (uint32)args.quantity ) {
            codelog(MARKET__ERROR, "%s: Tried to sell %d of item %d which has qty %d singleton %d", call.client->GetName(), args.quantity, item->itemID(), item->quantity(), item->singleton());
            call.client->SendErrorMsg("You cannot sell more than you have.");
            return NULL;
        }

        if(item->typeID() != (uint32)args.typeID) {
            codelog(MARKET__ERROR, "%s: Tried to sell item %d of type %d using type ID %d", call.client->GetName(), item->itemID(), item->typeID(), args.typeID);
            call.client->SendErrorMsg("Invalid sell order item type.");
            return NULL;
        }

        //TODO: verify orderRange against their skills.

        //ok, we think they are allowed to sell this thing...

        //try to satisfy immediately...
        uint32 order_id = m_db.FindBuyOrder(
            args.stationID,
            args.typeID,
            args.price,
            args.quantity,
            args.orderRange);
        if(order_id != 0) {
            _log(MARKET__TRACE, "%s: Found order %u to satisfy (type %u, station %u, price %f, qty %u, range %u)", call.client->GetName(), order_id, args.stationID, args.typeID, args.price, args.quantity, args.orderRange);

            _ExecuteBuyOrder(order_id, args.stationID, args.quantity, call.client, (InventoryItemRef)item, args.useCorp);
            return NULL;
        }

        //else, unable to satisfy immediately...
        _log(MARKET__TRACE, "%s: Unable to find an immediate order to satisfy (type %u, station %u, price %f, qty %u, range %u)", call.client->GetName(), args.stationID, args.typeID, args.price, args.quantity, args.orderRange);

        if(args.duration == 0) {
            _log(MARKET__ERROR, "%s: Failed to satisfy order for %d of %d at %f ISK.", call.client->GetName(), args.typeID, args.quantity, args.price);
            return NULL;
        }

        //TODO: take broker cost.

        //take item from seller
        if(item->quantity() == (uint32)args.quantity) {
            item->Delete();
        } else {
            //update the item.
            if(!item->AlterQuantity(-int32(args.quantity), true)) {
                codelog(MARKET__ERROR, "%s: Failed to consume %u units from item %u", call.client->GetName(), args.quantity, item->itemID());
                return NULL;
            }
        }

        //store the order in the DB.
        uint32 orderID = m_db.StoreSellOrder(
            call.client->GetCharacterID(),
            call.client->GetAccountID(),
            args.stationID,
            args.typeID,
            args.price,
            args.quantity,
            args.orderRange,
            args.minVolume,
            args.duration,
            args.useCorp);
        if(orderID == 0) {
            codelog(MARKET__ERROR, "%s: Failed to record order in the DB.", call.client->GetName());
            call.client->SendErrorMsg("Failed to record the order in the DB!");
            return NULL;
        }

        //notify client about new order.
        _InvalidateOrdersCache(args.typeID);
        _BroadcastOnOwnOrderChanged(call.client->GetRegionID(), orderID, "Add", args.useCorp);
    }

    //returns nothing.
    return NULL;
}

PyResult MarketProxyService::Handle_ModifyCharOrder(PyCallArgs &call) {
    Call_ModifyCharOrder args;
    if(!args.Decode(&call.tuple))
    {
        codelog(MARKET__ERROR, "Invalid arguments");
        return NULL;
    }

    uint32 typeID = 0;
    uint32 quantity = 0;
    double price = 0;
    bool isBuy = false;
    bool isCorp = false;

    if(!m_db.GetOrderInfo(args.orderID, NULL, &typeID, NULL, &quantity, &price, &isBuy, &isCorp)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about order %u.", call.client->GetName(), args.orderID);
        return NULL;
    }

    if(price == args.new_price)
        return NULL;
    
    if(isBuy)
    {
        double money = (price - args.new_price) * quantity;
        if(!call.client->AddBalance(money))
            return NULL;
    }

    if(!m_db.AlterOrderPrice(args.orderID, args.new_price)) {
        codelog(MARKET__ERROR, "%s: Failed to modify price for order %u.", call.client->GetName(), args.orderID);
        return NULL;
    }

    _InvalidateOrdersCache(typeID);
    _BroadcastOnOwnOrderChanged(call.client->GetRegionID(), args.orderID, "Modify", isCorp); //force a refresh of market data.
    
    return NULL;
}

PyResult MarketProxyService::Handle_CancelCharOrder(PyCallArgs &call) {
    Call_CancelCharOrder args;
    if(!args.Decode(&call.tuple))
    {
        codelog(MARKET__ERROR, "Invalid arguments");
        return NULL;
    }

    uint32 ownerID = 0;
    uint32 typeID = 0;
    uint32 stationID = 0;
    uint32 quantity = 0;
    double price = 0;
    bool isBuy = false;
    bool isCorp = false;
    
    if(!m_db.GetOrderInfo(args.orderID, &ownerID, &typeID, &stationID, &quantity, &price, &isBuy, &isCorp)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about order %u.", call.client->GetName(), args.orderID);
        return NULL;
    }

    ItemData idata(
        typeID,
        1, //temp owner ID, should really put the seller's ID in here...
        stationID,
        flagHangar,
        quantity
    );

    if(isBuy)
    {
        double money = price * quantity;
        if(!call.client->AddBalance(money))
            return NULL;
    }
    else
    {
        InventoryItemRef new_item = m_manager->item_factory.SpawnItem(idata);
        //use the owner change packet to alert the buyer of the new item
        new_item->ChangeOwner(call.client->GetCharacterID(), true);
    }

    PyRep* order = m_db.GetOrderRow(args.orderID);
    if(!m_db.DeleteOrder(args.orderID))
    {
        codelog(MARKET__ERROR, "Failed to delete order %u.", args.orderID);
        return NULL;
    }
    _InvalidateOrdersCache(typeID);
    _BroadcastOnOwnOrderChanged(call.client->GetRegionID(), args.orderID, "Expiry", isCorp, order); //force a refresh of market data.
    _BroadcastOnMarketRefresh(call.client->GetRegionID());

    return NULL;
}

PyResult MarketProxyService::Handle_CharGetNewTransactions(PyCallArgs &call)
{
	PyRep *result = NULL;
	Call_CharGetNewTransactions args;
	if(!args.Decode(&call.tuple))
	{
		codelog(MARKET__ERROR, "Invalid arguments");
		return NULL;
	}

	double minPrice;
	if(args.minPrice->IsInt())
		minPrice = args.minPrice->AsInt()->value();
	else if(args.minPrice->IsFloat())
		minPrice = args.minPrice->AsFloat()->value();
	else
	{
		codelog(CLIENT__ERROR, "%s: Invalid type %s for minPrice argument received.", call.client->GetName(), args.minPrice->TypeString());
		return NULL;
	}

	result = m_db.GetTransactions(args.clientID==0?call.client->GetCharacterID():args.clientID,
			args.typeID, args.quantity, minPrice, args.maxPrice, args.fromDate, args.buySell);
	if(result == NULL)
	{
		_log(SERVICE__ERROR, "%s: Failed to load CharGetNewTransactions", call.client->GetName());
		return NULL;
	}

	return result;
}

PyResult MarketProxyService::Handle_StartupCheck(PyCallArgs &call)
{
	//Don't have a clue what this is supposed to do.  If you figure it out, feel free to fill it in :)

	return NULL;
}

void MarketProxyService::_SendOnOwnOrderChanged(Client *who, uint32 orderID, const char *action, bool isCorp, PyRep* order) {
    Notify_OnOwnOrderChanged ooc;
    if(order != NULL)
        ooc.order = order;
    else
        ooc.order = m_db.GetOrderRow(orderID);
    ooc.reason = action;
    ooc.isCorp = isCorp;
    PyTuple *tmp = ooc.Encode();
    who->SendNotification("OnOwnOrderChanged", "clientID", &tmp);   //tmp consumed.
}

void MarketProxyService::_SendOnMarketRefresh(Client *who) {
    PyTuple *tmp = new PyTuple(0);
    who->SendNotification("OnMarketRefresh", "clientID", &tmp);   //tmp consumed.
}

void MarketProxyService::_BroadcastOnOwnOrderChanged(uint32 regionID, uint32 orderID, const char *action, bool isCorp, PyRep* order) {
    std::vector<Client *> clients;
    m_manager->entity_list.FindByRegionID(regionID, clients);
    std::vector<Client *>::iterator cur, end;
	cur = clients.begin();
	end = clients.end();
	for(; cur != end; cur++) {
        PySafeIncRef(order);
		_SendOnOwnOrderChanged(*cur, orderID, action, isCorp, order);
	}
    PySafeDecRef(order);
}

void MarketProxyService::_BroadcastOnMarketRefresh(uint32 regionID) {
    std::vector<Client *> clients;
    m_manager->entity_list.FindByRegionID(regionID, clients);
    std::vector<Client *>::iterator cur, end;
	cur = clients.begin();
	end = clients.end();
	for(; cur != end; cur++) {
		_SendOnMarketRefresh(*cur);
	}
}

void MarketProxyService::_InvalidateOrdersCache(uint32 typeID)
{
    std::string method_name ("GetOrders_");
    method_name += itoa(typeID);
    ObjectCachedMethodID method_id(GetName(), method_name.c_str());
    m_manager->cache_service->InvalidateCache( method_id );
}

//NOTE: there are a lot of race conditions to deal with here if we ever
//allow multiple market services to run at the same time.
void MarketProxyService::_ExecuteBuyOrder(uint32 buy_order_id, uint32 stationID, uint32 quantity, Client *seller, InventoryItemRef item, bool isCorp) {
    uint32 orderOwnerID = 0;
    uint32 typeID = 0;
    uint32 qtyReq = 0;
    double price = 0;

    if(!m_db.GetOrderInfo(buy_order_id, &orderOwnerID, &typeID, NULL, &qtyReq, &price, NULL, NULL)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about buy order %u.", seller->GetName(), buy_order_id);
        return;
    }

    if(typeID != item->typeID()) {
        //should never happen.
        codelog(MARKET__ERROR, "%s: Type mismatch executing order %u: order %u item %u", seller->GetName(), buy_order_id, typeID, item->typeID());
        seller->SendErrorMsg("Order type mismatch.");
        return;
    }

    if(quantity > qtyReq) {
        codelog(MARKET__ERROR, "%s: Tried to sell more (%u) than %u required (%u). Selling only what required.", seller->GetName(), quantity, orderOwnerID, qtyReq);
        quantity = qtyReq;
    }

    if(orderOwnerID == item->ownerID()) {
        //I just have a bad feeling that this is not going to work very well...
        codelog(MARKET__WARNING, "%s: Selling an item to ourself... this may not work...", seller->GetName());
    }

    if(item->singleton() || item->quantity() == quantity) {
        //entire item is moving...
        if(item->locationID() != stationID) {
            //do not notify for this move, the owner change will take care of everything for us.
            //however, the resulting item change packet will not be as accurate as it could be
            // because it will not show the location change data, but this is better anyhow.
            item->Move(stationID, flagHangar, false);
        }
        item->ChangeOwner(orderOwnerID, true);
    } else {
        //need to split item up...
        InventoryItemRef new_item = item->Split(quantity, true);
        if( !new_item ) {
            codelog(MARKET__ERROR, "Failed to split item %u.", item->itemID());
            return;
        }
        //use the owner change packet to alert the buyer of the new item
        new_item->ChangeOwner(orderOwnerID, true);
    }

    //the buyer has already paid out the money before the buy order
    //was recorded in the database.
    //give the money to the seller...
    double money = price * quantity;
    //TODO: take off market overhead fees...
    seller->AddBalance(money);
    //TODO: record this in the wallet history.

    Client *buyer = m_manager->entity_list.FindCharacter(orderOwnerID);
    if(quantity == qtyReq) {
        _log(MARKET__TRACE, "%s: Completely satisfied order %u, deleting.", seller->GetName(), buy_order_id);
        PyRep* order = m_db.GetOrderRow(buy_order_id);
        if(!m_db.DeleteOrder(buy_order_id)) {
            codelog(MARKET__ERROR, "Failed to delete order %u.", buy_order_id);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(seller->GetRegionID(), buy_order_id, "Expiry", isCorp, order);
        _BroadcastOnMarketRefresh(seller->GetRegionID());
    } else {
        _log(MARKET__TRACE, "%s: Partially satisfied order %u, altering quantity to %u.", seller->GetName(), buy_order_id, qtyReq - quantity);
        if(!m_db.AlterOrderQuantity(buy_order_id, qtyReq - quantity)) {
            codelog(MARKET__ERROR, "Failed to alter quantity of order %u.", buy_order_id);
            return;
        }
       _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(seller->GetRegionID(), buy_order_id, "Modify", isCorp);
    }

    //record this transaction in market_transactions
    //NOTE: regionID may not be accurate here...
    if(!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeSell, seller->GetCharacterID(), seller->GetRegionID(), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record sale side of transaction.", seller->GetName());
    }
    if(!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeBuy, orderOwnerID, seller->GetRegionID(), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record buy side of transaction.", seller->GetName());
    }
}

//NOTE: there are a lot of race conditions to deal with here if we ever
//allow multiple market services to run at the same time.
void MarketProxyService::_ExecuteSellOrder(uint32 sell_order_id, uint32 stationID, uint32 quantity, Client *buyer, bool isCorp) {
    uint32 orderOwnerID = 0;
    uint32 typeID = 0;
    uint32 qtyAvail = 0;
    double price = 0;

    if(!m_db.GetOrderInfo(sell_order_id, &orderOwnerID, &typeID, NULL, &qtyAvail, &price, NULL, NULL)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about sell order %u.", buyer->GetName(), sell_order_id);
        return;
    }

    if(quantity > qtyAvail) {
        codelog(MARKET__ERROR, "%s: Tried to buy more (%u) than available (%u). Buying all available.", buyer->GetName(), quantity, qtyAvail);
        quantity = qtyAvail;
    }

    if(orderOwnerID == buyer->GetCharacterID()) {
        //I just have a bad feeling that this is not going to work very well...
        codelog(MARKET__WARNING, "%s: Buying an item from ourself... this may not work...", buyer->GetName());
    }

    double money = price * quantity;

    //take the money from the buyer before we spawn the item.
    if(!buyer->AddBalance(-money)) {
        codelog(MARKET__ERROR, "%s: Failed to take buyer %s (%u)'s money (%.2f ISK) for order %u", buyer->GetName(), buyer->GetName(), buyer->GetCharacterID(), money, sell_order_id);
        buyer->SendErrorMsg("You cannot afford that.");
        return;
    }

    //spawn the item in the buyer's hangar.
    ItemData idata(
        typeID,
        1, //temp owner ID, should really put the seller's ID in here...
        stationID,
        flagHangar,
        quantity
    );

    InventoryItemRef new_item = m_manager->item_factory.SpawnItem(idata);
    //use the owner change packet to alert the buyer of the new item
    new_item->ChangeOwner(buyer->GetCharacterID(), true);

    //give the money to the seller...
    //TODO: take off market overhead fees...
    Client *seller = m_manager->entity_list.FindCharacter(orderOwnerID);
    if(seller != NULL) {
        //the seller is logged in, send them a notification...
        if(!seller->AddBalance(money))
            codelog(MARKET__ERROR, "%s: Failed to give seller %s (%u) %.2f ISK from order %u", buyer->GetName(), seller->GetName(), orderOwnerID, money, sell_order_id);
    } else {
        //seller is not online right now...
        if(!m_db.AddCharacterBalance(orderOwnerID, money))
           codelog(MARKET__ERROR, "%s: Failed to give seller ID %u %.2f ISK from order %u", buyer->GetName(), orderOwnerID, money, sell_order_id);
    }

    if(quantity == qtyAvail) {
        _log(MARKET__TRACE, "%s: Completely satisfied order %u, deleting.", buyer->GetName(), sell_order_id);
        PyRep* order = m_db.GetOrderRow(sell_order_id);
        if(!m_db.DeleteOrder(sell_order_id)) {
            codelog(MARKET__ERROR, "Failed to delete order %u.", sell_order_id);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(buyer->GetRegionID(), sell_order_id, "Expiry", isCorp, order);
        _BroadcastOnMarketRefresh(buyer->GetRegionID());
    } else {
        _log(MARKET__TRACE, "%s: Partially satisfied order %u, altering quantity to %u.", buyer->GetName(), sell_order_id, qtyAvail - quantity);
        if(!m_db.AlterOrderQuantity(sell_order_id, qtyAvail - quantity)) {
            codelog(MARKET__ERROR, "Failed to alter quantity of order %u.", sell_order_id);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(buyer->GetRegionID(), sell_order_id, "Modify", isCorp);
    }

    //record this transaction in market_transactions
    //NOTE: regionID may not be accurate here...
    if(!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeSell, orderOwnerID, buyer->GetRegionID(), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record sale side of transaction.", buyer->GetName());
    }
    if(!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeBuy, buyer->GetCharacterID(), buyer->GetRegionID(), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record buy side of transaction.", buyer->GetName());
    }
}






















