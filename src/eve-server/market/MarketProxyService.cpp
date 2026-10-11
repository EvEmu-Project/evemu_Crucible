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


#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "inventory/Inventory.h"
#include "market/MarketMgr.h"
#include "market/MarketProxyService.h"
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"
#include "standing/StandingDB.h"
#include "inventory/ItemType.h"
#include <cmath>
#include <limits>
#include <memory>

/*
 * MARKET__ERROR
 * MARKET__WARNING
 * MARKET__MESSAGE
 * MARKET__DEBUG
 * MARKET__TRACE
 * MARKET__DB_ERROR
 * MARKET__DB_TRACE
 */

MarketProxyService::MarketProxyService(EVEServiceManager& mgr) :
    Service("marketProxy"),
    m_manager (mgr)
{
    m_cache = this->m_manager.Lookup <ObjCacheService>("objectCaching");

    this->Add("StartupCheck", &MarketProxyService::StartupCheck);
    this->Add("GetStationAsks", &MarketProxyService::GetStationAsks);
    this->Add("GetSystemAsks", &MarketProxyService::GetSystemAsks);
    this->Add("GetRegionBest", &MarketProxyService::GetRegionBest);
    this->Add("GetMarketGroups", &MarketProxyService::GetMarketGroups);
    this->Add("GetOrders", &MarketProxyService::GetOrders);
    this->Add("GetOldPriceHistory", &MarketProxyService::GetOldPriceHistory);
    this->Add("GetNewPriceHistory", &MarketProxyService::GetNewPriceHistory);
    this->Add("PlaceCharOrder", static_cast <PyResult (MarketProxyService::*)(PyCallArgs &call, PyInt*, PyInt*, PyFloat*, PyInt*, PyInt*, PyInt*, std::optional <PyInt*>, PyInt*, PyInt*, PyBool*, std::optional<PyRep*>)> (&MarketProxyService::PlaceCharOrder));
    this->Add("PlaceCharOrder", static_cast <PyResult (MarketProxyService::*)(PyCallArgs &call, PyInt*, PyInt*, PyFloat*, PyInt*, PyInt*, PyInt*, std::optional <PyInt*>, PyInt*, PyInt*, PyInt*, std::optional<PyRep*>)> (&MarketProxyService::PlaceCharOrder));
    this->Add("GetCharOrders", &MarketProxyService::GetCharOrders);
    this->Add("ModifyCharOrder", &MarketProxyService::ModifyCharOrder);
    this->Add("CancelCharOrder", &MarketProxyService::CancelCharOrder);
    this->Add("CharGetNewTransactions", &MarketProxyService::CharGetNewTransactions);
    this->Add("CorpGetNewTransactions", &MarketProxyService::CorpGetNewTransactions);
    this->Add("GetCorporationOrders", &MarketProxyService::GetCorporationOrders);
}

PyResult MarketProxyService::GetMarketGroups(PyCallArgs &call) {
    return sMktMgr.GetMarketGroups();
}

PyResult MarketProxyService::StartupCheck(PyCallArgs &call) {
    //if (sMktMgr.NeedsUpdate())
    //    sMktMgr.UpdatePriceHistory();
    return nullptr;
}

PyResult MarketProxyService::GetCharOrders(PyCallArgs &call) {
    return MarketDB::GetOrdersForOwner(call.client->GetCharacterID());
}

PyResult MarketProxyService::GetCorporationOrders(PyCallArgs &call) {
    return MarketDB::GetOrdersForOwner(call.client->GetCorporationID());
}

/** @todo update these to use market manager and cache instead of hitting db? */
// station, system, region based on selection in market window
PyResult MarketProxyService::GetStationAsks(PyCallArgs &call) {
    return MarketDB::GetStationAsks(call.client->GetStationID());
}

PyResult MarketProxyService::GetSystemAsks(PyCallArgs &call) {
    return MarketDB::GetSystemAsks(call.client->GetSystemID());
}

PyResult MarketProxyService::GetRegionBest(PyCallArgs &call) {
    return MarketDB::GetRegionBest(call.client->GetRegionID());
}

// this is called 3x on every market transaction
PyResult MarketProxyService::GetOldPriceHistory(PyCallArgs &call, PyInt* typeID) {
    return sMktMgr.GetOldPriceHistory(call.client->GetRegionID(), typeID->value());
}

PyResult MarketProxyService::GetNewPriceHistory(PyCallArgs& call, PyInt* typeID) {
    return sMktMgr.GetNewPriceHistory(call.client->GetRegionID(), typeID->value());
}

PyResult MarketProxyService::CharGetNewTransactions(PyCallArgs &call, PyRep* sellBuy, PyRep* typeID, PyRep* clientID, PyRep* quantity, PyRep* fromDate, PyRep* maxPrice, PyRep* minPrice) {
    Market::TxData data = Market::TxData();

    data.clientID = clientID->IsNone() ? 0 : PyRep::IntegerValueU32(clientID);
    data.isBuy = sellBuy->IsNone() ? -1 : PyRep::IntegerValueU32(sellBuy);
    data.price = minPrice->IsNone() ? 0 : PyRep::IntegerValueU32(minPrice);
    data.quantity = quantity->IsNone() ? 0 : PyRep::IntegerValueU32(quantity);
    data.typeID = typeID->IsNone() ? 0 : PyRep::IntegerValueU32(typeID);
    data.time = fromDate->IsNone() ? 0 : PyRep::IntegerValue(fromDate);
    data.accountKey = Account::KeyType::Cash;

    return MarketDB::GetTransactions(call.client->GetCharacterID(), data);
}

PyResult MarketProxyService::CorpGetNewTransactions(PyCallArgs& call, PyRep* sellBuy, PyRep* typeID, PyRep* clientID, PyRep* quantity, PyRep* fromDate, PyRep* maxPrice, PyRep* minPrice, PyRep* accountKey, PyRep* memberID)
{
    Market::TxData data = Market::TxData();
        data.clientID = clientID->IsNone() ? 0 : PyRep::IntegerValueU32(clientID);
        data.isBuy = sellBuy->IsNone() ? 0 : PyRep::IntegerValueU32(sellBuy);
        data.price = minPrice->IsNone() ? 0 : PyRep::IntegerValueU32(minPrice);
        data.quantity = quantity->IsNone() ? 0 : PyRep::IntegerValueU32(quantity);
        data.typeID = typeID->IsNone() ? 0 : PyRep::IntegerValueU32(typeID);
        data.time = fromDate->IsNone() ? 0 : PyRep::IntegerValue(fromDate);
        data.accountKey = accountKey->IsNone() ? 0 : PyRep::IntegerValueU32(accountKey);
        data.memberID = memberID->IsNone() ? 0 : PyRep::IntegerValueU32(memberID);
    return MarketDB::GetTransactions(call.client->GetCorporationID(), data);
}

PyResult MarketProxyService::GetOrders(PyCallArgs &call, PyInt* typeID) {
    PyRep* result(nullptr);
    std::string method_name ("GetOrders_");
    method_name += std::to_string(call.client->GetRegionID());
    method_name += "_";
    method_name += std::to_string(typeID->value());
    ObjectCachedMethodID method_id(GetName().c_str(), method_name.c_str());
    //check to see if this method is in the cache already.
    if (!this->m_cache->IsCacheLoaded(method_id))
    {
        //this method is not in cache yet, load up the contents and cache it.
        result = MarketDB::GetOrders(call.client->GetRegionID(), typeID->value());
        if (result == nullptr) {
            _log(MARKET__DB_ERROR, "Failed to load cache, generating empty contents.");
            result = PyStatic.NewNone();
        }
        this->m_cache->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = this->m_cache->MakeObjectCachedMethodCallResult(method_id);

    /*{'FullPath': u'UI/Messages', 'messageID': 258616, 'label': u'MktMarketOpeningTitle'}(u'Market not open yet', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258617, 'label': u'MktMarketOpeningBody'}
     * (u'{region} market region is currently opening up for business and is not yet ready.
     * Please try again in a few minutes, or refer to another market region for your trading needs.',
     * None, {u'{region}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'region'}})
     */
    return result;
}

namespace {
using MarketOrderRow=std::shared_ptr<PyRep>;
MarketOrderRow SnapshotMarketOrder(uint32 id) {
    auto row=MarketOrderRow(MarketDB::GetOrderRow(id),[](PyRep* value) { PySafeDecRef(value); });
    if (!row) throw CustomError("Market order could not be read.");
    return row;
}
void PublishMarketOrder(uint32 owner,uint32 orderID,uint32 region,uint32 type,uint8 action,MarketOrderRow row) {
    DBTransaction::Publish([owner,orderID,region,type,action,row]() {
        sMktMgr.InvalidateOrdersCache(region,type);
        Client* client=sEntityList.FindClientByCharID(owner);
        if (client) sMktMgr.SendOnOwnOrderChanged(client,orderID,action,false,row->Clone());
    });
}
void RequireOwnedMarketOrder(Client* client,const Market::OrderInfo& order) {
    if (!client || !client->IsLoaded() || order.isCorp || order.ownerID!=client->GetCharacterID())
        throw CustomError("You do not own this market order.");
}
InventoryItemRef RetainedMarketStock(const Market::OrderInfo& order) {
    auto stock=sItemFactory.GetItemRef(order.escrowItemID);
    if (!stock || stock->typeID()!=order.typeID || stock->quantity()!=static_cast<int32>(order.quantity) ||
        stock->locationID()!=order.stationID || stock->flag()!=flagCorpMarket || stock->ownerID()!=stDataMgr.GetOwnerID(order.stationID))
        throw CustomError("The reserved market stock changed; the order was not completed.");
    return stock;
}
}

PyResult MarketProxyService::PlaceCharOrder(PyCallArgs& call,PyInt* stationID,PyInt* typeID,PyFloat* price,PyInt* quantity,PyInt* bid,PyInt* orderRange,std::optional<PyInt*> itemID,PyInt* minVolume,PyInt* duration,PyBool* useCorp,std::optional<PyRep*> located) {
    if (!call.client || !call.client->IsLoaded()) throw CustomError("Select a character before placing an order.");
    if (useCorp->value()) throw CustomError("Corporation Market transactions are not available at this time.");
    const double unitPrice=std::round(price->value()*100.0)/100.0;
    const bool buy=bid->value()==1;
    if ((bid->value()!=0 && bid->value()!=1) || !std::isfinite(unitPrice) || unitPrice<=0 || unitPrice>1e12 || quantity->value()<1 ||
        duration->value()<0 || duration->value()>90 || minVolume->value()<1 || minVolume->value()>quantity->value() ||
        orderRange->value() < -1 || orderRange->value()>32767 || !sDataMgr.IsStation(stationID->value()) ||
        typeID->value()<1 || typeID->value()>std::numeric_limits<uint16>::max())
        throw CustomError("Invalid market order price, quantity, station or duration.");
    const ItemType* type=sItemFactory.GetType(typeID->value());
    if (!type || !type->marketGroupID()) throw CustomError("This item type cannot be traded on the market.");
    if (sDataMgr.GetStationRegion(stationID->value())!=call.client->GetRegionID())
        throw CustomError("The order station must be in your current region.");
    const bool hasItem=itemID.has_value() && itemID.value()->value()>0;
    if ((buy && hasItem) || (!buy && !hasItem)) throw CustomError("Invalid market item selection.");
    const double total=std::round(unitPrice*quantity->value()*100.0)/100.0;
    if (!std::isfinite(total) || total>1e12) throw CustomError("Market order value is too large.");
    DBTransaction transaction;
    if (!transaction.Started()) throw CustomError("The market database is unavailable.");
    InventoryItemRef source(nullptr);
    if (!buy) {
        source=sItemFactory.GetItemRef(itemID.value()->value());
        if (!source || source->ownerID()!=call.client->GetCharacterID() || source->typeID()!=typeID->value() ||
            source->isSingleton() || source->quantity()<quantity->value()) throw CustomError("The sell item changed or is not yours.");
        const bool inHangar=source->locationID()==static_cast<uint32>(stationID->value()) && source->flag()==flagHangar;
        const bool inDockedCargo=call.client->IsDocked() && call.client->GetStationID()==stationID->value() &&
            source->locationID()==call.client->GetShipID() && source->flag()==flagCargoHold;
        if (!inHangar && !inDockedCargo) throw CustomError("The sell item is not in this station's hangar or your docked ship cargo.");
    }
    if (!duration->value()) {
        const uint32 id=buy ? MarketDB::FindSellOrder(typeID->value(),stationID->value(),quantity->value(),unitPrice) :
            MarketDB::FindBuyOrder(typeID->value(),stationID->value(),quantity->value(),unitPrice);
        if (!id) throw CustomError("No matching market order found.");
        if (buy) sMktMgr.ExecuteSellOrder(call.client,id,quantity->value(),unitPrice,stationID->value(),typeID->value(),false);
        else if (!sMktMgr.ExecuteBuyOrder(call.client,id,source,quantity->value(),false,typeID->value(),stationID->value(),unitPrice))
            throw CustomError("The market order changed; the sale was cancelled.");
        if (!transaction.Commit()) throw CustomError("Market exchange failed.");
        return nullptr;
    }
    Market::SaveData data={};
    data.bid=buy; data.ownerID=call.client->GetCharacterID(); data.typeID=typeID->value(); data.stationID=stationID->value();
    data.regionID=sDataMgr.GetStationRegion(data.stationID); data.solarSystemID=sDataMgr.GetStationSystem(data.stationID);
    data.orderRange=orderRange->value(); data.minVolume=minVolume->value(); data.price=unitPrice;
    data.volEntered=data.volRemaining=quantity->value(); data.duration=duration->value(); data.issued=GetFileTimeNow();
    data.accountKey=Account::KeyType::Cash; data.escrow=buy ? total : 0;
    data.contraband=source ? source->contraband() : false; data.jumps=1;
    const uint32 id=MarketDB::StoreOrder(data);
    if (!id) throw CustomError("Market order creation failed.");
    const uint32 stationOwner=stDataMgr.GetOwnerID(data.stationID);
    const float ownerStanding=StandingDB::GetStanding(stationOwner,data.ownerID);
    const float factionStanding=IsNPCCorp(stationOwner) ? StandingDB::GetStanding(sDataMgr.GetCorpFaction(stationOwner),data.ownerID) : 0;
    const double fee=std::round(EvEMath::Market::BrokerFee(call.client->GetChar()->GetSkillLevel(EvESkill::BrokerRelations),factionStanding,ownerStanding,total)*100.0)/100.0;
    const std::string reason="DESC: Setting up market order in "+stDataMgr.GetStationName(data.stationID);
    AccountService::TransferFunds(data.ownerID,stationOwner,fee,reason.c_str(),Journal::EntryType::Brokerfee,id);
    if (buy) {
        AccountService::TransferFunds(data.ownerID,stationOwner,total,reason.c_str(),Journal::EntryType::MarketEscrow,id,Account::KeyType::Cash,Account::KeyType::Escrow);
    } else {
        auto stock=source;
        if (source->quantity()!=quantity->value()) stock=source->Split(quantity->value(),true);
        if (!stock) throw CustomError("Market stock reservation failed.");
        stock->Donate(stationOwner,data.stationID,flagCorpMarket,true);
        DBerror error;
        if (!sDatabase.RunQuery(error,"INSERT INTO mktOrderItems(orderID,itemID) VALUES(%u,%u)",id,stock->itemID()))
            throw CustomError("Market stock reservation failed.");
    }
    auto row=SnapshotMarketOrder(id);
    if (!transaction.Commit()) throw CustomError("Market order creation failed.");
    PublishMarketOrder(data.ownerID,id,data.regionID,data.typeID,Market::Action::Add,row);
    return nullptr;
}

PyResult MarketProxyService::PlaceCharOrder(PyCallArgs& call,PyInt* stationID,PyInt* typeID,PyFloat* price,PyInt* quantity,PyInt* bid,PyInt* orderRange,std::optional<PyInt*> itemID,PyInt* minVolume,PyInt* duration,PyInt* useCorp,std::optional<PyRep*> located) {
    auto flag=std::shared_ptr<PyBool>(new PyBool(useCorp->value()!=0),[](PyBool* value) { PyDecRef(value); });
    return PlaceCharOrder(call,stationID,typeID,price,quantity,bid,orderRange,itemID,minVolume,duration,flag.get(),located);
}

PyResult MarketProxyService::ModifyCharOrder(PyCallArgs& call,PyInt* orderID,PyFloat* newPrice,PyInt* bid,PyInt* stationID,PyInt* solarSystemID,PyFloat* price,PyInt* range,PyInt* volRemaining,PyLong* issueDate) {
    const double unitPrice=std::round(newPrice->value()*100.0)/100.0;
    if (!std::isfinite(unitPrice) || unitPrice<=0 || unitPrice>1e12) throw CustomError("Invalid market price.");
    DBTransaction transaction;
    Market::OrderInfo order={};
    if (!transaction.Started() || !MarketDB::GetOrderInfo(orderID->value(),order)) throw CustomError("Market order not found.");
    RequireOwnedMarketOrder(call.client,order);
    const std::string reason="DESC: Altering Market Order #"+std::to_string(orderID->value());
    if (order.isBuy) {
        const double escrow=std::round(unitPrice*order.quantity*100.0)/100.0;
        if (!std::isfinite(escrow) || escrow>1e12 || !std::isfinite(order.escrow) || order.escrow<0) throw CustomError("Invalid market escrow.");
        const double change=std::round((escrow-order.escrow)*100.0)/100.0;
        const uint32 stationOwner=stDataMgr.GetOwnerID(order.stationID);
        if (change>0) AccountService::TransferFunds(order.ownerID,stationOwner,change,reason.c_str(),Journal::EntryType::MarketEscrow,orderID->value(),order.accountKey,Account::KeyType::Escrow);
        else if (change<0) AccountService::TransferFunds(stationOwner,order.ownerID,-change,reason.c_str(),Journal::EntryType::MarketEscrow,orderID->value(),Account::KeyType::Escrow,order.accountKey);
        DBerror error; uint32 affected=0;
        if (!sDatabase.RunQuery(error,affected,"UPDATE mktOrders SET price=%.2f,escrow=%.2f WHERE orderID=%u",unitPrice,escrow,orderID->value()) || affected!=1)
            throw CustomError("Market order update failed.");
    } else if (!MarketDB::AlterOrderPrice(orderID->value(),unitPrice)) throw CustomError("Market order update failed.");
    auto row=SnapshotMarketOrder(orderID->value());
    if (!transaction.Commit()) throw CustomError("Market order update failed.");
    PublishMarketOrder(order.ownerID,orderID->value(),order.regionID,order.typeID,Market::Action::Modify,row);
    return nullptr;
}

PyResult MarketProxyService::CancelCharOrder(PyCallArgs& call,PyInt* orderID,PyInt* regionID) {
    DBTransaction transaction;
    Market::OrderInfo order={};
    if (!transaction.Started() || !MarketDB::GetOrderInfo(orderID->value(),order)) throw CustomError("Market order not found.");
    RequireOwnedMarketOrder(call.client,order);
    auto row=SnapshotMarketOrder(orderID->value());
    if (order.isBuy) {
        if (!std::isfinite(order.escrow) || order.escrow<0) throw CustomError("Invalid market escrow.");
        const std::string reason="DESC: Canceling Market Order #"+std::to_string(orderID->value());
        AccountService::TransferFunds(stDataMgr.GetOwnerID(order.stationID),order.ownerID,order.escrow,reason.c_str(),
            Journal::EntryType::MarketEscrow,orderID->value(),Account::KeyType::Escrow,order.accountKey);
    } else {
        InventoryItemRef stock(nullptr);
        if (order.escrowItemID) stock=RetainedMarketStock(order);
        else {
            if (!order.quantity || order.quantity>static_cast<uint32>(std::numeric_limits<int32>::max())) throw CustomError("Invalid market quantity.");
            ItemData data(order.typeID,ownerStation,locTemp,flagHangar,order.quantity);
            stock=sItemFactory.SpawnItem(data);
            if (!stock) throw CustomError("Market stock could not be returned; cancellation failed.");
        }
        stock->Donate(order.ownerID,order.stationID,flagHangar,true);
    }
    if (!MarketDB::DeleteOrder(orderID->value()) || !transaction.Commit()) throw CustomError("Market cancellation failed.");
    PublishMarketOrder(order.ownerID,orderID->value(),order.regionID,order.typeID,Market::Action::Expiry,row);
    return nullptr;
}
