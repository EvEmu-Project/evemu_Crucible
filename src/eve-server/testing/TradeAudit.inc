// Guarded integration fixture. Persistent snapshots also permit recovery after a killed test.
double TradeScalar(const char* sql) {
    DBQueryResult result; DBResultRow row;
    if (!sDatabase.RunQuery(result,"%s",sql) || !result.GetRow(row)) throw std::runtime_error("Cannot read trade fixture");
    return row.GetDouble(0);
}
void TradeFixtureGuard() {
    DBQueryResult result; DBResultRow row;
    if (!sDatabase.RunQuery(result,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !result.GetRow(row) ||
        (std::strcmp(row.GetText(0),"isolated-persistence-audit") && std::strcmp(row.GetText(0),"isolated-travel-combat-audit")))
        throw std::runtime_error("Trade fixtures require the disposable persistence database");
}
void TradeFixturePrepare() {
    TradeFixtureGuard();DBQueryResult result;DBResultRow row;
    if (TradeScalar("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema=DATABASE() AND table_name='coreAuditTradeState'") ||
        TradeScalar("SELECT COUNT(*) FROM chrCharacters WHERE characterID=90000001") || TradeScalar("SELECT COUNT(*) FROM account WHERE accountID=2") || TradeScalar("SELECT COUNT(*) FROM entity WHERE ownerID=90000001"))
        throw std::runtime_error("Trade fixture already exists; inspect and recover it before another run");
    Query("CREATE TABLE coreAuditTradeState AS SELECT MAX(itemID) AS lastItem FROM entity");
    Query("ALTER TABLE coreAuditTradeState ADD lastJournal INT UNSIGNED");
    Query("UPDATE coreAuditTradeState SET lastJournal=(SELECT COALESCE(MAX(transactionID),0) FROM jnlCharacters)");
    Query("CREATE TABLE coreAuditTradeCharacters AS SELECT * FROM chrCharacters WHERE characterID=90000000");
    Query("CREATE TABLE coreAuditTradeAccount AS SELECT * FROM account WHERE accountID=1");
    Query("CREATE TABLE coreAuditTradeCharAttrs AS SELECT * FROM chrCharacterAttributes WHERE charID=90000000");
    Query("CREATE TABLE coreAuditTradeItems AS SELECT * FROM entity WHERE ownerID=90000000");
    Query("CREATE TABLE coreAuditTradeItemAttrs AS SELECT a.* FROM entity_attributes a JOIN coreAuditTradeItems i ON i.itemID=a.itemID");
    Query("CREATE TABLE coreAuditTradeWeaponGroups AS SELECT * FROM shipWeaponGroups WHERE shipID IN (SELECT itemID FROM coreAuditTradeItems)");
    Query("CREATE TEMPORARY TABLE coreAuditTradeClone AS SELECT * FROM coreAuditTradeCharacters");
    Query("UPDATE coreAuditTradeClone SET characterID=90000001,accountID=2,characterName='Core Audit Trade',balance=5000,stationID=60014689,locationID=60014689,online=0");
    Query("INSERT INTO chrCharacters SELECT * FROM coreAuditTradeClone");
    Query("DROP TEMPORARY TABLE coreAuditTradeClone");
    Query("INSERT INTO account(accountID,accountName,role) VALUES (2,'CoreAuditTrade',6917529029788565504)");
    Query("INSERT INTO chrCharacterAttributes SELECT 90000001,attributeID,valueInt,valueFloat FROM coreAuditTradeCharAttrs");
    Query("UPDATE chrCharacters SET stationID=60014689,locationID=60014689,balance=5000,online=0 WHERE characterID=90000000");
    Query("UPDATE entity SET locationID=60014689,flag=4 WHERE itemID IN (140000012,140000013)");
    std::vector<uint32> originals;
    if (!sDatabase.RunQuery(result,"SELECT itemID FROM coreAuditTradeItems WHERE flag=7 OR itemID IN (140000012,140000013)")) throw std::runtime_error("Cannot clone trade character inventory");
    while (result.GetRow(row)) originals.push_back(row.GetUInt(0));
    for (uint32 id:originals) {
        ItemData data;
        if (!ItemDB::GetItemData(id,data)) throw std::runtime_error("Cannot load source fixture item");
        data.ownerID=90000001;
        data.locationID=(data.flag==flagSkill ? 90000001 : 60014689);
        const uint32 child=ItemDB::NewItem(data);
        DBerror error;
        if (!child || !sDatabase.RunQuery(error,"INSERT INTO entity_attributes SELECT %u,attributeID,valueInt,valueFloat FROM entity_attributes WHERE itemID=%u",child,id)) throw std::runtime_error("Cannot clone trade item attributes");
        if (id==140000012 || id==140000013) {
            if (!sDatabase.RunQuery(error,"UPDATE chrCharacters SET %s=%u WHERE characterID=90000001",id==140000012 ? "capsuleID" : "shipID",child)) throw std::runtime_error("Cannot set trade ship identity");
        }
    }
}
void TradeFixtureRestore() {
    TradeFixtureGuard();
    Query("DROP TRIGGER IF EXISTS core_audit_trade_failure");
    Query("DROP TRIGGER IF EXISTS core_audit_trade_pause");
    Query("DELETE FROM chrTradeItems");
    DBQueryResult cached;DBResultRow row;std::vector<uint32> ids;
    if (!sDatabase.RunQuery(cached,"SELECT itemID FROM entity WHERE itemID IN (SELECT itemID FROM coreAuditTradeItems) OR itemID>(SELECT lastItem FROM coreAuditTradeState)")) throw std::runtime_error("Cannot enumerate fixture caches");
    while (cached.GetRow(row)) ids.push_back(row.GetUInt(0));
    const uint32 first=static_cast<uint32>(TradeScalar("SELECT lastItem FROM coreAuditTradeState"))+1;
    const uint32 last=static_cast<uint32>(TradeScalar("SELECT AUTO_INCREMENT-1 FROM information_schema.tables WHERE table_schema=DATABASE() AND table_name='entity'"));
    if (last-first>10000) throw std::runtime_error("Unexpectedly large trade fixture allocation range");
    for (uint32 id=first;id<=last;++id) ids.push_back(id);
    for (uint32 id:ids) {auto item=sItemFactory.GetItemRefFromID(id,false);if (item) item->DiscardUncommitted();}
    sItemFactory.RemoveItem(90000000);sItemFactory.RemoveItem(90000001);
    Query("DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM coreAuditTradeItems) OR itemID>(SELECT lastItem FROM coreAuditTradeState)");
    Query("DELETE FROM invBlueprints WHERE itemID>(SELECT lastItem FROM coreAuditTradeState)");
    Query("DELETE FROM shipInsurance WHERE shipID>(SELECT lastItem FROM coreAuditTradeState)");
    Query("DELETE FROM entity WHERE itemID IN (SELECT itemID FROM coreAuditTradeItems) OR itemID>(SELECT lastItem FROM coreAuditTradeState)");
    Query("INSERT INTO entity SELECT * FROM coreAuditTradeItems");
    Query("INSERT INTO entity_attributes SELECT * FROM coreAuditTradeItemAttrs");
    Query("DELETE FROM shipWeaponGroups WHERE shipID IN (SELECT itemID FROM coreAuditTradeItems) OR shipID>(SELECT lastItem FROM coreAuditTradeState)");
    Query("INSERT INTO shipWeaponGroups SELECT * FROM coreAuditTradeWeaponGroups");
    Query("DELETE FROM chrCharacterAttributes WHERE charID IN (90000000,90000001)");
    Query("INSERT INTO chrCharacterAttributes SELECT * FROM coreAuditTradeCharAttrs");
    Query("DELETE FROM chrCharacters WHERE characterID IN (90000000,90000001)");
    Query("INSERT INTO chrCharacters SELECT * FROM coreAuditTradeCharacters");
    Query("DELETE FROM account WHERE accountID IN (1,2)");
    Query("INSERT INTO account SELECT * FROM coreAuditTradeAccount");
    Query("DELETE FROM jnlCharacters WHERE transactionID>(SELECT lastJournal FROM coreAuditTradeState)");
    Query("DROP TABLE coreAuditTradeItems,coreAuditTradeItemAttrs,coreAuditTradeCharacters,coreAuditTradeAccount,coreAuditTradeCharAttrs,coreAuditTradeState,coreAuditTradeWeaponGroups");
    Check(TradeScalar("SELECT COUNT(*) FROM entity WHERE ownerID=90000001")==0,"trade fixture removes every temporary player item and prevents stale cache resurrection");
}
std::unique_ptr<Client> TradeClient(EVEServiceManager& services,uint32 id,uint32 account) {
    EVETCPConnection* connection=new EVETCPConnection();
    std::unique_ptr<Client> client(new Client(services,&connection));
    client->GetSession()->SetInt("userid",account);
    client->GetSession()->SetLong("role",6917529029788565504LL);
    client->GetSession()->SetString("address","127.0.0.1"); client->GetSession()->SetString("languageID","EN");
    if (!client->SelectCharacter(id) || !client->IsDocked()) throw std::runtime_error("Cannot select docked trade character");
    return client;
}
BoundID TradeBind(EVEServiceManager& services,Client* client) {
    auto parameters=new PyTuple(5);parameters->SetItem(0,new PyInt(60014689));
    auto parties=new PyTuple(2);parties->SetItem(0,new PyInt(90000000));parties->SetItem(1,new PyInt(90000001));parameters->SetItem(1,parties);
    parameters->SetItem(2,new PyFloat(999));parameters->SetItem(3,new PyFloat(999));parameters->SetItem(4,new PyLong(0));
    auto tuple=new PyTuple(2);tuple->SetItem(0,parameters);tuple->SetItem(1,PyStatic.NewNone());
    auto named=new PyDict();PyCallArgs args(client,tuple,named);PyDecRef(named);
    auto result=services.Dispatch("trademgr","MachoBindObject",args);
    if (!result.ssNamedResult) throw std::runtime_error("Trade binding returned no OID");
    auto oid=result.ssNamedResult->GetItemString("OID+");
    if (!oid || !oid->IsTuple()) throw std::runtime_error("Invalid trade OID");
    const std::string id=oid->AsTuple()->GetItem(0)->AsString()->content();
    return static_cast<BoundID>(std::stoul(id.substr(id.rfind(':')+1)));
}
bool TradeCall(EVEServiceManager& services,Client* client,BoundID bound,const char* name,PyTuple* tuple,PyDict* named=nullptr) {
    if (!named) named=new PyDict();PyCallArgs args(client,tuple,named);PyDecRef(named);
    try {services.Dispatch(bound,name,args);return true;}catch (const PyException& error) {error.ssException->Dump(CLIENT__ERROR,"trade audit rejection: ");return false;}
}
bool TradeCash(EVEServiceManager& services,Client* client,BoundID bound,double money) {
    auto tuple=new PyTuple(1);tuple->SetItem(0,new PyFloat(money));return TradeCall(services,client,bound,"OfferMoney",tuple);
}
bool TradeAccept(EVEServiceManager& services,Client* client,BoundID bound,bool force=false) {
    auto tuple=new PyTuple(1);tuple->SetItem(0,new PyBool(true));auto named=new PyDict();if (force) named->SetItemString("forceTrade",new PyBool(true));
    return TradeCall(services,client,bound,"ToggleAccept",tuple,named);
}
bool TradeAdd(EVEServiceManager& services,Client* client,BoundID bound,uint32 id,int32 quantity=0,uint32 location=60014689) {
    auto tuple=new PyTuple(2);tuple->SetItem(0,new PyInt(id));tuple->SetItem(1,new PyInt(location));auto named=new PyDict();named->SetItemString("qty",new PyInt(quantity));
    return TradeCall(services,client,bound,"Add",tuple,named);
}
void TradeAudit(EVEServiceManager& services) {
    TradeFixturePrepare();
    auto first=TradeClient(services,90000000,1); auto second=TradeClient(services,90000001,2);
    TradeService* service=services.Lookup<TradeService>("trademgr");
    auto bound=new TradeBound(services,*service); bound->NewReference(first.get());
    const uint32 journal=static_cast<uint32>(TradeScalar("SELECT COALESCE(MAX(transactionID),0) FROM jnlCharacters"));
    auto wallet=[&](uint32 id,double amount) {
        DBerror error; if (!sDatabase.RunQuery(error,"UPDATE chrCharacters SET balance=%.2f WHERE characterID=%u",amount,id)) throw std::runtime_error("Cannot fund trade fixture");
        (id==90000000 ? first : second)->GetChar()->SetBalanceFromDatabase(amount,Account::CreditType::ISK);
    };
    auto journals=[&]() { DBQueryResult result; DBResultRow row; sDatabase.RunQuery(result,"SELECT COUNT(*) FROM jnlCharacters WHERE transactionID>%u",journal); result.GetRow(row); return row.GetUInt(0); };
    auto reset=[&]() { Query("DROP TRIGGER IF EXISTS core_audit_trade_failure"); Query("DELETE FROM chrTradeItems"); DBerror error; sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE transactionID>%u",journal); wallet(90000000,5000);wallet(90000001,5000); };
    auto session=[&]() {
        auto trade=new TradeSession();
        trade->m_tradeSession={service->GetTradeSessionID(),60014689,90000000,90000001,true,true,100.25,50.25,static_cast<int64>(GetFileTimeNow())};
        first->SetTradeSession(trade); second->SetTradeSession(trade);
        TradeService::ActiveSession active={90000000,90000001,trade->m_tradeSession.containerID,trade};
        service->m_activeSessions[90000000]=active;service->m_activeSessions[90000001]=active;
        return trade;
    };
    auto finish=[&](TradeSession* trade) { first->ClearTradeSession();second->ClearTradeSession();service->RemoveActiveSession(90000000);service->RemoveActiveSession(90000001);delete trade; };
    auto exchange=[&](TradeSession* trade) { try { bound->ExchangeItems(first.get(),second.get(),trade);return true; } catch (const PyException&) {return false;} catch (const std::exception&) {return false;} };
    auto unchanged=[&]() {return first->GetBalance()==5000 && second->GetBalance()==5000 && TradeScalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000 && TradeScalar("SELECT balance FROM chrCharacters WHERE characterID=90000001")==5000 && journals()==0;};
    auto offer=[&](TradeSession* trade,uint32 owner) {
        ItemData data(34,owner,60014689,flagHangar,20); auto item=sItemFactory.SpawnItem(data);
        if (!item) throw std::runtime_error("Cannot create trade offer");
        DBerror error; if (!sDatabase.RunQuery(error,"INSERT INTO chrTradeItems VALUES (%u,%u,%u,60014689,60014689,4)",item->itemID(),trade->m_tradeSession.containerID,owner)) throw std::runtime_error("Cannot reserve trade fixture item");
        item->Move(trade->m_tradeSession.containerID,flagNone,false);
        trade->m_tradelist.push_back({item->itemID(),item->typeID(),owner,trade->m_tradeSession.containerID,0,20,item->groupID(),item->categoryID(),false,item->customInfo()});
        return item;
    };
    auto trade=session();
    DBerror error;
    sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_trade_failure BEFORE INSERT ON jnlCharacters FOR EACH ROW BEGIN IF NEW.ownerID=90000001 AND NEW.ownerID1=90000001 AND NEW.entryTypeID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected second trade payment'; END IF; END",Journal::EntryType::PlayerTrading);
    exchange(trade);
    Check(unchanged(),"second trade payment failure rolls back both wallets, live balances and journals");
    finish(trade);reset();
    trade=session();auto a=offer(trade,90000000);auto b=offer(trade,90000001);
    sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_trade_failure BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u AND NEW.ownerID<>OLD.ownerID THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected second trade item'; END IF; END",b->itemID());
    exchange(trade);
    ItemData storedA,storedB;ItemDB::GetItemData(a->itemID(),storedA);ItemDB::GetItemData(b->itemID(),storedB);
    Check(unchanged() && a->ownerID()==90000000 && b->ownerID()==90000001 && storedA.ownerID==90000000 && storedB.ownerID==90000001 && storedA.locationID==trade->m_tradeSession.containerID && storedB.locationID==trade->m_tradeSession.containerID,
        "later trade item failure rolls back earlier items and both payments");
    Query("DROP TRIGGER core_audit_trade_failure");Query("DELETE FROM chrTradeItems");a->Delete();b->Delete();finish(trade);reset();
    trade=session();wallet(90000001,0);trade->m_tradeSession.herMoney=50;
    const bool accepted=exchange(trade);
    Check(!accepted && first->GetBalance()==5000 && second->GetBalance()==0 && journals()==0,"incoming trade money cannot finance an unfunded outgoing offer");
    finish(trade);reset();
    trade=session();trade->m_tradelist.push_back({199999999,34,90000000,trade->m_tradeSession.containerID,0,20,18,4,false,""});
    exchange(trade);Check(unchanged(),"a missing offered item rejects the whole trade before payment");finish(trade);reset();
    trade=session();a=offer(trade,90000000);trade->m_tradelist.push_back(trade->m_tradelist.front());
    exchange(trade);Check(unchanged() && a->ownerID()==90000000,"duplicate item offers reject the whole trade");Query("DELETE FROM chrTradeItems");a->Delete();finish(trade);reset();
    trade=session();a=offer(trade,90000000);a->Move(60014689,flagHangar,false);
    exchange(trade);Check(unchanged() && a->ownerID()==90000000,"a stale item moved out of its trade container rejects the whole trade");Query("DELETE FROM chrTradeItems");a->Delete();finish(trade);reset();
    // Exercise the same binding and bound dispatcher used by the Windows client.
    BoundID firstBound=TradeBind(services,first.get()),secondBound=TradeBind(services,second.get());
    Check(first->GetTradeSession()==second->GetTradeSession() && first->GetTradeSession()->m_tradeSession.myMoney==0 && first->GetTradeSession()->m_tradeSession.herMoney==0,
        "actual RPC binding shares one session and ignores untrusted initial money");
    Check(!TradeCash(services,first.get(),firstBound,-1) && !TradeCash(services,first.get(),firstBound,std::numeric_limits<double>::infinity()) && !TradeCash(services,first.get(),firstBound,std::numeric_limits<double>::quiet_NaN()) && !TradeCash(services,first.get(),firstBound,5000.01),
        "RPC money offers reject negative, nonfinite and unfunded amounts");
    ItemData rpcData(34,90000000,60014689,flagHangar,20);a=sItemFactory.SpawnItem(rpcData);
    rpcData.ownerID=90000001;b=sItemFactory.SpawnItem(rpcData);
    auto multi=new PyTuple(2);auto ids=new PyList();ids->AddItem(new PyInt(a->itemID()));ids->AddItem(new PyInt(b->itemID()));multi->SetItem(0,ids);multi->SetItem(1,new PyInt(60014689));
    Check(!TradeCall(services,first.get(),firstBound,"MultiAdd",multi) && first->GetTradeSession()->m_tradelist.empty() && a->locationID()==60014689 && b->locationID()==60014689 && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==0,
        "MultiAdd ownership rejection rolls back earlier reservations and item moves");
    Query("CREATE TRIGGER core_audit_trade_failure BEFORE INSERT ON chrTradeItems FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected partial reservation'");
    const double beforeCount=TradeScalar("SELECT COUNT(*) FROM entity");
    Check(!TradeAdd(services,first.get(),firstBound,a->itemID(),7) && a->quantity()==20 && TradeScalar("SELECT COUNT(*) FROM entity")==beforeCount && first->GetTradeSession()->m_tradelist.empty(),
        "failed partial offer restores its source stack and removes the provisional child");
    Query("DROP TRIGGER core_audit_trade_failure");
    Check(TradeAdd(services,first.get(),firstBound,a->itemID(),7) && a->quantity()==13 && first->GetTradeSession()->m_tradelist.size()==1 && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==1,
        "partial RPC offer reserves exactly the split quantity durably");
    const uint32 child=first->GetTradeSession()->m_tradelist.front().itemID;
    Check(TradeCall(services,first.get(),firstBound,"List",new PyTuple(0)) && TradeCall(services,second.get(),secondBound,"GetItem",new PyTuple(0)),
        "both client trade list and container row serialize successfully");
    Check(TradeCash(services,first.get(),firstBound,100.25) && TradeCash(services,second.get(),secondBound,50.25),"both RPC money offers succeed without notification reference errors");
    Check(TradeAccept(services,first.get(),firstBound,true) && first->GetTradeSession() && !first->GetTradeSession()->m_tradeSession.herState && unchanged(),
        "forceTrade cannot accept the other player's offer");
    Check(TradeAccept(services,second.get(),secondBound) && !first->GetTradeSession() && !second->GetTradeSession() && first->GetBalance()==4950 && second->GetBalance()==5050 && journals()==4 && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==0,
        "second RPC acceptance commits both exact wallet payments and releases the reservation");
    auto delivered=sItemFactory.GetItemRef(child);ItemData deliveredRow;ItemDB::GetItemData(child,deliveredRow);
    Check(delivered && delivered->quantity()==7 && delivered->ownerID()==90000001 && deliveredRow.quantity==7 && deliveredRow.ownerID==90000001 && deliveredRow.locationID==60014689 && deliveredRow.flag==flagHangar,
        "successful partial trade delivers exactly the offered item to the recipient hangar");
    Check(!TradeCall(services,first.get(),firstBound,"GetItemID",new PyTuple(0)),"completed trade bounds cannot operate a closed session");
    delivered->Delete();a->Delete();b->Delete();reset();
    // The client withdraws items using its ordinary inventory bound, not trademgr.
    auto firstInventory=new InventoryBound(services,*services.Lookup<InvBrokerService>("invbroker"),sItemFactory.GetStationRef(60014689),flagHangar,90000000,false);firstInventory->NewReference(first.get());
    auto secondInventory=new InventoryBound(services,*services.Lookup<InvBrokerService>("invbroker"),sItemFactory.GetStationRef(60014689),flagHangar,90000001,false);secondInventory->NewReference(second.get());
    const BoundID firstInventoryID=firstInventory->GetBoundID(),secondInventoryID=secondInventory->GetBoundID();
    firstBound=TradeBind(services,first.get());secondBound=TradeBind(services,second.get());
    rpcData.ownerID=90000000;a=sItemFactory.SpawnItem(rpcData);TradeAdd(services,first.get(),firstBound,a->itemID());
    const uint32 tradeContainer=first->GetTradeSession()->m_tradeSession.containerID;
    Check(!TradeAdd(services,second.get(),secondInventoryID,a->itemID(),0,tradeContainer) && a->ownerID()==90000000 && a->locationID()==tradeContainer,
        "the peer cannot steal a reserved offer through an ordinary inventory RPC");
    TradeCash(services,first.get(),firstBound,10);TradeAccept(services,first.get(),firstBound);
    Query("CREATE TRIGGER core_audit_trade_failure BEFORE DELETE ON chrTradeItems FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected offer withdrawal'");
    Check(!TradeAdd(services,first.get(),firstInventoryID,a->itemID(),0,tradeContainer) && a->locationID()==tradeContainer && first->GetTradeSession()->m_tradelist.size()==1 && first->GetTradeSession()->m_tradeSession.myState && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==1,
        "failed inventory withdrawal restores the reservation, item and acceptance state");
    Query("DROP TRIGGER core_audit_trade_failure");
    {
        DBTransaction compound;
        if (!compound.Started() || !TradeAdd(services,first.get(),firstInventoryID,a->itemID(),0,tradeContainer)) throw std::runtime_error("Cannot stage compound withdrawal");
    }
    Check(a->locationID()==tradeContainer && first->GetTradeSession()->m_tradelist.size()==1 && first->GetTradeSession()->m_tradeSession.myState && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==1,
        "outer rollback preserves the offer list and consent after a staged inventory withdrawal");
    Check(TradeAdd(services,first.get(),firstInventoryID,a->itemID(),0,tradeContainer) && a->locationID()==60014689 && first->GetTradeSession()->m_tradelist.empty() && !first->GetTradeSession()->m_tradeSession.myState && !first->GetTradeSession()->m_tradeSession.herState && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==0,
        "successful inventory withdrawal returns the entire item and resets both acceptance states");
    TradeCall(services,second.get(),secondBound,"Abort",new PyTuple(0));a->Delete();reset();
    firstBound=TradeBind(services,first.get());secondBound=TradeBind(services,second.get());a=sItemFactory.SpawnItem(rpcData);TradeAdd(services,first.get(),firstBound,a->itemID());
    TradeCash(services,first.get(),firstBound,100.25);TradeCash(services,second.get(),secondBound,50.25);
    {
        DBTransaction compound;if (!compound.Started() || !TradeAccept(services,first.get(),firstBound) || !TradeAccept(services,second.get(),secondBound)) throw std::runtime_error("Cannot stage compound trade");
        Check(first->GetTradeSession() && second->GetTradeSession() && first->GetBalance()==5000 && second->GetBalance()==5000,"nested trade completion waits for the outer commit before closing the session or publishing wallets");
    }
    Check(unchanged() && a->ownerID()==90000000 && first->GetTradeSession() && first->GetTradeSession()==second->GetTradeSession() && !first->GetTradeSession()->m_tradeSession.myState && !first->GetTradeSession()->m_tradeSession.herState && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==1,
        "outer rollback restores both trade acceptances, items, money and the live session");
    TradeCall(services,second.get(),secondBound,"Abort",new PyTuple(0));a->Delete();reset();
    // Offer a ship whose nested contents were never loaded into its cached inventory.
    ItemData shipData(606,90000000,60014689,flagHangar,"core-audit traded ship");auto ship=sItemFactory.SpawnShip(shipData);
    ItemData nestedData(23,90000000,ship->itemID(),flagCargoHold,"core-audit nested container");nestedData.position=GPoint(0,0,0);const uint32 nested=ItemDB::NewItem(nestedData);
    ItemData oreData(34,90000000,nested,flagCargoHold,17);oreData.position=GPoint(0,0,0);const uint32 ore=ItemDB::NewItem(oreData);
    if (!nested || !ore) throw std::runtime_error("Cannot create unloaded nested trade contents");
    sDatabase.RunQuery(error,"INSERT INTO shipInsurance(shipID,shipName,ownerID,startDate,endDate,fraction,payOutAmount) VALUES (%u,'audit insurance',90000000,0,1,1,100)",ship->itemID());
    firstBound=TradeBind(services,first.get());secondBound=TradeBind(services,second.get());
    Check(TradeAdd(services,first.get(),firstBound,ship->itemID()) && TradeCash(services,first.get(),firstBound,100.25) && TradeCash(services,second.get(),secondBound,50.25),"an inactive ship with unloaded nested cargo can be offered");
    ItemData initialNested,initialOre;
    Check(ItemDB::GetItemData(nested,initialNested) && ItemDB::GetItemData(ore,initialOre) && initialNested.locationID==ship->itemID() && initialOre.locationID==nested,
        "unloaded nested trade fixture persists the expected parent-child tree");
    Check(!TradeAdd(services,first.get(),firstInventoryID,ore,0,nested) && !TradeAdd(services,second.get(),secondInventoryID,ore,0,nested),"neither trader can move nested contents out of a reserved ship through inventory RPCs");
    if (!sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_trade_failure BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u AND NEW.ownerID<>OLD.ownerID THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected nested cargo owner'; END IF; END",ore)) throw std::runtime_error("Cannot install nested trade failure trigger");
    TradeAccept(services,first.get(),firstBound);const bool nestedRejected=!TradeAccept(services,second.get(),secondBound);
    ItemData nestedRow,oreRow;ItemDB::GetItemData(nested,nestedRow);ItemDB::GetItemData(ore,oreRow);
    Check(nestedRejected && unchanged() && ship->ownerID()==90000000 && nestedRow.ownerID==90000000 && oreRow.ownerID==90000000 && TradeScalar("SELECT COUNT(*) FROM shipInsurance WHERE shipName='audit insurance'")==1 && !first->GetTradeSession()->m_tradeSession.myState && !first->GetTradeSession()->m_tradeSession.herState,
        "nested cargo failure restores ownership, insurance, money and both acceptance states");
    Query("DROP TRIGGER core_audit_trade_failure");
    Query("CREATE TRIGGER core_audit_trade_failure BEFORE DELETE ON shipInsurance FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected insurance deletion'");
    TradeAccept(services,first.get(),firstBound);const bool insuranceRejected=!TradeAccept(services,second.get(),secondBound);
    Check(insuranceRejected && unchanged() && ship->ownerID()==90000000 && TradeScalar("SELECT COUNT(*) FROM shipInsurance WHERE shipName='audit insurance'")==1,"insurance deletion failure rejects the whole ship trade");
    Query("DROP TRIGGER core_audit_trade_failure");
    Query("CREATE TRIGGER core_audit_trade_failure BEFORE DELETE ON chrTradeItems FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected final trade reservation release'");
    TradeAccept(services,first.get(),firstBound);const bool finalRejected=!TradeAccept(services,second.get(),secondBound);
    const bool treeRestored=ItemDB::GetItemData(nested,nestedRow) && ItemDB::GetItemData(ore,oreRow) && nestedRow.ownerID==90000000 && oreRow.ownerID==90000000;
    const bool lateRestored=finalRejected && unchanged() && treeRestored && ship->ownerID()==90000000 && TradeScalar("SELECT COUNT(*) FROM shipInsurance WHERE shipName='audit insurance'")==1;
    Check(lateRestored,"failure at final trade reservation release restores the full nested tree without deleting its container during cache rollback");
    Query("DROP TRIGGER core_audit_trade_failure");
    if (!lateRestored) throw std::runtime_error("Late trade rollback corrupted nested inventory; recover the saved fixture before another run");
    TradeAccept(services,first.get(),firstBound);Check(TradeAccept(services,second.get(),secondBound),"ship trade can be retried after a failed database write");
    ItemDB::GetItemData(nested,nestedRow);ItemDB::GetItemData(ore,oreRow);
    Check(ship->ownerID()==90000001 && nestedRow.ownerID==90000001 && oreRow.ownerID==90000001 && sItemFactory.GetItemRef(nested)->ownerID()==90000001 && sItemFactory.GetItemRef(ore)->ownerID()==90000001 && TradeScalar("SELECT COUNT(*) FROM shipInsurance WHERE shipName='audit insurance'")==0,
        "successful ship trade transfers unloaded recursive contents in database and cache and removes insurance");
    auto cleanupOre=sItemFactory.GetItemRef(ore);if (cleanupOre) cleanupOre->Delete();
    auto cleanupNested=sItemFactory.GetItemRef(nested);if (cleanupNested) cleanupNested->Delete();ship->Delete();reset();
    firstBound=TradeBind(services,first.get());secondBound=TradeBind(services,second.get());
    rpcData.ownerID=90000000;rpcData.locationID=first->GetShipID();rpcData.flag=flagCargoHold;a=sItemFactory.SpawnItem(rpcData);
    Check(TradeAdd(services,first.get(),firstBound,a->itemID(),0,first->GetShipID()),"docked cargo can be reserved with its original return address");
    second.reset();
    Check(!first->GetTradeSession() && service->m_activeSessions.empty() && a->locationID()==first->GetShipID() && a->flag()==flagCargoHold && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==0,
        "second trader logout safely clears both session pointers and returns cargo");
    a->Delete();second=TradeClient(services,90000001,2);
    firstBound=TradeBind(services,first.get());secondBound=TradeBind(services,second.get());
    rpcData.ownerID=90000001;rpcData.locationID=60014689;rpcData.flag=flagHangar;a=sItemFactory.SpawnItem(rpcData);
    TradeAdd(services,second.get(),secondBound,a->itemID());first.reset();
    Check(!second->GetTradeSession() && service->m_activeSessions.empty() && a->locationID()==60014689 && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==0,
        "first trader logout safely clears both session pointers and returns the peer's item");
    a->Delete();second.reset();TradeFixtureRestore();
}

void TradeCrashAudit(const char* mode,EVEServiceManager& services) {
    const bool recovery=std::strstr(mode,"recover")!=nullptr;
    const bool committed=std::strstr(mode,"committed")!=nullptr;
    const bool offered=std::strstr(mode,"offer")!=nullptr;
    const bool cancel=std::strstr(mode,"cancel")!=nullptr;
    if (!recovery) {
        TradeFixturePrepare();
        auto first=TradeClient(services,90000000,1);auto second=TradeClient(services,90000001,2);
        const BoundID firstBound=TradeBind(services,first.get()),secondBound=TradeBind(services,second.get());
        ItemData data(34,90000000,60014689,flagHangar,20);auto source=sItemFactory.SpawnItem(data);
        data.ownerID=90000001;auto peer=sItemFactory.SpawnItem(data);
        if (!source || !peer || !TradeAdd(services,first.get(),firstBound,source->itemID(),7) || !TradeAdd(services,second.get(),secondBound,peer->itemID()) ||
            !TradeCash(services,first.get(),firstBound,100.25) || !TradeCash(services,second.get(),secondBound,50.25)) throw std::runtime_error("Cannot prepare crash trade via RPC");
        const uint32 child=first->GetTradeSession()->m_tradelist.front().itemID;
        Query("ALTER TABLE coreAuditTradeState ADD sourceItem INT UNSIGNED,ADD offeredItem INT UNSIGNED,ADD peerItem INT UNSIGNED");
        DBerror error;
        if (!sDatabase.RunQuery(error,"UPDATE coreAuditTradeState SET sourceItem=%u,offeredItem=%u,peerItem=%u",source->itemID(),child,peer->itemID())) throw std::runtime_error("Cannot save crash trade identities");
        if (!offered) {
            if (!committed) {
                if (cancel) Query("CREATE TRIGGER core_audit_trade_pause BEFORE UPDATE ON entity FOR EACH ROW DO SLEEP(45)");
                else sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_trade_pause BEFORE INSERT ON jnlCharacters FOR EACH ROW BEGIN IF NEW.ownerID=90000001 AND NEW.ownerID1=90000001 AND NEW.entryTypeID=%u THEN DO SLEEP(45); END IF; END",Journal::EntryType::PlayerTrading);
            }
            if (cancel) {
                if (!TradeCall(services,second.get(),secondBound,"Abort",new PyTuple(0))) throw std::runtime_error("Crash trade cancellation failed");
            } else {
                if (!TradeAccept(services,first.get(),firstBound) || !TradeAccept(services,second.get(),secondBound)) throw std::runtime_error("Crash trade acceptance failed");
            }
        }
        std::printf("CORE_AUDIT TRADE COMMITTED: terminating without client or server cleanup\n");std::fflush(stdout);std::_Exit(86);
    }
    // The normal server startup path has already recovered persistent reservations.
    Query("DROP TRIGGER IF EXISTS core_audit_trade_pause");
    const uint32 source=static_cast<uint32>(TradeScalar("SELECT sourceItem FROM coreAuditTradeState"));
    const uint32 child=static_cast<uint32>(TradeScalar("SELECT offeredItem FROM coreAuditTradeState"));
    const uint32 peer=static_cast<uint32>(TradeScalar("SELECT peerItem FROM coreAuditTradeState"));
    const bool exchanged=committed && !cancel && !offered;
    auto first=TradeClient(services,90000000,1);auto second=TradeClient(services,90000001,2);
    Check(first->GetBalance()==(exchanged ? 4950 : 5000) && second->GetBalance()==(exchanged ? 5050 : 5000),"fresh client sessions reload both exact wallet outcomes after trade termination");
    auto sourceRef=sItemFactory.GetItemRef(source),childRef=sItemFactory.GetItemRef(child),peerRef=sItemFactory.GetItemRef(peer);
    ItemData sourceRow,childRow,peerRow;const bool loaded=ItemDB::GetItemData(source,sourceRow) && ItemDB::GetItemData(child,childRow) && ItemDB::GetItemData(peer,peerRow);
    Check(loaded && sourceRef && childRef && peerRef && sourceRow.quantity==13 && childRow.quantity==7 && peerRow.quantity==20 && sourceRef->quantity()+childRef->quantity()==20 && peerRef->quantity()==20,
        "trade crash recovery conserves source, split offer and peer item quantities");
    Check(loaded && sourceRow.ownerID==90000000 && childRow.ownerID==(exchanged ? 90000001u : 90000000u) && peerRow.ownerID==(exchanged ? 90000000u : 90000001u) && childRef->ownerID()==childRow.ownerID && peerRef->ownerID()==peerRow.ownerID,
        "fresh inventory caches match the complete committed or rolled-back ownership outcome");
    Check(loaded && sourceRow.locationID==60014689 && childRow.locationID==60014689 && peerRow.locationID==60014689 && sourceRow.flag==flagHangar && childRow.flag==flagHangar && peerRow.flag==flagHangar && TradeScalar("SELECT COUNT(*) FROM chrTradeItems")==0 && TradeScalar("SELECT COUNT(*) FROM entity WHERE locationID BETWEEN 64000000 AND 65999999")==0,
        "normal startup recovers abandoned offers to their return addresses and clears temporary trade locations");
    Check(TradeScalar("SELECT COUNT(*) FROM jnlCharacters WHERE transactionID>(SELECT lastJournal FROM coreAuditTradeState)")==(exchanged ? 4 : 0),"trade journals contain both committed payments or neither after abrupt termination");
    first.reset();second.reset();TradeFixtureRestore();
    Check(TradeScalar("SELECT COUNT(*) FROM chrCharacters WHERE characterID=90000001")==0 && TradeScalar("SELECT COUNT(*) FROM account WHERE accountID=2")==0,
        "crash fixture restores the original copied character and removes the temporary player");
}
