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
    Author:        Luck (outline only)
    Updates:    Allan   (coded working system)
*/

#include "eve-server.h"
#include "EntityList.h"
#include "account/AccountService.h"
#include "inventory/ItemDB.h"
#include "inventory/Inventory.h"
#include "station/TradeService.h"
#include "services/ServiceManager.h"
#include <cmath>
#include <memory>
#include <set>
#include <vector>

namespace {
void TradeSQL(const char* sql) {
    DBerror error;
    if (!sDatabase.RunQuery(error,"%s",sql)) throw CustomError("The trade could not be saved. Please try again.");
}
double TradeMoney(double amount) {
    if (!std::isfinite(amount) || amount<0 || amount>1000000000000.0) throw CustomError("Invalid trade money offer.");
    return std::round(amount*100.0)/100.0;
}
void TradeNotify(TradeSession* session,PyTuple* packet) {
    const uint32 first=session->m_tradeSession.myID,second=session->m_tradeSession.herID;
    auto owned=std::shared_ptr<PyTuple>(packet,[](PyTuple* value) {PyDecRef(value);});
    DBTransaction::Publish([owned,first,second]() {
        for (uint32 id:{first,second}) {
            Client* client=sEntityList.FindClientByCharID(id);
            if (client) {auto copy=owned->Clone()->AsTuple();client->SendNotification("OnTrade","charid",&copy);}
        }
    });
}
void TradeStates(TradeSession* session) {
    auto packet=new PyTuple(3); packet->SetItem(0,new PyString("StateToggle"));
    packet->SetItem(1,new PyBool(session->m_tradeSession.myState));packet->SetItem(2,new PyBool(session->m_tradeSession.herState));
    TradeNotify(session,packet);
}
void PrepareTradeMemory(TradeSession* session) {
    if (!DBTransaction::Active()) return;
    const auto before=session->m_tradeSession;
    DBTransaction::OnRollback([session,before]() {session->m_tradeSession=before;});
}
TradeSession::TradeItems TradeOffer(InventoryItemRef item) {
    return {item->itemID(),item->typeID(),item->ownerID(),item->locationID(),item->flag(),static_cast<uint32>(item->quantity()),item->groupID(),item->categoryID(),item->isSingleton(),item->customInfo()};
}
PyPackedRow* TradeRow(const TradeSession::TradeItems& item) {
    auto row=new PyPackedRow(sDataMgr.CreateHeader());
    row->SetField("itemID",new PyLong(item.itemID));row->SetField("typeID",new PyInt(item.typeID));
    row->SetField("ownerID",new PyInt(item.ownerID));row->SetField("locationID",new PyInt(item.locationID));
    row->SetField("flagID",new PyInt(item.flagID));row->SetField("groupID",new PyInt(item.groupID));
    row->SetField("categoryID",new PyInt(item.categoryID));row->SetField("quantity",new PyInt(item.singleton ? -1 : item.quantity));
    row->SetField("customInfo",new PyString(item.customInfo));return row;
}
// Read the persisted tree, including unloaded and nested contents. Lock and validate
// the entire tree before any payment or item mutation, so cached inventories cannot
// hide contents or let a descendant also appear as a separate trade offer.
void TradeTree(uint32 id,uint32 owner,std::set<uint32>& seen,std::vector<InventoryItemRef>& items,unsigned depth=0) {
    if (depth>64 || seen.size()>=10000 || !seen.insert(id).second) throw CustomError("Invalid or overlapping trade contents.");
    DBQueryResult locked;DBResultRow row;
    if (!sDatabase.RunQuery(locked,"SELECT ownerID FROM entity WHERE itemID=%u FOR UPDATE",id) || !locked.GetRow(row) || row.GetUInt(0)!=owner)
        throw CustomError("An offered item or its contents changed. Please reopen the trade.");
    auto item=sItemFactory.GetItemRef(id);
    if (!item || item->ownerID()!=owner || item->HasPilot()) throw CustomError("This item cannot be traded.");
    items.push_back(item);
    DBQueryResult children;std::vector<uint32> ids;
    if (!sDatabase.RunQuery(children,"SELECT itemID FROM entity WHERE locationID=%u ORDER BY itemID FOR UPDATE",id)) throw CustomError("Cannot read trade contents.");
    while (children.GetRow(row)) ids.push_back(row.GetUInt(0));
    for (uint32 child:ids) {
        const size_t index=items.size();TradeTree(child,owner,seen,items,depth+1);
        auto inventory=item->GetMyInventory();
        if (inventory && !inventory->ContainsItem(child)) inventory->AddItem(items[index]);
    }
}
void TradeReturnRows(uint32 container,bool startup) {
    DBQueryResult reservations;DBResultRow row;
    const char* filter=container ? "containerID=%u" : "containerID>=64000000";
    std::string sql="SELECT itemID,containerID,ownerID,stationID,originalLocationID,originalFlag FROM chrTradeItems WHERE ";sql+=filter;sql+=" ORDER BY itemID FOR UPDATE";
    if (!sDatabase.RunQuery(reservations,sql.c_str(),container)) throw CustomError("Cannot read reserved trade items.");
    struct Return {uint32 id,container,owner,station,location,flag;};std::vector<Return> items;
    while (reservations.GetRow(row)) items.push_back({row.GetUInt(0),row.GetUInt(1),row.GetUInt(2),row.GetUInt(3),row.GetUInt(4),row.GetUInt(5)});
    for (auto& item:items) {
        DBQueryResult current;
        if (!sDatabase.RunQuery(current,"SELECT ownerID,locationID FROM entity WHERE itemID=%u FOR UPDATE",item.id) || !current.GetRow(row) || row.GetUInt(0)!=item.owner || row.GetUInt(1)!=item.container)
            throw CustomError("A reserved trade item changed; recovery requires inspection.");
        if (item.location!=item.station) {
            DBQueryResult parent;
            if (!sDatabase.RunQuery(parent,"SELECT ownerID,locationID FROM entity WHERE itemID=%u FOR UPDATE",item.location)) throw CustomError("Cannot inspect trade return container.");
            if (!parent.GetRow(row) || row.GetUInt(0)!=item.owner || row.GetUInt(1)!=item.station) {item.location=item.station;item.flag=flagHangar;}
        }
        DBerror error;
        if (startup) {
            if (!sDatabase.RunQuery(error,"UPDATE entity SET locationID=%u,flag=%u WHERE itemID=%u",item.location,item.flag,item.id)) throw CustomError("Cannot recover trade item.");
        } else {
            auto reference=sItemFactory.GetItemRef(item.id);
            if (!reference || reference->ownerID()!=item.owner || reference->locationID()!=item.container) throw CustomError("Reserved trade cache changed.");
            reference->Move(item.location,static_cast<EVEItemFlags>(item.flag),true);
        }
        if (!sDatabase.RunQuery(error,"DELETE FROM chrTradeItems WHERE itemID=%u",item.id)) throw CustomError("Cannot release trade reservation.");
    }
}
}

TradeBound::TradeBound(EVEServiceManager& mgr,TradeService& parent,uint32 containerID)
    : EVEBoundObject(mgr,parent),m_TSvc(&parent),m_containerID(containerID) {
    EVEBoundObject::Add("List",&TradeBound::List);EVEBoundObject::Add("IsCEOTrade",&TradeBound::IsCEOTrade);
    EVEBoundObject::Add("GetItemID",&TradeBound::GetItemID);EVEBoundObject::Add("GetItem",&TradeBound::GetItem);
    EVEBoundObject::Add("Add",&TradeBound::Add);EVEBoundObject::Add("MultiAdd",&TradeBound::MultiAdd);
    EVEBoundObject::Add("ToggleAccept",&TradeBound::ToggleAccept);EVEBoundObject::Add("OfferMoney",&TradeBound::OfferMoney);
    EVEBoundObject::Add("Abort",&TradeBound::Abort);
}
TradeSession* TradeService::RequireSession(Client* client,uint32 container) {
    auto found=m_activeSessions.find(client->GetCharacterID());
    auto session=client->GetTradeSession();
    if (!session || found==m_activeSessions.end() || found->second.ourTS!=session || (container && session->m_tradeSession.containerID!=container)) throw CustomError("This trade is no longer active.");
    const auto& data=session->m_tradeSession;
    Client* first=sEntityList.FindClientByCharID(data.myID);Client* second=sEntityList.FindClientByCharID(data.herID);
    auto other=m_activeSessions.find(client->GetCharacterID()==data.myID ? data.herID : data.myID);
    if (data.myID==data.herID || (client->GetCharacterID()!=data.myID && client->GetCharacterID()!=data.herID) ||
        !first || !second || first->GetTradeSession()!=session || second->GetTradeSession()!=session ||
        other==m_activeSessions.end() || other->second.ourTS!=session || !first->IsDocked() || !second->IsDocked() ||
        first->GetStationID()!=data.stationID || second->GetStationID()!=data.stationID) throw CustomError("Both traders must be in the same station.");
    return session;
}
PyResult TradeBound::OfferMoney(PyCallArgs& call,PyFloat* amount) {
    auto session=m_TSvc->RequireSession(call.client,m_containerID);const double money=TradeMoney(amount->value());
    DBQueryResult balance;DBResultRow row;
    if (!sDatabase.RunQuery(balance,"SELECT balance FROM chrCharacters WHERE characterID=%u",call.client->GetCharacterID()) || !balance.GetRow(row) || row.GetDouble(0)<money)
        throw CustomError("You do not have enough money for this offer.");
    PrepareTradeMemory(session);
    if (call.client->GetCharacterID()==session->m_tradeSession.myID) session->m_tradeSession.myMoney=money;else session->m_tradeSession.herMoney=money;
    session->m_tradeSession.myState=session->m_tradeSession.herState=false;
    auto values=new PyList(2);values->SetItem(0,new PyFloat(session->m_tradeSession.myMoney));values->SetItem(1,new PyFloat(session->m_tradeSession.herMoney));
    auto packet=new PyTuple(3);packet->SetItem(0,new PyString("MoneyOffer"));packet->SetItem(1,new PyInt(session->m_tradeSession.containerID));packet->SetItem(2,values);
    TradeNotify(session,packet);TradeStates(session);return PyStatic.NewNone();
}
void TradeBound::AddItems(PyCallArgs& call,const std::vector<uint32>& ids,uint32 sourceContainer,int32 quantity) {
    auto session=m_TSvc->RequireSession(call.client,m_containerID);
    if (ids.empty() || ids.size()>1000 || quantity<0) throw CustomError("Invalid trade item quantity.");
    DBTransaction transaction;if (!transaction.Started()) throw CustomError("Cannot start trade reservation.");
    std::set<uint32> seen;std::vector<InventoryItemRef> items;
    for (uint32 id:ids) {
        if (!seen.insert(id).second || IsTempItem(id)) throw CustomError("Duplicate or temporary trade item.");
        DBQueryResult locked;DBResultRow row;
        if (!sDatabase.RunQuery(locked,"SELECT itemID FROM entity WHERE itemID=%u FOR UPDATE",id) || !locked.GetRow(row)) throw CustomError("Trade item is missing.");
        ItemData stored;
        auto item=sItemFactory.GetItemRef(id);
        if (!item || !ItemDB::GetItemData(id,stored) || stored.ownerID!=call.client->GetCharacterID() || stored.locationID!=sourceContainer || stored.quantity<=0 ||
            item->ownerID()!=stored.ownerID || item->locationID()!=stored.locationID || item->quantity()!=stored.quantity || item->flag()!=stored.flag ||
            id==call.client->GetShipID() || item->HasPilot() ||
            !((stored.locationID==session->m_tradeSession.stationID && stored.flag==flagHangar) || (stored.locationID==call.client->GetShipID() && stored.flag==flagCargoHold)))
            throw CustomError("Only your station hangar or docked cargo items can be offered.");
        if (quantity>stored.quantity || (quantity && quantity<stored.quantity && stored.singleton)) throw CustomError("Invalid partial trade stack.");
        if (quantity && quantity<stored.quantity) {item=item->Split(quantity,true);if (!item) throw CustomError("Cannot split this trade offer.");}
        DBerror error;
        if (!sDatabase.RunQuery(error,"INSERT INTO chrTradeItems(itemID,containerID,ownerID,stationID,originalLocationID,originalFlag) VALUES (%u,%u,%u,%u,%u,%u)",item->itemID(),session->m_tradeSession.containerID,item->ownerID(),session->m_tradeSession.stationID,stored.locationID,stored.flag)) throw CustomError("This item is already reserved or could not be saved.");
        item->Move(session->m_tradeSession.containerID,flagNone,false);items.push_back(item);
        for (uint32 trader:{session->m_tradeSession.myID,session->m_tradeSession.herID}) {
            std::map<int32,PyRep*> changes;changes[Inv::Update::Location]=new PyInt(stored.locationID);changes[Inv::Update::Flag]=new PyInt(stored.flag);
            item->SendItemChange(trader,changes);
        }
    }
    if (!transaction.Commit()) throw CustomError("Cannot save the trade offer.");
    DBTransaction::Publish([session,items]() {
        for (auto item:items) session->m_tradelist.push_back(TradeOffer(item));
        session->m_tradeSession.myState=session->m_tradeSession.herState=false;TradeStates(session);
    });
}
PyResult TradeBound::Add(PyCallArgs& call,PyInt* itemID,PyInt* containerID) {
    int32 quantity=0;auto found=call.byname.find("qty");
    if (found!=call.byname.end() && !found->second->IsNone()) {
        if (!found->second->IsInt() || found->second->AsInt()->value()<0) throw CustomError("Invalid trade quantity.");
        quantity=found->second->AsInt()->value();
    }
    AddItems(call,{static_cast<uint32>(itemID->value())},containerID->value(),quantity);return PyStatic.NewNone();
}
PyResult TradeBound::MultiAdd(PyCallArgs& call,PyList* itemIDs,PyInt* containerID) {
    std::vector<uint32> ids;
    for (auto value:*itemIDs) {if (!value->IsInt() || value->AsInt()->value()<=0) throw CustomError("Invalid trade item.");ids.push_back(value->AsInt()->value());}
    AddItems(call,ids,containerID->value(),0);return PyStatic.NewNone();
}
void TradeBound::ExchangeItems(Client* first,Client* second,TradeSession* session) {
    if (m_TSvc->RequireSession(first,m_containerID)!=session || m_TSvc->RequireSession(second,m_containerID)!=session) throw CustomError("Trade session changed.");
    auto& data=session->m_tradeSession;
    first=sEntityList.FindClientByCharID(data.myID);second=sEntityList.FindClientByCharID(data.herID);
    if (!data.myState || !data.herState) throw CustomError("Both traders must accept the current offer.");
    const double firstMoney=TradeMoney(data.myMoney),secondMoney=TradeMoney(data.herMoney);
    DBTransaction transaction;if (!transaction.Started()) throw CustomError("Cannot start trade exchange.");
    DBQueryResult wallets;DBResultRow row;std::map<uint32,double> balances;
    if (!sDatabase.RunQuery(wallets,"SELECT characterID,balance FROM chrCharacters WHERE characterID IN (%u,%u) ORDER BY characterID FOR UPDATE",data.myID,data.herID)) throw CustomError("Cannot lock trade wallets.");
    while (wallets.GetRow(row)) balances[row.GetUInt(0)]=row.GetDouble(1);
    if (balances.size()!=2 || !std::isfinite(balances[data.myID]) || !std::isfinite(balances[data.herID]) || balances[data.myID]<firstMoney || balances[data.herID]<secondMoney)
        throw CustomError("A trader does not have enough money for the current offer.");
    struct Transfer {InventoryItemRef root;std::vector<InventoryItemRef> tree;uint32 recipient;};
    std::vector<Transfer> transfers;std::set<uint32> seen;
    for (const auto& offer:session->m_tradelist) {
        if (offer.ownerID!=data.myID && offer.ownerID!=data.herID) throw CustomError("Invalid trade item owner.");
        DBQueryResult reservation;
        if (!sDatabase.RunQuery(reservation,"SELECT ownerID,containerID,stationID FROM chrTradeItems WHERE itemID=%u FOR UPDATE",offer.itemID) || !reservation.GetRow(row) || row.GetUInt(0)!=offer.ownerID || row.GetUInt(1)!=data.containerID || row.GetUInt(2)!=data.stationID) throw CustomError("The offered item is no longer reserved for this trade.");
        Transfer transfer;transfer.recipient=offer.ownerID==data.myID ? data.herID : data.myID;
        TradeTree(offer.itemID,offer.ownerID,seen,transfer.tree);transfer.root=transfer.tree.front();
        ItemData stored;
        if (!ItemDB::GetItemData(offer.itemID,stored) || stored.typeID!=offer.typeID || stored.quantity!=static_cast<int32>(offer.quantity) || stored.locationID!=data.containerID || stored.flag!=offer.flagID || stored.singleton!=offer.singleton ||
            transfer.root->locationID()!=stored.locationID || transfer.root->quantity()!=stored.quantity || transfer.root->flag()!=stored.flag || transfer.root->isSingleton()!=stored.singleton)
            throw CustomError("An offered item changed. Review and accept the offer again.");
        if (offer.itemID==first->GetShipID() || offer.itemID==second->GetShipID()) throw CustomError("An active ship cannot be traded.");
        transfers.push_back(transfer);
    }
    DBQueryResult count;
    if (!sDatabase.RunQuery(count,"SELECT COUNT(*) FROM chrTradeItems WHERE containerID=%u",data.containerID) || !count.GetRow(row) || row.GetUInt(0)!=transfers.size()) throw CustomError("The trade offer does not match its reserved items.");
    const std::string reason="Player Trade between "+first->GetCharName()+" and "+second->GetCharName();
    AccountService::TransferFunds(data.myID,data.herID,firstMoney,reason,Journal::EntryType::PlayerTrading,data.stationID);
    AccountService::TransferFunds(data.herID,data.myID,secondMoney,reason,Journal::EntryType::PlayerTrading,data.stationID);
    for (const auto& transfer:transfers) {
        for (auto item:transfer.tree) {
            if (item->categoryID()==EVEDB::invCategories::Ship) {
                DBerror error;if (!sDatabase.RunQuery(error,"DELETE FROM shipInsurance WHERE shipID=%u",item->itemID())) throw CustomError("Cannot remove traded ship insurance.");
            }
            if (item->itemID()!=transfer.root->itemID()) item->ChangeOwner(transfer.recipient,true);
        }
        transfer.root->Donate(transfer.recipient,data.stationID,flagHangar,true);
    }
    DBerror error;if (!sDatabase.RunQuery(error,"DELETE FROM chrTradeItems WHERE containerID=%u",data.containerID) || !transaction.Commit()) throw CustomError("The trade could not be completed. Nothing was exchanged.");
}
PyResult TradeBound::ToggleAccept(PyCallArgs& call,PyBool* accept) {
    auto session=m_TSvc->RequireSession(call.client,m_containerID);auto& data=session->m_tradeSession;
    PrepareTradeMemory(session);
    // A caller can accept only their own offer, including when forceTrade is supplied.
    if (call.client->GetCharacterID()==data.myID) data.myState=accept->value();else data.herState=accept->value();
    if (data.myState && data.herState) {
        try {ExchangeItems(sEntityList.FindClientByCharID(data.myID),sEntityList.FindClientByCharID(data.herID),session);}
        catch (...) {data.myState=data.herState=false;TradeStates(session);throw;}
        m_TSvc->FinishSession(session,"TradeComplete");
    } else TradeStates(session);
    return PyStatic.NewNone();
}
void TradeService::CancelSession(TradeSession* session) {
    DBTransaction transaction;if (!transaction.Started()) throw CustomError("Cannot cancel this trade yet.");
    TradeReturnRows(session->m_tradeSession.containerID,false);
    if (!transaction.Commit()) throw CustomError("Cannot save trade cancellation.");
    FinishSession(session,"Cancel");
}
void TradeService::FinishSession(TradeSession* session,const char* event) {
    const std::string name(event);
    DBTransaction::Publish([this,session,name]() {
    auto packet=new PyTuple(2);packet->SetItem(0,new PyString(name));packet->SetItem(1,new PyInt(session->m_tradeSession.containerID));TradeNotify(session,packet);
    for (uint32 id:{session->m_tradeSession.myID,session->m_tradeSession.herID}) {
        Client* client=sEntityList.FindClientByCharID(id);if (client && client->GetTradeSession()==session) client->ClearTradeSession();
        m_activeSessions.erase(id);
    }
    delete session;
    });
}
void TradeBound::CancelTrade(Client*,Client*,TradeSession* session) {m_TSvc->CancelSession(session);}
PyResult TradeBound::Abort(PyCallArgs& call) {m_TSvc->CancelSession(m_TSvc->RequireSession(call.client,m_containerID));return PyStatic.NewNone();}
void TradeService::CancelTrade(Client* client) {
    auto session=client->GetTradeSession();if (!session) return;
    try {CancelSession(session);}
    catch (...) {
        // This path is used from logout/destruction. Keep the durable reservation
        // for startup recovery instead of saving stale caches during partial cleanup.
        sLog.Error("Trade","Trade cancellation failed during client cleanup; stopping for reservation recovery.");std::_Exit(EXIT_FAILURE);
    }
}
bool TradeService::RecoverAbandonedTrades() {
    try {
        DBTransaction transaction;if (!transaction.Started()) return false;
        TradeReturnRows(0,true);
        DBQueryResult remaining;DBResultRow row;
        if (!sDatabase.RunQuery(remaining,"SELECT COUNT(*) FROM entity WHERE locationID BETWEEN 64000000 AND 65999999") || !remaining.GetRow(row) || row.GetUInt(0)) return false;
        return transaction.Commit();
    } catch (...) {return false;}
}
uint32 TradeService::ReservedRoot(InventoryItemRef item) {
    if (!item) throw CustomError("The item no longer exists.");
    uint32 id=item->itemID(),location=item->locationID();std::set<uint32> seen;
    for (unsigned depth=0;depth<=64;++depth) {
        if (!seen.insert(id).second) throw CustomError("Invalid item container hierarchy.");
        if (IsTrading(location)) return id;
        if (!IsPlayerItem(location)) return 0;
        ItemData parent;if (!ItemDB::GetItemData(location,parent)) throw CustomError("Cannot inspect the item container.");
        id=location;location=parent.locationID;
    }
    throw CustomError("The item container hierarchy is too deep.");
}
void TradeService::WithdrawItem(Client* client,InventoryItemRef item,uint32 destination,EVEItemFlags flag,int32 quantity,bool wholeStack) {
    auto session=RequireSession(client);auto& data=session->m_tradeSession;
    if (!item || ReservedRoot(item)!=item->itemID() || item->ownerID()!=client->GetCharacterID() ||
        !((destination==data.stationID && flag==flagHangar) || (destination==client->GetShipID() && flag==flagCargoHold)))
        throw CustomError("Only the offering player can withdraw an entire item into their hangar or cargo. Withdraw its containing ship first to change contents.");
    if (!wholeStack && quantity>0 && quantity!=item->quantity()) throw CustomError("Withdraw the complete offered stack before dividing it.");
    DBTransaction transaction;if (!transaction.Started()) throw CustomError("Cannot withdraw this offer yet.");
    DBQueryResult reservation;DBResultRow row;
    if (!sDatabase.RunQuery(reservation,"SELECT ownerID,containerID FROM chrTradeItems WHERE itemID=%u FOR UPDATE",item->itemID()) || !reservation.GetRow(row) || row.GetUInt(0)!=client->GetCharacterID() || row.GetUInt(1)!=data.containerID || item->locationID()!=data.containerID)
        throw CustomError("This item is not your current trade offer.");
    item->Move(destination,flag,false);
    for (uint32 id:{data.myID,data.herID}) {
        std::map<int32,PyRep*> changes;changes[Inv::Update::Location]=new PyInt(data.containerID);changes[Inv::Update::Flag]=new PyInt(flagNone);item->SendItemChange(id,changes);
    }
    DBerror error;if (!sDatabase.RunQuery(error,"DELETE FROM chrTradeItems WHERE itemID=%u",item->itemID()) || !transaction.Commit()) throw CustomError("Cannot save the withdrawn offer.");
    DBTransaction::Publish([session,item]() {
        session->m_tradelist.remove_if([item](const TradeSession::TradeItems& offer) {return offer.itemID==item->itemID();});
        session->m_tradeSession.myState=session->m_tradeSession.herState=false;TradeStates(session);
    });
}
PyResult TradeBound::GetItemID(PyCallArgs& call) {m_TSvc->RequireSession(call.client,m_containerID);return PyStatic.NewNone();}
PyResult TradeBound::IsCEOTrade(PyCallArgs& call) {m_TSvc->RequireSession(call.client,m_containerID);return new PyBool(false);}
PyResult TradeBound::GetItem(PyCallArgs& call) {
    auto session=m_TSvc->RequireSession(call.client,m_containerID);auto row=new PyPackedRow(sDataMgr.CreateHeader());
    row->SetField("itemID",new PyLong(session->m_tradeSession.containerID));row->SetField("typeID",new PyInt(53));row->SetField("ownerID",PyStatic.NewOne());
    row->SetField("locationID",new PyInt(session->m_tradeSession.stationID));row->SetField("flagID",PyStatic.NewNone());row->SetField("quantity",new PyInt(-1));
    row->SetField("groupID",new PyInt(EVEDB::invGroups::Trade_Session));row->SetField("categoryID",new PyInt(EVEDB::invCategories::Trading));row->SetField("customInfo",PyStatic.NewNone());return row;
}
PyResult TradeBound::List(PyCallArgs& call) {
    auto session=m_TSvc->RequireSession(call.client,m_containerID);auto list=new PyList();for (const auto& item:session->m_tradelist) list->AddItem(TradeRow(item));
    auto inner=new PyTuple(1);inner->SetItem(0,list);auto tuple=new PyTuple(2);tuple->SetItem(0,new PyToken("__builtin__.set"));tuple->SetItem(1,inner);
    TradeListData data;data.tradeContainerID=session->m_tradeSession.containerID;data.myID=session->m_tradeSession.myID;data.herID=session->m_tradeSession.herID;
    data.myMoney=session->m_tradeSession.myMoney;data.herMoney=session->m_tradeSession.herMoney;data.myState=session->m_tradeSession.myState;data.herState=session->m_tradeSession.herState;data.list=new PyObjectEx(false,tuple);
    auto names=new PyList(5);const char* fields[]={"tradeContainerID","traders","state","money","items"};for (size_t i=0;i<5;++i) names->SetItem(i,new PyString(fields[i]));
    TradeListRsp result;result.header=names;result.line=data.Encode();return result.Encode();
}
TradeService::TradeService(EVEServiceManager& mgr) : BindableService("trademgr",mgr,eAccessLevel_SolarSystem2),m_SessionID(minTradeCont) {Add("InitiateTrade",&TradeService::InitiateTrade);}
BoundDispatcher* TradeService::BindObject(Client* client,PyRep* parameters) {
    Trade_BindArgs args;PyRep* clone=parameters->Clone();if (!args.Decode(&clone)) {PySafeDecRef(clone);throw CustomError("Invalid trade binding.");}
    const uint32 caller=client->GetCharacterID();
    if (args.myID==args.herID || (caller!=args.myID && caller!=args.herID)) throw CustomError("You are not a participant in this trade.");
    Client* first=sEntityList.FindClientByCharID(args.myID);Client* second=sEntityList.FindClientByCharID(args.herID);
    if (!first || !second || !first->IsDocked() || !second->IsDocked() || first->GetStationID()!=args.stationID || second->GetStationID()!=args.stationID) throw CustomError("Both traders must be in the same station.");
    TradeSession* session=nullptr;auto found=m_activeSessions.find(args.myID);
    if (found!=m_activeSessions.end()) {
        session=RequireSession(client);
        if (session->m_tradeSession.myID!=args.myID || session->m_tradeSession.herID!=args.herID || session->m_tradeSession.stationID!=args.stationID) throw CustomError("A different trade is already active.");
    } else {
        if (first->GetTradeSession() || second->GetTradeSession() || m_activeSessions.count(args.herID)) throw CustomError("One of the traders is already trading.");
        session=new TradeSession();session->m_tradeSession={GetTradeSessionID(),static_cast<uint32>(args.stationID),static_cast<uint32>(args.myID),static_cast<uint32>(args.herID),false,false,0,0,static_cast<int64>(GetFileTimeNow())};
        ActiveSession active={static_cast<uint32>(args.myID),static_cast<uint32>(args.herID),session->m_tradeSession.containerID,session};m_activeSessions[args.myID]=active;m_activeSessions[args.herID]=active;
        first->SetTradeSession(session);second->SetTradeSession(session);
    }
    return new TradeBound(GetServiceManager(),*this,session->m_tradeSession.containerID);
}
void TradeService::BoundReleased(TradeBound*) {}
PyResult TradeService::InitiateTrade(PyCallArgs& call,PyInt* characterID) {
    auto target=sEntityList.FindClientByCharID(characterID->value());
    if (!target || target==call.client || !call.client->IsDocked() || !target->IsDocked() || call.client->GetStationID()!=target->GetStationID()) throw CustomError("The other trader must be in your station.");
    if (call.client->GetTradeSession() || target->GetTradeSession()) throw CustomError("One of the traders is already trading.");
    InitiateTradeRsp_NoCash result;result.nodeID=GetServiceManager().GetNodeID();result.stationID=target->GetStationID();result.myID=call.client->GetCharacterID();result.herID=target->GetCharacterID();result.money=0;result.state=0;result.when=GetFileTimeNow();
    auto response=result.Encode();auto packet=new PyTuple(3);packet->SetItem(0,new PyString("Initiate"));packet->SetItem(1,new PyInt(call.client->GetCharacterID()));packet->SetItem(2,response->Clone());call.client->SendNotification("OnTrade","charid",&packet);return response;
}
void TradeService::RemoveActiveSession(uint32 id) {m_activeSessions.erase(id);}
uint32 TradeService::GetTradeSessionID() {
    for (uint32 tries=0;tries<=maxTradeCont-minTradeCont;++tries) {
        m_SessionID=m_SessionID<maxTradeCont ? m_SessionID+1 : minTradeCont;
        bool used=false;for (const auto& active:m_activeSessions) if (active.second.contID==m_SessionID) {used=true;break;}
        if (!used) return m_SessionID;
    }
    throw CustomError("No trade session identifiers are available.");
}
