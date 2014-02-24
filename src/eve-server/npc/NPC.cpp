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
    Author:        Zhur
*/

#include "eve-server.h"

#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/SpawnManager.h"
#include "ship/DestinyManager.h"
#include "system/SystemManager.h"

using namespace Destiny;

NPC::NPC(
    SystemManager *system,
    PyServiceMgr &services,
    InventoryItemRef self,
    uint32 corporationID,
    uint32 allianceID,
    const GPoint &position,
    SpawnEntry *spawner)
: DynamicSystemEntity(new DestinyManager(this, system), self),
  m_system(system),
  m_services(services),
  m_spawner(spawner),
//  m_itemID(self->itemID()),
//  m_typeID(self->typeID()),
//  m_ownerID(self->ownerID()),
  m_corporationID(corporationID),
  m_allianceID(allianceID),
  m_orbitingID(0)
{
    //NOTE: this is bad if we inherit NPC!
    m_AI = new NPCAIMgr(this);

	// SET ALL ATTRIBUTES MISSING FROM DATABASE BEFORE USING THEM FOR ANYTHING:
    // Create default dynamic attributes in the AttributeMap:
    self->SetAttribute(AttrIsOnline,            1, true);											// Is Online
    self->SetAttribute(AttrShieldCharge,        self->GetAttribute(AttrShieldCapacity), true);		// Shield Charge
    self->SetAttribute(AttrArmorDamage,         0.0, true);											// Armor Damage
    self->SetAttribute(AttrMass,                self->type().attributes.mass(), true);				// Mass
    self->SetAttribute(AttrRadius,              self->type().attributes.radius(), true);			// Radius
    self->SetAttribute(AttrVolume,              self->type().attributes.volume(), true);			// Volume
    self->SetAttribute(AttrCapacity,            self->type().attributes.capacity(), true);			// Capacity
    self->SetAttribute(AttrInertia,             1, true);											// Inertia
    self->SetAttribute(AttrCharge,              self->GetAttribute(AttrCapacitorCapacity), true);	// Set Capacitor Charge to the Capacitor Capacity
	self->SetAttribute(AttrShieldCharge,        self->GetAttribute(AttrShieldCapacity), true);		// Shield Charge

	// Agility - WARNING!  NO NPC Ships have Agility, so we're setting it to 1 for ALL NPC ships
	if( !(self->HasAttribute(AttrAgility)) )
        self->SetAttribute(AttrAgility, 1, true);

	// Shield Resonance
	// AttrShieldEmDamageResonance
	if( !(self->HasAttribute(AttrShieldEmDamageResonance)) )
		self->SetAttribute(AttrShieldEmDamageResonance, 1.0, true);
	// AttrShieldExplosiveDamageResonance
	if( !(self->HasAttribute(AttrShieldExplosiveDamageResonance)) )
		self->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, true);
	// AttrShieldKineticDamageResonance
	if( !(self->HasAttribute(AttrShieldKineticDamageResonance)) )
		self->SetAttribute(AttrShieldKineticDamageResonance, 1.0, true);
	// AttrShieldThermalDamageResonance
	if( !(self->HasAttribute(AttrShieldThermalDamageResonance)) )
		self->SetAttribute(AttrShieldThermalDamageResonance, 1.0, true);

	// Armor Resonance
	// AttrArmorEmDamageResonance
	if( !(self->HasAttribute(AttrArmorEmDamageResonance)) )
		self->SetAttribute(AttrArmorEmDamageResonance, 1.0, true);
	// AttrArmorExplosiveDamageResonance
	if( !(self->HasAttribute(AttrArmorExplosiveDamageResonance)) )
		self->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, true);
	// AttrArmorKineticDamageResonance
	if( !(self->HasAttribute(AttrArmorKineticDamageResonance)) )
		self->SetAttribute(AttrArmorKineticDamageResonance, 1.0, true);
	// AttrArmorThermalDamageResonance
	if( !(self->HasAttribute(AttrArmorThermalDamageResonance)) )
		self->SetAttribute(AttrArmorThermalDamageResonance, 1.0, true);

	// Hull Damage
	if( !(self->HasAttribute(AttrDamage)) )
        self->SetAttribute(AttrDamage, 0, true );
	// AttrHullEmDamageResonance
	if( !(self->HasAttribute(AttrHullEmDamageResonance)) )
		self->SetAttribute(AttrHullEmDamageResonance, 1.0, true);
	// AttrHullExplosiveDamageResonance
	if( !(self->HasAttribute(AttrHullExplosiveDamageResonance)) )
		self->SetAttribute(AttrHullExplosiveDamageResonance, 1.0, true);
	// AttrHullKineticDamageResonance
	if( !(self->HasAttribute(AttrHullKineticDamageResonance)) )
		self->SetAttribute(AttrHullKineticDamageResonance, 1.0, true);
	// AttrHullThermalDamageResonance
	if( !(self->HasAttribute(AttrHullThermalDamageResonance)) )
		self->SetAttribute(AttrHullThermalDamageResonance, 1.0, true);

	// AttrOrbitRange
	if( !(self->HasAttribute(AttrOrbitRange)) )
		self->SetAttribute(AttrOrbitRange, 5000, true);


	// Set internal and Destiny values FROM these Attributes, now that they are all setup:
    m_destiny->SetPosition(position, false);
	m_destiny->SetShipCapabilities(self);
	m_destiny->SetSpeedFraction(1.0);
	m_destiny->Halt();

	/* Gets the value from the NPC and put on our own vars */
    //m_shieldCharge = self->shieldCharge();
    m_shieldCharge = self->GetAttribute(AttrShieldCapacity).get_float();
    m_armorDamage = 0.0;
    m_hullDamage = 0.0;
}

NPC::~NPC() {
    //it is so dangerous to do this stuff in a destructor, with the
    //possibility of any of these things making virtual calls...
    //
    // this makes inheriting NPC a bad idea (see constructor)

    //m_system->RemoveNPC(this);
    if(m_spawner != NULL)
        m_spawner->SpawnDepoped(m_self->itemID());
    targets.DoDestruction();
    delete m_AI;
}

void NPC::Process() {
    SystemEntity::Process();
    m_AI->Process();
}

void NPC::Orbit(SystemEntity *who) {
    if(who == NULL) {
        m_orbitingID = 0;
    } else {
        m_orbitingID = who->GetID();
    }
}

void NPC::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

bool NPC::Load(ServiceDB &from) {
    //The old purpose for this was eliminated. But we might find
    //something else to stick in here eventually, so it stays for now.
    return true;
}

void NPC::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void NPC::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void NPC::EncodeDestiny( Buffer& into ) const
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
        ship.velocity_x = GetVelocity().x;
        ship.velocity_y = GetVelocity().y;
        ship.velocity_z = GetVelocity().z;
        ship.agility = static_cast<float>(GetAgility());
        ship.speed_fraction = (float)(Destiny()->GetSpeedFraction());
        into.Append( ship );

        DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    }
}


void NPC::MakeDamageState(DoDestinyDamageState &into) const {
    //into.shield = m_shieldCharge / m_self->shieldCapacity();
    into.shield = m_shieldCharge / m_self->GetAttribute(AttrShieldCapacity).get_float();
    into.tau = 100000;    //no freakin clue.
    into.timestamp = Win32TimeNow();
    //into.armor = 1.0 - (m_armorDamage / m_self->armorHP());
    //into.structure = 1.0 - (m_hullDamage / m_self->hp());

    // the get_float is still a hack... majorly
    into.armor = 1.0 - (m_armorDamage / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_hullDamage / m_self->GetAttribute(AttrHp).get_float());
}

void NPC::UseShieldRecharge()
{
    // We recharge our shield until it's reaches the shield capacity.
    if( this->Item()->GetAttribute(AttrShieldCapacity) > m_shieldCharge )
    {
        // Not found the information on how much it's consume from capacitor
        //m_shieldCharge += this->Item()->entityShieldBoostAmount();
        m_shieldCharge += this->Item()->GetAttribute(AttrEntityShieldBoostAmount).get_float();
    }
    // TODO: Need to send SpecialFX / amount update
}

void NPC::UseArmorRepairer()
{
    // We recharge our shield until it's reaches the shield capacity.
    if( m_armorDamage > 0 )
    {
        // Not found the information on how much it's consume from capacitor
        //m_armorDamage -= this->Item()->entityArmorRepairAmount();
        m_armorDamage -= this->Item()->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if( m_armorDamage < 0.0 )
            m_armorDamage = 0.0;
    }
    // TODO: Need to send SpecialFX / amount update
}

void NPC::SaveNPC()
{
	// Save all data for this NPC ship to the database:
	Item()->SaveItem();
}
