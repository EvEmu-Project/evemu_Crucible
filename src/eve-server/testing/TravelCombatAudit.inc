// Ordinary player travel requests run only in the dedicated disposable database.
void TravelFixtureGuard() {
    DBQueryResult result; DBResultRow row;
    if (!sDatabase.RunQuery(result,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !result.GetRow(row) ||
        std::strcmp(row.GetText(0),"isolated-travel-combat-audit"))
        throw std::runtime_error("Travel/combat fixtures require their disposable database");
}
void TravelFixturePrepare() {
    TravelFixtureGuard();TradeFixturePrepare();
    Query("CREATE TABLE coreAuditTravelVisits AS SELECT * FROM chrVisitedSystems WHERE characterID IN (90000000,90000001)");
    Query("CREATE TABLE coreAuditTravelMap AS SELECT * FROM mapDynamicData");
    Query("CREATE TABLE coreAuditTravelKills AS SELECT * FROM chrKillTable WHERE victimCharacterID IN (90000000,90000001) OR finalCharacterID IN (90000000,90000001)");
    Query("CREATE TABLE coreAuditTravelInsurance AS SELECT * FROM shipInsurance WHERE shipID IN (SELECT itemID FROM coreAuditTradeItems)");
}
void TravelFixtureRestore() {
    TravelFixtureGuard();Query("DROP TRIGGER IF EXISTS core_audit_travel_failure");
    if (TradeScalar("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema=DATABASE() AND table_name='coreAuditTravelMap'")) {
        Query("DELETE FROM chrVisitedSystems WHERE characterID IN (90000000,90000001)");Query("INSERT INTO chrVisitedSystems SELECT * FROM coreAuditTravelVisits");
        Query("DELETE FROM mapDynamicData");Query("INSERT INTO mapDynamicData SELECT * FROM coreAuditTravelMap");
        Query("DELETE FROM chrKillTable WHERE victimCharacterID IN (90000000,90000001) OR finalCharacterID IN (90000000,90000001)");
        Query("INSERT INTO chrKillTable SELECT * FROM coreAuditTravelKills");
        Query("DELETE FROM shipInsurance WHERE shipID IN (SELECT itemID FROM coreAuditTradeItems)");Query("INSERT INTO shipInsurance SELECT * FROM coreAuditTravelInsurance");
        Query("DROP TABLE coreAuditTravelMap,coreAuditTravelVisits,coreAuditTravelKills,coreAuditTravelInsurance");
    }
    TradeFixtureRestore();
}
void TravelSettle(Client* client) {
    client->SetStateTimer(Player::State::Idle,0);
    client->SetStateTimer(client->IsLogin() ? Player::State::Login : Player::State::Idle,1);
    const auto until=std::chrono::steady_clock::now()+std::chrono::milliseconds(1200);
    while (std::chrono::steady_clock::now()<until) {
        Timer::SetCurrentTime();client->ProcessClient();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    client->SetStateTimer(Player::State::Idle,0);
    client->SetSessionChange(false);
    if (client->GetShipSE()) {
        client->SetLoginWarpComplete();client->GetShipSE()->DestinyMgr()->UnCloak();client->SetInvul(false);
    }
}
bool TravelUndock(EVEServiceManager& services,Client* client) {
    auto bound=new ShipBound(services,*services.Lookup<ShipService>("ship"),client->GetShip().get());
    bound->NewReference(client);
    auto tuple=new PyTuple(2);tuple->SetItem(0,new PyInt(client->GetShipID()));tuple->SetItem(1,new PyBool(false));
    return TradeCall(services,client,bound->GetBoundID(),"Undock",tuple);
}
bool TravelGate(EVEServiceManager& services,Client* client,BoundID bound,uint32 from,uint32 to,uint32 ship=0) {
    auto tuple=new PyTuple(3);tuple->SetItem(0,new PyInt(from));tuple->SetItem(1,new PyInt(to));tuple->SetItem(2,new PyInt(ship ? ship : client->GetShipID()));
    return TradeCall(services,client,bound,"CmdStargateJump",tuple);
}
void TravelTick(Client* client,unsigned milliseconds) {
    const auto until=std::chrono::steady_clock::now()+std::chrono::milliseconds(milliseconds);
    while (std::chrono::steady_clock::now()<until) {
        Timer::SetCurrentTime();client->ProcessClient();std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
bool TravelLocations(Client* client,uint32 location,bool docked) {
    auto ship=client->GetShip();auto pod=client->GetPod();
    DBQueryResult result;DBResultRow row;
    const bool loaded=sDatabase.RunQuery(result,"SELECT c.locationID,c.stationID,c.solarSystemID,e.locationID,p.locationID FROM chrCharacters c JOIN entity e ON e.itemID=c.shipID JOIN entity p ON p.itemID=c.capsuleID WHERE c.characterID=%u",client->GetCharacterID()) && result.GetRow(row);
    const bool correct=client->GetLocationID()==location && client->IsDocked()==docked &&
        ship->locationID()==location && pod->locationID()==location &&
        loaded && row.GetUInt(0)==location && row.GetUInt(1)==(docked ? location : 0) &&
        row.GetUInt(2)==client->GetSystemID() && row.GetUInt(3)==location && row.GetUInt(4)==location;
    if (!correct && loaded) std::printf("CORE_AUDIT LOCATION: expected=%u/%u live=%u/%u ship=%u pod=%u stored=%u/%u/%u/%u/%u system=%u\n",location,docked,client->GetLocationID(),client->IsDocked(),ship->locationID(),pod->locationID(),row.GetUInt(0),row.GetUInt(1),row.GetUInt(2),row.GetUInt(3),row.GetUInt(4),client->GetSystemID());
    return correct;
}
void TravelLootCheck() {
    DBQueryResult result;DBResultRow row;
    if (!sDatabase.RunQuery(result,"SELECT killBlob FROM chrKillTable WHERE victimCharacterID=90000000 AND victimShipTypeID=606 ORDER BY killID DESC LIMIT 1") || !result.GetRow(row)) throw std::runtime_error("Cannot read the ship-loss item record");
    const std::string blob=row.GetText(0);size_t at=0;bool correct=true;uint32 records=0;
    while ((at=blob.find("<i t=",at))!=std::string::npos) {
        unsigned type,flag,singleton,dropped,destroyed;
        if (sscanf(blob.c_str()+at,"<i t=%u f=%u s=%u d=%u x=%u/>",&type,&flag,&singleton,&dropped,&destroyed)!=5) {correct=false;break;}
        char query[512];snprintf(query,sizeof(query),"SELECT COALESCE(SUM(quantity),0) FROM coreAuditTradeItems WHERE locationID=140000013 AND typeID=%u AND flag=%u",type,flag);
        correct=correct && TradeScalar(query)==dropped+destroyed;
        snprintf(query,sizeof(query),"SELECT COALESCE(SUM(e.quantity),0) FROM entity e JOIN entity w ON w.itemID=e.locationID WHERE w.ownerID=90000000 AND w.itemName LIKE '%% Wreck' AND e.typeID=%u",type);
        correct=correct && TradeScalar(query)==dropped;
        ++records;at+=5;
    }
    Check(correct && records==4,"wreck contents match the recorded surviving quantities and conserve every original fitted module and cargo stack");
}
void TravelAudit(const char* mode,EVEServiceManager& services) {
    TravelFixtureGuard();
    if (!std::strcmp(mode,"--core-audit-travel-recover-fixture")) {
        TravelFixtureRestore();return;
    }
    TravelFixturePrepare();
    auto client=TradeClient(services,90000000,1);TravelSettle(client.get());
    const uint32 shipID=client->GetShipID();
    Check(client->IsDocked() && !client->IsLogin(),"travel fixture loads an ordinary docked player and finishes login");
    if (!std::strcmp(mode,"--core-audit-travel-undock-failure")) {
        Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=140000013 AND NEW.locationID=30004971 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected undock ship move'; END IF; END");
        const bool accepted=TravelUndock(services,client.get());
        Query("DROP TRIGGER core_audit_travel_failure");
        Check(!accepted,"a failed undock is reported as a rejected ordinary RPC");
        Check(client->IsDocked() && client->GetLocationID()==60014689 && !client->GetShipSE(),"failed undock preserves the live station session without a space entity");
        Check(client->GetShip()->locationID()==60014689 && client->GetChar()->stationID()==60014689 &&
              TradeScalar("SELECT locationID FROM entity WHERE itemID=140000013")==60014689 &&
              TradeScalar("SELECT stationID FROM chrCharacters WHERE characterID=90000000")==60014689 &&
              TradeScalar("SELECT locationID FROM entity WHERE itemID=140000012")==60014689,
              "failed undock preserves durable and cached ship, capsule and character locations");
        // Leave crash-prone baseline state for a separate guarded recovery process.
        if (!client->IsDocked() && !client->GetShipSE()) {std::fflush(stdout);std::_Exit(EXIT_FAILURE);}
    } else if (!std::strcmp(mode,"--core-audit-travel-docked-rpc")) {
        auto bound=new BeyonceBound(services,*services.Lookup<BeyonceService>("beyonce"),client.get());bound->NewReference(client.get());
        TradeCall(services,client.get(),bound->GetBoundID(),"CmdStop",new PyTuple(0));
        Check(client->IsDocked() && client->GetShipID()==shipID && !client->GetShipSE(),"a docked movement request is rejected safely without changing the ship session");
    } else if (!std::strcmp(mode,"--core-audit-travel-location-failures")) {
        for (const char* trigger:{
            "CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000000 AND NEW.stationID=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected character travel'; END IF; END",
            "CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON entity_attributes FOR EACH ROW BEGIN IF NEW.itemID=140000013 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected ship state'; END IF; END"}) {
            Query(trigger);const bool accepted=TravelUndock(services,client.get());Query("DROP TRIGGER core_audit_travel_failure");
            Check(!accepted && TravelLocations(client.get(),60014689,true) && !client->GetShipSE(),"late undock write failure restores character, ship, capsule, cache and station session together");
        }
        Check(TravelUndock(services,client.get()) && TravelLocations(client.get(),30004971,false),"the same player can retry undocking after rejected writes");
        TravelSettle(client.get());
        Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000000 AND NEW.stationID=60014689 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected docking'; END IF; END");
        client->SetDockStationID(60014689);client->SetStateTimer(Player::State::Dock,1);TravelTick(client.get(),300);
        Query("DROP TRIGGER core_audit_travel_failure");
        Check(TravelLocations(client.get(),30004971,false) && client->GetShipSE() && client->IsIdle(),"a failed timed docking keeps the ship in its original system and cancels the pending move safely");
    } else {
        Check(TravelUndock(services,client.get()) && client->GetShipSE(),"ordinary undock creates the player ship in space");
        TravelSettle(client.get());
        auto bound=new BeyonceBound(services,*services.Lookup<BeyonceService>("beyonce"),client.get());bound->NewReference(client.get());
        Check(client->IsIdle() && !client->IsSessionChange() && !client->IsLoginWarping(),"travel requests begin from a verified idle space session");
        if (!std::strcmp(mode,"--core-audit-travel-gate-validation")) {
            TravelGate(services,client.get(),bound->GetBoundID(),0,40007985);
            Check(client->IsIdle() && client->GetLocationID()==30004971,"a nonexistent origin gate cannot queue travel to an arbitrary celestial");
            TravelSettle(client.get());TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769);
            Check(client->IsIdle(),"a valid distant gate cannot teleport a player across the system");
            TravelSettle(client.get());
            auto gate=client->SystemMgr()->GetSE(50001480);GPoint position=gate->GetPosition();position.x+=gate->GetRadius()+client->GetShipSE()->GetRadius()+2499;
            client->MoveToPosition(position);
            TravelGate(services,client.get(),bound->GetBoundID(),50001480,50001403);
            Check(client->IsIdle(),"a nearby gate cannot be paired with an unrelated destination");
            TravelSettle(client.get());TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769,shipID+99);
            Check(client->IsIdle(),"a jump RPC cannot name another ship");
            TravelSettle(client.get());position.x+=2;client->MoveToPosition(position);TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769);
            Check(client->IsIdle(),"a gate request outside the activation range cannot queue a jump");
            TravelSettle(client.get());position.x-=2;client->MoveToPosition(position);TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769);
            Check(client->IsGateJump(),"the real paired gate accepts a ship inside the activation range");
            TravelSettle(client.get());
        } else if (!std::strcmp(mode,"--core-audit-travel-gate-failure")) {
            auto gate=client->SystemMgr()->GetSE(50001480);GPoint position=gate->GetPosition();position.x+=gate->GetRadius()+500;client->MoveToPosition(position);
            SystemManager* origin=client->SystemMgr();
            const uint32 jumps=TradeScalar("SELECT SUM(jumpsHour) FROM mapDynamicData");
            Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000000 AND NEW.solarSystemID=30002668 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected gate travel'; END IF; END");
            TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769);client->SetStateTimer(Player::State::Jump,0);client->SetStateTimer(Player::State::Jump,1);TravelTick(client.get(),300);
            Query("DROP TRIGGER core_audit_travel_failure");
            Check(client->IsIdle() && TravelLocations(client.get(),30004971,false) && client->SystemMgr()==origin && origin->GetSE(shipID)==client->GetShipSE(),"a rejected gate commit retains the character, capsule, ship and original system membership");
            Check(TradeScalar("SELECT SUM(jumpsHour) FROM mapDynamicData")==jumps && TradeScalar("SELECT COUNT(*) FROM chrVisitedSystems WHERE characterID=90000000 AND solarSystemID=30002668")==0,"rejected gate travel does not publish a system visit or jump counters");
        } else if (!std::strcmp(mode,"--core-audit-travel-inputs")) {
            for (double value:{-1.0,2.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()}) {
                auto tuple=new PyTuple(1);tuple->SetItem(0,new PyFloat(value));
                Check(!TradeCall(services,client.get(),bound->GetBoundID(),"CmdSetSpeedFraction",tuple),"invalid speed fractions are rejected by the ordinary movement RPC");
            }
            for (double value:{0.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),1e300}) {
                auto tuple=new PyTuple(3);tuple->SetItem(0,new PyFloat(value));tuple->SetItem(1,new PyFloat(0));tuple->SetItem(2,new PyFloat(0));
                Check(!TradeCall(services,client.get(),bound->GetBoundID(),"CmdGotoDirection",tuple),"invalid direction vectors cannot reach the movement simulation");
            }
            for (double value:{-1.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()}) {
                auto tuple=new PyTuple(2);tuple->SetItem(0,new PyInt(50001480));tuple->SetItem(1,new PyFloat(value));
                Check(!TradeCall(services,client.get(),bound->GetBoundID(),"CmdOrbit",tuple),"invalid orbit ranges are rejected safely");
            }
        } else if (!std::strcmp(mode,"--core-audit-travel-warp")) {
            Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=140000013 THEN SET @core_audit_warp_rejected=1; SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected warp arrival'; END IF; END");
            auto tuple=new PyTuple(2);tuple->SetItem(0,new PyString("item"));tuple->SetItem(1,new PyInt(50001480));
            Check(TradeCall(services,client.get(),bound->GetBoundID(),"CmdWarpToStuff",tuple),"the ordinary warp RPC accepts the real destination gate");
            bool started=false,finished=false,rejected=false;const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(100);
            while (std::chrono::steady_clock::now()<until) {
                Timer::SetCurrentTime();sEntityList.Process();client->ProcessClient();
                const bool warping=client->GetShipSE()->DestinyMgr()->IsWarping();started=started||warping;
                if (!rejected && TradeScalar("SELECT COALESCE(@core_audit_warp_rejected,0)")) {
                    rejected=true;Check(warping,"a rejected arrival write keeps warp pending instead of publishing unsaved completion");
                    Query("DROP TRIGGER core_audit_travel_failure");
                }
                if (started && !warping) {finished=true;break;}
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            Query("DROP TRIGGER IF EXISTS core_audit_travel_failure");
            Check(rejected,"the full warp simulation reaches the deliberately rejected arrival write");
            Check(started && finished,"the real movement simulation enters warp and completes it");
            ItemData stored;const GPoint position=client->GetShipSE()->GetPosition();
            Check(ItemDB::GetItemData(shipID,stored) && std::abs(stored.position.x-position.x)<1 && std::abs(stored.position.y-position.y)<1 && std::abs(stored.position.z-position.z)<1,"warp completion saves its final position before logout");
            auto gate=client->SystemMgr()->GetSE(50001480);const double range=GVector(position,gate->GetPosition()).length()-gate->GetRadius()-client->GetShipSE()->GetRadius();
            Check(std::isfinite(range) && range<=2500,"warp to zero lands inside the real gate activation range");
        } else if (!std::strcmp(mode,"--core-audit-travel-roundtrip")) {
            auto gate=client->SystemMgr()->GetSE(50001480);GPoint position=gate->GetPosition();position.x+=gate->GetRadius()+500;client->MoveToPosition(position);
            TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769);client->SetStateTimer(Player::State::Jump,0);client->SetStateTimer(Player::State::Jump,1);TravelTick(client.get(),300);
            Check(TravelLocations(client.get(),30002668,false) && client->GetShipSE()->SystemMgr()==client->SystemMgr(),"a valid gate jump persists one destination for the character, ship and capsule and switches the live system");
            TravelSettle(client.get());gate=client->SystemMgr()->GetSE(50000769);position=gate->GetPosition();position.x+=gate->GetRadius()+500;client->MoveToPosition(position);
            TravelGate(services,client.get(),bound->GetBoundID(),50000769,50001480);client->SetStateTimer(Player::State::Jump,0);client->SetStateTimer(Player::State::Jump,1);TravelTick(client.get(),300);
            Check(TravelLocations(client.get(),30004971,false),"the return gate persists the original system consistently");
            TravelSettle(client.get());client->SetDockStationID(60014689);client->SetStateTimer(Player::State::Dock,1);TravelTick(client.get(),300);
            Check(TravelLocations(client.get(),60014689,true) && !client->GetShipSE(),"docking persists the character, ship and capsule together and removes the space entity");
            client.reset();client=TradeClient(services,90000000,1);TravelSettle(client.get());
            Check(TravelLocations(client.get(),60014689,true),"ordinary relog after a gate round trip recovers the correct docked state");
        } else {
            auto ship=client->GetShip();auto weapon=sItemFactory.GetItemRef(140000015);
            ship->SetAttribute(AttrShieldCapacity,100,false);ship->SetAttribute(AttrShieldCharge,80,false);
            ship->SetAttribute(AttrArmorHP,100,false);ship->SetAttribute(AttrArmorDamage,20,false);
            ship->SetAttribute(AttrHP,100,false);ship->SetAttribute(AttrDamage,30,false);
            ship->SetAttribute(AttrShieldKineticDamageResonance,1.0,false);
            ship->SetAttribute(AttrArmorKineticDamageResonance,1.0,false);
            ship->SetAttribute(AttrKineticDamageResonance,1.0,false);ship->SaveShip();
            if (!std::strcmp(mode,"--core-audit-combat-invalid-damage")) {
                for (float value:{-10.0f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}) {
                    Damage damage(client->GetShipSE(),weapon,value,0,0,0,1,EVEEffectID::targetAttack);
                    client->GetShipSE()->ApplyDamage(damage);
                    Check(client->GetShipID()==shipID && !ship->IsPopped() && ship->GetAttribute(AttrShieldCharge)==80 && ship->GetAttribute(AttrArmorDamage)==20 && ship->GetAttribute(AttrDamage)==30,"invalid damage cannot heal, corrupt or destroy a player ship");
                    if (client->GetShipID()!=shipID) {std::fflush(stdout);std::_Exit(EXIT_FAILURE);}
                }
            } else if (!std::strcmp(mode,"--core-audit-combat-damage-persistence")) {
                Damage damage(client->GetShipSE(),weapon,10,0,0,0,1,EVEEffectID::targetAttack);client->GetShipSE()->ApplyDamage(damage);
                ship->SetAttribute(AttrCapacitorCharge,40,false);ship->SaveShip();
                Check(ship->GetAttribute(AttrShieldCharge)==70 && TradeScalar("SELECT COALESCE(valueFloat,valueInt) FROM entity_attributes WHERE itemID=140000013 AND attributeID=264")==70,"a nonlethal hit is persisted before it is published to players");
                Query("CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON entity_attributes FOR EACH ROW BEGIN IF NEW.itemID=140000013 AND NEW.attributeID=264 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected combat state'; END IF; END");
                Damage rejected(client->GetShipSE(),weapon,10,0,0,0,1,EVEEffectID::targetAttack);client->GetShipSE()->ApplyDamage(rejected);Query("DROP TRIGGER core_audit_travel_failure");
                Check(ship->GetAttribute(AttrShieldCharge)==70 && TradeScalar("SELECT COALESCE(valueFloat,valueInt) FROM entity_attributes WHERE itemID=140000013 AND attributeID=264")==70,"a rejected combat state write restores live and durable health together");
                client.reset();client=SelectMiningAuditCharacter(services);TravelSettle(client.get());
                std::printf("CORE_AUDIT HEALTH_RELOAD: shield=%.17g capacity=%.17g armor=%.17g hull=%.17g stored=%.17g\n",client->GetShip()->GetAttribute(AttrShieldCharge).get_double(),client->GetShip()->GetAttribute(AttrShieldCapacity).get_double(),client->GetShip()->GetAttribute(AttrArmorDamage).get_double(),client->GetShip()->GetAttribute(AttrDamage).get_double(),TradeScalar("SELECT COALESCE(valueFloat,valueInt) FROM entity_attributes WHERE itemID=140000013 AND attributeID=264"));
                Check(client->GetShip()->GetAttribute(AttrShieldCharge)==70 && client->GetShip()->GetAttribute(AttrCapacitorCharge)==40 && client->GetShip()->GetAttribute(AttrArmorDamage)==20 && client->GetShip()->GetAttribute(AttrDamage)==30,"ordinary relog reloads the last committed combat health and capacitor without free repairs");
            } else if (!std::strcmp(mode,"--core-audit-combat-pod-loss")) {
                Damage loss(client->GetShipSE(),weapon,100000,0,0,0,1,EVEEffectID::targetAttack);client->GetShipSE()->ApplyDamage(loss);
                TravelLootCheck();
                Check(client->InPod(),"ordinary lethal damage first transfers the pilot to the original capsule");
                auto pod=client->GetPod();const uint32 podID=pod->itemID();
                for (const char* trigger:{
                    "CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON entity FOR EACH ROW BEGIN IF NEW.typeID=10041 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected corpse'; END IF; END",
                    "CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON entity FOR EACH ROW BEGIN IF NEW.typeID=670 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected replacement pod'; END IF; END"}) {
                    Query(trigger);Damage fatal(client->GetShipSE(),weapon,100000,0,0,0,1,EVEEffectID::targetAttack);client->GetShipSE()->ApplyDamage(fatal);Query("DROP TRIGGER core_audit_travel_failure");
                    Check(client->GetShipID()==podID && !pod->IsPopped() && TravelLocations(client.get(),30004971,false) && TradeScalar("SELECT COUNT(*) FROM chrKillTable WHERE victimCharacterID=90000000")==1,"failed corpse or replacement capsule allocation rolls back the entire pod loss");
                }
                Damage fatal(client->GetShipSE(),weapon,100000,0,0,0,1,EVEEffectID::targetAttack);client->GetShipSE()->ApplyDamage(fatal);
                Check(client->IsDocked() && client->InPod() && client->GetShipID()!=podID && TravelLocations(client.get(),client->GetCloneStationID(),true),"successful pod loss respawns the pilot and replacement capsule together at the clone station");
                Check(TradeScalar("SELECT COUNT(*) FROM entity WHERE itemID=140000012")==0 && TradeScalar("SELECT COUNT(*) FROM chrKillTable WHERE victimCharacterID=90000000")==2 && TradeScalar("SELECT COUNT(*) FROM entity WHERE ownerID=90000000 AND typeID=10041")==1,"pod loss records one corpse and one kill while deleting the original capsule");
                client.reset();client=TradeClient(services,90000000,1);TravelSettle(client.get());
                Check(TravelLocations(client.get(),client->GetCloneStationID(),true),"ordinary relog recovers the complete clone-station respawn");
            } else if (!std::strcmp(mode,"--core-audit-combat-weaponless-kill")) {
                Damage fatal(client->GetShipSE(),true);client->GetShipSE()->Killed(fatal);
                Check(client->InPod() && client->GetShipID()!=shipID,"a weaponless fatal event transfers the player safely to their capsule");
            } else {
                for (const char* trigger:{
                    "CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON entity FOR EACH ROW BEGIN IF NEW.typeID=26557 OR NEW.itemName LIKE '% Wreck' THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected wreck'; END IF; END",
                    "CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON chrKillTable FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected kill record'",
                    "CREATE TRIGGER core_audit_travel_failure BEFORE DELETE ON entity FOR EACH ROW BEGIN IF OLD.itemID=140000013 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected final ship delete'; END IF; END"}) {
                Query(trigger);
                Damage fatal(client->GetShipSE(),weapon,10000,0,0,0,1,EVEEffectID::targetAttack);
                try {client->GetShipSE()->ApplyDamage(fatal);}catch (const PyException&) {}
                Query("DROP TRIGGER core_audit_travel_failure");
                Check(client->GetShipID()==shipID && !ship->IsPopped() && TradeScalar("SELECT COUNT(*) FROM entity WHERE itemID=140000013")==1 && TradeScalar("SELECT COUNT(*) FROM chrKillTable WHERE victimCharacterID=90000000")==0,"a failed wreck allocation preserves the ship, pilot and kill record as one consistent state");
                Check(TradeScalar("SELECT SUM(quantity) FROM entity WHERE locationID=140000013 AND flag=5")==TradeScalar("SELECT SUM(quantity) FROM coreAuditTradeItems WHERE locationID=140000013 AND flag=5") && ship->GetModule(flagHiSlot0) && ship->GetModule(flagHiSlot1),"failed ship loss restores cargo and both live fitted modules after early or late SQL failures");
                }
            }
        }
    }
    client.reset();TravelFixtureRestore();
}

void TravelCrashAudit(const char* mode,EVEServiceManager& services) {
    TravelFixtureGuard();
    const bool committed=std::strstr(mode,"-committed")!=nullptr;
    const bool loss=std::strstr(mode,"-loss")!=nullptr;
    const bool podLoss=std::strstr(mode,"-pod")!=nullptr;
    const bool gateJump=std::strstr(mode,"-gate")!=nullptr;
    const bool warp=std::strstr(mode,"-warp")!=nullptr;
    if (std::strstr(mode,"--core-audit-crash-")==mode) {
        TravelFixturePrepare();auto client=TradeClient(services,90000000,1);TravelSettle(client.get());
        if (loss || podLoss) {
            if (!TravelUndock(services,client.get())) throw std::runtime_error("Cannot undock the ship-loss crash fixture");
            TravelSettle(client.get());
            if (!committed && !podLoss) Query("CREATE TRIGGER core_audit_travel_failure BEFORE INSERT ON chrKillTable FOR EACH ROW DO SLEEP(45)");
            Damage fatal(client->GetShipSE(),sItemFactory.GetItemRef(140000015),100000,0,0,0,1,EVEEffectID::targetAttack);
            if (!client->GetShipSE()->ApplyDamage(fatal)) throw std::runtime_error("The ship-loss crash fixture did not commit");
            if (podLoss) {
                if (!committed) Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000000 AND NEW.capsuleID<>OLD.capsuleID THEN DO SLEEP(45); END IF; END");
                Damage podFatal(client->GetShipSE(),fatal.weaponRef,100000,0,0,0,1,EVEEffectID::targetAttack);
                if (!client->GetShipSE()->ApplyDamage(podFatal)) throw std::runtime_error("The capsule-loss crash fixture did not commit");
            }
        } else if (warp) {
            if (!TravelUndock(services,client.get())) throw std::runtime_error("Cannot undock the warp crash fixture");TravelSettle(client.get());
            if (!committed) Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=140000013 THEN DO SLEEP(45); END IF; END");
            auto bound=new BeyonceBound(services,*services.Lookup<BeyonceService>("beyonce"),client.get());bound->NewReference(client.get());
            auto tuple=new PyTuple(2);tuple->SetItem(0,new PyString("item"));tuple->SetItem(1,new PyInt(50001480));TravelSettle(client.get());
            if (!TradeCall(services,client.get(),bound->GetBoundID(),"CmdWarpToStuff",tuple)) throw std::runtime_error("Cannot request the crash-fixture warp");
            bool started=false,finished=false;const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(100);
            while (std::chrono::steady_clock::now()<until) {
                Timer::SetCurrentTime();sEntityList.Process();client->ProcessClient();const bool moving=client->GetShipSE()->DestinyMgr()->IsWarping();started=started||moving;
                if (started && !moving) {finished=true;break;}std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            if (!finished) throw std::runtime_error("The crash-fixture warp did not complete");
        } else if (gateJump) {
            if (!TravelUndock(services,client.get())) throw std::runtime_error("Cannot undock the gate crash fixture");TravelSettle(client.get());
            auto gate=client->SystemMgr()->GetSE(50001480);GPoint position=gate->GetPosition();position.x+=gate->GetRadius()+500;client->MoveToPosition(position);
            auto bound=new BeyonceBound(services,*services.Lookup<BeyonceService>("beyonce"),client.get());bound->NewReference(client.get());
            if (!committed) Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000000 AND NEW.solarSystemID=30002668 THEN DO SLEEP(45); END IF; END");
            TravelGate(services,client.get(),bound->GetBoundID(),50001480,50000769);client->SetStateTimer(Player::State::Jump,0);client->SetStateTimer(Player::State::Jump,1);TravelTick(client.get(),300);
            if (!TravelLocations(client.get(),30002668,false)) throw std::runtime_error("The gate crash fixture did not commit");
        } else {
            if (!committed) Query("CREATE TRIGGER core_audit_travel_failure BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000000 AND NEW.stationID=0 THEN DO SLEEP(45); END IF; END");
            if (!TravelUndock(services,client.get())) throw std::runtime_error("The undock crash fixture did not commit");
        }
        std::printf("CORE_AUDIT TRAVEL_COMMITTED\n");std::fflush(stdout);std::_Exit(86);
    }
    Query("DROP TRIGGER IF EXISTS core_audit_travel_failure");
    ItemData storedWarp;if (warp && !ItemDB::GetItemData(140000013,storedWarp)) throw std::runtime_error("Cannot read the recovered warp position");
    auto client=SelectMiningAuditCharacter(services);TravelSettle(client.get());
    if (warp) {
        auto gate=client->SystemMgr()->GetSE(50001480);const double range=GVector(storedWarp.position,gate->GetPosition()).length()-gate->GetRadius()-client->GetShipSE()->GetRadius();
        Check(TravelLocations(client.get(),30004971,false) && std::isfinite(range) && (committed ? range<=2500 : range>1000000),"fresh process reloads the original or committed gate arrival position after abrupt warp termination");
        Check(client->GetShipID()==140000013 && client->GetBalance()==5000,"warp recovery preserves the selected ship and player wallet");
    } else if (podLoss) {
        Check(client->InPod() && TravelLocations(client.get(),committed ? client->GetCloneStationID() : 30004971,committed) &&
            ((client->GetShipID()==140000012)==!committed),"fresh login recovers the original capsule or complete clone-station respawn after abrupt pod loss");
        Check(TradeScalar("SELECT COUNT(*) FROM entity WHERE itemID=140000012")==(!committed ? 1 : 0) &&
            TradeScalar("SELECT COUNT(*) FROM entity WHERE ownerID=90000000 AND typeID=10041")== (committed ? 1 : 0) &&
            TradeScalar("SELECT COUNT(*) FROM chrKillTable WHERE victimCharacterID=90000000")== (committed ? 2 : 1),"capsule deletion, corpse and second kill record recover as one committed outcome");
        Check(client->GetBalance()==5000,"capsule recovery preserves the player wallet");
    } else if (gateJump) {
        Check(TravelLocations(client.get(),committed ? 30002668 : 30004971,false) && client->GetShipSE()->SystemMgr()==client->SystemMgr(),"fresh login recovers one complete original or destination system after abrupt gate travel");
        Check(TradeScalar("SELECT COUNT(*) FROM chrVisitedSystems WHERE characterID=90000000 AND solarSystemID=30002668")== (committed ? 1 : 0),"only a committed gate destination becomes a visited system");
        Check(client->GetShipID()==140000013 && client->GetBalance()==5000,"gate recovery preserves the selected ship and wallet");
    } else if (loss) {
        Check(client->GetShipID()==(committed ? 140000012u : 140000013u) && client->InPod()==committed &&
            TravelLocations(client.get(),30004971,false),"fresh ordinary login recovers the complete live ship or committed replacement capsule after an abrupt exit");
        Check(TradeScalar("SELECT COUNT(*) FROM entity WHERE itemID=140000013")==(!committed ? 1 : 0) &&
            TradeScalar("SELECT COUNT(*) FROM chrKillTable WHERE victimCharacterID=90000000")== (committed ? 1 : 0) &&
            TradeScalar("SELECT COUNT(*) FROM entity WHERE ownerID=90000000 AND itemName LIKE '% Wreck'")== (committed ? 1 : 0),
            "crash recovery exposes the destroyed ship, kill record and wreck as one outcome");
        if (!committed) {
            Check(TradeScalar("SELECT SUM(quantity) FROM entity WHERE locationID=140000013 AND flag=5")==
                TradeScalar("SELECT SUM(quantity) FROM coreAuditTradeItems WHERE locationID=140000013 AND flag=5"),"interrupted ship loss preserves every original cargo unit");
        } else {
            DBQueryResult wrecks;DBResultRow row;
            if (!sDatabase.RunQuery(wrecks,"SELECT itemID FROM entity WHERE ownerID=90000000 AND itemName LIKE '% Wreck'") || !wrecks.GetRow(row))
                throw std::runtime_error("Cannot find the committed wreck after restart");
            auto wreck=client->SystemMgr()->GetSE(row.GetUInt(0));
            Check(wreck && wreck->IsWreckSE(),"the committed wreck reloads into the live system after a process restart");
            TravelLootCheck();
        }
        Check(client->GetBalance()==5000,"ship-loss rollback and recovery preserve the rookie pilot wallet");
    } else {
        Check(TravelLocations(client.get(),committed ? 30004971 : 60014689,!committed) &&
            ((client->GetShipSE()!=nullptr)==committed),"fresh ordinary login recovers one complete docked or undocked state after an abrupt exit");
        Check(client->GetShipID()==140000013 && client->GetBalance()==5000,"travel crash recovery preserves the selected ship and player wallet");
    }
    Check(TradeScalar("SELECT COUNT(*) FROM entity_attributes a LEFT JOIN entity e ON e.itemID=a.itemID WHERE e.itemID IS NULL")==0,
        "travel/combat crash recovery leaves no orphan item attributes");
    client.reset();TravelFixtureRestore();
}
