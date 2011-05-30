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
    Author:     Aknor Jaden
*/

#include "EVEServerPCH.h"


/*
 * Structure
 */
Structure::Structure(
    ItemFactory &_factory,
    uint32 _structureID,
    // InventoryItem stuff:
    const ItemType &_itemType,
    const ItemData &_data)
: InventoryItem(_factory, _structureID, _itemType, _data) {}

StructureRef Structure::Load(ItemFactory &factory, uint32 structureID)
{
    return InventoryItem::Load<Structure>( factory, structureID );
}

template<class _Ty>
RefPtr<_Ty> Structure::_LoadStructure(ItemFactory &factory, uint32 structureID,
    // InventoryItem stuff:
    const ItemType &itemType, const ItemData &data)
{
    // we don't need any additional stuff
    return StructureRef( new Structure( factory, structureID, itemType, data ) );
}

StructureRef Structure::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    uint32 structureID = Structure::_Spawn( factory, data );
    if( structureID == 0 )
        return StructureRef();
    return Structure::Load( factory, structureID );
}

uint32 Structure::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // make sure it's a Structure
    const ItemType *st = factory.GetType(data.typeID);
    if(st == NULL)
        return 0;

    // store item data
    uint32 structureID = InventoryItem::_Spawn(factory, data);
    if(structureID == 0)
        return 0;

    // nothing additional

    return structureID;
}

bool Structure::_Load()
{
    // load contents
    if( !LoadContents( m_factory ) )
        return false;

    return InventoryItem::_Load();
}

void Structure::Delete()
{
    // delete contents first
    DeleteContents( m_factory );

    InventoryItem::Delete();
}

double Structure::GetCapacity(EVEItemFlags flag) const
{
    switch( flag ) {
        // the .get_float() part is a evil hack.... as this function should return a EvilNumber.
        case flagAutoFit:
        case flagCargoHold:           return GetAttribute(AttrCapacity).get_float();
        case flagSecondaryStorage:    return GetAttribute(AttrCapacitySecondary).get_float();
        case flagSpecializedAmmoHold: return GetAttribute(AttrAmmoCapacity).get_float();
        default:                      return 0.0;
    }
}

void Structure::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item, Client *c)
{
	CharacterRef character = c->GetChar();
	
    if( flag == flagCargoHold )
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
    else if( flag == flagSecondaryStorage )
    {
        //get all items in SecondaryStorage
		EvilNumber capacityUsed(0);
		std::vector<InventoryItemRef> items;
		c->GetShip()->FindByFlag(flag, items);
		for(uint32 i = 0; i < items.size(); i++){
			capacityUsed += items[i]->GetAttribute(AttrVolume);
		}
		if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrCapacitySecondary) )
			throw PyException( MakeCustomError( "Not enough Secondary Storage space!") );
    }
    else if( flag == flagSpecializedAmmoHold )
    {
        //get all items in ammo hold
		EvilNumber capacityUsed(0);
		std::vector<InventoryItemRef> items;
		c->GetShip()->FindByFlag(flag, items);
		for(uint32 i = 0; i < items.size(); i++){
			capacityUsed += items[i]->GetAttribute(AttrVolume);
		}
		if( capacityUsed + item->GetAttribute(AttrVolume) > c->GetShip()->GetAttribute(AttrAmmoCapacity) )
			throw PyException( MakeCustomError( "Not enough Ammo Storage space!") );
    }
}

PyObject *Structure::StructureGetInfo()
{
    if( !LoadContents( m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for StructureGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the Structure.
    if( !Populate( entry ) )
        return NULL;    //print already done.

    result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void Structure::AddItem(InventoryItemRef item)
{
    InventoryEx::AddItem( item );
}


using namespace Destiny;

StructureEntity::StructureEntity(
    StructureRef structure,
	SystemManager *system,
	PyServiceMgr &services,
	const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), structure),
  m_system(system),
  m_services(services)
{
    _structureRef = structure;
	m_destiny->SetPosition(position, false);
}

void StructureEntity::Process() {
	SystemEntity::Process();
}

void StructureEntity::ForcedSetPosition(const GPoint &pt) {
	m_destiny->SetPosition(pt, false);
}

void StructureEntity::EncodeDestiny( Buffer& into ) const
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
	} else {
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
*/
    BallHeader head;
	head.entityID = GetID();
	head.mode = Destiny::DSTBALL_RIGID;
	head.radius = GetRadius();
	head.x = position.x;
	head.y = position.y;
	head.z = position.z;
	head.sub_type = /*HasMiniBalls | */IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
	main.formationID = 0xFF;
    into.Append( main );
    
/*
    const uint16 miniballsCount = 1;
    into.Append( miniballsCount );

    MiniBall miniball;
    miniball.x = -7701.181;
    miniball.y = 8060.06;
    miniball.z = 27878.900;
    miniball.radius = 1639.241;
    into.Append( miniball );
*/
    const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
    into.Append( nameLen );

    const Buffer::iterator<uint16> name = into.end<uint16>();
    into.ResizeAt( name, nameLen );
    utf8::utf8to16( itemName.begin(), itemName.end(), name );
}

void StructureEntity::MakeDamageState(DoDestinyDamageState &into) const
{
	into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
	into.tau = 100000;	//no freaking clue.
	into.timestamp = Win32TimeNow();
//	armor damage isn't working...
	into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
	into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHp).get_float());
}

