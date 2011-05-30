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
: InventoryItem(_factory, _shipID, _shipType, _data)
{
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
    return ShipRef( new Ship( factory, shipID, shipType, data ) );
}

ShipRef Ship::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    uint32 shipID = Ship::_Spawn( factory, data );
    if( shipID == 0 )
        return ShipRef();

    ShipRef sShipRef = Ship::Load( factory, shipID );

    // Create default dynamic attributes in the AttributeMap:
    sShipRef->SetAttribute(AttrIsOnline,            1);                                             // Is Online
    sShipRef->SetAttribute(AttrShieldCharge,        sShipRef->GetAttribute(AttrShieldCapacity));    // Shield Charge
    sShipRef->SetAttribute(AttrArmorDamage,         0.0);                                           // Armor Damage
    sShipRef->SetAttribute(AttrMass,                sShipRef->type().attributes.mass());            // Mass
    sShipRef->SetAttribute(AttrRadius,              sShipRef->type().attributes.radius());          // Radius
    sShipRef->SetAttribute(AttrVolume,              sShipRef->type().attributes.volume());          // Volume
    sShipRef->SetAttribute(AttrCapacity,            sShipRef->type().attributes.capacity());        // Capacity
    sShipRef->SetAttribute(AttrInertia,             1);                                             // Inertia
    sShipRef->SetAttribute(AttrCharge,              sShipRef->GetAttribute(AttrCapacitorCapacity)); // Set Capacitor Charge to the Capacitor Capacity

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
    // Maximum Range Capacitor
    if( !(sShipRef->HasAttribute(AttrMaximumRangeCap)) )
        sShipRef->SetAttribute(AttrMaximumRangeCap, 249999.0 );
    // Maximum Armor Damage Resonance
    if( !(sShipRef->HasAttribute(AttrArmorMaxDamageResonance)) )
        sShipRef->SetAttribute(AttrArmorMaxDamageResonance, 1.0f);
    // Maximum Shield Damage Resonance
    if( !(sShipRef->HasAttribute(AttrShieldMaxDamageResonance)) )
        sShipRef->SetAttribute(AttrShieldMaxDamageResonance, 1.0f);
    // Warp Speed Multiplier
    if( !(sShipRef.get()->HasAttribute(AttrWarpSpeedMultiplier)) )
        sShipRef.get()->SetAttribute(AttrWarpSpeedMultiplier, 1.0f);

    return sShipRef;
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
        /*case flagCargoHold:     return capacity();
        case flagDroneBay:      return droneCapacity();
        case flagShipHangar:    return shipMaintenanceBayCapacity();
        case flagHangar:        return corporateHangarCapacity();*/

        // the .get_float() part is a evil hack.... as this function should return a EvilNumber.
        case flagAutoFit:
        case flagCargoHold:     return GetAttribute(AttrCapacity).get_float();
        case flagDroneBay:      return GetAttribute(AttrDroneCapacity).get_float();
        case flagShipHangar:    return GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
        case flagHangar:        return GetAttribute(AttrCorporateHangarCapacity).get_float();
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
		if( c->GetShip()->GetAttribute(AttrHasShipMaintenanceBay ) != 0)
            // We have no ship maintenance bay
			throw PyException( MakeCustomError( "%s has no ship maintenance bay.", item->itemName().c_str() ) );
        if( item->categoryID() != EVEDB::invCategories::Ship )
            // Only ships may be put here
            throw PyException( MakeCustomError( "Only ships may be placed into ship maintenance bay." ) );
    }
    else if( flag == flagHangar )
    {
		if( c->GetShip()->GetAttribute(AttrHasCorporateHangars ) != 0)
            // We have no corporate hangars
            throw PyException( MakeCustomError( "%s has no corporate hangars.", item->itemName().c_str() ) );
    }
    else if( flag == flagCargoHold )
	{
		//get all items in cargohold
		EvilNumber capacityUsed(0);
		std::vector<InventoryItemRef> items;
		c->GetShip()->FindByFlag(flag, items);
		for(uint32 i = 0; i < items.size(); i++){
			capacityUsed += items[i]->GetAttribute(AttrVolume);
		}
		if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrCapacity) )
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
			if(module->GetAttribute(AttrChargeSize) != item->GetAttribute(AttrChargeSize) )
				throw PyException( MakeCustomError( "The charge is not the correct size for this module." ) );
			if(module->GetAttribute(AttrChargeGroup1) != item->groupID())
				throw PyException( MakeCustomError( "Incorrect charge type for this module.") );
		}
	}
	else if( flag > flagRigSlot0  &&  flag < flagRigSlot7 )
	{
		if(!Skill::FitModuleSkillCheck(item, character))
			throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
		if(c->GetShip()->GetAttribute(AttrRigSize) != item->GetAttribute(AttrRigSize))
			throw PyException( MakeCustomError( "Your ship cannot fit this size module" ) );
		if( c->GetShip()->GetAttribute(AttrUpgradeLoad) + item->GetAttribute(AttrUpgradeCost) > c->GetShip()->GetAttribute(AttrUpgradeCapacity) )
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
}

void Ship::SaveShip()
{
    sLog.Debug( "Ship::SaveShip()", "Saving all 'entity' info and attribute info to DB for ship %s (%u)...", itemName().c_str(), itemID() );
    SaveItem();
    SaveAttributes();
}

bool Ship::ValidateItemSpecifics(Client *c, InventoryItemRef equip) {

	//declaring explicitly as int...not sure if this is needed or not
	int groupID = c->GetShip()->groupID();
	int typeID = c->GetShip()->typeID();
	EvilNumber canFitShipGroup1 = equip->GetAttribute(AttrCanFitShipGroup1);
	EvilNumber canFitShipGroup2 = equip->GetAttribute(AttrCanFitShipGroup2);
	EvilNumber canFitShipGroup3 = equip->GetAttribute(AttrCanFitShipGroup3);
	EvilNumber canFitShipGroup4 = equip->GetAttribute(AttrCanFitShipGroup4);
	EvilNumber canFitShipType1 = equip->GetAttribute(AttrCanFitShipType1);
	EvilNumber canFitShipType2 = equip->GetAttribute(AttrCanFitShipType2);
	EvilNumber canFitShipType3 = equip->GetAttribute(AttrCanFitShipType3);
	EvilNumber canFitShipType4 = equip->GetAttribute(AttrCanFitShipType4);

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
		item->head.sub_type = IsMassive | IsFree;
		
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
		head.sub_type = IsMassive | IsFree;
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

void ShipEntity::MakeDamageState(DoDestinyDamageState &into) const
{
	into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
	into.tau = 100000;	//no freaking clue.
	into.timestamp = Win32TimeNow();
//	armor damage isn't working...
	into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
	into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHp).get_float());
}

