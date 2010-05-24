/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		ItWasLuck
*/
#include "EVEServerPCH.h"

//Creates the module manager for the client
ModuleManager::ModuleManager(Client *pilot)
: m_pilot(pilot)  //Accessing m_pilot here will cause serious problems
{
	memset(m_modules, 0, sizeof(m_modules));
}

//Deconstructor for the module manager
//iterates through and destroys all modules
ModuleManager::~ModuleManager() {
	uint8 r;
	for(r = 0; r < MAX_MODULE_COUNT; r++) {
		delete m_modules[r];
	}
}

//used for the GM macro "Online my modules" and online command
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

//used for GM macro "Repare my modules" and repaimodules command
void ModuleManager::RepairModules()
{
	std::map<uint32, uint8> ::iterator p;
	for(p = m_moduleByID.begin(); p!=m_moduleByID.end(); ++p)
	{
		ShipModule *mod = m_modules[p->second];
		mod->item()->Set_damage(0);
	}
}

//used for GM macro and unload command
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

//used for GM macro and unload command
void ModuleManager::UnloadAllModules()
{
	ShipModule *mod;

	std::map<uint32, uint8> ::iterator p;
	for(p = m_moduleByID.begin(); p!=m_moduleByID.end(); ++p)
	{
		mod = m_modules[p->second];
		mod->Deactivate("online");
		m_modules[p->second]->item()->Move(m_pilot->GetStationID(), flagHangar, true);
		if( m_modules[p->second]->item()->massAddition() != 0 )
			m_pilot->GetShip()->Set_mass( m_pilot->GetShip()->mass() - m_modules[p->second]->item()->massAddition() );
	}
	//shouldn't need these
	//m_pilot->GetShip()->Set_cpuLoad( 0 );
	//m_pilot->GetShip()->Set_powerLoad( 0 );

	m_moduleByID.clear();
}

//Basic module mananger process
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
	
	_log(SHIP__MODULE_TRACE, "%s: Refreshing modules for ship %s (%u).", m_pilot->GetName(), ship->itemName().c_str(), ship->itemID());
	
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
					_log(SHIP__ERROR, "%s: Unexpected item category %d in slot %d (wanted charge)", m_pilot->GetName(), items[1]->categoryID(), flag);
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
					_log(SHIP__ERROR, "%s: Unexpected item category %d in slot %d (wanted module)", m_pilot->GetName(), items[1]->categoryID(), flag);
					valid = false;
				}
			}
			else
			{
				_log(SHIP__ERROR, "%s: Unexpected item category %d in slot %d (wanted module or charge)", m_pilot->GetName(), items[0]->categoryID(), flag);
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
			_log(SHIP__ERROR, "%s: More than two items in a module slot (%d). We do not understand this yet!", m_pilot->GetName(), flag);
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
					_log(SHIP__MODULE_TRACE, "%s: Item %s (%u) is still in slot %d.", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot);
				}
				else
				{
					_log(SHIP__MODULE_TRACE, "%s: Item %s (%u) is still in slot %d, but has a new charge %s (%u).", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot, ( !charge ? "None" : charge->itemName().c_str() ), ( !charge ? 0 : charge->itemID() ));
					m_modules[slot]->ChangeCharge(charge);
				}
				//no change...
				m_moduleByID[module->itemID()] = slot;
				continue;
			}

			SafeDelete( m_modules[slot] );
		}
		_log(SHIP__MODULE_TRACE, "%s: Item %s (%u) is now in slot %d. Using charge %s (%u)", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot, ( !charge ? "None" : charge->itemName().c_str() ), ( !charge ? 0 : charge->itemID() ));
		m_modules[slot] = ShipModule::CreateModule(m_pilot, module, charge);
		m_moduleByID[module->itemID()] = slot;
	}
}

int ModuleManager::Activate(uint32 itemID, const std::string &effectName, uint32 target, uint32 repeat) {

	std::map<uint32, uint8>::const_iterator res;
	res = m_moduleByID.find(itemID);

	//check if you have the module on your ship
	if(res == m_moduleByID.end()) {

		//log error
		_log(SHIP__ERROR, "Activate: %s: failed to find module %u", m_pilot->GetName(), itemID);
		return 0;
	}

	//grab the actual module
	ShipModule *mod = m_modules[res->second];
	if(mod == NULL) {

		//if this happens we have bigger problems than your module not working
		codelog(SHIP__ERROR, "%s: failed to activate module %u", m_pilot->GetName(), itemID);
		return 0;
	}

	//activate it and return that we did to the client
	return(mod->Activate(effectName, target, repeat));
}

void ModuleManager::Deactivate(uint32 itemID, const std::string &effectName) {

	std::map<uint32, uint8>::const_iterator res;
	res = m_moduleByID.find(itemID);

	//check if you have the module on your ship
	if(res == m_moduleByID.end()) {

		//log error
		_log(SHIP__ERROR, "Deactivate: %s: failed to find module %u.", m_pilot->GetName(), itemID);
		return;
	}

	//grab the actual module
	ShipModule *mod = m_modules[res->second];

	if(mod == NULL) {
		//if this happens we have bigger problems than your module not working
		codelog(SHIP__ERROR, "%s: failed to deactivate module %u", m_pilot->GetName(), itemID);
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
				//TODO: implement deactivation logic
            }
            catch( char * str )
            {
                _log(SHIP__MODULE_TRACE, "Error (%s)", str);
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
  m_repeatCount(1000),
  m_target(0),
  m_fit(NewModule),
  m_type(Turret),
  m_interval(0),
  m_apGroup(ActiveModule),
  m_effectID(effectShieldBoosting),
  m_active(true)
{

	//activate it if it's a new module
	if( !m_item->isOnline() )
		Activate("online", NULL, NULL);

}

//Ship Module Deconstructor
ShipModule::~ShipModule()
{
}

//ShipModule module constructor.  Creates new instance
ShipModule *ShipModule::CreateModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_) {

	//This module is then built out to a specific type
	return(new ShipModule(owner, self, charge_));

}

void ShipModule::ChangeCharge(InventoryItemRef new_charge) {

	//You can only change charges while not active, but must be online

	//check if online
	if(m_state == Online ) {

		//check if active
		if( m_state == Active || Overloaded ) {

			//Tell pilot he needs to deactive this weapon before changing the charge
			m_pilot->SendNotifyMsg( "You must deactivate the weapon before changing the charge" );
		
		} else {

			//change charges
			m_charge = new_charge;

		}

	}

}


////////////////////////////////////////////////////////////
/////////////////ShipModule Processors//////////////////////
////////////////////////////////////////////////////////////

void ShipModule::Process() {
	switch(m_state) {




	}
	

}

int ShipModule::Activate(const std::string &effectName, uint32 target, uint32 repeat) {

	if( effectName == "online" )  {

		if( m_state == Online ){
			m_pilot->SendNotifyMsg("Module is already online");
			return 0;
		}

		//online the module
		if( Validate_Online() ) {
			Handle_Online();
			return 1;
		} else {
			//do nothing
			return 0;
		}

	} else if ( effectName == m_effectName ) {

		if( Validate_Active() ) {
			Handle_Active(true);
			return 1;
		} else {
			//do nothing
			return 0;
		}

	} else if ( effectName == "overload" ) { //idk if this is correct

		//do overload stuff

	}

	return 0;
}

void ShipModule::Deactivate(const std::string &effectName) {

	if( effectName == "online" ) {

		//offline the module
		if( Validate_Offline() )
			Handle_Offline();
		else 
			m_pilot->SendNotifyMsg("You cannont deactivate this module");
			//Note to self: this should go in Validate_Offline

	} else if ( effectName == m_effectName ) {

		//deactivate module

	}
}


////////////////////////////////////////////////////////////
///////////////ShipModule CoProcessors//////////////////////
////////////////////////////////////////////////////////////
bool ShipModule::Validate_Online() {

	//check for cpu
	if( m_item->cpu() + m_pilot->GetShip()->cpuLoad() > m_pilot->GetShip()->cpuOutput() ) {
		m_pilot->SendNotifyMsg("You do not have enough available cpu for this");
		return false;
	}

	//check for powergrid
	if (m_item->power() + m_pilot->GetShip()->powerLoad() > m_pilot->GetShip()->powerOutput() ) {
		m_pilot->SendNotifyMsg("You do not have enough available power for this");
		return false;
	}

	//if in space, check for cap
	if( m_pilot->IsInSpace() )
		if( !m_pilot->GetShip()->charge() == m_pilot->GetShip()->capacitorCapacity() ){
			m_pilot->SendNotifyMsg("You do not have enough available capacitor charge for this");
			return false;
		}


	//check for ship conflicts - should this go in the _ExecAdd command?
	//strip miner
	if( m_item->groupID() == EVEDB::invGroups::Strip_Miner )
		if( !m_pilot->GetShip()->groupID() == EVEDB::invGroups::Mining_Barge ) {
			m_pilot->SendNotifyMsg("Your ship cannont use this module");
			return false;
		}

	//super weapons
	if( m_item->groupID() == EVEDB::invGroups::Super_Weapon ) {
		if( m_item->typeID() == 24554 ) { //Erebus super weapon
			if( !m_pilot->GetShip()->typeID() == 671 ) { //Erebus
				m_pilot->SendNotifyMsg("Your ship cannont use this module");
				return false;
			}
		} else if( m_item->typeID() == 23674 ) { //Ragnorok super weapon
			if( !m_pilot->GetShip()->typeID() == 23773 ) { //Ragnorock
				m_pilot->SendNotifyMsg("Your ship cannont use this module");
				return false;
			}
		} else if( m_item->typeID() == 24550 ) { //Avatar super weapon
			if( !m_pilot->GetShip()->typeID() == 11567 ) { //Avatar
				m_pilot->SendNotifyMsg("Your ship cannont use this module");
				return false;
			}
		} else if( m_item->typeID() == 24552 ) { //Leviathan super weapon
			if( !m_pilot->GetShip()->typeID() == 3764 ) { // Leviathan
				m_pilot->SendNotifyMsg("Your ship cannont use this module");
				return false;
			}
		}
	}

	
	//if we pass everything, return true
	return true;

}

void ShipModule::Handle_Online() {

	//change module state to online
	ChangeMState(Online);

	//Put item online
	m_item->PutOnline();

	//consume cpu
	m_pilot->GetShip()->Set_cpuLoad( m_pilot->GetShip()->cpuLoad() + m_item->cpu() );

	//consume powergrid
	m_pilot->GetShip()->Set_powerLoad( m_pilot->GetShip()->powerLoad() + m_item->power() );

	//if in space, consume all capacitor
	if( m_pilot->IsInSpace() )
		m_pilot->GetShip()->Set_charge( 0 );

	//do all passive effects
	//go through all modules which have passive effects
	DoPassiveEffects(true);

		

		/* 
		 * The AffectsShield, AffectsHull and AffectsArmor functions are necessary
		 * because hardeners for all three use the same attribute (ex. emDamageResistanceBonus)
		 * and therefore we have to check which one it should be affecting
		 */
	
		/* Questions and TODO:
		 * What do amarrNavyBonus and those type things do...
		 * Whats the best way to access other modules on the ship from here...
		 * Remeber to check power and cpu when modules that modify it are offlined
		 * Neet to figure out a good way to apply stacking penalties
		 * Need to make unfitting effects handled in offline
		 */


	  //do the few additions that active modules do when just online

		//check for other modules that will effect these modules.  Shield boosting bonuses, tracking speed bonus, fire rates...ect
		//drone control range stuff, scanning amplifiers

		//etc...

	//TODO: CHECK FOR SKILLS
	

}

bool ShipModule::Validate_Offline() {


	//for now, just return true, till i figure out what needs to go in here
	return true;

}

void ShipModule::Handle_Offline() {

	//check if it's already online
	if( m_state == Offline )
		return;

	//change module state to offline
	ChangeMState(Offline);

	//Put item offline
	m_item->PutOffline();

	//return cpu
	m_pilot->GetShip()->Set_cpuLoad( m_pilot->GetShip()->cpuLoad() - m_item->cpu() );

	//return powergrid
	m_pilot->GetShip()->Set_powerLoad( m_pilot->GetShip()->powerLoad() - m_item->power() );

	//undo all passive effects
	//go through all modules which have passive effects
	DoPassiveEffects(false);


	//TODO: CHECK FOR SKILLS

}

bool ShipModule::Validate_Active() {

	//check for activation energy
	//if( m_item->



	return false;
}

void ShipModule::Handle_Active(bool start) {

	if( start ) {

		//do module startup stuff here
		//first time activation energy
		//shenanagans

	} else {

		//normal operation

	}

}



void ShipModule::DoPassiveEffects(bool online) {
	
	switch ( m_item->groupID() ) {
		case EVEDB::invGroups::Armor_Coating:
			ArmorHPEffects(online);
			ArmorResistanceEffects(online);
			break;
		case EVEDB::invGroups::Armor_Hardener:
			ActiveModulePassiveArmorResists(online);
			break;
		case EVEDB::invGroups::Armor_Plating_Energized:
			ArmorHPEffects(online);
			ArmorResistanceEffects(online);
			break;
		case EVEDB::invGroups::Armor_Reinforcer:
			ArmorHPEffects(online);
			break;
		case EVEDB::invGroups::Automated_Targeting_System:
			ShipMaxTargetsEffects(online);
			break;
		case EVEDB::invGroups::Auxiliary_Power_Core:
			ShipPowerEffects(online);
			break;
		case EVEDB::invGroups::Capacitor_Battery:
			ShipCapacitorChargeEffects(online);
			break;
		case EVEDB::invGroups::Capacitor_Flux_Coil:
			ShipCapacitorChargeEffects(online);
			ShipCapacitorRechargeRateEffects(online);
			ShieldHPEffects(online);
			ShieldRechargeEffects(online);
			break;
		case EVEDB::invGroups::Capacitor_Power_Relay:
			ShipCapacitorChargeEffects(online);
			ShipCapacitorRechargeRateEffects(online);
			break;
		case EVEDB::invGroups::Capacitor_Recharger:
			ShipCapacitorRechargeRateEffects(online);
			break;
		case EVEDB::invGroups::CPU_Enhancer:
			ShipCpuEffects(online);
			break;
		case EVEDB::invGroups::DroneBayExpander:
			ShipDroneCapacityEffects(online);
			break;
		case EVEDB::invGroups::ECM_Stabilizer:
			ShipScanStrengthEffects(online);
			break;
		case EVEDB::invGroups::Expanded_Cargohold:
			ShipCargoCapacityEffects(online);
			ShipVelocityEffects(online);
			HullHPEffects(online);
			break;
		case EVEDB::invGroups::Inertial_Stabilizer:
			ShipAgilityEffects(online);
			break;
		case EVEDB::invGroups::Nanofiber_Internal_Structure:
			ShipAgilityEffects(online);
			ShipVelocityEffects(online);
			HullHPEffects(online);
			break;
		case EVEDB::invGroups::Overdrive_Injector_System:
			ShipCargoCapacityEffects(online);
			ShipVelocityEffects(online);
			break;
		case EVEDB::invGroups::Power_Diagnostics:
			ShipPowerEffects(online);
			ShipCapacitorRechargeRateEffects(online);
			ShipCapacitorChargeEffects(online);
			ShieldHPEffects(online);
			ShieldRechargeEffects(online);
			break;
		case EVEDB::invGroups::Reactor_Control_Unit:
			ShipPowerEffects(online);
			ShipCapacitorRechargeRateEffects(online);
			ShipCapacitorChargeEffects(online);
			ShieldHPEffects(online);
			ShieldRechargeEffects(online);
			break;
		case EVEDB::invGroups::Reinforced_Bulkheads:
			HullHPEffects(online);
			ShipVelocityEffects(online);
			ShipAgilityEffects(online);
			break;
		case EVEDB::invGroups::Sensor_Backup_Array:
			ShipScanStrengthEffects(online);
			break;
		case EVEDB::invGroups::Shield_Amplifier:
			ShieldResistanceEffects(online);
			break;
		case EVEDB::invGroups::Shield_Extender:
			ShieldHPEffects(online);
			ShipSignatureRadiusEffects(online);
			break;
		case EVEDB::invGroups::Shield_Flux_Coil:
			ShipCapacitorChargeEffects(online);
			ShipCapacitorRechargeRateEffects(online);
			ShipPowerEffects(online);
			ShieldHPEffects(online);
			ShieldRechargeEffects(online);
			break;
		case EVEDB::invGroups::Shield_Hardener:
			ActiveModulePassiveShieldResists(online);
			break;
		case EVEDB::invGroups::Shield_Power_Relay:
			ShipCapacitorChargeEffects(online);
			ShipCapacitorRechargeRateEffects(online);
			ShipPowerEffects(online);
			ShieldHPEffects(online);
			ShieldRechargeEffects(online);
			break;
		case EVEDB::invGroups::Shield_Recharger:
			ShieldRechargeEffects(online);
			break;
		case EVEDB::invGroups::Signal_Amplifier:
			ShipModule::ShipMaxTargetRangeEffects(online);
			ShipModule::ShipMaxTargetsEffects(online);
			ShipModule::ShipScanResolutionEffects(online);
			break;
		case EVEDB::invGroups::Warp_Core_Stabilizer:
			ShipMaxTargetRangeEffects(online);
			ShipPropulsionStrengthEffects(online);
			ShipScanResolutionEffects(online);
			ShipWarpScrambleStrengthEffects(online);
			break;
	}

}


void ShipModule::DoActiveEffects(bool online) {

	//do all active effects
	switch ( m_item->groupID() ) {
		case EVEDB::invGroups::Afterburner:
			ActiveShipVelocityEffects(online);
			break;
	}

}
////////////////////////////////////////////////////////////
///////////////ShipModule Effect Functions//////////////////
////////////////////////////////////////////////////////////

/* All ship effects functions follow the same layout. You should be able 
 * to do copy and replace all on the three names in brackets, throw in your formula and have your function
 * Enjoy :)
 
void ShipModule::[AttributeModifiedName]Effects( bool online ) {
 		
	double [AttributeModifiedName];
	double [AttributeModifiedName]Modifier;
	double new[AttributeModifiedName];

	if( online ) {
		//Attribute [AttributeModifierName]
		if( !m_item->[AttributeModifierName] == [AttributeDefaultValue] ) {
			[AttributeModifiedName] = m_pilot->GetShip()->[AttributeModifiedName];
			[AttributeModifiedName]Modifier = m_item->[AttributeModifierName];
			new[AttributeModifiedName] = [Formula For calculating online value]
			m_pilot->GetShip()->Set_[AttributeModifiedName]( new[AttributeModifiedName] );
		}
	} else {
		//Attribute [AttributeModifierName]
		if( !m_item->[AttributeModifierName] == [AttributeDefaultValue] || [value which would result in division by 0] ) {
			[AttributeModifiedName] = m_pilot->GetShip()->[AttributeModifiedName];
			[AttributeModifiedName]Modifier = m_item->[AttributeModifierName];
			new[AttributeModifiedName] = [Formula For calculating offline value]
			m_pilot->GetShip()->Set_[AttributeModifiedName]( new[AttributeModifiedName] );
		}
	}
}

*/
void ShipModule::ActiveShipVelocityEffects(bool online)	{

	//handles microwarp, afterburners and cloaks
	//Attributes available: speedFactor, speedBoostFactor, speedBoostFactorCalc1 speedBoostFactorCalc2
	
	//Cruising speed * (1 + (base boost of AB or MWD * (1 + accel control level * 0.05) * (ab/mwd implant(s)) * (thrust / (ship mass + modules mass)))
	
	//calculate new speed
	double maxVelocity = m_pilot->GetShip()->maxVelocity();
	double shipMass = m_pilot->GetShip()->mass();
	double maxVelocityMod1 = m_item->speedBoostFactor();
	double maxVelocityMod2;
	double maxVelocityMod3;
	double newMaxVelocity;


}
void ShipModule::ArmorHPEffects(bool online) {

	double armorHP;
	double armorHPModifier;
	double newArmorHP;
	
	if( online ) { 
		//Attribute armorHpBonus
		if( !m_item->armorHpBonus() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonus();
			newArmorHP = armorHP + armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}

		//Attribute armorHpBonusAdd
		if( !m_item->armorHpBonusAdd() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonusAdd();
			newArmorHP = armorHP + armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}

		//Attribute armorHPMultiplier
		if( !Equals(m_item->armorHPMultiplier(), 1) ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHPMultiplier();
			newArmorHP = armorHP * armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}
	} else { 
		//Attribute armorHpBonus
		if( !m_item->armorHpBonus() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonus();
			newArmorHP = armorHP - armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}

		//Attribute armorHpBonusAdd
		if( !m_item->armorHpBonusAdd() == 0 ) {
			armorHP = m_pilot->GetShip()->armorHP();
			armorHPModifier = m_item->armorHpBonusAdd();
			newArmorHP = armorHP - armorHPModifier;
			m_pilot->GetShip()->Set_armorHP( newArmorHP );
		}

		//Attribute armorHPMultiplier
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

void ShipModule::ArmorResistanceEffects(bool online) {

	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( online ) {
		//Attribute emDamageResistanceBonus
		if( !Equals(m_item->emDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}

		//Attribute explosiveDamageResistanceBonus
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}

		//Attribute kineticDamageResistanceBonus
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}

		//Attribute thermalDamageResistanceBonus
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0) ) {
			armorResonance = m_pilot->GetShip()->armorThermalDamageResonance();
			resistanceModifier = m_item->thermalDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorThermalDamageResonance( newArmorResonance );
		}

	} else {
		//Attribute emDamageResistanceBonus
		if( !Equals(m_item->emDamageResistanceBonus(), 0)		&&
			!Equals(m_item->emDamageResistanceBonus(), -100)	)
		{
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}

		//Attribute explosiveDamageResistanceBonus
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0)	 &&
			!Equals(m_item->explosiveDamageResistanceBonus(), -100)	 )
		{
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}

		//Attribute kineticDamageResistanceBonus
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0)	&&
			!Equals(m_item->kineticDamageResistanceBonus(), -100)  )
		{
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}

		//Attribute thermalDamageResistanceBonus
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

void ShipModule::ShieldHPEffects(bool online) {

	double shieldCapacity;
	double capacityModifier;
	int newShieldCapacity;

	if( online ) {
		//Attribute capacity
		if( !m_item->capacity() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacity();
			newShieldCapacity = shieldCapacity + capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}

		//Attribute capacityBonus
		if( !m_item->capacityBonus() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacityBonus();
			newShieldCapacity = shieldCapacity + capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}

		//Attribute shieldCapacityMultiplier
		if( !Equals(m_item->shieldCapacityMultiplier(), 1) ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->shieldCapacityMultiplier();
			newShieldCapacity = shieldCapacity * capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	} else {
		//Attribute capacity
		if( !m_item->capacity() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacity();
			newShieldCapacity = shieldCapacity - capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}

		//Attribute capacityBonus
		if( !m_item->capacityBonus() == 0 ) {
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->capacityBonus();
			newShieldCapacity = shieldCapacity - capacityModifier;
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}

		//Attribute shieldCapacityMultiplier
		if( !Equals(m_item->shieldCapacityMultiplier(), 1)	||
			!Equals(m_item->shieldCapacityMultiplier(), 0)	)
		{
			shieldCapacity = m_pilot->GetShip()->shieldCapacity();
			capacityModifier = m_item->shieldCapacityMultiplier();
			newShieldCapacity = shieldCapacity / capacityModifier + 0.8; //round up
			m_pilot->GetShip()->Set_shieldCapacity( newShieldCapacity );
		}
	}

}

void ShipModule::ShieldResistanceEffects(bool online) {

	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( online ) {
		//Attribute emDamageResistanceBonus
		if( !Equals(m_item->emDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}

		//Attribute explosiveDamageResistanceBonus
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}

		//Attribute kineticDamageResistanceBonus
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}

		//Attribute thermalDamageResistanceBonus
		if( !Equals(m_item->thermalDamageResistanceBonus(), 0) ) {
			shipResonance = m_pilot->GetShip()->shieldThermalDamageResonance();
			resistanceModifier = m_item->thermalDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldThermalDamageResonance( newResonance );
		}

	} else {
		//Attribute emDamageResistanceBonus
		if( !Equals(m_item->emDamageResistanceBonus(), 0)		&&
			!Equals(m_item->emDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->emDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}

		//Attribute explosiveDamageResistanceBonus
		if( !Equals(m_item->explosiveDamageResistanceBonus(), 0)	&&
			!Equals(m_item->explosiveDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->explosiveDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}

		//Attribute kineticDamageResistanceBonus
		if( !Equals(m_item->kineticDamageResistanceBonus(), 0)		&&
			!Equals(m_item->kineticDamageResistanceBonus(), -100)	)
		{
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->kineticDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}

		//Attribute thermalDamageResistanceBonus
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

void ShipModule::ShieldRechargeEffects(bool online) {

	double shieldRechargeRate;
	double rechargeRateModifier;
	double newShieldRechargeRate;

	if( online ) {
		//Attribute shieldRechargeRateMultiplier
		if( !Equals(m_item->shieldRechargeRateMultiplier(), 1) ) {
			shieldRechargeRate = m_pilot->GetShip()->shieldRechargeRate();
			rechargeRateModifier = m_item->shieldRechargeRateMultiplier();
			newShieldRechargeRate = shieldRechargeRate * rechargeRateModifier;
			m_pilot->GetShip()->Set_shieldRechargeRate( ToInt(newShieldRechargeRate) );
		}
	} else {
		//Attribute shieldRechargeRateMultiplier
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

void ShipModule::HullHPEffects(bool online) {

	double hullHP;
	double hullHPModifier;
	double newHullHP;

	if( online ) {
		//Attribute hullHpBonus
		if( !m_item->hullHpBonus() == 0 ) {
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->hullHpBonus();
			newHullHP = hullHP + hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
		
		//Attribute structureHPMultiplier
		if( !Equals(m_item->structureHPMultiplier(), 1) ) {
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->structureHPMultiplier();
			newHullHP = hullHP * hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
	} else {
		//Attribute hullHpBonus
		if( !m_item->hullHpBonus() == 0 ) {
			hullHP = m_pilot->GetShip()->hp();
			hullHPModifier = m_item->hullHpBonus();
			newHullHP = hullHP - hullHPModifier;
			m_pilot->GetShip()->Set_hp( newHullHP );
		}
		
		//Attribute structureHPMultiplier
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

void ShipModule::HullResistanceEffects(bool online) {

	double hullResonance;
	double hullResonanceModifier;
	double newHullResonance;

	if( online ) {
		//Attribute emDamageResistanceBonus
		if( !m_item->emDamageResistanceBonus() == 0 ) {
			hullResonance = m_pilot->GetShip()->hullEmDamageResonance();
			hullResonanceModifier = m_item->emDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
		
		//Attribute explosiveDamageResistanceBonus
		if( !m_item->explosiveDamageResistanceBonus() == 0 ) {
			hullResonance = m_pilot->GetShip()->hullExplosiveDamageResonance();
			hullResonanceModifier = m_item->explosiveDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}

		//Attribute kineticDamageResistanceBonus
		if( !m_item->kineticDamageResistanceBonus() == 0 ) {
			hullResonance = m_pilot->GetShip()->hullKineticDamageResonance();
			hullResonanceModifier = m_item->kineticDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}

		//Attribute thermalDamageResistanceBonus
		if( !m_item->thermalDamageResistanceBonus() == 0 ) {
			hullResonance = m_pilot->GetShip()->hullThermalDamageResonance();
			hullResonanceModifier = m_item->thermalDamageResistanceBonus();
			newHullResonance = hullResonance + hullResonance * hullResonanceModifier / 100;
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	} else {
		//Attribute emDamageResistanceBonus
		if( !m_item->emDamageResistanceBonus() == 0 || -100 ) {
			hullResonance = m_pilot->GetShip()->hullEmDamageResonance();
			hullResonanceModifier = m_item->emDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
		
		//Attribute explosiveDamageResistanceBonus
		if( !m_item->explosiveDamageResistanceBonus() == 0 || -100 ) {
			hullResonance = m_pilot->GetShip()->hullExplosiveDamageResonance();
			hullResonanceModifier = m_item->explosiveDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}

		//Attribute kineticDamageResistanceBonus
		if( !m_item->kineticDamageResistanceBonus() == 0 || -100 ) {
			hullResonance = m_pilot->GetShip()->hullKineticDamageResonance();
			hullResonanceModifier = m_item->kineticDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}

		//Attribute thermalDamageResistanceBonus
		if( !m_item->thermalDamageResistanceBonus() == 0 || -100 ) {
			hullResonance = m_pilot->GetShip()->hullThermalDamageResonance();
			hullResonanceModifier = m_item->thermalDamageResistanceBonus();
			newHullResonance = hullResonance / ( 1 + hullResonanceModifier / 100 );
			m_pilot->GetShip()->Set_hullEmDamageResonance( newHullResonance );
		}
	}
}

void ShipModule::ShipVelocityEffects(bool online) {

	double maxVelocity;
	double maxVelocityModifier;
	double newMaxVelocity;

	if( online ) {
		//Attribute implantBonusVelocity
		if( !m_item->implantBonusVelocity() == 0 ) {
			maxVelocity = m_pilot->GetShip()->maxVelocity();
			maxVelocityModifier = m_item->implantBonusVelocity();
			newMaxVelocity = maxVelocity + maxVelocity * maxVelocityModifier / 100;
			m_pilot->GetShip()->Set_maxVelocity( newMaxVelocity );
		}
		
		//Attribute speedBonus
		if( !m_item->speedBonus() == 0 ) {
			maxVelocity = m_pilot->GetShip()->maxVelocity();
			maxVelocityModifier = m_item->speedBonus();
			newMaxVelocity = maxVelocity + maxVelocityModifier;
			m_pilot->GetShip()->Set_maxVelocity( newMaxVelocity );
		}
		
		//Attribute maxVelocityBonus
		if( !Equals(m_item->maxVelocityBonus(), 1) ) {
			maxVelocity = m_pilot->GetShip()->maxVelocity();
			maxVelocityModifier = m_item->maxVelocityBonus();
			newMaxVelocity = maxVelocity * maxVelocityModifier;
			m_pilot->GetShip()->Set_maxVelocity( newMaxVelocity );
		}
	} else {
		//Attribute implantBonusVelocity
		if( !m_item->implantBonusVelocity() == 0 || -100 ) {
			maxVelocity = m_pilot->GetShip()->maxVelocity();
			maxVelocityModifier = m_item->implantBonusVelocity();
			newMaxVelocity = maxVelocity / ( 1 + maxVelocityModifier / 100 ) + 0.8; //round up
			m_pilot->GetShip()->Set_maxVelocity( newMaxVelocity );
		}
		
		//Attribute speedBonus
		if( !m_item->speedBonus() == 0 ) {
			maxVelocity = m_pilot->GetShip()->maxVelocity();
			maxVelocityModifier = m_item->speedBonus();
			newMaxVelocity = maxVelocity - maxVelocityModifier;
			m_pilot->GetShip()->Set_maxVelocity( newMaxVelocity );
		}
		
		//Attribute maxVelocityBonus
		if( !Equals(m_item->maxVelocityBonus(), 1) &&
			!Equals(m_item->maxVelocityBonus(), 0) )
		{
			maxVelocity = m_pilot->GetShip()->maxVelocity();
			maxVelocityModifier = m_item->maxVelocityBonus();
			newMaxVelocity = maxVelocity / maxVelocityModifier + 0.8; //round up
			m_pilot->GetShip()->Set_maxVelocity( newMaxVelocity );
		}
	}

}

void ShipModule::ShipCpuEffects(bool online) {

	double cpuOutput;
	double cpuOutputModifier;
	int newCpuOutput;

	if( online ) {
		//Attribute cpuMultiplier
		if( !Equals(m_item->cpuMultiplier(), 1) ) {
			cpuOutput = m_pilot->GetShip()->cpuOutput();
			cpuOutputModifier = m_item->cpuMultiplier();
			newCpuOutput = cpuOutput * cpuOutputModifier + 0.5;//rounding
			m_pilot->GetShip()->Set_cpuOutput( newCpuOutput ); 
		}
	} else {
		//Attribute cpuMultiplier
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

void ShipModule::ShipPowerEffects(bool online) {

	double powerOutput;
	double powerOutputModifier;
	double newPowerOutput;

	if( online ) {
		//Attribute powerIncrease
		if( !m_item->powerIncrease() == 0 ) {
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerIncrease();
			newPowerOutput = powerOutput + powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
		
		//Attribute powerOutputBonus

		if( !m_item->powerOutputBonus() == 0	&&
			!m_item->powerOutputBonus() == 1	) 
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputBonus();
			newPowerOutput = powerOutput * powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}

		//Attribute powerOutputMultiplier
		if( !m_item->powerOutputMultiplier() == 0	&&
			!m_item->powerOutputMultiplier() == 1	)
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputMultiplier();
			newPowerOutput = powerOutput * powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}
	} else {
		//Attribute powerIncrease
		if( !m_item->powerIncrease() == 0 ) {
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerIncrease();
			newPowerOutput = powerOutput - powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}

		//Attribute powerOutputBonus
		if( !m_item->powerOutputBonus() == 1	&&
			!m_item->powerOutputBonus() == 0	)
		{
			powerOutput = m_pilot->GetShip()->powerOutput();
			powerOutputModifier = m_item->powerOutputBonus();
			newPowerOutput = powerOutput / powerOutputModifier;
			m_pilot->GetShip()->Set_powerOutput( newPowerOutput );
		}

		//Attribute powerOutputMultiplier
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

void ShipModule::ShipCapacitorChargeEffects(bool online) {

	double capacitorCapacity;
	double capacitorCapacityModifier;
	int newCapacitorCapacity;

	if( online ) {
		//Attribute capacityBonus
		if( !m_item->capacitorBonus() == 0 ) {
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorBonus();
			newCapacitorCapacity = capacitorCapacity + capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}

		//Attribute capacitorCapacityMultiplier
		if( !Equals(m_item->capacitorCapacityMultiplier(), 0)	&&
			!Equals(m_item->capacitorCapacityMultiplier(), 1)	)
		{
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorCapacityMultiplier();
			newCapacitorCapacity = capacitorCapacity * capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}
	} else {
		//Attribute capacityBonus
		if( !m_item->capacitorBonus() == 0 ) {
			capacitorCapacity = m_pilot->GetShip()->capacitorCapacity();
			capacitorCapacityModifier = m_item->capacitorBonus();
			newCapacitorCapacity = capacitorCapacity - capacitorCapacityModifier;
			m_pilot->GetShip()->Set_capacitorCapacity( newCapacitorCapacity );
		}

		//Attribute capacitorCapacityMultiplier
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

void ShipModule::ShipCapacitorRechargeRateEffects(bool online) {

	double rechargeRate;
	double rechargeRateModifier;
	double newRechargeRate;

	if( online ) {
		//Attribute capacitorRechargeRateMultiplier
		if( !Equals(m_item->capacitorRechargeRateMultiplier(), 1) ) {
			rechargeRate = m_pilot->GetShip()->rechargeRate();
			rechargeRateModifier = m_item->capacitorRechargeRateMultiplier();
			newRechargeRate = rechargeRate * rechargeRateModifier;
			m_pilot->GetShip()->Set_rechargeRate( newRechargeRate );
		}
	} else {
		//Attribute capacitorRechargeRateMultiplier
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

void ShipModule::ShipCargoCapacityEffects(bool online) {

	double cargoCapacity;
	double cargoCapacityModifier;
	double newCargoCapacity;
	
	if( online ) {
		//Attribute cargoCapacityMultiplier
		if( !Equals(m_item->cargoCapacityMultiplier(), 1) ) {
			cargoCapacity = m_pilot->GetShip()->capacity();
			cargoCapacityModifier = m_item->cargoCapacityMultiplier();
			newCargoCapacity = cargoCapacity * cargoCapacityModifier;
			m_pilot->GetShip()->Set_capacity( newCargoCapacity );
		}
	} else {
		//Attribute cargoCapacityMultiplier
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

void ShipModule::ShipScanStrengthEffects(bool online) {

	double scanStrength;
	double scanStrengthModifier;
	double newScanStrength;

	if( online ) {
		//Attribute scanStrengthBonus
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

		//Attribute scanGravimetricStrengthPercent
		if( !m_item->scanGravimetricStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
			scanStrengthModifier = m_item->scanGravimetricStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
		}
		//Attribute scanLadarStrengthPercent
		if( !m_item->scanLadarStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanLadarStrength();
			scanStrengthModifier = m_item->scanLadarStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
		}
		//Attribute scanMagnetometricStrengthPercent
		if( !m_item->scanMagnetometricStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
			scanStrengthModifier = m_item->scanMagnetometricStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanMagnetometricStrength( newScanStrength );
		}
		//Attribute scanRadarStrengthPercent
		if( !m_item->scanRadarStrengthPercent() == 0 ) {
			scanStrength = m_pilot->GetShip()->scanRadarStrength();
			scanStrengthModifier = m_item->scanRadarStrengthPercent();
			newScanStrength = scanStrength + scanStrength * scanStrengthModifier / 100;
			m_pilot->GetShip()->Set_scanRadarStrength( newScanStrength );
		}
	} else {
		//Attribute scanStrengthBonus
		if( !m_item->scanStrengthBonus() == 0 ) {
			//uses gravimetric scanners
			if( !m_pilot->GetShip()->scanGravimetricStrength() == 0		&&
				!m_pilot->GetShip()->scanGravimetricStrength() == -100	)
			{
				scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
			}
			//uses ladar scanners
			if( !m_pilot->GetShip()->scanLadarStrength() == 0		&&
				!m_pilot->GetShip()->scanLadarStrength() == -100	)
			{
				scanStrength = m_pilot->GetShip()->scanLadarStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
			}
			//uses magnetometric scanners
			if( !m_pilot->GetShip()->scanMagnetometricStrength() == 0		&&
				!m_pilot->GetShip()->scanMagnetometricStrength() == -100	)
			{
				scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanMagnetometricStrength( ToInt(newScanStrength) );
			}
			//uses radar scanners
			if( !m_pilot->GetShip()->scanRadarStrength() == 0		&&
				!m_pilot->GetShip()->scanRadarStrength() == -100	) 
			{
				scanStrength = m_pilot->GetShip()->scanRadarStrength();
				scanStrengthModifier = m_item->scanStrengthBonus();
				newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
				m_pilot->GetShip()->Set_scanRadarStrength( newScanStrength );
			}
		}

		//Attribute scanGravimetricStrengthPercent
		if( !m_item->scanGravimetricStrengthPercent() == 0		&&
			!m_item->scanGravimetricStrengthPercent() == -100	)
		{
			scanStrength = m_pilot->GetShip()->scanGravimetricStrength();
			scanStrengthModifier = m_item->scanGravimetricStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanGravimetricStrength( newScanStrength );
		}
		//Attribute scanLadarStrengthPercent
		if( !m_item->scanLadarStrengthPercent() == 0		&&
			!m_item->scanLadarStrengthPercent() == -100		)
		{
			scanStrength = m_pilot->GetShip()->scanLadarStrengthPercent();
			scanStrengthModifier = m_item->scanLadarStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanLadarStrength( newScanStrength );
		}
		//Attribute scanMagnetometricStrengthPercent
		if( !m_item->scanMagnetometricStrengthPercent() == 0		&&
			!m_item->scanMagnetometricStrengthPercent() == -100		)
		{
			scanStrength = m_pilot->GetShip()->scanMagnetometricStrength();
			scanStrengthModifier = m_item->scanMagnetometricStrengthPercent();
			newScanStrength = scanStrength / ( 1 + scanStrengthModifier / 100 );
			m_pilot->GetShip()->Set_scanMagnetometricStrength( newScanStrength );
		}
		//Attribute scanRadarStrengthPercent
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

void ShipModule::ShipSignatureRadiusEffects(bool online) {

	double signatureRadius;
	double signatureRadiusModifier;
	double newSignatureRadius;

	if( online ) {
		//Attribute signatureRadiusBonus
		if( !m_item->signatureRadiusBonus() == 0 ) {
			signatureRadius = m_pilot->GetShip()->signatureRadius();
			signatureRadiusModifier = m_item->signatureRadiusBonus();
			newSignatureRadius = signatureRadius + signatureRadius * signatureRadiusModifier / 100;
			m_pilot->GetShip()->Set_signatureRadius( newSignatureRadius );
		}
	} else {
		//Attribute signatureRadiusBonus
		if( !m_item->signatureRadiusBonus() == 0 ) {
			signatureRadius = m_pilot->GetShip()->signatureRadius();
			signatureRadiusModifier = m_item->signatureRadiusBonus();
			newSignatureRadius = signatureRadius / ( 1 + signatureRadiusModifier / 100 );
			m_pilot->GetShip()->Set_signatureRadius( newSignatureRadius );
		}
	}

}

void ShipModule::ShipDroneCapacityEffects(bool online) {

	double droneCapacity;
	double droneCapacityModifier;
	double newDroneCapacity;

	if( online ) {
		//Attribute droneCapacityBonus
		if( !m_item->droneCapacityBonus() == 0 ) {
			droneCapacity = m_pilot->GetShip()->droneCapacity();
			droneCapacityModifier = m_item->droneCapacityBonus();
			newDroneCapacity = droneCapacity + droneCapacityModifier;
			m_pilot->GetShip()->Set_droneCapacity( newDroneCapacity );
		}
	} else {
		//Attribute droneCapacityBonus
		if( !m_item->droneCapacityBonus() == 0 ) {
			droneCapacity = m_pilot->GetShip()->droneCapacity();
			droneCapacityModifier = m_item->droneCapacityBonus();
			newDroneCapacity = droneCapacity - droneCapacityModifier;
			m_pilot->GetShip()->Set_droneCapacity( newDroneCapacity );
		}
	}

}

void ShipModule::ShipAgilityEffects(bool online) {

	double agility;
	double agilityModifier;
	double newAgility;

	if( online ) {
		//Attribute agilityMultiplier
		//for whatever reason it's different for inertial stabilizers
		if( IsInertialStabilizer() ) {
			if( !Equals(m_item->agilityMultiplier(), 1) ) {
				agility = m_pilot->GetShip()->agility();
				agilityModifier = m_item->agilityMultiplier();
				newAgility = agility - agility * agilityModifier / 100;
				m_pilot->GetShip()->Set_agility( newAgility );
			}
		} else {
			//Attribute agilityMultiplier
			if( !Equals(m_item->agilityMultiplier(), 1) ) {
				agility = m_pilot->GetShip()->agility();
				agilityModifier = m_item->agilityMultiplier();
				newAgility = agility * -agilityModifier;
				m_pilot->GetShip()->Set_agility( newAgility );
			} 
		}
	} else {
		//Attribute agilityMultiplier
		//for whatever reason it's different for inertial stabilizers
		if( IsInertialStabilizer() ) {
			if( !Equals(m_item->agilityMultiplier(), 1)		&&
				!Equals(m_item->agilityMultiplier(), 100)	)
			{
				agility = m_pilot->GetShip()->agility();
				agilityModifier = m_item->agilityMultiplier();
				newAgility = agility / ( 1 - agilityModifier / 100 );
				m_pilot->GetShip()->Set_agility( newAgility );
			}
		} else {
			//Attribute agilityMultiplier
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

}

void ShipModule::ShipScanResolutionEffects(bool online) {

	double scanResolution;
	double scanResolutionModifier;
	double newScanResolution;

	if( online ) {
		//Attribute scanResolutionBonus
		if( !Equals(m_item->scanResolutionBonus(), 0) ) {
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionBonus();
			newScanResolution = scanResolution + scanResolution * scanResolutionModifier / 100 + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}

		//Attribute scanResolutionMultiplier
		if( !Equals(m_item->scanResolutionMultiplier(), 0)	&&
			!Equals(m_item->scanResolutionMultiplier(), 1)	)
		{
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionMultiplier();
			newScanResolution = scanResolution * scanResolutionModifier + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}
	} else {
		//Attribute scanResolutionBonus
		if( !Equals(m_item->scanResolutionBonus(), 0)		&&
			!Equals(m_item->scanResolutionBonus(), -100)	)
		{
			scanResolution = m_pilot->GetShip()->scanResolution();
			scanResolutionModifier = m_item->scanResolutionBonus();
			newScanResolution = scanResolution / ( 1 + scanResolutionModifier / 100 ) + 0.5; //correction factor
			m_pilot->GetShip()->Set_scanResolution( newScanResolution );
		}

		//Attribute scanResolutionMultiplier
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

void ShipModule::ShipMaxTargetRangeEffects(bool online) {

	double maxTargetRange;
	double maxTargetRangeModifier;
	double newMaxTargetRange;

	if( online ) {
		//Attribute maxTargetRangeBonus
		if( !Equals(m_item->maxTargetRangeBonus(), 0) ) {
			maxTargetRange = m_pilot->GetShip()->maxTargetRange();
			maxTargetRangeModifier = m_item->maxTargetRangeBonus();
			newMaxTargetRange = maxTargetRange + maxTargetRange * maxTargetRangeModifier / 100;
			m_pilot->GetShip()->Set_maxTargetRange( newMaxTargetRange );
		}
	} else {
		//Attribute maxTargetRangeBonus
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

void ShipModule::ShipMaxTargetsEffects(bool online) {

	double maxLockedTargets;
	double maxLockedTargetsModifier;
	double newMaxLockedTargets;

	if( online ) {
		//Attribute maxLockedTargetsBonus
		if( !m_item->maxLockedTargetsBonus() == 0 ) {
			maxLockedTargets = m_pilot->GetShip()->maxLockedTargets();
			maxLockedTargetsModifier = m_item->maxLockedTargetsBonus();
			newMaxLockedTargets = maxLockedTargets + maxLockedTargetsModifier;
			m_pilot->GetShip()->Set_maxLockedTargets( newMaxLockedTargets );
		}
	} else {
		//Attribute maxLockedTargetsBonus
		if( !m_item->maxLockedTargetsBonus() == 0 ) {
			maxLockedTargets = m_pilot->GetShip()->maxLockedTargets();
			maxLockedTargetsModifier = m_item->maxLockedTargetsBonus();
			newMaxLockedTargets = maxLockedTargets - maxLockedTargetsModifier;
			m_pilot->GetShip()->Set_maxLockedTargets( newMaxLockedTargets );
		}
	}


}

void ShipModule::ShipWarpScrambleStrengthEffects(bool online) {

	double warpScrambleStrength;
	double warpScrambleStrengthModifier;
	double newWarpScrambleStrength;

	if( online ) {
		//Attribute warpScrambleStrength
		if( !m_item->warpScrambleStrength() == 0 ) {
			warpScrambleStrength = m_pilot->GetShip()->warpScrambleStrength();
			warpScrambleStrengthModifier = m_item->warpScrambleStrength();
			newWarpScrambleStrength = warpScrambleStrength - warpScrambleStrengthModifier;
			m_pilot->GetShip()->Set_warpScrambleStrength( newWarpScrambleStrength );
		}
	} else {
		//Attribute warpScrambleStrength
		if( !m_item->warpScrambleStrength() == 0 ) {
			warpScrambleStrength = m_pilot->GetShip()->warpScrambleStrength();
			warpScrambleStrengthModifier = m_item->warpScrambleStrength();
			newWarpScrambleStrength = warpScrambleStrength + warpScrambleStrengthModifier;
			m_pilot->GetShip()->Set_warpScrambleStrength( newWarpScrambleStrength );
		}
	}

}

void ShipModule::ShipPropulsionStrengthEffects(bool online) {

	double propulsionStrength;
	double propulsionStrengthModifier;
	double newPropulsionStrength;

	if( online ) {
		//Attribute propulsionFusionStrength
		if( !m_item->propulsionFusionStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionFusionStrength();
			propulsionStrengthModifier = m_item->propulsionFusionStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionFusionStrength( newPropulsionStrength );
		}
		
		//Attribute propulsionIonStrength
		if( !m_item->propulsionIonStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionIonStrength();
			propulsionStrengthModifier = m_item->propulsionIonStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionIonStrength( newPropulsionStrength );
		}
		
		//Attribute propulsionMagpulseStrength
		if( !m_item->propulsionMagpulseStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionMagpulseStrength();
			propulsionStrengthModifier = m_item->propulsionMagpulseStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionMagpulseStrength( newPropulsionStrength );
		}
		
		//Attribute propulsionPlasmaStrength
		if( !m_item->propulsionPlasmaStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionPlasmaStrength();
			propulsionStrengthModifier = m_item->propulsionPlasmaStrength();
			newPropulsionStrength = propulsionStrength + propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionPlasmaStrength( newPropulsionStrength );
		}
	} else {
		//Attribute propulsionFusionStrength
		if( !m_item->propulsionFusionStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionFusionStrength();
			propulsionStrengthModifier = m_item->propulsionFusionStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionFusionStrength( newPropulsionStrength );
		}
		
		//Attribute propulsionIonStrength
		if( !m_item->propulsionIonStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionIonStrength();
			propulsionStrengthModifier = m_item->propulsionIonStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionIonStrength( newPropulsionStrength );
		}
		
		//Attribute propulsionMagpulseStrength
		if( !m_item->propulsionMagpulseStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionMagpulseStrength();
			propulsionStrengthModifier = m_item->propulsionMagpulseStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionMagpulseStrength( newPropulsionStrength );
		}
		
		//Attribute propulsionPlasmaStrength
		if( !m_item->propulsionPlasmaStrength() == 0 ) {
			propulsionStrength = m_pilot->GetShip()->propulsionPlasmaStrength();
			propulsionStrengthModifier = m_item->propulsionPlasmaStrength();
			newPropulsionStrength = propulsionStrength - propulsionStrengthModifier;
			m_pilot->GetShip()->Set_propulsionPlasmaStrength( newPropulsionStrength );
		}
	}


}

void ShipModule::ActiveModulePassiveArmorResists(bool online) {

	double armorResonance;
	double resistanceModifier;
	double newArmorResonance;

	if( online ) {
		//Attribute passiveEmDamageResistanceBonus
		if( !m_item->passiveEmDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}

		//Attribute passiveExplosiveDamageResistanceBonus
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}

		//Attribute passiveKineticDamageResistanceBonus
		if( !m_item->passiveKineticDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}

		//Attribute passiveThermalDamageResistanceBonus
		if( !m_item->passiveThermicDamageResistanceBonus() == 0 ) {
			armorResonance = m_pilot->GetShip()->armorThermalDamageResonance();
			resistanceModifier = m_item->passiveThermicDamageResistanceBonus();
			newArmorResonance = armorResonance + armorResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_armorThermalDamageResonance( newArmorResonance );
		}

	} else {
		//Attribute passiveEmDamageResistanceBonus
		if( !m_item->passiveEmDamageResistanceBonus() == 0		&&
			!m_item->passiveEmDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorEmDamageResonance( newArmorResonance );
		}

		//Attribute passiveExplosiveDamageResistanceBonus
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0		&&
			!m_item->passiveExplosiveDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorExplosiveDamageResonance( newArmorResonance );
		}

		//Attribute passiveKineticDamageResistanceBonus
		if( !m_item->passiveKineticDamageResistanceBonus() == 0		&&
			!m_item->passiveKineticDamageResistanceBonus() == -100	)
		{
			armorResonance = m_pilot->GetShip()->armorKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newArmorResonance = armorResonance / ( 1 + resistanceModifier/ 100 );
			m_pilot->GetShip()->Set_armorKineticDamageResonance( newArmorResonance );
		}

		//Attribute passiveThermalDamageResistanceBonus
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

void ShipModule::ActiveModulePassiveShieldResists(bool online) {


	double shipResonance;
	double resistanceModifier;
	double newResonance;

	if( online ) {
		//Attribute passiveEmDamageResistanceBonus
		if( !m_item->passiveEmDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}

		//Attribute passiveExplosiveDamageResistanceBonus
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}

		//Attribute passiveKineticDamageResistanceBonus
		if( !m_item->passiveKineticDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}

		//Attribute passiveThermicDamageResistanceBonus
		if( !m_item->passiveThermicDamageResistanceBonus() == 0 ) {
			shipResonance = m_pilot->GetShip()->shieldThermalDamageResonance();
			resistanceModifier = m_item->passiveThermicDamageResistanceBonus();
			newResonance = shipResonance + shipResonance * resistanceModifier / 100;
			m_pilot->GetShip()->Set_shieldThermalDamageResonance( newResonance );
		}

	} else {
		//Attribute passiveEmDamageResistanceBonus
		if( !m_item->passiveEmDamageResistanceBonus() == 0		&&
			!m_item->passiveEmDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldEmDamageResonance();
			resistanceModifier = m_item->passiveEmDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldEmDamageResonance( newResonance );
		}

		//Attribute passiveExplosiveDamageResistanceBonus
		if( !m_item->passiveExplosiveDamageResistanceBonus() == 0		&&
			!m_item->passiveExplosiveDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldExplosiveDamageResonance();
			resistanceModifier = m_item->passiveExplosiveDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldExplosiveDamageResonance( newResonance );
		}

		//Attribute passiveKineticDamageResistanceBonus
		if( !m_item->passiveKineticDamageResistanceBonus() == 0		&&
			!m_item->passiveKineticDamageResistanceBonus() == -100	)
		{
			shipResonance = m_pilot->GetShip()->shieldKineticDamageResonance();
			resistanceModifier = m_item->passiveKineticDamageResistanceBonus();
			newResonance = shipResonance / ( 1 + resistanceModifier / 100 );
			m_pilot->GetShip()->Set_shieldKineticDamageResonance( newResonance );
		}

		//Attribute passiveThermicDamageResistanceBonus
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


////////////////////////////////////////////////////////////
///////////////ShipModule Helper Functions//////////////////
////////////////////////////////////////////////////////////

bool ShipModule::IsTurret() {

	//check that the module is a member of the correct group

	if( m_item->groupID() == EVEDB::invGroups::Energy_Weapon		||
		m_item->groupID() == EVEDB::invGroups::Projectile_Weapon	||
		m_item->groupID() == EVEDB::invGroups::Hybrid_Weapon		)
	{
		return true;

	}

	return false;
	

}

bool ShipModule::IsLauncher() {

	//check that the module is a member of the correct group
	if( m_item->groupID() == EVEDB::invGroups::Missile_Launcher				||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Bomb		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Citadel		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Snowball	||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Cruise		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Rocket		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Siege		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Standard	||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Heavy		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Assault		||
		m_item->groupID() == EVEDB::invGroups::Missile_Launcher_Defender	)
	{
		return true;

	}

	return false;

}

bool ShipModule::IsMiner() {

	//check that the module is a member of the correct group
	if( m_item->groupID() == EVEDB::invGroups::Mining_Laser				||
		m_item->groupID() == EVEDB::invGroups::Strip_Miner				||
		m_item->groupID() == EVEDB::invGroups::Frequency_Mining_Laser	)
	{
		return true;
	}

	return false;

}

bool ShipModule::IsStripMiner() {

	if( m_item->groupID() == EVEDB::invGroups::Strip_Miner )
		return true;

	return false;

}

bool ShipModule::IsShieldExtender() {

	if( m_item->groupID() == EVEDB::invGroups::Shield_Extender )
		return true;

	return false;
}

bool ShipModule::IsAutomatedTargetingSystem() {

	if( m_item->groupID() == EVEDB::invGroups::Automated_Targeting_System )
		return true;

	return false;
}

bool ShipModule::AffectsArmor() {

	if( m_item->groupID() == EVEDB::invGroups::Armor_Coating			||
		m_item->groupID() == EVEDB::invGroups::Armor_Reinforcer			||
		m_item->groupID() == EVEDB::invGroups::Armor_Plating_Energized	||
		m_item->groupID() == EVEDB::invGroups::Armor_Repair_Projector	||
		m_item->groupID() == EVEDB::invGroups::Armor_Repair_Unit		||
		m_item->groupID() == EVEDB::invGroups::Damage_Control			||
		m_item->groupID() == EVEDB::invGroups::Armor_Hardener			)
	{
		return true;
	}

	return false;
}

bool ShipModule::AffectsShield() {
	
	if( m_item->groupID() == EVEDB::invGroups::Shield_Amplifier		||
		m_item->groupID() == EVEDB::invGroups::Shield_Disruptor		||
		m_item->groupID() == EVEDB::invGroups::Shield_Extender		||
		m_item->groupID() == EVEDB::invGroups::Shield_Booster		||
		m_item->groupID() == EVEDB::invGroups::Shield_Hardener		||
		m_item->groupID() == EVEDB::invGroups::Shield_Recharger		||
		m_item->groupID() == EVEDB::invGroups::Damage_Control		||
		m_item->groupID() == EVEDB::invGroups::Shield_Transporter	)
	{
		return true;
	}

	return false;
}

bool ShipModule::AffectsHull() {

	if( m_item->groupID() == EVEDB::invGroups::Hull_Mods		||
		m_item->groupID() == EVEDB::invGroups::Damage_Control	||
		m_item->groupID() == EVEDB::invGroups::Hull_Repair_Unit )
	{
		return true;
	}

	return false;
}

bool ShipModule::AffectsDrones() {

	if( m_item->groupID() == EVEDB::invGroups::Drone_Control_Range_Module	||
		m_item->groupID() == EVEDB::invGroups::Drone_Control_Unit			||
		m_item->groupID() == EVEDB::invGroups::Drone_Damage_Modules			||
		m_item->groupID() == EVEDB::invGroups::Drone_Modules				||
		m_item->groupID() == EVEDB::invGroups::Drone_Navigation_Computer	||
		m_item->groupID() == EVEDB::invGroups::Drone_Tracking_Modules		||
		m_item->groupID() == EVEDB::invGroups::DroneBayExpander				)
	{
		return true;
	}

	return false;
}

bool ShipModule::IsInertialStabilizer() {

	if( m_item->groupID() == EVEDB::invGroups::Inertial_Stabilizer )
		return true;

	return false;
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

/* TODO LIST
 * -Handle skill modifiers
 * -Handle attribute affects
 * -Handle deactivation effects
 */

//Contributors Mobbel, DanTheBanjoMan, Deer_Hunter
