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
    Author:        Luck
    Updates:    Allan
*/

#ifndef __TRADE_SERVICE_H_INCL__
#define __TRADE_SERVICE_H_INCL__

#include "services/BoundService.h"
#include "packets/Trade.h"

class Client;
class TradeBound;

class TradeService : public BindableService <TradeService, TradeBound>
{
public:
    TradeService(EVEServiceManager& mgr);

    void BoundReleased (TradeBound* bound) override;

    uint32 GetTradeSessionID();

    void RemoveActiveSession(uint32 myID);

    void CancelTrade(Client* pClient);
    void TransferContainerContents(SystemManager* pSysMgr, InventoryItemRef itemRef, uint32 newOwnerID);

    struct ActiveSession {
        uint32 myID;
        uint32 herID;
        uint32 contID;
        TradeSession* ourTS;
    };
    std::map<uint32, ActiveSession> m_activeSessions;      // mapped as myID, ActiveSession

protected:
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters) override;

    PyResult InitiateTrade(PyCallArgs& call, PyInt* characterID);

    uint32 m_SessionID;
};

class TradeSession
{
    friend class TradeBound;
    friend class TradeService;

public:
    TradeSession() {}
    ~TradeSession() {}

    struct Session {
        uint32 containerID;
        uint32 stationID;
        uint32 myID;
        uint32 herID;
        bool myState;
        bool herState;
        double myMoney;
        double herMoney;
        int64 fileTime;
    };

    struct TradeItems {
        uint32 itemID;
        uint32 typeID;
        uint32 ownerID;
        uint32 locationID;
        uint32 flagID;
        uint32 quantity;
        uint32 groupID;
        uint32 categoryID;
        bool singleton;
        std::string customInfo;
    };

    Session m_tradeSession;         // 2 party trade session of this bound object

    std::list<TradeItems> m_tradelist;    //list of items for this trade session

};

class TradeBound : public EVEBoundObject <TradeBound>
{
    friend TradeService;

public:
    TradeBound(EVEServiceManager& mgr, TradeService& parent);

    void ExchangeItems(Client* pClient, Client* pOther, TradeSession* pTSes);
    void CancelTrade(Client* pClient, Client* pOther, TradeSession* pTSes);

protected:
    TradeService* m_TSvc;                // get registered TradeService object

    bool CanClientCall(Client* client) override;

    PyResult OfferMoney(PyCallArgs& call, PyFloat* amount);
    PyResult Abort(PyCallArgs& call);
    PyResult ToggleAccept(PyCallArgs& call, PyBool* newAccept);
    PyResult GetItemID(PyCallArgs& call);
    PyResult Add(PyCallArgs& call, PyInt* itemID, PyInt* containerID);
    PyResult MultiAdd(PyCallArgs& call, PyList* itemIDs, PyInt* containerID);
    PyResult GetItem(PyCallArgs& call);
    PyResult IsCEOTrade(PyCallArgs& call);
    PyResult List(PyCallArgs& call);
    PyResult InitiateTrade(PyCallArgs& call, PyInt* characterID);
};

#endif//__TRADE_SERVICE_H_INCL__
