
 /**
  * @name MarketBotMgr.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  * @version:  0.15 (config version)
  */


#ifndef EVEMU_MARKET_MARKETBOTMGR_H_
#define EVEMU_MARKET_MARKETBOTMGR_H_


#include "eve-compat.h"
#include "eve-common.h"
#include "utils/Singleton.h"

class MarketBotDataMgr
: public Singleton<MarketBotDataMgr>
{
public:
    MarketBotDataMgr();
    ~MarketBotDataMgr() { /* do nothing here */ }

    int Initialize();

private:
    bool m_initalized;
};

//Singleton
#define sMktBotDataMgr \
( MarketBotDataMgr::get() )


class MarketBotMgr
: public Singleton<MarketBotMgr>
{
public:
    MarketBotMgr();
    ~MarketBotMgr() { /* do nothing here */ }

    int Initialize();
    void Process();

    void AddSystem();
    void RemoveSystem();

private:
    Timer m_updateTimer;

    bool m_initalized;
};

//Singleton
#define sMktBotMgr \
( MarketBotMgr::get() )

#endif  // EVEMU_MARKET_MARKETBOTMGR_H_