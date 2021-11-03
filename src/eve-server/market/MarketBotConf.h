
 /**
  * @name MarketBotConf.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  */

#ifndef EVEMU_MARKET_BOTCONFIG_H_
#define EVEMU_MARKET_BOTCONFIG_H_

#include "../eve-server.h"
#include "../../eve-core/utils/XMLParserEx.h"


class MarketBotConf
:public XMLParserEx,
 public Singleton< MarketBotConf >
{
public:
    MarketBotConf();
    ~MarketBotConf() { /* do nothing here */ }

    // From <main/>
    struct
    {
        bool EnableRegional;
        bool EnableConst;
        uint8 DataRefreshTime;
        uint8 OrderLifetime;
        uint16 OrdersPerRefresh;
        uint32 MaxISKPerOrder;
    } main;

    // From <buy/>
    struct
    {
        uint8 RegionJumps;
        uint8 ConstJumps;
        uint8 SystemJumps;
        uint8 OrdersPerRegion;
        uint8 OrdersPerConst;
        uint8 OrdersPerSystem;
        uint8 DupeOrdersPerRegion;
        uint8 DupeOrdersPerConst;
        uint8 DupeOrdersPerSystem;
        uint8 MinBuyAmount;
    } buy;

    // From <sell/>
    struct
    {
        bool SellNamedItem;
        uint8 OrdersPerRegion;
        uint8 OrdersPerConst;
        uint8 OrdersPerSystem;
        uint8 DupeOrdersPerRegion;
        uint8 DupeOrdersPerConst;
        uint8 DupeOrdersPerSystem;
        uint8 SellItemMetaLevelMin;
        uint8 SellItemMetaLevelMax;
        uint8 MinSellAmount;
    } sell;

protected:
    bool ProcessBotConf( const TiXmlElement* ele );
    bool ProcessMain( const TiXmlElement* ele );
    bool ProcessBuy( const TiXmlElement* ele );
    bool ProcessSell( const TiXmlElement* ele );
};


#define sMBotConf \
   (MarketBotConf::get())

#endif  // EVEMU_MARKET_BOTCONFIG_H_
