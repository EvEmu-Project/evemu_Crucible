
#include "EVEServerPCH.h"

using namespace Destiny;

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

using namespace Destiny;

AsteroidEntity::AsteroidEntity(
    InventoryItemRef asteroid,
	SystemManager *system,
	PyServiceMgr &services,
	const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), asteroid),
  m_system(system),
  m_services(services)
{
    _asteroidRef = asteroid;
	m_destiny->SetPosition(position, false);
}

void AsteroidEntity::Process() {
	SystemEntity::Process();
}

void AsteroidEntity::ForcedSetPosition(const GPoint &pt) {
	m_destiny->SetPosition(pt, false);
}

void AsteroidEntity::EncodeDestiny( Buffer& into ) const
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

void AsteroidEntity::MakeDamageState(DoDestinyDamageState &into) const {
//	into.shield = m_shieldCharge / m_self->shieldCapacity();
//	into.tau = 100000;	//no freakin clue.
//	into.timestamp = Win32TimeNow();
//	into.armor = 1.0 - (m_armorDamage / m_self->armorHP());
//	into.structure = 1.0 - (m_hullDamage / m_self->hp());
}


// OLD Asteroid class implementation:
/*
Asteroid::Asteroid(
	SystemManager *system,
	InventoryItemRef self
)
: ItemSystemEntity(self),
  m_system(system)
{
}

Asteroid::~Asteroid() {
}
	
void Asteroid::Grow() {
}

bool Asteroid::ApplyDamage(Damage &d)
{
	//ignore it.
	_log( MINING__DEBUG, "Asteroid %u: Ignoring %f damage from %s", m_self->itemID(), d.GetTotal(), d.source->GetName() );

	return false;
}

void Asteroid::EncodeDestiny( Buffer& into ) const
{
	const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

    BallHeader head;
	head.entityID = GetID();
	head.mode = Destiny::DSTBALL_RIGID;
	head.radius = GetRadius();
	head.x = position.x;
	head.y = position.y;
	head.z = position.z;
	head.sub_type = AddBallSubType_cargoContainer_asteroid;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
	main.formationID = 0xFF;
    into.Append( main );

    const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
    into.Append( nameLen );

    const Buffer::iterator<uint16> name = into.end<uint16>();
    into.ResizeAt( name, nameLen );
    utf8::utf8to16( itemName.begin(), itemName.end(), name );
}

/*PyDict *Asteroid::MakeSlimItem() const {
	PyDict *slim = new PyDict();
	slim->add("typeID", new PyInt(m_typeID));
	slim->add("ownerID", new PyInt(500021));	//no idea.
	slim->add("itemID", new PyInt(m_itemID));
	return(slim);
}*/
