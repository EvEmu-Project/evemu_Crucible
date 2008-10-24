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

#include "EvemuPCH.h"


using namespace Destiny;

SystemManager::SystemManager(const char *systemName, uint32 systemID, DBcore *db, PyServiceMgr *svc)
: m_systemName(systemName),
  m_systemID(systemID),
  m_db(db),
  m_services(svc),
  m_spawnManager(new SpawnManager(db, this, m_services)),
  m_nextNPCID(900000000),	//TODO: look in logs to figure out what they really use.
  m_clientChanged(false),
  m_entityChanged(false)
{
	m_systemSecurity = m_db.GetSystemSecurity(m_systemID);
	//create our chat channel
	m_services->lsc_service->CreateSystemChannel(m_systemID);
}

SystemManager::~SystemManager() {
	//all clients get removed first without delete. they are owned by the entity list.
	std::set<Client *>::const_iterator curc, endc;
	curc = m_clients.begin();
	endc = m_clients.end();
	for(; curc != endc; curc++) {
		m_entities.erase((*curc)->GetID());
	}

	//now free any non-client entities
	std::map<uint32, SystemEntity *>::iterator cur, end, tmp;
	cur = m_entities.begin();
	end = m_entities.end();
	while(cur != end) {
		tmp = cur++;
		delete tmp->second;
	}

	//must be deleted AFTER all the NPCs which it spawn have been, since
	//they will call back to their spawning spawn entries.
	delete m_spawnManager;

	bubbles.clear();
}

static const int num_hack_sentry_locs = 8;
GPoint hack_sentry_locs[num_hack_sentry_locs] = {
	GPoint(-35000.0f, -35000.0f, -35000.0f),
	GPoint(-35000.0f, -35000.0f, 35000.0f),
	GPoint(-35000.0f, 35000.0f, -35000.0f),
	GPoint(-35000.0f, 35000.0f, 35000.0f),
	GPoint(35000.0f, -35000.0f, -35000.0f),
	GPoint(35000.0f, -35000.0f, 35000.0f),
	GPoint(35000.0f, 35000.0f, -35000.0f),
	GPoint(35000.0f, 35000.0f, 35000.0f)
};

bool SystemManager::_LoadSystemCelestials() {
	std::vector<DBSystemEntity> entities;
	if(!m_db.LoadSystemEntities(m_systemID, entities)) {
		_log(SERVICE__ERROR, "Unable to load celestial entities during boot of system %lu.", m_systemID);
		return(false);
    }

	//uint32 next_hack_entity_ID = m_systemID + 900000000;
	
	std::vector<DBSystemEntity>::iterator cur, end;
	cur = entities.begin();
	end = entities.end();
	for(; cur != end; ++cur) {
		SimpleSystemEntity *se = SimpleSystemEntity::MakeEntity(this, *cur);
		if(se == NULL) {
			codelog(SERVICE__ERROR, "Failed to create entity for item %lu (type %lu)", cur->itemID, cur->typeID);
			continue;
		}
		if(!se->LoadExtras(&m_db)) {
			_log(SERVICE__ERROR, "Failed to load additional data for entity %lu. Skipping.", se->GetID());
			delete se;
			continue;
		}
		m_entities[se->GetID()] = se;
		bubbles.Add(se, false);
		m_entityChanged = true;
	}
	
	return(true);
}

class DynamicEntityFactory {
public:
	static SystemEntity *BuildEntity(SystemManager *system, ItemFactory *factory, const DBSystemDynamicEntity &entity) {
		using namespace EVEDB;
		switch(entity.categoryID) {
		
		case invCategories::Asteroid: {
			//first load up the item.
			InventoryItem *i;
			i = factory->Load(entity.itemID, false);	//should not have any contents...
			if(i == NULL) {
				//this should not happen... we just got this list from the DB...
				codelog(SERVICE__ERROR, "Unable to load item for entity %lu", entity.itemID);
				return(NULL);
			}
			return(new Asteroid(system, i));	//takes a ref.
		} break;
		case invCategories::Ship: {
			codelog(SERVICE__ERROR, "Ship item in space unhandled: item %lu of type %lu", entity.itemID, entity.typeID);
			//TODO: figure out if it is occupied or not.. can we
			//filter this in the DB instead?
		} break;
		default:
			codelog(SERVICE__ERROR, "Unhandled dynamic entity category %d for item %lu of type %lu", entity.categoryID, entity.itemID, entity.typeID);
			break;
		}
		return(NULL);
	}
};

bool SystemManager::_LoadSystemDynamics() {
	std::vector<DBSystemDynamicEntity> entities;
	if(!m_db.LoadSystemDynamicEntities(m_systemID, entities)) {
		_log(SERVICE__ERROR, "Unable to load dynamic entities during boot of system %lu.", m_systemID);
		return(false);
    }

	//uint32 next_hack_entity_ID = m_systemID + 900000000;
	
	std::vector<DBSystemDynamicEntity>::iterator cur, end;
	cur = entities.begin();
	end = entities.end();
	for(; cur != end; cur++) {
		SystemEntity *se = DynamicEntityFactory::BuildEntity(this, m_services->item_factory, *cur);
		if(se == NULL) {
			codelog(SERVICE__ERROR, "Failed to create entity for item %lu (type %lu)", cur->itemID, cur->typeID);
			continue;
		}
		//TODO: use proper log type.
		_log(SPAWN__MESSAGE, "Loaded dynamic entity %lu of type %lu for system %lu", cur->itemID, cur->typeID, m_systemID);
		m_entities[se->GetID()] = se;
		bubbles.Add(se, false);
		m_entityChanged = true;
	}
	
	return(true);
}

bool SystemManager::BootSystem() {
	
	//load the static system stuff...
	if(!_LoadSystemCelestials())
		return(false);
	
	//load the dynamic system stuff (items, roids, etc...)
	if(!_LoadSystemDynamics())
		return(false);
	
	/* temporarily commented out until we find out why they
	 * make client angry ...
	//the statics have been loaded, now load up the spawns...
	if(!m_spawnManager->Load()) {
		_log(SERVICE__ERROR, "Unable to load spawns during boot of system %lu.", m_systemID);
		return(false);
	}
	//spawns are loaded, fire up the initial spawn.
	if(!m_spawnManager->DoInitialSpawn()) {
		_log(SERVICE__ERROR, "Unable to do initial spawns during boot of system %lu.", m_systemID);
		return(false);
	}
	*/
	
	return(true);
}

//called many times a second
bool SystemManager::Process() {
	m_entityChanged = false;
	
	std::map<uint32, SystemEntity *>::const_iterator cur;
	cur = m_entities.begin();
	while(cur != m_entities.end()) {
		cur->second->Process();
		if(m_entityChanged) {
			//somebody changed the entity list, need to start over or bail...
			m_entityChanged = false;
			cur = m_entities.begin();
		} else {
			cur++;
		}
	}
	
	bubbles.Process();
	
	return(true);
}

//called once per second.
void SystemManager::ProcessDestiny() {
	//this is here so it isnt called so frequently.
	m_spawnManager->Process();


	m_entityChanged = false;
	std::map<uint32, SystemEntity *>::const_iterator cur;
	cur = m_entities.begin();
	while(cur != m_entities.end()) {
		cur->second->ProcessDestiny();
		if(m_entityChanged) {
			//somebody changed the entity list, need to start over or bail...
			m_entityChanged = false;
			cur = m_entities.begin();
		} else {
			cur++;
		}
	}
}


void SystemManager::AddClient(Client *who) {
	m_clients.insert(who);
	m_clientChanged = true;
	m_entities[who->GetID()] = who;
	m_entityChanged = true;
	//this is actually handled in SetPosition via UpdateBubble.
	//if(who->IsInSpace()) {
	//	bubbles.Add(who, false);
	//}
	_log(CLIENT__TRACE, "%s: Added to system manager for %lu", who->GetName(), m_systemID);
}

void SystemManager::RemoveClient(Client *who) {
	m_clients.erase(who);
	m_clientChanged = true;
	
	RemoveEntity(who);
	_log(CLIENT__TRACE, "%s: Removed from system manager for %lu", who->GetName(), m_systemID);
}

void SystemManager::AddNPC(NPC *who) {
	//nothing special to do yet...
	AddEntity(who);
}

void SystemManager::RemoveNPC(NPC *who) {
	//nothing special to do yet...
	RemoveEntity(who);
}

void SystemManager::AddEntity(SystemEntity *who) {
	m_entities[who->GetID()] = who;
	m_entityChanged = true;
	bubbles.Add(who, false);
}

void SystemManager::RemoveEntity(SystemEntity *who) {
	std::map<uint32, SystemEntity *>::iterator itr = m_entities.find(who->GetID());
	if(itr != m_entities.end()) {
		m_entities.erase(itr);
		m_entityChanged = true;
	} else
		_log(SERVICE__ERROR, "Entity %lu not found is system %lu to be deleted.", who->GetID(), GetID());
	bubbles.Remove(who, false);
}

SystemEntity *SystemManager::get(uint32 entityID) const {
	std::map<uint32, SystemEntity *>::const_iterator res;
	res = m_entities.find(entityID);
	if(res == m_entities.end())
		return(NULL);
	return(res->second);
}

//in m/s
double SystemManager::GetWarpSpeed() const {
	static const double one_AU_in_m = 1.495978707e11;
	//right now, warp speed is hard coded to 3 AU/s
	_log(COMMON__WARNING, "SystemManager::GetWarpSpeed is hard coded to 3 AU right now!");
	return(3.0f * one_AU_in_m);
}

void SystemManager::MakeSetState(const SystemBubble *bubble, DoDestiny_SetState &ss) const {
	std::vector<byte> setstate_buffer(sizeof(Destiny::AddBall_header));
	setstate_buffer.reserve(10240);
	
	Destiny::AddBall_header *head = (Destiny::AddBall_header *) &setstate_buffer[0];
	head->more = 0;
	head->sequence = ss.stamp;
	
	//I am not thrilled with this mechanism, but I cant think of a better
	//way to deal with it right now. The issue is that we need to send out
	// info for all system-wide entities (celestials, etc), as well as all
	// entities in our current bubble. Well, it is likely that some things
	// in our bubble are system-wide, and we would be sending out duplciates.
	// so, we use a set to enforce uniqueness.
	std::set<SystemEntity *> visible_entities;
	{
		std::map<uint32, SystemEntity *>::const_iterator cur, end;
		cur = m_entities.begin();
		end = m_entities.end();
		for(; cur != end; ++cur) {
			if(!cur->second->IsVisibleSystemWide()) {
//_log(COMMON__WARNING, "%lu is not visible!", cur->first);
				continue;
			}
//_log(COMMON__WARNING, "%lu is system wide visible!", cur->first);
			visible_entities.insert(cur->second);
		}
	}

//bubble is null??? why???
	bubble->GetEntities(visible_entities);
	
	//go through all entities and gather the info we need...
	std::set<SystemEntity *>::const_iterator cur, end;
	cur = visible_entities.begin();
	end = visible_entities.end();
	for(; cur != end; ++cur) {
		SystemEntity *ent = *cur;
//_log(COMMON__WARNING, "Encoding entity %lu", ent->GetID());
		//ss.damageState
		ss.damageState[ ent->GetID() ] = ent->MakeDamageState();

		//ss.slims
		PyRepDict *slim_dict = ent->MakeSlimItem();
		ss.slims.add(new PyRepObject("foo.SlimItem", slim_dict));

		//append the destiny binary data...
		ent->EncodeDestiny(setstate_buffer);
	}

	//ss.destiny_state
	ss.destiny_state = new PyRepBuffer(&setstate_buffer[0], setstate_buffer.size());
	setstate_buffer.clear();
	
	//ss.gangCorps

	//ss.aggressors

	//ss.droneState
	ss.droneState = m_db.GetSolDroneState(m_systemID);
	if(ss.droneState == NULL) {
		_log(SERVICE__ERROR, "Unable to query dronestate entity for destiny update in system %lu!", m_systemID);
		ss.droneState = new PyRepNone();
	}

	//ss.solItem
	ss.solItem = m_db.GetSolRow(m_systemID);
	if(ss.solItem == NULL) {
		_log(CLIENT__ERROR, "Unable to query solarsystem entity for destiny update in system %lu!", m_systemID);
		ss.solItem = new PyRepNone();
	}

	
	_log(DESTINY__TRACE, "Set State:");
	ss.Dump(DESTINY__TRACE, "    ");
	_log(DESTINY__TRACE, "    Buffer:");
	_hex(DESTINY__TRACE, ss.destiny_state->GetBuffer(), ss.destiny_state->GetLength());
	
	_log(DESTINY__TRACE, "    Decoded:");
	Destiny::DumpUpdate(DESTINY__TRACE, ss.destiny_state->GetBuffer(), ss.destiny_state->GetLength());
}

//NOTE: this could be optimized to eliminate one clone of the payload.
void SystemManager::RangecastDestinyUpdate(const GPoint &pt, double range, PyRepTuple **in_payload) const {
	double range2 = range*range;
	
	PyRepTuple *payload = *in_payload;
	*in_payload = NULL;
	
	std::set<Client *>::const_iterator curc, endc;
	curc = m_clients.begin();
	endc = m_clients.end();
	for(; curc != endc; curc++) {
		const GPoint &position = (*curc)->GetPosition();
		GVector v(position, pt);
		if(v.lengthSquared() > range2)
			continue;
		//else, in range
		PyRepTuple *dup = payload->TypedClone();
		(*curc)->QueueDestinyUpdate(&dup);
		delete dup;	//just in case. We know these are clients, so we can count on them normally consuming it, so we do not need to try to optimize the clone like we do with entities
	}

	delete payload;
}

//NOTE: this could be optimized to eliminate one clone of the payload.
void SystemManager::RangecastDestinyEvent(const GPoint &pt, double range, PyRepTuple **in_payload) const {
	double range2 = range*range;
	
	PyRepTuple *payload = *in_payload;
	*in_payload = NULL;
	
	std::set<Client *>::const_iterator curc, endc;
	curc = m_clients.begin();
	endc = m_clients.end();
	for(; curc != endc; curc++) {
		const GPoint &position = (*curc)->GetPosition();
		GVector v(position, pt);
		if(v.lengthSquared() > range2)
			continue;
		//else, in range
		PyRepTuple *dup = payload->TypedClone();
		(*curc)->QueueDestinyEvent(&dup);
		delete dup;	//just in case. We know these are clients, so we can count on them normally consuming it, so we do not need to try to optimize the clone like we do with entities
	}

	delete payload;
}

//NOTE: this could be optimized to eliminate one clone of the payload.
void SystemManager::RangecastDestiny(const GPoint &pt, double range, std::vector<PyRepTuple *> &updates, std::vector<PyRepTuple *> &events) const {
	if(!updates.empty() || !events.empty()) {
		double range2 = range*range;
		std::vector<PyRepTuple *>::const_iterator cur, end;
		
		std::set<Client *>::const_iterator curc, endc;
		curc = m_clients.begin();
		endc = m_clients.end();
		for(; curc != endc; curc++) {
			const GPoint &position = (*curc)->GetPosition();
			GVector v(position, pt);
			if(v.lengthSquared() > range2)
				continue;
			//else, in range
			
			cur = updates.begin();
			end = updates.end();
			for(; cur != end; cur++) {
				PyRepTuple *t = (*cur)->TypedClone();
				(*curc)->QueueDestinyUpdate(&t);
				delete t;	//just in case. We know these are clients, so we can count on them normally consuming it, so we do not need to try to optimize the clone like we do with entities
			}
			
			cur = events.begin();
			end = events.end();
			for(; cur != end; cur++) {
				PyRepTuple *t = (*cur)->TypedClone();
				(*curc)->QueueDestinyEvent(&t);
				delete t;	//just in case. We know these are clients, so we can count on them normally consuming it, so we do not need to try to optimize the clone like we do with entities
			}
		}
	}

	//we consume all updates.
	
	std::vector<PyRepTuple *>::iterator curu, endu;
	curu = updates.begin();
	endu = updates.end();
	for(; curu != endu; curu++) {
		delete *curu;
	}
	updates.clear();
	
	curu = events.begin();
	endu = events.end();
	for(; curu != endu; curu++) {
		delete *curu;
	}
	events.clear();
}

ItemFactory *SystemManager::itemFactory() const {
	return(m_services->item_factory);
}









