
 /**
  * @name MarketBotConf.h
  *   system for automating/emulating buy and sell orders on the market.
  * base config code taken from EVEServerConfig
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  * @version:  0.15
  */



#include "market/MarketBotConf.h"

MarketBotConf::MarketBotConf()
{
    // register needed parsers
    AddMemberParser( "marketBot", &MarketBotConf::ProcessBotConf );

    // Set sane defaults
    // items with a "N" behind them are NOT implemented
    // items with a "P" behind them are PARTIALLY implemented
    // items with /*x*/ behind them denote time idetifier, with x = (s=seconds, m=minutes, etc)

    // main
    main.EnableRegional = false;//N
    main.EnableConst = false;//N
    main.DataRefreshTime = 15/*m*/;//N
    main.OrderLifetime = 5/*d*/;//N
    main.OrdersPerRefresh = 10;//N
    main.MaxISKPerOrder = 1500000000;//N

    // buy
    buy.RegionJumps = 10;//N
    buy.ConstJumps = 8;//N
    buy.SystemJumps = 5;//N
    buy.OrdersPerRegion = 35;//N
    buy.OrdersPerConst = 20;//N
    buy.OrdersPerSystem = 10;//N
    buy.DupeOrdersPerRegion = 10;//N
    buy.DupeOrdersPerConst = 5;//N
    buy.DupeOrdersPerSystem = 2;//N
    buy.MinBuyAmount = 1;//N

    // sell
    sell.SellNamedItem = false;//N
    sell.OrdersPerRegion = 20;//N
    sell.OrdersPerConst = 10;//N
    sell.OrdersPerSystem = 2;//N
    sell.DupeOrdersPerRegion = 5;//N
    sell.DupeOrdersPerConst = 3;//N
    sell.DupeOrdersPerSystem = 1;//N
    sell.SellItemMetaLevelMin = 0;//N
    sell.SellItemMetaLevelMax = 4;//N
    sell.MinSellAmount = 1;//N
}

bool MarketBotConf::ProcessBotConf(const TiXmlElement* ele)
{
    // entering element, extend allowed syntax
    AddMemberParser( "main",      &MarketBotConf::ProcessMain );
    AddMemberParser( "buy",       &MarketBotConf::ProcessBuy );
    AddMemberParser( "sell",      &MarketBotConf::ProcessSell );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "main" );
    RemoveParser( "buy" );
    RemoveParser( "sell" );

    // return status of parsing
    return result;
}

bool MarketBotConf::ProcessMain(const TiXmlElement* ele)
{
    AddValueParser( "EnableRegional",           main.EnableRegional );
    AddValueParser( "EnableConst",              main.EnableConst );
    AddValueParser( "DataRefreshTime",          main.DataRefreshTime );
    AddValueParser( "OrderLifetime",            main.OrderLifetime );
    AddValueParser( "OrdersPerRefresh",         main.OrdersPerRefresh );
    AddValueParser( "MaxISKPerOrder",           main.MaxISKPerOrder );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "EnableRegional" );
    RemoveParser( "EnableConst" );
    RemoveParser( "DataRefreshTime" );
    RemoveParser( "OrderLifetime" );
    RemoveParser( "OrdersPerRefresh" );
    RemoveParser( "MaxISKPerOrder" );

    return result;
}

bool MarketBotConf::ProcessBuy(const TiXmlElement* ele)
{
    AddValueParser( "MinBuyAmount",             buy.MinBuyAmount );
    AddValueParser( "RegionJumps",              buy.RegionJumps );
    AddValueParser( "ConstJumps",               buy.ConstJumps );
    AddValueParser( "SystemJumps",              buy.SystemJumps );
    AddValueParser( "OrdersPerRegion",          buy.OrdersPerRegion );
    AddValueParser( "OrdersPerConst",           buy.OrdersPerConst );
    AddValueParser( "OrdersPerSystem",          buy.OrdersPerSystem );
    AddValueParser( "DupeOrdersPerRegion",      buy.DupeOrdersPerRegion );
    AddValueParser( "DupeOrdersPerConst",       buy.DupeOrdersPerConst );
    AddValueParser( "DupeOrdersPerSystem",      buy.DupeOrdersPerSystem );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "MinBuyAmount" );
    RemoveParser( "RegionJumps" );
    RemoveParser( "ConstJumps" );
    RemoveParser( "SystemJumps" );
    RemoveParser( "OrdersPerRegion" );
    RemoveParser( "OrdersPerConst" );
    RemoveParser( "OrdersPerSystem" );
    RemoveParser( "DupeOrdersPerRegion" );
    RemoveParser( "DupeOrdersPerConst" );
    RemoveParser( "DupeOrdersPerSystem" );

    return result;
}

bool MarketBotConf::ProcessSell(const TiXmlElement* ele)
{
    AddValueParser( "SellNamedItem",            sell.SellNamedItem );
    AddValueParser( "MinSellAmount",            sell.MinSellAmount );
    AddValueParser( "OrdersPerRegion",          sell.OrdersPerRegion );
    AddValueParser( "OrdersPerConst",           sell.OrdersPerConst );
    AddValueParser( "OrdersPerSystem",          sell.OrdersPerSystem );
    AddValueParser( "DupeOrdersPerRegion",      sell.DupeOrdersPerRegion );
    AddValueParser( "DupeOrdersPerConst",       sell.DupeOrdersPerConst );
    AddValueParser( "DupeOrdersPerSystem",      sell.DupeOrdersPerSystem );
    AddValueParser( "SellItemMetaLevelMin",     sell.SellItemMetaLevelMin );
    AddValueParser( "SellItemMetaLevelMax",     sell.SellItemMetaLevelMax );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "SellNamedItem" );
    RemoveParser( "MinSellAmount" );
    RemoveParser( "OrdersPerRegion" );
    RemoveParser( "OrdersPerConst" );
    RemoveParser( "OrdersPerSystem" );
    RemoveParser( "DupeOrdersPerRegion" );
    RemoveParser( "DupeOrdersPerConst" );
    RemoveParser( "DupeOrdersPerSystem" );
    RemoveParser( "SellItemMetaLevelMax" );
    RemoveParser( "SellItemMetaLevelMin" );

    return result;
}

