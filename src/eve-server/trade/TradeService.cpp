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
	Author:		Luck
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(TradeService);

class TradeBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(TradeBound)

    TradeBound(PyServiceMgr *mgr)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "TradeBound";

        PyCallable_REG_CALL(TradeBound, List)
    }
    virtual ~TradeBound()
	{
		delete m_dispatch;
	}

    virtual void Release() {
        //this needs to die
        delete this;
    }

    PyCallable_DECL_CALL(List)

protected:
    Dispatcher *const m_dispatch;

};


TradeService::TradeService(PyServiceMgr *mgr)
: PyService(mgr, "trademgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(TradeService, InitiateTrade)
}

TradeService::~TradeService() {
	delete m_dispatch;
}

PyBoundObject *TradeService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    Trade_BindArgs args;
    //temp crap until I rework _CreateBoundObject's signature
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", c->GetName());
        return NULL;
    }
    _log(CLIENT__MESSAGE, "Trade bind request for:");
    args.Dump(CLIENT__MESSAGE, "    ");

    return(new TradeBound(m_manager));
}

PyResult TradeService::Handle_InitiateTrade(PyCallArgs &call){
	
	Call_SingleIntegerArg args;

    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

	Client *target = call.client->services().entity_list.FindCharacter( args.arg );

	InitiateTradeRsp rsp;
	
    rsp.nodeID = call.client->services().GetNodeID();
	rsp.stationID = target->GetStationID();
	rsp.ownerID = call.client->GetCharacterID();
	rsp.targetID = target->GetCharacterID();
	rsp.moneyToGive = 0;
	rsp.moneyToReceive = 0;
	rsp.when = Win32TimeNow();
	rsp.unknown7 = 0;

	sLog.Debug("Trade Service", "Called InitiateTrade with character: %u", args.arg);

	return rsp.Encode();
}
PyResult TradeBound::Handle_List(PyCallArgs &call) {

    TradeListRsp tradeListResponse; 
	sLog.Debug("TradeBound", "Called List stub");
	return tradeListResponse.Encode();
}
