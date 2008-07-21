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

#include "SystemEntities.h"
#include "../common/DestinyStructs.h"
#include "../common/PyRep.h"
#include "../packets/Destiny.h"
#include "../common/EVEUtils.h"
#include "../common/tables/invGroups.h"
#include "../mining/AsteroidBeltManager.h"


using namespace Destiny;


SimpleSystemEntity *SimpleSystemEntity::MakeEntity(SystemManager *system, const DBSystemEntity &entity) {
	switch(entity.groupID) {
	case EVEDB::invGroups::Sun:
	case EVEDB::invGroups::Planet:
	case EVEDB::invGroups::Moon:
		return(new SystemPlanetEntity(system, entity));
	case EVEDB::invGroups::Asteroid_Belt:
		return(new SystemAsteroidBeltEntity(system, entity));
	case EVEDB::invGroups::Stargate:	//Stargate
		return(new SystemStargateEntity(system, entity));
	//case EVEDB::invGroup::Cargo_Container:	//Cargo Container
	case EVEDB::invGroups::Station:		//Station
		return(new SystemStationEntity(system, entity));
	default:
		_log(SERVICE__ERROR, "Unrecognized entity type '%lu' on '%s' (%lu), falling back to simple space item.", entity.typeID, entity.itemName.c_str(), entity.itemID);
		break;
	}
	return(new SystemSimpleEntity(system, entity));
}

SimpleSystemEntity::SimpleSystemEntity(SystemManager *system, const DBSystemEntity &entity)
: InanimateSystemEntity(system),
  data(entity)
{
}

bool SimpleSystemEntity::LoadExtras(SystemDB *db) {
	return(true);
}

PyRepDict *SimpleSystemEntity::MakeSlimItem() const {
	PyRepDict *slim = new PyRepDict();
	slim->add("typeID", new PyRepInteger(data.typeID));
	slim->add("ownerID", new PyRepInteger(1));
	slim->add("itemID", new PyRepInteger(data.itemID));
	return(slim);
}

void SimpleSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
	into.shield = 0.75;		//just for testing so we know we are doing something.
	into.tau = 100000;	//no idea
	into.armor = 0.95;		//just for testing so we know we are doing something.
	into.structure = 1.00;
	into.timestamp = Win32TimeNow();
}

void InanimateSystemEntity::QueueDestinyUpdate(PyRepTuple **du) {
	//do nothing, the caller will handle it properly.
	//this gives them the opportunity to reduce a copy if they are smart.
}

void InanimateSystemEntity::QueueDestinyEvent(PyRepTuple **multiEvent) {
	//do nothing, the caller will handle it properly.
	//this gives them the opportunity to reduce a copy if they are smart.
}




SystemPlanetEntity::SystemPlanetEntity(SystemManager *system, const DBSystemEntity &entity)
: SimpleSystemEntity(system, entity) {
}

void SystemPlanetEntity::EncodeDestiny(std::vector<byte> &into) const {
	#pragma pack(1)
	struct AddBall_Planet {
		BallHeader head;
		DSTBALL_RIGID_Struct main;
		NameStruct name;
	};
	#pragma pack()
	int start = into.size();
	into.resize(start 
		+ sizeof(AddBall_Planet)
		+ data.itemName.length()*sizeof(uint16) );
	byte *ptr = &into[start];
	AddBall_Planet *item = (AddBall_Planet *) ptr;
	ptr += sizeof(AddBall_Planet);

	item->head.entityID = data.itemID;
	item->head.mode = Destiny::DSTBALL_RIGID;
	item->head.radius = data.radius;
	item->head.x = data.position.x;
	item->head.y = data.position.y;
	item->head.z = data.position.z;
	item->head.sub_type = AddBallSubType_planet;
	item->main.formationID = 0xFF;
	
	item->name.name_len = data.itemName.length();
	strcpy_fake_unicode(item->name.name, data.itemName.c_str());
}







SystemStationEntity::SystemStationEntity(SystemManager *system, const DBSystemEntity &entity)
: SimpleSystemEntity(system, entity) {
}

void SystemStationEntity::EncodeDestiny(std::vector<byte> &into) const {
	#pragma pack(1)
	struct AddBall_Station {
		BallHeader head;
		DSTBALL_RIGID_Struct main;
		MiniBallList miniballs;
		NameStruct name;
	};
	#pragma pack()
	
	int miniball_count = 1;
	
	int start = into.size();
	into.resize(start 
		+ sizeof(AddBall_Station) 
		+ (miniball_count-1)*sizeof(MiniBall)
		+ data.itemName.length()*sizeof(uint16) );
	byte *ptr = &into[start];
	AddBall_Station *item = (AddBall_Station *) ptr;

	item->head.entityID = data.itemID;
	item->head.mode = Destiny::DSTBALL_RIGID;
	item->head.radius = data.radius;
	item->head.x = data.position.x;
	item->head.y = data.position.y;
	item->head.z = data.position.z;
	item->head.sub_type = AddBallSubType_station;
	item->main.formationID = 0xFF;
	
	item->miniballs.count = miniball_count;
	//TOTAL CRAP: we do not have this in our DB right now... 
	item->miniballs.balls[0].x = -7701.181;
	item->miniballs.balls[0].y = 8060.06;
	item->miniballs.balls[0].z = 27878.900;
	item->miniballs.balls[0].radius = 1639.241;
	
	//slide pointer over points...
	item = (AddBall_Station *) (((byte *) item)+(miniball_count-1)*sizeof(MiniBall));
	
	item->name.name_len = data.itemName.length();
	strcpy_fake_unicode(item->name.name, data.itemName.c_str());
}

PyRepDict *SystemStationEntity::MakeSlimItem() const {
	PyRepDict *slim = new PyRepDict();
	slim->add("typeID", new PyRepInteger(data.typeID));
	//HACKED:::
	slim->add("ownerID", new PyRepInteger(1000044));
	slim->add("itemID", new PyRepInteger(data.itemID));
	return(slim);
}




SystemStargateEntity::SystemStargateEntity(SystemManager *system, const DBSystemEntity &entity)
: SystemStationEntity(system, entity),
  m_jumps(NULL)
{
}
SystemStargateEntity::~SystemStargateEntity() {
	targets.DoDestruction();
	delete m_jumps;
}
	
bool SystemStargateEntity::LoadExtras(SystemDB *db) {
	if(!SystemStationEntity::LoadExtras(db))
		return(false);

	m_jumps = db->ListJumps(GetID());
	if(m_jumps == NULL)
		return(false);
	return(true);
}

PyRepDict *SystemStargateEntity::MakeSlimItem() const {
	PyRepDict *slim = SystemStationEntity::MakeSlimItem();
	if(m_jumps != NULL)
		slim->add("jumps", m_jumps->Clone());
	return(slim);
}






SystemAsteroidBeltEntity::SystemAsteroidBeltEntity(SystemManager *system, const DBSystemEntity &entity)
: SimpleSystemEntity(system, entity),
  m_manager(NULL)
{
}

SystemAsteroidBeltEntity::~SystemAsteroidBeltEntity() {
	targets.DoDestruction();
	delete m_manager;
}

void SystemAsteroidBeltEntity::EncodeDestiny(std::vector<byte> &into) const {
	#pragma pack(1)
	struct AddBall_AsteroidBelt {
		BallHeader head;
		DSTBALL_RIGID_Struct main;
		NameStruct name;
	};
	#pragma pack()
	
	
	int start = into.size();
	into.resize(start 
		+ sizeof(AddBall_AsteroidBelt)
		+ data.itemName.length()*sizeof(uint16) );
	byte *ptr = &into[start];
	AddBall_AsteroidBelt *item = (AddBall_AsteroidBelt *) ptr;
	ptr += sizeof(AddBall_AsteroidBelt);

	item->head.entityID = data.itemID;
	item->head.mode = Destiny::DSTBALL_RIGID;
	item->head.radius = data.radius;
	item->head.x = data.position.x;
	item->head.y = data.position.y;
	item->head.z = data.position.z;
	item->head.sub_type = AddBallSubType_asteroidBelt;
	item->main.formationID = 0xFF;
	
	item->name.name_len = data.itemName.length();
	strcpy_fake_unicode(item->name.name, data.itemName.c_str());
}

bool SystemAsteroidBeltEntity::LoadExtras(SystemDB *db) {
	if(!SimpleSystemEntity::LoadExtras(db))
		return(false);
	
	//TODO: fire up the belt manager.
	//m_manager = new AsteroidBeltManager(new MiningDB(db), data.itemID);
	//TODO: decide if we are really going to try to use a MiningDB wrapper, or if we 
	// should just cram it into the system DB.. Obviously a mining DB is desirable, but
	//  we need to solve the issue of who owns the object and where to keep it.
	
	return(true);
}

void SystemAsteroidBeltEntity::Process() {
	SimpleSystemEntity::Process();
	
}






//TODO: split this into its own file.
#include "../inventory/InventoryItem.h"

SystemDungeonEntranceEntity::SystemDungeonEntranceEntity(SystemManager *system, InventoryItem *self)
: ItemSystemEntity(self),
  m_system(system)
{
	//will use effects.WarpGateEffect
}

//this is a big hack just to document the kind of stuff a dungeon conveys.
PyRepDict *SystemDungeonEntranceEntity::MakeSlimItem() const {
	
	PyRepDict *slim = ItemSystemEntity::MakeSlimItem();
	if(slim == NULL)
		return(NULL);
	
	//slim->add("itemID", new PyRepInteger(2100000396));
	//slim->add("typeID", new PyRepInteger(12273));
	//slim->add("ownerID", new PyRepInteger(500021));
	
	slim->add("dunSkillLevel", new PyRepInteger(0));
	slim->add("dunSkillTypeID", new PyRepNone);
	slim->add("dunObjectID", new PyRepInteger(160449));
	slim->add("dunWipeNPC", new PyRepInteger(1));
	slim->add("dunToGateID", new PyRepInteger(160484));
	slim->add("dunCloaked", new PyRepInteger(0));
	slim->add("dunScenarioID", new PyRepInteger(23));
	slim->add("dunSpawnID", new PyRepInteger(4));
	slim->add("dunAmount", new PyRepReal(0.0));
	slim->add("dunShipClasses", new PyRepList(/*237, 31*/));
	slim->add("dunDirection", new PyRepList(/*235, 0, 1*/));
	slim->add("dunKeyLock", new PyRepInteger(0));
	//slim->add("dunKeyQuantity", new PyRepInteger(1));
	//slim->add("dunKeyTypeID", new PyRepInteger(21839));
	//slim->add("dunOpenUntil", new PyRepInteger(Win32TimeNow()+Win32Time_Hour));
	slim->add("dunMusicUrl", new PyRepString("res:/Sound/Music/Ambient031combat.ogg"));
	
	return(slim);
}

void SystemDungeonEntranceEntity::EncodeDestiny(std::vector<byte> &into) const {
	#pragma pack(1)
	struct AddBall_Dungeon {
		BallHeader head;
		MassSector mass;
		DSTBALL_STOP_Struct main;
		MiniBallList miniballs;
		NameStruct name;
	};
	#pragma pack()
	
	int miniball_count = 1;

	const std::string itemName = m_self->itemName();
	
	int start = into.size();
	into.resize(start 
		+ sizeof(AddBall_Dungeon)
		+ (miniball_count-1)*sizeof(MiniBall)
		+ itemName.length()*sizeof(uint16) );
	byte *ptr = &into[start];
	AddBall_Dungeon *item = (AddBall_Dungeon *) ptr;
	ptr += sizeof(AddBall_Dungeon);

	item->head.entityID = m_self->itemID();
	item->head.mode = Destiny::DSTBALL_STOP;
	item->head.radius = m_self->radius();
	const GPoint &pos = m_self->position();
	item->head.x = pos.x;
	item->head.y = pos.y;
	item->head.z = pos.z;
	item->head.sub_type = AddBallSubType_dungeonEntrance;
	item->mass.mass = 10000000000.00;
	item->mass.unknown51 = 0;
	item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
	item->mass.corpID = m_self->ownerID();	//a little hacky...
	item->mass.allianceID = 0xFFFFFFFF;
	item->main.formationID = 0xFF;
	
	item->miniballs.count = miniball_count;
	//TOTAL CRAP: we do not have this in our DB right now... 
	item->miniballs.balls[0].x = -7701.181;
	item->miniballs.balls[0].y = 8060.06;
	item->miniballs.balls[0].z = 2778.900;
	item->miniballs.balls[0].radius = m_self->radius();

	//slide pointer over points...
	item = (AddBall_Dungeon *) (((byte *) item)+(miniball_count-1)*sizeof(MiniBall));

	item->name.name_len = itemName.length();
	strcpy_fake_unicode(item->name.name, itemName.c_str());
}































