/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Luck
*/
#include "EVEServerPCH.h"

ModuleManager::ModuleManager(Client *pilot)
: m_pilot(pilot)  //Accessing m_pilot here will cause serious problems
{
	memset(m_modules, 0, sizeof(m_modules));
}

//De constructor for the module manager
//iterates through and destroys all modules
ModuleManager::~ModuleManager() {
	
	uint8 r;
	for(r = 0; r < MAX_MODULE_COUNT; r++) {
		delete m_modules[r];
	}
}

void ModuleManager::OnlineAll()
{
	std::map<uint32, uint8> ::iterator p;
	for(p = m_moduleByID.begin(); p!=m_moduleByID.end(); ++p)
	{
		ShipModule *mod = m_modules[p->second];
		if(mod->GetState() == 0)
			mod->Activate("online",m_pilot->GetAccountID(),0);
	}
}

void ModuleManager::RepairModules()
{
	std::map<uint32, uint8> ::iterator p;
	for(p = m_moduleByID.begin(); p!=m_moduleByID.end(); ++p)
	{
		ShipModule *mod = m_modules[p->second];
        mod->item()->SetAttribute(AttrDamage, 0.0);
	}
}

void ModuleManager::UnloadModule(uint32 itemID)
{
	std::map<uint32, uint8> ::iterator p;

	p=m_moduleByID.begin();
	while(p!=m_moduleByID.end())
	{
		ShipModule *mod = m_modules[p->second];
		
		if(mod->item()->typeID()==itemID)
		{
			mod->Deactivate("online");

			m_modules[p->second]->item()->Move(m_pilot->GetStationID(), flagHangar, true);
			m_moduleByID.erase(p);
			p=m_moduleByID.begin();
		}
		else
			++p;
	}
}

void ModuleManager::UnloadAllModules()
{
	ShipModule *mod;

	std::map<uint32, uint8> ::iterator p;
	for(p = m_moduleByID.begin(); p!=m_moduleByID.end(); ++p)
	{
		mod = m_modules[p->second];
		mod->Deactivate("online");
		m_modules[p->second]->item()->Move(m_pilot->GetStationID(), flagHangar, true);
        if( m_modules[p->second]->item()->GetAttribute(AttrMassAddition).get_float() != 0 )
        {
            m_pilot->GetShip()->SetAttribute
            (
                AttrMass,
                (m_pilot->GetShip()->GetAttribute(AttrMass) - m_pilot->GetShip()->GetAttribute(AttrMassAddition))
            );
        }
	}
	// Reset CPU and Power load
    m_pilot->GetShip()->SetAttribute( AttrCpuLoad, 0 );
    m_pilot->GetShip()->SetAttribute( AttrPowerLoad, 0 );

	m_moduleByID.clear();
}

//Basic module manager process
//essentially goes through all fitted modules, and processes them individually
void ModuleManager::Process() {
	uint8 r;
	ShipModule **cur = m_modules;
	for(r = 0; r < MAX_MODULE_COUNT; r++, cur++) {
		if(*cur == NULL)
			continue;
		(*cur)->Process();
	}
}

void ModuleManager::UpdateModules() {
	
	//This leaves us somewhat vulnerable to hacking, but oh well...
	ShipRef ship = m_pilot->GetShip();
	
	sLog.Debug("ModuleMgr", "%s: Refreshing modules for ship %s (%u).", m_pilot->GetName(), ship->itemName().c_str(), ship->itemID());
	
	//TODO: iterate through all slots and fill in m_modules
	//checking for inconsistencies as to not re-create 
	// module entries which already exist.
	m_moduleByID.clear();
	
	uint8 slot;
	for(slot = 0; slot < MAX_MODULE_COUNT; slot++) {
		EVEItemFlags flag = SlotToFlag(slot);
		
		//apparently they thought it was a good idea to put the module AND its 
		// charges in the module slot of the main ship, instead of storing the
		// charges inside the module itself. This makes our life much more
		// difficult because we need to figure out what is what.
		std::vector<InventoryItemRef> items;
		if(ship->FindByFlag(flag, items) == 0) {
			SafeDelete( m_modules[slot] );
			continue;	//nothing in that slot..
		}

		//ok, we have the list of crap in this slot, figure out what is the actual module.
		InventoryItemRef module;
		InventoryItemRef charge;

		if(items.size() == 1)
			module = items[0];
		else if(items.size() == 2)
		{
			//could be done better
			bool valid = true;
			if(items[0]->categoryID() == EVEDB::invCategories::Module)
			{
				if(items[1]->categoryID() == EVEDB::invCategories::Charge)
				{
					module = items[0];
					charge = items[1];
				}
				else
				{
					sLog.Error("ModuleMgr","%s: Unexpected item category %d in slot %d (wanted charge)", m_pilot->GetName(), items[1]->categoryID(), flag);
					valid = false;
				}
			}
			else if(items[0]->categoryID() == EVEDB::invCategories::Charge)
			{
				if(items[1]->categoryID() == EVEDB::invCategories::Module)
				{
					module = items[1];
					charge = items[0];
				}
				else
				{
					sLog.Error("ModuleMgr","%s: Unexpected item category %d in slot %d (wanted module)", m_pilot->GetName(), items[1]->categoryID(), flag);
					valid = false;
				}
			}
			else
			{ 
				sLog.Error("ModuleMgr","%s: Unexpected item category %d in slot %d (wanted module or charge)", m_pilot->GetName(), items[0]->categoryID(), flag);
				valid = false;
			}

			if(!valid)
			{
				//remove offending module
				SafeDelete( m_modules[slot] );
				continue;
			}
		}
		else
		{
			//no idea why this would happen.. but its maybe possible..
			sLog.Error("ModuleMgr","%s: More than two items in a module slot (%d). We do not understand this yet!", m_pilot->GetName(), flag);
			//ignore for now...
			SafeDelete( m_modules[slot] );
			continue;
		}
		
		if( m_modules[slot] != NULL )
		{
			if( m_modules[slot]->item() == module )
			{
				if( m_modules[slot]->charge() == charge )
				{
					sLog.Debug("ModuleMgr","%s: Item %s (%u) is still in slot %d.", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot);
				}
				else
				{
					sLog.Debug("ModuleMgr","%s: Item %s (%u) is still in slot %d, but has a new charge %s (%u).", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot, ( !charge ? "None" : charge->itemName().c_str() ), ( !charge ? 0 : charge->itemID() ));
					m_modules[slot]->ChangeCharge(charge);
				}
				//no change...
				m_moduleByID[module->itemID()] = slot;
				continue;
			}

			SafeDelete( m_modules[slot] );
		}
		sLog.Debug("ModuleMgr","%s: Item %s (%u) is now in slot %d. Using charge %s (%u)", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot, ( !charge ? "None" : charge->itemName().c_str() ), ( !charge ? 0 : charge->itemID() ));
		m_modules[slot] = ShipModule::CreateModule(m_pilot, module, charge);
		m_moduleByID[module->itemID()] = slot;
	}

	//Rig Handler
	//this is substantially easier because there are no charges to handle, just one item
	uint8 rSlot;
	for(rSlot = 92; rSlot < 100; rSlot++) { //92 - rigSlot0 //100 rigSlot7 + 1
		EVEItemFlags rFlag = (EVEItemFlags)rSlot;
		
		uint32 slotIndex = rSlot - 92; //rigSlot0
		InventoryItemRef item;
		if( !ship->FindSingleByFlag(rFlag, item) ) {
			sLog.Debug("ModuleMgr","FindSingleByFlag returned no results");
			continue;
		}

		if ( m_rigByID.find(item->itemID()) == m_rigByID.end()  ) {
			m_rigByID.insert( std::make_pair( item->itemID(), slotIndex ) );
			m_rigs[slotIndex] = ShipModule::CreateRig(m_pilot, item);
		}
	}
}

int ModuleManager::Activate(uint32 itemID, const std::string &effectName, uint32 target, uint32 repeat) {

	std::map<uint32, uint8>::const_iterator res;
	res = m_moduleByID.find(itemID);

	//check if you have the module on your ship
	if(res == m_moduleByID.end()) {

		//log error
		sLog.Error("ModuleMgr","Activate: %s: failed to find module %u", m_pilot->GetName(), itemID);
		return 0;
	}

	//grab the actual module
	ShipModule *mod = m_modules[res->second];
	if(mod == NULL) {

		//if this happens we have bigger problems than your module not working
		sLog.Error("ModuleMgr","%s: failed to activate module %u", m_pilot->GetName(), itemID);
		return 0;
	}

	//activate it and return that we did to the client
	return mod->Activate(effectName, target, repeat);
}

int ModuleManager::Upgrade(uint32 itemID) {
	
	std::map<uint32, uint8>::const_iterator res;
	res = m_rigByID.find(itemID);
	//check if you have the module on your ship
	if(res == m_rigByID.end()) {

		//log error
		sLog.Error("ModuleManager","Upgrade: %s: failed to find rig %u", m_pilot->GetName(), itemID);
		return 0;
	}

	//grab the actual module
	ShipModule *mod = m_rigs[res->second];
	if(mod == NULL) {

		//if this happens we have bigger problems than your module not working
		sLog.Error("ModuleMgr","%s: failed to integrate rig %u", m_pilot->GetName(), itemID);
		return 0;
	}

	return mod->Upgrade();
}

void ModuleManager::Downgrade(uint32 itemID) {
	
	std::map<uint32, uint8>::const_iterator res;
	res = m_rigByID.find(itemID);
	//check if you have the module on your ship
	if(res == m_rigByID.end()) {

		//log error
		sLog.Error("ModuleMgr","Downgrade: %s: failed to find rig %u", m_pilot->GetName(), itemID);
		return;
	}

	//grab the actual module
	ShipModule *mod = m_rigs[res->second];
	if(mod == NULL) {

		//if this happens we have bigger problems than your module not working
		sLog.Error("ModuleMgr","%s: failed to remove rig %u", m_pilot->GetName(), itemID);
		return;
	}

	mod->Downgrade();
}

void ModuleManager::Deactivate(uint32 itemID, const std::string &effectName) {

	std::map<uint32, uint8>::const_iterator res;
	res = m_moduleByID.find(itemID);

	//check if you have the module on your ship
	if(res == m_moduleByID.end()) {

		//log error
		sLog.Error("ModuleMgr","Deactivate: %s: failed to find module %u.", m_pilot->GetName(), itemID);
		return;
	}

	//grab the actual module
	ShipModule *mod = m_modules[res->second];

	if(mod == NULL) {
		//if this happens we have bigger problems than your module not working
		sLog.Error("ModuleMgr","%s: failed to deactivate module %u", m_pilot->GetName(), itemID);
		return;
	}

	//deactivate it
	mod->Deactivate(effectName);
}

void ModuleManager::ReplaceCharges(EVEItemFlags flag, InventoryItemRef new_charge) {

	uint8 slot = FlagToSlot(flag);
	InventoryItemRef charge = m_modules[slot]->charge();
	
	//check that the charge exists
	if( charge ) {

		//put the previous charge into the cargo hold.
		charge->Move(m_pilot->GetShipID(), flagCargoHold);
	}

	// put new charge into the module
	new_charge->Move(m_pilot->GetShipID(), flag);

	//make it singleton
	new_charge->ChangeSingleton(true);
	
	//call change charge to send update
	m_modules[slot]->ChangeCharge(new_charge);
}

void ModuleManager::DeactivateAllModules()
{
    std::map<uint32, uint8>::const_iterator res = m_moduleByID.begin();
    for(; res != m_moduleByID.end(); res++ )
    {
        ShipModule *mod = m_modules[res->second];
        if( mod != NULL )
        {
            try
            {
				//mod->Deactivate();
            }
            catch( char * str )
            {
				sLog.Debug("ModuleMgr", "Error (%s)", str);
            }
        }
    }
}

ShipModule::ShipModule(Client *pilot, InventoryItemRef self, InventoryItemRef charge_)
: m_state(Offline),
  m_pilot(pilot),
  m_item(self),
  m_charge(charge_),
  m_activationTimer(0),
  m_effectTimer(0),
  m_effectName("none"),
  m_repeatCount(0),
  m_target(0),
  m_activationInterval(0)
{
	//activate it if it's a new module
	// TODO: update to use GetAttribute(AttrOnline) or something like that
	/*(if( !m_item->isOnline() )
		Activate();
	else
		m_state = Online;*/
}

ShipModule::ShipModule(Client *pilot, InventoryItemRef self)
: m_state(Offline),
  m_pilot(pilot),
  m_item(self),
  m_activationTimer(0),
  m_effectTimer(0),
  m_effectName("none"),
  m_repeatCount(1000),
  m_target(0)
{

	sLog.Debug("ModuleMgr", "Called Upgrade Ship stub");

	//activate it if it's a new rig
	// TODO: update to use GetAttribute(AttrOnline) or something like that
	/*if( !m_item->isOnline() )
		Upgrade();
	else
		m_state = Online;*/
}

ShipModule::~ShipModule()
{
}

ShipModule *ShipModule::CreateModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_) {

	//This module is then built out to a specific type
	return new ShipModule(owner, self, charge_);
}

ShipModule *ShipModule::CreateRig(Client *owner, InventoryItemRef self) {
	//These modules are all extremely similar, and simple
	return new ShipModule(owner, self);
}

void ShipModule::ChangeCharge(InventoryItemRef new_charge) {

	//You can only change charges while not active, but must be online
	//check if online
	if(m_state == Online ) {
		//check if active
		if( m_state == Active || m_state == Overloaded ) {
			//Tell pilot he needs to deactive this weapon before changing the charge
			m_pilot->SendNotifyMsg( "You must deactivate the weapon before changing the charge" );
		} else {
			//change charges
			m_charge = new_charge;
		}
	}
}

void ShipModule::Process() {
	
	switch(m_state)
    {
		case Active:
			if( m_activationTimer.Check() )
			break;
		case Overloaded:
			//not implemented yet
            sLog.Error( "ModuleManager ShipModule::Process()", "Module Overload state NOT supported at this time." );
			break;

	}
}

int ShipModule::Activate(const std::string &effectName, uint32 target, uint32 repeat) {

	m_target = target;
	m_effectName = effectName;
	m_repeatCount = repeat;

	//build effect here

	if( ValidateEffect(true) ) {
		DoEffect(true);
		return 1;
	} else {
		return 0;
	}
}

void ShipModule::Deactivate(const std::string &effectName) {

	m_effectName = effectName;

	if( ValidateEffect(false) ) {
		DoEffect(false);
	}
}

int ShipModule::Upgrade() {

    if( (m_item->GetAttribute(AttrUpgradeCost) + m_pilot->GetShip()->GetAttribute(AttrUpgradeLoad))
        > m_pilot->GetShip()->GetAttribute(AttrUpgradeCapacity) )
		    sLog.Error( "ModuleManager ShipModule::Upgrade()","Invalid ship upgrade. Check Ship::ValidateAddItem function");
	
	m_item->PutOnline();
	m_state = Online;

	DoUpgradeLoad(true,false);

	DoPassiveEffects(true,false);
	return 1;
}

void ShipModule::Downgrade() {

	//TODO: check that modules exists
	DoUpgradeLoad(false,false);

	DoPassiveEffects(false,false);
}

bool ShipModule::ValidateEffect(bool activate) {

	//only really 2 types of effects we have to worry about.  Online/Offline, and everything else
	if( activate ) {
		if( m_effectName == "online" ) {
			if( ValidateOnline() )
				return true;
		} else {
			if( ValidateActive() )
				return true;
		}
	} else {
		if( m_effectName == "online" ) {
			if( ValidateOffline() )
				return true;
		} else {
			if( ValidateDeactive() )
				return true;
		}
	}
	return false;
}

bool ShipModule::ValidateOnline() {

	//check for cpu
    if( m_item->GetAttribute(AttrCpu) + m_pilot->GetShip()->GetAttribute(AttrCpuLoad) > m_pilot->GetShip()->GetAttribute(AttrCpuOutput) ) {
		m_pilot->SendNotifyMsg("You do not have enough available cpu for this");
		return false;
	}

	//check for powergrid
    if (m_item->GetAttribute(AttrPower) + m_pilot->GetShip()->GetAttribute(AttrPowerLoad) > m_pilot->GetShip()->GetAttribute(AttrPowerOutput) ) {
		m_pilot->SendNotifyMsg("You do not have enough available power for this");
		return false;
	}

	//if in space, check for cap
	if( m_pilot->IsInSpace() ) {
        if( !(m_pilot->GetShip()->GetAttribute(AttrCharge) == m_pilot->GetShip()->GetAttribute(AttrCapacitorCapacity)) ){
			m_pilot->SendNotifyMsg("You do not have enough available capacitor charge for this");
			return false;
		}
	}

	//check maxgroupsOnline attribute (id 978) 0 = no limit, 1 = 1

	//if we pass everything, return true
	return true;

}

bool ShipModule::ValidateOffline() {

	//for now, just return true, till i figure out what needs to go in here
	if( m_state != Offline )
		return true;
	
	return false;
}

bool ShipModule::ValidateActive() {

	//check that module is online
	if( !m_state == Online )
		return false;

	//don't know when this would happen, but just in case
	if( !m_pilot->IsInSpace() )
			return false;

	//check for activation energy
    double capacitorNeed = m_item->GetAttribute(AttrCapacitorNeed).get_float();
    double charge = m_pilot->GetShip()->GetAttribute(AttrCharge).get_float();
	if( capacitorNeed > charge )
    {
		m_pilot->SendNotifyMsg("You do not have enough capacitor to activate this module");
		return false;
	}

	//check reactivation delay
	//check maxGroupActive 0 = no limit, 1 = only 1;
	//check disallowRepeatingActivation

	return true;
}

bool ShipModule::ValidateDeactive(){

	if( m_activationTimer.Check() )
		return true;

	ChangeMState(Deactivating);
	return false;
}

void ShipModule::DoEffect(bool active) {

	if( active ) {
		if(m_effectName == "online") {
			//change state
			ChangeMState(Online);
			m_item->PutOnline();

			//consume cpu and powergrid
            m_pilot->GetShip()->SetAttribute( AttrCpuLoad, (m_pilot->GetShip()->GetAttribute(AttrCpuLoad) + m_item->GetAttribute(AttrCpu)) );
            m_pilot->GetShip()->SetAttribute( AttrPowerLoad, (m_pilot->GetShip()->GetAttribute(AttrPowerLoad) + m_item->GetAttribute(AttrPower)) );

			//if in space, consume all capacitor
			if( m_pilot->IsInSpace() )
				m_pilot->GetShip()->SetAttribute(AttrCharge,0);

			//if the module is passive, do passive effects
            if( m_item->GetAttribute(AttrCapacitorNeed).get_float() == 0 ){
				DoPassiveEffects(true,false);
			} else {
				DoActiveModulePassiveEffects(true,false);
			}
		} else {
			sLog.Debug("ModuleManager ShipModule::DoEffect()","Called Activate Effect stub");
			//don't forget to consume ammo Attr 713
			//jumpDelayDuration timer public interface needed
		}
	} else {
		if( m_effectName == "online") {
			//change state
			ChangeMState(Offline);
			m_item->PutOffline();
	
			//give back cpu and powergrid
			m_pilot->GetShip()->SetAttribute( AttrCpuLoad, (m_pilot->GetShip()->GetAttribute(AttrCpuLoad) - m_item->GetAttribute(AttrCpu)) );
			m_pilot->GetShip()->SetAttribute( AttrPowerLoad, (m_pilot->GetShip()->GetAttribute(AttrPowerLoad) - m_item->GetAttribute(AttrPower)) );

			//if the module is passive, do passive effects
			if( m_item->GetAttribute(AttrCapacitorNeed).get_float() == 0 ){
				DoPassiveEffects(false,false);
			} else {
				DoActiveModulePassiveEffects(false,false);
			}
		} else {
			sLog.Debug("ModuleManager ShipModule::DoEffect()", "Called Deactivate Effect stub");
		}
	}
}

/* change stuff
    Notify_OnModuleAttributeChange omac;
    omac.ownerID = m_ownerID;
    omac.itemKey = m_itemID;
    omac.attributeID = ItemAttributeMgr::Attr_isOnline;
    omac.time = Win32TimeNow();
    omac.newValue = new PyInt(newval?1:0);
    omac.oldValue = new PyInt(newval?0:1);   //hack... should use old, but its not cooperating today.
*/

void ShipModule::DoPassiveEffects(bool add, bool notify) {
/*	
	//this is extremely slow compared to the switch, however, as the effect "online" tells us little about what actually happens, this is how it's done
	 DoArmorHPBonus(add,notify);
	 DoArmorHPBonusAdd(add,notify);
	 DoArmorHPMultiplier(add,notify);
	 DoArmorEmDamageResistanceBonus(add,notify);
	 DoArmorExplosiveDamageResistanceBonus(add,notify);
	 DoArmorKineticDamageResistanceBonus(add,notify);
	 DoArmorThermalDamageResistanceBonu(add,notify);
	 DoShieldCapacity(add,notify);
	 DoShieldCapacityBonus(add,notify);
	 DoShieldCapacityMultiplier(add,notify);
	 DoShieldEmDamageResistanceBonus(add,notify);
	 DoShieldExplosiveDamageResistanceBonus(add,notify);
	 DoShieldKineticDamageResistanceBonus(add,notify);
	 DoShieldThermalDamageResistanceBonu(add,notify);
	 DoShieldRechargeRateMultiplier(add,notify);
	 DoHullHpBonus(add,notify);
	 DoStructureHPMultiplier(add,notify);
	 DoStructureEmDamageResistanceBonus(add,notify);
	 DoStructureExplosiveDamageResistanceBonus(add,notify);
	 DoStructureKineticDamageResistanceBonus(add,notify);
	 DoStructureThermalDamageResistanceBonu(add,notify);
*/	 DoImplantBonusVelocity(add,notify);
	 DoSpeedBonus(add,notify);
	 DoMaxVelocityBonus(add,notify);
/*	 DoCpuMultiplier(add,notify);
	 DoPowerIncrease(add,notify);
	 DoPowerOutputBonus(add,notify);
	 DoPowerOutputMultiplier(add,notify);
	 DoCapacitorBonus(add,notify);
	 DoCapacitorCapacityMultiplier(add,notify);
	 DoCapacitorRechargeRateMultiplier(add,notify);
	 DoCargoCapacityMultiplier(add,notify);
	 DoScanStrengthBonus(add,notify);
	 DoScanGravimetricStrengthPercent(add,notify);
	 DoScanLadarStrengthPercent(add,notify);
	 DoScanMagnetometricStrengthPercent(add,notify);
	 DoScanRadarStrengthPercent(add,notify);
	 DoSignatureRadiusBonus(add,notify);
	 DoDroneCapacityBonus(add,notify);
	 DoAgilityMultiplier(add,notify);
	 DoScanResultionBonus(add,notify);
	 DoScanResultionMultiplier(add,notify);
	 DoMaxTargetRangeBonus(add,notify);
	 DoMaxLockedTargetsBonus(add,notify);
	 DoWarpScrambleStrength(add,notify);
	 DoPropulsionFusionStrength(add,notify);
	 DoPropulsionIonStrength(add,notify);
	 DoPropulsionMagpulseStrength(add,notify);
	 DoPropulsionPlasmaStrength(add,notify);
*/
}

void ShipModule::DoActiveModulePassiveEffects(bool add, bool notify) {
/*	
	 DoPassiveArmorEmDamageResistanceBonus(add,notify);
	 DoPassiveArmorExplosiveDamageResistanceBonus(add,notify);
	 DoPassiveArmorKineticDamageResistanceBonus(add,notify);
	 DoPassiveArmorThermalDamageResistanceBonu(add,notify);
	 DoPassiveShieldEmDamageResistanceBonus(add,notify);
	 DoPassiveShieldExplosiveDamageResistanceBonus(add,notify);
	 DoPassiveShieldKineticDamageResistanceBonus(add,notify);
	 DoPassiveShieldThermalDamageResistanceBonus(add,notify);
*/
}

void ShipModule::DoUpgradeLoad(bool add, bool notify) {

	double upgradeLoad;
	double upgradeLoadModifier;
	double newUpgradeLoad;

	if( add ) {
        if( !Equals(m_item->GetAttribute(AttrUpgradeCost).get_float(), 0) ) {
            upgradeLoad = m_pilot->GetShip()->GetAttribute(AttrUpgradeLoad).get_float();
            upgradeLoadModifier = m_item->GetAttribute(AttrUpgradeCost).get_float();
			newUpgradeLoad = upgradeLoad + upgradeLoadModifier;
			m_pilot->GetShip()->SetAttribute( AttrUpgradeCost, newUpgradeLoad );
		}
	} else {
		if( !Equals(m_item->GetAttribute(AttrUpgradeCost).get_float(), 0) ) {
			upgradeLoad = m_pilot->GetShip()->GetAttribute(AttrUpgradeLoad).get_float();
            upgradeLoadModifier = m_item->GetAttribute(AttrUpgradeCost).get_float();
			newUpgradeLoad = upgradeLoad - upgradeLoadModifier;
			m_pilot->GetShip()->SetAttribute( AttrUpgradeCost, newUpgradeLoad );
		}
    }
}

void ShipModule::DoCapacitorNeed(bool startup, bool notify) {

	double capacitorCharge;
	double capacitorChargeModifier;
	double newCapacitorCharge;
	
	//idk if there is a difference, but for now, we'll keep the startup case in here
	if( startup ) {
		//do first time activation charge requirements
        if( !Equals(m_item->GetAttribute(AttrCapacitorNeed).get_float(), 0) ) {
            capacitorCharge = m_pilot->GetShip()->GetAttribute(AttrCharge).get_float();
            capacitorChargeModifier = m_item->GetAttribute(AttrCapacitorNeed).get_float();
			newCapacitorCharge = capacitorCharge - capacitorChargeModifier;
            m_pilot->GetShip()->SetAttribute(AttrCharge, newCapacitorCharge );
		}
	} else {
		//consume normal activation cap charge
		if( !Equals(m_item->GetAttribute(AttrCapacitorNeed).get_float(), 0) ) {
			capacitorCharge = m_pilot->GetShip()->GetAttribute(AttrCharge).get_float();
			capacitorChargeModifier = m_item->GetAttribute(AttrCapacitorNeed).get_float();
			newCapacitorCharge = capacitorCharge - capacitorChargeModifier;
            m_pilot->GetShip()->SetAttribute(AttrCharge, newCapacitorCharge );
		}
	}
	if( notify ) {
		
		Notify_OnModuleAttributeChange omac;
		
		omac.ownerID = m_pilot->GetCharacterID();
		omac.itemKey = m_item->itemID();
        omac.attributeID = AttrCharge;
		omac.time = Win32TimeNow();
		omac.newValue = new PyInt(newCapacitorCharge);
		omac.oldValue = new PyInt(capacitorCharge);
		PyTuple* tmp = omac.Encode();

		m_pilot->SendNotification("OnItemAttributeChange", "clientID", &tmp );
	}
}

#if 0
void ShipModule::DoArmorHPBonus(bool add, bool notify) {
	
	if( !AffectsArmor() )
		return;

	double armorHP;
	double armorHPModifier;
	double newArmorHP;

	if( add ) {
		if( !m_item->armorHpBonus() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonus();
			newArmorHP = armorHP + armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	} else {
		if( !m_item->armorHpBonus() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonus();
			newArmorHP = armorHP - armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	}
}

void ShipModule::DoArmorHPBonusAdd(bool add, bool notify) {

	if( !AffectsArmor() )
		return;

	double armorHP;
	double armorHPModifier;
	double newArmorHP;

	if( add ) {
		if( !m_item->armorHpBonusAdd() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonusAdd();
			newArmorHP = armorHP + armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	} else {
		if( !m_item->armorHpBonusAdd() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonusAdd();
			newArmorHP = armorHP - armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	}
}

void ShipModule::DoArmorHPMultiplier(bool add, bool notify) {

	if( !AffectsArmor() )
		return;

	double armorHP;
	double armorHPModifier;
	double newArmorHP;

	if( add ) {
		if( !Equals(m_item->armorHPMultiplier(), 1) ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHPMultiplier();
			newArmorHP = armorHP * armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	} else {
		if( !Equals(m_item->armorHPMultiplier(), 1) &&
			!Equals(m_item->armorHPMultiplier(), 0)	) 
		{
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHPMultiplier();
			newArmorHP = armorHP / armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	}
}

void ShipModule::DoArmorEmDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsArmor() )
		return;
	
	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !Equals(m_item->emDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}
	} else {
		if( !Equals(m_item->emDamageResistanceBonus(), 0)		&&
			!Equals(m_item->emDamageResistanceBonus(), -100)	)
		{
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoArmorExplosiveDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsArmor() )
		return;

	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}
	} else {
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0)	 &&
			!Equals(m_item->explosiveDamageResistanceBonus(), -100)	 )
		{
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoArmorKineticDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsArmor() )
		return;

	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}
	} else {
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0)	&&
			!Equals(m_item->kineticDamageResistanceBonus(), -100)  )
		{
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoArmorThermalDamageResistanceBonu(bool add, bool notify) {

	if( !AffectsArmor() )
		return;

	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorThermalDamageResonance();
			resistanceModifier = m_item->thermalDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorThermalDamageResonance( newArmorResonance );
		}
	} else {
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0)		&&
			!Equals(m_item->thermalDamageResistanceBonus(), -100)	)
		{
			armorResonance = m_pilot->GetShip()->armorThermalDamageResonance();
			resistanceModifier = m_item->thermalDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorThermalDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoShieldCapacity(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shieldCapacity;
	double capacityModifier;
	double newShieldCapacity;

	if( add ) {
		if( !m_item->capacity() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacity();
			newShieldCapacity = shieldCapacity + capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	} else {
		if( !m_item->capacity() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacity();
			newShieldCapacity = shieldCapacity - capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	}
}

void ShipModule::DoShieldCapacityBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shieldCapacity;
	double capacityModifier;
	double newShieldCapacity;

	if( add ) {
		if( !m_item->capacityBonus() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacityBonus();
			newShieldCapacity = shieldCapacity + capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	} else {
		if( !m_item->capacityBonus() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacityBonus();
			newShieldCapacity = shieldCapacity - capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	}
}

void ShipModule::DoShieldCapacityMultiplier(bool add, bool notify) {

	if( !AffectsShield() )
		return;
	
	double shieldCapacity;
	double capacityModifier;
	double newShieldCapacity;

	if( add ) {
		if( !Equals(m_item->shieldCapacityMultiplier(), 1) ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->shieldCapacityMultiplier();
			newShieldCapacity = shieldCapacity * capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	} else {
		if( !Equals(m_item->shieldCapacityMultiplier(), 1)	&&
			!Equals(m_item->shieldCapacityMultiplier(), 0)	)
		{
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->shieldCapacityMultiplier();
			newShieldCapacity = shieldCapacity / capacityModifier + 0.8; //round up
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	}
}

void ShipModule::DoShieldEmDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !Equals(m_item->emDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}
	} else {
		if( !Equals(m_item->emDamageResistanceBonus(), 0)		&&
			!Equals(m_item->emDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoShieldExplosiveDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shipResonance;
	double resistanceModifier;
	double newResonance;
	
	if( add ) {
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}
	} else {
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0)	&&
			!Equals(m_item->explosiveDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoShieldKineticDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}
	} else {
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0)		&&
			!Equals(m_item->kineticDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoShieldThermalDamageResistanceBonu(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldThermalDamageResonance();
			resistanceModifier = m_item->thermalDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldThermalDamageResonance( newResonance );
		}
	} else {
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0)		&&
			!Equals(m_item->thermalDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldThermalDamageResonance();
			resistanceModifier = m_item->thermalDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldThermalDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoShieldRechargeRateMultiplier(bool add, bool notify) {

	if( !AffectsShield() )
		return;

	double shieldRechargeRate;
	double rechargeRateModifier;
	double newShieldRechargeRate;

	if( add ) {
		if( !Equals(m_item->shieldRechargeRateMultiplier(), 1) ) {
			shieldRechargeRate = m_pilot->GetShip()->shieldRechargeRate();
			rechargeRateModifier = m_item->shieldRechargeRateMultiplier();
			newShieldRechargeRate = shieldRechargeRate * rechargeRateModifier;
			m_pilot->GetShip()->Set_shieldRechargeRate( ToInt(newShieldRechargeRate) );
		}
	} else {
		if( !Equals(m_item->shieldRechargeRateMultiplier(), 1)	&&
			!Equals(m_item->shieldRechargeRateMultiplier(), 0)	)
		{
			shieldRechargeRate = m_pilot->GetShip()->shieldRechargeRate();
			rechargeRateModifier = m_item->shieldRechargeRateMultiplier();
			newShieldRechargeRate = shieldRechargeRate / rechargeRateModifier;
			m_pilot->GetShip()->Set_shieldRechargeRate( ToInt(newShieldRechargeRate) );
		}
	}
}

void ShipModule::DoHullHpBonus(bool add, bool notify) {

	if( !AffectsHull() )
		return;

	double hullHP;
	double hullHPModifier;
	double newHullHP;

	if( add ) {
		if( !m_item->hullHpBonus() == 0 ) {
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->hullHpBonus();
			newHullHP = hullHP + hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
	} else {
		if( !m_item->hullHpBonus() == 0 ) {
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->hullHpBonus();
			newHullHP = hullHP - hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
	}
}

void ShipModule::DoStructureHPMultiplier(bool add, bool notify) {

	if( !AffectsHull() )
		return;

	double hullHP;
	double hullHPModifier;
	double newHullHP;

	if( add ) {
		if( !Equals(m_item->structureHPMultiplier(), 1) ) {
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->structureHPMultiplier();
			newHullHP = hullHP * hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
	} else {
		if( !Equals(m_item->structureHPMultiplier(), 1)	&&
			!Equals(m_item->structureHPMultiplier(), 0)	)
		{
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->structureHPMultiplier();
			newHullHP = hullHP / hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
	}
}

void ShipModule::DoStructureEmDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsHull() )
		return;

	double hullResonance;
	double hullResonanceModifier;
	double newHullResonance;

	if( add ) {
		if( !Equals(m_item->emDamageResistanceBonus(), 0) ) {
			hullResonance = m_pilot->GetShip()->hullEmDamageResonance();
			hullResonanceModifier = m_item->emDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	} else {
		if( !Equals(m_item->emDamageResistanceBonus(), 0)		&&
			!Equals(m_item->emDamageResistanceBonus(), -100)	) 
		{
			hullResonance = m_pilot->GetShip()->hullEmDamageResonance();
			hullResonanceModifier = m_item->emDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	}
}

void ShipModule::DoStructureExplosiveDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsHull() )
		return;

	double hullResonance;
	double hullResonanceModifier;
	double newHullResonance;

	if( add ) {
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0) ) {
			hullResonance = m_pilot->GetShip()->hullExplosiveDamageResonance();
			hullResonanceModifier = m_item->explosiveDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	} else {
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0)	&&
			!Equals(m_item->explosiveDamageResistanceBonus(), -100) )
		{
			hullResonance = m_pilot->GetShip()->hullExplosiveDamageResonance();
			hullResonanceModifier = m_item->explosiveDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	}
}

void ShipModule::DoStructureKineticDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsHull() )
		return;

	double hullResonance;
	double hullResonanceModifier;
	double newHullResonance;

	if( add ) {
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0) ) {
			hullResonance = m_pilot->GetShip()->hullKineticDamageResonance();
			hullResonanceModifier = m_item->kineticDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	} else {
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0)		&&
			!Equals(m_item->kineticDamageResistanceBonus(), -100)	)
		{
			hullResonance = m_pilot->GetShip()->hullKineticDamageResonance();
			hullResonanceModifier = m_item->kineticDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	}
}

void ShipModule::DoStructureThermalDamageResistanceBonu(bool add, bool notify) {

	if( !AffectsHull() )
		return;

	double hullResonance;
	double hullResonanceModifier;
	double newHullResonance;

	if( add ) {
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0) ) {
			hullResonance = m_pilot->GetShip()->hullThermalDamageResonance();
			hullResonanceModifier = m_item->thermalDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	} else {
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0)		&&
			!Equals(m_item->thermalDamageResistanceBonus(), -100)	)
		{
			hullResonance = m_pilot->GetShip()->hullThermalDamageResonance();
			hullResonanceModifier = m_item->thermalDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	}
}
#endif

void ShipModule::DoImplantBonusVelocity(bool add, bool notify) {

	double maxVelocity;
	double maxVelocityModifier;
	double newMaxVelocity;

	if( add ) {
        if( !m_item->GetAttribute(AttrImplantBonusVelocity).get_float() == 0 ) {
            maxVelocity = m_pilot->GetShip()->GetAttribute(AttrMaxVelocity).get_float();
            maxVelocityModifier = m_item->GetAttribute(AttrImplantBonusVelocity).get_float();
			newMaxVelocity = maxVelocity + maxVelocity * maxVelocityModifier / 100;
            m_pilot->GetShip()->SetAttribute( (uint32)AttrMaxVelocity, (int64)newMaxVelocity );
		}
	} else {
        if( !m_item->GetAttribute(AttrImplantBonusVelocity).get_float() == 0 || -100 ) {
            maxVelocity = m_pilot->GetShip()->GetAttribute(AttrMaxVelocity).get_float();
            maxVelocityModifier = m_item->GetAttribute(AttrImplantBonusVelocity).get_float();
			newMaxVelocity = maxVelocity / ( 1 + maxVelocityModifier / 100 ) + 0.8; //round up
            m_pilot->GetShip()->SetAttribute( (uint32)AttrMaxVelocity, (int64)newMaxVelocity );
		}
	}
}

void ShipModule::DoSpeedBonus(bool add, bool notify) {

	double maxVelocity;
	double maxVelocityModifier;
	double newMaxVelocity;

	if( add ) {
        if( !m_item->GetAttribute(AttrSpeedBonus).get_float() == 0 ) {
            maxVelocity = m_pilot->GetShip()->GetAttribute(AttrMaxVelocity).get_float();
            maxVelocityModifier = m_item->GetAttribute(AttrSpeedBonus).get_float();
			newMaxVelocity = maxVelocity + maxVelocityModifier;
            m_pilot->GetShip()->SetAttribute((uint32)AttrMaxVelocity, (int64)newMaxVelocity );
		}
	} else {
        if( !m_item->GetAttribute(AttrSpeedBonus).get_float() == 0 ) {
            maxVelocity = m_pilot->GetShip()->GetAttribute(AttrMaxVelocity).get_float();
            maxVelocityModifier = m_item->GetAttribute(AttrSpeedBonus).get_float();
			newMaxVelocity = maxVelocity - maxVelocityModifier;
            m_pilot->GetShip()->SetAttribute((uint32)AttrMaxVelocity, (int64)newMaxVelocity );
		}
	}
}

void ShipModule::DoMaxVelocityBonus(bool add, bool notify) {

	double maxVelocity;
	double maxVelocityModifier;
	double newMaxVelocity;

	if( add ) {
        if( !Equals(m_item->GetAttribute(AttrMaxVelocityBonus).get_float(), 1) ) {
            maxVelocity = m_pilot->GetShip()->GetAttribute(AttrMaxVelocity).get_float();
			maxVelocityModifier = m_item->GetAttribute(AttrMaxVelocityBonus).get_float();
			newMaxVelocity = maxVelocity * maxVelocityModifier;
            m_pilot->GetShip()->SetAttribute( (uint32)AttrMaxVelocity, (int64)newMaxVelocity );
		}
	} else {
		if( !Equals(m_item->GetAttribute(AttrMaxVelocityBonus).get_float(), 1) &&
			!Equals(m_item->GetAttribute(AttrMaxVelocityBonus).get_float(), 0) )
		{
			maxVelocity = m_pilot->GetShip()->GetAttribute(AttrMaxVelocity).get_float();
			maxVelocityModifier = m_item->GetAttribute(AttrMaxVelocityBonus).get_float();
			newMaxVelocity = (maxVelocity / maxVelocityModifier) + 0.8; //round up
            m_pilot->GetShip()->SetAttribute( (uint32)AttrMaxVelocity, (int64)newMaxVelocity );
		}
	}
}

#if 0
void ShipModule::DoCpuMultiplier(bool add, bool notify) {
	
	double cpuOutput;
	double cpuOutputModifier;
	int newCpuOutput;

	if( add ) {
		if( !Equals(m_item->cpuMultiplier(), 1) ) {
			cpuOutput = m_pilot->GetShip()->cpuOutput();
			cpuOutputModifier = m_item->cpuMultiplier();
			newCpuOutput = cpuOutput * cpuOutputModifier + 0.5;//rounding
			m_pilot->GetShip()->Set_cpuOutput( newCpuOutput ); 
		}
	} else {
		if( !Equals(m_item->cpuMultiplier(), 1) && 
			!Equals(m_item->cpuMultiplier(), 0) ) 
		{
			cpuOutput = m_pilot->GetShip()->cpuOutput();
			cpuOutputModifier = m_item->cpuMultiplier();
			newCpuOutput = cpuOutput / cpuOutputModifier + 0.5; //rounding
			m_pilot->GetShip()->Set_cpuOutput( newCpuOutput ); //rounding
		}
	}
}

void ShipModule::DoPowerIncrease(bool add, bool notify) {

	double powerOutput;
	double powerOutputModifier;
	double newPowerOutput;

	if( add ) {
		if( !m_item->powerIncrease() == 0 ) {
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerIncrease();
			newPowerOutput = powerOutput + powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	} else {
		if( !m_item->powerIncrease() == 0 ) {
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerIncrease();
			newPowerOutput = powerOutput - powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	}
}

void ShipModule::DoPowerOutputBonus(bool add, bool notify) {

	double powerOutput;
	double powerOutputModifier;
	double newPowerOutput;

	if( add ) {
		if( !m_item->powerOutputBonus() == 0	&&
			!m_item->powerOutputBonus() == 1	) 
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputBonus();
			newPowerOutput = powerOutput * powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	} else {
		if( !m_item->powerOutputBonus() == 1	&&
			!m_item->powerOutputBonus() == 0	)
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputBonus();
			newPowerOutput = powerOutput / powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	}
}

void ShipModule::DoPowerOutputMultiplier(bool add, bool notify) {

	double powerOutput;
	double powerOutputModifier;
	double newPowerOutput;

	if( add ) {
		if( !m_item->powerOutputMultiplier() == 0	&&
			!m_item->powerOutputMultiplier() == 1	)
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputMultiplier();
			newPowerOutput = powerOutput * powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	} else {
		if( !m_item->powerOutputMultiplier() == 1	&&
			!m_item->powerOutputMultiplier() == 0	)
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputMultiplier();
			newPowerOutput = powerOutput / powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	}
}

void ShipModule::DoCapacitorBonus(bool add, bool notify) {

	double capacitorCapacity;
	double capacitorCapacityModifier;
	int newCapacitorCapacity;

	if( add ) {
		if( !m_item->capacitorBonus() == 0 ) {
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorBonus();
			newCapacitorCapacity = capacitorCapacity + capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}
	} else {
		if( !m_item->capacitorBonus() == 0 ) {
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorBonus();
			newCapacitorCapacity = capacitorCapacity - capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}
	}
}

void ShipModule::DoCapacitorCapacityMultiplier(bool add, bool notify) {

	double capacitorCapacity;
	double capacitorCapacityModifier;
	int newCapacitorCapacity;

	if( add ) {
		if( !Equals(m_item->capacitorCapacityMultiplier(), 0)	&&
			!Equals(m_item->capacitorCapacityMultiplier(), 1)	)
		{
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorCapacityMultiplier();
			newCapacitorCapacity = capacitorCapacity * capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}
	} else {
		if( !Equals(m_item->capacitorCapacityMultiplier(), 0)	&&
			!Equals(m_item->capacitorCapacityMultiplier(), 1)	)
		{
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorCapacityMultiplier();
			newCapacitorCapacity = capacitorCapacity / capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}
	}
}

void ShipModule::DoCapacitorRechargeRateMultiplier(bool add, bool notify) {

	double rechargeRate;
	double rechargeRateModifier;
	double newRechargeRate;

	if( add ) {
		if( !Equals(m_item->capacitorRechargeRateMultiplier(), 1) ) {
			rechargeRate = m_pilot->GetShip()->rechargeRate();
			rechargeRateModifier = m_item->capacitorRechargeRateMultiplier();
			newRechargeRate = rechargeRate * rechargeRateModifier;
			m_pilot->GetShip()->Set_rechargeRate( newRechargeRate );
		}
	} else {
		if( !Equals(m_item->capacitorRechargeRateMultiplier(), 1)	&&
			!Equals(m_item->capacitorRechargeRateMultiplier(), 0)	)
		{
			rechargeRate = m_pilot->GetShip()->rechargeRate();
			rechargeRateModifier = m_item->capacitorRechargeRateMultiplier();
			newRechargeRate = rechargeRate / rechargeRateModifier;
			m_pilot->GetShip()->Set_rechargeRate( newRechargeRate );
		}
	}
}

void ShipModule::DoCargoCapacityMultiplier(bool add, bool notify) {

	double cargoCapacity;
	double cargoCapacityModifier;
	double newCargoCapacity;
	
	if( add ) {
		if( !Equals(m_item->cargoCapacityMultiplier(), 1) ) {
			cargoCapacity = m_pilot->GetShip()->capacity();
			cargoCapacityModifier = m_item->cargoCapacityMultiplier();
			newCargoCapacity = cargoCapacity * cargoCapacityModifier;
			m_pilot->GetShip()->Set_capacity( newCargoCapacity );
		}
	} else {
		if( !Equals(m_item->cargoCapacityMultiplier(), 1)	&&
			!Equals(m_item->cargoCapacityMultiplier(), 0)	)
		{
			cargoCapacity = m_pilot->GetShip()->capacity();
			cargoCapacityModifier = m_item->cargoCapacityMultiplier();
			newCargoCapacity = cargoCapacity / cargoCapacityModifier;
			m_pilot->GetShip()->Set_capacity( newCargoCapacity );
		}
	}
}

void ShipModule::DoScanStrengthBonus(bool add, bool notify) {

	double scanStrength;
	double scanStrengthModifier;
	double newScanStrength;

	if( add ) {
		if( !m_item->scanStrengthBonus() == 0 ) {
			//uses gravimetric scanners
			if( !m_pilot->GetShip()->scanGravimetricStrength() == 0 ) {
				scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
				m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
			}
			//uses ladar scanners
			if( !m_pilot->GetShip()->scanLadarStrength() == 0 ) {
				scanStrength = m_pilot->GetShip()->scanLadarStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
				m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
			}
			//uses magnetometric scanners
			if( !m_pilot->GetShip()->scanMagnetometricStrength() == 0 ) {
				scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
				m_pilot->GetShip()->Set_scanMagnetometricStrength( ToInt(newScanStrength) );
			}
			//uses radar scanners
			if( !m_pilot->GetShip()->scanRadarStrength() == 0 ) {
				scanStrength = m_pilot->GetShip()->scanRadarStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
				m_pilot->GetShip()->Set_scanRadarStrength( newScanStrength );
			}
		}
	} else {
		if( !m_item->scanStrengthBonus() == 0 ) {
			//uses gravimetric scanners
			if( !(m_pilot->GetShip()->scanGravimetricStrength() == 0) &&
				!(m_pilot->GetShip()->scanGravimetricStrength() == -100) )
			{
				scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
			}
			//uses ladar scanners
			if( !(m_pilot->GetShip()->scanLadarStrength() == 0)	&&
				!(m_pilot->GetShip()->scanLadarStrength() == -100) )
			{
				scanStrength = m_pilot->GetShip()->scanLadarStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
			}
			//uses magnetometric scanners
			if( !(m_pilot->GetShip()->scanMagnetometricStrength() == 0) &&
				!(m_pilot->GetShip()->scanMagnetometricStrength() == -100) )
			{
				scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanMagnetometricStrength( ToInt(newScanStrength) );
			}
			//uses radar scanners
			if( !(m_pilot->GetShip()->scanRadarStrength() == 0) &&
				!(m_pilot->GetShip()->scanRadarStrength() == -100) ) 
			{
				scanStrength = m_pilot->GetShip()->scanRadarStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanRadarStrength( newScanStrength );
			}
		}
	}
}

void ShipModule::DoScanGravimetricStrengthPercent(bool add, bool notify) {
	
	double scanStrength;
	double scanStrengthModifier;
	double newScanStrength;

	if( add ) {
		if( !m_item->scanGravimetricStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
			scanStrengthModifier = m_item->scanGravimetricStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
		}
	} else {
		if( !m_item->scanGravimetricStrengthPercent() == 0		&&
			!m_item->scanGravimetricStrengthPercent() == -100	)
		{
			scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
			scanStrengthModifier = m_item->scanGravimetricStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
		}
	}
}

void ShipModule::DoScanLadarStrengthPercent(bool add, bool notify) {

	double scanStrength;
	double scanStrengthModifier;
	double newScanStrength;
	
	if( add ) {
		if( !m_item->scanLadarStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanLadarStrength();
			scanStrengthModifier = m_item->scanLadarStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
		}
	} else {
		if( !m_item->scanLadarStrengthPercent() == 0		&&
			!m_item->scanLadarStrengthPercent() == -100		)
		{
			scanStrength = m_pilot->GetShip()->scanLadarStrengthPercent();
			scanStrengthModifier = m_item->scanLadarStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
		}
	}
}

void ShipModule::DoScanMagnetometricStrengthPercent(bool add, bool notify) {
	
	double scanStrength;
	double scanStrengthModifier;
	double newScanStrength;

	if( add ) {
		if( !m_item->scanMagnetometricStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
			scanStrengthModifier = m_item->scanMagnetometricStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanMagnetometricStrength( newScanStrength );
		}
	} else {
		if( !m_item->scanMagnetometricStrengthPercent() == 0		&&
			!m_item->scanMagnetometricStrengthPercent() == -100		)
		{
			scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
			scanStrengthModifier = m_item->scanMagnetometricStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanMagnetometricStrength( newScanStrength );
		}
	}
}

void ShipModule::DoScanRadarStrengthPercent(bool add, bool notify) {
	
	double scanStrength;
	double scanStrengthModifier;
	double newScanStrength;

	if( add ) {
		if( !m_item->scanRadarStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanRadarStrength();
			scanStrengthModifier = m_item->scanRadarStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanRadarStrength( newScanStrength );
		}
	} else {
		if( !m_item->scanRadarStrengthPercent() == 0		&&
			!m_item->scanRadarStrengthPercent() == -100		)
		{
			scanStrength = m_pilot->GetShip()->scanRadarStrength();
			scanStrengthModifier = m_item->scanRadarStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanRadarStrength( newScanStrength );
		}
	}
}

void ShipModule::DoSignatureRadiusBonus(bool add, bool notify) {

	double signatureRadius;
	double signatureRadiusModifier;
	double newSignatureRadius;

	if( add ) {
		if( !m_item->signatureRadiusBonus() == 0 ) {
			signatureRadius = m_pilot->GetShip()->signatureRadius();
			signatureRadiusModifier = m_item->signatureRadiusBonus();
			newSignatureRadius = signatureRadius + signatureRadius * signatureRadiusModifier / 100;
			m_pilot->GetShip()->Set_signatureRadius( newSignatureRadius );
		}
	} else {
		if( !m_item->signatureRadiusBonus() == 0 ) {
			signatureRadius = m_pilot->GetShip()->signatureRadius();
			signatureRadiusModifier = m_item->signatureRadiusBonus();
			newSignatureRadius = signatureRadius / ( 1 + signatureRadiusModifier / 100 );
			m_pilot->GetShip()->Set_signatureRadius( newSignatureRadius );
		}
	}
}

void ShipModule::DoDroneCapacityBonus(bool add, bool notify) {

	double droneCapacity;
	double droneCapacityModifier;
	double newDroneCapacity;

	if( add ) {
		if( !m_item->droneCapacityBonus() == 0 ) {
			droneCapacity = m_pilot->GetShip()->droneCapacity();
			droneCapacityModifier = m_item->droneCapacityBonus();
			newDroneCapacity = droneCapacity + droneCapacityModifier;
			m_pilot->GetShip()->Set_droneCapacity( newDroneCapacity );
		}
	} else {
		if( !m_item->droneCapacityBonus() == 0 ) {
			droneCapacity = m_pilot->GetShip()->droneCapacity();
			droneCapacityModifier = m_item->droneCapacityBonus();
			newDroneCapacity = droneCapacity - droneCapacityModifier;
			m_pilot->GetShip()->Set_droneCapacity( newDroneCapacity );
		}
	}
}

void ShipModule::DoAgilityMultiplier(bool add, bool notify) {

	double agility;
	double agilityModifier;
	double newAgility;

	if( add ) {
		if( !Equals(m_item->agilityMultiplier(), 1) ) {
			agility = m_pilot->GetShip()->agility();
			agilityModifier = m_item->agilityMultiplier();
			newAgility = agility * -agilityModifier;
			m_pilot->GetShip()->Set_agility( newAgility );
		} 
	} else {
		if( !Equals(m_item->agilityMultiplier(), 1)	&&
			!Equals(m_item->agilityMultiplier(), 0)	)
		{
			agility = m_pilot->GetShip()->agility();
			agilityModifier = m_item->agilityMultiplier();
			newAgility = agility / agilityModifier;
			m_pilot->GetShip()->Set_agility( newAgility );
		}
	}
}

void ShipModule::DoScanResultionBonus(bool add, bool notify) {

	double scanResolution;
	double scanResolutionModifier;
	double newScanResolution;

	if( add ) {
		if( !Equals(m_item->scanResolutionBonus(), 0) ) {
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionBonus();
			newScanResolution = scanResolution + scanResolution * scanResolutionModifier / 100 + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}
	} else {
		if( !Equals(m_item->scanResolutionBonus(), 0)		&&
			!Equals(m_item->scanResolutionBonus(), -100)	)
		{
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionBonus();
			newScanResolution = scanResolution / ( 1 + scanResolutionModifier / 100 ) + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}
	}
}

void ShipModule::DoScanResultionMultiplier(bool add, bool notify) {

	double scanResolution;
	double scanResolutionModifier;
	double newScanResolution;

	if( add ) {
		if( !Equals(m_item->scanResolutionMultiplier(), 0)	&&
			!Equals(m_item->scanResolutionMultiplier(), 1)	)
		{
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionMultiplier();
			newScanResolution = scanResolution * scanResolutionModifier + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}
	} else {
		if( !Equals(m_item->scanResolutionMultiplier(), 0)	&&
			!Equals(m_item->scanResolutionMultiplier(), 1)	)
		{
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionMultiplier();
			newScanResolution = scanResolution / scanResolutionModifier + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}
	}
}

void ShipModule::DoMaxTargetRangeBonus(bool add, bool notify) {

	double maxTargetRange;
	double maxTargetRangeModifier;
	double newMaxTargetRange;

	if( add ) {
		if( !Equals(m_item->maxTargetRangeBonus(), 0) ) {
			maxTargetRange = m_pilot->GetShip()->maxTargetRange();
			maxTargetRangeModifier = m_item->maxTargetRangeBonus();
			newMaxTargetRange = maxTargetRange + maxTargetRange * maxTargetRangeModifier / 100;
			m_pilot->GetShip()->Set_maxTargetRange( newMaxTargetRange );
		}
	} else {
		if( !Equals(m_item->maxTargetRangeBonus(), 0)		&&
			!Equals(m_item->maxTargetRangeBonus(), -100)	)
		{
			maxTargetRange = m_pilot->GetShip()->maxTargetRange();
			maxTargetRangeModifier = m_item->maxTargetRangeBonus();
			newMaxTargetRange = maxTargetRange / ( 1 + maxTargetRangeModifier / 100 );
			m_pilot->GetShip()->Set_maxTargetRange( newMaxTargetRange );
		}
	}
}

void ShipModule::DoMaxLockedTargetsBonus(bool add, bool notify) {

	double maxLockedTargets;
	double maxLockedTargetsModifier;
	double newMaxLockedTargets;

	if( add ) {
		if( !m_item->maxLockedTargetsBonus() == 0 ) {
			maxLockedTargets = m_pilot->GetShip()->maxLockedTargets();
			maxLockedTargetsModifier = m_item->maxLockedTargetsBonus();
			newMaxLockedTargets = maxLockedTargets + maxLockedTargetsModifier;
			m_pilot->GetShip()->Set_maxLockedTargets( newMaxLockedTargets );
		}
	} else {
		if( !m_item->maxLockedTargetsBonus() == 0 ) {
			maxLockedTargets = m_pilot->GetShip()->maxLockedTargets();
			maxLockedTargetsModifier = m_item->maxLockedTargetsBonus();
			newMaxLockedTargets = maxLockedTargets - maxLockedTargetsModifier;
			m_pilot->GetShip()->Set_maxLockedTargets( newMaxLockedTargets );
		}
	}
}

void ShipModule::DoWarpScrambleStrength(bool add, bool notify) {

	double warpScrambleStrength;
	double warpScrambleStrengthModifier;
	double newWarpScrambleStrength;

	if( add ) {
		if( !m_item->warpScrambleStrength() == 0 ) {
			warpScrambleStrength = m_pilot->GetShip()->warpScrambleStrength();
			warpScrambleStrengthModifier = m_item->warpScrambleStrength();
			newWarpScrambleStrength = warpScrambleStrength - warpScrambleStrengthModifier;
			m_pilot->GetShip()->Set_warpScrambleStrength( newWarpScrambleStrength );
		}
	} else {
		if( !m_item->warpScrambleStrength() == 0 ) {
			warpScrambleStrength = m_pilot->GetShip()->warpScrambleStrength();
			warpScrambleStrengthModifier = m_item->warpScrambleStrength();
			newWarpScrambleStrength = warpScrambleStrength + warpScrambleStrengthModifier;
			m_pilot->GetShip()->Set_warpScrambleStrength( newWarpScrambleStrength );
		}
	}
}

void ShipModule::DoPropulsionFusionStrength(bool add, bool notify) {

	double propulsionStrength;
	double propulsionStrengthModifier;
	double newPropulsionStrength;

	if( add ) {
		if( !m_item->propulsionFusionStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionFusionStrength();
			propulsionStrengthModifier = m_item->propulsionFusionStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionFusionStrength( newPropulsionStrength );
		}
	} else {
		if( !m_item->propulsionFusionStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionFusionStrength();
			propulsionStrengthModifier = m_item->propulsionFusionStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionFusionStrength( newPropulsionStrength );
		}
	}
}

void ShipModule::DoPropulsionIonStrength(bool add, bool notify) {

	double propulsionStrength;
	double propulsionStrengthModifier;
	double newPropulsionStrength;

	if( add ) {
		if( !m_item->propulsionIonStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionIonStrength();
			propulsionStrengthModifier = m_item->propulsionIonStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionIonStrength( newPropulsionStrength );
		}
	} else {
		if( !m_item->propulsionIonStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionIonStrength();
			propulsionStrengthModifier = m_item->propulsionIonStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionIonStrength( newPropulsionStrength );
		}
	}
}

void ShipModule::DoPropulsionMagpulseStrength(bool add, bool notify) {

	double propulsionStrength;
	double propulsionStrengthModifier;
	double newPropulsionStrength;

	if( add ) {
		if( !m_item->propulsionMagpulseStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionMagpulseStrength();
			propulsionStrengthModifier = m_item->propulsionMagpulseStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionMagpulseStrength( newPropulsionStrength );
		}
	} else {
		if( !m_item->propulsionMagpulseStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionMagpulseStrength();
			propulsionStrengthModifier = m_item->propulsionMagpulseStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionMagpulseStrength( newPropulsionStrength );
		}
	}
}

void ShipModule::DoPropulsionPlasmaStrength(bool add, bool notify) {

	double propulsionStrength;
	double propulsionStrengthModifier;
	double newPropulsionStrength;

	if( add ) {
		if( !m_item->propulsionPlasmaStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionPlasmaStrength();
			propulsionStrengthModifier = m_item->propulsionPlasmaStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionPlasmaStrength( newPropulsionStrength );
		}
	} else {
		if( !m_item->propulsionPlasmaStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionPlasmaStrength();
			propulsionStrengthModifier = m_item->propulsionPlasmaStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionPlasmaStrength( newPropulsionStrength );
		}
	}
}

void ShipModule::DoPassiveArmorEmDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsArmor() )
		return;
	
	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !m_item->passiveEmDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}
	} else {
		if( !m_item->passiveEmDamageResistanceBonus() == 0		&&
			!m_item->passiveEmDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoPassiveArmorExplosiveDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsArmor() )
		return;
		
	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}
	} else {
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0		&&
			!m_item->passiveExplosiveDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoPassiveArmorKineticDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsArmor() )
		return;
		
	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !m_item->passiveKineticDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}
	} else {
		if( !m_item->passiveKineticDamageResistanceBonus() == 0		&&
			!m_item->passiveKineticDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoPassiveArmorThermalDamageResistanceBonu(bool add, bool notify) {

	if( !AffectsArmor() )
		return;
		
	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( add ) {
		if( !m_item->passiveThermicDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorThermalDamageResonance();
			resistanceModifier = m_item->passiveThermicDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorThermalDamageResonance( newArmorResonance );
		}
	} else {
		if( !m_item->passiveThermicDamageResistanceBonus() == 0		&&
			!m_item->passiveThermicDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorThermalDamageResonance();
			resistanceModifier = m_item->passiveThermicDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorThermalDamageResonance( newArmorResonance );
		}
	}
}

void ShipModule::DoPassiveShieldEmDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;
		
	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !m_item->passiveEmDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}
	} else {
		if( !m_item->passiveEmDamageResistanceBonus() == 0		&&
			!m_item->passiveEmDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoPassiveShieldExplosiveDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;
		
	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}
	} else {
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0		&&
			!m_item->passiveExplosiveDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoPassiveShieldKineticDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;
		
	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !m_item->passiveKineticDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}
	} else {
		if( !m_item->passiveKineticDamageResistanceBonus() == 0		&&
			!m_item->passiveKineticDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}
	}
}

void ShipModule::DoPassiveShieldThermalDamageResistanceBonus(bool add, bool notify) {

	if( !AffectsShield() )
		return;
		
	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( add ) {
		if( !m_item->passiveThermicDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldThermalDamageResonance();
			resistanceModifier = m_item->passiveThermicDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldThermalDamageResonance( newResonance );
		}
	} else {
		if( !m_item->passiveThermicDamageResistanceBonus() == 0		&&
			!m_item->passiveThermicDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldThermalDamageResonance();
			resistanceModifier = m_item->passiveThermicDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldThermalDamageResonance( newResonance );
		}
	}
}
#endif

void ShipModule::DoGodmaEffects(bool active) {
	
	Notify_OnGodmaShipEffect gse;
	gse.itemID = m_item->itemID();
	gse.effectID =
	gse.when = Win32TimeNow();
	gse.start = active?1:0;
	gse.active = active?1:0;
	gse.env_itemID = m_item->itemID();
	gse.env_charID = m_item->ownerID();	//a little questionable
	gse.env_shipID = m_item->locationID();
	gse.env_target = m_target;

	if( active )
    {
        // env_others are people and object that target you
		PyTuple* env_other = new PyTuple( 3 );
        env_other->SetItem( 0, new PyInt( m_item->locationID() ) ); //ship ID.
        env_other->SetItem( 1, new PyInt( 29 ) ); //no idea
		env_other->SetItem( 2, new PyInt( 215 ) );	//no idea

		gse.env_other = env_other;
	}
    else
		gse.env_other = new PyNone;
	gse.env_effectID = gse.effectID;
	gse.startTime = gse.when;
	gse.duration =  m_activationInterval;
	gse.repeat = m_repeatCount;
	gse.randomSeed = new PyNone;
	gse.error = new PyNone;

	//should this only go to ourself?
	PyTuple* up = gse.Encode();
	m_pilot->QueueDestinyEvent( &up );
    PySafeDecRef( up );

}

void ShipModule::DoSpecialFX(bool online) {

	DoDestiny_OnSpecialFX13 sfx;

		sfx.entityID = m_pilot->GetShipID();
		sfx.moduleID = m_item->itemID();
		sfx.moduleTypeID = m_item->typeID();
		sfx.targetID = m_target;
		sfx.otherTypeID = 0;	//target->Item()->typeID();
		//sfx.effect_type = 
		//sfx.isOffensive = 
		sfx.start = 1;
		sfx.active = 1;
		sfx.duration_ms = SFXEffectInterval();
		sfx.repeat = m_repeatCount;
		sfx.startTime = Win32TimeNow();
	
		PyTuple* up = sfx.Encode();
		m_pilot->Destiny()->SendSingleDestinyUpdate( &up );	//consumed
		PySafeDecRef( up );

}

int ShipModule::SFXEffectInterval() {

	//these are totally arbitrary :)

	switch( m_item->groupID() ) {
		case EVEDB::invGroups::Shield_Booster:
			return m_activationInterval / 5;

		default:
			return m_activationInterval;
			break;
	}
}
void ShipModule::ChangeMState(State new_state) {

	//simply to clean up the code a bit
	m_state = new_state;

}

bool ShipModule::Equals(double a, int b, double percision) {

	if( abs( a - b ) < percision )
		return true;

	return false;
}

int ShipModule::ToInt(double a) {
	int b = a;
	return b;
}
bool ShipModule::TypeCast(double a) {

	uint64 b = a;

	if( a == b )
		return true;

	return false;
}
void ShipModule::GetActivationInterval() {
	
//	m_activationInterval = 4000;  //need to fix this

}


bool ShipModule::AffectsArmor() {

	//for now this will have to do, as I do not know where in the database where this information is
	switch( m_item->groupID() ) {
		case EVEDB::invGroups::Armor_Coating:
		case EVEDB::invGroups::Armor_Hardener:
		case EVEDB::invGroups::Armor_Plating_Energized:
		case EVEDB::invGroups::Armor_Reinforcer:
		case EVEDB::invGroups::Armor_Repair_Projector:
		case EVEDB::invGroups::Armor_Repair_Unit:
		case EVEDB::invGroups::Rig_Armor:
			return true;
	}
	//TODO: add the rest of the modules
	return false;
}
bool ShipModule::AffectsShield() {

	//for now this will have to do, as I do not know where in the database where this information is
	switch( m_item->groupID() ) {
		case EVEDB::invGroups::Shield_Amplifier:
		case EVEDB::invGroups::Shield_Boost_Amplifier:
		case EVEDB::invGroups::Shield_Booster:
		case EVEDB::invGroups::Shield_Disruptor:
		case EVEDB::invGroups::Shield_Extender:
		case EVEDB::invGroups::Shield_Flux_Coil:
		case EVEDB::invGroups::Shield_Hardener:
		case EVEDB::invGroups::Shield_Hardening_Array:
		case EVEDB::invGroups::Shield_Power_Relay:
		case EVEDB::invGroups::Shield_Recharger:
		case EVEDB::invGroups::Shield_Transporter:
		case EVEDB::invGroups::Rig_Shield:
			return true;
	}
	//TODO: add the rest of the modules
	return false;
		
}
bool ShipModule::AffectsHull() {

	//for now this will have to do, as I do not know where in the database where this information is
	switch( m_item->groupID() ) {
		case EVEDB::invGroups::Hull_Mods:
		case EVEDB::invGroups::Hull_Repair_Unit:
			return true;
	}
	//TODO: add the rest of the modules
	return false;

}
ModuleEffect::ModuleEffect(){

}
ModuleEffect::~ModuleEffect(){

}

