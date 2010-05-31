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
    Author:     Bloody.Rabbit
*/

#include "EVEServerPCH.h"

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
: InventoryItem(_factory, _shipID, _shipType, _data) {}

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
    return ShipRef( new Ship( factory, shipID, shipType, data ) );
}

ShipRef Ship::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    uint32 shipID = Ship::_Spawn( factory, data );
    if( shipID == 0 )
        return ShipRef();
    return Ship::Load( factory, shipID );
}

uint32 Ship::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
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

    return InventoryItem::_Load();
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
        case flagCargoHold:     return capacity();
        case flagDroneBay:      return droneCapacity();
        case flagShipHangar:    return shipMaintenanceBayCapacity();
        case flagHangar:        return corporateHangarCapacity();
        default:                return 0.0;
    }
}

void Ship::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item, Client *c)
{
	CharacterRef character = c->GetChar();
	
	if( flag == flagDroneBay )
    {
        if( item->categoryID() != EVEDB::invCategories::Drone )
            //Can only put drones in drone bay
            throw PyException( MakeUserError( "ItemCannotBeInDroneBay" ) );
    }
    else if( flag == flagShipHangar )
    {
		if( !c->GetShip()->attributes.Attr_hasShipMaintenanceBay )
            // We have no ship maintenance bay
			throw PyException( MakeCustomError( "%s has no ship maintenance bay.", item->itemName().c_str() ) );
        if( item->categoryID() != EVEDB::invCategories::Ship )
            // Only ships may be put here
            throw PyException( MakeCustomError( "Only ships may be placed into ship maintenance bay." ) );
    }
    else if( flag == flagHangar )
    {
		if( !c->GetShip()->attributes.Attr_hasCorporateHangars )
            // We have no corporate hangars
            throw PyException( MakeCustomError( "%s has no corporate hangars.", item->itemName().c_str() ) );
    }
    else if( flag == flagCargoHold )
	{
		//get all items in cargohold
		uint32 capacityUsed = 0;
		std::vector<InventoryItemRef> items;
		c->GetShip()->FindByFlag(flag, items);
		for(uint32 i = 0; i < items.size(); i++){
			capacityUsed += items[i]->volume();
		}
		if( capacityUsed + item->volume() > c->GetShip()->capacity() )
			throw PyException( MakeCustomError( "Not enough cargo space!") );

	}
	else if( flag > flagLowSlot0  &&  flag < flagHiSlot7 )
	{
		if(!Skill::FitModuleSkillCheck(item, character))
			throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
		if(item->categoryID() == EVEDB::invCategories::Charge) {
			InventoryItemRef module;
			c->GetShip()->FindSingleByFlag(flag, module);
			//Not sure if modules have to be online to recieve ammo
			/*
			if(!module->isOnline())
				throw PyException( MakeCustomError( "This module is not online" ) );
			*/
			if(module->chargeSize() != item->chargeSize() )
				throw PyException( MakeCustomError( "The charge is not the correct size for this module." ) );
			if(module->chargeGroup1() != item->groupID())
				throw PyException( MakeCustomError( "Incorrect charge type for this module.") );
		}
	}
	else if( flag > flagRigSlot0  &&  flag < flagRigSlot7 )
	{
		if(!Skill::FitModuleSkillCheck(item, character))
			throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
		if(c->GetShip()->rigSize() != item->rigSize())
			throw PyException( MakeCustomError( "Your ship cannot fit this size module" ) );
		
		uint32 calibration = 0;
		std::vector<InventoryItemRef> items;
		c->GetShip()->FindByFlagRange(flagRigSlot0, flagRigSlot7, items);
		for(uint32 i = 0; i < items.size(); i++ ) {
			calibration += items[i]->upgradeCost();
		}
		if( calibration + item->upgradeCost() > c->GetShip()->upgradeCapacity() )
			throw PyException( MakeCustomError( "Your ship cannot handle the extra calibration" ) );
	}
	else if( flag > flagSubSystem0  &&  flag < flagSubSystem7 )
	{
		if(!Skill::FitModuleSkillCheck(item, character))
			throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
	}
	
}

PyObject *Ship::ShipGetInfo()
{
    if( !LoadContents( m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the ship.
    if( !Populate( entry ) )
        return NULL;    //print already done.

    //hacking:
    //maximumRangeCap
    entry.attributes[ 797 ] = new PyFloat( 250000.000000 );

    result.items[ itemID() ] = entry.Encode();

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
            result.items[ (*cur)->itemID() ] = entry.Encode();
    }

    return result.Encode();
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
	
	//Primary Skill
	if(!ship->requiredSkill1() == 0)
	{
		requiredSkill = character->GetSkill( ship->requiredSkill1() );
		if( !requiredSkill )
			return false;

		if( ship->requiredSkill1Level() > requiredSkill->skillLevel() )
			return false;
	}

	//Secondary Skill
	if(!ship->requiredSkill2() == 0)
	{
		requiredSkill = character->GetSkill( ship->requiredSkill2() );
		if( !requiredSkill )
			return false;

		if( ship->requiredSkill2Level() > requiredSkill->skillLevel() )
			return false;
	}
	
	//Tertiary Skill
	if(!ship->requiredSkill3() == 0)
	{
		requiredSkill = character->GetSkill( ship->requiredSkill3() );
		if( !requiredSkill )
			return false;

		if( ship->requiredSkill3Level() > requiredSkill->skillLevel() )
			return false;
	}
	
	//Quarternary Skill
	if(!ship->requiredSkill4() == 0)
	{
		requiredSkill = character->GetSkill( ship->requiredSkill4() );
		if( !requiredSkill )
			return false;

		if( ship->requiredSkill4Level() > requiredSkill->skillLevel() )
			return false;
	}
	
	//Quinary Skill
	if(!ship->requiredSkill5() == 0)
	{
		requiredSkill = character->GetSkill( ship->requiredSkill5() );
		if( !requiredSkill )
			return false;

		if( ship->requiredSkill5Level() > requiredSkill->skillLevel() )
			return false;
	}
	
	//Senary Skill
	if(!ship->requiredSkill6() == 0)
	{
		requiredSkill = character->GetSkill( ship->requiredSkill6() );
		if( !requiredSkill )
			return false;

		if( ship->requiredSkill6Level() > requiredSkill->skillLevel() )
			return false;
	}

	return true;
}
