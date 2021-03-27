
 /**
  * @name MarketBotMgr.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  * @version:  0.15 (config version)
  */

#include "eve-server.h"
#include "EVEServerConfig.h"
#include "market/MarketBotConf.h"
#include "market/MarketBotMgr.h"
#include "market/MarketMgr.h"
#include "market/MarketProxyService.h"


static const char* const BOT_CONFIG_FILE = EVEMU_ROOT "/etc/MarketBot.xml";


MarketBotDataMgr::MarketBotDataMgr()
{
    m_initalized = false;
}

int MarketBotDataMgr::Initialize()
{
    m_initalized = true;

    /* load current data */

    sLog.Blue(" MarketBotDataMgr", "Market Bot Data Manager Initialized.");
    return 1;
}



MarketBotMgr::MarketBotMgr()
: m_updateTimer(20 *60 *1000)    // arbitrary 20m default
{
    m_initalized = false;
}

int MarketBotMgr::Initialize()
{
    if (!sMBotConf.ParseFile(BOT_CONFIG_FILE)) {
        sLog.Error("       ServerInit", "Loading Market Bot Config file '%s' failed.", BOT_CONFIG_FILE);
        return 0;
    }

    m_initalized = true;
    sMktBotDataMgr.Initialize();

    /* start timers, process current data, and create new orders, if needed */

    sLog.Blue("     MarketBotMgr", "Market Bot Manager Initialized.");
    return 1;
}

// this is called on a minute timer from EntityList
void MarketBotMgr::Process()
{
    if (!m_initalized)
        return;

    if (m_updateTimer.Check()) {
    /* process current data, process orders, xfer funds, reset timers, create new orders */

    }
}


void MarketBotMgr::AddSystem()
{

}

void MarketBotMgr::RemoveSystem()
{

}

