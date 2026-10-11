// Actual RPC placement, stopped at a SQL pause or immediately after commit.
void StandingCrashAudit(const char* mode,EVEServiceManager& services) {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Standing crash tests require the disposable persistence database");
    const bool recovery=std::strstr(mode,"recover")!=nullptr;
    const bool committed=std::strstr(mode,"committed")!=nullptr;
    const bool buy=std::strstr(mode,"buy")!=nullptr;
    const uint32 station=60014689;
    auto scalar=[](const char* sql) {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"%s",sql) || !result.GetRow(row)) throw std::runtime_error("Cannot read standing crash fixture");
        return row.GetDouble(0);
    };
    Query("DROP TRIGGER IF EXISTS core_audit_standing_pause");
    DBerror error;
    if (!recovery) {
        if (scalar("SELECT COUNT(*) FROM coreAuditFixture WHERE label LIKE 'standing-%'")) throw std::runtime_error("Unrecovered standing-order crash exists");
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        auto client=SelectMiningAuditCharacter(services);
        const uint32 lastOrder=scalar("SELECT MAX(orderID) FROM mktOrders");
        const uint32 lastJournal=scalar("SELECT COALESCE(MAX(transactionID),0) FROM jnlCharacters");
        ItemData stockData(691,ownerID,station,flagHangar,1);
        auto stock=BlueprintRef::StaticCast(sItemFactory.SpawnItem(stockData));
        stock->SetMLevel(7); stock->SetPLevel(3); stock->SetRuns(12);
        if (!sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES('standing-stock',%u),('standing-cutoff',%u),('standing-journal',%u)",stock->itemID(),lastOrder,lastJournal)) throw std::runtime_error("Cannot record standing crash identities");
        if (!committed) {
            if (buy) sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_standing_pause BEFORE INSERT ON jnlCharacters FOR EACH ROW BEGIN IF NEW.entryTypeID=%u THEN DO SLEEP(45); END IF; END",Journal::EntryType::MarketEscrow);
            else Query("CREATE TRIGGER core_audit_standing_pause BEFORE INSERT ON mktOrderItems FOR EACH ROW DO SLEEP(45)");
        }
        PyTuple* tuple=new PyTuple(11);
        tuple->SetItem(0,new PyInt(station)); tuple->SetItem(1,new PyInt(691)); tuple->SetItem(2,new PyFloat(12.5));
        tuple->SetItem(3,new PyInt(1)); tuple->SetItem(4,new PyInt(buy?1:0)); tuple->SetItem(5,new PyInt(-1));
        tuple->SetItem(6,buy ? PyStatic.NewNone() : static_cast<PyRep*>(new PyInt(stock->itemID())));
        tuple->SetItem(7,new PyInt(1)); tuple->SetItem(8,new PyInt(30)); tuple->SetItem(9,new PyBool(false)); tuple->SetItem(10,PyStatic.NewNone());
        PyDict* named=new PyDict(); PyCallArgs args(client.get(),tuple,named); PyDecRef(named);
        services.Dispatch("marketProxy","PlaceCharOrder",args);
        std::printf("CORE_AUDIT STANDING COMMITTED: abrupt termination before orderly shutdown\n"); std::fflush(stdout); std::_Exit(86);
    }
    const uint32 stockID=scalar("SELECT itemID FROM coreAuditFixture WHERE label='standing-stock'");
    const uint32 lastOrder=scalar("SELECT itemID FROM coreAuditFixture WHERE label='standing-cutoff'");
    const uint32 lastJournal=scalar("SELECT itemID FROM coreAuditFixture WHERE label='standing-journal'");
    DBQueryResult result; DBResultRow row;
    sDatabase.RunQuery(result,"SELECT orderID FROM mktOrders WHERE orderID>%u",lastOrder);
    const bool found=result.GetRow(row);
    const uint32 id=found ? row.GetUInt(0) : 0;
    Check(found==committed,"fresh process sees the standing order only when its whole placement committed");
    auto client=SelectMiningAuditCharacter(services);
    const double expectedBalance=committed ? (buy ? 4887.5 : 4900) : 5000;
    Check(client->GetBalance()==expectedBalance && scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==expectedBalance,
        "restarted player session loads exactly the matching broker-fee and escrow outcome");
    auto stock=BlueprintRef::StaticCast(sItemFactory.GetItemRef(stockID));
    Check(stock && stock->quantity()==1 && stock->mLevel()==7 && stock->pLevel()==3 && stock->runs()==12 && stock->locationID()==station &&
        stock->ownerID()==((committed && !buy) ? stDataMgr.GetOwnerID(station) : ownerID) && stock->flag()==((committed && !buy) ? flagCorpMarket : flagHangar),
        "fresh process preserves blueprint identity and metadata in either the hangar or committed sell escrow");
    Market::OrderInfo info={};
    Check(!committed || (MarketDB::GetOrderInfo(id,info) && info.quantity==1 && info.price==12.5 && info.escrow==(buy ? 12.5 : 0) && info.escrowItemID==(buy ? 0 : stockID)),
        "committed standing order reloads its exact escrow value or original stock mapping");
    sDatabase.RunQuery(result,"SELECT COUNT(*) FROM jnlCharacters WHERE transactionID>%u",lastJournal); result.GetRow(row);
    Check(row.GetUInt(0)==(committed ? (buy ? 2u : 1u) : 0u),"standing-order crash leaves all fee journals or none");
    if (committed) {
        PyTuple* tuple=new PyTuple(2); tuple->SetItem(0,new PyInt(id)); tuple->SetItem(1,new PyInt(0));
        PyDict* named=new PyDict(); PyCallArgs args(client.get(),tuple,named); PyDecRef(named);
        services.Dispatch("marketProxy","CancelCharOrder",args);
        Check(stock->ownerID()==ownerID && stock->flag()==flagHangar && client->GetBalance()==4900,
            "reloaded committed standing order can be cancelled once with its broker fee retained");
    }
    client.reset();
    stock->Delete();
    sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE transactionID>%u",lastJournal);
    Query("DELETE FROM coreAuditFixture WHERE label LIKE 'standing-%'");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
}
