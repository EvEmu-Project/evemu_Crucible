#include "eve-server.h"
#include "testing/CoreAudit.h"
#include "EntityList.h"
#include "inventory/ItemDB.h"
#include "inventory/ItemType.h"
#include "system/SystemManager.h"
#include "system/SystemEntity.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/TargetManager.h"
#include "system/Container.h"
#include "ship/Ship.h"
#include "ship/modules/MiningLaser.h"
#include "ship/modules/ModuleItem.h"
#include "Client.h"
#include "network/EVETCPConnection.h"
#include "system/DestinyManager.h"
#include "account/AccountService.h"
#include "market/MarketDB.h"
#include "market/MarketMgr.h"
#include "station/StationDataMgr.h"
#include "standing/StandingDB.h"
#include "manufacturing/Blueprint.h"
#include "station/TradeService.h"
#include "station/Station.h"
#include "inventory/InventoryBound.h"
#include "inventory/InvBrokerService.h"
#include "dogmaim/DogmaIMService.h"
#include <chrono>
#include <limits>
#include <memory>
#include <stdexcept>
#include <thread>

namespace {
unsigned failures = 0;
const uint32 ownerID = 90000000;
const uint32 stationID = 60003760;

void Check(bool condition, const char* description) {
    std::printf("CORE_AUDIT %s: %s\n", condition ? "PASS" : "FAIL", description);
    std::fflush(stdout);
    if (!condition) ++failures;
}

void Query(const char* sql) {
    DBerror error;
    if (!sDatabase.RunQuery(error, "%s", sql))
        throw std::runtime_error(error.c_str());
}

InventoryItemRef Stack(uint32 quantity) {
    ItemData data(34, ownerID, stationID, flagHangar, quantity);
    InventoryItemRef item = sItemFactory.SpawnItem(data);
    if (!item) throw std::runtime_error("Could not create audit item");
    return item;
}

bool Stored(InventoryItemRef item, int32 quantity, uint32 location = stationID) {
    ItemData data;
    return ItemDB::GetItemData(item->itemID(), data) && data.quantity == quantity && data.locationID == location;
}

void InventoryAudit() {
    // Each case uses an independent real inventory item and checks the persisted row.
    auto item = Stack(100);
    Check(!item->AlterQuantity(-101) && item->quantity() == 100 && Stored(item, 100),
          "over-removal preserves the entire original stack");
    item->Delete();

    item = Stack(100);
    Check(!item->SetQuantity(-1) && item->quantity() == 100 && Stored(item, 100),
          "negative absolute quantity is rejected without deleting items");
    item->Delete();

    item = Stack(100);
    Check(!item->AlterQuantity(std::numeric_limits<int32>::max()) && item->quantity() == 100 && Stored(item, 100),
          "quantity overflow is rejected without deleting items");
    item->Delete();

    item = Stack(100);
    Check(!item->Split(101, false) && item->quantity() == 100 && Stored(item, 100),
          "oversized split preserves the original stack");
    item->Delete();

    item = Stack(100);
    Query("CREATE TRIGGER core_audit_fail_spawn BEFORE INSERT ON entity FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='core audit injected spawn failure'");
    auto split = item->Split(25, false);
    Check(sDatabase.GetStatus() == DBcore::Connected, "a rejected SQL statement preserves the database connection");
    Query("DROP TRIGGER core_audit_fail_spawn");
    Check(!split && item->quantity() == 100 && Stored(item, 100),
          "failed split allocation preserves the original stack");
    if (split) split->Delete();
    item->Delete();

    item = Stack(100);
    split = item->Split(100, false);
    Check(split && split->quantity() == 100 && split->locationID() == stationID && split->flag() == flagHangar && Stored(split, 100),
          "full split preserves location and inventory flag");
    if (split) split->Delete();

    item = Stack(100);
    Check(!item->Merge(item, 0, false) && item->quantity() == 100 && Stored(item, 100),
          "self-merge is rejected without deleting items");
    item->Delete();

    auto destination = Stack(std::numeric_limits<int32>::max());
    auto source = Stack(10);
    Check(!destination->Merge(source, 0, false) && Stored(destination, std::numeric_limits<int32>::max()) && Stored(source, 10),
          "overflowing merge preserves both stacks");
    destination->Delete();
    source->Delete();

    destination = Stack(50);
    source = Stack(100);
    Check(destination->Merge(source, 25, false) && Stored(destination, 75) && Stored(source, 75),
          "partial merge conserves quantity in memory and database");
    Check(destination->Merge(source, 0, false) && Stored(destination, 150),
          "whole merge conserves quantity and removes the exhausted source");
    ItemData removed;
    Check(!ItemDB::GetItemData(source->itemID(), removed), "exhausted source is absent from the database");
    destination->Delete();

    item = Stack(100);
    split = item->Split(25, false);
    Check(split && Stored(item, 75) && Stored(split, 25), "partial split conserves quantity in the database");
    if (split) split->Delete();
    item->Delete();
}

void SystemAudit(EVEServiceManager& services) {
    for (unsigned cycle = 0; cycle < 3; ++cycle) {
        SystemManager manager(30004971, services);
        auto system = &manager;
        if (!system->BootSystem()) throw std::runtime_error("Could not boot audit solar system");
        DBQueryResult result;
        if (!sDatabase.RunQuery(result, "SELECT itemID FROM mapDenormalize WHERE solarSystemID=30004971 AND groupID=9 LIMIT 1"))
            throw std::runtime_error(result.error.c_str());
        DBResultRow row;
        if (!result.GetRow(row)) throw std::runtime_error("Audit belt not found");
        auto belt = system->GetSE(row.GetUInt(0));
        if (!belt || !belt->SysBubble()) throw std::runtime_error("Audit belt bubble not found");
        system->GetBeltMgr()->CheckSpawn(belt->SysBubble()->GetID());
        std::vector<SystemEntity*> scan;
        system->DScan(3000000000000LL, belt->GetPosition(), scan);
        Check(!scan.empty(), "loaded belt and celestial entities can be scanned");
        system->UnloadSystem();
        Check(system->GetSE(row.GetUInt(0)) == nullptr, "solar system unload removes its entities");
    }
}

class AuditMiningLaser : public MiningLaser {
public:
    AuditMiningLaser(ModuleItemRef module, ShipItemRef ship, SystemEntity* target)
        : MiningLaser(module, ship) {
        m_targetSE = target;
        target->TargetMgr()->AddTargetModule(this);
    }
};

class AuditStoppedModule : public ActiveModule {
public:
    AuditStoppedModule(ModuleItemRef module, ShipItemRef ship, SystemEntity* target)
        : ActiveModule(module, ship), completedCycles(0) {
        m_targetSE = target;
        target->TargetMgr()->AddTargetModule(this);
        SetModuleState(Module::State::Deactivating);
        m_Stop = true;
    }
    void DeactivateCycle(bool abort = false) override {
        ++completedCycles;
        Clear();
    }
    unsigned completedCycles;
};

void ModuleLifecycleAudit(EVEServiceManager& services) {
    SystemManager system(30004971, services);
    if (!system.BootSystem()) throw std::runtime_error("Could not boot module audit solar system");
    ItemData targetData(34, ownerID, system.GetID(), flagNone, 100);
    auto targetItem = sItemFactory.SpawnItem(targetData);
    auto target = new ObjectSystemEntity(targetItem, services, &system);
    system.AddEntity(target, false);
    ItemData shipData(606, ownerID, stationID, flagHangar, "core-audit ship");
    auto ship = sItemFactory.SpawnShip(shipData);
    ItemData moduleData(3651, ownerID, stationID, flagHangar, "core-audit miner");
    auto module = sItemFactory.SpawnModule(moduleData);
    if (!ship || !module) throw std::runtime_error("Could not prepare mining-module fixture");
    auto miner = new AuditMiningLaser(module, ship, target);
    delete miner;
    // Target destruction must never call into a module whose owner has already deleted it.
    target->TargetMgr()->ClearModules();
    Check(true, "deleting a mining module detaches it from the target before target cleanup");
    miner = new AuditMiningLaser(module, ship, target);
    miner->SetModuleState(Module::State::Offline);
    miner->PrepareForLogout();
    Check(miner->GetModuleState() == Module::State::Offline && miner->GetTargetSE() == nullptr,
          "logout cleanup preserves an offline module's state");
    delete miner;
    auto stoppedModule = new AuditStoppedModule(module, ship, target);
    stoppedModule->PrepareForLogout();
    Check(stoppedModule->completedCycles == 1 && stoppedModule->GetTargetSE() == nullptr && !stoppedModule->IsActive(),
          "logout finishes a queued stop once before clearing target references");
    stoppedModule->PrepareForLogout();
    Check(stoppedModule->completedCycles == 1, "repeated logout cleanup does not finish the same cycle twice");
    delete stoppedModule;
    miner = new AuditMiningLaser(module, ship, target);
    target->TargetMgr()->ClearModules();
    system.UnloadSystem();
    delete miner;
    Check(true, "target cleanup detaches a retained module before destroying the target");
    targetItem->Delete();
    module->Delete();
    ship->Delete();
}

void ProcessFor(unsigned milliseconds) {
    const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
    while (std::chrono::steady_clock::now() < until) {
        Timer::SetCurrentTime();
        sEntityList.Process();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int64 CargoQuantity(uint32 shipID, uint32 typeID) {
    DBQueryResult result;
    DBResultRow row;
    if (!sDatabase.RunQuery(result, "SELECT COALESCE(SUM(quantity),0) FROM entity WHERE locationID=%u AND typeID=%u AND flag=5", shipID, typeID)
        || !result.GetRow(row))
        throw std::runtime_error("Could not read mining cargo");
    return row.GetInt64(0);
}

std::unique_ptr<Client> SelectMiningAuditCharacter(EVEServiceManager& services) {
    EVETCPConnection* connection = new EVETCPConnection();
    std::unique_ptr<Client> client(new Client(services, &connection));
    // This is a guarded server integration fixture, with no Windows login automation.
    client->GetSession()->SetInt("userid", 1);
    client->GetSession()->SetLong("role", 6917529029788565504LL);
    client->GetSession()->SetString("address", "127.0.0.1");
    client->GetSession()->SetString("languageID", "EN");
    if (!client->SelectCharacter(ownerID))
        throw std::runtime_error("Could not select the copied mining character");
    return client;
}

void MiningLogoutAudit(EVEServiceManager& services) {
    DBQueryResult environment;
    DBResultRow marker;
    if (!sDatabase.RunQuery(environment, "SELECT marker FROM coreAuditEnvironment WHERE id=1")
        || !environment.GetRow(marker) || std::strcmp(marker.GetText(0), "isolated-mining-logout-audit"))
        throw std::runtime_error("Mining logout requires its own disposable database copy");

    auto client = SelectMiningAuditCharacter(services);
    Check(client->IsLoaded() && client->IsInSpace(), "normal character selection loads a player and ship in space");
    client->SetStateTimer(Player::State::Login, 1);
    ProcessFor(1200);
    client->SetLoginWarpComplete();
    client->SetStateTimer(Player::State::Idle, 0);
    client->GetShipSE()->DestinyMgr()->UnCloak();
    client->SetInvul(false);
    auto system = client->SystemMgr();
    auto belt = system->GetSE(40314829);
    if (!belt || !belt->SysBubble()) throw std::runtime_error("Mining audit belt not found");
    client->MoveToPosition(belt->GetPosition());
    system->GetBeltMgr()->CheckSpawn(belt->SysBubble()->GetID());
    SystemEntity* asteroid = nullptr;
    for (auto entity : system->GetEntities()) {
        if (entity.second->IsAsteroidSE() && entity.second->GetTypeID() == 1230
            && entity.second->GetSelf()->GetAttribute(AttrQuantity).get_float() > 5000) {
            asteroid = entity.second;
            break;
        }
    }
    if (!asteroid) throw std::runtime_error("No sufficiently large Veldspar asteroid in the audit belt");
    GPoint miningPosition = asteroid->GetPosition();
    miningPosition.x += asteroid->GetRadius() + 500;
    client->MoveToPosition(miningPosition);
    auto ship = client->GetShip();
    const uint32 shipID = ship->itemID();
    auto module = ship->GetModuleManager()->GetModule(uint32(140000014));
    if (!module || !module->IsMiningLaser()) throw std::runtime_error("Copied civilian miner is missing");
    module->Online();
    if (!client->GetShipSE()->TargetMgr()->StartTargeting(asteroid, ship))
        throw std::runtime_error("Could not target the mining asteroid");
    ProcessFor(2500);
    const int64 before = CargoQuantity(shipID, 1230);
    ship->Activate(module->itemID(), "miningLaser", asteroid->GetID(), 1000);
    const auto cycleStarted = std::chrono::steady_clock::now();
    if (!module->IsActive()) throw std::runtime_error("Civilian mining laser did not activate");
    Check(module->IsActive(), "civilian miner activates against a real belt asteroid");
    ProcessFor(12000);
    const auto cycleElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - cycleStarted).count();
    const auto cycleDuration = module->GetAttribute(AttrDuration).get_uint32();
    Check(module->GetModuleState() == Module::State::Activated && cycleElapsed < cycleDuration,
          "logout occurs while the mining cycle is still active");
    std::printf("CORE_AUDIT mining before logout: ship=%u ore=%lld elapsed_ms=%lld cycle_ms=%u\n", shipID,
                static_cast<long long>(CargoQuantity(shipID, 1230)), static_cast<long long>(cycleElapsed), cycleDuration);
    std::fflush(stdout);
    ship = ShipItemRef(nullptr);
    client.reset(); // Execute the real Client destructor, module teardown and ship/character saves.
    const int64 after = CargoQuantity(shipID, 1230);
    Check(!sEntityList.IsOnline(ownerID) && sEntityList.GetPlayerCount() == 0,
          "normal logout removes the player from the world");
    Check(after > before && after - before < 400,
          "active-cycle logout saves a partial mining yield without duplicating a full cycle");
    ItemData savedShip;
    Check(ItemDB::GetItemData(shipID, savedShip) && savedShip.locationID == 30004971,
          "logout persists the original ship in its solar system");
    std::printf("CORE_AUDIT mining after logout: ore=%lld added=%lld; waiting for normal idle unload\n",
                static_cast<long long>(after), static_cast<long long>(after - before));
    std::fflush(stdout);
    for (unsigned seconds = 0; seconds < 360 && sEntityList.IsSystemLoaded(30004971); seconds += 10) {
        ProcessFor(10000);
        std::printf("CORE_AUDIT idle wait: %u seconds, system_loaded=%u\n", seconds + 10,
                    sEntityList.IsSystemLoaded(30004971) ? 1 : 0);
        std::fflush(stdout);
    }
    Check(!sEntityList.IsSystemLoaded(30004971), "last-player solar system unloads normally after mining logout");
    Check(CargoQuantity(shipID, 1230) == after, "saved mining cargo stays unchanged across idle system unload");
    ItemData unloadedShip;
    Check(ItemDB::GetItemData(shipID, unloadedShip) && unloadedShip.locationID == savedShip.locationID
        && unloadedShip.position.distance(savedShip.position) < 1.0,
          "saved ship location and coordinates survive idle system unload");
    client = SelectMiningAuditCharacter(services);
    ship = client->GetShip();
    auto reloadedMiner = ship->GetModuleManager()->GetModule(uint32(140000014));
    Check(ship->itemID() == shipID && CargoQuantity(shipID, 1230) == after,
          "a new player session reloads the same ship and mining cargo");
    Check(reloadedMiner && !reloadedMiner->IsActive(), "the reconnected mining module is no longer cycling");
    ship = ShipItemRef(nullptr);
    client.reset();
}

const char* persistedInfo = "core audit O'Brien 100% complete \\ cargo";

void PersistenceWrite() {
    auto item = Stack(100);
    auto split = item->Split(25, false);
    if (!split || !item->Merge(split, 0, false)) throw std::runtime_error("Persistence fixture split/merge failed");
    item->SetCustomInfo(persistedInfo);
    item->Move(60008494, flagHangar, false);
    Inv::SaveData data = Inv::SaveData();
    data.itemID = item->itemID(); data.typeID = item->typeID(); data.ownerID = item->ownerID();
    data.locationID = item->locationID(); data.flag = item->flag(); data.quantity = item->quantity();
    data.position = item->position(); data.customInfo = persistedInfo;
    DBerror error;
    if (!sDatabase.RunQuery(error, "UPDATE entity SET customInfo='stale database value' WHERE itemID=%u", item->itemID()))
        throw std::runtime_error(error.c_str());
    std::vector<Inv::SaveData> batch{data};
    ItemDB::SaveItems(batch);
    ItemData stored;
    Check(ItemDB::GetItemData(item->itemID(), stored) && stored.customInfo == persistedInfo,
          "bulk save preserves quotes, percent signs and backslashes");
    if (!sDatabase.RunQuery(error, "INSERT INTO coreAuditFixture (label,itemID) VALUES ('restart',%u)", item->itemID()))
        throw std::runtime_error(error.c_str());
    std::printf("CORE_AUDIT persistence item %u prepared\n", item->itemID());
}

void PersistenceRead() {
    DBQueryResult result;
    if (!sDatabase.RunQuery(result, "SELECT itemID FROM coreAuditFixture WHERE label='restart'"))
        throw std::runtime_error(result.error.c_str());
    DBResultRow row;
    if (!result.GetRow(row)) throw std::runtime_error("Restart fixture was not prepared");
    auto item = sItemFactory.GetItemRef(row.GetUInt(0));
    Check(item && item->quantity() == 100 && item->locationID() == 60008494 && item->flag() == flagHangar && item->customInfo() == persistedInfo,
          "item quantity, location, flag and custom info survive a new server process");
    if (item) item->Delete();
    Query("DELETE FROM coreAuditFixture WHERE label='restart'");
    std::vector<Inv::AttrData> empty;
    ItemDB::SaveAttributes(true, empty);
    ItemDB::SaveAttributes(false, empty);
    Check(true, "empty attribute-save batches are safe");
}

void PersistenceFailures(EVEServiceManager& services) {
    DBQueryResult guard;
    DBResultRow marker;
    if (!sDatabase.RunQuery(guard, "SELECT marker FROM coreAuditEnvironment WHERE id=1") ||
        !guard.GetRow(marker) || std::strcmp(marker.GetText(0), "isolated-persistence-audit"))
        throw std::runtime_error("Persistence failures require the disposable persistence database");
    auto failUpdate = [](uint32 id) {
        DBerror err;
        if (!sDatabase.RunQuery(err, "CREATE TRIGGER core_audit_fail_update BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected entity update'; END IF; END", id))
            throw std::runtime_error(err.c_str());
    };
    auto item = Stack(100);
    failUpdate(item->itemID());
    bool rejected=false;
    bool changed = item->SetQuantity(50, false);
    Check(!changed && item->quantity() == 100 && Stored(item, 100), "rejected quantity write leaves memory and stored stack unchanged");
    Query("DROP TRIGGER core_audit_fail_update");
    item->Delete();

    item=Stack(100);
    failUpdate(item->itemID());
    rejected=false;
    try { item->Donate(90000001,60008494,flagHangar,false); } catch (...) { rejected=true; }
    Check(rejected && item->ownerID()==ownerID && item->locationID()==stationID && Stored(item,100),
          "rejected ownership transfer preserves the original owner and container");
    Query("DROP TRIGGER core_audit_fail_update");
    item->Delete();

    item = Stack(100);
    failUpdate(item->itemID());
    rejected = false;
    try { item->Move(60008494, flagHangar, false); } catch (...) { rejected = true; }
    Check(rejected && item->locationID() == stationID && Stored(item, 100), "rejected cargo move leaves both inventory locations unchanged");
    Query("DROP TRIGGER core_audit_fail_update");
    item->Delete();

    auto destination = Stack(50);
    auto source = Stack(100);
    failUpdate(destination->itemID());
    changed = destination->Merge(source, 25, false);
    Check(!changed && destination->quantity() == 50 && source->quantity() == 100 && Stored(destination,50) && Stored(source,100), "failed merge destination write restores both stacks");
    Query("DROP TRIGGER core_audit_fail_update");
    destination->Delete(); source->Delete();

    source = Stack(100);
    failUpdate(source->itemID());
    auto split = source->Split(25, false);
    Check(!split && source->quantity() == 100 && Stored(source,100), "failed split debit creates no new stack and preserves source");
    Query("DROP TRIGGER core_audit_fail_update");
    if (split) split->Delete();
    source->Delete();

    item = Stack(100);
    item->SetAttribute(AttrMass, 42, false);
    item->SaveAttributes();
    DBerror attributeError;
    if (!sDatabase.RunQuery(attributeError,"INSERT INTO entity_attributes(itemID,attributeID,valueInt) VALUES (%u,%u,42) ON DUPLICATE KEY UPDATE valueInt=42",item->itemID(),AttrMass))
        throw std::runtime_error(attributeError.c_str());
    Query("CREATE TRIGGER core_audit_fail_delete BEFORE DELETE ON entity_attributes FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected attribute delete'");
    rejected = false;
    try { item->Delete(); } catch (...) { rejected = true; }
    Check(rejected && Stored(item,100), "failed attribute delete preserves the owning item");
    Query("DROP TRIGGER core_audit_fail_delete");
    item->Delete();

    Query("INSERT INTO chrCharacters (characterID,characterName,balance) VALUES (90000001,'Persistence recipient',5000) ON DUPLICATE KEY UPDATE balance=5000");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    Query("DELETE FROM jnlCharacters WHERE referenceID=424242");
    auto balance = [](uint32 id) {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"SELECT balance FROM chrCharacters WHERE characterID=%u",id) || !result.GetRow(row))
            throw std::runtime_error("Cannot read audit balance");
        return row.GetDouble(0);
    };
    auto journals = []() {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"SELECT COUNT(*) FROM jnlCharacters WHERE referenceID=424242") || !result.GetRow(row))
            throw std::runtime_error("Cannot read audit journal");
        return row.GetInt(0);
    };
    Query("CREATE TRIGGER core_audit_fail_credit BEFORE UPDATE ON chrCharacters FOR EACH ROW BEGIN IF OLD.characterID=90000001 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected wallet credit'; END IF; END");
    rejected = false;
    try { AccountService::TransferFunds(ownerID,90000001,125.25,"audit",Journal::EntryType::PlayerDonation,424242); } catch (...) { rejected = true; }
    Check(rejected && balance(ownerID)==5000 && balance(90000001)==5000 && journals()==0, "failed recipient credit rolls back sender and journals");
    Query("DROP TRIGGER core_audit_fail_credit");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID IN (90000000,90000001)");
    Query("DELETE FROM jnlCharacters WHERE referenceID=424242");
    Query("CREATE TRIGGER core_audit_fail_journal BEFORE INSERT ON jnlCharacters FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected journal'");
    rejected = false;
    try { AccountService::TransferFunds(ownerID,90000001,125.25,"audit",Journal::EntryType::PlayerDonation,424242); } catch (...) { rejected = true; }
    Check(rejected && balance(ownerID)==5000 && balance(90000001)==5000 && journals()==0, "failed journal insert rolls back the entire payment");
    Query("DROP TRIGGER core_audit_fail_journal");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID IN (90000000,90000001)");
    rejected = false;
    try { AccountService::TransferFunds(ownerID,90000001,5000.01,"audit",Journal::EntryType::PlayerDonation,424242); } catch (...) { rejected = true; }
    Check(rejected && balance(ownerID)==5000 && balance(90000001)==5000 && journals()==0, "offline sender cannot spend more than its wallet");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID IN (90000000,90000001)");
    Query("DELETE FROM jnlCharacters WHERE referenceID=424242");
    AccountService::TransferFunds(ownerID,90000001,125.25,"O'Brien 100% \\ audit",Journal::EntryType::PlayerDonation,424242);
    Check(balance(ownerID)==4874.75 && balance(90000001)==5125.25 && journals()==2, "successful offline payment commits balanced journal pair");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    Query("INSERT INTO crpWalletDivisons(corporationID,balance2) VALUES (98000001,100000000.25) ON DUPLICATE KEY UPDATE balance2=100000000.25");
    Query("CREATE TRIGGER core_audit_fail_corp_journal BEFORE INSERT ON jnlCorporations FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected corp journal'");
    rejected=false;
    try { AccountService::TransferFunds(ownerID,98000001,0.01,"audit",Journal::EntryType::PlayerDonation,424242,Account::KeyType::Cash,Account::KeyType::Cash2); } catch (...) { rejected=true; }
    DBQueryResult corpResult; DBResultRow corpRow;
    sDatabase.RunQuery(corpResult,"SELECT balance2 FROM crpWalletDivisons WHERE corporationID=98000001"); corpResult.GetRow(corpRow);
    Check(rejected && balance(ownerID)==5000 && corpRow.GetDouble(0)==100000000.25,
          "failed corporation journal rolls back the division wallet and sender");
    Query("DROP TRIGGER core_audit_fail_corp_journal");
    AccountService::TransferFunds(ownerID,98000001,0.01,"audit",Journal::EntryType::PlayerDonation,424242,Account::KeyType::Cash,Account::KeyType::Cash2);
    sDatabase.RunQuery(corpResult,"SELECT balance2 FROM crpWalletDivisons WHERE corporationID=98000001"); corpResult.GetRow(corpRow);
    Check(balance(ownerID)==4999.99 && corpRow.GetDouble(0)==100000000.26,"corporation division wallet preserves one-cent payment at large balance");
    Query("INSERT INTO crpCorporation(corporationID,corporationName,tickerName,taxRate,url) VALUES (98000001,'Persistence tax','AUDIT',0.10,'') ON DUPLICATE KEY UPDATE taxRate=0.10");
    Query("UPDATE crpWalletDivisons SET balance1=1000 WHERE corporationID=98000001");
    Query("UPDATE chrCharacters SET balance=5000,corporationID=98000001 WHERE characterID=90000001");
    Query("DELETE FROM jnlCharacters WHERE referenceID=424244");
    Query("DELETE FROM jnlCorporations WHERE referenceID=424244");
    Query("CREATE TRIGGER core_audit_fail_corp_journal BEFORE INSERT ON jnlCorporations FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected tax journal'");
    rejected=false;
    try { AccountService::TransferFunds(1,90000001,300000,"audit bounty",Journal::EntryType::BountyPrize,424244); } catch (...) { rejected=true; }
    sDatabase.RunQuery(corpResult,"SELECT balance1 FROM crpWalletDivisons WHERE corporationID=98000001"); corpResult.GetRow(corpRow);
    Check(rejected && balance(90000001)==5000 && corpRow.GetDouble(0)==1000,"failed tax journal rolls back the bounty and corporation tax together");
    Query("DROP TRIGGER core_audit_fail_corp_journal");
    AccountService::TransferFunds(1,90000001,300000,"audit bounty",Journal::EntryType::BountyPrize,424244);
    sDatabase.RunQuery(corpResult,"SELECT balance1 FROM crpWalletDivisons WHERE corporationID=98000001"); corpResult.GetRow(corpRow);
    Check(balance(90000001)==275000 && corpRow.GetDouble(0)==31000,"successful bounty commits net payout and corporation tax together");
    Query("UPDATE chrCharacters SET balance=5125.25,corporationID=0 WHERE characterID=90000001");
    Query("DELETE FROM crpCorporation WHERE corporationID=98000001");
    Query("DELETE FROM jnlCharacters WHERE referenceID=424244");
    Query("DELETE FROM jnlCorporations WHERE referenceID=424244");
    Query("DELETE FROM crpWalletDivisons WHERE corporationID=98000001");
    Query("DELETE FROM jnlCorporations WHERE referenceID=424242");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    {
        DBTransaction transaction;
        Query("UPDATE chrCharacters SET balance=4000 WHERE characterID=90000000");
        DBQueryResult connection; DBResultRow connectionRow;
        sDatabase.RunQuery(connection,"SELECT CONNECTION_ID()"); connection.GetRow(connectionRow);
        DBerror error;
        sDatabase.RunQuery(error,"KILL CONNECTION %u",connectionRow.GetUInt(0));
        Check(!sDatabase.RunQuery(error,"UPDATE chrCharacters SET balance=6000 WHERE characterID=90000001") && !transaction.Commit(),
              "lost transaction connection rejects further writes without replay");
    }
    Check(balance(ownerID)==5000 && balance(90000001)==5125.25,"connection loss rolls back debit before a fresh database connection opens");
    Query("UPDATE chrCharacters SET balance=100000000.25 WHERE characterID=90000000");
    auto client = SelectMiningAuditCharacter(services);
    Check(client->GetBalance()==100000000.25, "online wallet load preserves cents at large balances");
    Query("CREATE TRIGGER core_audit_fail_journal BEFORE INSERT ON jnlCharacters FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected online journal'");
    rejected = false;
    try { AccountService::TransferFunds(ownerID,90000001,0.01,"audit",Journal::EntryType::PlayerDonation,424242); } catch (...) { rejected = true; }
    Check(rejected && client->GetBalance()==100000000.25 && balance(ownerID)==100000000.25,
          "rejected online payment leaves the live wallet cache unchanged");
    Query("DROP TRIGGER core_audit_fail_journal");
    AccountService::TransferFunds(ownerID,90000001,0.01,"audit",Journal::EntryType::PlayerDonation,424242);
    Check(client->GetBalance()==100000000.24 && balance(ownerID)==100000000.24,
          "one-cent payment updates live wallet only after commit");
    auto miner = client->GetShip()->GetModuleManager()->GetModule(uint32(140000014));
    failUpdate(140000014);
    rejected = false;
    auto moduleItem = sItemFactory.GetItemRef(140000014);
    try { moduleItem->Move(client->GetShip()->itemID(),flagCargoHold,false); } catch (...) { rejected = true; }
    Check(rejected && moduleItem->flag()==flagHiSlot0 && client->GetShip()->GetModuleManager()->GetModule(uint32(140000014))==miner,
          "failed unfit keeps the fitted item and module manager consistent");
    Query("DROP TRIGGER core_audit_fail_update");
    // The successful unfit creates an empty slot for an ordinary fit operation.
    moduleItem->Move(client->GetShip()->itemID(),flagCargoHold,false);
    auto fitted = ModuleItemRef::StaticCast(moduleItem);
    failUpdate(140000014);
    rejected = false;
    try { client->GetShip()->AddItemByFlag(flagHiSlot0,moduleItem,client.get()); } catch (...) { rejected = true; }
    Check(rejected && moduleItem->flag()==flagCargoHold && !client->GetShip()->GetModuleManager()->GetModule(flagHiSlot0),
          "failed fit creates no phantom module manager entry");
    Query("DROP TRIGGER core_audit_fail_update");
    Check(client->GetShip()->AddItemByFlag(flagHiSlot0,moduleItem,client.get())==140000014 && moduleItem->flag()==flagHiSlot0,
          "successful refit persists the module in its original slot");
    client.reset();
    client=SelectMiningAuditCharacter(services);
    Check(client->GetBalance()==100000000.24 && client->GetShip()->GetModuleManager()->GetModule(uint32(140000014)),
          "wallet cents and refitted module survive a new player session");
    client.reset();
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    Query("DELETE FROM chrCharacters WHERE characterID=90000001");
    Query("DELETE FROM jnlCharacters WHERE referenceID=424242");
    item=Stack(10);
    item->Delete();
    ItemData missing;
    missing.typeID=34; missing.ownerID=ownerID; missing.locationID=stationID; missing.quantity=10;
    Check(!ItemDB::SaveItem(item->itemID(),missing),"missing item row cannot report a successful save");
}

void PersistenceCrash(const char* mode) {
    DBQueryResult guard; DBResultRow row;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(row) || std::strcmp(row.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Crash tests require the disposable persistence database");
    const bool wallet = std::strstr(mode,"wallet") != nullptr;
    const bool committed = std::strstr(mode,"committed") != nullptr;
    DBerror error;
    if (wallet) {
        Query("INSERT INTO chrCharacters (characterID,characterName,balance) VALUES (90000001,'Crash recipient',5000) ON DUPLICATE KEY UPDATE balance=5000");
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        Query("DELETE FROM jnlCharacters WHERE referenceID=424243");
        if (!committed) Query("CREATE TRIGGER core_audit_pause BEFORE INSERT ON jnlCharacters FOR EACH ROW BEGIN IF NEW.ownerID=90000001 THEN DO SLEEP(45); END IF; END");
        std::printf("CORE_AUDIT CRASH_READY wallet\n"); std::fflush(stdout);
        AccountService::TransferFunds(ownerID,90000001,125.25,"crash audit",Journal::EntryType::PlayerDonation,424243);
    } else {
        auto destination=Stack(50), source=Stack(100);
        if (!sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES ('crash-destination',%u),('crash-source',%u)",destination->itemID(),source->itemID()))
            throw std::runtime_error(error.c_str());
        if (!committed && !sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_pause BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u THEN DO SLEEP(45); END IF; END",destination->itemID()))
            throw std::runtime_error(error.c_str());
        std::printf("CORE_AUDIT CRASH_READY merge\n"); std::fflush(stdout);
        if (!destination->Merge(source,25,false)) throw std::runtime_error("Crash merge failed");
    }
    std::printf("CORE_AUDIT CRASH_COMMITTED\n"); std::fflush(stdout);
    std::_Exit(86); // Deliberately bypass every logout/shutdown save after commit.
}

void PersistenceRecovery(const char* mode) {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Recovery tests require the disposable persistence database");
    const bool wallet=std::strstr(mode,"wallet") != nullptr;
    const bool committed=std::strstr(mode,"committed") != nullptr;
    Query("DROP TRIGGER IF EXISTS core_audit_pause");
    DBQueryResult result; DBResultRow row;
    if (wallet) {
        if (!sDatabase.RunQuery(result,"SELECT (SELECT balance FROM chrCharacters WHERE characterID=90000000),(SELECT balance FROM chrCharacters WHERE characterID=90000001),(SELECT COUNT(*) FROM jnlCharacters WHERE referenceID=424243)") || !result.GetRow(row))
            throw std::runtime_error("Cannot read recovered wallets");
        Check(row.GetDouble(0)==(committed?4874.75:5000) && row.GetDouble(1)==(committed?5125.25:5000) && row.GetInt(2)==(committed?2:0),
              committed ? "committed wallet transfer and journals survive immediate process exit" : "killed wallet transfer rolls back both balances and journals");
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        Query("DELETE FROM chrCharacters WHERE characterID=90000001");
        Query("DELETE FROM jnlCharacters WHERE referenceID=424243");
    } else {
        if (!sDatabase.RunQuery(result,"SELECT label,itemID FROM coreAuditFixture WHERE label IN ('crash-destination','crash-source') ORDER BY label"))
            throw std::runtime_error("Cannot read crash fixtures");
        unsigned count=0;
        while(result.GetRow(row)) {
            auto item=sItemFactory.GetItemRef(row.GetUInt(1));
            const bool source=!std::strcmp(row.GetText(0),"crash-source");
            Check(item && item->quantity()==(source?(committed?75:100):(committed?75:50)),
                  committed ? "committed stack quantity survives immediate process exit" : "killed merge restores original stack quantity");
            if (item) item->Delete();
            ++count;
        }
        Check(count==2,"both crash-test stack identities remain available");
        Query("DELETE FROM coreAuditFixture WHERE label IN ('crash-destination','crash-source')");
    }
}
}

void MarketExchangeAudit(EVEServiceManager& services) {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Market exchanges require the disposable persistence database");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    auto buyer=SelectMiningAuditCharacter(services);
    auto scalar=[](const char* sql) {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"%s",sql) || !result.GetRow(row)) throw std::runtime_error("Cannot read market fixture");
        return row.GetDouble(0);
    };
    const uint32 initialItem=static_cast<uint32>(scalar("SELECT MAX(itemID) FROM entity"));
    const uint32 initialTransaction=static_cast<uint32>(scalar("SELECT COALESCE(MAX(transactionID),0) FROM mktTransactions"));
    Market::SaveData data={};
    data.typeID=34; data.ownerID=1; data.stationID=stationID;
    data.solarSystemID=sDataMgr.GetStationSystem(stationID); data.regionID=sDataMgr.GetStationRegion(stationID);
    data.price=1.25; data.volEntered=data.volRemaining=20; data.minVolume=1;
    data.accountKey=Account::KeyType::Cash; data.issued=GetFileTimeNow(); data.duration=90;
    const uint32 orderID=MarketDB::StoreOrder(data);
    if (!orderID) throw std::runtime_error("Cannot create market fixture order");
    const char* triggers[]={
        "CREATE TRIGGER core_audit_exchange_failure BEFORE INSERT ON entity FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected market delivery'",
        "CREATE TRIGGER core_audit_exchange_failure BEFORE UPDATE ON mktOrders FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected market order update'",
        "CREATE TRIGGER core_audit_exchange_failure BEFORE INSERT ON mktTransactions FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected market transaction'"};
    const char* descriptions[]={
        "failed market delivery preserves money, order quantity and live wallet",
        "failed market order update rolls back delivered items and payment",
        "failed market transaction record rolls back delivery, order and payment"};
    for (unsigned n=0;n<3;++n) {
        Query(triggers[n]);
        try { sMktMgr.ExecuteSellOrder(buyer.get(),orderID,10,1.25,stationID,34,false); } catch (...) {}
        Query("DROP TRIGGER core_audit_exchange_failure");
        Market::OrderInfo order={};
        DBQueryResult items; DBResultRow row;
        sDatabase.RunQuery(items,"SELECT COUNT(*) FROM entity WHERE itemID>%u",initialItem); items.GetRow(row);
        Check(scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000 && buyer->GetBalance()==5000 &&
            MarketDB::GetOrderInfo(orderID,order) && order.quantity==20 && row.GetUInt(0)==0, descriptions[n]);
        // Reset only disposable fixture state so the original binary can expose every failure.
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        buyer->GetChar()->SetBalanceFromDatabase(5000,Account::CreditType::ISK);
        MarketDB::AlterOrderQuantity(orderID,20);
        std::vector<uint32> cleanup;
        DBQueryResult spawned;
        sDatabase.RunQuery(spawned,"SELECT itemID FROM entity WHERE itemID>%u",initialItem);
        while (spawned.GetRow(row)) cleanup.push_back(row.GetUInt(0));
        for (uint32 id:cleanup) { auto item=sItemFactory.GetItemRef(id); if (item) item->Delete(); }
    }
    sMktMgr.ExecuteSellOrder(buyer.get(),orderID,10,1.25,stationID,34,false);
    Market::OrderInfo order={};
    DBQueryResult items; DBResultRow row;
    sDatabase.RunQuery(items,"SELECT COALESCE(SUM(quantity),0) FROM entity WHERE itemID>%u AND ownerID=90000000 AND locationID=%u AND typeID=34 AND flag=4",initialItem,stationID); items.GetRow(row);
    Check(buyer->GetBalance()==4987.5 && scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==4987.5 &&
        MarketDB::GetOrderInfo(orderID,order) && order.quantity==10 && row.GetUInt(0)==10,
        "successful ordinary-player NPC purchase commits money, delivery and remaining order");
    sMktMgr.ExecuteSellOrder(buyer.get(),orderID,10,2.0,stationID,34,false);
    Check(buyer->GetBalance()==4975 && !MarketDB::GetOrderInfo(orderID,order),
        "complete market purchase charges the locked order price and consumes the order");
    std::vector<uint32> cleanup;
    sDatabase.RunQuery(items,"SELECT itemID FROM entity WHERE itemID>%u",initialItem);
    while (items.GetRow(row)) cleanup.push_back(row.GetUInt(0));
    for (uint32 id:cleanup) { auto item=sItemFactory.GetItemRef(id); if (item) item->Delete(); }
    DBerror error;
    sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE referenceID=%u",orderID);
    sDatabase.RunQuery(error,"DELETE FROM mktTransactions WHERE transactionID>%u",initialTransaction);
    buyer->GetChar()->SetBalanceFromDatabase(5000,Account::CreditType::ISK);
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    for (uint32 type:{691u,606u}) {
        const double blueprints=scalar("SELECT COUNT(*) FROM invBlueprints");
        data.typeID=type; data.volEntered=data.volRemaining=2;
        const uint32 specialOrder=MarketDB::StoreOrder(data);
        Query("CREATE TRIGGER core_audit_exchange_failure BEFORE UPDATE ON mktOrders FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected special-item purchase'");
        try { sMktMgr.ExecuteSellOrder(buyer.get(),specialOrder,1,1.25,stationID,type,false); } catch (...) {}
        Query("DROP TRIGGER core_audit_exchange_failure");
        sDatabase.RunQuery(items,"SELECT COUNT(*) FROM entity WHERE itemID>%u",initialItem); items.GetRow(row);
        Check(buyer->GetBalance()==5000 && scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000 && row.GetUInt(0)==0 &&
            scalar("SELECT COUNT(*) FROM invBlueprints")==blueprints && MarketDB::GetOrderInfo(specialOrder,order) && order.quantity==2,
            type==691 ? "blueprint purchase rollback removes its item and blueprint metadata" : "ship purchase rollback removes the newly spawned ship and payment");
        MarketDB::DeleteOrder(specialOrder);
        sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE referenceID=%u",specialOrder);
    }
    sDatabase.RunQuery(error,"DELETE FROM mktTransactions WHERE transactionID>%u",initialTransaction);
}

void MarketSaleAudit(EVEServiceManager& services) {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Market sales require the disposable persistence database");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    auto seller=SelectMiningAuditCharacter(services);
    auto scalar=[](const char* sql) {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"%s",sql) || !result.GetRow(row)) throw std::runtime_error("Cannot read sale fixture");
        return row.GetDouble(0);
    };
    const uint32 initialItem=static_cast<uint32>(scalar("SELECT MAX(itemID) FROM entity"));
    const uint32 initialTransaction=static_cast<uint32>(scalar("SELECT COALESCE(MAX(transactionID),0) FROM mktTransactions"));
    for (unsigned n=0;n<2;++n) {
        auto source=Stack(100);
        Market::SaveData data={};
        data.bid=true; data.typeID=34; data.ownerID=1; data.stationID=stationID;
        data.solarSystemID=sDataMgr.GetStationSystem(stationID); data.regionID=sDataMgr.GetStationRegion(stationID);
        data.price=1.25; data.volEntered=data.volRemaining=20; data.minVolume=1;
        data.accountKey=Account::KeyType::Cash; data.issued=GetFileTimeNow(); data.duration=90;
        const uint32 orderID=MarketDB::StoreOrder(data);
        if (!orderID) throw std::runtime_error("Cannot create NPC buy order");
        if (n==0) Query("CREATE TRIGGER core_audit_exchange_failure BEFORE INSERT ON mktTransactions FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected sale record'");
        bool completed=false;
        try { completed=sMktMgr.ExecuteBuyOrder(seller.get(),orderID,source,10,false,34,stationID,1.0); } catch (...) {}
        if (n==0) Query("DROP TRIGGER core_audit_exchange_failure");
        Market::OrderInfo order={};
        if (n==0) {
            Check(!completed && source->quantity()==100 && Stored(source,100) && seller->GetBalance()==5000 &&
                scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000 && MarketDB::GetOrderInfo(orderID,order) && order.quantity==20,
                "rejected market sale record restores the source stack, proceeds and buy order");
        } else {
            const double tax=std::round(EvEMath::Market::SalesTax(sConfig.market.salesTax,seller->GetChar()->GetSkillLevel(EvESkill::Accounting),seller->GetChar()->GetSkillLevel(EvESkill::TaxEvasion))*12.5*100.0)/100.0;
            Check(completed && source->quantity()==90 && Stored(source,90) && MarketDB::GetOrderInfo(orderID,order) && order.quantity==10,
                "NPC market sale consumes only the requested ten units from a larger stack");
            Check(seller->GetBalance()==5000+12.5-tax && scalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000+12.5-tax,
                "NPC market sale pays the locked bid price and commits its sales tax");
        }
        MarketDB::DeleteOrder(orderID);
        DBerror error;
        sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE referenceID=%u",orderID);
        std::vector<uint32> cleanup; DBQueryResult items; DBResultRow row;
        sDatabase.RunQuery(items,"SELECT itemID FROM entity WHERE itemID>%u",initialItem);
        while (items.GetRow(row)) cleanup.push_back(row.GetUInt(0));
        for (uint32 id:cleanup) { auto item=sItemFactory.GetItemRef(id); if (item) item->Delete(); }
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        seller->GetChar()->SetBalanceFromDatabase(5000,Account::CreditType::ISK);
    }
    Query("INSERT INTO chrCharacters(characterID,characterName,balance) VALUES (90000001,'Market Audit Recipient',4975)");
    auto source=Stack(100);
    Market::SaveData data={};
    data.bid=true; data.typeID=34; data.ownerID=90000001; data.stationID=stationID;
    data.solarSystemID=sDataMgr.GetStationSystem(stationID); data.regionID=sDataMgr.GetStationRegion(stationID);
    data.price=1.25; data.escrow=25; data.volEntered=data.volRemaining=20; data.minVolume=1;
    data.accountKey=Account::KeyType::Cash; data.issued=GetFileTimeNow(); data.duration=90;
    const uint32 orderID=MarketDB::StoreOrder(data);
    Query("CREATE TRIGGER core_audit_exchange_failure BEFORE UPDATE ON mktOrders FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected escrow update'");
    bool completed=false;
    try { completed=sMktMgr.ExecuteBuyOrder(seller.get(),orderID,source,10,false,34,stationID,1.0); } catch (...) {}
    Query("DROP TRIGGER core_audit_exchange_failure");
    Market::OrderInfo order={};
    Check(!completed && source->quantity()==100 && Stored(source,100) && seller->GetBalance()==5000 && MarketDB::GetOrderInfo(orderID,order) && order.quantity==20 && order.escrow==25 &&
        scalar("SELECT COUNT(*) FROM entity WHERE ownerID=90000001")==0,
        "rejected escrow update restores the player sale, split child, payment and order");
    completed=sMktMgr.ExecuteBuyOrder(seller.get(),orderID,source,10,false,34,stationID,1.0);
    Check(completed && source->quantity()==90 && Stored(source,90) && MarketDB::GetOrderInfo(orderID,order) && order.quantity==10 && order.escrow==12.5 &&
        scalar("SELECT SUM(quantity) FROM entity WHERE ownerID=90000001 AND typeID=34")==10 && scalar("SELECT balance FROM chrCharacters WHERE characterID=90000001")==4975,
        "player buy-order fill conserves items and reduces escrow without charging its buyer twice");
    MarketDB::DeleteOrder(orderID);
    std::vector<uint32> cleanup; DBQueryResult items; DBResultRow row;
    sDatabase.RunQuery(items,"SELECT itemID FROM entity WHERE itemID>%u",initialItem);
    while (items.GetRow(row)) cleanup.push_back(row.GetUInt(0));
    for (uint32 id:cleanup) { auto item=sItemFactory.GetItemRef(id); if (item) item->Delete(); }
    DBerror error;
    sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE referenceID=%u",orderID);
    sDatabase.RunQuery(error,"DELETE FROM mktTransactions WHERE transactionID>%u",initialTransaction);
    Query("DELETE FROM chrCharacters WHERE characterID=90000001");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
    seller->GetChar()->SetBalanceFromDatabase(5000,Account::CreditType::ISK);
}

void TransactionCacheAudit() {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Transaction cache tests require the disposable persistence database");
    auto source=Stack(100), destination=Stack(50);
    {
        DBTransaction outer;
        Check(destination->Merge(source,25,false) && destination->Merge(source,25,false) && source->quantity()==50 && destination->quantity()==100,
            "repeated mutations within an exchange use the current staged quantities");
    }
    Check(source->quantity()==100 && destination->quantity()==50 && Stored(source,100) && Stored(destination,50),
        "outer rollback restores both live stacks after repeated nested merges");
    {
        DBTransaction outer;
        if (!destination->Merge(source,0,false)) throw std::runtime_error("Cannot merge rollback fixture");
    }
    Check(source->quantity()==100 && destination->quantity()==50 && Stored(source,100) && Stored(destination,50) && sItemFactory.GetItemRef(source->itemID()).get()==source.get(),
        "rollback restores an exhausted stack to the factory and its original identity");
    InventoryItemRef child;
    {
        DBTransaction outer;
        child=source->Split(25,true);
        if (!child) throw std::runtime_error("Cannot split rollback fixture");
        child->Donate(90000001,60008494,flagHangar,true);
        source->Donate(90000001,60008494,flagHangar,true);
    }
    ItemData absent;
    Check(source->quantity()==100 && source->ownerID()==ownerID && source->locationID()==stationID && Stored(source,100) &&
        !ItemDB::GetItemData(child->itemID(),absent) && !sItemFactory.GetItemRef(child->itemID()),
        "rollback removes a split child and restores ownership and inventory membership");
    InventoryItemRef spawned;
    {
        DBTransaction outer;
        spawned=Stack(10);
        spawned->Delete();
    }
    Check(!ItemDB::GetItemData(spawned->itemID(),absent) && !sItemFactory.GetItemRef(spawned->itemID()),
        "rollback discards a newly spawned then deleted item without resurrecting it");
    unsigned published=0,undone=0;
    {
        DBTransaction outer;
        DBTransaction::OnRollback([&undone]() { ++undone; });
        DBTransaction::Publish([&published]() { ++published; });
        { DBTransaction inner; if (!inner.Commit()) throw std::runtime_error("Cannot commit nested fixture"); }
        Check(published==0 && undone==0,"nested commits defer publication until the outer exchange commits");
        if (!outer.Commit()) throw std::runtime_error("Cannot commit transaction fixture");
    }
    Check(published==1 && undone==0 && !DBTransaction::Active(),"outer commit publishes once and releases its transaction state");
    {
        DBTransaction outer;
        DBTransaction::OnRollback([&undone]() { ++undone; });
        DBTransaction::Publish([&published]() { ++published; });
    }
    Check(published==1 && undone==1,"rollback discards notifications and invokes cache recovery once");
    source->Delete(); destination->Delete();
    ItemData blueprintData(691,ownerID,stationID,flagHangar,2);
    auto blueprint=sItemFactory.SpawnItem(blueprintData);
    if (!blueprint) throw std::runtime_error("Cannot create blueprint split fixture");
    Query("CREATE TRIGGER core_audit_exchange_failure BEFORE INSERT ON invBlueprints FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected blueprint split metadata'");
    auto failed=blueprint->Split(1,false);
    Query("DROP TRIGGER core_audit_exchange_failure");
    Check(!failed && blueprint->quantity()==2 && Stored(blueprint,2),"rejected blueprint metadata copy preserves the unsplit source");
    auto split=blueprint->Split(1,false);
    Check(split && blueprint->quantity()==1 && Stored(blueprint,1) && Stored(split,1),"blueprint split commits metadata before loading its child");
    Query("CREATE TRIGGER core_audit_exchange_failure BEFORE DELETE ON invBlueprints FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected blueprint delete'");
    bool rejected=false;
    try { blueprint->Delete(); } catch (...) { rejected=true; }
    Query("DROP TRIGGER core_audit_exchange_failure");
    Check(rejected && Stored(blueprint,1),"failed blueprint metadata deletion preserves the owning blueprint item");
    blueprint->Delete(); split->Delete();
}

void MarketCrashAudit(const char* mode, EVEServiceManager& services) {
    DBQueryResult guard; DBResultRow marker;
    if (!sDatabase.RunQuery(guard,"SELECT marker FROM coreAuditEnvironment WHERE id=1") || !guard.GetRow(marker) || std::strcmp(marker.GetText(0),"isolated-persistence-audit"))
        throw std::runtime_error("Market crash tests require the disposable persistence database");
    const bool recovery=std::strstr(mode,"recover")!=nullptr;
    const bool committed=std::strstr(mode,"committed")!=nullptr;
    Query("DROP TRIGGER IF EXISTS core_audit_market_pause");
    auto scalar=[](const char* sql) {
        DBQueryResult result; DBResultRow row;
        if (!sDatabase.RunQuery(result,"%s",sql) || !result.GetRow(row)) throw std::runtime_error("Cannot read market crash fixture");
        return row.GetUInt(0);
    };
    if (!recovery) {
        if (scalar("SELECT COUNT(*) FROM coreAuditFixture WHERE label LIKE 'market-%'")) throw std::runtime_error("Unrecovered market crash fixture exists");
        Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
        auto buyer=SelectMiningAuditCharacter(services);
        const uint32 lastItem=scalar("SELECT MAX(itemID) FROM entity");
        const uint32 lastTransaction=scalar("SELECT COALESCE(MAX(transactionID),0) FROM mktTransactions");
        Market::SaveData data={};
        data.typeID=34; data.ownerID=1; data.stationID=stationID;
        data.solarSystemID=sDataMgr.GetStationSystem(stationID); data.regionID=sDataMgr.GetStationRegion(stationID);
        data.price=1.25; data.volEntered=data.volRemaining=20; data.minVolume=1;
        data.accountKey=Account::KeyType::Cash; data.issued=GetFileTimeNow(); data.duration=90;
        const uint32 orderID=MarketDB::StoreOrder(data);
        if (!orderID) throw std::runtime_error("Cannot create market crash order");
        DBerror error;
        if (!sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES ('market-order',%u),('market-last-item',%u),('market-last-transaction',%u)",orderID,lastItem,lastTransaction))
            throw std::runtime_error("Cannot record market crash identities");
        if (!committed) Query("CREATE TRIGGER core_audit_market_pause BEFORE UPDATE ON mktOrders FOR EACH ROW DO SLEEP(45)");
        sMktMgr.ExecuteSellOrder(buyer.get(),orderID,10,1.25,stationID,34,false);
        std::printf("CORE_AUDIT MARKET COMMITTED: terminating before orderly client or server shutdown\n");
        std::fflush(stdout);
        std::_Exit(86);
    }
    const uint32 orderID=scalar("SELECT itemID FROM coreAuditFixture WHERE label='market-order'");
    const uint32 lastItem=scalar("SELECT itemID FROM coreAuditFixture WHERE label='market-last-item'");
    const uint32 lastTransaction=scalar("SELECT itemID FROM coreAuditFixture WHERE label='market-last-transaction'");
    Market::OrderInfo order={};
    Check(MarketDB::GetOrderInfo(orderID,order) && order.quantity==(committed ? 10u : 20u),
        "fresh process reloads the correct remaining order after abrupt market termination");
    DBQueryResult wallet; DBResultRow row;
    sDatabase.RunQuery(wallet,"SELECT balance FROM chrCharacters WHERE characterID=90000000"); wallet.GetRow(row);
    Check(row.GetDouble(0)==(committed ? 4987.5 : 5000),"fresh process reloads the matching wallet outcome after a market crash");
    std::vector<uint32> cleanup;
    DBQueryResult items;
    sDatabase.RunQuery(items,"SELECT itemID FROM entity WHERE itemID>%u",lastItem);
    uint32 quantity=0;
    while (items.GetRow(row)) {
        auto item=sItemFactory.GetItemRef(row.GetUInt(0));
        if (!item || item->ownerID()!=ownerID || item->locationID()!=stationID || item->typeID()!=34) throw std::runtime_error("Unexpected market crash item");
        quantity+=item->quantity(); cleanup.push_back(item->itemID());
    }
    Check(quantity==(committed ? 10u : 0u),"fresh process sees exactly the delivered quantity or the rolled-back absence");
    sDatabase.RunQuery(items,"SELECT COUNT(*) FROM mktTransactions WHERE transactionID>%u",lastTransaction); items.GetRow(row);
    Check(row.GetUInt(0)==(committed ? 2u : 0u),"market history has both committed transaction records or neither after a crash");
    for (uint32 id:cleanup) sItemFactory.GetItemRef(id)->Delete();
    MarketDB::DeleteOrder(orderID);
    DBerror error;
    sDatabase.RunQuery(error,"DELETE FROM mktTransactions WHERE transactionID>%u",lastTransaction);
    sDatabase.RunQuery(error,"DELETE FROM jnlCharacters WHERE referenceID=%u",orderID);
    Query("DELETE FROM coreAuditFixture WHERE label LIKE 'market-%'");
    Query("UPDATE chrCharacters SET balance=5000 WHERE characterID=90000000");
}

#include "testing/StandingOrderAudit.inc"
#include "testing/StandingCrashAudit.inc"
#include "testing/TradeAudit.inc"
#include "testing/ItemStateAudit.inc"

bool IsCoreAuditMode(const char* mode) {
    if (mode && (!std::strcmp(mode,"--core-audit-crash-fitting-fit") || !std::strcmp(mode,"--core-audit-recover-fitting-fit") ||
        !std::strcmp(mode,"--core-audit-crash-fitting-fit-committed") || !std::strcmp(mode,"--core-audit-recover-fitting-fit-committed") ||
        !std::strcmp(mode,"--core-audit-crash-fitting-unfit") || !std::strcmp(mode,"--core-audit-recover-fitting-unfit") ||
        !std::strcmp(mode,"--core-audit-crash-fitting-unfit-committed") || !std::strcmp(mode,"--core-audit-recover-fitting-unfit-committed"))) return true;
    if (mode && (!std::strcmp(mode,"--core-audit-crash-item-state") || !std::strcmp(mode,"--core-audit-recover-item-state") ||
        !std::strcmp(mode,"--core-audit-crash-item-state-committed") || !std::strcmp(mode,"--core-audit-recover-item-state-committed"))) return true;
    if (mode && (!std::strcmp(mode,"--core-audit-blueprint-state") || !std::strcmp(mode,"--core-audit-typed-spawns") || !std::strcmp(mode,"--core-audit-compound-fitting"))) return true;
    if (mode && !std::strcmp(mode,"--core-audit-recover-trade-fixture")) return true;
    if (mode && (!std::strcmp(mode,"--core-audit-crash-trade-offer") || !std::strcmp(mode,"--core-audit-recover-trade-offer") ||
        !std::strcmp(mode,"--core-audit-crash-trade-exchange") || !std::strcmp(mode,"--core-audit-recover-trade-exchange") ||
        !std::strcmp(mode,"--core-audit-crash-trade-exchange-committed") || !std::strcmp(mode,"--core-audit-recover-trade-exchange-committed") ||
        !std::strcmp(mode,"--core-audit-crash-trade-cancel") || !std::strcmp(mode,"--core-audit-recover-trade-cancel") ||
        !std::strcmp(mode,"--core-audit-crash-trade-cancel-committed") || !std::strcmp(mode,"--core-audit-recover-trade-cancel-committed"))) return true;
    if (mode && !std::strcmp(mode,"--core-audit-trades")) return true;
    return mode && (!std::strcmp(mode,"--core-audit-crash-standing-buy") || !std::strcmp(mode,"--core-audit-recover-standing-buy") || !std::strcmp(mode,"--core-audit-crash-standing-buy-committed") || !std::strcmp(mode,"--core-audit-recover-standing-buy-committed") || !std::strcmp(mode,"--core-audit-crash-standing-sell") || !std::strcmp(mode,"--core-audit-recover-standing-sell") || !std::strcmp(mode,"--core-audit-crash-standing-sell-committed") || !std::strcmp(mode,"--core-audit-recover-standing-sell-committed") || !std::strcmp(mode,"--core-audit-standing-orders") || !std::strcmp(mode,"--core-audit-market-sales") || !std::strcmp(mode,"--core-audit-transaction-cache") || !std::strcmp(mode,"--core-audit-crash-market") ||
                    !std::strcmp(mode,"--core-audit-recover-market") || !std::strcmp(mode,"--core-audit-crash-market-committed") ||
                    !std::strcmp(mode,"--core-audit-recover-market-committed") || !std::strcmp(mode, "--core-audit-market-exchanges") || !std::strcmp(mode, "--core-audit-inventory") || !std::strcmp(mode, "--core-audit-system") ||
                    !std::strcmp(mode, "--core-audit-write") || !std::strcmp(mode, "--core-audit-read") ||
                    !std::strcmp(mode, "--core-audit-lifecycle") || !std::strcmp(mode, "--core-audit-mining-logout") || !std::strcmp(mode,"--core-audit-failures") ||
                    !std::strcmp(mode,"--core-audit-crash-merge") || !std::strcmp(mode,"--core-audit-crash-wallet") ||
                    !std::strcmp(mode,"--core-audit-crash-merge-committed") || !std::strcmp(mode,"--core-audit-crash-wallet-committed") ||
                    !std::strcmp(mode,"--core-audit-recover-merge") || !std::strcmp(mode,"--core-audit-recover-wallet") ||
                    !std::strcmp(mode,"--core-audit-recover-merge-committed") || !std::strcmp(mode,"--core-audit-recover-wallet-committed"));
}

bool CoreAuditEnvironmentValid() {
    const char* enabled = std::getenv("EVEMU_CORE_AUDIT");
    if (!enabled || std::strcmp(enabled, "1")) return false;
    DBQueryResult result;
    DBResultRow row;
    return sDatabase.RunQuery(result, "SELECT marker FROM coreAuditEnvironment WHERE id=1") &&
           result.GetRow(row) && (!std::strcmp(row.GetText(0), "isolated-evemu-development") ||
                                 !std::strcmp(row.GetText(0), "isolated-mining-logout-audit") || !std::strcmp(row.GetText(0), "isolated-persistence-audit"));
}

int RunCoreAudit(const char* mode, EVEServiceManager& services) {
    try {
        if (std::strstr(mode,"-fitting-") && (std::strstr(mode,"crash") || std::strstr(mode,"recover"))) {
            FittingCrashAudit(mode,services);
            std::printf("CORE_AUDIT RESULT: %u failures\n",failures);
            return failures ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        if (std::strstr(mode,"-item-state") && (std::strstr(mode,"crash") || std::strstr(mode,"recover"))) {
            ItemStateCrashAudit(mode,services);
            std::printf("CORE_AUDIT RESULT: %u failures\n",failures);
            return failures ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        if (!std::strcmp(mode,"--core-audit-blueprint-state") || !std::strcmp(mode,"--core-audit-typed-spawns") || !std::strcmp(mode,"--core-audit-compound-fitting")) {
            ItemStateAudit(mode,services);
            std::printf("CORE_AUDIT RESULT: %u failures\n",failures);
            return failures ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        if (!std::strcmp(mode,"--core-audit-recover-trade-fixture")) {
            TradeFixtureRestore();std::printf("CORE_AUDIT RESULT: %u failures\n",failures);return failures ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        if (std::strstr(mode,"-trade-") && (std::strstr(mode,"crash") || std::strstr(mode,"recover"))) {
            TradeCrashAudit(mode,services);
            std::printf("CORE_AUDIT RESULT: %u failures\n",failures);
            return failures ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        if (!std::strcmp(mode,"--core-audit-trades")) {
            TradeAudit(services);
            std::printf("CORE_AUDIT RESULT: %u failures\n",failures);
            return failures ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        Query("DROP TRIGGER IF EXISTS core_audit_fail_spawn");
        if (std::strstr(mode,"standing-") && (std::strstr(mode,"crash") || std::strstr(mode,"recover"))) StandingCrashAudit(mode,services);
        else if (!std::strcmp(mode,"--core-audit-standing-orders")) StandingOrderAudit(services);
        else if (!std::strcmp(mode,"--core-audit-market-sales")) MarketSaleAudit(services);
        else if (std::strstr(mode,"market") && std::strstr(mode,"exchanges")==nullptr) MarketCrashAudit(mode,services);
        else if (!std::strcmp(mode, "--core-audit-transaction-cache")) TransactionCacheAudit();
        else if (!std::strcmp(mode, "--core-audit-market-exchanges")) MarketExchangeAudit(services);
        else if (!std::strcmp(mode, "--core-audit-inventory")) InventoryAudit();
        else if (!std::strcmp(mode, "--core-audit-system")) SystemAudit(services);
        else if (!std::strcmp(mode, "--core-audit-lifecycle")) ModuleLifecycleAudit(services);
        else if (!std::strcmp(mode, "--core-audit-mining-logout")) MiningLogoutAudit(services);
        else if (!std::strcmp(mode, "--core-audit-failures")) PersistenceFailures(services);
        else if (std::strstr(mode,"--core-audit-crash-")==mode) PersistenceCrash(mode);
        else if (std::strstr(mode,"--core-audit-recover-")==mode) PersistenceRecovery(mode);
        else if (!std::strcmp(mode, "--core-audit-write")) PersistenceWrite();
        else PersistenceRead();
    } catch (const std::exception& error) {
        Query("DROP TRIGGER IF EXISTS core_audit_fail_spawn");
        std::printf("CORE_AUDIT FAIL: %s\n", error.what());
        ++failures;
    }
    std::printf("CORE_AUDIT RESULT: %u failures\n", failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
