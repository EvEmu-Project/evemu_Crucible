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
        case flagCargoHold:             return capacity();
        case flagDroneBay:              return droneCapacity();
        case flagShipHangar:            return shipMaintenanceBayCapacity();
        case flagHangar:                return corporateHangarCapacity();
        case flagSpecializedFuelBay:    return fuelCargoCapacity();
        default:                        return 0.0;
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
		if(!ValidateItemSpecifics(c,item))
			throw PyException( MakeCustomError( "Your ship cannot equip this module" ) );
		if(item->categoryID() == EVEDB::invCategories::Charge) {
			InventoryItemRef module;
			c->GetShip()->FindSingleByFlag(flag, module);
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
		if( c->GetShip()->upgradeLoad() + item->upgradeCost() > c->GetShip()->upgradeCapacity() )
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

bool Ship::ValidateItemSpecifics(Client *c, InventoryItemRef equip) {

	//declaring explicitly as int...not sure if this is needed or not
	int groupID = c->GetShip()->groupID();
	int typeID = c->GetShip()->typeID();
	int canFitShipGroup1 = equip->canFitShipGroup1();
	int canFitShipGroup2 = equip->canFitShipGroup2();
	int canFitShipGroup3 = equip->canFitShipGroup3();
	int canFitShipGroup4 = equip->canFitShipGroup4();
	int canFitShipType1 = equip->canFitShipType1();
	int canFitShipType2 = equip->canFitShipType2();
	int canFitShipType3 = equip->canFitShipType3();
	int canFitShipType4 = equip->canFitShipType4();

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

	return true;

}


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

void ShipEntity::Process() {
	SystemEntity::Process();
}

void ShipEntity::ForcedSetPosition(const GPoint &pt) {
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
		item->head.sub_type = AddBallSubType_orbitingNPC;
		
		item->mass.mass = m_self->mass();
		item->mass.unknown51 = 0;
		item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		item->mass.corpID = GetCorporationID();
		item->mass.unknown64 = 0xFFFFFFFF;
		
		item->ship.max_speed = m_self->maxVelocity();
		item->ship.velocity_x = m_self->maxVelocity();	//hacky hacky
		item->ship.velocity_y = 0.0;
		item->ship.velocity_z = 0.0;
		item->ship.agility = 1.0;	//hacky
		item->ship.speed_fraction = 0.133f;	//just strolling around. TODO: put in speed fraction!
		
		item->main.unknown116 = 0xFF;
		item->main.followID = m_orbitingID;
		item->main.followRange = 6000.0f;
		
		item->name.name_len = slen;	// in number of unicode chars
		//strcpy_fake_unicode(item->name.name, GetName());
	} else */{
        BallHeader head;
		head.entityID = GetID();
		head.mode = Destiny::DSTBALL_STOP;
		head.radius = GetRadius();
		head.x = position.x;
		head.y = position.y;
		head.z = position.z;
		head.sub_type = AddBallSubType_orbitingNPC;
        into.Append( head );

        MassSector mass;
		mass.mass = GetMass();
		mass.cloak = 0;
		mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		mass.corpID = GetCorporationID();
		mass.allianceID = GetAllianceID();
        into.Append( mass );

        ShipSector ship;
		ship.max_speed = GetMaxVelocity();
		ship.velocity_x = 0.0;
		ship.velocity_y = 0.0;
		ship.velocity_z = 0.0;
        ship.unknown_x = 0.0;
        ship.unknown_y = 0.0;
        ship.unknown_z = 0.0;
		ship.agility = GetAgility();
		ship.speed_fraction = 0.0;
        into.Append( ship );

        DSTBALL_STOP_Struct main;
		main.formationID = 0xFF;
        into.Append( main );

        const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
        into.Append( nameLen );

        const Buffer::iterator<uint16> name = into.end<uint16>();
        into.ResizeAt( name, nameLen );
        utf8::utf8to16( itemName.begin(), itemName.end(), name );
	}
}

void ShipEntity::MakeDamageState(DoDestinyDamageState &into) const {
	into.shield = m_shieldCharge / m_self->shieldCapacity();
	into.tau = 100000;	//no freakin clue.
	into.timestamp = Win32TimeNow();
	into.armor = 1.0 - (m_armorDamage / m_self->armorHP());
	into.structure = 1.0 - (m_hullDamage / m_self->hp());
}

