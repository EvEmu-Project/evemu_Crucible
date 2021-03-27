
 /**
  * @name MarketMgr.cpp
  *   singleton object for storing, manipulating and managing in-game market data
  *   this mgr keeps track of market data without abusing the db on every call. (vs old system)
  *
  * @Author:         Allan
  * @date:          19Dec17
  *
  */

#include "Client.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "inventory/InventoryItem.h"
#include "market/MarketMgr.h"
#include "station/StationDataMgr.h"

/*
 * MARKET__ERROR
 * MARKET__WARNING
 * MARKET__MESSAGE
 * MARKET__DEBUG
 * MARKET__TRACE
 * MARKET__DB_ERROR
 * MARKET__DB_TRACE
 */

MarketMgr::MarketMgr()
: m_marketGroups(nullptr),
m_manager(nullptr)
{
    m_timeStamp = 0;
}

MarketMgr::~MarketMgr()
{
    //PyDecRef(m_marketGroups);
}

void MarketMgr::Close()
{
    /** @todo put a save method here which will save anything changed before shutdown */
    PyDecRef(m_marketGroups);
    sLog.Warning("        MarketMgr", "Market Manager has been closed." );
}

int MarketMgr::Initialize(PyServiceMgr* pManager)
{
    m_manager = pManager;

    m_timeStamp = MarketDB::GetUpdateTime();

    Populate();
    sLog.Blue("        MarketMgr", "Market Manager Initialized.");
    return 1;
}

void MarketMgr::Populate()
{
    double start = GetTimeMSeconds();
    m_marketGroups = m_db.GetMarketGroups();

    Process();

    // market orders stored as {regionID/typeID}    --do we want to store orders in memory for loaded region??
    // m_db.GetOrders(call.client->GetRegionID(), args.arg);

    sLog.Blue("        MarketMgr", "Market Manager loaded in %.3fms.", (GetTimeMSeconds() - start));
    sLog.Cyan("        MarketMgr", "Market Manager Updates Price History every %u hours.", sConfig.market.HistoryUpdateTime);
}

void MarketMgr::GetInfo()
{
    /* get info in current market data? */
}

void MarketMgr::Process()
{
    // make cache timer of xx(time) then invalidate the price history cache

    if (m_timeStamp > GetFileTimeNow())
        UpdatePriceHistory();
}

void MarketMgr::SystemStartup(SystemData& data)
{

}

void MarketMgr::SystemShutdown(SystemData& data)
{

}

void MarketMgr::UpdatePriceHistory()
{
    m_timeStamp = GetFileTimeNow() + (EvE::Time::Hour * sConfig.market.HistoryUpdateTime);
    MarketDB::SetUpdateTime(m_timeStamp);

    DBerror err;
    int64 cutoff_time = m_timeStamp;
    cutoff_time -= cutoff_time % EvE::Time::Day;    //round down to an even day boundary.
    cutoff_time -= EvE::Time::Day * 2;  //the cutoff between "new" and "old" price history in days

    //build the history record from the recent market transactions.
    sDatabase.RunQuery(err,
            "INSERT INTO"
            "    mktHistory"
            "     (regionID, typeID, historyDate, lowPrice, highPrice, avgPrice, volume, orders)"
            " SELECT"
            "    regionID,"
            "    typeID,"
            "    transactionDate,"
            "    MIN(price),"
            "    MAX(price),"
            "    AVG(price),"
            "    SUM(quantity),"
            "    COUNT(transactionID)"
            " FROM mktTransactions"
            " WHERE transactionType=%u AND transactionDate < %li",
            //" GROUP BY regionID, typeID, transactionDate",
            Market::Type::Sell, cutoff_time);

    // remove the transactions which have been aged out?
    if (sConfig.market.DeleteOldTransactions)
        sDatabase.RunQuery(err, "DELETE FROM mktTransactions WHERE transactionDate < %li", (cutoff_time - EvE::Time::Year));
}

    /*DBColumnTypeMap colmap;
     *    colmap["historyDate"] = DBTYPE_FILETIME;
     *    colmap["lowPrice"] = DBTYPE_CY;
     *    colmap["highPrice"] = DBTYPE_CY;
     *    colmap["avgPrice"] = DBTYPE_CY;
     *    colmap["volume"] = DBTYPE_I8;
     *    colmap["orders"] = DBTYPE_I4;
     * SELECT    transactionDate AS historyDate,    MIN(price) AS lowPrice,    MAX(price) AS highPrice,    AVG(price) AS avgPrice,    quantity AS volume,    COUNT(transactionID) AS orders FROM mktTransactions  WHERE regionID=10000030 AND typeID=487    AND transactionType=0     AND historyDate > 20604112 GROUP BY historyDate
     */

// there is a 1 day difference (from 0000UTC) between "Old" and "New" prices
PyRep *MarketMgr::GetNewPriceHistory(uint32 regionID, uint32 typeID) {
    PyRep* result(nullptr);
    std::string method_name ("GetNewHistory_");
    method_name += std::to_string(regionID);
    method_name += "_";
    method_name += std::to_string(typeID);
    ObjectCachedMethodID method_id("marketProxy", method_name.c_str());
    //check to see if this method is in the cache already.
    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it
        DBQueryResult res;
        if(!sDatabase.RunQuery(res,
            "SELECT historyDate, lowPrice, highPrice, avgPrice, volume, orders"
            " FROM mktHistory "
            " WHERE regionID=%u AND typeID=%u"
            " AND historyDate > %li  LIMIT %u",
            regionID, typeID, (m_timeStamp - EvE::Time::Day), sConfig.market.NewPriceLimit))
        {
            _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }
        _log(MARKET__DB_TRACE, "MarketMgr::GetNewPriceHistory() - Fetched %u buy orders for type %u in region %u from mktTransactions", res.GetRowCount(), typeID, regionID);

        result = DBResultToCRowset(res);
        if (result == nullptr) {
            _log(MARKET__DB_ERROR, "Failed to load cache, generating empty contents.");
            result = PyStatic.NewNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    if (is_log_enabled(MARKET__DB_TRACE))
        result->Dump(MARKET__DB_TRACE, "    ");
    return result;
}

PyRep *MarketMgr::GetOldPriceHistory(uint32 regionID, uint32 typeID) {
    PyRep* result(nullptr);
    std::string method_name ("GetOldHistory_");
    method_name += std::to_string(regionID);
    method_name += "_";
    method_name += std::to_string(typeID);
    ObjectCachedMethodID method_id("marketProxy", method_name.c_str());
    //check to see if this method is in the cache already.
    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it
        DBQueryResult res;
        if(!sDatabase.RunQuery(res,
            "SELECT historyDate, lowPrice, highPrice, avgPrice, volume, orders"
            " FROM mktHistory WHERE regionID=%u AND typeID=%u"
            " AND historyDate > %li AND historyDate < %li LIMIT %u",
            regionID, typeID, (m_timeStamp - (EvE::Time::Day *3)), (m_timeStamp - EvE::Time::Day), sConfig.market.OldPriceLimit))
        {
            _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }
        _log(MARKET__DB_TRACE, "MarketMgr::GetOldPriceHistory() - Fetched %u orders for type %u in region %u from mktHistory", res.GetRowCount(), typeID, regionID);

        result = DBResultToCRowset(res);
        if (result == nullptr) {
            _log(MARKET__DB_ERROR, "Failed to load cache, generating empty contents.");
            result = PyStatic.NewNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    if (is_log_enabled(MARKET__DB_TRACE))
        result->Dump(MARKET__DB_TRACE, "    ");
    return result;
}

void MarketMgr::SendOnOwnOrderChanged(Client* pClient, uint32 orderID, uint8 action, bool isCorp/*false*/, PyRep* order/*nullptr*/) {
    if (pClient == nullptr)
        return;
    Notify_OnOwnOrderChanged ooc;
    if (order != nullptr) {
        ooc.order = order;
    } else {
        ooc.order = m_db.GetOrderRow(orderID);
    }

    switch (action) {
        case Market::Action::Add:
            ooc.reason = "Add";
            break;
        case Market::Action::Modify:
            ooc.reason = "Modify";
            break;
        case Market::Action::Expiry:
            ooc.reason = "Expiry";
            break;
    }

    ooc.isCorp = isCorp;
    PyTuple* tmp = ooc.Encode();
    // send journal blink and call 'self.RefreshOrders()' in client
    if (isCorp) {
        sEntityList.CorpNotify(pClient->GetCorporationID(), 125 /*MarketOrder*/, "OnOwnOrderChanged", "*corpid&corprole", tmp);
    } else {
        pClient->SendNotification("OnOwnOrderChanged", "clientID", &tmp);
    }
}


void MarketMgr::InvalidateOrdersCache(uint32 regionID, uint32 typeID)
{
    std::string method_name ("GetOrders_");
    method_name += std::to_string(regionID);
    method_name += "_";
    method_name += std::to_string(typeID);
    ObjectCachedMethodID method_id("marketProxy", method_name.c_str());
    m_manager->cache_service->InvalidateCache( method_id );
}


/** @todo take off market overhead fees */
/*
 *    def BrokersFee(self, stationID, amount, commissionPercentage):
 *        if amount < 0.0:
 *            raise AttributeError('Amount must be positive')
 *        orderValue = float(amount)
 *        station = sm.GetService('ui').GetStation(stationID)
 *        stationOwnerID = None
 *        if station is not None:
 *            stationOwnerID = station.ownerID
 *        factionChar = 0
 *        corpChar = 0
 *        if stationOwnerID:
 *            if util.IsNPC(stationOwnerID):
 *                factionID = sm.GetService('faction').GetFaction(stationOwnerID)
 *                factionChar = sm.GetService('standing').GetStanding(factionID, eve.session.charid) or 0.0
 *            corpChar = sm.GetService('standing').GetStanding(stationOwnerID, eve.session.charid) or 0.0
 *        weightedStanding = (0.7 * factionChar + 0.3 * corpChar) / 10.0
 *        commissionPercentage = commissionPercentage * 2.0 ** (-2 * weightedStanding)
 *        tax = util.KeyVal()
 *        tax.amt = commissionPercentage * orderValue
 *        tax.percentage = commissionPercentage
 *        if tax.amt <= const.mktMinimumFee:
 *            tax.amt = const.mktMinimumFee
 *            tax.percentage = -1.0
 *        return tax
 */

bool MarketMgr::ExecuteBuyOrder(Client* seller, uint32 orderID, InventoryItemRef iRef, Call_PlaceCharOrder& args, uint16 accountKey/*Account::KeyType::Cash*/) {

    Market::OrderInfo oInfo = Market::OrderInfo();
    if (!m_db.GetOrderInfo(orderID, oInfo)) {
        _log(MARKET__ERROR, "ExecuteBuyOrder - Failed to get order info for #%u.", orderID);
        return true;
    }

    /*  will this method also be used to buy/sell using aurm?
     * unknown yet
     */

    // get buyer id and determine if buyer is player or corp (or bot for later)
    bool isPlayer(false), isCorp(false), isTrader(false);
    if (IsPlayerCorp(oInfo.ownerID)) {
        // buyer is player corp
        isCorp = true;
    } else if (oInfo.ownerID == 1) {
        oInfo.ownerID = stDataMgr.GetOwnerID(args.stationID);
    } else if (IsCharacter(oInfo.ownerID)) {
        isPlayer = true;
    } else if (IsTraderJoe(oInfo.ownerID)) {
        isTrader = true;
    } else {
        // none of above conditionals hit....
        _log(MARKET__WARNING, "ExecuteBuyOrder - ownerID %u not corp, not char, not system, not joe.", oInfo.ownerID);
    }

    // quantity status of seller's item vs buyer's order
    uint8 qtyStatus(Market::QtyStatus::Invalid);
    if (iRef->quantity() == args.quantity) {
        qtyStatus = Market::QtyStatus::Complete;
        //use the owner change packet to alert the buyer of the new item
        if (isPlayer) {
            iRef->Donate(oInfo.ownerID, args.stationID, flagHangar, true);
        } else if (isCorp) {
            iRef->Donate(oInfo.ownerID, args.stationID, flagCorpMarket, true);
        }
    } else if (iRef->quantity() > args.quantity) {
        // qty for sale > buy order amt
        qtyStatus = Market::QtyStatus::Over;
        //need to split item up...
        InventoryItemRef siRef = iRef->Split(args.quantity);
        if (siRef.get() == nullptr) {
            _log(MARKET__ERROR, "ExecuteBuyOrder - Failed to split %u %s.", siRef->itemID(), siRef->name());
            return true;
        }
        //use the owner change packet to alert the buyer of the new item
        if (isPlayer) {
            siRef->Donate(oInfo.ownerID, args.stationID, flagHangar, true);
        } else if (isCorp) {
            siRef->Donate(oInfo.ownerID, args.stationID, flagCorpMarket, true);
        }
    } else {
        // qty for sale < buy order amt
        qtyStatus = Market::QtyStatus::Under;
        //use the owner change packet to alert the buyer of the new item
        if (isPlayer) {
            iRef->Donate(oInfo.ownerID, args.stationID, flagHangar, true);
        } else if (isCorp) {
            iRef->Donate(oInfo.ownerID, args.stationID, flagCorpMarket, true);
        }
    }

    uint32 qtySold(args.quantity);
    switch (qtyStatus) {
        case Market::QtyStatus::Invalid: {
            // this should never hit. make error here.
        } break;
        case Market::QtyStatus::Under:  // order requires more items than seller is offering.  delete args.quantity and update order
        case Market::QtyStatus::Complete: { // order qty matches item qty.  delete args.quantity and delete order
            qtySold = args.quantity;
            args.quantity = 0;
        } break;
        case Market::QtyStatus::Over: {
            // more for sale than order requires.  update args.quantity and delete order
            qtySold = oInfo.quantity;
            args.quantity -= qtySold;
        } break;
    }

    float money = args.price * qtySold;
    std::string reason = "DESC:  Buying items in ";
    reason += stDataMgr.GetStationName(args.stationID).c_str();
    if (isPlayer) {
        // get data needed and compute tax
        uint8 lvl(0);
        Client* pBuyer = sEntityList.FindClientByCharID(oInfo.ownerID);
        if (pBuyer == nullptr) {
            lvl = CharacterDB::GetSkillLevel(oInfo.ownerID, EvESkill::Accounting);
        } else {
            lvl = pBuyer->GetChar()->GetSkillLevel(EvESkill::Accounting);
        }
        float tax = EvEMath::Market::SalesTax(lvl);
        tax *= money;
        _log(MARKET__DEBUG, "ExecuteBuyOrder - Buyer is Player: Price: %.2f, Tax: %.2f", money, tax);
        AccountService::TranserFunds(oInfo.ownerID, corpSCC, tax, reason.c_str(), \
                        Journal::EntryType::TransactionTax, orderID);
    } else if (isCorp) {
        // is corp taxes modified by using character skills?  im thinking yes...
        // get data needed and compute tax
        uint8 lvl(0);
        Client* pBuyer = sEntityList.FindClientByCharID(oInfo.memberID);
        if (pBuyer == nullptr) {
            lvl = CharacterDB::GetSkillLevel(oInfo.memberID, EvESkill::Accounting);
        } else {
            lvl = pBuyer->GetChar()->GetSkillLevel(EvESkill::Accounting);
        }
        float tax = EvEMath::Market::SalesTax(lvl);
        tax *= money;
        _log(MARKET__DEBUG, "ExecuteBuyOrder - Buyer is Corp: Price: %.2f, Tax: %.2f", money, tax);
        AccountService::TranserFunds(oInfo.ownerID, corpSCC, tax, reason.c_str(), \
                        Journal::EntryType::TransactionTax, orderID, oInfo.accountKey);
    }
    // npc buyers dont pay tax

    // send wallet blink event and record the transaction in their journal.
    reason.clear();
    reason += "DESC:  Selling items in ";
    reason += stDataMgr.GetStationName(args.stationID).c_str();
    // this is fulfilling a buy order.  seller will receive isk from escrow if buyer is player or corp
    if (isPlayer or isCorp) {
        //give the money to the seller from the escrow acct at station
        AccountService::TranserFunds(stDataMgr.GetOwnerID(args.stationID), seller->GetCharacterID(), \
                                money, reason.c_str(), Journal::EntryType::MarketTransaction, orderID, \
                                Account::KeyType::Escrow, accountKey);
    } else {
        // npc buyer.  direct xfer to seller
        AccountService::TranserFunds(oInfo.ownerID, seller->GetCharacterID(), money, reason.c_str(), \
                                    Journal::EntryType::MarketTransaction, orderID, Account::KeyType::Cash, accountKey);
    }

    // add data to StatisticMgr
    sStatMgr.Add(Stat::iskMarket, money);

    //record this sell transaction in market_transactions
    Market::TxData data = Market::TxData();
    data.accountKey     = accountKey;
    data.isBuy          = Market::Type::Sell;
    data.isCorp         = args.useCorp;
    data.memberID       = args.useCorp?seller->GetCharacterID():0;
    data.clientID       = args.useCorp?seller->GetCorporationID():seller->GetCharacterID();
    data.price          = args.price;
    data.quantity       = args.quantity;
    data.stationID      = args.stationID;
    data.regionID       = sDataMgr.GetStationRegion(args.stationID);
    data.typeID         = args.typeID;

    if (!m_db.RecordTransaction(data)) {
        _log(MARKET__ERROR, "ExecuteBuyOrder - Failed to record sale side of transaction.");
    }

    if (isPlayer or isCorp) {
        // update data for other side if player or player corp
        data.isBuy          = Market::Type::Buy;
        data.clientID       = oInfo.ownerID;
        data.memberID       = isCorp?oInfo.memberID:0;
        if (!m_db.RecordTransaction(data)) {
            _log(MARKET__ERROR, "ExecuteBuyOrder - Failed to record buy side of transaction.");
        }
    }

    if (qtyStatus == Market::QtyStatus::Under) {
        uint32 newQty(oInfo.quantity - args.quantity);
        _log(MARKET__TRACE, "ExecuteBuyOrder - Partially satisfied order #%u, altering quantity to %u.", orderID, newQty);
        if (!m_db.AlterOrderQuantity(orderID, newQty)) {
            _log(MARKET__ERROR, "ExecuteBuyOrder - Failed to alter quantity of order #%u.", orderID);
            return true;
        }
        InvalidateOrdersCache(oInfo.regionID, args.typeID);
        if (isPlayer or isCorp)
            SendOnOwnOrderChanged(seller, orderID, Market::Action::Modify, args.useCorp);

        return false;
    }

    _log(MARKET__TRACE, "ExecuteBuyOrder - Satisfied order #%u, deleting.", orderID);
    PyRep* order = m_db.GetOrderRow(orderID);
    if (!m_db.DeleteOrder(orderID)) {
        _log(MARKET__ERROR, "ExecuteBuyOrder - Failed to delete order #%u.", orderID);
        return true;
    }
    InvalidateOrdersCache(oInfo.regionID, args.typeID);
    if (isPlayer or isCorp)
        SendOnOwnOrderChanged(seller, orderID, Market::Action::Expiry, args.useCorp, order);
    return true;
}

void MarketMgr::ExecuteSellOrder(Client* buyer, uint32 orderID, Call_PlaceCharOrder& args) {
    Market::OrderInfo oInfo = Market::OrderInfo();
    if (!m_db.GetOrderInfo(orderID, oInfo)) {
        _log(MARKET__ERROR, "ExecuteSellOrder - Failed to get info about sell order %u.", orderID);
        return;
    }

    bool orderConsumed(false);
    if (args.quantity > oInfo.quantity)
        args.quantity = oInfo.quantity;
    if (args.quantity == oInfo.quantity)
        orderConsumed = true;

    if (IsStation(oInfo.ownerID))
        oInfo.ownerID = stDataMgr.GetOwnerID(oInfo.ownerID);

    /** @todo  get/implement accountKey here.... */
    float money = args.price * args.quantity;
    // send wallet blink event and record the transaction in their journal.
    std::string reason = "DESC:  Buying market items in ";
    reason += stDataMgr.GetStationName(args.stationID).c_str();
    // this will throw if funds not available.
    AccountService::TranserFunds(buyer->GetCharacterID(), oInfo.ownerID, money, reason.c_str(), \
                    Journal::EntryType::MarketTransaction, orderID, Account::KeyType::Cash);

    // get data needed and compute tax
    /** @todo standings incomplete.  need to finish */
    float tax = EvEMath::Market::SalesTax(buyer->GetChar()->GetSkillLevel(EvESkill::Accounting), buyer->GetChar()->GetSkillLevel(EvESkill::TaxEvasion));
    tax *= money;
    _log(MARKET__DEBUG, "ExecuteSellOrder - Buyer is Player: Price: %.2f, Tax: %.2f", money, tax);
    AccountService::TranserFunds(buyer->GetCharacterID(), corpSCC, money, reason.c_str(), \
            Journal::EntryType::TransactionTax, orderID, Account::KeyType::Cash);

    // after money is xferd, create and add item.
    ItemData idata(args.typeID, ownerStation, locTemp, flagNone, args.quantity);
    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);
    if (iRef.get() == nullptr)
        return;

    //use the owner change packet to alert the buyer of the new item
    iRef->Donate(buyer->GetCharacterID(), args.stationID, flagHangar, true);

    // add data to StatisticMgr
    sStatMgr.Add(Stat::iskMarket, money);

    Client* seller(nullptr);
    if (IsCharacter(oInfo.ownerID))
        seller = sEntityList.FindClientByCharID(oInfo.ownerID);

    if (orderConsumed) {
        _log(MARKET__TRACE, "ExecuteSellOrder - satisfied order #%u, deleting.", orderID);
        PyRep* order = m_db.GetOrderRow(orderID);
        if (!m_db.DeleteOrder(orderID)) {
            _log(MARKET__ERROR, "ExecuteSellOrder - Failed to delete order #%u.", orderID);
            return;
        }
        InvalidateOrdersCache(oInfo.regionID, args.typeID);
        SendOnOwnOrderChanged(seller, orderID, Market::Action::Expiry, args.useCorp, order);
    } else {
        uint32 newQty(oInfo.quantity - args.quantity);
        _log(MARKET__TRACE, "ExecuteSellOrder - Partially satisfied order #%u, altering quantity to %u.", orderID, newQty);
        if (!m_db.AlterOrderQuantity(orderID, newQty)) {
            _log(MARKET__ERROR, "ExecuteSellOrder - Failed to alter quantity of order #%u.", orderID);
            return;
        }
        InvalidateOrdersCache(oInfo.regionID, args.typeID);
        SendOnOwnOrderChanged(seller, orderID, Market::Action::Modify, args.useCorp);
    }

    //record this transaction in market_transactions
    /** @todo for corp shit, implement accountKey  */
    Market::TxData data = Market::TxData();
    data.accountKey     = Account::KeyType::Cash;       // args.useCorp?accountKey: Account::KeyType::Cash;
    data.isBuy          = Market::Type::Buy;
    data.isCorp         = args.useCorp;
    data.memberID       = args.useCorp?buyer->GetCharacterID():0;
    data.clientID       = args.useCorp?buyer->GetCorporationID():buyer->GetCharacterID();
    data.price          = args.price;
    data.quantity       = args.quantity;
    data.stationID      = args.stationID;
    data.regionID       = sDataMgr.GetStationRegion(args.stationID);
    data.typeID         = args.typeID;

    if (!m_db.RecordTransaction(data)) {
        _log(MARKET__ERROR, "ExecuteSellOrder - Failed to record buy side of transaction.");
    }

    // update data for other side
    data.accountKey     = Account::KeyType::Cash;       // args.useCorp?accountKey: Account::KeyType::Cash;
    data.isBuy          = Market::Type::Sell;
    data.clientID       = oInfo.ownerID;
    if (!m_db.RecordTransaction(data)) {
        _log(MARKET__ERROR, "ExecuteSellOrder - Failed to record sell side of transaction.");
    }
}

void MarketMgr::SetBasePrice()
{
    /* method to estimate item base price, based on materials to manufacture that item
     * this will fudge cost a bit for material delivery, factory setup, line run cost, and storage of raw and final items
     *
     * eventually, this will query materials prices,
     * then set estimated prices for items based on ME0 BPO
     *
     * mineral prices are queried from db, based on median of buy/sell orders, leveraged with base price in db
     * then, the base price is updated in db and saved for later use
     *
     * ships and modules are loaded and queried from static data for manufacturing materials
     * those materials are queried (as required) for minerals needed
     * once a total mineral value has been calculated, calculate estimated cost based on
     * current mineral values.
     * that cost will be modified for above additions (cost added)
     *
     *
     * NOTES FOR IMPLEMETING THIS SHIT
     *
     *        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
     *        EvERam::RamMaterials ramMatls = EvERam::RamMaterials();
     *        ramMatls.quantity       = row.GetInt(2);
     *        ramMatls.materialTypeID = row.GetInt(1);
     *
     *        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements
     *        EvERam::RamRequirements ramReq = EvERam::RamRequirements();
     *        ramReq.activityID       = row.GetInt(1);
     *        ramReq.requiredTypeID   = row.GetInt(2);
     *        ramReq.quantity         = row.GetInt(3);
     *        ramReq.damagePerJob     = row.GetFloat(4);
     *        ramReq.extra            = row.GetBool(5);
     *
     *        //SELECT blueprintTypeID, parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime,
     *        //  researchTechTime, productivityModifier, materialModifier, wasteFactor, maxProductionLimit, chanceOfRE, catID FROM invBlueprintTypes
     *        EvERam::bpTypeData bpTypeData = EvERam::bpTypeData();
     *        bpTypeData.parentBlueprintTypeID    = row.GetInt(1);
     *        bpTypeData.productTypeID            = row.GetInt(2);
     *        bpTypeData.productionTime           = row.GetInt(3);
     *        bpTypeData.techLevel                = row.GetInt(4);
     *        bpTypeData.researchProductivityTime = row.GetInt(5);
     *        bpTypeData.researchMaterialTime     = row.GetInt(6);
     *        bpTypeData.researchCopyTime         = row.GetInt(7);
     *        bpTypeData.researchTechTime         = row.GetInt(8);
     *        bpTypeData.productivityModifier     = row.GetInt(9);
     *        bpTypeData.materialModifier         = row.GetInt(10);
     *        bpTypeData.wasteFactor              = row.GetInt(11);
     *        bpTypeData.maxProductionLimit       = row.GetInt(12);
     *        bpTypeData.chanceOfRE               = row.GetFloat(13);
     *        bpTypeData.catID                    = row.GetInt(14);
     *
     */

    /* get current mineral prices
     *
     * how to get data?
     *  pull straight from db?
     * ....nah.  hard-code minerals.
     *    it's not like they change
     * - nope.  pull from static data.  duh.
     *
     */
    std::vector< Market::matlData > data;
    sDataMgr.GetMineralData(data);

    //  get mineral prices here and put into data vector
    MarketDB::GetMineralPrices(data);

    // get shipIDs
    //  which ones?
    // start with frigates for testing


    // get minerals required for ship
    std::vector< EvERam::RequiredItem > matVec;

    // sDataMgr.GetRamRequiredItems(typeID, EvERam::Activity::Manufacturing, matVec);

    /*
     *        matVec.typeID = it->second.materialTypeID;
     *        matVec.quantity = it->second.quantity;
     */


    // determine base price of ship based on mineral requirements


    // apply modifier to base price


    // update db for 'new' base price



}
