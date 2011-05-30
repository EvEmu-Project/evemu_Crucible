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

SystemManager::SystemManager(uint32 systemID, PyServiceMgr &svc)//, ItemData idata)
: m_systemID(systemID),
  m_systemName(""),
  m_services(svc),
  m_spawnManager(new SpawnManager(*this, m_services)),
  m_entityChanged(false)//,
//  InventoryItem( svc.item_factory, systemID, *(svc.item_factory.GetType( 5 )), idata )
{
	m_db.GetSystemInfo(GetID(), NULL, NULL, &m_systemName, &m_systemSecurity);

    m_solarSystemRef = svc.item_factory.GetSolarSystem( systemID );
    uint32 inventoryID = m_solarSystemRef->itemID();

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
        if( itemFactory().GetItem( cur->itemID ) )
        {
            if( itemFactory().GetItem( cur->itemID )->categoryID() == EVEDB::invCategories::Station )
            {
                StationRef station = Station::Load( itemFactory(), cur->itemID );
                StationEntity *stationEntity = new StationEntity( station, this, *(GetServiceMgr()), cur->position );
		        if(stationEntity == NULL) {
			        codelog(SERVICE__ERROR, "Failed to create entity for item %u (type %u)", cur->itemID, cur->typeID);
			        continue;
		        }

                // Create default dynamic attributes in the AttributeMap:
                StationRef stationRef = stationEntity->GetStationObject();
                stationRef->SetAttribute(AttrIsOnline,      1);                                        // Is Online
                stationRef->SetAttribute(AttrDamage,        0.0);                                        // Structure Damage
                stationRef->SetAttribute(AttrShieldCapacity,20000000.0);                         // Shield Capacity
                stationRef->SetAttribute(AttrShieldCharge,  stationRef->GetAttribute(AttrShieldCapacity));     // Shield Charge
                stationRef->SetAttribute(AttrArmorHP,       stationRef->type().attributes.armorHP());   // Armor HP
                stationRef->SetAttribute(AttrArmorDamage,   0.0);                                   // Armor Damage
                stationRef->SetAttribute(AttrMass,          stationRef->type().attributes.mass());         // Mass
                stationRef->SetAttribute(AttrRadius,        stationRef->type().attributes.radius());     // Radius
                stationRef->SetAttribute(AttrVolume,        stationRef->type().attributes.volume());     // Volume

                m_entities[stationEntity->GetStationObject()->itemID()] = stationEntity;
		        bubbles.Add(stationEntity, true);
		        m_entityChanged = true;
            }
            else
            {
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
        }
	}
	
	return true;
}

class DynamicEntityFactory {
public:
	static SystemEntity *BuildEntity(SystemManager &system, ItemFactory &factory, const DBSystemDynamicEntity &entity) {
        GPoint location;

        switch(entity.categoryID) {
            case EVEDB::invCategories::Asteroid: {
				// Asteroids of all kinds!
                location.x = entity.x;
                location.y = entity.y;
                location.z = entity.z;

                ItemData idata(
                    entity.typeID,
                    entity.ownerID,
		            entity.locationID,
		            flagAutoFit,
                    entity.itemName.c_str(),
		            location
	            );

                InventoryItemRef asteroid = system.GetServiceMgr()->item_factory.GetItem( entity.itemID );
	            if( !asteroid )
                    throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                // Add the ItemRef to SystemManagers' Inventory:
                system.AddItemToInventory( asteroid );

                // TODO: this should really be loaded from the 'entity_attributes' table assuming it's been saved there for
                // each asteroid, amongst other attributes for asteroids.  Attribute manager should have already done this.
                //asteroid->SetAttribute(AttrRadius, EvilNumber(600));

                AsteroidEntity * asteroidObj = new AsteroidEntity( asteroid, &system, *(system.GetServiceMgr()), location );
                return asteroidObj;
			} break;
			case EVEDB::invCategories::Ship: {
                // Ships of all kinds NOT owned by any player but only by the EVE System (ownerID = 1):
                if( entity.ownerID == 1 )
                {
                    location.x = entity.x;
                    location.y = entity.y;
                    location.z = entity.z;

                    ItemData idata(
                        entity.typeID,
                        1,  // owner is EVE System (itemID = 1 from 'entity' table)
		                entity.locationID,
		                flagAutoFit,
                        entity.itemName.c_str(),
		                location
	                );

                    ShipRef ship = system.GetServiceMgr()->item_factory.GetShip( entity.itemID );
	                if( !ship )
                        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                    // Add the ItemRef to SystemManagers' Inventory:
                    system.AddItemToInventory( ship );

                    ShipEntity * shipObj = new ShipEntity( ship, &system, *(system.GetServiceMgr()), location );
                    return shipObj;
                }
                else
                    sLog.Error("DynamicEntityFactory::BuildEntity()", "We do not want to create ShipEntity objects for ANY ships owned by players, only those left in space owned by no one." );
			} break;
            case EVEDB::invCategories::Deployable: {        // Deployable structures of all kinds!  Warp disruptor bubbles.
                location.x = entity.x;
                location.y = entity.y;
                location.z = entity.z;

                ItemData idata(
                    entity.typeID,
                    entity.ownerID,
		            entity.locationID,
		            flagAutoFit,
                    entity.itemName.c_str(),
		            location
	            );

                InventoryItemRef deployable = system.GetServiceMgr()->item_factory.GetItem( entity.itemID );
	            if( !deployable )
                    throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                // Set radius of warp disruptor bubble
                // TODO: GET THIS FROM DB 'entity_attributes' perhaps
                //deployable->SetAttribute(AttrRadius, EvilNumber(deployable->type().radius()) );     // Can you set this somehow from the type class ?

                // Add the ItemRef to SystemManagers' Inventory:
                system.AddItemToInventory( deployable );

                DeployableEntity * deployableObj = new DeployableEntity( deployable, &system, *(system.GetServiceMgr()), location );
                return deployableObj;
			} break;
            case EVEDB::invCategories::Structure: {         // Structures of all kinds!  POS towers, modules, and equipment
                location.x = entity.x;
                location.y = entity.y;
                location.z = entity.z;

                ItemData idata(
                    entity.typeID,
                    entity.ownerID,
		            entity.locationID,
		            (EVEItemFlags)entity.flag,
                    entity.itemName.c_str(),
		            location
	            );

                //InventoryItemRef structure = system.GetServiceMgr()->item_factory.GetItem( entity.itemID );
                StructureRef structure = system.GetServiceMgr()->item_factory.GetStructure( entity.itemID );
	            if( !structure )
                    throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                // Add the ItemRef to SystemManagers' Inventory:
                system.AddItemToInventory( structure );

                StructureEntity * structureObj = new StructureEntity( structure, &system, *(system.GetServiceMgr()), location );
                
                return structureObj;
			} break;
            case EVEDB::invCategories::Celestial: {
                // Test either groupID to selectively spawn either a generic celestial or some kind of cargo container
                // groupIDs to test for:
                // * ContainerEntity <-- Audit_Log_Secure_Container "OR" Secure_Cargo_Container "OR" Cargo_Container "OR" Freight_Container
                // * CelestialEntity <-- Biomass "OR" Wreck "OR" Large_Collidable_Object "OR" Cloud "OR" Comet "OR" 
                //                       Construction_Platform "OR" Beacon "OR" Planetary_Cloud "OR" Landmark "OR" 
                //                       Global_Warp_Disruptor "OR" Shipping_Crates "OR" Force_Field "OR" 
                //                       Cosmic_Signature "OR" Harvestable_Cloud "OR" Station_Upgrade_Platform "OR" 
                //                       Station_Improvement_Platform "OR" Destructable_Station_Services "OR" 
                //                       Cosmic_Anomaly "OR" Covert_Beacon "OR" Effect_Beacon

                // TODO: (just use CelestialEntity class for these until their own classes are writen)
                // * WarpGateEntity  <-- Warp_Gate
                // * WormholeEntity  <-- Wormhole

                if( (entity.groupID == EVEDB::invGroups::Wreck)
                    || (entity.groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
                    || (entity.groupID == EVEDB::invGroups::Secure_Cargo_Container)
                    || (entity.groupID == EVEDB::invGroups::Cargo_Container)
                    || (entity.groupID == EVEDB::invGroups::Freight_Container) )
                {
                    location.x = entity.x;
                    location.y = entity.y;
                    location.z = entity.z;

                    ItemData idata(
                        entity.typeID,
                        entity.ownerID,
		                entity.locationID,
		                (EVEItemFlags)entity.flag,
                        entity.itemName.c_str(),
		                location
	                );

                    //InventoryItemRef container = system.GetServiceMgr()->item_factory.GetItem( entity.itemID );
                    CargoContainerRef container = system.GetServiceMgr()->item_factory.GetCargoContainer( entity.itemID );
	                if( !container )
                        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                    // Add the ItemRef to SystemManagers' Inventory:
                    system.AddItemToInventory( container );

                    ContainerEntity* containerObj = new ContainerEntity( container, &system, *(system.GetServiceMgr()), location );
	                system.AddEntity( containerObj );
                    return containerObj;
                }
                else if( (entity.groupID == EVEDB::invGroups::Biomass)
                    || (entity.groupID == EVEDB::invGroups::Sun) || (entity.groupID == EVEDB::invGroups::Planet)
                    || (entity.groupID == EVEDB::invGroups::Moon) || (entity.groupID == EVEDB::invGroups::Stargate)
                    || (entity.groupID == EVEDB::invGroups::Ring) || (entity.groupID == EVEDB::invGroups::Secondary_Sun)
                    || (entity.groupID == EVEDB::invGroups::Large_Collidable_Object) || (entity.groupID == EVEDB::invGroups::Cloud)
                    || (entity.groupID == EVEDB::invGroups::Harvestable_Cloud) || (entity.groupID == EVEDB::invGroups::Planetary_Cloud)
                    || (entity.groupID == EVEDB::invGroups::Landmark) || (entity.groupID == EVEDB::invGroups::Global_Warp_Disruptor)
                    || (entity.groupID == EVEDB::invGroups::Shipping_Crates) || (entity.groupID == EVEDB::invGroups::Force_Field)
                    || (entity.groupID == EVEDB::invGroups::Cosmic_Signature) || (entity.groupID == EVEDB::invGroups::Cosmic_Anomaly)
                    || (entity.groupID == EVEDB::invGroups::Beacon) || (entity.groupID == EVEDB::invGroups::Covert_Beacon)
                    || (entity.groupID == EVEDB::invGroups::Effect_Beacon) || (entity.groupID == EVEDB::invGroups::Station_Upgrade_Platform)
                    || (entity.groupID == EVEDB::invGroups::Construction_Platform) || (entity.groupID == EVEDB::invGroups::Large_Collidable_Structure)
                    || (entity.groupID == EVEDB::invGroups::Station_Improvement_Platform) || (entity.groupID == EVEDB::invGroups::Destructable_Station_Services)
                    || (entity.groupID == EVEDB::invGroups::Warp_Gate) || (entity.groupID == EVEDB::invGroups::Wormhole)
                    || (entity.groupID == EVEDB::invGroups::Comet) )
                {
                    location.x = entity.x;
                    location.y = entity.y;
                    location.z = entity.z;

                    ItemData idata(
                        entity.typeID,
                        1,  // owner is EVE System (itemID = 1 from 'entity' table)
		                entity.locationID,
		                flagAutoFit,
                        entity.itemName.c_str(),
		                location
	                );

                    CelestialObjectRef celestial = system.GetServiceMgr()->item_factory.GetCelestialObject( entity.itemID );
	                if( !celestial )
                        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                    // Set radius of celestial object
                    // TODO: GET THIS FROM DB 'entity_attributes' perhaps
                    //celestial->Set_radius( 5000 );
                    celestial->SetAttribute(AttrRadius, EvilNumber(celestial->type().radius()) );     // Can you set this somehow from the type class ?

                    // Add the ItemRef to SystemManagers' Inventory:
                    system.AddItemToInventory( celestial );

                    CelestialEntity* celestialObj = new CelestialEntity( celestial, &system, *(system.GetServiceMgr()), location );
	                system.AddEntity( celestialObj );
                    return celestialObj;
                }
			} break;
            case EVEDB::invCategories::Entity: {            // Entities (some kinds of wrecks)
                if( entity.groupID == EVEDB::invGroups::Spawn_Container )
                {
                    // For category=Entity, group=Spawn Container, create a CargoContainer object:
                    location.x = entity.x;
                    location.y = entity.y;
                    location.z = entity.z;

                    ItemData idata(
                        entity.typeID,
                        entity.ownerID,
		                entity.locationID,
		                (EVEItemFlags)entity.flag,
                        entity.itemName.c_str(),
		                location
	                );

                    //InventoryItemRef container = system.GetServiceMgr()->item_factory.GetItem( entity.itemID );
                    CargoContainerRef container = system.GetServiceMgr()->item_factory.GetCargoContainer( entity.itemID );
	                if( !container )
                        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                    // Add the ItemRef to SystemManagers' Inventory:
                    system.AddItemToInventory( container );

                    ContainerEntity* containerObj = new ContainerEntity( container, &system, *(system.GetServiceMgr()), location );
	                system.AddEntity( containerObj );
                    return containerObj;
                }
                else
                {
                    location.x = entity.x;
                    location.y = entity.y;
                    location.z = entity.z;

                    ItemData idata(
                        entity.typeID,
                        1,  // owner is EVE System (itemID = 1 from 'entity' table)
		                entity.locationID,
		                flagAutoFit,
                        entity.itemName.c_str(),
		                location
	                );

                    CelestialObjectRef celestial = system.GetServiceMgr()->item_factory.GetCelestialObject( entity.itemID );
	                if( !celestial )
                        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                    // Add the ItemRef to SystemManagers' Inventory:
                    system.AddItemToInventory( celestial );

                    CelestialEntity* celestialObj = new CelestialEntity( celestial, &system, *(system.GetServiceMgr()), location );
	                system.AddEntity( celestialObj );
                    return celestialObj;
                }
			} break;
            case EVEDB::invCategories::Drone: {             // Drones of all kinds!
                location.x = entity.x;
                location.y = entity.y;
                location.z = entity.z;

                ItemData idata(
                    entity.typeID,
                    1,  // owner is EVE System (itemID = 1 from 'entity' table)
		            entity.locationID,
		            flagAutoFit,
                    entity.itemName.c_str(),
		            location
	            );

                InventoryItemRef drone = system.GetServiceMgr()->item_factory.GetItem( entity.itemID );
	            if( !drone )
                    throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                // Add the ItemRef to SystemManagers' Inventory:
                system.AddItemToInventory( drone );

                DroneEntity * droneObj = new DroneEntity( drone, &system, *(system.GetServiceMgr()), location );
                return droneObj;
			} break;
            case EVEDB::invCategories::Station: {             // Dynamic Stations ONLY !!
                location.x = entity.x;
                location.y = entity.y;
                location.z = entity.z;

                ItemData idata(
                    entity.typeID,
                    entity.ownerID,
		            entity.locationID,
		            flagAutoFit,
                    entity.itemName.c_str(),
		            location
	            );

                StationRef station = system.GetServiceMgr()->item_factory.GetStation( entity.itemID );
	            if( !station )
                    throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );

                // Add the ItemRef to SystemManagers' Inventory:
                system.AddItemToInventory( station );

                StationEntity * stationObj = new StationEntity( station, &system, *(system.GetServiceMgr()), location );
                return stationObj;
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

bool SystemManager::BuildDynamicEntity(Client *who, const DBSystemDynamicEntity &entity)
{
    SystemEntity *se = DynamicEntityFactory::BuildEntity(*this, m_services.item_factory, entity );
	if( se == NULL )
    {
        sLog.Error( "SystemManager::BuildDynamicEntity()", "Failed to create entity for item %u (type %u)", entity.itemID, entity.typeID );
		return false;
	}
	
    sLog.Debug( "SystemManager::BuildDynamicEntity()", "Loaded dynamic entity %u of type %u for system %u", entity.itemID, entity.typeID, m_systemID );
	m_entities[se->GetID()] = se;
	bubbles.Add(se, false);
	m_entityChanged = true;

    return true;
}

void SystemManager::AddClient(Client *who) {
    AddEntity( who );
	//m_entities[who->GetID()] = who;
	//m_entityChanged = true;
	//this is actually handled in SetPosition via UpdateBubble.
	//if(who->IsInSpace()) {
	//	bubbles.Add(who, false);
	//}
	_log(CLIENT__TRACE, "%s: Added to system manager for %u", who->GetName(), m_systemID);

    // Add character's Ship Item Ref to Solar System dynamic inventory:
    //AddItemToInventory( who->GetShip() );
}

void SystemManager::RemoveClient(Client *who) {
	RemoveEntity(who);
	_log(CLIENT__TRACE, "%s: Removed from system manager for %u", who->GetName(), m_systemID);

    // Remove character's Ship Item Ref from Solar System dynamic inventory:
    //RemoveItemFromInventory( who->GetShip() );
}

void SystemManager::AddNPC(NPC *who) {
	//nothing special to do yet...
	AddEntity(who);

    // Add NPC's Item Ref to Solar System Dynamic Inventory:
    //AddItemToInventory( this->itemFactory().GetItem( who->GetID() ) );
}

void SystemManager::RemoveNPC(NPC *who) {
	//nothing special to do yet...
	RemoveEntity(who);

    // Remove NPC's Item Ref from Solar System Dynamic Inventory:
    //RemoveItemFromInventory( this->itemFactory().GetItem( who->GetID() ) );
}

void SystemManager::AddEntity(SystemEntity *who) {
	m_entities[who->GetID()] = who;
	m_entityChanged = true;
	bubbles.Add(who, false);

    // Add Entity's Item Ref to Solar System Dynamic Inventory:
    AddItemToInventory( this->itemFactory().GetItem( who->GetID() ) );
}

void SystemManager::RemoveEntity(SystemEntity *who) {
	std::map<uint32, SystemEntity *>::iterator itr = m_entities.find(who->GetID());
	if(itr != m_entities.end()) {
		m_entities.erase(itr);
		m_entityChanged = true;
	} else
		_log(SERVICE__ERROR, "Entity %u not found is system %u to be deleted.", who->GetID(), GetID());

	bubbles.Remove(who, false);

    // Remove Entity's Item Ref from Solar System Dynamic Inventory:
    RemoveItemFromInventory( this->itemFactory().GetItem( who->GetID() ) );
}

SystemEntity *SystemManager::get(uint32 entityID) const {
	std::map<uint32, SystemEntity *>::const_iterator res;
	res = m_entities.find(entityID);
	if(res == m_entities.end())
		return NULL;
	return(res->second);
}

/* maybe this is the reason why warping sucks... */
//in m/s
double SystemManager::GetWarpSpeed() const {
	//right now, warp speed is hard coded to 3 AU/s
	_log(COMMON__WARNING, "SystemManager::GetWarpSpeed is hard coded to 3 AU right now!");
	return(3.0f * ONE_AU_IN_METERS);
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

void SystemManager::AddItemToInventory(InventoryItemRef item)
{
    m_solarSystemRef->AddItemToInventory( item );
}

ShipRef SystemManager::GetShipFromInventory(uint32 shipID)
{
    return RefPtr<Ship>::StaticCast( m_solarSystemRef->GetByID( shipID ) );
}

void SystemManager::RemoveItemFromInventory(InventoryItemRef item)
{
    m_solarSystemRef->RemoveItemFromInventory( item );
}
