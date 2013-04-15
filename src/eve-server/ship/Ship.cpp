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
    Author:     Bloody.Rabbit
*/

#include "eve-server.h"

#include "character/Character.h"
#include "ship/DestinyManager.h"
#include "ship/Ship.h"
#include "ship/ShipOperatorInterface.h"
#include "system/BubbleManager.h"

/*
 * ShipTypeData
 */
ShipTypeData::ShipTypeData( uint32 weaponTypeID, uint32 miningTypeID, uint32 skillTypeID) : mWeaponTypeID(weaponTypeID),
    mMiningTypeID(miningTypeID), mSkillTypeID(skillTypeID) {}
/*
 * ShipType
 */
 ShipType::ShipType(
    uint32 _id,
    // ItemType stuff:
    const ItemGroup &_group,
    const TypeData &_data,
    // ShipType stuff:
    const ItemType *_weaponType,
    const ItemType *_miningType,
    const ItemType *_skillType,
    const ShipTypeData &stData)
: ItemType(_id, _group, _data),
  m_weaponType(_weaponType),
  m_miningType(_miningType),
  m_skillType(_skillType)
 {
    // data consistency checks:
    if(_weaponType != NULL)
        assert(_weaponType->id() == stData.mWeaponTypeID);
    if(_miningType != NULL)
        assert(_miningType->id() == stData.mMiningTypeID);
    if(_skillType != NULL)
        assert(_skillType->id() == stData.mSkillTypeID);
}

ShipType *ShipType::Load(ItemFactory &factory, uint32 shipTypeID)
{
    return ItemType::Load<ShipType>( factory, shipTypeID );
}

template<class _Ty>
_Ty *ShipType::_LoadShipType(ItemFactory &factory, uint32 shipTypeID,
    // ItemType stuff:
    const ItemGroup &group, const TypeData &data,
    // ShipType stuff:
    const ItemType *weaponType, const ItemType *miningType, const ItemType *skillType, const ShipTypeData &stData)
{
    // we have all the data, let's create new object
    return new ShipType(shipTypeID, group, data, weaponType, miningType, skillType, stData );
}

/*
 * Ship
 */
Ship::Ship(
    ItemFactory &_factory,
    uint32 _shipID,
    // InventoryItem stuff:
    const ShipType &_shipType,
    const ItemData &_data)
: InventoryItem(_factory, _shipID, _shipType, _data)
{
    m_ModuleManager = NULL;
    m_pOperator = new ShipOperatorInterface();

    // Activate Save Info Timer with somewhat randomized timer value:
    //SetSaveTimerExpiry( MakeRandomInt( (10 * 60), (15 * 60) ) );        // Randomize save timer expiry to between 10 and 15 minutes
    //DisableSaveTimer();
}

ShipRef Ship::Load(ItemFactory &factory, uint32 shipID)
{
    return InventoryItem::Load<Ship>( factory, shipID );
}

template<class _Ty>
RefPtr<_Ty> Ship::_LoadShip(ItemFactory &factory, uint32 shipID,
    // InventoryItem stuff:
    const ShipType &shipType, const ItemData &data)
{
    // we don't need any additional stuff
    return ShipRef( new Ship(factory, shipID, shipType, data ) );
}

ShipRef Ship::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 shipID = Ship::_Spawn( factory, data );
    if( shipID == 0 )
        return ShipRef();

    ShipRef sShipRef = Ship::Load( factory, shipID );

    // Create default dynamic attributes in the AttributeMap:
    sShipRef->SetAttribute(AttrIsOnline,            1, true);												// Is Online
    sShipRef->SetAttribute(AttrShieldCharge,        sShipRef->GetAttribute(AttrShieldCapacity), true);		// Shield Charge
    sShipRef->SetAttribute(AttrArmorDamage,         0.0, true);												// Armor Damage
    sShipRef->SetAttribute(AttrMass,                sShipRef->type().attributes.mass(), true);				// Mass
    sShipRef->SetAttribute(AttrRadius,              sShipRef->type().attributes.radius(), true);			// Radius
    sShipRef->SetAttribute(AttrVolume,              sShipRef->type().attributes.volume(), true);			// Volume
    sShipRef->SetAttribute(AttrCapacity,            sShipRef->type().attributes.capacity(), true);			// Capacity
    sShipRef->SetAttribute(AttrInertia,             1, true);												// Inertia
    sShipRef->SetAttribute(AttrCharge,              sShipRef->GetAttribute(AttrCapacitorCapacity), true);	// Set Capacitor Charge to the Capacitor Capacity

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
	// Hull Damage
	if( !(sShipRef->HasAttribute(AttrDamage)) )
        sShipRef->SetAttribute(AttrDamage, 0, true );
    // Theoretical Maximum Targeting Range
    if( !(sShipRef->HasAttribute(AttrMaximumRangeCap)) )
        sShipRef->SetAttribute(AttrMaximumRangeCap, ((double)BUBBLE_RADIUS_METERS), true );
    // Maximum Armor Damage Resonance
    if( !(sShipRef->HasAttribute(AttrArmorMaxDamageResonance)) )
        sShipRef->SetAttribute(AttrArmorMaxDamageResonance, 1.0f, true);
    // Maximum Shield Damage Resonance
    if( !(sShipRef->HasAttribute(AttrShieldMaxDamageResonance)) )
        sShipRef->SetAttribute(AttrShieldMaxDamageResonance, 1.0f, true);
    // Warp Speed Multiplier
    if( !(sShipRef.get()->HasAttribute(AttrWarpSpeedMultiplier)) )
        sShipRef.get()->SetAttribute(AttrWarpSpeedMultiplier, 1.0f, true);
    // CPU Load of the ship (new ships have zero load with no modules fitted, of course):
    if( !(sShipRef.get()->HasAttribute(AttrCpuLoad)) )
        sShipRef.get()->SetAttribute(AttrCpuLoad, 0, true);
    // Power Load of the ship (new ships have zero load with no modules fitted, of course):
    if( !(sShipRef.get()->HasAttribute(AttrPowerLoad)) )
        sShipRef.get()->SetAttribute(AttrPowerLoad, 0, true);
	// Warp Scramble Status of the ship (most ships have zero warp scramble status, but some already have it defined):
	if( !(sShipRef.get()->HasAttribute(AttrWarpScrambleStatus)) )
		sShipRef.get()->SetAttribute(AttrWarpScrambleStatus, 0.0, true);

	// Shield Resonance
	// AttrShieldEmDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrShieldEmDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldEmDamageResonance, 1.0, true);
	// AttrShieldExplosiveDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrShieldExplosiveDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, true);
	// AttrShieldKineticDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrShieldKineticDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldKineticDamageResonance, 1.0, true);
	// AttrShieldThermalDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrShieldThermalDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldThermalDamageResonance, 1.0, true);

	// Armor Resonance
	// AttrArmorEmDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrArmorEmDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorEmDamageResonance, 1.0, true);
	// AttrArmorExplosiveDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrArmorExplosiveDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, true);
	// AttrArmorKineticDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrArmorKineticDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorKineticDamageResonance, 1.0, true);
	// AttrArmorThermalDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrArmorThermalDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorThermalDamageResonance, 1.0, true);

	// Hull Resonance
	// AttrHullEmDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrHullEmDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullEmDamageResonance, 1.0, true);
	// AttrHullExplosiveDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrHullExplosiveDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullExplosiveDamageResonance, 1.0, true);
	// AttrHullKineticDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrHullKineticDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullKineticDamageResonance, 1.0, true);
	// AttrHullThermalDamageResonance
	if( !(sShipRef.get()->HasAttribute(AttrHullThermalDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullThermalDamageResonance, 1.0, true);

	// AttrTurretSlotsLeft
	if( !(sShipRef.get()->HasAttribute(AttrTurretSlotsLeft)) )
		sShipRef.get()->SetAttribute(AttrTurretSlotsLeft, 0, true);
	// AttrLauncherSlotsLeft
	if( !(sShipRef.get()->HasAttribute(AttrLauncherSlotsLeft)) )
		sShipRef.get()->SetAttribute(AttrLauncherSlotsLeft, 0, true);

    return sShipRef;
}

uint32 Ship::_Spawn(ItemFactory &factory, ItemData &data) {
    // make sure it's a ship
    const ShipType *st = factory.GetShipType(data.typeID);
    if(st == NULL)
        return 0;

    // store item data
    uint32 shipID = InventoryItem::_Spawn(factory, data);
    if(shipID == 0)
        return 0;

    // nothing additional

    return shipID;
}

bool Ship::_Load()
{
    // load contents
    if( !LoadContents( m_factory ) )
        return false;

    bool loadSuccess = InventoryItem::_Load();      // Attributes are loaded here!

    // TODO: MOVE THIS TO Ship::Load() or some other place AFTER InventoryItem::mAttributeMap has been loaded
    // allocate the module manager, only the first time:
    if( m_ModuleManager == NULL )
        m_ModuleManager = new ModuleManager(this);

    // check for module manager load success
    if( m_ModuleManager == NULL )
        loadSuccess = false;

    return loadSuccess;
}

void Ship::Delete()
{
    // delete contents first
    DeleteContents( m_factory );

    InventoryItem::Delete();
}

double Ship::GetCapacity(EVEItemFlags flag) const
{
    switch( flag ) {
        // the .get_float() part is a evil hack.... as this function should return a EvilNumber.
        case flagAutoFit:
        case flagCargoHold:     return GetAttribute(AttrCapacity).get_float();
        case flagDroneBay:      return GetAttribute(AttrDroneCapacity).get_float();
        case flagShipHangar:    return GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
        case flagHangar:        return GetAttribute(AttrCorporateHangarCapacity).get_float();
        default:                return 0.0;
    }
}

void Ship::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item)
{
    CharacterRef character = m_pOperator->GetChar();        // Operator assumed to be Client *

    if( flag == flagDroneBay )
    {
        if( item->categoryID() != EVEDB::invCategories::Drone )
            //Can only put drones in drone bay
            throw PyException( MakeUserError( "ItemCannotBeInDroneBay" ) );
    }
    else if( flag == flagShipHangar )
    {
        if( m_pOperator->GetShip()->GetAttribute(AttrHasShipMaintenanceBay ) != 0)      // Operator assumed to be Client *
            // We have no ship maintenance bay
            throw PyException( MakeCustomError( "%s has no ship maintenance bay.", item->itemName().c_str() ) );
        if( item->categoryID() != EVEDB::invCategories::Ship )
            // Only ships may be put here
            throw PyException( MakeCustomError( "Only ships may be placed into ship maintenance bay." ) );
    }
    else if( flag == flagHangar )
    {
        if( m_pOperator->GetShip()->GetAttribute(AttrHasCorporateHangars ) != 0)        // Operator assumed to be Client *
            // We have no corporate hangars
            throw PyException( MakeCustomError( "%s has no corporate hangars.", item->itemName().c_str() ) );
    }
    else if( flag == flagCargoHold )
    {
        //get all items in cargohold
        EvilNumber capacityUsed(0);
        std::vector<InventoryItemRef> items;
        m_pOperator->GetShip()->FindByFlag(flag, items);        // Operator assumed to be Client *
        for(uint32 i = 0; i < items.size(); i++){
            capacityUsed += items[i]->GetAttribute(AttrVolume);
        }
        if( capacityUsed + item->GetAttribute(AttrVolume) > m_pOperator->GetShip()->GetAttribute(AttrCapacity) )    // Operator assumed to be Client *
            throw PyException( MakeCustomError( "Not enough cargo space!") );
    }
    else if( (flag >= flagLowSlot0)  &&  (flag <= flagHiSlot7) )
    {
        if( m_pOperator->IsClient() )
            if(!Skill::FitModuleSkillCheck(item, character))        // SKIP THIS SKILL CHECK if Operator is NOT Client *
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
        if(!ValidateItemSpecifics(item))
            throw PyException( MakeCustomError( "Your ship cannot equip this module" ) );
        if( m_ModuleManager->IsSlotOccupied(flag) )
            throw PyException( MakeUserError( "SlotAlreadyOccupied" ) );
        if(item->categoryID() == EVEDB::invCategories::Charge) {
            InventoryItemRef module;
            m_pOperator->GetShip()->FindSingleByFlag(flag, module);     // Operator assumed to be Client *
            if(module->GetAttribute(AttrChargeSize) != item->GetAttribute(AttrChargeSize) )
                throw PyException( MakeCustomError( "The charge is not the correct size for this module." ) );
            if(module->GetAttribute(AttrChargeGroup1) != item->groupID())
                throw PyException( MakeCustomError( "Incorrect charge type for this module.") );
        }
    }
    else if( (flag >= flagRigSlot0)  &&  (flag <= flagRigSlot7) )
    {
        if( m_pOperator->IsClient() )
            if(!Skill::FitModuleSkillCheck(item, character))        // SKIP THIS SKILL CHECK if Operator is NOT Client *
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
        if(m_pOperator->GetShip()->GetAttribute(AttrRigSize) != item->GetAttribute(AttrRigSize))        // Operator assumed to be Client *
            throw PyException( MakeCustomError( "Your ship cannot fit this size module" ) );
        if( m_pOperator->GetShip()->GetAttribute(AttrUpgradeLoad) + item->GetAttribute(AttrUpgradeCost) > m_pOperator->GetShip()->GetAttribute(AttrUpgradeCapacity) )   // Operator assumed to be Client *
            throw PyException( MakeCustomError( "Your ship cannot handle the extra calibration" ) );
    }
    else if( (flag >= flagSubSystem0)  &&  (flag <= flagSubSystem7) )
    {
        if( m_pOperator->IsClient() )
            if(!Skill::FitModuleSkillCheck(item, character))        // SKIP THIS SKILL CHECK if Operator is NOT Client *
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
    }

}

PyDict *Ship::ShipGetInfo()
{
    if( !LoadContents( m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    PyDict *result = new PyDict;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the ship.
    if( !Populate( entry ) )
        return NULL;    //print already done.

    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));

    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    std::vector<InventoryItemRef> integrated;
    //find all the equipped items and rigs
    FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    FindByFlagRange( flagRigSlot0, flagRigSlot7, integrated );
    //append them into one list
    equipped.insert(equipped.end(), integrated.begin(), integrated.end() );
    //encode an entry for each one.
    std::vector<InventoryItemRef>::iterator cur, end;
    cur = equipped.begin();
    end = equipped.end();
    for(; cur != end; cur++)
    {
        if( !(*cur)->Populate( entry ) )
        {
            codelog( ITEM__ERROR, "%s (%u): Failed to load item %u for ShipGetInfo", itemName().c_str(), itemID(), (*cur)->itemID() );
        }
        else
            result->SetItem(new PyInt((*cur)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
    }

    return result;
}

PyDict *Ship::ShipGetState()
{
    if( !LoadContents( m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

	// Create new dictionary for "shipState":
    PyDict *result = new PyDict;

	// Create entry in "shipState" dictionary for Ship itself:
    result->SetItem(new PyInt(itemID()), GetItemStatusRow());

	// Create entries in "shipState" dictionary for ALL modules, rigs, and subsystems present on ship:
	std::vector<InventoryItemRef> moduleList;
	m_ModuleManager->GetModuleListOfRefs( &moduleList );

	for(int i=0; i<moduleList.size(); i++)
		result->SetItem(new PyInt(moduleList.at(i)->itemID()), moduleList.at(i)->GetItemStatusRow());

	return result;
}

void Ship::AddItem(InventoryItemRef item)
{
    InventoryEx::AddItem( item );

    if( item->flag() >= flagSlotFirst &&
        item->flag() <= flagSlotLast &&
        item->categoryID() != EVEDB::invCategories::Charge)
    {
        // make singleton
        item->ChangeSingleton( true );
    }
}

bool Ship::ValidateBoardShip(ShipRef ship, CharacterRef character)
{

    SkillRef requiredSkill;
    uint32 skillTypeID = 0;

    if( (skillTypeID = static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill1).get_int())) != 0)
        if( !(character->HasSkillTrainedToLevel( skillTypeID, static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill1Level).get_int()) )) )
            return false;

    if( (skillTypeID = static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill2).get_int())) != 0)
        if( !(character->HasSkillTrainedToLevel( skillTypeID, static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill2Level).get_int() ))) )
            return false;

    if( (skillTypeID = static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill3).get_int())) != 0)
        if( !(character->HasSkillTrainedToLevel( skillTypeID, static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill3Level).get_int() ))) )
            return false;

    if( (skillTypeID = static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill4).get_int())) != 0)
        if( !(character->HasSkillTrainedToLevel( skillTypeID, static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill4Level).get_int() ))) )
            return false;

    if( (skillTypeID = static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill5).get_int())) != 0)
        if( !(character->HasSkillTrainedToLevel( skillTypeID, static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill5Level).get_int() )) ))
            return false;

    if( (skillTypeID = static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill6).get_int())) != 0)
        if( !(character->HasSkillTrainedToLevel( skillTypeID, static_cast<uint32>(ship->GetAttribute(AttrRequiredSkill6Level).get_int() )) ))
            return false;

    return true;
/*
    //Primary Skill
    if(ship->GetAttribute(AttrRequiredSkill1).get_int() != 0)
    {
        requiredSkill = character->GetSkill( ship->GetAttribute(AttrRequiredSkill1).get_int() );
        if( !requiredSkill )
            return false;

        if( ship->GetAttribute(AttrRequiredSkill1Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

    //Secondary Skill
    if(ship->GetAttribute(AttrRequiredSkill2).get_int() != 0)
    {
        requiredSkill = character->GetSkill( ship->GetAttribute(AttrRequiredSkill2).get_int() );
        if( !requiredSkill )
            return false;

        if( ship->GetAttribute(AttrRequiredSkill2Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

    //Tertiary Skill
    if(ship->GetAttribute(AttrRequiredSkill3).get_int() != 0)
    {
        requiredSkill = character->GetSkill( ship->GetAttribute(AttrRequiredSkill3).get_int() );
        if( !requiredSkill )
            return false;

        if( ship->GetAttribute(AttrRequiredSkill3Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

    //Quarternary Skill
    if(ship->GetAttribute(AttrRequiredSkill4).get_int() != 0)
    {
        requiredSkill = character->GetSkill( ship->GetAttribute(AttrRequiredSkill4).get_int() );
        if( !requiredSkill )
            return false;

        if( ship->GetAttribute(AttrRequiredSkill4Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

    //Quinary Skill
    if(ship->GetAttribute(AttrRequiredSkill5).get_int() != 0)
    {
        requiredSkill = character->GetSkill( ship->GetAttribute(AttrRequiredSkill5).get_int() );
        if( !requiredSkill )
            return false;

        if( ship->GetAttribute(AttrRequiredSkill5Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

    //Senary Skill
    if(ship->GetAttribute(AttrRequiredSkill6).get_int() != 0)
    {
        requiredSkill = character->GetSkill( ship->GetAttribute(AttrRequiredSkill6).get_int() );
        if( !requiredSkill )
            return false;

        if( ship->GetAttribute(AttrRequiredSkill6Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

    return true;
*/
}

void Ship::SaveShip()
{
    sLog.Debug( "Ship::SaveShip()", "Saving all 'entity' info and attribute info to DB for ship %s (%u)...", itemName().c_str(), itemID() );

    SaveItem();                         // Save all attributes and item info
    m_ModuleManager->SaveModules();     // Save all attributes and item info for all modules fitted to this ship
}

bool Ship::ValidateItemSpecifics(InventoryItemRef equip) {

    //declaring explicitly as int...not sure if this is needed or not
    int groupID = m_pOperator->GetShip()->groupID();
    int typeID = m_pOperator->GetShip()->typeID();
    EvilNumber canFitShipGroup1 = equip->GetAttribute(AttrCanFitShipGroup1);
    EvilNumber canFitShipGroup2 = equip->GetAttribute(AttrCanFitShipGroup2);
    EvilNumber canFitShipGroup3 = equip->GetAttribute(AttrCanFitShipGroup3);
    EvilNumber canFitShipGroup4 = equip->GetAttribute(AttrCanFitShipGroup4);
    EvilNumber canFitShipType1 = equip->GetAttribute(AttrCanFitShipType1);
    EvilNumber canFitShipType2 = equip->GetAttribute(AttrCanFitShipType2);
    EvilNumber canFitShipType3 = equip->GetAttribute(AttrCanFitShipType3);
    EvilNumber canFitShipType4 = equip->GetAttribute(AttrCanFitShipType4);

	if( canFitShipGroup1 != 0 || canFitShipGroup2 != 0 || canFitShipGroup3 != 0 || canFitShipGroup4 != 0 )
		if( canFitShipGroup1 != groupID && canFitShipGroup2 != groupID && canFitShipGroup3 != groupID && canFitShipGroup4 != groupID )
			return false;
	/*
    if( canFitShipGroup1 != 0 )
        if( canFitShipGroup1 != groupID )
            return false;

    if( canFitShipGroup2 != 0 )
        if( canFitShipGroup2 != groupID )
            return false;

    if( canFitShipGroup3 != 0 )
        if( canFitShipGroup3 != groupID )
            return false;

    if( canFitShipGroup4 != 0 )
        if( canFitShipGroup4 != groupID )
            return false;
	*/

    if( canFitShipType1 != 0 || canFitShipType2 != 0 || canFitShipType3 != 0 || canFitShipType4 != 0 )
        if( canFitShipType1 != typeID && canFitShipType2 != typeID && canFitShipType3 != typeID && canFitShipType4 != typeID )
            return false;
	/*
    if( canFitShipType1 != 0 )
        if( canFitShipType1 != typeID )
            return false;

    if( canFitShipType2 != 0 )
        if( canFitShipType2 != typeID )
            return false;

    if( canFitShipType3 != 0 )
        if( canFitShipType3 != typeID )
            return false;

    if( canFitShipType4 != 0 )
        if( canFitShipType4 != typeID )
            return false;
	*/
    return true;

}

/* Begin new Module Manager Interface */
uint32 Ship::FindAvailableModuleSlot( InventoryItemRef item )
{
	uint32 slotFound = flagIllegal;

    // 1) get slot bank (low, med, high, rig, subsystem) from dgmTypeEffects using item->itemID()
    // 2) query this ship's ModuleManager to determine if there are any free slots in that bank,
    //    it should return a slot flag number for the next available slot starting at the lowest number
    //    for that bank
    // 3) return that slot flag number

	if( item->type().HasEffect(Effect_loPower) )
	{
		slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_loPower);
	}
	else if( item->type().HasEffect(Effect_medPower) )
	{
		slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_medPower);
	}
	else if( item->type().HasEffect(Effect_hiPower) )
	{
		slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_hiPower);
	}
	else if( item->type().HasEffect(Effect_subSystem) )
	{
		slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_subSystem);
	}
	else if( item->type().HasEffect(Effect_rigSlot) )
	{
		slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_rigSlot);
	}
	else
	{
		// ERROR: This is not a module that fits in any of the slot banks
	}

    return slotFound;
}

void Ship::AddItem(EVEItemFlags flag, InventoryItemRef item)
{

    ValidateAddItem( flag, item );

    //it's a new module, make sure it's state starts at offline so that it is added correctly
    if( item->categoryID() != EVEDB::invCategories::Charge )
        item->PutOffline();

    // TODO: Somehow, if this returns FALSE, the item->Move() above has to be "undone"... can we do the move AFTER attempting to fit?
    // what if we pass the flag into FitModule().... then if it returns true, the item->Move() can be called
    if( m_ModuleManager->FitModule(item, flag) )
        item->Move(m_pOperator->GetLocationID(), flag);  //TODO - check this
}

void Ship::RemoveItem(InventoryItemRef item, uint32 inventoryID, EVEItemFlags flag)
{
    //coming from ship, we need to deactivate it and remove mass if it isn't a charge
    if( item->categoryID() != EVEDB::invCategories::Charge ) {
        m_pOperator->GetShip()->Deactivate( item->itemID(), "online" );
        // m_pOperator->GetShip()->Set_mass( m_pOperator->GetShip()->mass() - item->massAddition() );
        //m_pOperator->GetShip()->SetAttribute(AttrMass,  m_pOperator->GetShip()->GetAttribute(AttrMass) - item->GetAttribute(AttrMassAddition) );
        m_pOperator->GetShip()->UnloadModule( item->itemID() );
    }

    //Move New item to its new location
    m_pOperator->MoveItem(item->itemID(), inventoryID, flag);
}

void Ship::UpdateModules()
{

}

void Ship::UnloadModule(uint32 itemID)
{
    m_ModuleManager->UnfitModule(itemID);
}

void Ship::UnloadAllModules()
{

}

void Ship::RepairModules()
{

}

void Ship::Online (uint32 moduleID)
{
	m_ModuleManager->Online(moduleID);
}

void Ship::Offline (uint32 moduleID)
{
	m_ModuleManager->Offline(moduleID);
}

int32 Ship::Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat)
{
    return m_ModuleManager->Activate( itemID, effectName, targetID, repeat );
}

void Ship::Deactivate(int32 itemID, std::string effectName)
{
    m_ModuleManager->Deactivate(itemID, effectName);
}

void Ship::Overload()
{

}

void Ship::CancelOverloading()
{

	}

void Ship::RemoveRig( InventoryItemRef item, uint32 inventoryID )
{
    m_ModuleManager->UninstallRig(item->itemID());

    //move the item to the void or w/e
    m_pOperator->MoveItem(item->itemID(), inventoryID, flagAutoFit);

    //delete the item
    item->Delete();
}

void Ship::Process()
{
    m_ModuleManager->Process();
}

void Ship::OnlineAll()
{
    m_ModuleManager->OnlineAll();
}

void Ship::ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge)
{

}

void Ship::DeactivateAllModules()
{
    m_ModuleManager->DeactivateAllModules();
}

std::vector<GenericModule *> Ship::GetStackedItems(uint32 typeID, ModulePowerLevel level)
{
    return m_ModuleManager->GetStackedItems(typeID, level);
}

/* End new Module Manager Interface */

using namespace Destiny;

ShipEntity::ShipEntity(
    ShipRef ship,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), ship),
  m_system(system),
  m_services(services)
{
    _shipRef = ship;
    m_destiny->SetPosition(position, false);
}

ShipEntity::~ShipEntity()
{

}

void ShipEntity::Process()
{
    SystemEntity::Process();
}

void ShipEntity::ForcedSetPosition( const GPoint &pt ) {
    m_destiny->SetPosition(pt, false);
}

void ShipEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

    /*if(m_orbitingID != 0) {
        #pragma pack(1)
        struct AddBall_Orbit {
            BallHeader head;
            MassSector mass;
            ShipSector ship;
            DSTBALL_ORBIT_Struct main;
            NameStruct name;
        };
        #pragma pack()

        into.resize(start
            + sizeof(AddBall_Orbit)
            + slen*sizeof(uint16) );
        uint8 *ptr = &into[start];
        AddBall_Orbit *item = (AddBall_Orbit *) ptr;
        ptr += sizeof(AddBall_Orbit);

        item->head.entityID = GetID();
        item->head.mode = Destiny::DSTBALL_ORBIT;
        item->head.radius = m_self->radius();
        item->head.x = x();
        item->head.y = y();
        item->head.z = z();
        item->head.sub_type = IsMassive | IsFree;

        item->mass.mass = m_self->mass();
        item->mass.unknown51 = 0;
        item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
        item->mass.corpID = GetCorporationID();
        item->mass.unknown64 = 0xFFFFFFFF;

        item->ship.max_speed = m_self->maxVelocity();
        item->ship.velocity_x = m_self->maxVelocity();    //hacky hacky
        item->ship.velocity_y = 0.0;
        item->ship.velocity_z = 0.0;
        item->ship.agility = 1.0;    //hacky
        item->ship.speed_fraction = 0.133f;    //just strolling around. TODO: put in speed fraction!

        item->main.unknown116 = 0xFF;
        item->main.followID = m_orbitingID;
        item->main.followRange = 6000.0f;

        item->name.name_len = slen;    // in number of unicode chars
        //strcpy_fake_unicode(item->name.name, GetName());
    } else */{
        BallHeader head;
        head.entityID = GetID();
        head.mode = Destiny::DSTBALL_STOP;
        head.radius = GetRadius();
        head.x = position.x;
        head.y = position.y;
        head.z = position.z;
        head.sub_type = IsMassive | IsFree;
        into.Append( head );

        MassSector mass;
        mass.mass = GetMass();
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corpID = GetCorporationID();
        mass.allianceID = GetAllianceID();
        into.Append( mass );

        ShipSector ship;
        ship.max_speed = static_cast<float>(GetMaxVelocity());
        ship.velocity_x = 0.0;
        ship.velocity_y = 0.0;
        ship.velocity_z = 0.0;
        ship.agility = static_cast<float>(GetAgility());
        ship.speed_fraction = 0.0;
        into.Append( ship );

        DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    }
}

void ShipEntity::MakeDamageState(DoDestinyDamageState &into) const
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.tau = 100000;    //no freaking clue.
    into.timestamp = Win32TimeNow();
//    armor damage isn't working...
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHp).get_float());
}

