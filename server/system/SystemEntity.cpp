/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "SystemEntity.h"
#include "../common/DestinyStructs.h"
#include "../common/DestinyBinDump.h"
#include "../common/PyRep.h"
#include "../Client.h"
#include "../NPC.h"
#include "../inventory/InventoryItem.h"
#include "../packets/Destiny.h"
#include "../ship/DestinyManager.h"
#include "../system/Damage.h"
#include "../common/EVEUtils.h"


using namespace Destiny;




SystemEntity::SystemEntity()
: targets(this),
  m_bubble(NULL)
{
}

void SystemEntity::Process() {
	targets.Process();
}

double SystemEntity::DistanceTo2(const SystemEntity *other) const {
	GVector delta(GetPosition(), other->GetPosition());
	return(delta.lengthSquared());
}

void SystemEntity::MakeAddBall(DoDestiny_AddBall &addball, uint32 updateID) const {
	std::vector<byte> ball_buffer(sizeof(Destiny::AddBall_header));
	ball_buffer.reserve(512);
	
	Destiny::AddBall_header *head = (Destiny::AddBall_header *) &ball_buffer[0];
	head->more = 1;
	head->sequence = updateID;
	
	EncodeDestiny(ball_buffer);
	
	addball.destiny_binary.assign((const char *) &ball_buffer[0], ball_buffer.size());

	addball.damages[ GetID() ] = MakeDamageState();
	
	PyRepDict *slim_dict = MakeSlimItem();
	addball.slim = new PyRepObject("foo.SlimItem", slim_dict);

	_log(DESTINY__TRACE, "Add Ball:");
	addball.Dump(DESTINY__TRACE, "    ");
	_log(DESTINY__TRACE, "    Ball Binary: (%lu bytes)", addball.destiny_binary.length());
	_hex(DESTINY__TRACE, addball.destiny_binary.c_str(), addball.destiny_binary.length());
	_log(DESTINY__TRACE, "    Ball Decoded:");
	Destiny::DumpUpdate(DESTINY__TRACE, (const byte *) addball.destiny_binary.c_str(), addball.destiny_binary.length());
	
}

PyRepTuple *SystemEntity::MakeDamageState() const {
	DoDestinyDamageState ddds;
	MakeDamageState(ddds);
	return(ddds.FastEncode());
}

ItemSystemEntity::ItemSystemEntity(InventoryItem *self)
: SystemEntity(),
  m_self(NULL)
{
	if(self != NULL) {
		_SetSelf(self);
	}
	//setup some default attributes which normally do not initilize.
}

ItemSystemEntity::~ItemSystemEntity() {
	if(m_self != NULL)
		m_self->Release();
}

void ItemSystemEntity::_SetSelf(InventoryItem *self) {
	if(self == NULL) {
		codelog(ITEM__ERROR, "Tried to set self to NULL!");
		return;
	}
	if(m_self != NULL) {
		m_self->Release();
	}
	m_self = self;
	
	int sc = m_self->shieldCapacity();
	if(sc > 0) {	//avoid polluting the attribute list with worthless entries.
		m_self->Set_shieldCharge(m_self->shieldCapacity());
	}
}

const char *ItemSystemEntity::GetName() const {
	if(m_self == NULL)
		return("NoName");
	return(m_self->itemName().c_str());
}

double ItemSystemEntity::GetRadius() const {
	if(m_self == NULL)
		return(1.0f);
	return(m_self->radius());
}

const GPoint &ItemSystemEntity::GetPosition() const {
	static const GPoint err_return(1, 2, 3);
	if(m_self == NULL)
		return(err_return);
	return(m_self->position());
}

PyRepDict *ItemSystemEntity::MakeSlimItem() const {
	PyRepDict *slim = new PyRepDict();
	slim->add("itemID", new PyRepInteger(m_self->itemID()));
	slim->add("typeID", new PyRepInteger(m_self->typeID()));
	slim->add("ownerID", new PyRepInteger(m_self->ownerID()));
	return(slim);
}

uint32 ItemSystemEntity::GetID() const {
	return(m_self->itemID());
}

DynamicSystemEntity::DynamicSystemEntity(DestinyManager *dm, InventoryItem *self)
: ItemSystemEntity(self),
  m_destiny(dm)
{
}

DynamicSystemEntity::~DynamicSystemEntity() {
	if(m_destiny != NULL) {
		//Do not do anything with the destiny manager, as it's m_self
		//is now partially destroyed, which will majorly upset things.
		delete m_destiny;
	}
}

void DynamicSystemEntity::ProcessDestiny(uint32 stamp) {
	if(m_destiny != NULL)
		m_destiny->Process(stamp);
}

const GPoint &DynamicSystemEntity::GetPosition() const {
	static const GPoint _zeroPoint(0.0, 0.0, 0.0);
	if(m_destiny == NULL)
		return(_zeroPoint);
	return(m_destiny->GetPosition());
}

//TODO: ask the destiny manager to do this for us!
void DynamicSystemEntity::EncodeDestiny(std::vector<byte> &into) const {
	int start = into.size();
	int slen = strlen(GetName());
	
	const GPoint &position = GetPosition();
	
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
		byte *ptr = &into[start];
		AddBall_Warp *item = (AddBall_Warp *) ptr;
		ptr += sizeof(AddBall_Warp);
		
		item->head.entityID = GetID();
		item->head.mode = Destiny::DSTBALL_WARP;
		item->head.radius = Ship()->radius();
		item->head.x = position.x;
		item->head.y = position.y;
		item->head.z = position.z;
		item->head.sub_type = AddBallSubType_player;
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
		strcpy_fake_unicode(item->name.name, GetName());
	} else*/ {
		#pragma pack(1)
		struct AddBall_Stop {
			BallHeader head;
			MassSector mass;
			ShipSector ship;
			DSTBALL_STOP_Struct main;
			NameStruct name;
		};
		#pragma pack()
		
		into.resize(start 
			+ sizeof(AddBall_Stop) 
			+ slen*sizeof(uint16) );
		byte *ptr = &into[start];
		AddBall_Stop *item = (AddBall_Stop *) ptr;
		ptr += sizeof(AddBall_Stop);
		
		item->head.entityID = GetID();
		item->head.mode = Destiny::DSTBALL_STOP;
		item->head.radius = GetRadius();
		item->head.x = position.x;
		item->head.y = position.y;
		item->head.z = position.z;
		item->head.sub_type = AddBallSubType_player;
		item->mass.mass = GetMass();
		item->mass.unknown51 = 0;
		item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		item->mass.corpID = GetCorporationID();
		item->mass.allianceID = 0xFFFFFFFF;
		
		item->ship.max_speed = GetMaxVelocity();
		item->ship.velocity_x = 0.0;	//TODO: use destiny's velocity
		item->ship.velocity_y = 0.0;
		item->ship.velocity_z = 0.0;
		item->ship.agility = GetAgility();
		item->ship.speed_fraction = 1.0;	//TODO: put in speed fraction!
		
		item->main.formationID = 0xFF;
		
		item->name.name_len = slen;	// in number of unicode chars
		strcpy_fake_unicode(item->name.name, GetName());
	}
}

double Client::GetRadius() const {
	return(Ship()->radius());
}

PyRepDict *Client::MakeSlimItem() const {
	PyRepDict *slim = new PyRepDict();
	slim->add("bounty", new PyRepInteger(0));
	slim->add("ownerID", new PyRepInteger(GetCharacterID()));
	slim->add("charID", new PyRepInteger(GetCharacterID()));
	slim->add("securityStatus", new PyRepReal(0));
	slim->add("itemID", new PyRepInteger(GetShipID()));
	slim->add("typeID", new PyRepInteger(Ship()->typeID()));
	slim->add("corpID", new PyRepInteger(GetCorporationID()));
	
	//encode the modules list, if we have any visible modules
	std::vector<InventoryItem *> items;
	Ship()->FindByFlagRange(flagHiSlot0, flagHiSlot7, items, false);
	if(!items.empty()) {
		PyRepList *l = new PyRepList();
		std::vector<InventoryItem *>::iterator cur, end;
		cur = items.begin();
		end = items.end();
		for(; cur != end; cur++) {
			InventoryItem *i = *cur;
			PyRepTuple *t = new PyRepTuple(2);
			t->items[0] = new PyRepInteger(i->itemID());
			t->items[1] = new PyRepInteger(i->typeID());
			l->add(t);
		}
		slim->add("modules", l);
	}
	
	return(slim);
}

void ItemSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
	into.shield = m_self->shieldCharge() / m_self->shieldCapacity();
	into.tau = 100000;	//no freakin clue.
	into.timestamp = Win32TimeNow();
//	armor damage isnt working...
	into.armor = 1.0 - (m_self->armorDamage() / m_self->armorHP());
	into.structure = 1.0 - (m_self->damage() / m_self->hp());
}

//clients use their ship, not their char.
void Client::MakeDamageState(DoDestinyDamageState &into) const {
	into.shield = m_ship->shieldCharge() / m_ship->shieldCapacity();
	into.tau = 100000;	//no freakin clue.
	into.timestamp = Win32TimeNow();
	into.armor = 1.0 - (m_ship->armorDamage() / m_ship->armorHP());
	into.structure = 1.0 - (m_ship->damage() / m_ship->hp());
}

double NPC::GetRadius() const {
	return(m_self->radius());
}

void NPC::EncodeDestiny(std::vector<byte> &into) const {
	int start = into.size();
	int slen = 0;
	
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
		byte *ptr = &into[start];
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
		#pragma pack(1)
		struct AddBall_Stop {
			BallHeader head;
			MassSector mass;
			ShipSector ship;
			DSTBALL_STOP_Struct main;
			NameStruct name;
		};
		#pragma pack()
		
		into.resize(start 
			+ sizeof(AddBall_Stop) 
			+ slen*sizeof(uint16) );
		byte *ptr = &into[start];
		AddBall_Stop *item = (AddBall_Stop *) ptr;
		ptr += sizeof(AddBall_Stop);
		
		item->head.entityID = GetID();
		item->head.mode = Destiny::DSTBALL_STOP;
		item->head.radius = m_self->radius();
		item->head.x = x();
		item->head.y = y();
		item->head.z = z();
		item->head.sub_type = AddBallSubType_orbitingNPC;
		
		item->ship.max_speed = m_self->maxVelocity();
		item->ship.velocity_x = 0.0;
		item->ship.velocity_y = 0.0;
		item->ship.velocity_z = 0.0;
		item->ship.agility = m_self->agility();
		item->ship.speed_fraction = 1.0;
		
		item->mass.mass = m_self->mass();
		item->mass.unknown51 = 0;
		item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		item->mass.corpID = GetCorporationID();
		item->mass.allianceID = 0xFFFFFFFF;
		
		item->main.formationID = 0xFF;
		
		item->name.name_len = slen;	// in number of unicode chars
		//strcpy_fake_unicode(item->name.name, GetName());
	}
}

/*PyRepDict *NPC::MakeSlimItem() const {
	PyRepDict *slim = DynamicSystemEntity::MakeSlimItem();
	if(slim == NULL)
		return(NULL);
	//Not doing any additions yet, but might need eventually.
	return(slim);
}*/



























