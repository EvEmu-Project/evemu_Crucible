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
	Author:		Zhur
*/

#ifndef __MARKETPROXY_SERVICE_H_INCL__
#define __MARKETPROXY_SERVICE_H_INCL__

#include "market/MarketDB.h"
#include "PyService.h"

class MarketProxyService
: public PyService
{
public:
	MarketProxyService(PyServiceMgr *mgr);
	virtual ~MarketProxyService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;
	
	MarketDB m_db;
	
	PyCallable_DECL_CALL(GetStationAsks)
	PyCallable_DECL_CALL(GetSystemAsks)
	PyCallable_DECL_CALL(GetRegionBest)
	PyCallable_DECL_CALL(GetMarketGroups)
	PyCallable_DECL_CALL(GetOrders)
	PyCallable_DECL_CALL(GetOldPriceHistory)
	PyCallable_DECL_CALL(GetNewPriceHistory)
	PyCallable_DECL_CALL(PlaceCharOrder)
	PyCallable_DECL_CALL(GetCharOrders)
    PyCallable_DECL_CALL(ModifyCharOrder)
    PyCallable_DECL_CALL(CancelCharOrder)
    PyCallable_DECL_CALL(CharGetNewTransactions)
	PyCallable_DECL_CALL(StartupCheck)
	//PyCallable_DECL_CALL(GetCorporationOrders) //()
	
	void _ExecuteBuyOrder(uint32 buy_order_id, uint32 stationID, uint32 quantity, Client *seller, InventoryItemRef item, bool isCorp);
	void _ExecuteSellOrder(uint32 sell_order_id, uint32 stationID, uint32 quantity, Client *buyer, bool isCorp);
	void _SendOnOwnOrderChanged(Client *who, uint32 orderID, const char *action, bool isCorp, PyRep* order = NULL);
    void _BroadcastOnOwnOrderChanged(uint32 regionID, uint32 orderID, const char *action, bool isCorp, PyRep* order = NULL);
    void _SendOnMarketRefresh(Client *who);
    void _BroadcastOnMarketRefresh(uint32 regionID);
    void _InvalidateOrdersCache(uint32 regionID);
	
	
	//overloaded in order to support bound objects:
	//virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
};





#endif


