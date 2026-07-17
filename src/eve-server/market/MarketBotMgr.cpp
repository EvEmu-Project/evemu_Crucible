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

// ---marketbot update; everything past this point has been completely changed.
#include "market/MarketDB.h"
#include "inventory/ItemType.h"
#include "inventory/ItemFactory.h"
#include "inventory/InventoryItem.h"
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"
#include "system/SystemEntity.h"
#include <random>
#include <cstdint>
#include <chrono>

extern SystemManager* sSystemMgr;

static constexpr int64 FILETIME_TICKS_PER_DAY = 864000000000;  // 100ns ticks per day; for expelorders to be removed prematurally

static const uint32 MARKETBOT_MAX_ITEM_ID = 30000;

// Только минералы (ID 34-41)
static const std::vector<uint32> MINERAL_IDS = {
    34,  // Tritanium
    35,  // Pyerite
    36,  // Mexallon
    37,  // Isogen
    38,  // Nocxium
    39,  // Zydrine
    40,  // Megacyte
    11399   // Morphite
};

static constexpr uint32 BOT_OWNER_ID = 90000002; // Реальный персонаж

// helper random generators
int GetRandomInt(int min, int max) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

float GetRandomFloat(float min, float max) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

MarketBotDataMgr::MarketBotDataMgr() {
    m_initalized = false;
}

int MarketBotDataMgr::Initialize() {
    m_initalized = true;
    sLog.Blue(" MarketBotDataMgr", "Market Bot Data Manager Initialized.");
    return 1;
}

MarketBotMgr::MarketBotMgr() {
    m_initalized = false;
}

int MarketBotMgr::Initialize() {
    if (!sMBotConf.ParseFile(sConfig.files.marketBotSettings.c_str())) {
        sLog.Error("       ServerInit", "Loading Market Bot Config file '%s' failed.", sConfig.files.marketBotSettings.c_str());
        return 0;
    }

    m_initalized = true;
    sMktBotDataMgr.Initialize();

    m_nextRunTime = Clock::now() + std::chrono::minutes(sMBotConf.main.DataRefreshTime);
    sLog.Cyan("     MarketBotMgr", "Timer initialized. First automated cycle will run in %d minutes.", sMBotConf.main.DataRefreshTime);

    sLog.Blue("     MarketBotMgr", "Market Bot Manager Initialized.");
    return 1;
}

// Called on minute tick from EntityList
void MarketBotMgr::Process(bool overrideTimer) {
    TimePoint now = Clock::now();

    if (!m_initalized) {
        sLog.Error("     MarketBotMgr", "MarketBotMgr not initialized - skipping run");
        return;
    }

    if (!overrideTimer && now + std::chrono::seconds(5) < m_nextRunTime) {
        auto timeLeft = std::chrono::duration_cast<std::chrono::milliseconds>(m_nextRunTime - now).count();
        if (timeLeft > 0) {
            return;
        }
    }
    
    sLog.Green("     Market Bot Mgr", "Processing old orders...");
    ExpireOldOrders();

    std::vector<uint32> eligibleSystems = GetEligibleSystems();
    sLog.Green("     Market Bot Mgr", "Bot found %zu eligible systems for order placement.", eligibleSystems.size());

    int totalBuyOrders = 0;
    int totalSellOrders = 0;

    for (uint32 systemID : eligibleSystems) {
        totalBuyOrders += PlaceBuyOrders(systemID);
        totalSellOrders += PlaceSellOrders(systemID);
    }

    sLog.Green("     Trader Joe", "Master Summary: Created %d buy orders and %d sell orders across %u systems.",
               totalBuyOrders, totalSellOrders, static_cast<uint32>(eligibleSystems.size()));

    m_nextRunTime = Clock::now() + std::chrono::minutes(sMBotConf.main.DataRefreshTime);
}

void MarketBotMgr::ForceRun(bool resetTimer) {
    sLog.Warning("     ForceRun", "Manually starting Trader Joe.");

    if (!m_initalized) {
        sLog.Yellow("     Trader Joe", "MarketBotMgr not initialized - skipping run.");
        return;
    }

    this->Process(true);
    
    if (resetTimer) {
        m_nextRunTime = Clock::now() + std::chrono::minutes(sMBotConf.main.DataRefreshTime);
        sLog.Green("     Trader Joe", "Timer reset. Next run in %d minutes.", sMBotConf.main.DataRefreshTime);
    }
}

void MarketBotMgr::AddSystem() { /* To be implemented if needed */ }
void MarketBotMgr::RemoveSystem() { /* To be implemented if needed */ }

int MarketBotMgr::ExpireOldOrders() {
    uint64_t now = GetFileTimeNow();

    DBQueryResult res;
    DBResultRow row;

    int expiredCount = 0;

    if (!sDatabase.RunQuery(res,
        "SELECT orderID FROM mktOrders WHERE (issued + CAST(duration AS UNSIGNED) * %" PRIu64 ") < CAST(%" PRIu64 " AS UNSIGNED) AND ownerID = %u",
        FILETIME_TICKS_PER_DAY, now, BOT_OWNER_ID)) {
        codelog(MARKET__DB_ERROR, "Failed to query expired bot orders.");
        return 0;
    }

    while (res.GetRow(row)) {
        uint32 orderID = row.GetUInt(0);
        MarketDB::DeleteOrder(orderID);
        ++expiredCount;
        codelog(MARKET__TRACE, "Expired bot order %u", orderID);
    }

    return expiredCount;
}

int MarketBotMgr::PlaceBuyOrders(uint32 systemID) {
    SystemData sysData;
    if (!sDataMgr.GetSystemData(systemID, sysData)) {
        codelog(MARKET__ERROR, "Failed to get system data for system %u", systemID);
        return 0;
    }

    std::vector<uint32> availableStations;
    if (!sDataMgr.GetStationListForSystem(systemID, availableStations)) {
        codelog(MARKET__ERROR, "No stations found for system %u", systemID);
        return 0;
    }

    size_t stationCount = availableStations.size();
    size_t stationLimit = stationCount;
    std::shuffle(availableStations.begin(), availableStations.end(), std::mt19937{std::random_device{}()});

    int orderCount = 0;

    for (size_t i = 0; i < std::min<size_t>(stationLimit, sMBotConf.main.OrdersPerRefresh); ++i) {
        uint32 stationID = availableStations[i];
        uint32 itemID = SelectRandomItemID();
        const ItemType* type = sItemFactory.GetType(itemID);
        if (!type) continue;

        // Проверяем, сколько уже есть BUY ордеров для этого минерала
        if (HasTooManyOrders(itemID, stationID, true)) {
            codelog(MARKET__TRACE, "Skipping BUY for mineral %u - already has 10+ active orders", itemID);
            continue;
        }

        uint32 quantity = GetRandomQuantity(itemID);
        double price = CalculateBuyPrice(itemID);

        if (price * quantity > sMBotConf.main.MaxISKPerOrder) {
            if (quantity > 1) {
                quantity = 1;
                if (price > sMBotConf.main.MaxISKPerOrder) {
                    codelog(MARKET__TRACE, "Skipping itemID %u due to price %.2f ISK exceeding MaxISKPerOrder.", itemID, price);
                    continue;
                }
            } else {
                codelog(MARKET__TRACE, "Skipping itemID %u even at quantity = 1 due to price %.2f ISK", itemID, price);
                continue;
            }
        }

        double escrow = price * quantity;

        Market::SaveData order;
        order.typeID = itemID;
        order.regionID = sysData.regionID;
        order.stationID = stationID;
        order.solarSystemID = systemID;
        order.minVolume = 1;
        order.volEntered = quantity;
        order.volRemaining = quantity;
        order.price = price;
        order.escrow = escrow;
        order.duration = sMBotConf.main.OrderLifetime;
        order.bid = true;
        order.issued = GetFileTimeNow();
        order.isCorp = false;
        order.ownerID = BOT_OWNER_ID;
        order.orderRange = 32767;
        order.memberID = 0;
        order.accountKey = 1000;

        bool success = MarketDB::StoreOrder(order);
        if (success) {
            ++orderCount;
            codelog(MARKET__TRACE, "BUY order created for typeID %u, qty %u, price %.2f ISK, station %u",
                order.typeID, order.volEntered, order.price, order.stationID);
        } else {
            codelog(MARKET__ERROR, "Failed to store BUY order for typeID %u at station %u",
                order.typeID, order.stationID);
        }
    }
    return orderCount;
}

int MarketBotMgr::PlaceSellOrders(uint32 systemID) {
    SystemData sysData;
    if (!sDataMgr.GetSystemData(systemID, sysData)) {
        codelog(MARKET__ERROR, "Bot: Failed to get system data for system %u", systemID);
        return 0;
    }

    std::vector<uint32> availableStations;
    if (!sDataMgr.GetStationListForSystem(systemID, availableStations)) {
        codelog(MARKET__ERROR, "Bot: No stations found for system %u", systemID);
        return 0;
    }

    size_t stationCount = availableStations.size();
    size_t stationLimit = stationCount;
    std::shuffle(availableStations.begin(), availableStations.end(), std::mt19937{std::random_device{}()});

    int orderCount = 0;

    for (size_t i = 0; i < std::min<size_t>(stationLimit, sMBotConf.main.OrdersPerRefresh); ++i) {
        uint32 stationID = availableStations[i];
        uint32 itemID = SelectRandomItemID();
        const ItemType* type = sItemFactory.GetType(itemID);
        if (!type) continue;

        // Проверяем, сколько уже есть SELL ордеров для этого минерала
        if (HasTooManyOrders(itemID, stationID, false)) {
            codelog(MARKET__TRACE, "Skipping SELL for mineral %u - already has 10+ active orders", itemID);
            continue;
        }

        uint32 quantity = GetRandomQuantity(itemID);
        double price = CalculateSellPrice(itemID);

        if (price * quantity > sMBotConf.main.MaxISKPerOrder) {
            if (quantity > 1) {
                quantity = 1;
                if (price > sMBotConf.main.MaxISKPerOrder) {
                    codelog(MARKET__TRACE, "Skipping itemID %u due to price %.2f ISK exceeding MaxISKPerOrder.", itemID, price);
                    continue;
                }
            } else {
                codelog(MARKET__TRACE, "Skipping itemID %u even at quantity = 1 due to price %.2f ISK", itemID, price);
                continue;
            }
        }

        Market::SaveData order;
        order.typeID = itemID;
        order.regionID = sysData.regionID;
        order.stationID = stationID;
        order.solarSystemID = systemID;
        order.minVolume = 1;
        order.volEntered = quantity;
        order.volRemaining = quantity;
        order.price = price;
        order.escrow = 0;
        order.duration = sMBotConf.main.OrderLifetime;
        order.bid = false;
        order.issued = GetFileTimeNow();
        order.isCorp = false;
        order.ownerID = BOT_OWNER_ID;
        order.orderRange = -1;
        order.memberID = 0;
        order.accountKey = 1000;

        bool success = MarketDB::StoreOrder(order);
        if (success) {
            ++orderCount;
            codelog(MARKET__TRACE, "SELL order created for typeID %u, qty %u, price %.2f, station %u",
                order.typeID, order.volEntered, order.price, order.stationID);
        } else {
            codelog(MARKET__ERROR, "Bot: Failed to store SELL order for typeID %u at station %u",
                order.typeID, order.stationID);
        }
    }
    return orderCount;
}

std::vector<uint32> MarketBotMgr::GetEligibleSystems() {
    std::vector<uint32> validSystems;
    std::vector<uint32> candidateSystems;
    
    // Получаем список систем (50 штук)
    sDataMgr.GetRandomSystemIDs(50, candidateSystems);
    
    // Проходим по системам, пока не найдем 5 с станциями
    for (uint32 systemID : candidateSystems) {
        std::vector<uint32> stations;
        // Проверяем, есть ли станции в системе
        if (sDataMgr.GetStationListForSystem(systemID, stations) && !stations.empty()) {
            validSystems.push_back(systemID);
            codelog(MARKET__TRACE, "Found valid system %u with %zu stations", systemID, stations.size());
            if (validSystems.size() >= 5) {
                break; // Нашли 5 систем - выходим
            }
        }
        // Если станций нет - просто переходим к следующей системе
    }
    
    // Если не нашли ни одной системы со станциями - используем Jita
    if (validSystems.empty()) {
        sLog.Warning("MarketBotMgr", "No systems with stations found, using Jita as fallback");
        validSystems.push_back(30000142); // Jita
    }
    
    codelog(MARKET__TRACE, "Selected %zu systems for order placement", validSystems.size());
    return validSystems;
}

uint32 MarketBotMgr::SelectRandomItemID() {
    // Рандом от 34 до 41 (минералы)
    return GetRandomInt(34, 41);
}

uint32 MarketBotMgr::GetRandomQuantity(uint32 itemID) {
    // Для минералов - большие партии
    return GetRandomInt(1000, 1000000);
}

double MarketBotMgr::CalculateBuyPrice(uint32 itemID) {
    const ItemType* type = sItemFactory.GetType(itemID);
    if (!type) {
        sLog.Error("MarketBot", "Item %u not found!", itemID);
        return 1000.0;
    }
    // Скупка: от 0.85 до 0.95 от базовой цены
    return type->basePrice() * GetRandomFloat(0.85f, 0.95f);
}

double MarketBotMgr::CalculateSellPrice(uint32 itemID) {
    const ItemType* type = sItemFactory.GetType(itemID);
    if (!type) {
        sLog.Error("MarketBot", "Item %u not found!", itemID);
        return 1000.0;
    }
    // Продажа: от 1.01 до 1.2 от базовой цены
    return type->basePrice() * GetRandomFloat(1.01f, 1.2f);
}

bool MarketBotMgr::HasTooManyOrders(uint32 itemID, uint32 stationID, bool isBuy) {
    DBQueryResult res;
    DBResultRow row;
    
    uint64_t now = GetFileTimeNow();
    
    if (sDatabase.RunQuery(res,
        "SELECT COUNT(*) FROM mktOrders "
        "WHERE typeID = %u "
        "AND stationID = %u "
        "AND ownerID = %u "
        "AND bid = %d "
        "AND (issued + CAST(duration AS UNSIGNED) * %" PRIu64 ") > CAST(%" PRIu64 " AS UNSIGNED)",
        itemID, stationID, BOT_OWNER_ID, isBuy ? 1 : 0, 
        FILETIME_TICKS_PER_DAY, now)) {
        
        if (res.GetRow(row)) {
            uint32 count = row.GetUInt(0);
            return count >= 10; // Если 10 или больше - возвращаем true
        }
    }
    
    return false;
}