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
	Author:		Zhur
*/

#include "EVEServerPCH.h"


using namespace Destiny;

SystemManager::SystemManager(uint32 systemID, PyServiceMgr &svc)
: m_systemID(systemID),
  m_systemName(""),
  m_services(svc),
  m_spawnManager(new SpawnManager(*this, m_services)),
  m_entityChanged(false)
{
	m_db.GetSystemInfo(GetID(), NULL, NULL, &m_systemName, &m_systemSecurity);
	//create our chat channel
	m_services.lsc_service->CreateSystemChannel(m_systemID);
}

SystemManager::~SystemManager() {
	//we mustn't delete clients because they are owned by the entity list.
	std::map<uint32, SystemEntity *>::iterator cur, end, tmp;
	cur = m_entities.begin();
	end = m_entities.end();
	while(cur != end) {
		SystemEntity *se = cur->second;
		cur++;

		if(!se->IsClient())
			delete se;
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
		_log(SERVICE__ERROR, "Unable to load celestial entities during boot of system %u.", m_systemID);
		return false;
    }

	//uint32 next_hack_entity_ID = m_systemID + 900000000;
	
	std::vector<DBSystemEntity>::iterator cur, end;
	cur = entities.begin();
	end = entities.end();
	for(; cur != end; ++cur) {
		SimpleSystemEntity *se = SimpleSystemEntity::MakeEntity(this, *cur);
		if(se == NULL) {
			codelog(SERVICE__ERROR, "Failed to create entity for item %u (type %u)", cur->itemID, cur->typeID);
			continue;
		}
		if(!se->LoadExtras(&m_db)) {
			_log(SERVICE__ERROR, "Failed to load additional data for entity %u. Skipping.", se->GetID());
			delete se;
			continue;
		}
		m_entities[se->GetID()] = se;
		bubbles.Add(se, false);
		m_entityChanged = true;
	}
	
	return true;
}

class DynamicEntityFactory {
public:
	static SystemEntity *BuildEntity(SystemManager &system, ItemFactory &factory, const DBSystemDynamicEntity &entity) {
		using namespace EVEDB;
		switch(entity.categoryID) {
			case invCategories::Asteroid: {
				//first load up the item.
				InventoryItemRef i = factory.GetItem( entity.itemID );	//should not have any contents...
				if( !i ) {
					//this should not happen... we just got this list from the DB...
					codelog(SERVICE__ERROR, "Unable to load item for entity %u", entity.itemID);
					return NULL;
				}
				return(new Asteroid(&system, i));	//takes a ref.
			} break;
			case invCategories::Ship: {
				codelog(SERVICE__ERROR, "Ship item in space unhandled: item %u of type %u", entity.itemID, entity.typeID);
				//TODO: figure out if it is occupied or not.. can we
				//filter this in the DB instead?
			} break;
			default: {
				codelog(SERVICE__ERROR, "Unhandled dynamic entity category %d for item %u of type %u", entity.categoryID, entity.itemID, entity.typeID);
			} break;
		}
		return NULL;
	}
};

bool SystemManager::_LoadSystemDynamics() {
	std::vector<DBSystemDynamicEntity> entities;
	if(!m_db.LoadSystemDynamicEntities(m_systemID, entities)) {
		_log(SERVICE__ERROR, "Unable to load dynamic entities during boot of system %u.", m_systemID);
		return false;
    }

	//uint32 next_hack_entity_ID = m_systemID + 900000000;
	
	std::vector<DBSystemDynamicEntity>::iterator cur, end;
	cur = entities.begin();
	end = entities.end();
	for(; cur != end; cur++) {
		SystemEntity *se = DynamicEntityFactory::BuildEntity(*this, m_services.item_factory, *cur);
		if(se == NULL) {
			codelog(SERVICE__ERROR, "Failed to create entity for item %u (type %u)", cur->itemID, cur->typeID);
			continue;
		}
		//TODO: use proper log type.
		_log(SPAWN__MESSAGE, "Loaded dynamic entity %u of type %u for system %u", cur->itemID, cur->typeID, m_systemID);
		m_entities[se->GetID()] = se;
		bubbles.Add(se, false);
		m_entityChanged = true;
	}
	
	return true;
}

bool SystemManager::BootSystem() {
	
	//load the static system stuff...
	if(!_LoadSystemCelestials())
		return false;
	
	//load the dynamic system stuff (items, roids, etc...)
	if(!_LoadSystemDynamics())
		return false;
	
	/* temporarily commented out until we find out why they
	 * make client angry ...
	//the statics have been loaded, now load up the spawns...
	if(!m_spawnManager->Load()) {
		_log(SERVICE__ERROR, "Unable to load spawns during boot of system %u.", m_systemID);
		return false;
	}
	//spawns are loaded, fire up the initial spawn.
	if(!m_spawnManager->DoInitialSpawn()) {
		_log(SERVICE__ERROR, "Unable to do initial spawns during boot of system %u.", m_systemID);
		return false;
	}
	*/
	
	return true;
}

//called many times a second
bool SystemManager::Process() {
	m_entityChanged = false;
	
	std::map<uint32, SystemEntity *>::const_iterator cur, end;
	cur = m_entities.begin();
	end = m_entities.end();
	while(cur != end) {
		cur->second->Process();

		if(m_entityChanged) {
			//somebody changed the entity list, need to start over or bail...
			m_entityChanged = false;

			cur = m_entities.begin();
			end = m_entities.end();
		} else {
			cur++;
		}
	}
	
	bubbles.Process();
	
	return true;
}

//called once per second.
void SystemManager::ProcessDestiny() {
	//this is here so it isnt called so frequently.
	m_spawnManager->Process();

	m_entityChanged = false;

	std::map<uint32, SystemEntity *>::const_iterator cur, end;
	cur = m_entities.begin();
	end = m_entities.end();
	while(cur != end) {
		cur->second->ProcessDestiny();

		if(m_entityChanged) {
			//somebody changed the entity list, need to start over or bail...
			m_entityChanged = false;

			cur = m_entities.begin();
			end = m_entities.end();
		} else {
			cur++;
		}
	}
}


void SystemManager::AddClient(Client *who) {
	m_entities[who->GetID()] = who;
	m_entityChanged = true;
	//this is actually handled in SetPosition via UpdateBubble.
	//if(who->IsInSpace()) {
	//	bubbles.Add(who, false);
	//}
	_log(CLIENT__TRACE, "%s: Added to system manager for %u", who->GetName(), m_systemID);
}

void SystemManager::RemoveClient(Client *who) {
	RemoveEntity(who);
	_log(CLIENT__TRACE, "%s: Removed from system manager for %u", who->GetName(), m_systemID);
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
		_log(SERVICE__ERROR, "Entity %u not found is system %u to be deleted.", who->GetID(), GetID());

	bubbles.Remove(who, false);
}

SystemEntity *SystemManager::get(uint32 entityID) const {
	std::map<uint32, SystemEntity *>::const_iterator res;
	res = m_entities.find(entityID);
	if(res == m_entities.end())
		return NULL;
	return(res->second);
}

//in m/s
double SystemManager::GetWarpSpeed() const {
	static const double one_AU_in_m = 1.495978707e11;
	//right now, warp speed is hard coded to 3 AU/s
	_log(COMMON__WARNING, "SystemManager::GetWarpSpeed is hard coded to 3 AU right now!");
	return(3.0f * one_AU_in_m);
}

void SystemManager::MakeSetState(const SystemBubble *bubble, DoDestiny_SetState &ss) const
{
    Buffer* stateBuffer = new Buffer;

    AddBall_header head;
	head.more = 0;
	head.sequence = ss.stamp;
    stateBuffer->Append( head );
	
	//I am not thrilled with this mechanism, but I cant think of a better
	//way to deal with it right now. The issue is that we need to send out
	// info for all system-wide entities (celestials, etc), as well as all
	// entities in our current bubble. Well, it is likely that some things
	// in our bubble are system-wide, and we would be sending out duplciates.
	// so, we use a set to enforce uniqueness.
	std::set<SystemEntity*> visibleEntities;
	{
		std::map<uint32, SystemEntity*>::const_iterator cur, end;
		cur = m_entities.begin();
		end = m_entities.end();
		for(; cur != end; ++cur)
        {
			if( !cur->second->IsVisibleSystemWide() )
            {
                //_log(COMMON__WARNING, "%u is not visible!", cur->first);
				continue;
			}

            //_log(COMMON__WARNING, "%u is system wide visible!", cur->first);
			visibleEntities.insert( cur->second );
		}
	}

    //bubble is null??? why???
	bubble->GetEntities( visibleEntities );

    PySafeDecRef( ss.slims );
    ss.slims = new PyList;

	//go through all entities and gather the info we need...
	std::set<SystemEntity*>::const_iterator cur, end;
	cur = visibleEntities.begin();
	end = visibleEntities.end();
	for(; cur != end; ++cur)
    {
		SystemEntity* ent = *cur;
        //_log(COMMON__WARNING, "Encoding entity %u", ent->GetID());

		//ss.damageState
		ss.damageState[ ent->GetID() ] = ent->MakeDamageState();

		//ss.slims
		ss.slims->AddItem( new PyObject( new PyString( "foo.SlimItem" ), ent->MakeSlimItem() ) );

		//append the destiny binary data...
		ent->EncodeDestiny( *stateBuffer );
	}

	//ss.destiny_state
	ss.destiny_state = new PyBuffer( &stateBuffer );
    SafeDelete( stateBuffer );
	
	//ss.gangCorps

	//ss.aggressors

	//ss.droneState
	ss.droneState = m_db.GetSolDroneState( m_systemID );
	if( NULL == ss.droneState )
    {
		_log( SERVICE__ERROR, "Unable to query dronestate entity for destiny update in system %u!", m_systemID );
		ss.droneState = new PyNone;
	}

	//ss.solItem
	ss.solItem = m_db.GetSolRow( m_systemID );
	if( NULL == ss.solItem )
    {
		_log( CLIENT__ERROR, "Unable to query solarsystem entity for destiny update in system %u!", m_systemID );
		ss.solItem = new PyNone;
	}

	//ss.effectStates
    ss.effectStates = new PyList;

	//ss.allianceBridges
    ss.allianceBridges = new PyList;

    _log( DESTINY__TRACE, "Set State:" );
    ss.Dump( DESTINY__TRACE, "    " );
    _log( DESTINY__TRACE, "    Buffer:" );
    _hex( DESTINY__TRACE, &( ss.destiny_state->content() )[0],
                          ss.destiny_state->content().size() );

    _log( DESTINY__TRACE, "    Decoded:" );
    Destiny::DumpUpdate( DESTINY__TRACE, &( ss.destiny_state->content() )[0],
                                         ss.destiny_state->content().size() );
}

ItemFactory& SystemManager::itemFactory() const
{
	return m_services.item_factory;
}









