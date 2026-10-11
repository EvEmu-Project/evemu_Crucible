// Included by the guarded integration harness; these calls use the real RPC dispatcher.
void StandingOrderAudit(EVEServiceManager& services) {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Standing orders require the disposable persistence database");
    auto scalar=[](const char* sql) {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"%s",sql) || !result.GetRow(row)) throw std::runtime_error("Cannot read standing-order fixture");
        return row.GetDouble(0);
    };
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    auto client=SelectMiningAuditCharacter(services);
    const uint32 lastItem=static_cast<uint32>(scalar("SELECT MAX(itemID) FROM entity"));
    const uint32 lastOrder=static_cast<uint32>(scalar("SELECT MAX(orderID) FROM mktOrders"));
    const uint32 lastTransaction=static_cast<uint32>(scalar("SELECT COALESCE(MAX(transactionID),0) FROM mktTransactions"));
    const uint32 lastJournal=static_cast<uint32>(scalar("SELECT COALESCE(MAX(transactionID),0) FROM jnlCharacters"));
    auto call=[&](const char* name,PyTuple* tuple) {
        PyDict* named=new PyDict(); PyCallArgs args(client.get(),tuple,named); PyDecRef(named);
        try { services.Dispatch("marketProxy",name,args); return true; } catch (const PyException& error) { error.ssException->Dump(CLIENT__ERROR,"standing audit rejection: "); return false; }
    };
    const uint32 station=60014689;
    auto place=[&](bool buy,uint32 item,double price=1.25,int32 quantity=10,int32 type=34) {
        PyTuple* tuple=new PyTuple(11);
        tuple->SetItem(0,new PyInt(station)); tuple->SetItem(1,new PyInt(type)); tuple->SetItem(2,new PyFloat(price));
        tuple->SetItem(3,new PyInt(quantity)); tuple->SetItem(4,new PyInt(buy?1:0)); tuple->SetItem(5,new PyInt(-1));
        tuple->SetItem(6,item ? static_cast<PyRep*>(new PyInt(item)) : PyStatic.NewNone());
        tuple->SetItem(7,new PyInt(1)); tuple->SetItem(8,new PyInt(30)); tuple->SetItem(9,new PyBool(false)); tuple->SetItem(10,PyStatic.NewNone());
        return call("PlaceCharOrder",tuple);
    };
    auto cancel=[&](uint32 id) {
        PyTuple* tuple=new PyTuple(2); tuple->SetItem(0,new PyInt(id)); tuple->SetItem(1,new PyInt(0)); return call("CancelCharOrder",tuple);
    };
    auto modify=[&](uint32 id,double price,double fakeOld,int32 fakeQuantity) {
        PyTuple* tuple=new PyTuple(9);
        tuple->SetItem(0,new PyInt(id)); tuple->SetItem(1,new PyFloat(price)); tuple->SetItem(2,new PyInt(0));
        tuple->SetItem(3,new PyInt(60003760)); tuple->SetItem(4,new PyInt(30000142)); tuple->SetItem(5,new PyFloat(fakeOld));
        tuple->SetItem(6,new PyInt(0)); tuple->SetItem(7,new PyInt(fakeQuantity)); tuple->SetItem(8,new PyLong(0));
        return call("ModifyCharOrder",tuple);
    };
    auto order=[&](bool buy,uint32 owner=ownerID,double price=2.0,double escrow=20.0) {
        Market::SaveData data={}; data.bid=buy; data.ownerID=owner; data.typeID=34; data.stationID=station;
        data.regionID=sDataMgr.GetStationRegion(station); data.solarSystemID=sDataMgr.GetStationSystem(station);
        data.price=price; data.escrow=buy?escrow:0; data.volEntered=data.volRemaining=10; data.minVolume=1;
        data.accountKey=Account::KeyType::Cash; data.issued=GetFileTimeNow(); data.duration=30;
        const uint32 id=MarketDB::StoreOrder(data); if (!id) throw std::runtime_error("Cannot seed standing order"); return id;
    };
    auto reset=[&]() {
        Query("DROP TRIGGER IF EXISTS core_audit_standing_failure");
        DBerror error;
        sDatabase.RunQuery(error,"DELETE FROM mktOrders WHERE orderID>%u",lastOrder);
        sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE transactionID>%u",lastJournal);
        sDatabase.RunQuery(error,"DELETE FROM mktTransactions WHERE transactionID>%u",lastTransaction);
        DBQueryResult items; DBResultRow row; std::vector<uint32> cleanup;
        sDatabase.RunQuery(items,"SELECT itemID FROM entity WHERE itemID>%u",lastItem);
        while (items.GetRow(row)) cleanup.push_back(row.GetUInt(0));
        for (uint32 id:cleanup) { auto item=sItemFactory.GetItemRef(id); if (item) item->Delete(); }
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        client->GetChar()->SetBalanceFromDatabase(5000,Account::CreditType::ISK);
    };
    auto funded=[&](double value) {
        DBerror error; sDatabase.RunQuery(error,"UPDATE chrCharacters SET balance=%.2f WHERE characterID=90000000",value);
        client->GetChar()->SetBalanceFromDatabase(value,Account::CreditType::ISK);
    };
    auto newOrders=[&]() { DBQueryResult result; DBResultRow row; sDatabase.RunQuery(result,"SELECT COUNT(*) FROM mktOrders WHERE orderID>%u",lastOrder); result.GetRow(row); return row.GetUInt(0); };
    auto latestOrder=[&]() { DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"SELECT orderID FROM mktOrders WHERE orderID>%u ORDER BY orderID DESC LIMIT 1",lastOrder) || !result.GetRow(row)) throw std::runtime_error("Standing order was not created");
        return row.GetUInt(0);
    };
    Check(place(true,0) && newOrders()==1 && client->GetBalance()==4887.5,"standing buy placement charges one broker fee and full escrow"); reset();
    DBerror error;
    sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_standing_failure BEFORE INSERT ON jnlCharacters FOR EACH ROW BEGIN IF NEW.entryTypeID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected escrow journal'; END IF; END",Journal::EntryType::MarketEscrow);
    place(true,0);
    Check(newOrders()==0 && client->GetBalance()==5000 && scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000,
        "failed standing buy escrow restores broker fee and removes the unfunded order"); reset();
    place(true,0,1000,10);
    Check(newOrders()==0 && client->GetBalance()==5000,"insufficient standing-buy funds cannot leave an order or broker debit"); reset();
    auto source=Stack(100); source->Move(station,flagHangar,false);
    Query("CREATE TRIGGER core_audit_standing_failure BEFORE INSERT ON mktOrders FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected order creation'");
    place(false,source->itemID());
    Check(newOrders()==0 && client->GetBalance()==5000 && source->quantity()==100 && Stored(source,100,station),"failed sell-order creation restores its broker fee and stock"); reset();
    source=Stack(100); source->Move(station,flagHangar,false);
    sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_standing_failure BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected sell stock debit'; END IF; END",source->itemID());
    place(false,source->itemID());
    Check(newOrders()==0 && client->GetBalance()==5000 && source->quantity()==100 && Stored(source,100,station),"failed sell-stock debit restores its fee and order creation"); reset();
    uint32 id=order(true); funded(4980);
    Query("CREATE TRIGGER core_audit_standing_failure BEFORE DELETE ON mktOrders FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected order cancellation'");
    cancel(id); Market::OrderInfo info={};
    Check(MarketDB::GetOrderInfo(id,info) && info.escrow==20 && client->GetBalance()==4980,"failed buy cancellation cannot refund escrow while retaining the order"); reset();
    id=order(false);
    Query("CREATE TRIGGER core_audit_standing_failure BEFORE INSERT ON entity FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected returned sell stock'");
    cancel(id);
    Check(MarketDB::GetOrderInfo(id,info) && info.quantity==10,"failed sell cancellation retains the order when item delivery is rejected"); reset();
    id=order(true,90000001); cancel(id);
    Check(MarketDB::GetOrderInfo(id,info) && client->GetBalance()==5000,"another player's order cannot be cancelled for a stolen escrow refund"); reset();
    id=order(true,90000001); modify(id,2.5,2.5,10);
    Check(MarketDB::GetOrderInfo(id,info) && info.price==2,"another player's order cannot be modified"); reset();
    id=order(true); funded(4980); modify(id,3,100,999);
    Check(MarketDB::GetOrderInfo(id,info) && info.price==3 && info.escrow==30 && client->GetBalance()==4970,"buy price increase uses stored volume and charges only additional escrow"); reset();
    id=order(true,ownerID,3,30); funded(4970); modify(id,2,3,10);
    Check(MarketDB::GetOrderInfo(id,info) && info.price==2 && info.escrow==20 && client->GetBalance()==4980,"buy price decrease refunds the excess recorded escrow"); reset();
    ItemData blueprintData(691,ownerID,station,flagHangar,1);
    auto blueprint=BlueprintRef::StaticCast(sItemFactory.SpawnItem(blueprintData));
    blueprint->SetMLevel(7); blueprint->SetPLevel(3); blueprint->SetRuns(12);
    const bool listed=place(false,blueprint->itemID(),1.25,1,691);
    ItemData saved;
    Check(listed && ItemDB::GetItemData(blueprint->itemID(),saved) && saved.ownerID==stDataMgr.GetOwnerID(station) && saved.flag==flagCorpMarket && newOrders()==1,"standing sell order retains its actual item and metadata in escrow");
    DBQueryResult result; DBResultRow row; sDatabase.RunQuery(result,"SELECT orderID FROM mktOrders WHERE orderID>%u ORDER BY orderID DESC LIMIT 1",lastOrder);
    if (result.GetRow(row)) cancel(row.GetUInt(0));
    sDatabase.RunQuery(result,"SELECT mLevel,pLevel,runs FROM invBlueprints WHERE itemID=%u",blueprint->itemID());
    const bool metadata=result.GetRow(row) && row.GetInt(0)==7 && row.GetInt(1)==3 && row.GetInt(2)==12;
    Check(ItemDB::GetItemData(blueprint->itemID(),saved) && saved.ownerID==ownerID && saved.locationID==station && saved.flag==flagHangar && metadata,
        "sell cancellation returns the original blueprint identity and research metadata"); reset();

    source=Stack(100); source->Move(station,flagHangar,false);
    Check(place(false,source->itemID()) && source->quantity()==90 && client->GetBalance()==4900,"standing sell placement reserves only the listed quantity and charges one broker fee");
    id=latestOrder(); MarketDB::GetOrderInfo(id,info);
    auto reserved=sItemFactory.GetItemRef(info.escrowItemID);
    sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_standing_failure BEFORE DELETE ON mktOrders FOR EACH ROW BEGIN IF OLD.orderID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected retained cancellation'; END IF; END",id);
    Check(!cancel(id) && MarketDB::GetOrderInfo(id,info) && reserved->ownerID()==stDataMgr.GetOwnerID(station) && reserved->flag()==flagCorpMarket && reserved->quantity()==10,
        "failed retained-stock cancellation restores reserved ownership and order mapping");
    Query("DROP TRIGGER core_audit_standing_failure");
    Check(cancel(id) && !MarketDB::GetOrderInfo(id,info) && reserved->ownerID()==ownerID && Stored(reserved,10,station) && reserved->flag()==flagHangar,
        "successful retained-stock cancellation returns all remaining stock exactly once");
    Check(!cancel(id) && reserved->quantity()==10 && client->GetBalance()==4900,"repeated sell cancellation cannot duplicate returned stock or fees"); reset();

    id=order(true); funded(4980);
    Query("CREATE TRIGGER core_audit_standing_failure BEFORE UPDATE ON mktOrders FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected price update'");
    Check(!modify(id,3,999,999) && MarketDB::GetOrderInfo(id,info) && info.price==2 && info.escrow==20 && client->GetBalance()==4980,
        "failed price update rolls back additional escrow and the live wallet"); reset();
    id=order(true); funded(4980); cancel(id);
    Check(!MarketDB::GetOrderInfo(id,info) && client->GetBalance()==5000,"buy cancellation returns exactly its recorded escrow");
    Check(!cancel(id) && client->GetBalance()==5000,"repeated buy cancellation cannot duplicate escrow refunds"); reset();
    Check(!place(true,0,std::numeric_limits<double>::quiet_NaN()) && !place(true,0,-1) && !place(true,0,1,0) && newOrders()==0 && client->GetBalance()==5000,
        "invalid standing prices and quantities cannot create orders or debit fees"); reset();

    Query("INSERT INTO chrCharacters(characterID,characterName,balance) VALUES(90000001,'Standing buyer',5000) ON DUPLICATE KEY UPDATE balance=5000");
    auto buyerBalance=[&]() { return scalar("SELECT balance FROM chrCharacters WHERE characterID=90000001"); };
    auto purchase=[&](uint32 quantity,uint32 type) {
        // The loaded actor is the buyer; the order owner uses the offline wallet path.
        sMktMgr.ExecuteSellOrder(client.get(),id,quantity,1.25,station,type,false);
    };
    ItemData metadataData(34,ownerID,station,flagHangar,10);
    metadataData.customInfo="standing audit metadata";
    source=sItemFactory.SpawnItem(metadataData);
    source->SetAttribute(AttrMass,42,false);
    sDatabase.RunQuery(error,"INSERT INTO entity_attributes(itemID,attributeID,valueInt) VALUES(%u,%u,42)",source->itemID(),AttrMass);
    Check(place(false,source->itemID(),1.25,10) && newOrders()==1,"full-stack sell listing retains the original entity identity");
    id=latestOrder();
    // Use a different offline seller to check conservation without a self-purchase.
    sDatabase.RunQuery(error,"UPDATE mktOrders SET ownerID=90000001 WHERE orderID=%u",id);
    Query("CREATE TRIGGER core_audit_standing_failure BEFORE INSERT ON mktTransactions FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected retained delivery record'");
    bool rejected=false; try { purchase(4,34); } catch (...) { rejected=true; }
    Check(rejected && MarketDB::GetOrderInfo(id,info) && info.quantity==10 && source->quantity()==10 && source->flag()==flagCorpMarket && client->GetBalance()==4900 && buyerBalance()==5000,
        "failed partial retained-stock fill rolls back both wallets and all item quantities");
    Query("DROP TRIGGER core_audit_standing_failure"); purchase(4,34);
    Check(MarketDB::GetOrderInfo(id,info) && info.quantity==6 && info.escrowItemID==source->itemID() && source->quantity()==6,
        "partial retained-stock fill keeps the remaining order and original stock in step");
    DBQueryResult delivered; DBResultRow deliveredRow;
    sDatabase.RunQuery(delivered,"SELECT e.itemID,e.quantity,e.customInfo,COALESCE(a.valueInt,a.valueFloat) FROM entity e JOIN entity_attributes a ON a.itemID=e.itemID AND a.attributeID=%u WHERE e.itemID>%u AND e.ownerID=90000000 AND e.flag=4 AND e.customInfo='standing audit metadata'",AttrMass,source->itemID());
    const bool foundDelivery=delivered.GetRow(deliveredRow);
    if (foundDelivery) std::printf("CORE_AUDIT DETAIL retained delivery qty=%i mass=%.2f info=%s\n",deliveredRow.GetInt(1),deliveredRow.GetDouble(3),deliveredRow.GetText(2));
    Check(foundDelivery && deliveredRow.GetInt(1)==4 && !std::strcmp(deliveredRow.GetText(2),"standing audit metadata") && deliveredRow.GetInt(3)==42,
        "partial fill carries custom item metadata and saved attributes into the delivered stack");
    const double tax=std::round(EvEMath::Market::SalesTax(sConfig.market.salesTax,0,0)*5*100)/100;
    std::printf("CORE_AUDIT DETAIL wallets buyer=%.2f seller=%.2f expected-tax=%.2f\n",client->GetBalance(),buyerBalance(),tax);
    Check(client->GetBalance()==4895 && buyerBalance()==std::round((5005-tax)*100)/100,"partial fill pays only the actual purchased quantity and records sales tax");
    purchase(6,34);
    Check(!MarketDB::GetOrderInfo(id,info) && source->ownerID()==ownerID && source->flag()==flagHangar && Stored(source,6,station),
        "final retained-stock fill delivers the original entity and removes its order");
    Check(scalar("SELECT COUNT(*) FROM mktOrderItems")==0,"filled and cancelled orders leave no stock mappings"); reset();
    Query("DELETE FROM chrCharacters WHERE characterID=90000001");

    const double largePrice=100000000.25;
    const double largeFee=std::round(EvEMath::Market::BrokerFee(client->GetChar()->GetSkillLevel(EvESkill::BrokerRelations),
        StandingDB::GetStanding(sDataMgr.GetCorpFaction(stDataMgr.GetOwnerID(station)),ownerID),StandingDB::GetStanding(stDataMgr.GetOwnerID(station),ownerID),largePrice)*100)/100;
    funded(200000000); Check(place(true,0,largePrice,1),"ordinary player can place a fully funded large-price buy order");
    id=latestOrder(); MarketDB::GetOrderInfo(id,info);
    Check(info.escrow==largePrice,"large buy-order escrow preserves the final quarter ISK in durable storage");
    cancel(id);
    Check(client->GetBalance()==200000000-largeFee,"large buy cancellation returns every escrow cent while retaining the broker fee"); reset();
    funded(200000000);
    Market::SaveData largeOrder={}; largeOrder.ownerID=1; largeOrder.typeID=34; largeOrder.stationID=station;
    largeOrder.regionID=sDataMgr.GetStationRegion(station); largeOrder.solarSystemID=sDataMgr.GetStationSystem(station);
    largeOrder.price=largePrice; largeOrder.volEntered=largeOrder.volRemaining=1; largeOrder.minVolume=1;
    largeOrder.accountKey=Account::KeyType::Cash; largeOrder.duration=30; largeOrder.issued=GetFileTimeNow();
    id=MarketDB::StoreOrder(largeOrder); sMktMgr.ExecuteSellOrder(client.get(),id,1,largePrice,station,34,false);
    sDatabase.RunQuery(result,"SELECT price FROM mktTransactions WHERE transactionID>%u ORDER BY transactionID",lastTransaction);
    bool exact=true; uint32 records=0; while(result.GetRow(row)) { exact=exact && row.GetDouble(0)==largePrice; ++records; }
    Check(exact && records==2 && client->GetBalance()==99999999.75,"large market purchase records preserve exactly the price charged to the wallet"); reset();
}
