
 /**
  * @name MarketBot.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  */


#ifndef EVEMU_MARKET_MARKETBOT_H_
#define EVEMU_MARKET_MARKETBOT_H_


#include "eve-compat.h"
#include "eve-common.h"

#include "market/MarketBotMgr.h"

class MarketBot
{
public:
    MarketBot();
    ~MarketBot() { /* do nothing here */ }

    void Init();
    void Process();

private:
    bool m_initalized;
};

#endif  // EVEMU_MARKET_MARKETBOT_H_