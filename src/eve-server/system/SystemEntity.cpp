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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

using namespace Destiny;

SystemEntity::SystemEntity()
: targets(this),
  m_bubble(NULL)
{
}

void SystemEntity::Process() {
	targets.Process();
}

uint32 SystemEntity::GetLocationID(SystemEntity *se)
{
	uint32 itemID = se->GetID();
	
	return (SystemDB::GetObjectLocationID( itemID ) );
}

double SystemEntity::DistanceTo2(const SystemEntity *other) const {
	GVector delta(GetPosition(), other->GetPosition());
	return(delta.lengthSquared());
}

PyTuple *SystemEntity::MakeDamageState() const {
	DoDestinyDamageState ddds;
	MakeDamageState(ddds);
	return(ddds.Encode());
}

ItemSystemEntity::ItemSystemEntity(InventoryItemRef self)
: SystemEntity(),
  m_self()
{
	if( self )
		_SetSelf( self );
	//setup some default attributes which normally do not initialize.
}

ItemSystemEntity::~ItemSystemEntity()
{
}

void ItemSystemEntity::_SetSelf(InventoryItemRef self) {
	if( !self ) {
		codelog(ITEM__ERROR, "Tried to set self to NULL!");
		return;
	}

	m_self = self;

    // DEPRECATED NOW WITH THE USE OF NEW ATTRIBUTE SYSTEM AND SAVING OF THOSE ATTRIBUTES TO THE DB -- Aknor Jaden
	//I am not sure where the right place to do this is, but until
	//we properly persist ship attributes into the DB, we are just
	//going to do it here. Could be exploited. oh well.
	// TODO: use the ship aggregate value.
	/*int sc = m_self->shieldCapacity();
	if( sc > 0 )	//avoid polluting the attribute list with worthless entries.
		m_self->Set_shieldCharge( sc );*/
    //EvilNumber sc = m_self->GetAttribute(AttrShieldCapacity);
    //if (sc > 0)
    //    m_self->SetAttribute(AttrShieldCharge, sc);
}

const char *ItemSystemEntity::GetName() const {
	if(!m_self)
		return("NoName");
	return(m_self->itemName().c_str());
}

float ItemSystemEntity::GetRadius() const {
	if(!m_self)
		return(1.0f);
	//return(m_self->radius());
    return static_cast<float>(m_self->GetAttribute(AttrRadius).get_float());
}

const GPoint &ItemSystemEntity::GetPosition() const {
	static const GPoint err(0.0, 0.0, 0.0);
	if(!m_self)
		return(err);
	return(m_self->position());
}

const GVector &ItemSystemEntity::GetVelocity() const {
	static const GVector err(0.0, 0.0, 0.0);
	return(err);
}

PyDict *ItemSystemEntity::MakeSlimItem() const {
	PyDict *slim = new PyDict();
	slim->SetItemString("itemID", new PyInt(Item()->itemID()));
	slim->SetItemString("typeID", new PyInt(Item()->typeID()));
	slim->SetItemString("ownerID", new PyInt(Item()->ownerID()));
	return(slim);
}

uint32 ItemSystemEntity::GetID() const {
	if(!Item())
		return(0);
	return(Item()->itemID());
}

DynamicSystemEntity::DynamicSystemEntity(DestinyManager *dm, InventoryItemRef self)
: ItemSystemEntity(self),
  m_destiny(dm)
{
}

DynamicSystemEntity::~DynamicSystemEntity() {
	if(m_destiny != NULL) {
		//Do not do anything with the destiny manager, as it's m_self
		//is now partially destroyed, which will majority upset things.
		delete m_destiny;
	}
}

void DynamicSystemEntity::ProcessDestiny() {
	if(m_destiny != NULL)
		m_destiny->Process();
}

const GPoint &DynamicSystemEntity::GetPosition() const {
	if(m_destiny == NULL)
		return(ItemSystemEntity::GetPosition());
	return(m_destiny->GetPosition());
}

const GVector &DynamicSystemEntity::GetVelocity() const {
   	static const GVector err(0.0, 0.0, 0.0);
	if(m_destiny == NULL)
		return(err);
    return(m_destiny->GetVelocity());
}

double DynamicSystemEntity::GetMass() const {
	if(!Item())
		return(0.0f);
	return Item()->GetAttribute(AttrMass).get_float();
}

double DynamicSystemEntity::GetMaxVelocity() const {
	if(!Item())
		return(0.0f);
    return Item()->GetAttribute(AttrMaxVelocity).get_float();
}

double DynamicSystemEntity::GetAgility() const {
	if(!Item())
		return(0.0f);
	return Item()->GetAttribute(AttrAgility).get_float();
}

//TODO: ask the destiny manager to do this for us!
void DynamicSystemEntity::EncodeDestiny( Buffer& into ) const
{
	const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

/*	if(m_warpActive) {
		#pragma pack(1)
		struct AddBall_Warp {
			BallHeader head;
			MassSector mass;
			ShipSector ship;
			DSTBALL_WARP_Struct main;
			NameStruct name;
		};
		#pragma pack()
		
		into.resize(start 
			+ sizeof(AddBall_Warp) 
			+ slen*sizeof(uint16) );
		uint8 *ptr = &into[start];
		AddBall_Warp *item = (AddBall_Warp *) ptr;
		ptr += sizeof(AddBall_Warp);
		
		item->head.entityID = GetID();
		item->head.mode = Destiny::DSTBALL_WARP;
		item->head.radius = Ship()->radius();
		item->head.x = position.x;
		item->head.y = position.y;
		item->head.z = position.z;
		item->head.sub_type = IsFree | IsMassive | IsInteractive;
		item->mass.mass = Ship()->mass();
		item->mass.unknown51 = 0;
		item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		item->mass.corpID = GetCorporationID();
		item->mass.unknown64 = 0xFFFFFFFF;

		GVector vec(GetPosition(), m_movePoint);
		vec.normalize();
		vec *= 45.0;	//no idea what to use...
		
		item->ship.max_speed = Ship()->maxVelocity();
		item->ship.velocity_x = vec.x;
		item->ship.velocity_y = vec.y;
		item->ship.velocity_z = vec.z;
		item->ship.agility = Ship()->agility();
		item->ship.speed_fraction = 1.0;	//TODO: put in speed fraction!
		
		item->main.unknown116 = 0xFF;
		item->main.unknown_x = m_movePoint.x;
		item->main.unknown_y = m_movePoint.y;
		item->main.unknown_z = m_movePoint.z;
		item->main.effectStamp = 0xFFFFFFFF;
		item->main.followRange = 1000.0;
		item->main.followID = 0;
		item->main.ownerID = 0x1e;	//no idea.
		
		item->name.name_len = slen;	// in number of unicode chars
		py_mbstowcs(item->name.name, GetName());
	} else*/ {
        BallHeader head;
		head.entityID = GetID();
		head.mode = Destiny::DSTBALL_STOP;
		head.radius = GetRadius();
		head.x = position.x;
		head.y = position.y;
		head.z = position.z;
		head.sub_type = IsFree | IsMassive | IsInteractive;
        into.Append( head );

        MassSector mass;
		mass.mass = GetMass();
		mass.cloak = 0;
        mass.Harmonic = -1.0f;
		mass.corpID = GetCorporationID();
		mass.allianceID = GetAllianceID();
        into.Append( mass );

        ShipSector ship;
        if( Destiny() == NULL )     // We dont have a destiny object or dont have one YET
        {
		    ship.max_speed = static_cast<float>(GetMaxVelocity());
            ship.velocity_x = 0.0;
		    ship.velocity_y = 0.0;
		    ship.velocity_z = 0.0;
		    ship.agility = static_cast<float>(GetAgility());
            ship.speed_fraction = 0.0;
        }
        else
        {
		    ship.max_speed = static_cast<float>(GetMaxVelocity());
            ship.velocity_x = Destiny()->GetVelocity().x;
		    ship.velocity_y = Destiny()->GetVelocity().y;
		    ship.velocity_z = Destiny()->GetVelocity().z;
		    ship.agility = static_cast<float>(GetAgility());
            ship.speed_fraction = static_cast<float>(Destiny()->GetSpeedFraction());
        }
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



void ItemSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
	into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
	into.tau = 100000;	//no freaking clue.
	into.timestamp = Win32TimeNow();
//	armor damage isn't working...
	into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
	into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHp).get_float());
}


CelestialDynamicSystemEntity::CelestialDynamicSystemEntity(DestinyManager *dm, InventoryItemRef self)
: DynamicSystemEntity(dm, self)
{
}

CelestialDynamicSystemEntity::~CelestialDynamicSystemEntity() {
	if(m_destiny != NULL) {
		//Do not do anything with the destiny manager, as it's m_self
		//is now partially destroyed, which will majority upset things.
		delete m_destiny;
	}
}

//TODO: ask the destiny manager to do this for us!
void CelestialDynamicSystemEntity::EncodeDestiny( Buffer& into ) const
{
	const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

    // From SimpleSystemEntity::EncodeDestiny() in SystemEntities.cpp:
/*
	BallHeader head;
	head.entityID = data.itemID;
	head.mode = Destiny::DSTBALL_RIGID;
	head.radius = data.radius;
	head.x = data.position.x;
	head.y = data.position.y;
	head.z = data.position.z;
	head.sub_type = IsMassive | IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
	main.formationID = 0xFF;
    into.Append( main );

*/

    BallHeader head;
	head.entityID = GetID();
	head.mode = Destiny::DSTBALL_RIGID;
	head.radius = GetRadius();
	head.x = position.x;
	head.y = position.y;
	head.z = position.z;
	head.sub_type = IsMassive | IsGlobal;
    into.Append( head );
/*
    MassSector mass;
	mass.mass = GetMass();
	mass.cloak = 0;
	mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
	mass.corpID = GetCorporationID();
	mass.allianceID = GetAllianceID();
    into.Append( mass );
*/
    DSTBALL_RIGID_Struct main;
	main.formationID = 0xFF;
    into.Append( main );

    const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
    into.Append( nameLen );

    const Buffer::iterator<uint16> name = into.end<uint16>();
    into.ResizeAt( name, nameLen );
    utf8::utf8to16( itemName.begin(), itemName.end(), name );
}
