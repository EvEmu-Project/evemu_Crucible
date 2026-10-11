
 /**
  * @name MarketMgr.cpp
  *   singleton object for storing, manipulating and managing in-game market data
  *   this mgr keeps track of market data without abusing the db on every call. (vs old system)
  *   this mgr will also track mineral pricing, taking monthly averages, then updating base prices accordingly
  *
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
#include <cmath>
#include <limits>
#include <memory>

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
: m_marketGroups(nullptr)
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

int MarketMgr::Initialize(EVEServiceManager& svc)
{
    m_cache = svc.Lookup <ObjCacheService>("objectCaching");
    m_timeStamp = MarketDB::GetUpdateTime();

    Populate();
    sLog.Blue("        MarketMgr", "Market Manager Initialized.");
    return 1;
}

void MarketMgr::Populate()
{
    double start = GetTimeMSeconds();
    m_marketGroups = MarketDB::GetMarketGroups();

    Process();

    // market orders stored as {regionID/typeID}    --do we want to store orders in memory for loaded region??
    // MarketDB::GetOrders(call.client->GetRegionID(), args.arg);

    sLog.Cyan("        MarketMgr", "Market Manager Updates Price History every %u hours.", sConfig.market.HistoryUpdateTime);
    sLog.Blue("        MarketMgr", "Market Manager loaded in %.3fms.", (GetTimeMSeconds() - start));
}

void MarketMgr::GetInfo()
{
    /* get info in current market data? */
}

void MarketMgr::Process()
{
    // make cache timer of xx(time) then invalidate the price history cache

    //if (m_timeStamp > GetFileTimeNow())
    //    UpdatePriceHistory();
}

void MarketMgr::SystemStartup(SolarSystemData& data)
{
    // not used yet.
}

void MarketMgr::SystemShutdown(SolarSystemData& data)
{
    // not used yet.
}

void MarketMgr::UpdatePriceHistory()
{
    m_timeStamp = GetFileTimeNow() + (EvE::Time::Hour * sConfig.market.HistoryUpdateTime);
    MarketDB::SetUpdateTime(m_timeStamp);

    DBerror err;
    int64 cutoff_time = m_timeStamp;
    cutoff_time -= cutoff_time % EvE::Time::Day;    //round down to an even day boundary.
    cutoff_time -= EvE::Time::Day * 2;  //the cutoff between "new" and "old" price history in days

    /** @todo  this doesnt belong here...  */
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
            " WHERE transactionType=%u AND transactionDate < %lli",
            //" GROUP BY regionID, typeID, transactionDate",
            Market::Type::Sell, cutoff_time);

    /** @todo  this doesnt belong here...  */
    // remove the transactions which have been aged out?
    if (sConfig.market.DeleteOldTransactions)
        sDatabase.RunQuery(err, "DELETE FROM mktTransactions WHERE transactionDate < %lli", (cutoff_time - EvE::Time::Year));
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
    if (!this->m_cache->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it
        DBQueryResult res;

        /** @todo  this doesnt belong here...  */
        if (!sDatabase.RunQuery(res,
            "SELECT historyDate, lowPrice, highPrice, avgPrice, volume, orders"
            " FROM mktHistory "
            " WHERE regionID=%u AND typeID=%u"
            " AND historyDate > %lli  LIMIT %u",
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
        this->m_cache->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = this->m_cache->MakeObjectCachedMethodCallResult(method_id);

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
    if (!this->m_cache->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it
        DBQueryResult res;

        /** @todo  this doesnt belong here...  */
        if (!sDatabase.RunQuery(res,
            "SELECT historyDate, lowPrice, highPrice, avgPrice, volume, orders"
            " FROM mktHistory WHERE regionID=%u AND typeID=%u"
            " AND historyDate > %lli AND historyDate < %lli LIMIT %u",
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
        this->m_cache->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = this->m_cache->MakeObjectCachedMethodCallResult(method_id);

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
        ooc.order = MarketDB::GetOrderRow(orderID);
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


void MarketMgr::InvalidateOrdersCache(uint32 regionID, uint32 typeID) {
    std::string method_name ("GetOrders_");
    method_name += std::to_string(regionID);
    method_name += "_";
    method_name += std::to_string(typeID);
    ObjectCachedMethodID method_id("marketProxy", method_name.c_str());
    this->m_cache->InvalidateCache(method_id);
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

/*
 * Does not currently handle aurum transactions, but it would be good to do that
 * in the future.
 */
bool MarketMgr::ExecuteBuyOrder(Client* seller, uint32 orderID, InventoryItemRef source, uint32 quantity, bool useCorp, uint32 typeID, uint32 stationID, double price, uint16 accountKey) {
    DBTransaction transaction;
    if (!transaction.Started()) throw CustomError("The market database is unavailable.");
    Market::OrderInfo order={};
    if (!seller || !source || useCorp || !quantity || quantity>static_cast<uint32>(source->quantity()) || source->isSingleton() ||
        source->ownerID()!=seller->GetCharacterID() || source->typeID()!=typeID || !std::isfinite(price) || price<=0 ||
        !MarketDB::GetOrderInfo(orderID,order) || !order.isBuy || order.typeID!=typeID || order.stationID!=stationID ||
        !order.quantity || !std::isfinite(order.price) || order.price<=0 || order.price<price) return false;
    quantity=std::min(quantity,order.quantity);
    if (quantity<order.minVolume && quantity<order.quantity) return false;
    const bool consumed=quantity==order.quantity;
    const bool playerBuyer=IsCharacterID(order.ownerID) || IsPlayerCorp(order.ownerID);
    const double money=std::round(order.price*quantity*100.0)/100.0;
    if (playerBuyer && (!std::isfinite(order.escrow) || order.escrow<money))
        throw CustomError("The buy order has insufficient escrow; the sale was cancelled.");
    if (playerBuyer) {
        auto delivered=source;
        if (quantity!=static_cast<uint32>(source->quantity())) delivered=source->Split(quantity,true);
        if (!delivered) throw CustomError("Market item transfer failed; the sale was cancelled.");
        delivered->Donate(order.ownerID,stationID,order.isCorp ? flagCorpMarket : flagHangar,true);
    } else if (!source->AlterQuantity(-static_cast<int32>(quantity),true)) {
        throw CustomError("Market item debit failed; the sale was cancelled.");
    }
    const uint32 payer=playerBuyer ? stDataMgr.GetOwnerID(stationID) :
        ((order.ownerID==1 || IsTrader(order.ownerID) || IsTraderJoe(order.ownerID)) ? stDataMgr.GetOwnerID(stationID) : order.ownerID);
    const std::string reason="DESC: Selling market items in "+stDataMgr.GetStationName(stationID);
    AccountService::TransferFunds(payer,seller->GetCharacterID(),money,reason.c_str(),Journal::EntryType::MarketTransaction,
        orderID,playerBuyer ? Account::KeyType::Escrow : Account::KeyType::Cash,accountKey);
    const double tax=std::round(EvEMath::Market::SalesTax(sConfig.market.salesTax,seller->GetChar()->GetSkillLevel(EvESkill::Accounting),
        seller->GetChar()->GetSkillLevel(EvESkill::TaxEvasion))*money*100.0)/100.0;
    AccountService::TransferFunds(seller->GetCharacterID(),corpSCC,tax,reason.c_str(),Journal::EntryType::TransactionTax,orderID,accountKey);
    Market::TxData record={};
    record.accountKey=accountKey; record.isBuy=Market::Type::Sell; record.memberID=seller->GetCharacterID();
    record.clientID=order.ownerID; record.price=order.price; record.quantity=quantity;
    record.stationID=stationID; record.regionID=order.regionID; record.typeID=typeID;
    if (!MarketDB::RecordTransaction(record)) throw CustomError("Market sale record failed.");
    record.accountKey=order.accountKey; record.isBuy=Market::Type::Buy; record.isCorp=order.isCorp;
    record.clientID=seller->GetCharacterID(); record.memberID=order.isCorp ? order.memberID : order.ownerID;
    if (!MarketDB::RecordTransaction(record)) throw CustomError("Market purchase record failed.");
    auto row=std::shared_ptr<PyRep>(MarketDB::GetOrderRow(orderID),[](PyRep* value) { PySafeDecRef(value); });
    if (!row) throw CustomError("Market order could not be read.");
    if (consumed) {
        if (!MarketDB::DeleteOrder(orderID)) throw CustomError("Market buy order completion failed.");
    } else {
        DBerror error; uint32 affected=0;
        const double escrow=playerBuyer ? std::round((order.escrow-money)*100.0)/100.0 : order.escrow;
        if (!sDatabase.RunQuery(error,affected,"UPDATE mktOrders SET volRemaining=%u,escrow=%.2f WHERE orderID=%u",order.quantity-quantity,escrow,orderID) || affected!=1)
            throw CustomError("Market buy order update failed.");
        row=std::shared_ptr<PyRep>(MarketDB::GetOrderRow(orderID),[](PyRep* value) { PySafeDecRef(value); });
        if (!row) throw CustomError("Market order could not be read.");
    }
    if (!transaction.Commit()) throw CustomError("Market sale failed; the exchange was cancelled.");
    DBTransaction::Publish([this,order,orderID,typeID,consumed,money,row]() {
        sStatMgr.Add(Stat::iskMarket,money);
        InvalidateOrdersCache(order.regionID,typeID);
        Client* buyer=sEntityList.FindClientByCharID(order.isCorp ? order.memberID : order.ownerID);
        if (buyer) SendOnOwnOrderChanged(buyer,orderID,consumed ? Market::Action::Expiry : Market::Action::Modify,order.isCorp,row->Clone());
    });
    return true;
}

void MarketMgr::ExecuteSellOrder(Client* buyer, uint32 orderID, uint32 sellQuantity, double price, uint32 stationID, uint32 typeID, bool useCorp) {
    DBTransaction transaction;
    if (!transaction.Started()) throw CustomError("The market database is unavailable.");
    Market::OrderInfo order={};
    if (!buyer || useCorp || !sellQuantity || !std::isfinite(price) || price<=0 ||
        !MarketDB::GetOrderInfo(orderID,order) || order.isBuy || order.typeID!=typeID ||
        order.stationID!=stationID || !order.quantity || !std::isfinite(order.price) || order.price<=0 || order.price>price)
        throw CustomError("The market order changed; refresh the market and try again.");
    sellQuantity=std::min(sellQuantity,order.quantity);
    if (sellQuantity>static_cast<uint32>(std::numeric_limits<int32>::max()))
        throw CustomError("Invalid market quantity.");
    const bool consumed=sellQuantity==order.quantity;
    const double money=std::round(order.price*sellQuantity*100.0)/100.0;
    const uint32 walletOwner=(order.ownerID==1 || IsTrader(order.ownerID) || IsTraderJoe(order.ownerID))
        ? stDataMgr.GetOwnerID(stationID) : (sDataMgr.IsStation(order.ownerID) ? stDataMgr.GetOwnerID(order.ownerID) : order.ownerID);
    const std::string reason="DESC: Buying market items in "+stDataMgr.GetStationName(stationID);
    AccountService::TransferFunds(buyer->GetCharacterID(),walletOwner,money,reason.c_str(),
        Journal::EntryType::MarketTransaction,orderID,Account::KeyType::Cash,order.accountKey);
    const uint32 sellerCharacter=order.isCorp ? order.memberID : order.ownerID;
    if (IsCharacterID(sellerCharacter)) {
        Client* seller=sEntityList.FindClientByCharID(sellerCharacter);
        const uint8 accounting=seller ? seller->GetChar()->GetSkillLevel(EvESkill::Accounting) : CharacterDB::GetSkillLevel(sellerCharacter,EvESkill::Accounting);
        const uint8 evasion=seller ? seller->GetChar()->GetSkillLevel(EvESkill::TaxEvasion) : CharacterDB::GetSkillLevel(sellerCharacter,EvESkill::TaxEvasion);
        const double tax=std::round(EvEMath::Market::SalesTax(sConfig.market.salesTax,accounting,evasion)*money*100.0)/100.0;
        AccountService::TransferFunds(walletOwner,corpSCC,tax,reason.c_str(),Journal::EntryType::TransactionTax,orderID,order.accountKey);
    }
    InventoryItemRef delivered(nullptr);
    if (order.escrowItemID) {
        auto stock=sItemFactory.GetItemRef(order.escrowItemID);
        if (!stock || stock->typeID()!=order.typeID || stock->quantity()!=static_cast<int32>(order.quantity) ||
            stock->locationID()!=stationID || stock->flag()!=flagCorpMarket || stock->ownerID()!=stDataMgr.GetOwnerID(stationID))
            throw CustomError("Reserved market stock changed; the purchase was cancelled.");
        delivered=stock;
        if (!consumed) delivered=stock->Split(sellQuantity,true);
    } else {
        ItemData data(typeID,ownerStation,locTemp,flagNone,sellQuantity);
        delivered=sItemFactory.SpawnItem(data);
    }
    if (!delivered) throw CustomError("Market delivery failed; the purchase was cancelled.");
    delivered->Donate(buyer->GetCharacterID(),stationID,flagHangar,true);
    Market::TxData record={};
    record.accountKey=Account::KeyType::Cash; record.isBuy=Market::Type::Buy; record.memberID=buyer->GetCharacterID();
    record.clientID=order.ownerID; record.price=order.price; record.quantity=sellQuantity;
    record.stationID=stationID; record.regionID=order.regionID; record.typeID=typeID;
    if (!MarketDB::RecordTransaction(record)) throw CustomError("Market transaction record failed.");
    record.accountKey=order.accountKey; record.isBuy=Market::Type::Sell; record.isCorp=order.isCorp;
    record.clientID=buyer->GetCharacterID(); record.memberID=sellerCharacter;
    if (!MarketDB::RecordTransaction(record)) throw CustomError("Market transaction record failed.");
    std::shared_ptr<PyRep> notificationOrder(nullptr,[](PyRep* value) { PySafeDecRef(value); });
    if (consumed) {
        notificationOrder=std::shared_ptr<PyRep>(MarketDB::GetOrderRow(orderID),[](PyRep* value) { PySafeDecRef(value); });
        if (!notificationOrder || !MarketDB::DeleteOrder(orderID)) throw CustomError("Market order completion failed.");
    } else {
        if (!MarketDB::AlterOrderQuantity(orderID,order.quantity-sellQuantity)) throw CustomError("Market order update failed.");
        notificationOrder=std::shared_ptr<PyRep>(MarketDB::GetOrderRow(orderID),[](PyRep* value) { PySafeDecRef(value); });
        if (!notificationOrder) throw CustomError("Market order could not be read.");
    }
    if (!transaction.Commit()) throw CustomError("Market purchase failed; the exchange was cancelled.");
    DBTransaction::Publish([this,order,orderID,typeID,consumed,money,notificationOrder]() {
        sStatMgr.Add(Stat::iskMarket,money);
        InvalidateOrdersCache(order.regionID,typeID);
        Client* seller=sEntityList.FindClientByCharID(order.isCorp ? order.memberID : order.ownerID);
        if (seller) SendOnOwnOrderChanged(seller,orderID,consumed ? Market::Action::Expiry : Market::Action::Modify,order.isCorp,notificationOrder->Clone());
    });
}


// after finding price data from Crucible, this may be moot.   -allan 28Feb21
void MarketMgr::SetBasePrice()
{
    /* method to estimate item base price, based on materials to manufacture that item
     *
     * mineral prices are queried from db with a +10% markup
     *
     * ships and modules are loaded and queried from static data for manufacturing materials
     * those materials are queried (as required) for minerals needed
     * once a total mineral value has been calculated, calculate estimated cost based on
     * current mineral values
     *
     * final prices will have markup based on item type
     *
     *
     * NOTES FOR IMPLEMETING THIS SHIT
     *
     *        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
     *        EvERam::RamMaterials ramMatls = EvERam::RamMaterials();
     *        ramMatls.quantity       = row.GetInt(2);
     *        ramMatls.materialTypeID = row.GetInt(1);
     *
     * eventually, this will query all items that can be manufactured from BP or refined into minerals
     *
     */

    //  get mineral prices and put into data map
    // typeID/data{typeID, price, name}
    std::map<uint16, Market::matlData> mineralMap;
    mineralMap.clear();
    sDataMgr.GetMineralData(mineralMap);        // 8

    // this will have to use db to get current data.
    //  mineral prices are (will be) updated via a 'price average' method yet to be written
    MarketDB::GetMineralPrices(mineralMap);


    // get 'building blocks' used for cap ships and put into data map
    //block typeID/vector<data{materialTypeID, qty}>
    std::map<uint16, Market::matlData> materialMap;
    materialMap.clear();
    sDataMgr.GetComponentData(materialMap);         // 125

    // get compounds from ice and put into data map
    sDataMgr.GetCompoundData(materialMap);         // 7

    // get misc commodities and put into data map
    sDataMgr.GetMiscCommodityData(materialMap);         // 456

    // get salvage items for rigs and other items made from them
    sDataMgr.GetSalvageData(materialMap);         // 53

    // get PI resources
    sDataMgr.GetPIResourceData(materialMap);         // 15

    // get PI commodities
    sDataMgr.GetPICommodityData(materialMap);         // 66

    // hack to add this item to materialMap, instead of getting entire group
    //  22175 is Codebreaker I, which is a reproc item from Purloined Sansha Codebreaker.
    Market::matlData data = Market::matlData();
    data.typeID = 22175;
    data.name = "Codebreaker I";
    materialMap[22175] = data;

    // this will have to use db to get current data.
    //  mineral prices are (will be) updated via a 'price average' method yet to be written
    MarketDB::GetMaterialPrices(materialMap);

    // add minerals to material maps
    materialMap.insert(mineralMap.begin(), mineralMap.end());
    //sDataMgr.GetMineralData(materialMap);        // 8


    // item typeID/data{inventory data}
    std::map<uint16, Inv::TypeData> itemMap;
    itemMap.clear();
    // this gets only ships
    //MarketDB::GetShipIDs(itemMap);
    // this gets all items made from minerals either directly or indirectly
    MarketDB::GetManufacturedItems(itemMap);

    //item typeID/vector<data{materialTypeID, qty}>
    std::map<uint16, std::vector<EvERam::RamMaterials>> itemMatMap;
    itemMatMap.clear();
    std::map<uint16, Inv::TypeData>::iterator itemItr = itemMap.begin();
    for (; itemItr != itemMap.end(); ++itemItr) {
        // pull data for this item  -need r/w iterator to work
        sDataMgr.GetType(itemItr->first, itemItr->second);

        // get materials required for this item
        std::vector<EvERam::RamMaterials> matVec;
        matVec.clear();
        sDataMgr.GetRamMaterials(itemItr->first, matVec);
        itemMatMap[itemItr->first] = matVec;
    }


    // estimate price of item based on mineral requirements
    bool found(true);
    uint8 mLevel(0);
    double current(0);
    Inv::GrpData gData = Inv::GrpData();
    EvERam::bpTypeData bpData = EvERam::bpTypeData();
    // item typeID/data{inventory data}
    std::map<uint16, Inv::TypeData> missingItemMap;
    missingItemMap.clear();
    std::map<uint16, Market::matlData>::iterator materialItr = materialMap.begin();
    std::map<uint16, std::vector<EvERam::RamMaterials>>::iterator itemMatItr = itemMatMap.end();
    for (itemItr = itemMap.begin(); itemItr != itemMap.end(); ++itemItr) {
        itemMatItr = itemMatMap.find(itemItr->first);
        if (itemMatItr == itemMatMap.end())
            continue;

        found = true;
        current = itemItr->second.basePrice;
        // reset basePrice
        itemItr->second.basePrice = 0.0;
        // sum mineral counts with current prices for this ship
        for (auto cur2 : itemMatItr->second) {
            materialItr = materialMap.find(cur2.materialTypeID);
            if (materialItr == materialMap.end()) {
                sLog.Error("     SetBasePrice", "resource %u for %s(%u) not found in materialMap", \
                        cur2.materialTypeID, itemItr->second.name.c_str(), itemItr->first);
                missingItemMap[itemItr->first] = Inv::TypeData();
                found = false;
                continue;
            }
            itemItr->second.basePrice += (materialItr->second.price * cur2.quantity);
        }
        if (!found)
            continue;

        // add manuf costs to base price
        // currently uses production time to add cost at line rental default of 1k install and 2500/hr
        if (sDataMgr.GetBpDataForItem(itemItr->first, bpData)) {
            itemItr->second.basePrice += 1000 + (2500 * (bpData.productionTime / 3600));  // time is in seconds
        }

        mLevel = itemItr->second.metaLvl;
        gData = Inv::GrpData();
        sDataMgr.GetGroup(itemItr->second.groupID, gData);

        // apply modifier to base price according to item category (complexity, rarity, demand)
        // these should also be adjusted for portion size
        switch (gData.catID) {
            case EVEDB::invCategories::Drone:{
                itemItr->second.basePrice /= itemItr->second.portionSize;
                // modify price based on meta
                switch (mLevel) {
                    case 0: {   //basic
                        itemItr->second.basePrice *= 2;
                    } break;
                    case 1: {   //t1
                        itemItr->second.basePrice *= 2.5;
                    } break;
                    case 2: {   //t2
                        itemItr->second.basePrice *= 3.5;
                    } break;
                }
            } break;
            case EVEDB::invCategories::Celestial:
            case EVEDB::invCategories::Entity:
            case EVEDB::invCategories::Commodity:
            case EVEDB::invCategories::Material:
            case EVEDB::invCategories::Charge:{
                if (mLevel)
                    itemItr->second.basePrice *= mLevel;
                itemItr->second.basePrice /= itemItr->second.portionSize;
            } break;
            case EVEDB::invCategories::Asteroid: {
                if (mLevel)
                    itemItr->second.basePrice *= mLevel;
                // asteroids cannot be 'created' per se, but the mined ore can be sold.
                //  this cat covers mined ore, so use same pricing method as charges
                if (itemItr->first < 28000)
                    itemItr->second.basePrice /= itemItr->second.portionSize;
            } break;
            case EVEDB::invCategories::Module: {
                itemItr->second.basePrice *= 2;
                // multiply price by metaLvl
                if (mLevel) {
                    switch (gData.id) {
                        case EVEDB::invGroups::Rig_Mining:
                        case EVEDB::invGroups::Rig_Armor:
                        case EVEDB::invGroups::Rig_Shield:
                        case EVEDB::invGroups::Rig_Energy_Weapon:
                        case EVEDB::invGroups::Rig_Hybrid_Weapon:
                        case EVEDB::invGroups::Rig_Projectile_Weapon:
                        case EVEDB::invGroups::Rig_Drones:
                        case EVEDB::invGroups::Rig_Launcher:
                        case EVEDB::invGroups::Rig_Electronics:
                        case EVEDB::invGroups::Rig_Energy_Grid:
                        case EVEDB::invGroups::Rig_Astronautic:
                        case EVEDB::invGroups::Rig_Electronics_Superiority:
                        case EVEDB::invGroups::Rig_Security_Transponder:  {
                            itemItr->second.basePrice *= (mLevel + 1);
                        } break;
                        default: {
                            itemItr->second.basePrice *= (mLevel + 10);
                        } break;
                    }
                }
            } break;
            case EVEDB::invCategories::Ship: {
                // multiply price by metaLvl
                if (mLevel)
                    itemItr->second.basePrice *= mLevel;
                // modify price based on class
                switch (gData.id) {
                    case EVEDB::invGroups::Frigate:
                    case EVEDB::invGroups::Destroyer:
                    case EVEDB::invGroups::Rookieship:
                    case EVEDB::invGroups::Industrial: {
                        itemItr->second.basePrice *= 2.1;       // +80%
                    } break;
                    case EVEDB::invGroups::CovertOps: {
                        itemItr->second.basePrice *= 1.2;       // +20%
                    } break;
                    case EVEDB::invGroups::Capsule:
                    case EVEDB::invGroups::Cruiser:
                    case EVEDB::invGroups::Freighter:
                    case EVEDB::invGroups::Battleship:
                    case EVEDB::invGroups::Battlecruiser:
                    case EVEDB::invGroups::Interdictor:
                    case EVEDB::invGroups::Interceptor:
                    case EVEDB::invGroups::HeavyInterdictors:
                    case EVEDB::invGroups::IndustrialCommandShip:
                    case EVEDB::invGroups::ElecAttackShip:
                    case EVEDB::invGroups::Supercarrier:
                    case EVEDB::invGroups::StrategicCruiser: {
                        itemItr->second.basePrice *= 1.1;       // +10%
                    } break;
                    // these are good...
                    case EVEDB::invGroups::Carrier:
                    case EVEDB::invGroups::MiningBarge:
                    case EVEDB::invGroups::StealthBomber:
                    case EVEDB::invGroups::CapitalIndustrialShip: {
                        itemItr->second.basePrice *= 1.5;       // +50%
                    } break;
                    case EVEDB::invGroups::Exhumer:
                    case EVEDB::invGroups::Logistics:
                    case EVEDB::invGroups::Marauder:
                    case EVEDB::invGroups::BlackOps:
                    case EVEDB::invGroups::CombatRecon:
                    case EVEDB::invGroups::AssaultShip:
                    case EVEDB::invGroups::TransportShip:
                    case EVEDB::invGroups::HeavyAssaultShip:
                    // these can only be crated in pos.
                    case EVEDB::invGroups::Titan:
                    case EVEDB::invGroups::Dreadnought:
                    case EVEDB::invGroups::CommandShip: {
                        itemItr->second.basePrice *= 0.6;       // -40%  these are all outrageous at calculated prices
                    } break;
                    case EVEDB::invGroups::JumpFreighter:  {
                        itemItr->second.basePrice *= 0.35;        // this one is weird...
                    } break;
                    case EVEDB::invGroups::Shuttle: {
                        itemItr->second.basePrice *= 3;        // x3 for shuttles
                    } break;
                    case EVEDB::invGroups::Prototype_Exploration_Ship:
                    case EVEDB::invGroups::EliteBattleship: {
                        itemItr->second.basePrice *= 1000;        // x1000
                    } break;
                }
            } break;
            case EVEDB::invCategories::Station: {
                // for some reason, station pricing is way off....
                itemItr->second.basePrice *= 100;
            } break;
        }

        if (itemItr->second.basePrice < 0.01) {
            sLog.Error("     SetBasePrice", "Calculated price for %s(%u) is 0", \
                    itemItr->second.name.c_str(), itemItr->first);
        } /*else {
            sLog.Blue("     SetBasePrice", "Calculated price for %u %s(cat:%u - %s - meta %u) from %.2f to %.2f", \
                    itemItr->first, itemItr->second.name.c_str(), gData.catID, gData.name.c_str(), \
                    itemItr->second.metaLvl, current, itemItr->second.basePrice);
        }*/
    }

    // check for missing items and get average price using crucible mkt data table
    if (missingItemMap.empty()) {
        // update db for 'new' base price
        MarketDB::UpdateInvPrice(itemMap);
    } else {
        MarketDB::GetCruPriceAvg(missingItemMap);
        MarketDB::UpdateInvPrice(missingItemMap);
        sLog.Error("     SetBasePrice", "Missing material.  run again.");
    }
}

void MarketMgr::UpdateMineralPrice()
{
    //  get mineral prices and put into data map
    // typeID/data{typeID, price, name}
    std::map<uint16, Market::matlData> mineralMap;
    mineralMap.clear();
    sDataMgr.GetMineralData(mineralMap);        // 8

    //  update mineral price
    MarketDB::UpdateMktPrice(mineralMap);
}

void MarketMgr::GetCruPrices()
{
    /*
    std::map<uint16, Market::matlData> materialMap;
    materialMap.clear();
    // dont update minerals...they are set.
    //sDataMgr.GetMineralData(materialMap);       // 8
    sDataMgr.GetSalvageData(materialMap);       // 53
    sDataMgr.GetCompoundData(materialMap);      // 7
    sDataMgr.GetComponentData(materialMap);     // 114
    sDataMgr.GetPIResourceData(materialMap);    // 15
    sDataMgr.GetPICommodityData(materialMap);   // 66

    MarketDB::GetCruPriceAvg(materialMap);

    MarketDB::UpdateMktPrice(materialMap);
    */

    sLog.Warning("     SetBasePrice", "Getting types.");
    std::map<uint16, Inv::TypeData> types;
    sDataMgr.GetTypes(types);           //19669 unique items in type data
    sLog.Green("     SetBasePrice", "GetTypes returned %lu items.  Getting price avg.", types.size());
    // delete the typeID '0'
    types.erase(0);

    MarketDB::GetCruPriceAvg(types);    //7723 unique items in price data
    sLog.Green("     SetBasePrice", "Got Avg prices.  updating items.");
    MarketDB::UpdateInvPrice(types);    //7712 non-zero prices
    sLog.Cyan("     SetBasePrice", "Completed.");
}
