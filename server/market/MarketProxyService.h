/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __MARKETPROXY_SERVICE_H_INCL__
#define __MARKETPROXY_SERVICE_H_INCL__

#include "../PyService.h"

#include "MarketDB.h"

class MarketProxyService
: public PyService {
public:
	MarketProxyService(PyServiceMgr *mgr, DBcore *db);
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
	//PyCallable_DECL_CALL(GetCorporationOrders) //()
	
	void _ExecuteBuyOrder(uint32 buy_order_id, uint32 stationID, uint32 quantity, Client *seller, InventoryItem *item);
	void _ExecuteSellOrder(uint32 sell_order_id, uint32 stationID, uint32 quantity, Client *buyer);
	void _SendOnOwnOrderChanged(Client *who, uint32 orderID, const char *action);
	
	//overloaded in order to support bound objects:
	//virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
};





#endif


