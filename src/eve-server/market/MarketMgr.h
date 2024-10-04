
 /**
  * @name MarketMgr.h
  *   singleton object for storing, manipulating and managing in-game market data
  *
  * @Author:         Allan
  * @date:          19Dec17
  *
  */


#ifndef _EVE_SERVER_MARKET_MANAGER_H__
#define _EVE_SERVER_MARKET_MANAGER_H__


#include "../eve-server.h"

#include "EntityList.h"
#include "market/MarketDB.h"
#include "cache/ObjCacheService.h"

class Client;

class MarketMgr
: public Singleton< MarketMgr >
{
public:
    MarketMgr();
    ~MarketMgr();

    int Initialize(EVEServiceManager& svc);

    void Close();
    void GetInfo();
    void Process();

    void SystemStartup(SolarSystemData &data);
    void SystemShutdown(SolarSystemData &data);

    void UpdatePriceHistory();

    // fulfill market order placed by buyer to buy items (usually at reduced
    // prices).
    //
    // Updates qty in args based on order request.
    //
    // Returns true if the order is complete; false otherwise.
    bool ExecuteBuyOrder(Client* seller, uint32 orderID, InventoryItemRef iRef, uint32 quantity, bool useCorp, uint32 typeID, uint32 stationID, double price, uint16 accountKey = Account::KeyType::Cash);
    // market order placed by seller to sell items (usually at higher prices)
    void ExecuteSellOrder(Client *buyer, uint32 orderID, uint32 quantity, float price, uint32 stationID, uint32 typeID, bool useCorp);
    //forces a refresh of market data.
    void SendOnOwnOrderChanged(Client* pClient, uint32 orderID, uint8 action, bool isCorp = false, PyRep* order = nullptr);

    void InvalidateOrdersCache(uint32 regionID, uint32 typeID);

    bool NeedsUpdate()                                  { return m_timeStamp > GetFileTimeNow()?false:true; }

    PyRep* GetMarketGroups()                            { PyIncRef(m_marketGroups); return m_marketGroups; }
    // cached
    PyRep* GetNewPriceHistory(uint32 regionID, uint32 typeID);
    // cached
    PyRep* GetOldPriceHistory(uint32 regionID, uint32 typeID);


    // base price update method
    void SetBasePrice();         // this uses current mineral values to estimate base price of item
    void UpdateMineralPrice();
    void GetCruPrices();


protected:
    void Populate();

private:
    MarketDB m_db;
    ObjCacheService* m_cache;

    PyRep* m_marketGroups;  // static market group data

    int64 m_timeStamp;

    // markets are regional.  there are 66 regions.
    // market orders are stored as {regionID/typeID}
    //  load market data by region, sorted by system/station.
    //  station data will also store StationRef to owning/containing station for easier access later
    //  will be able to implement 'jumps' and other market conditionals
    //  ** this could take ~16m for average market data per region
};

//Singleton
#define sMktMgr \
( MarketMgr::get() )


#endif  // _EVE_SERVER_MARKET_MANAGER_H__

/*
 *    def GetStationDistance(self, stationID, getFastestRoute = True):
 *        if session.stationid == stationID:
 *            return -1
 *        station = sm.GetService('ui').GetStation(stationID)
 *        solarSystemID = station.solarSystemID
 *        regionID = sm.GetService('map').GetRegionForSolarSystem(solarSystemID)
 *        if regionID != session.regionid:
 *            return const.rangeRegion
 *        if getFastestRoute:
 *            jumps = sm.StartService('pathfinder').GetShortestJumpCountFromCurrent(solarSystemID)
 *        else:
 *            jumps = sm.StartService('pathfinder').GetJumpCountFromCurrent(solarSystemID)
 *        if jumps >= 1:
 *            return jumps
 *        return 0
 */

/*
 *    def GetSkillLimits(self):
 *        limits = {}
 *        currentOpen = 0
 *        myskills = sm.GetService('skills').MySkillLevelsByID()
 *        retailLevel = myskills.get(const.typeRetail, 0)
 *        tradeLevel = myskills.get(const.typeTrade, 0)
 *        wholeSaleLevel = myskills.get(const.typeWholesale, 0)
 *        accountingLevel = myskills.get(const.typeAccounting, 0)
 *        brokerLevel = myskills.get(const.typeBrokerRelations, 0)
 *        tycoonLevel = myskills.get(const.typeTycoon, 0)
 *        marginTradingLevel = myskills.get(const.typeMarginTrading, 0)
 *        marketingLevel = myskills.get(const.typeMarketing, 0)
 *        procurementLevel = myskills.get(const.typeProcurement, 0)
 *        visibilityLevel = myskills.get(const.typeVisibility, 0)
 *        daytradingLevel = myskills.get(const.typeDaytrading, 0)
 *        maxOrderCount = 5 + tradeLevel * 4 + retailLevel * 8 + wholeSaleLevel * 16 + tycoonLevel * 32
 *        limits['cnt'] = maxOrderCount
 *        commissionPercentage = const.marketCommissionPercentage / 100.0
 *        commissionPercentage *= 1 - brokerLevel * 0.05
 *        transactionTax = const.mktTransactionTax / 100.0
 *        transactionTax *= 1 - accountingLevel * 0.1
 *        limits['fee'] = commissionPercentage
 *        limits['acc'] = transactionTax
 *        limits['ask'] = jumpsPerSkillLevel[marketingLevel]
 *        limits['bid'] = jumpsPerSkillLevel[procurementLevel]
 *        limits['vis'] = jumpsPerSkillLevel[visibilityLevel]
 *        limits['mod'] = jumpsPerSkillLevel[daytradingLevel]
 *        limits['esc'] = 0.75 ** marginTradingLevel
 *        return limits
 */
