
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

// ---marketbot update; this has to be declared first.
// ---marketbot update; issue with global timers, will have to fix later; this makes marketbot timer self contained.
#include <chrono>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
// ---marketbot update

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
    void Process(bool overrideTimer = false);

    void AddSystem();
    void RemoveSystem();
    // ---marketbot changes
    int PlaceBuyOrders(uint32 systemID);
    int PlaceSellOrders(uint32 systemID);
    int ExpireOldOrders();

    std::vector<uint32> GetEligibleSystems();
    uint32 SelectRandomItemID();
    uint32 GetRandomQuantity(uint32 groupID);
    double CalculateBuyPrice(uint32 itemID);
    double CalculateSellPrice(uint32 itemID);

    void ForceRun(bool resetTimer = true); // debug command to force MarketBot to run first cycle to generate NPC buy and sell orders.
    // ---
private:
    TimePoint m_nextRunTime;
    bool m_initalized;
};

//Singleton
#define sMktBotMgr \
( MarketBotMgr::get() )

#endif  // EVEMU_MARKET_MARKETBOTMGR_H_