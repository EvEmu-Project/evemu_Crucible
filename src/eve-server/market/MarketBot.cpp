
 /**
  * @name MarketBotMgr.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  */

#include "eve-server.h"
#include "EVEServerConfig.h"
#include "market/MarketBot.h"
#include "market/MarketMgr.h"
#include "market/MarketProxyService.h"


MarketBot::MarketBot()
{
    m_initalized = false;
}

void MarketBot::Init()
{
    m_initalized = false;
}

void MarketBot::Process()
{
    if (!m_initalized)
        return;
}

