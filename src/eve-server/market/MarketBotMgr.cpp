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
static const std::vector<uint32> VALID_GROUPS = {
    // Ores & Mining
    18,                                    // Minerals
    450, 451, 452, 453, 454, 455, 456,     // Raw ores (part 1)
    457, 458, 459, 460, 461, 462,
    465, 466, 467, 468, 469,              // Raw ores (part 2)
    479,                                  // Scanner Probe
    482,                                  // Mining Crystals
    492,                                  // Survey Probe
    538,                                  // Data Miners
    548,                                  // Interdiction Probe
    663,                                  // Mercoxit Mining Crystals

    // Ammo / Charges
    83,                                   // Projectile Ammo
    84,                                   // Missiles
    85,                                   // Hybrid Charges
    86,                                   // Frequency Crystals
    87,                                   // Cap Booster Charges
    88,                                   // Defender Missiles
    89,                                   // Torpedoes
    90,                                   // Bombs
    92,                                   // Mines
    372, 373, 374, 375, 376, 377,         // Advanced Ammo
    384, 385, 386, 387, 388, 389,         // Extended Missiles (part 1)
    390, 391, 392, 393, 394, 395, 396,    // Extended Missiles (part 2)
    648,                                  // Advanced Rocket
    653, 654, 655, 656, 657,              // Advanced Missiles
    772,                                  // Assault Missiles

    // Boosters for Implants
    303
};

static constexpr uint32 BOT_OWNER_ID = 1000125; // NPC corp owner for sell orders
static constexpr uint32 BOT_BUY_OWNER_ID = npcTraderJoe; // 4000000; supported NPC buyer in ExecuteBuyOrder

// Minerals and actual Crucible ore/ice groups, not mining crystals or charges.
// New NPC market orders use 100 million units per mineral or ore type.
static constexpr uint32 ORE_MINERAL_ORDER_QUANTITY = 100000000;
static constexpr double ORE_MINERAL_MAX_UNIT_PRICE = 10000000.0;

static bool IsOreOrMineralMarketGroup(uint32 groupID) {
    return groupID == 18
        || (groupID >= 450 && groupID <= 462)
        || groupID == 465 || groupID == 467 || groupID == 468 || groupID == 469;
}

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
    sLog.Blue(" MarketBotDataMgr", "Market Bot Data Manager Initialized."); // load current data
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

    sLog.Green("     MarketBotMgr", "MarketBot Process() invoked on tick.");
    codelog(MARKET__TRACE, "MarketBot Process() invoked on tick.");

    sLog.Green("     MarketBotMgr", "Entered MarketBotMgr::Process()\n");
    codelog(MARKET__TRACE, ">> Entered MarketBotMgr::Process()");

    if (!m_initalized) {
        sLog.Error("     MarketBotMgr", "MarketBotMgr not initialized — skipping run\n");
        codelog(MARKET__ERROR, "Process() called but MarketBotMgr is not initialized.");
        return;
    }

    if (!overrideTimer && now + std::chrono::seconds(5) < m_nextRunTime) { // ---marketbot update; 5 second jitter
        auto timeLeft = std::chrono::duration_cast<std::chrono::milliseconds>(m_nextRunTime - now).count();
        if (timeLeft > 0) {
            sLog.Green("     Trader Joe", "Update timer not ready yet. Next run in %lld seconds.", timeLeft / 1000);
            codelog(MARKET__TRACE, "Trader Joe waiting — next run in %lld seconds.", timeLeft);
            return;
        }
    }
    
    // ExpireOldOrders() runs once below; do not scan all NPC orders twice.
    std::vector<uint32> eligibleSystems = GetEligibleSystems();
    sLog.Green("     Market Bot Mgr", "Trader Joe found %zu eligible systems for order placement.", eligibleSystems.size());
    codelog(MARKET__TRACE, "Trader Joe found %zu eligible systems for order placement.", eligibleSystems.size());

    int totalBuyOrders = 0;
    int totalSellOrders = 0;
    int expiredOrders = ExpireOldOrders();

    for (uint32 systemID : eligibleSystems) {
        sLog.Green("     Market Bot Mgr", "Trader Joe placing orders in systemID: %u", systemID);
        codelog(MARKET__TRACE, "Trader Joe placing orders in systemID: %u", systemID);

        totalBuyOrders += PlaceBuyOrders(systemID);
        totalSellOrders += PlaceSellOrders(systemID);
    }

    sLog.Green("     Trader Joe", "Master Summary: Created %d buy orders and %d sell orders across %u systems. Removed %d old orders.",
    totalBuyOrders, totalSellOrders, static_cast<uint32>(eligibleSystems.size()), expiredOrders);

    codelog(MARKET__TRACE, "Trader Joe Master Summary: Created %d buy orders and %d sell orders across %zu systems. Removed %d old orders.",
        totalBuyOrders, totalSellOrders, eligibleSystems.size(), expiredOrders);

    sLog.Green("     Trader Joe", "Cycle complete. Resetting timer.");
    codelog(MARKET__TRACE, "Trader Joe cycle complete. Resetting timer.");
    m_nextRunTime = Clock::now() + std::chrono::minutes(sMBotConf.main.DataRefreshTime);
    sLog.Green("     Trader Joe", "Timer reset. Next run in %d minutes.", sMBotConf.main.DataRefreshTime);
}

void MarketBotMgr::ForceRun(bool resetTimer) {
    sLog.Warning("     ForceRun", "Manually starting Trader Joe.");

    if (!m_initalized) {
        sLog.Yellow("     Trader Joe", "MarketBotMgr not initialized — skipping run.");
        return;
    }

    sLog.Green("     Trader Joe", "Running Process() now...");
    this->Process(true);  // force override
    sLog.Green("     Trader Joe", "Finished Process().");
    
    if (resetTimer) {
        m_nextRunTime = Clock::now() + std::chrono::minutes(sMBotConf.main.DataRefreshTime);
        sLog.Green("     Trader Joe", "Timer reset. Next run in %d minutes.", sMBotConf.main.DataRefreshTime);
    }
}

void MarketBotMgr::AddSystem() { /* To be implemented if needed */ }
void MarketBotMgr::RemoveSystem() { /* To be implemented if needed */ }

int MarketBotMgr::ExpireOldOrders() {
    const uint64_t now = GetFileTimeNow();
    DBQueryResult res;
    DBResultRow row;
    // Restrict by bot owner first. Keep the original 550-unit exclusion.
    if (!sDatabase.RunQuery(res,
        "SELECT orderID FROM mktOrders"
        " WHERE ownerID IN (%u, %u) AND volEntered != 550"
        " AND (issued + CAST(duration AS UNSIGNED) * %" PRIu64 ")"
        " < CAST(%" PRIu64 " AS UNSIGNED)",
        BOT_OWNER_ID, BOT_BUY_OWNER_ID, FILETIME_TICKS_PER_DAY, now)) {
        codelog(MARKET__DB_ERROR, "Failed to query expired Trader Joe orders.");
        return 0;
    }

    std::vector<uint32> expiredIDs;
    expiredIDs.reserve(res.GetRowCount());
    while (res.GetRow(row))
        expiredIDs.push_back(row.GetUInt(0));

    // Batch the former one-DELETE-per-order loop. The extra conditions on
    // DELETE avoid deleting a refreshed order or an unrelated player order.
    constexpr size_t kDeleteBatchSize = 128;
    int processed = 0;
    for (size_t offset = 0; offset < expiredIDs.size(); offset += kDeleteBatchSize) {
        const size_t limit = std::min(offset + kDeleteBatchSize, expiredIDs.size());
        std::string ids;
        ids.reserve((limit - offset) * 12);
        for (size_t i = offset; i < limit; ++i) {
            if (!ids.empty())
                ids += ',';
            ids += std::to_string(expiredIDs[i]);
        }
        DBerror err;
        if (!sDatabase.RunQuery(err,
            "DELETE FROM mktOrders WHERE orderID IN (%s)"
            " AND ownerID IN (%u, %u) AND volEntered != 550"
            " AND (issued + CAST(duration AS UNSIGNED) * %" PRIu64 ")"
            " < CAST(%" PRIu64 " AS UNSIGNED)",
            ids.c_str(), BOT_OWNER_ID, BOT_BUY_OWNER_ID,
            FILETIME_TICKS_PER_DAY, now)) {
            codelog(MARKET__DB_ERROR, "Failed to delete expired Trader Joe order batch: %s.", err.c_str());
            continue;
        }
        // Counts successfully processed candidates; concurrent refresh can
        // make actual removed-row count smaller.
        processed += static_cast<int>(limit - offset);
    }
    codelog(MARKET__TRACE, "Trader Joe expired-order candidates processed: %d.", processed);
    return processed;
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

        uint32 quantity = GetRandomQuantity(type->groupID());
        double price = CalculateBuyPrice(itemID);
        const bool isOreMineral = IsOreOrMineralMarketGroup(type->groupID());
        // Do not silently reduce the 100M-unit ore/mineral quantity to one.
        // At most 10M ISK/unit keeps its notional value <= 10^15 ISK.
        if (isOreMineral && (!(price > 0.0) || price > ORE_MINERAL_MAX_UNIT_PRICE)) {
            codelog(MARKET__WARNING, "Skipping ore/mineral type %u: price %.2f exceeds 100M-unit order safety limit.", itemID, price);
            continue;
        }

        if (!isOreMineral && price * quantity > sMBotConf.main.MaxISKPerOrder) {
            if (quantity > 1) {
                quantity = 1;
                if (price > sMBotConf.main.MaxISKPerOrder) {
                    codelog(MARKET__TRACE, "Skipping itemID %u due to price %.2f ISK exceeding MaxISKPerOrder.", itemID, price);
                    continue;
                }
                codelog(MARKET__TRACE, "Price too high for bulk, retrying with quantity = 1 for itemID %u", itemID);
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
        order.escrow = escrow; // required, without this no one can sell items to bot orders.
        order.duration = sMBotConf.main.OrderLifetime;
        order.bid = true;
        order.issued = GetFileTimeNow();
        order.isCorp = false;
        order.ownerID = BOT_BUY_OWNER_ID;
        order.orderRange = 32767; // -1 station, 0 solarsystem, 1-5 10 20 30 40 jumps, 32767 region (default)
        // marketbot update; unsure if these are really needed, but aligns with how orders are created manually in-game and through seed-market.
        order.memberID = 0;       // default value for who placed the order (0 for char order)
        order.accountKey = 1000;  // default value for corp account key

        bool success = MarketDB::StoreOrder(order);
        if (success) {
            ++orderCount;
            codelog(MARKET__TRACE, "%s order created for typeID %u, qty %u, price %.2f ISK, station %u",
                (order.bid ? "BUY" : "SELL"), order.typeID, order.volEntered, order.price, order.stationID);
        } else {
            codelog(MARKET__ERROR, "Failed to store %s order for typeID %u at station %u",
                (order.bid ? "BUY" : "SELL"), order.typeID, order.stationID);
        }
    }
    return orderCount;
}

int MarketBotMgr::PlaceSellOrders(uint32 systemID) {
    SystemData sysData;
    if (!sDataMgr.GetSystemData(systemID, sysData)) {
        codelog(MARKET__ERROR, "Trader Joe: Failed to get system data for system %u", systemID);
        return 0;
    }

    std::vector<uint32> availableStations;

    if (!sDataMgr.GetStationListForSystem(systemID, availableStations)) {
        codelog(MARKET__ERROR, "Trader Joe: No stations found for system %u — skipping order creation.", systemID);
        return 0;
    } else {
        codelog(MARKET__TRACE, "Trader Joe: Found %zu stations in system %u", availableStations.size(), systemID);
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

        uint32 quantity = GetRandomQuantity(type->groupID());
        double price = CalculateSellPrice(itemID);
        const bool isOreMineral = IsOreOrMineralMarketGroup(type->groupID());
        // Do not silently reduce the 100M-unit ore/mineral quantity to one.
        // At most 10M ISK/unit keeps its notional value <= 10^15 ISK.
        if (isOreMineral && (!(price > 0.0) || price > ORE_MINERAL_MAX_UNIT_PRICE)) {
            codelog(MARKET__WARNING, "Skipping ore/mineral type %u: price %.2f exceeds 100M-unit order safety limit.", itemID, price);
            continue;
        }

        if (!isOreMineral && price * quantity > sMBotConf.main.MaxISKPerOrder) {
            if (quantity > 1) {
                quantity = 1;
                if (price > sMBotConf.main.MaxISKPerOrder) {
                    codelog(MARKET__TRACE, "Skipping itemID %u due to price %.2f ISK exceeding MaxISKPerOrder.", itemID, price);
                    continue;
                }
                codelog(MARKET__TRACE, "Price too high for bulk, retrying with quantity = 1 for itemID %u", itemID);
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
        order.escrow = 0; // not required for sell orders; place here for possible future use.
        order.duration = sMBotConf.main.OrderLifetime;
        order.bid = false;
        order.issued = GetFileTimeNow();
        order.isCorp = false;
        order.ownerID = BOT_OWNER_ID;
        order.orderRange = -1; // -1 station (default), 0 solarsystem, 1-5 10 20 30 40 jumps, 32767 region
        // marketbot update; unsure if these are really needed, but aligns with how orders are created manually in-game and through seed-market.
        order.memberID = 0;       // default value for who placed the order (0 for char order)
        order.accountKey = 1000;  // default value for corp account key

        codelog(MARKET__TRACE, "System %u maps to region %u via GetSystemData", systemID, sysData.regionID);

        codelog(MARKET__TRACE, "Trader Joe: Storing sell order with orderRange = %u", order.orderRange);

        bool success = MarketDB::StoreOrder(order);
        if (success) {
            ++orderCount;
            codelog(MARKET__TRACE, "Trader Joe: Creating %s order for typeID %u, qty %u, price %.2f, station %u, region %u",
                (order.bid ? "BUY" : "SELL"), order.typeID, order.volEntered, order.price, order.stationID, order.regionID);
        } else {
            codelog(MARKET__ERROR, "Trader Joe: Failed to store %s order for typeID %u at station %u",
                (order.bid ? "BUY" : "SELL"), order.typeID, order.stationID);
        }
    }
    return orderCount;
}

std::vector<uint32> MarketBotMgr::GetEligibleSystems() {
    bool useStaticSystems = false; // ---marketbot update; only turn to true, for testing changes with markbot or want to have it only populate orders in one system
    if (useStaticSystems) {
        return { 30000142 };  // Jita (default system for testing)
    }

    std::vector<uint32> systemIDs;
    sDataMgr.GetRandomSystemIDs(5, systemIDs); // pulls a randomized list of systems
    codelog(MARKET__TRACE, "GetEligibleSystems(): Pulled %zu systems from StaticDataMgr", systemIDs.size());
    return systemIDs;
}

uint32 MarketBotMgr::SelectRandomItemID() {
    uint32 itemID = 0;
    const ItemType* type = nullptr;
    uint32 tries = 0;

    do {
        ++tries;
        itemID = GetRandomInt(10, MARKETBOT_MAX_ITEM_ID);
        type = sItemFactory.GetType(itemID);

        if (type && std::find(VALID_GROUPS.begin(), VALID_GROUPS.end(), type->groupID()) != VALID_GROUPS.end()) {
            codelog(MARKET__TRACE, "Selected itemID %u after %u attempts", itemID, tries);
            return itemID;
        }
    } while (tries < 50);

    // If we fail after 50 attempts, log a warning and return fallback value
    codelog(MARKET__WARNING, "Failed to select valid itemID after %u attempts. Returning fallback itemID = 34 (Tritanium)", tries);
    return 34;  // Tritanium, as a safe default
}

uint32 MarketBotMgr::GetRandomQuantity(uint32 groupID) {
    if (IsOreOrMineralMarketGroup(groupID))
        return ORE_MINERAL_ORDER_QUANTITY;

    // Non-ore high-volume items retain the former random quantities.
    if (
        (groupID >= 83 && groupID <= 92) ||   // Basic ammo/charges
        (groupID >= 372 && groupID <= 377) || // Advanced ammo
        (groupID >= 384 && groupID <= 396) || // More missiles
        groupID == 479 ||                     // Scanner Probes
        groupID == 482 ||                     // Mining Crystals
        groupID == 492 ||                     // Survey Probes
        groupID == 538 ||                     // Data Miners
        groupID == 548 ||                     // Interdiction Probe
        groupID == 648 ||                     // Advanced Rocket
        (groupID >= 653 && groupID <= 657) || // Advanced Missiles
        groupID == 663 ||                     // Mercoxit Mining Crystals
        groupID == 772                       // Assault Missiles
    ) {
        return GetRandomInt(1000, 1000000);  // Large stack sizes
    }

    // Medium-volume: modules, rigs, etc.; way to many to list... disabled for the time being
    // leaving below as an example.
    /*if (
        groupID == 62 ||  // Armor Repairers
        groupID == 63 ||  // Hull Repair
        groupID == 205    // Heat Sink
    ) {
        return GetRandomInt(10, 100);
    }*/

    // Fallback for anything else
    return GetRandomInt(10, 500);
}

double MarketBotMgr::CalculateBuyPrice(uint32 itemID) {
    const ItemType* type = sItemFactory.GetType(itemID);
    // All NPC buy orders follow the same 100% static reference-price
    // policy as the regional seed. No randomized 80%-110% adjustment.
    return (type && type->basePrice() > 0.0)
        ? std::max(0.01, static_cast<double>(type->basePrice()))
        : 100.0;
}

double MarketBotMgr::CalculateSellPrice(uint32 itemID) {
    const ItemType* type = sItemFactory.GetType(itemID);
    // NPC sell orders use 100% of the static item reference value.
    // A zero/negative base price cannot define a percentage; use the same
    // explicit 100 ISK fallback as EVEDBTool's market seeder.
    return (type && type->basePrice() > 0.0)
        ? std::max(0.01, static_cast<double>(type->basePrice()))
        : 100.0;
}
