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

#include <unordered_map>

#include "EntityList.h"


#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "station/TradeService.h"
#include "system/SystemManager.h"
#include "system/Container.h"

TradeBound::TradeBound (EVEServiceManager& mgr, TradeService& parent) :
    EVEBoundObject(mgr, parent)
{
    this->m_TSvc = this->GetServiceManager().Lookup <TradeService>("trademgr");

    EVEBoundObject::Add("List", &TradeBound::List);
    EVEBoundObject::Add("IsCEOTrade", &TradeBound::IsCEOTrade);
    EVEBoundObject::Add("GetItemID", &TradeBound::GetItemID);
    EVEBoundObject::Add("GetItem", &TradeBound::GetItem);
    EVEBoundObject::Add("Add", &TradeBound::Add);
    EVEBoundObject::Add("MultiAdd", &TradeBound::MultiAdd);
    EVEBoundObject::Add("ToggleAccept", &TradeBound::ToggleAccept);
    EVEBoundObject::Add("OfferMoney", &TradeBound::OfferMoney);
    EVEBoundObject::Add("Abort", &TradeBound::Abort);
}

PyResult TradeBound::OfferMoney(PyCallArgs &call, PyFloat* amount) {
    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
    PyList* list = new PyList(2);

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
        pTSes->m_tradeSession.myMoney = amount->value();
        list->SetItem(0, new PyFloat(pTSes->m_tradeSession.myMoney));  //myMoney
        list->SetItem(1, new PyFloat(pTSes->m_tradeSession.herMoney)); //herMoney
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
        pTSes->m_tradeSession.herMoney = amount->value();
        list->SetItem(0, new PyFloat(pTSes->m_tradeSession.myMoney));  //myMoney
        list->SetItem(1, new PyFloat(pTSes->m_tradeSession.herMoney)); //herMoney
    } else {
        _log(CLIENT__ERROR, "TradeBound::Handle_OfferMoney() : %s(%u) - clients are neither mine nor hers.", \
                call.client->GetName(), call.client->GetCharacterID());
        PySafeDecRef(list);
        return PyStatic.NewNone();
    }

    //  reset states after offer changes..
    pTSes->m_tradeSession.myState  = false;
    pTSes->m_tradeSession.herState = false;
    // send changes
    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyString("MoneyOffer"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
        tuple->SetItem(2, list);
    PyIncRef(tuple);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple);

    // cleanup
    PySafeDecRef(tuple);
    PySafeDecRef(list);

    // returns none
    return PyStatic.NewNone();
}

PyResult TradeBound::Abort(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    CancelTrade(pClient, pOther, pTSes);

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("Cancel"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    PyIncRef(tuple);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple);
    m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.myID);
    m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.herID);
    SafeDelete(pTSes);
    pClient->ClearTradeSession();
    pOther->ClearTradeSession();
    // returns none
    return PyStatic.NewNone();
}

void TradeBound::CancelTrade(Client* pClient, Client* pOther, TradeSession* pTSes)
{
    // trade canceled.  send items back to owner. (monies not taken at this point)
    //PyDict* dict = new PyDict();
    //dict->SetItem(new PyInt(Inv::Update::Location), new PyInt(pTSes->m_tradeSession.containerID));

    uint32 stationID = pTSes->m_tradeSession.stationID;
    for (auto cur : pTSes->m_tradelist) {
        InventoryItemRef itemRef = sItemFactory.GetItemRef(cur.itemID);
        if (itemRef.get() == nullptr)  {
            _log(PLAYER__ERROR, "TradeBound::CancelTrade() - Failed to get ItemRef.");
            continue;
        }

        itemRef->Move(stationID, flagHangar, true);
    }
}

PyResult TradeBound::ToggleAccept(PyCallArgs &call, PyBool* newAccept) {
    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient(nullptr);
    Client* pOther(nullptr);

    bool myAccept = pTSes->m_tradeSession.myState;
    bool herAccept = pTSes->m_tradeSession.herState;

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
        pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
        pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
        myAccept = newAccept->value();
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
        pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
        pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
        herAccept = newAccept->value();
    } else {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_ToggleAccept() : %s(%u) - clients are neither mine nor hers.", \
                    call.client->GetName(), call.client->GetCharacterID());
        return PyStatic.NewNone();
    }

    bool forceTrade = false;
    if (call.byname.find("forceTrade") != call.byname.cend())
        if (!call.byname.find("forceTrade")->second->IsNone())
            forceTrade = call.byname.find("forceTrade")->second->AsBool()->value();

    if (forceTrade) {
        pTSes->m_tradeSession.myState  = true;
        pTSes->m_tradeSession.herState = true;
    } else {
        pTSes->m_tradeSession.myState  = myAccept;
        pTSes->m_tradeSession.herState = herAccept;
    }

    _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_ToggleAccept() is now %s/%s. forceTrade is %s.", \
                (myAccept ? "true" : "false"), (herAccept ? "true" : "false"), (forceTrade ? "true" : "false"));

    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyString("StateToggle"));
        tuple->SetItem(1, new PyBool(myAccept));
        tuple->SetItem(2, new PyBool(herAccept));
    PyIncRef(tuple);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple);

    if (myAccept && herAccept) {
        ExchangeItems(pClient, pOther, pTSes);      // trade completed.
        m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.myID);
        m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.herID);
        pClient->ClearTradeSession();
        pOther->ClearTradeSession();
    }

    // returns none
    return PyStatic.NewNone();
}

PyResult TradeBound::GetItemID(PyCallArgs &call) {
    _log(CLIENT__CALL_DUMP, "TradeBound::Handle_GetItemID()");
    call.Dump(CLIENT__CALL_DUMP);
    // still not sure what this does...only returns PyNone in packet logs.
    // returns none
    return PyStatic.NewNone();
}

/** @todo  refresh other window when item added */
PyResult TradeBound::Add(PyCallArgs &call, PyInt* itemID, PyInt* containerID) {
    InventoryItemRef itemRef = sItemFactory.GetItemRef(itemID->value());
    if (itemRef.get() == nullptr)  {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_Add() - Failed to get ItemRef.");
        //  should i abort trade, or just return null here?  single add, so not a big deal.
        //     return null, let them try again if they want.  maybe later add config option?
        //Handle_Abort(call);   << this will cancel and nullify the trade session
        return PyStatic.NewNone();
    }

    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient(sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID));
    Client* pOther(sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID));

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
    } else {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_Add() : %s(%u) & %s(%u) - clients are neither mine nor hers.", \
                pClient->GetName(), pClient->GetCharacterID(), pOther->GetName(), pOther->GetCharacterID());
        return PyStatic.NewNone();
    }

    uint32 flag(0);
    if (call.byname.find("flag") != call.byname.cend())
        flag = PyRep::IntegerValueU32(call.byname.find("flag")->second);
    uint32 qty(0);
    if (call.byname.find("qty") != call.byname.cend())
        qty = PyRep::IntegerValue(call.byname.find("qty")->second);

    uint32 tradeContainerID(pTSes->m_tradeSession.containerID);
    TradeSession::TradeItems mTI = TradeSession::TradeItems();
        mTI.itemID = itemID->value();
        mTI.typeID = itemRef->typeID();
        mTI.ownerID = call.client->GetCharacterID();
        mTI.locationID = tradeContainerID;
        mTI.flagID = itemRef->flag();
        mTI.quantity = qty;
        mTI.groupID = itemRef->groupID();
        mTI.singleton = itemRef->isSingleton();
        mTI.categoryID = itemRef->categoryID();
        mTI.customInfo = "";
        pTSes->m_tradelist.insert(pTSes->m_tradelist.end(), mTI);

    itemRef->Move(tradeContainerID, (EVEItemFlags)flag, true);

    PyDict* dict = new PyDict();
        dict->SetItem(new PyInt(Inv::Update::Location), containerID);

    PyPackedRow* row = new PyPackedRow( sDataMgr.CreateHeader() );
        row->SetField("itemID",        new PyLong(mTI.itemID));
        row->SetField("typeID",        new PyInt(mTI.typeID));
        row->SetField("ownerID",       new PyInt(mTI.ownerID));
        row->SetField("locationID",    new PyInt(mTI.locationID));
        row->SetField("flagID",        new PyInt(mTI.flagID));
        row->SetField("groupID",       new PyInt(mTI.groupID));
        row->SetField("categoryID",    new PyInt(mTI.categoryID));
        row->SetField("quantity",      new PyInt(mTI.singleton?-1:mTI.quantity));
        row->SetField("customInfo",    new PyString(mTI.customInfo));

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, row);
        tuple->SetItem(1, dict);
    PyIncRef(tuple);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnItemChange", "charid", &tuple);
    pOther->SendNotification("OnItemChange", "charid", &tuple);

    //  reset states after offer changes..
    pTSes->m_tradeSession.myState  = false;
    pTSes->m_tradeSession.herState = false;
    // return none
    return PyStatic.NewNone();
}

PyResult TradeBound::MultiAdd(PyCallArgs &call, PyList* itemIDs, PyInt* containerID) {
    /*  .ints = list of itemIDs to insert into trade
     *  .contID = item's current containerID
     *  call.byname "flag"
     */
    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = itemIDs->begin();
    for (size_t list_2_index(0); list_2_cur != itemIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode TradeMultiAddList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    uint32 flag(0);
    if (call.byname.find("flag") != call.byname.end())
        flag = PyRep::IntegerValueU32(call.byname.find("flag")->second);

    PyDict* dict = new PyDict();
        dict->SetItem(new PyInt(Inv::Update::Location), containerID);

    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    if ((call.client->GetCharacterID() != pTSes->m_tradeSession.myID)
    and (call.client->GetCharacterID() != pTSes->m_tradeSession.herID)) {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_MultiAdd() : %s(%u) & %s(%u) - clients are neither mine nor hers.", \
                pClient->GetName(), pClient->GetCharacterID(), pOther->GetName(), pOther->GetCharacterID());
        PySafeDecRef(dict);
        return PyStatic.NewNone();
    }

    uint32 charID = call.client->GetCharacterID();
    uint32 tradeContID = pTSes->m_tradeSession.containerID;

    DBRowDescriptor* header = sDataMgr.CreateHeader();
    std::vector<int32> list = ints;
    for (auto cur : list) {
        InventoryItemRef itemRef = sItemFactory.GetItemRef(cur);
        if (itemRef.get() == nullptr)  {
            _log(PLAYER__ERROR, "TradeBound::Handle_Add() - Failed to get ItemRef.");
            continue;
        }

        /** @todo figure out how to update this to avoid multiple create/sends in this loop */
        TradeSession::TradeItems mTI = TradeSession::TradeItems();
            mTI.itemID = cur;
            mTI.typeID = itemRef->typeID();
            mTI.ownerID = charID;
            mTI.locationID = tradeContID;
            mTI.flagID = flag;
            mTI.quantity = itemRef->quantity();
            mTI.groupID = itemRef->groupID();
            mTI.singleton = itemRef->isSingleton();
            mTI.categoryID = itemRef->categoryID();
            mTI.customInfo = "";
        pTSes->m_tradelist.insert(pTSes->m_tradelist.end(), mTI);
        itemRef->Move(tradeContID, (EVEItemFlags)flag, true);

        PyPackedRow* row = new PyPackedRow(header);
            row->SetField("itemID",        new PyLong(mTI.itemID));
            row->SetField("typeID",        new PyInt(mTI.typeID));
            row->SetField("ownerID",       new PyInt(mTI.ownerID));
            row->SetField("locationID",    new PyInt(mTI.locationID));
            row->SetField("flagID",        new PyInt(mTI.flagID));
            row->SetField("groupID",       new PyInt(mTI.groupID));
            row->SetField("categoryID",    new PyInt(mTI.categoryID));
            row->SetField("quantity",      new PyInt(mTI.singleton?-1:mTI.quantity));
            row->SetField("customInfo",    new PyString(mTI.customInfo));
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, row);
            tuple->SetItem(1, dict);
        PyIncRef(tuple);
        // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
        pClient->SendNotification("OnItemChange", "charid", &tuple);
        pOther->SendNotification("OnItemChange", "charid", &tuple);
    }

    //  reset states after offer changes.
    pTSes->m_tradeSession.myState  = false;
    pTSes->m_tradeSession.herState = false;

    // cleanup
    PySafeDecRef(dict);
    PySafeDecRef(header);

    // return none
    return PyStatic.NewNone();
}

PyResult TradeBound::GetItem(PyCallArgs &call) {
    // TODO: this might need parameters but I'm not sure
    TradeSession* pTSes = call.client->GetTradeSession();
    PyPackedRow* row = new PyPackedRow( sDataMgr.CreateHeader() );
        row->SetField("itemID",        new PyLong(pTSes->m_tradeSession.containerID));
        row->SetField("typeID",        new PyInt(53));     // type Trade Window
        row->SetField("ownerID",       PyStatic.NewOne());      // EvE_System
        row->SetField("locationID",    new PyInt(pTSes->m_tradeSession.stationID));
        row->SetField("flagID",        PyStatic.NewNone());
        row->SetField("quantity",      new PyInt(-1));     // singleton
        row->SetField("groupID",       new PyInt(EVEDB::invGroups::Trade_Session ) );
        row->SetField("categoryID",    new PyInt(EVEDB::invCategories::Trading));
        row->SetField("customInfo",    PyStatic.NewNone());
    return row;
}

PyResult TradeBound::IsCEOTrade(PyCallArgs &call) {
    _log(CLIENT__CALL_DUMP, "TradeBound::Handle_IsCEOTrade()");
    call.Dump(CLIENT__CALL_DUMP);

    //TODO will have to work on this later.  need corps working correctly first.
    return new PyBool(false);
}

PyResult TradeBound::List(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    PyList* list = new PyList();

    DBRowDescriptor* header = sDataMgr.CreateHeader();
    for (auto cur : pTSes->m_tradelist) {
        PyPackedRow* row = new PyPackedRow( header );
            row->SetField("itemID",        new PyLong(cur.itemID));
            row->SetField("typeID",        new PyInt(cur.typeID));
            row->SetField("ownerID",       new PyInt(cur.ownerID));
            row->SetField("locationID",    new PyInt(cur.locationID));
            row->SetField("flagID",        new PyInt(cur.flagID));
            row->SetField("groupID",       new PyInt(cur.groupID));
            row->SetField("quantity",      new PyInt(cur.singleton?-1:cur.quantity));
            row->SetField("categoryID",    new PyInt(cur.categoryID));
            row->SetField("customInfo",    new PyString(cur.customInfo));
        list->AddItem(row);
    }

    PyTuple* tuple2 = new PyTuple(1);
        tuple2->SetItem(0, list);
    PyToken* token = new PyToken("__builtin__.set");
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, token);
        tuple->SetItem(1, tuple2);
    TradeListData tld;
        tld.tradeContainerID = pTSes->m_tradeSession.containerID;
        tld.myID             = pTSes->m_tradeSession.myID;
        tld.herID            = pTSes->m_tradeSession.herID;
        tld.myMoney          = pTSes->m_tradeSession.myMoney;
        tld.herMoney         = pTSes->m_tradeSession.herMoney;
        tld.myState          = pTSes->m_tradeSession.myState;
        tld.herState         = pTSes->m_tradeSession.herState;
        tld.list             = new PyObjectEx(false, tuple);
    PyList* itemNames = new PyList(5);
        itemNames->SetItem(0, new PyString("tradeContainerID"));
        itemNames->SetItem(1, new PyString("traders"));
        itemNames->SetItem(2, new PyString("state"));
        itemNames->SetItem(3, new PyString("money"));
        itemNames->SetItem(4, new PyString("items"));
    TradeListRsp tlr;
        tlr.header  = itemNames;
        tlr.line    = tld.Encode();
    return tlr.Encode();
}

void TradeBound::ExchangeItems(Client* pClient, Client* pOther, TradeSession* pTSes) {
    // trade completed.  perform item and money exchange
    if (pClient->GetCharacterID() == pTSes->m_tradeSession.herID) {
        pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
        pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    }
    // transfer funds and add journal entries for both sides
    std::string reason = "Player Trade between ";
    reason += pClient->GetCharName();
    reason += " and ";
    reason += pOther->GetCharName();
    reason += " in ";
    reason += pClient->GetSystemName();     // use system name or station name here?
    AccountService::TranserFunds(pClient->GetCharacterID(), pOther->GetCharacterID(), pTSes->m_tradeSession.myMoney, reason, Journal::EntryType::PlayerTrading, pClient->GetStationID());
    AccountService::TranserFunds(pOther->GetCharacterID(), pClient->GetCharacterID(), pTSes->m_tradeSession.herMoney, reason, Journal::EntryType::PlayerTrading, pClient->GetStationID());

    //PyDict* dict = new PyDict();
    //    dict->SetItem(new PyInt(Inv::Update::Location), new PyInt(pTSes->m_tradeSession.containerID));

    uint32 stationID = pTSes->m_tradeSession.stationID;
    for (auto cur : pTSes->m_tradelist) {
        InventoryItemRef itemRef = sItemFactory.GetItemRef(cur.itemID);
        if (!itemRef)  {
            _log(PLAYER__ERROR, "TradeBound::Handle_Add() - Failed to get ItemRef.");
            continue;
        }

        uint32 newOwnerID = (cur.ownerID == pTSes->m_tradeSession.myID ? pTSes->m_tradeSession.herID : pTSes->m_tradeSession.myID);
        itemRef->ChangeOwner(newOwnerID, true);
        itemRef->Move(stationID, flagHangar, true);

        if ((itemRef->categoryID() == EVEDB::invCategories::Ship)
            || (itemRef->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
            || (itemRef->groupID() == EVEDB::invGroups::Cargo_Container)
            || (itemRef->groupID() == EVEDB::invGroups::Freight_Container)
            || (itemRef->groupID() == EVEDB::invGroups::Secure_Cargo_Container))
            m_TSvc->TransferContainerContents(pClient->SystemMgr(), itemRef, newOwnerID);
    }

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("TradeComplete"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    PyIncRef(tuple);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple);
}

TradeService::TradeService(EVEServiceManager& mgr)
    : BindableService("trademgr", mgr, eAccessLevel_Station)
{
    m_SessionID = minTradeCont;

    this->Add("InitiateTrade", &TradeService::InitiateTrade);
}

BoundDispatcher* TradeService::BindObject(Client* client, PyRep* bindParameters) {
    // each client's trade session has it's own bound object.
    //   create code for multiple sessions per client, using TradeBound and TradeSession.
    Trade_BindArgs args;
    //crap
    PyRep* tmp(bindParameters->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode bind args.", GetName());
        return nullptr;
    }

    _log(COLLECT__OTHER_DUMP, "Trade bind request for:");
    args.Dump(COLLECT__OTHER_DUMP, "    ");

    /** @todo update to multiple trade sessions per client.  current code only allows one at a time. */

    // check to see if this is target calling for a bound object.  if not, create new session
    std::map<uint32, ActiveSession>::iterator itr = m_activeSessions.find(args.myID);
    if (itr == m_activeSessions.end()) {
        TradeSession* pTSes = new TradeSession();
        client->SetTradeSession(pTSes);
        uint32 contID(GetTradeSessionID());
        pTSes->m_tradeSession.containerID = contID;
        pTSes->m_tradeSession.stationID = args.stationID;
        pTSes->m_tradeSession.myID = args.myID;
        pTSes->m_tradeSession.herID = args.herID;
        pTSes->m_tradeSession.myState = false;
        pTSes->m_tradeSession.herState = false;
        pTSes->m_tradeSession.myMoney = args.myMoney;
        pTSes->m_tradeSession.herMoney = args.herMoney;
        pTSes->m_tradeSession.fileTime = args.fileTime;
        ActiveSession cAS = ActiveSession();
        cAS.myID = args.myID;
        cAS.herID = args.herID;
        cAS.contID = contID;
        cAS.ourTS = pTSes;
        m_activeSessions.insert(std::make_pair(args.myID, cAS));
        m_activeSessions.insert(std::make_pair(args.herID, cAS));
    } else {
        client->SetTradeSession(itr->second.ourTS);
    }

    return new TradeBound(this->GetServiceManager(), *this);
}

void TradeService::BoundReleased (TradeBound* bound) {

}

void TradeService::TransferContainerContents(SystemManager* pSysMgr, InventoryItemRef itemRef, uint32 newOwnerID)
{
    std::map<uint32, InventoryItemRef> InventoryMap;
    InventoryMap.clear();

    if (itemRef->categoryID() == EVEDB::invCategories::Ship) {
        //  if we change this to use shipItem, this will need rework
        ShipDB::DeleteInsuranceByShipID(itemRef->itemID());
        ShipItemRef shipRef = pSysMgr->GetShipFromInventory(itemRef->itemID());
        if (shipRef.get() == nullptr)
            shipRef = sItemFactory.GetShipRef(itemRef->itemID());
        if (!shipRef->GetMyInventory()->IsEmpty())
            shipRef->GetMyInventory()->GetInventoryMap(InventoryMap);
    } else {
        CargoContainerRef contRef = pSysMgr->GetContainerFromInventory(itemRef->itemID());
        if (contRef.get() == nullptr)
            contRef = sItemFactory.GetCargoRef(itemRef->itemID());
        if (!contRef->IsEmpty())
            contRef->GetMyInventory()->GetInventoryMap(InventoryMap);
    }

    for (auto cur : InventoryMap)
        cur.second->ChangeOwner(newOwnerID, true);
}

PyResult TradeService::InitiateTrade(PyCallArgs &call, PyInt* characterID) {
    Client* target(nullptr);
    if (call.client->GetTradeSession()) {
        target = sEntityList.FindClientByCharID( call.client->GetTradeSession()->m_tradeSession.herID );
        call.client->SendErrorMsg("You are currently trading with %s.  You can only trade with one player at a time.", target->GetName());
        return nullptr;
    }

    target = sEntityList.FindClientByCharID( characterID->value() );
    if (target->GetTradeSession()) {
        Client* otarget = sEntityList.FindClientByCharID( call.client->GetTradeSession()->m_tradeSession.herID );
        call.client->SendErrorMsg("%s is currently trading with %s.  Try again later.", target->GetName(), otarget->GetName());
        return nullptr;
    }

    // wtf is this ???
    uint32 warID(0);
    if (call.byname.find("warID") != call.byname.cend())
        warID = PyRep::IntegerValue(call.byname.find("warID")->second);

    InitiateTradeRsp_NoCash rsp_nc;
        rsp_nc.nodeID    = call.client->services().GetNodeID();
        rsp_nc.stationID = target->GetStationID();
        rsp_nc.myID      = call.client->GetCharacterID();
        rsp_nc.herID     = target->GetCharacterID();
        rsp_nc.money     = 0;
        rsp_nc.state     = 0;
        rsp_nc.when      = GetFileTimeNow();

    PyObject* resp = rsp_nc.Encode();
    PyTuple* tuple = new PyTuple(3);
    tuple->SetItem(0, new PyString("Initiate"));
    tuple->SetItem(1, new PyInt(call.client->GetCharacterID()));
    tuple->SetItem(2, resp->Clone());
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    call.client->SendNotification("OnTrade", "charid", &tuple);
    return resp;
}

void TradeService::RemoveActiveSession(uint32 myID) {
    m_activeSessions.erase(myID);
}

void TradeService::CancelTrade(Client* pClient) {
    TradeSession* pTSes = pClient->GetTradeSession();
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    TradeBound* pTB = new TradeBound(this->GetServiceManager(), *this);
    pTB->CancelTrade(pClient, pOther, pTSes);
    delete pTB;

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("Cancel"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    PyIncRef(tuple);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple);
    RemoveActiveSession(pTSes->m_tradeSession.myID);
    RemoveActiveSession(pTSes->m_tradeSession.herID);
    SafeDelete(pTSes);
    pClient->ClearTradeSession();
    pOther->ClearTradeSession();
    // returns none
}

uint32 TradeService::GetTradeSessionID()
{
    if (m_SessionID < maxTradeCont)
        return ++m_SessionID;

    return (m_SessionID = minTradeCont);
}
