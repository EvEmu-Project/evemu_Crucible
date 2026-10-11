// These tests run only in the disposable persistence schema, with recoverable snapshots.
void ItemStateFailure() {
    DBerror error;
    if (sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES ('item-state-failure',0)"))
        throw std::runtime_error("The deliberate late-write failure was not rejected");
}
void ItemStateFault(bool enable) {
    Query("DROP TRIGGER IF EXISTS core_audit_item_state");
    if (enable) Query("CREATE TRIGGER core_audit_item_state BEFORE INSERT ON coreAuditFixture FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected later write'");
}
bool BlueprintStored(BlueprintRef blueprint, bool copy, int me, int pe, int runs) {
    EvERam::bpData data={};
    return FactoryDB::GetBlueprintData(blueprint->itemID(),data) && data.copy==copy &&
        data.mLevel==me && data.pLevel==pe && data.runs==runs;
}
BlueprintRef ItemStateBlueprint(bool copy, int me=2, int pe=3, int runs=10) {
    ItemData data(691,ownerID,stationID,flagHangar,2);
    EvERam::bpData metadata={};metadata.copy=copy;metadata.mLevel=me;metadata.pLevel=pe;metadata.runs=runs;
    auto blueprint=Blueprint::Spawn(data,metadata);
    if (!blueprint) throw std::runtime_error("Cannot allocate blueprint state fixture");
    return blueprint;
}
void BlueprintStateAudit() {
    auto original=ItemStateBlueprint(false),copy=ItemStateBlueprint(true);
    InventoryItemRef base=original;
    Check(!base->Merge(copy,1,false) && original->quantity()==2 && copy->quantity()==2 && Stored(original,2) && Stored(copy,2),
        "inventory reference rejects stacking a blueprint original with a copy");
    auto research=ItemStateBlueprint(false,7);
    Check(!base->Merge(research,1,false) && Stored(research,2),"inventory reference preserves different blueprint research levels");
    auto other=Stack(4);
    Check(!base->Merge(other,1,false) && Stored(other,4),"blueprint stacking safely rejects a non-blueprint item");
    original->SetCopy(true);
    Check(original->copy() && BlueprintStored(original,true,2,3,10),"changing blueprint copy status persists the new status");
    original->SetCopy(false);
    const auto id=original->itemID();
    DBerror error;
    if (!sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_item_state BEFORE UPDATE ON invBlueprints FOR EACH ROW BEGIN IF OLD.itemID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected blueprint update'; END IF; END",id))
        throw std::runtime_error(error.c_str());
    bool rejected=false;
    try { original->SetMLevel(8); } catch (...) { rejected=true; }
    Check(rejected && original->mLevel()==2 && BlueprintStored(original,false,2,3,10),"rejected research save preserves blueprint memory and database state");
    Query("DROP TRIGGER core_audit_item_state");
    // Discard any baseline cache mutation so the next check has a known start.
    original->SetMLevel(2);
    ItemStateFault(true);
    {
        DBTransaction outer;
        original->SetMLevel(4);original->SetPLevel(5);original->SetRuns(6);original->SetCopy(true);
        ItemStateFailure();
        Check(!outer.Commit(),"late failure rejects the compound blueprint update");
    }
    ItemStateFault(false);
    Check(!original->copy() && original->mLevel()==2 && original->pLevel()==3 && original->runs()==10 && BlueprintStored(original,false,2,3,10),
        "late failure restores all blueprint metadata in memory and storage");
    original->SetCopy(false);original->SetMLevel(2);original->SetPLevel(3);original->SetRuns(10);
    Query("CREATE TRIGGER core_audit_item_state BEFORE INSERT ON invBlueprints FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected blueprint allocation'");
    Query("CREATE TRIGGER core_audit_blueprint_cleanup BEFORE DELETE ON entity FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected partial allocation cleanup'");
    const auto count=TradeScalar("SELECT COUNT(*) FROM entity");
    ItemData data(691,ownerID,stationID,flagHangar,1);EvERam::bpData metadata={};
    auto missing=Blueprint::Spawn(data,metadata);
    Query("DROP TRIGGER core_audit_item_state");
    Query("DROP TRIGGER core_audit_blueprint_cleanup");
    Check(!missing && TradeScalar("SELECT COUNT(*) FROM entity")==count,"failed direct blueprint allocation leaves no partial entity row");
    original->SetQuantity(2,false);
    auto equal=ItemStateBlueprint(false);
    Check(base->Merge(equal,1,false) && Stored(original,3) && Stored(equal,1),"identical blueprints still stack through the inventory interface");
    ItemStateFault(true);
    {
        DBTransaction outer;
        if (!base->Merge(equal,1,false)) throw std::runtime_error("Cannot stage blueprint merge");
        ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late blueprint merge failure committed");
    }
    ItemStateFault(false);
    Check(original->quantity()==3 && equal->quantity()==1 && Stored(original,3) && Stored(equal,1) && BlueprintStored(equal,false,2,3,10),
        "late merge failure restores blueprint quantities, metadata and deleted source cache");
    rejected=false;try { original->UpdateMLevel(std::numeric_limits<int32>::max()); } catch (...) { rejected=true; }
    Check(rejected && original->mLevel()==2 && BlueprintStored(original,false,2,3,10),"overflowing research cannot wrap blueprint material levels");
    rejected=false;try { original->UpdateRuns(std::numeric_limits<int32>::min()); } catch (...) { rejected=true; }
    Check(rejected && original->runs()==10 && BlueprintStored(original,false,2,3,10),"overflowing run updates preserve licensed blueprint runs");
    DBerror changed;
    if (!sDatabase.RunQuery(changed,"UPDATE invBlueprints SET runs=11 WHERE itemID=%u",equal->itemID())) throw std::runtime_error(changed.c_str());
    Check(!base->Merge(equal,1,false) && Stored(original,3) && Stored(equal,1),"stacking rejects blueprint metadata changed since the item was loaded");
    BlueprintRef provisional;uint32 provisionalID=0;
    ItemStateFault(true);
    {
        DBTransaction outer;
        provisional=ItemStateBlueprint(true,9,8,7);provisionalID=provisional->itemID();
        ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late direct blueprint allocation failure committed");
    }
    ItemStateFault(false);
    ItemData removed;
    Check(!sItemFactory.GetItemRefFromID(provisionalID,false) && !ItemDB::GetItemData(provisionalID,removed),
        "late direct blueprint allocation failure discards its factory entry while a caller retains the reference");
    sItemFactory.SaveItems();
    EvERam::bpData removedMetadata={};
    Check(!ItemDB::GetItemData(provisionalID,removed) && !FactoryDB::GetBlueprintData(provisionalID,removedMetadata),
        "factory save cannot resurrect a directly spawned blueprint or its metadata after rollback");
    Check(TradeScalar("SELECT COUNT(*) FROM entity_attributes a LEFT JOIN entity e ON e.itemID=a.itemID WHERE e.itemID IS NULL")==0,
        "direct blueprint rollback with a retained reference leaves no orphan attributes");
    copy->Delete();
    PyPackedRow* deletedRow=copy->GetItemRow();
    Check(deletedRow && PyRep::IntegerValue(deletedRow->GetField(deletedRow->header()->FindColumn("quantity")))==-2,
        "a retained deleted blueprint copy can encode its notification row without reloading a missing factory entry");
    PyDecRef(deletedRow);
    original->Delete();research->Delete();other->Delete();equal->Delete();
}
void TypedSpawnAudit() {
    for (unsigned kind=0;kind<4;++kind) {
        InventoryItemRef item;uint32 id=0;
        ItemStateFault(true);
        {
            DBTransaction outer;
            ItemData data(kind==0 ? 606 : kind==1 ? 3300 : kind==2 ? 3651 : 3297,ownerID,stationID,flagHangar,1);
            if (kind==0) item=sItemFactory.SpawnShip(data);
            if (kind==1) item=sItemFactory.SpawnSkill(data);
            if (kind==2) item=sItemFactory.SpawnModule(data);
            if (kind==3) item=sItemFactory.SpawnCargoContainer(data);
            if (!item) throw std::runtime_error("Cannot allocate typed item fixture");
            id=item->itemID();ItemStateFailure();
            if (outer.Commit()) throw std::runtime_error("Late item creation failure committed");
        }
        ItemStateFault(false);
        ItemData stored;
        const char* cache[] = {"rolled-back ship leaves no factory entry","rolled-back skill leaves no factory entry","rolled-back module leaves no factory entry","rolled-back cargo container leaves no factory entry"};
        Check(!ItemDB::GetItemData(id,stored) && !sItemFactory.GetItemRefFromID(id,false),cache[kind]);
        sItemFactory.SaveItems();
        const char* saved[] = {"saving factory items cannot resurrect a rolled-back ship","saving factory items cannot resurrect a rolled-back skill","saving factory items cannot resurrect a rolled-back module","saving factory items cannot resurrect a rolled-back cargo container"};
        Check(!ItemDB::GetItemData(id,stored),saved[kind]);
        // Baseline cleanup also clears the still-cached provisional reference.
        item->DiscardUncommitted();item=InventoryItemRef(nullptr);
    }
}
void CompoundFittingAudit(EVEServiceManager& services) {
    auto client=TradeClient(services,ownerID,1);
    auto ship=client->GetShip();auto manager=ship->GetModuleManager();auto item=sItemFactory.GetItemRef(140000014);
    item->Move(ship->itemID(),flagCargoHold,false);
    ItemStateFault(true);
    {
        DBTransaction outer;
        if (ship->AddItemByFlag(flagHiSlot0,item,client.get())!=item->itemID()) throw std::runtime_error("Cannot fit the fixture miner");
        ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late fitting failure committed");
    }
    ItemStateFault(false);
    Check(item->flag()==flagCargoHold && !manager->GetModule(flagHiSlot0),"late fitting failure restores an empty module slot with no phantom entry");
    // A failed baseline fit may leave the map populated; clear it before the second case.
    if (manager->GetModule(flagHiSlot0)) manager->UnfitModule(flagHiSlot0);
    if (ship->AddItemByFlag(flagHiSlot0,item,client.get())!=item->itemID()) throw std::runtime_error("Cannot refit the fixture miner");
    auto miner=manager->GetModule(flagHiSlot0);
    ItemStateFault(true);
    {
        DBTransaction outer;
        item->Move(ship->itemID(),flagCargoHold,false);
        ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late unfitting failure committed");
    }
    ItemStateFault(false);
    Check(item->flag()==flagHiSlot0 && manager->GetModule(flagHiSlot0)==miner,"late unfitting failure restores the original module manager entry");
    // Use a normal weapon because the civilian blaster cannot load ammunition.
    item->Move(60014689,flagHangar,false);
    ItemData weaponData(562,ownerID,60014689,flagHangar,1);
    auto weapon=sItemFactory.SpawnModule(weaponData);
    if (!weapon || ship->AddItemByFlag(flagHiSlot0,weapon,client.get())!=weapon->itemID())
        throw std::runtime_error("Cannot fit the ordinary blaster fixture");
    manager->Online(weapon->itemID());
    ItemData chargeData(222,ownerID,60014689,flagHangar,20);
    auto ammunition=sItemFactory.SpawnItem(chargeData);
    if (!ammunition || !ship->AddItemByFlag(flagHiSlot0,ammunition,client.get()))
        throw std::runtime_error("Cannot load the ordinary blaster fixture");
    auto loaded=manager->GetLoadedChargeOnModule(flagHiSlot0);
    auto blaster=manager->GetModule(flagHiSlot0);
    if (!loaded || !blaster || !blaster->IsLoaded() || loaded->quantity()!=20)
        throw std::runtime_error("The fixture did not load its actual ammunition");
    const auto turretsBefore=ship->GetAttribute(AttrTurretSlotsLeft);
    auto onlineBefore=weapon->GetAttribute(AttrOnline);
    Check(onlineBefore.get_bool() && loaded->locationID()==ship->itemID() && loaded->flag()==flagHiSlot0,
        "ordinary player fixture has an online blaster loaded with twenty real charges");
    ItemStateFault(true);
    {
        DBTransaction outer;
        weapon->Move(60014689,flagHangar,false);
        ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late loaded-unfitting failure committed");
    }
    ItemStateFault(false);
    auto restored=manager->GetModule(flagHiSlot0);
    Check(restored==blaster && restored && restored->IsLoaded() && restored->GetLoadedChargeRef().get()==loaded.get(),
        "late loaded-unfitting failure preserves the original weapon and its loaded charge reference");
    Check(weapon->GetAttribute(AttrOnline)==onlineBefore && ship->GetAttribute(AttrTurretSlotsLeft)==turretsBefore,
        "late loaded-unfitting failure restores online state and turret slot counters");
    ItemData storedCharge;
    Check(loaded->quantity()==20 && loaded->locationID()==ship->itemID() && loaded->flag()==flagHiSlot0 &&
        ItemDB::GetItemData(loaded->itemID(),storedCharge) && storedCharge.quantity==20 && storedCharge.locationID==ship->itemID() && storedCharge.flag==flagHiSlot0,
        "late loaded-unfitting failure restores the actual ammunition quantity and sublocation in cache and database");
    Check(!manager->GetModule(loaded->itemID()),"a loaded charge item ID cannot alias its weapon's module lookup");
    DBerror error;
    if (!sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_item_state BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected ammunition move'; END IF; END",loaded->itemID()))
        throw std::runtime_error(error.c_str());
    bool rejected=false;try {weapon->Move(60014689,flagHangar,true);} catch (...) {rejected=true;}
    ItemStateFault(false);ItemData storedWeapon;
    Check(rejected && manager->GetModule(flagHiSlot0)==blaster && blaster->IsLoaded() && weapon->flag()==flagHiSlot0 &&
        ItemDB::GetItemData(weapon->itemID(),storedWeapon) && storedWeapon.flag==flagHiSlot0 && loaded->quantity()==20,
        "a rejected ammunition move rolls back the standalone weapon move and preserves its original runtime object");
    ItemData refillData(222,ownerID,60014689,flagHangar,10);auto refill=sItemFactory.SpawnItem(refillData);
    manager->LoadCharge(refill,flagHiSlot0);
    Check(blaster->GetLoadedChargeRef().get()==loaded.get() && loaded->quantity()==30 && Stored(loaded,30,ship->itemID()) && !sItemFactory.GetItemRefFromID(refill->itemID(),false),
        "reloading merges real ammunition and retains the destination charge rather than the deleted source");
    ItemStateFault(true);
    {
        DBTransaction outer;
        manager->UnloadCharge(blaster);
        manager->LoadCharge(loaded,flagHiSlot0);
        weapon->Move(60014689,flagHangar,false);
        if (!ship->AddItemByFlag(flagHiSlot0,weapon,client.get())) throw std::runtime_error("Cannot stage repeated fitting changes");
        ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Repeated fitting failure committed");
    }
    ItemStateFault(false);
    Check(manager->GetModule(flagHiSlot0)==blaster && blaster->IsLoaded() && blaster->GetLoadedChargeRef().get()==loaded.get() &&
        loaded->quantity()==30 && loaded->flag()==flagHiSlot0 && weapon->GetAttribute(AttrOnline)==onlineBefore && ship->GetAttribute(AttrTurretSlotsLeft)==turretsBefore,
        "repeated unload, reload, unfit and refit in one failed unit restore the original weapon, ammunition and counters");
    manager->UnloadCharge(blaster);
    auto civilian=sItemFactory.GetItemRef(140000015);civilian->Move(60014689,flagHangar,false);
    auto slave=sItemFactory.SpawnModule(weaponData);
    if (!slave || !ship->AddItemByFlag(flagHiSlot1,slave,client.get())) throw std::runtime_error("Cannot fit linked weapon fixture");
    manager->Online(slave->itemID());
    auto slaveModule=manager->GetModule(slave->itemID());
    ship->LinkWeapon(weapon->itemID(),slave->itemID());
    Check(blaster->IsLinked() && blaster->IsMaster() && slaveModule->IsLinked() && ship->GetLinkedCount(blaster)==2 &&
        TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups WHERE shipID=140000013")==1,"ordinary player weapon linking persists the real master and slave");
    ItemStateFault(true);
    {
        DBTransaction outer;slave->Move(60014689,flagHangar,false);ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Linked weapon rollback committed");
    }
    ItemStateFault(false);
    Check(manager->GetModule(slave->itemID())==slaveModule && blaster->IsLinked() && blaster->IsMaster() && slaveModule->IsLinked() &&
        ship->GetLinkedCount(blaster)==2 && TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups WHERE shipID=140000013")==1,
        "failed linked slave removal restores both module pointers, link flags and the stored weapon group");
    Query("CREATE TRIGGER core_audit_item_state BEFORE DELETE ON shipWeaponGroups FOR EACH ROW SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected weapon unlink'");
    rejected=false;try {slave->Move(60014689,flagHangar,false);} catch (...) {rejected=true;}
    ItemStateFault(false);
    Check(rejected && manager->GetModule(slave->itemID())==slaveModule && slave->flag()==flagHiSlot1 && blaster->IsLinked() &&
        ship->GetLinkedCount(blaster)==2 && TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups WHERE shipID=140000013")==1,
        "a rejected linked-group delete preserves the standalone fitting move and all group state");
    slave->Move(60014689,flagHangar,false);
    Check(!blaster->IsLinked() && ship->GetLinkedCount(blaster)==1 && !manager->GetModule(slave->itemID()) &&
        TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups WHERE shipID=140000013")==0,
        "successful linked slave removal safely retires the module and clears durable group membership");
    manager->LoadCharge(loaded,flagHiSlot0);
    weapon->Move(60014689,flagHangar,false);
    Check(!manager->GetModule(weapon->itemID()) && loaded->locationID()==60014689 && loaded->flag()==flagHangar && loaded->quantity()==30 &&
        weapon->flag()==flagHangar && !weapon->GetAttribute(AttrOnline).get_bool(),"committed unfitting preserves the ammunition and leaves the weapon offline in the hangar");
    const auto countBefore=TradeScalar("SELECT COUNT(*) FROM entity");
    weaponData.quantity=2;auto stack=sItemFactory.SpawnModule(weaponData);
    auto bound=new InventoryBound(services,*services.Lookup<InvBrokerService>("invbroker"),ship,flagCargoHold,ownerID,false);
    bound->NewReference(client.get());
    Query("CREATE TRIGGER core_audit_item_state BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF NEW.typeID=562 AND NEW.flag=27 AND OLD.flag<>27 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected fitting after stack split'; END IF; END");
    auto tuple=new PyTuple(2);tuple->SetItem(0,new PyInt(stack->itemID()));tuple->SetItem(1,new PyInt(60014689));
    auto named=new PyDict();named->SetItemString("qty",new PyInt(1));named->SetItemString("flag",new PyInt(flagHiSlot0));
    rejected=!TradeCall(services,client.get(),bound->GetBoundID(),"Add",tuple,named);
    ItemStateFault(false);
    Check(rejected && stack->quantity()==2 && Stored(stack,2,60014689) && !manager->GetModule(flagHiSlot0) &&
        TradeScalar("SELECT COUNT(*) FROM entity")==countBefore+1,"failed client inventory fitting restores the split source and leaves no provisional module or item");
    tuple=new PyTuple(2);tuple->SetItem(0,new PyInt(stack->itemID()));tuple->SetItem(1,new PyInt(60014689));
    named=new PyDict();named->SetItemString("qty",new PyInt(1));named->SetItemString("flag",new PyInt(flagHiSlot0));
    Check(TradeCall(services,client.get(),bound->GetBoundID(),"Add",tuple,named) && stack->quantity()==1 && manager->GetModule(flagHiSlot0),
        "ordinary player inventory fitting can be retried successfully after a rejected split and assembly");
    // Board a real ship with rig slots instead of granting slots to the rookie ship.
    ItemData rifterData(587,ownerID,60014689,flagHangar,1);auto rifter=sItemFactory.SpawnShip(rifterData);
    if (!rifter || !rifter->ChangeSingleton(true,false)) throw std::runtime_error("Cannot assemble the rig fixture ship");
    client->BoardShip(rifter);
    ItemData rigData(31213,ownerID,60014689,flagHangar,1);auto rig=sItemFactory.SpawnModule(rigData);
    if (!rig) throw std::runtime_error("Cannot allocate rig fixture");
    rig->SetAttribute(AttrHP,9,false);
    if (!rig->GetAttributeMap()->Save() || !rifter->AddItemByFlag(flagRigSlot0,rig,client.get())) throw std::runtime_error("Cannot fit rig fixture");
    auto rigManager=rifter->GetModuleManager();auto originalRig=rigManager->GetModule(rig->itemID());
    const auto calibration=rifter->GetAttribute(AttrUpgradeLoad),rigSlots=rifter->GetAttribute(AttrUpgradeSlotsLeft);
    const float scanBonus=rigManager->GetRigScanBonus();
    auto rigBound=new InventoryBound(services,*services.Lookup<InvBrokerService>("invbroker"),rifter,flagCargoHold,ownerID,false);rigBound->NewReference(client.get());
    auto removeRig=[&]() {auto args=new PyTuple(1);args->SetItem(0,new PyInt(rig->itemID()));return TradeCall(services,client.get(),rigBound->GetBoundID(),"DestroyFitting",args);};
    struct TestServerMode {bool before;~TestServerMode(){sConfig.debug.IsTestServer=before;}} restoreMode{sConfig.debug.IsTestServer};
    sConfig.debug.IsTestServer=false;
    if (!sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_item_state BEFORE DELETE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected rig deletion'; END IF; END",rig->itemID())) throw std::runtime_error(error.c_str());
    rejected=!removeRig();ItemStateFault(false);
    Check(rejected && rigManager->GetModule(rig->itemID())==originalRig && rig->locationID()==rifter->itemID() && rig->flag()==flagRigSlot0 &&
        rifter->GetAttribute(AttrUpgradeLoad)==calibration && rifter->GetAttribute(AttrUpgradeSlotsLeft)==rigSlots && rigManager->GetRigScanBonus()==scanBonus,
        "rejected rig destruction through the ordinary inventory RPC restores its module, calibration, slots and scan bonus");
    ItemStateFault(true);
    {
        DBTransaction outer;if (!removeRig()) throw std::runtime_error("Cannot stage rig destruction");ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late rig destruction failure committed");
    }
    ItemStateFault(false);
    std::map<uint32,InventoryItemRef> rigInventory;rifter->GetMyInventory()->GetInventoryMap(rigInventory);
    Check(rigManager->GetModule(rig->itemID())==originalRig && rigInventory.count(rig->itemID()) && rig->flag()==flagRigSlot0 &&
        sItemFactory.GetItemRefFromID(rig->itemID(),false).get()==rig.get() && Stored(rig,1,rifter->itemID()) &&
        rifter->GetAttribute(AttrUpgradeLoad)==calibration && rigManager->GetRigScanBonus()==scanBonus,
        "late rig destruction failure restores the original object, stored item and ship inventory without phantom cargo");
    const bool removed=removeRig();rigInventory.clear();rifter->GetMyInventory()->GetInventoryMap(rigInventory);
    ItemData removedRig;
    Check(removed && !rigManager->GetModule(rig->itemID()) && !rigInventory.count(rig->itemID()) && !sItemFactory.GetItemRefFromID(rig->itemID(),false) &&
        !ItemDB::GetItemData(rig->itemID(),removedRig),"successful rig destruction does not move a deleted rig back into the ship's cargo");
    ItemData onlineData(562,ownerID,60014689,flagHangar,1);auto onlineWeapon=sItemFactory.SpawnModule(onlineData);
    if (!onlineWeapon || !rifter->AddItemByFlag(flagHiSlot0,onlineWeapon,client.get())) throw std::runtime_error("Cannot fit online-state fixture");
    auto onlineModule=rigManager->GetModule(onlineWeapon->itemID());
    auto dogma=new DogmaIMBound(services,*services.Lookup<DogmaIMService>("dogmaIM"),rifter->itemID(),rifter->groupID());dogma->NewReference(client.get());
    auto toggle=[&](bool online) {
        auto args=new PyTuple(2);args->SetItem(0,new PyInt(rifter->itemID()));args->SetItem(1,new PyInt(onlineWeapon->itemID()));
        return TradeCall(services,client.get(),dogma->GetBoundID(),online ? "SetModuleOnline" : "TakeModuleOffline",args);
    };
    auto storedOnline=[&]() {
        DBQueryResult result;DBResultRow row;
        if (!sDatabase.RunQuery(result,"SELECT COALESCE(MAX(COALESCE(valueFloat,valueInt)),0) FROM entity_attributes WHERE itemID=%u AND attributeID=%u",onlineWeapon->itemID(),AttrOnline) || !result.GetRow(row)) throw std::runtime_error("Cannot read online persistence");
        return row.GetDouble(0)!=0;
    };
    Check(toggle(true) && onlineModule->GetModuleState()==Module::State::Online && storedOnline(),
        "ordinary player online RPC durably saves the online state before acknowledging success");
    // Align the unfixed baseline so each rejection case begins from a known stored state.
    if (!onlineWeapon->GetAttributeMap()->Save()) throw std::runtime_error("Cannot save online fixture baseline");
    auto rejectOnlineWrite=[&]() {
        if (!sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_item_state BEFORE UPDATE ON entity_attributes FOR EACH ROW BEGIN IF OLD.itemID=%u AND OLD.attributeID=%u THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected online state'; END IF; END",onlineWeapon->itemID(),AttrOnline)) throw std::runtime_error(error.c_str());
    };
    rejectOnlineWrite();rejected=!toggle(false);ItemStateFault(false);
    Check(rejected && onlineModule->GetModuleState()==Module::State::Online && onlineWeapon->GetAttribute(AttrOnline).get_bool() && storedOnline(),
        "rejected offline attribute write preserves the live module state, cached attribute and stored online state");
    onlineModule->Online();
    ItemStateFault(true);
    {
        DBTransaction outer;if (!toggle(false)) throw std::runtime_error("Cannot stage offline RPC");ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late offline failure committed");
    }
    ItemStateFault(false);
    Check(onlineModule->GetModuleState()==Module::State::Online && onlineWeapon->GetAttribute(AttrOnline).get_bool() && storedOnline(),
        "late offline RPC failure restores the original runtime state and online attribute together");
    onlineModule->Online();
    Check(toggle(false) && onlineModule->GetModuleState()==Module::State::Offline && !storedOnline(),
        "ordinary player offline RPC durably saves the offline state before acknowledging success");
    if (!onlineWeapon->GetAttributeMap()->Save()) throw std::runtime_error("Cannot save offline fixture baseline");
    rejectOnlineWrite();rejected=!toggle(true);ItemStateFault(false);
    Check(rejected && onlineModule->GetModuleState()==Module::State::Offline && !onlineWeapon->GetAttribute(AttrOnline).get_bool() && !storedOnline(),
        "rejected online attribute write preserves the live module state, cached attribute and stored offline state");
    onlineModule->Offline();
    ItemStateFault(true);
    {
        DBTransaction outer;if (!toggle(true)) throw std::runtime_error("Cannot stage online RPC");ItemStateFailure();
        if (outer.Commit()) throw std::runtime_error("Late online failure committed");
    }
    ItemStateFault(false);
    Check(onlineModule->GetModuleState()==Module::State::Offline && !onlineWeapon->GetAttribute(AttrOnline).get_bool() && !storedOnline(),
        "late online RPC failure restores the original runtime state and offline attribute together");
    onlineModule->Offline();
    ItemData batchData(562,ownerID,60014689,flagHangar,2);
    auto batchFirst=sItemFactory.SpawnModule(batchData),batchSecond=sItemFactory.SpawnModule(batchData);
    if (!batchFirst || !batchSecond) throw std::runtime_error("Cannot allocate batch fitting fixture");
    const auto batchCount=TradeScalar("SELECT COUNT(*) FROM entity");
    auto fitBatch=[&]() {
        auto ids=new PyList();ids->AddItem(new PyInt(batchFirst->itemID()));ids->AddItem(new PyInt(batchSecond->itemID()));
        auto args=new PyTuple(2);args->SetItem(0,ids);args->SetItem(1,new PyInt(60014689));
        auto keywords=new PyDict();keywords->SetItemString("qty",new PyInt(1));keywords->SetItemString("flag",new PyInt(flagNone));
        return TradeCall(services,client.get(),rigBound->GetBoundID(),"MultiAdd",args,keywords);
    };
    Query("CREATE TRIGGER core_audit_item_state BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF NEW.typeID=562 AND NEW.flag=29 AND OLD.flag<>29 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected second batch fitting'; END IF; END");
    rejected=!fitBatch();ItemStateFault(false);
    Check(rejected && batchFirst->quantity()==2 && batchSecond->quantity()==2 && Stored(batchFirst,2,60014689) && Stored(batchSecond,2,60014689) &&
        !rigManager->GetModule(flagHiSlot1) && !rigManager->GetModule(flagHiSlot2) && TradeScalar("SELECT COUNT(*) FROM entity")==batchCount,
        "rejected second batch fitting restores both source stacks and removes every provisional module and split item");
    const bool fittedBatch=fitBatch();
    auto firstBatchModule=rigManager->GetModule(flagHiSlot1),secondBatchModule=rigManager->GetModule(flagHiSlot2);
    Check(fittedBatch && firstBatchModule && secondBatchModule && batchFirst->quantity()==1 && batchSecond->quantity()==1 &&
        TradeScalar("SELECT COUNT(*) FROM entity")==batchCount+2,"ordinary player batch fitting retries both stack splits and fitted modules successfully");
    if (!firstBatchModule || !secondBatchModule) throw std::runtime_error("Batch retry did not fit both weapons");
    rigManager->Online(firstBatchModule->itemID());rigManager->Online(secondBatchModule->itemID());
    rifter->LinkWeapon(firstBatchModule->itemID(),secondBatchModule->itemID());
    ItemData batchChargeData(222,ownerID,60014689,flagHangar,45);auto batchCharge=sItemFactory.SpawnItem(batchChargeData);
    rigManager->LoadCharge(batchCharge,flagHiSlot1);
    const auto totalBatchAmmunition=TradeScalar("SELECT SUM(quantity) FROM entity WHERE ownerID=90000000 AND typeID=222");
    auto strip=[&]() {return TradeCall(services,client.get(),rigBound->GetBoundID(),"StripFitting",new PyTuple(0));};
    if (!sDatabase.RunQuery(error,"CREATE TRIGGER core_audit_item_state BEFORE UPDATE ON entity FOR EACH ROW BEGIN IF OLD.itemID=%u AND NEW.flag=4 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='audit rejected last strip fitting move'; END IF; END",secondBatchModule->itemID())) throw std::runtime_error(error.c_str());
    rejected=!strip();ItemStateFault(false);
    Check(rejected && rigManager->GetModule(flagHiSlot0)==onlineModule && rigManager->GetModule(flagHiSlot1)==firstBatchModule &&
        rigManager->GetModule(flagHiSlot2)==secondBatchModule && firstBatchModule->IsLoaded() && firstBatchModule->GetLoadedChargeRef().get()==batchCharge.get() &&
        batchCharge->quantity()==45 && batchCharge->flag()==flagHiSlot1 && firstBatchModule->isOnline() && secondBatchModule->isOnline() &&
        firstBatchModule->IsLinked() && secondBatchModule->IsLinked() && rifter->GetLinkedCount(firstBatchModule)==2 &&
        TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups")==1,
        "failed strip fitting restores every original module, online state, loaded ammunition and linked weapon group");
    Check(strip() && !rigManager->GetModule(flagHiSlot0) && !rigManager->GetModule(flagHiSlot1) && !rigManager->GetModule(flagHiSlot2) &&
        TradeScalar("SELECT SUM(quantity) FROM entity WHERE ownerID=90000000 AND typeID=222")==totalBatchAmmunition &&
        TradeScalar("SELECT SUM(quantity) FROM entity WHERE ownerID=90000000 AND typeID=222 AND locationID=60014689 AND flag=4")==totalBatchAmmunition &&
        TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups")==0,"successful strip fitting preserves all ammunition and clears durable weapon groups");
}
void FittingCrashAudit(const char* mode,EVEServiceManager& services) {
    TradeFixtureGuard();
    const bool recovery=std::strstr(mode,"recover")!=nullptr;
    const bool committed=std::strstr(mode,"committed")!=nullptr;
    const bool unfit=std::strstr(mode,"unfit")!=nullptr;
    if (!recovery) {
        TradeFixturePrepare();
        auto client=TradeClient(services,ownerID,1);auto ship=client->GetShip();
        sItemFactory.GetItemRef(140000014)->Move(60014689,flagHangar,false);
        sItemFactory.GetItemRef(140000015)->Move(60014689,flagHangar,false);
        ItemData weaponData(562,ownerID,60014689,flagHangar,1);auto weapon=sItemFactory.SpawnModule(weaponData);
        ItemData chargeData(222,ownerID,60014689,flagHangar,40);auto charge=sItemFactory.SpawnItem(chargeData);
        if (!weapon || !charge) throw std::runtime_error("Cannot allocate restart fitting fixture");
        weapon->SetAttribute(AttrOnline,EvilZero,false);
        if (!weapon->GetAttributeMap()->Save()) throw std::runtime_error("Cannot persist initial online fixture attribute");
        DBerror error;
        if (!sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES ('fitting-weapon',%u),('fitting-charge',%u)",weapon->itemID(),charge->itemID()))
            throw std::runtime_error(error.c_str());
        auto dogma=new DogmaIMBound(services,*services.Lookup<DogmaIMService>("dogmaIM"),ship->itemID(),ship->groupID());dogma->NewReference(client.get());
        auto fit=[&]() {
            if (!ship->AddItemByFlag(flagHiSlot0,weapon,client.get())) throw std::runtime_error("Cannot fit restart fixture");
            auto args=new PyTuple(2);args->SetItem(0,new PyInt(ship->itemID()));args->SetItem(1,new PyInt(weapon->itemID()));
            if (!TradeCall(services,client.get(),dogma->GetBoundID(),"SetModuleOnline",args) || !ship->AddItemByFlag(flagHiSlot0,charge,client.get()))
                throw std::runtime_error("Cannot online and load restart fixture");
        };
        if (unfit) fit();
        DBTransaction outer;
        if (unfit) weapon->Move(60014689,flagHangar,false);
        else fit();
        std::printf("CORE_AUDIT FITTING_READY\n");std::fflush(stdout);
        if (!committed) Query("DO SLEEP(45)");
        if (!outer.Commit()) throw std::runtime_error("Cannot commit restart fitting");
        std::printf("CORE_AUDIT FITTING_COMMITTED\n");std::fflush(stdout);std::_Exit(86);
    }
    const uint32 weaponID=static_cast<uint32>(TradeScalar("SELECT itemID FROM coreAuditFixture WHERE label='fitting-weapon'"));
    const uint32 chargeID=static_cast<uint32>(TradeScalar("SELECT itemID FROM coreAuditFixture WHERE label='fitting-charge'"));
    auto client=TradeClient(services,ownerID,1);auto ship=client->GetShip();
    auto weapon=sItemFactory.GetItemRef(weaponID),charge=sItemFactory.GetItemRef(chargeID);
    const bool fitted=unfit ? !committed : committed;
    auto module=ship->GetModuleManager()->GetModule(weaponID);
    ItemData storedWeapon,storedCharge;
    Check(weapon && ItemDB::GetItemData(weaponID,storedWeapon) && weapon->ownerID()==ownerID && storedWeapon.ownerID==ownerID &&
        weapon->locationID()==(fitted ? ship->itemID() : 60014689) && storedWeapon.locationID==weapon->locationID() &&
        weapon->flag()==(fitted ? flagHiSlot0 : flagHangar) && storedWeapon.flag==weapon->flag(),
        "fresh process and ordinary player login reload the complete fitted or unfitted weapon outcome");
    Check(charge && ItemDB::GetItemData(chargeID,storedCharge) && charge->ownerID()==ownerID && charge->quantity()==40 && storedCharge.quantity==40 &&
        charge->locationID()==(fitted ? ship->itemID() : 60014689) && storedCharge.locationID==charge->locationID() &&
        charge->flag()==(fitted ? flagHiSlot0 : flagHangar) && storedCharge.flag==charge->flag(),
        "abrupt fitting termination preserves all forty actual ammunition units and their matching sublocation");
    Check(fitted ? module && module->isOnline() && module->IsLoaded() && module->GetLoadedChargeRef().get()==charge.get() : !module,
        "fresh login restores the matching module lifetime, online state and loaded charge reference");
    DBQueryResult onlineResult;DBResultRow onlineRow;
    Check(weapon && weapon->GetAttribute(AttrOnline).get_bool()==fitted &&
        sDatabase.RunQuery(onlineResult,"SELECT COALESCE(MAX(COALESCE(valueFloat,valueInt)),0) FROM entity_attributes WHERE itemID=%u AND attributeID=%u",weaponID,AttrOnline) &&
        onlineResult.GetRow(onlineRow) && (onlineRow.GetDouble(0)!=0)==fitted,
        "fresh process reload preserves the committed ordinary online RPC or unfitting state without a later save");
    Check(client->GetBalance()==5000 && TradeScalar("SELECT balance FROM chrCharacters WHERE characterID=90000000")==5000,
        "fitting, termination and fresh login preserve the player wallet");
    Check(TradeScalar("SELECT COUNT(*) FROM entity_attributes a LEFT JOIN entity e ON e.itemID=a.itemID WHERE e.itemID IS NULL")==0 &&
        TradeScalar("SELECT COUNT(*) FROM shipWeaponGroups WHERE shipID=140000013")==0,"fitting restart leaves no orphan attributes or stale weapon groups");
    client.reset();Query("DELETE FROM coreAuditFixture WHERE label LIKE 'fitting-%'");TradeFixtureRestore();
}

void ItemStateAudit(const char* mode,EVEServiceManager& services) {
    TradeFixturePrepare();
    try {
        if (std::strstr(mode,"blueprint")) BlueprintStateAudit();
        else if (std::strstr(mode,"typed-spawns")) TypedSpawnAudit();
        else CompoundFittingAudit(services);
        TradeFixtureRestore();
    } catch (...) {
        ItemStateFault(false);Query("DROP TRIGGER IF EXISTS core_audit_blueprint_cleanup");TradeFixtureRestore();throw;
    }
}
void ItemStateCrashAudit(const char* mode,EVEServiceManager& services) {
    TradeFixtureGuard();
    const bool recovery=std::strstr(mode,"recover")!=nullptr;
    const bool committed=std::strstr(mode,"committed")!=nullptr;
    if (!recovery) {
        TradeFixturePrepare();
        Query("INSERT INTO coreAuditFixture(label,itemID) SELECT 'item-state-cutoff',MAX(itemID) FROM entity");
        DBTransaction outer;
        for (unsigned kind=0;kind<4;++kind) {
            ItemData data(kind==0 ? 606 : kind==1 ? 3300 : kind==2 ? 3651 : 3297,ownerID,60014689,flagHangar,1);
            InventoryItemRef item;
            if (kind==0) item=sItemFactory.SpawnShip(data);
            if (kind==1) item=sItemFactory.SpawnSkill(data);
            if (kind==2) item=sItemFactory.SpawnModule(data);
            if (kind==3) item=sItemFactory.SpawnCargoContainer(data);
            DBerror error;
            if (!item || !sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES ('item-state-kind-%u',%u)",kind,item->itemID()))
                throw std::runtime_error("Cannot stage restart item identities");
        }
        ItemData data(691,ownerID,60014689,flagHangar,2);EvERam::bpData metadata={};metadata.copy=true;metadata.mLevel=11;metadata.pLevel=12;metadata.runs=13;
        auto blueprint=Blueprint::Spawn(data,metadata);DBerror error;
        if (!blueprint || !sDatabase.RunQuery(error,"INSERT INTO coreAuditFixture(label,itemID) VALUES ('item-state-blueprint',%u)",blueprint->itemID()))
            throw std::runtime_error("Cannot stage restart blueprint identity");
        blueprint->SetCopy(false);blueprint->SetMLevel(4);blueprint->SetPLevel(5);blueprint->SetRuns(6);
        std::printf("CORE_AUDIT ITEM_STATE_READY\n");std::fflush(stdout);
        if (!committed) Query("DO SLEEP(45)");
        if (!outer.Commit()) throw std::runtime_error("Cannot commit restart item fixture");
        std::printf("CORE_AUDIT ITEM_STATE_COMMITTED\n");std::fflush(stdout);std::_Exit(86);
    }
    const uint32 cutoff=static_cast<uint32>(TradeScalar("SELECT itemID FROM coreAuditFixture WHERE label='item-state-cutoff'"));
    DBQueryResult result;DBResultRow row;unsigned found=0;
    if (!sDatabase.RunQuery(result,"SELECT label,itemID FROM coreAuditFixture WHERE label LIKE 'item-state-kind-%%' OR label='item-state-blueprint' ORDER BY label"))
        throw std::runtime_error("Cannot load restart item fixture");
    std::vector<std::pair<std::string,uint32>> items;
    while (result.GetRow(row)) items.emplace_back(row.GetText(0),row.GetUInt(1));
    auto client=TradeClient(services,ownerID,1);
    for (const auto& fixture:items) {
        auto item=sItemFactory.GetItemRef(fixture.second);
        Check(item && item->ownerID()==ownerID && item->locationID()==60014689 && item->flag()==flagHangar && item->quantity()==(fixture.first=="item-state-blueprint" ? 2 : 1),
            "fresh process and player login reload a committed typed item with its correct owner, quantity and location");
        ++found;
        if (fixture.first=="item-state-blueprint") {
            auto blueprint=BlueprintRef::StaticCast(item);
            Check(!blueprint->copy() && blueprint->mLevel()==4 && blueprint->pLevel()==5 && blueprint->runs()==6 && BlueprintStored(blueprint,false,4,5,6),
                "fresh process reloads the complete committed blueprint metadata");
        }
    }
    Check(found==(committed ? 5u : 0u),"fresh process sees all committed item identities or none of the interrupted allocation");
    if (!sDatabase.RunQuery(result,"SELECT COUNT(*) FROM entity WHERE itemID>%u",cutoff) || !result.GetRow(row)) throw std::runtime_error("Cannot count recovered item allocations");
    Check(row.GetUInt(0)==(committed ? 5u : 0u),"interrupted item creation leaves no partial entity rows");
    Check(TradeScalar("SELECT COUNT(*) FROM entity_attributes a LEFT JOIN entity e ON e.itemID=a.itemID WHERE e.itemID IS NULL")==0 &&
        TradeScalar("SELECT COUNT(*) FROM invBlueprints b LEFT JOIN entity e ON e.itemID=b.itemID WHERE e.itemID IS NULL")==0,
        "restart leaves no orphan attributes or blueprint metadata");
    Check(client->GetBalance()==5000,"item creation and restart preserve the player's wallet balance");
    client.reset();Query("DELETE FROM coreAuditFixture WHERE label LIKE 'item-state-%'");TradeFixtureRestore();
}
