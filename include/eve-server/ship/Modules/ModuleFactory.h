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
	Author:		Luck
*/


#ifndef __MODULEFACTORY_H__
#define __MODULEFACTORY_H__

//#include "EVEServerPCH.h"
#include "ActiveModules.h"
#include "PassiveModules.h"
#include "RigModule.h"
#include "SubSystemModules.h"

//how you should access the modules
static GenericModule* ModuleFactory(InventoryItemRef item, ShipRef ship)
{

	if( item->categoryID() != EVEDB::invCategories::Module )
	{
		sLog.Error("ModuleFactory","Cannot create module from non module type item");
		assert(false);
	}
	else
	{
		switch(item->groupID())
		{
            // Armor Modules Subgroup:
            case EVEDB::invGroups::Damage_Control:                          return NULL;    // Active
            case EVEDB::invGroups::Armor_Repair_Unit:                       return NULL;    // Active
            case EVEDB::invGroups::Hull_Repair_Unit:                        return NULL;    // Active
            case EVEDB::invGroups::Reinforced_Bulkheads:                    return NULL;    // Passive
            case EVEDB::invGroups::Armor_Coating:                           return NULL;    // Passive
            case EVEDB::invGroups::Armor_Repair_Projector:                  return NULL;    // Active
            case EVEDB::invGroups::Armor_Plating_Energized:                 return NULL;    // Active
            case EVEDB::invGroups::Armor_Hardener:                          return NULL;    // Active
            case EVEDB::invGroups::Armor_Reinforcer:                        new PassiveModule(item, ship); break;
            case EVEDB::invGroups::Remote_Hull_Repairer:                    return NULL;    // Active
            case EVEDB::invGroups::Expanded_Cargohold:                      return NULL;    // Passive

            // Electronics Modules Subgroup:
            case EVEDB::invGroups::Cargo_Scanner:                           return NULL;    // Active
            case EVEDB::invGroups::Ship_Scanner:                            return NULL;    // Active
            case EVEDB::invGroups::Survey_Scanner:                          return NULL;    // Active
            case EVEDB::invGroups::Cloaking_Device:                         return NULL;    // Active
            case EVEDB::invGroups::Target_Painter:                          return NULL;    // Active
            case EVEDB::invGroups::Drone_Control_Unit:                      return NULL;    // Passive
            case EVEDB::invGroups::System_Scanner:                          return NULL;    // Active
            case EVEDB::invGroups::Scan_Probe_Launcher:                     return NULL;    // Active
            case EVEDB::invGroups::Drone_Navigation_Computer:               return NULL;    // Passive
            case EVEDB::invGroups::Drone_Tracking_Modules:                  return NULL;    // Passive
            case EVEDB::invGroups::Drone_Control_Range_Module:              return NULL;    // Passive
            case EVEDB::invGroups::Tractor_Beam:                            return NULL;    // Active

            // Engineering Modules Subgroup:
            case EVEDB::invGroups::Capacitor_Recharger:                     return NULL;    // Passive
            case EVEDB::invGroups::Capacitor_Battery:                       return NULL;    // Passive
            case EVEDB::invGroups::Energy_Transfer_Array:                   return NULL;    // Active
            case EVEDB::invGroups::Capacitor_Booster:                       return NULL;    // Active
            case EVEDB::invGroups::Auxiliary_Power_Core:                    new PassiveModule(item, ship); break;
            case EVEDB::invGroups::Power_Diagnostic_System:                 return NULL;    // Passive
            case EVEDB::invGroups::Capacitor_Power_Relay:                   return NULL;    // Passive
            case EVEDB::invGroups::Capacitor_Flux_Coil:                     return NULL;    // Passive
            case EVEDB::invGroups::Reactor_Control_Unit:                    return NULL;    // Passive
            case EVEDB::invGroups::Shield_Flux_Coil:                        return NULL;    // Passive

            // EWAR Modules Subgroup:
            case EVEDB::invGroups::Warp_Scrambler:                          return NULL;    // Active
            case EVEDB::invGroups::Stasis_Web:                              return NULL;    // Active
            case EVEDB::invGroups::ECM_Burst:                               return NULL;    // Active
            case EVEDB::invGroups::Passive_Targeting_System:                return NULL;    // Active
            case EVEDB::invGroups::Automated_Targeting_System:              return NULL;    // Active
            case EVEDB::invGroups::ECM:                                     return NULL;    // Active
            case EVEDB::invGroups::ECCM:                                    return NULL;    // Active
            case EVEDB::invGroups::Sensor_Backup_Array:                     return NULL;    // Active
            case EVEDB::invGroups::Remote_Sensor_Damper:                    return NULL;    // Active
            case EVEDB::invGroups::Tracking_Link:                           return NULL;    // Active
            case EVEDB::invGroups::Signal_Amplifier:                        return NULL;    // Active
            case EVEDB::invGroups::Tracking_Enhancer:                       return NULL;    // Active
            case EVEDB::invGroups::Sensor_Booster:                          return NULL;    // Active
            case EVEDB::invGroups::Tracking_Computer:                       return NULL;    // Active
            case EVEDB::invGroups::CPU_Enhancer:                            return NULL;    // Passive
            case EVEDB::invGroups::Projected_ECCM:                          return NULL;    // Active
            case EVEDB::invGroups::Remote_Sensor_Booster:                   return NULL;    // Active
            case EVEDB::invGroups::Tracking_Disruptor:                      return NULL;    // Active
            case EVEDB::invGroups::ECM_Stabilizer:                          return NULL;    // Active
            case EVEDB::invGroups::Remote_ECM_Burst:                        return NULL;    // Active

            // Gang Assist Modules Subgroup:
            case EVEDB::invGroups::Gang_Coordinator:                        return NULL;    // Active
            case EVEDB::invGroups::Siege_Module:                            return NULL;    // Active
            case EVEDB::invGroups::Data_Miners:                             return NULL;    // Active
            case EVEDB::invGroups::Jump_Portal_Generator:                   return NULL;    // Active
            case EVEDB::invGroups::Cynosural_Field:                         return NULL;    // Active
            case EVEDB::invGroups::Clone_Vat_Bay:                           return NULL;    // Passive

            // Mining Modules Subgroup:
            case EVEDB::invGroups::Mining_Laser:                            return NULL;    // Active
            case EVEDB::invGroups::Strip_Miner:                             return NULL;    // Active
            case EVEDB::invGroups::Frequency_Mining_Laser:                  return NULL;    // Active
            case EVEDB::invGroups::Mining_Upgrade:                          return NULL;    // Passive
            case EVEDB::invGroups::Gas_Cloud_Harvester:                     return NULL;    // Active

            // Propulsion Modules Subgroup:
            case EVEDB::invGroups::Afterburner:                             return new Afterburner(item, ship); break;
            case EVEDB::invGroups::Warp_Core_Stabilizer:                    return NULL;    // Passive
            case EVEDB::invGroups::Inertial_Stabilizer:                     return NULL;    // Passive
            case EVEDB::invGroups::Nanofiber_Internal_Structure:            return NULL;    // Passive
            case EVEDB::invGroups::Overdrive_Injector_System:               return NULL;    // Passive

            // Shield Modules Subgroup:
            case EVEDB::invGroups::Shield_Extender:                         return NULL;    // Passive
            case EVEDB::invGroups::Shield_Recharger:                        return NULL;    // Active
            case EVEDB::invGroups::Shield_Booster:                          return NULL;    // Active
            case EVEDB::invGroups::Shield_Transporter:                      return NULL;    // Active
            case EVEDB::invGroups::Shield_Power_Relay:                      return NULL;    // Passive
            case EVEDB::invGroups::Shield_Hardener:                         return NULL;    // Active
            case EVEDB::invGroups::Shield_Amplifier:                        return NULL;    // Active
            case EVEDB::invGroups::Shield_Boost_Amplifier:                  return NULL;    // Active
            case EVEDB::invGroups::Shield_Disruptor:                        return NULL;    // Active

            // Weapon Modules Subgroup:
            case EVEDB::invGroups::Energy_Weapon:                           return NULL;    // Active
            case EVEDB::invGroups::Projectile_Weapon:                       return NULL;    // Active
            case EVEDB::invGroups::Gyrostabilizer:                          return NULL;    // Passive
            case EVEDB::invGroups::Energy_Vampire:                          return NULL;    // Active
            case EVEDB::invGroups::Energy_Destabilizer:                     return NULL;    // Active
            case EVEDB::invGroups::Smart_Bomb:                              return NULL;    // Active
            case EVEDB::invGroups::Hybrid_Weapon:                           return NULL;    // Active
            case EVEDB::invGroups::Heat_Sink:                               return NULL;    // Passive
            case EVEDB::invGroups::Magnetic_Field_Stabilizer:               return NULL;    // Passive
            case EVEDB::invGroups::Ballistic_Control_system:                new PassiveModule(item, ship); break;
            case EVEDB::invGroups::Missile_Launcher_Snowball:               return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Cruise:                 return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Rocket:                 return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Siege:                  return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Standard:               return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Heavy:                  return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Assault:                return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Defender:               return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Citadel:                return NULL;    // Active
            case EVEDB::invGroups::Super_Weapon:                            return NULL;    // Active
            case EVEDB::invGroups::Interdiction_Sphere_Launcher:            return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Heavy_Assault:          return NULL;    // Active
            case EVEDB::invGroups::Missile_Launcher_Bomb:                   return NULL;    // Active
            case EVEDB::invGroups::Warp_Disrupt_Field_Generator:            return NULL;    // Active


            // Uncategorized and Unknown Modules Groups (some of these groups contain NO REAL typeIDs in the 'invTypes' table:
            case EVEDB::invGroups::Computer_Interface_Node:                 return NULL;
            case EVEDB::invGroups::GM_Modules:                              return NULL;
            case EVEDB::invGroups::Cruise_Control:                          return NULL;
            case EVEDB::invGroups::Smartbomb_Supercharger:                  return NULL;
            case EVEDB::invGroups::Anti_Ballistic_Defense_System:           return NULL;
            case EVEDB::invGroups::Microwarpdrive:                          return NULL;
            case EVEDB::invGroups::New_EW_Testing:                          return NULL;
            case EVEDB::invGroups::Missile_Launcher:                        return NULL;
            case EVEDB::invGroups::Countermeasure_Launcher:                 return NULL;
            case EVEDB::invGroups::Anti_Cloaking_Pulse:                     return NULL;
            case EVEDB::invGroups::Signature_Scrambling:                    return NULL;
            case EVEDB::invGroups::Energy_Vampire_Slayer:                   return NULL;
            case EVEDB::invGroups::Cheat_Module_Group:                      return NULL;
            case EVEDB::invGroups::Autopilot:                               return NULL;
            case EVEDB::invGroups::DroneBayExpander:                        return NULL;
            case EVEDB::invGroups::Drone_Modules:                           return NULL;
            case EVEDB::invGroups::Navigation_Computer:                     return NULL;
            case EVEDB::invGroups::Super_Gang_Enhancer:                     return NULL;
            case EVEDB::invGroups::Drone_Damage_Modules:                    return NULL;
            case EVEDB::invGroups::ECM_Enhancer:                            return NULL;
            case EVEDB::invGroups::Cloak_Enhancements:                      return NULL;
            case EVEDB::invGroups::Mining_Enhancer:                         return NULL;
            case EVEDB::invGroups::Covert_Cynosural_Field_Generator:        return NULL;


            /************************************/
            /*              Rigs                */
            /************************************/

            case EVEDB::invGroups::Rig_Armor:                               return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Shield:                              return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Energy_Weapon:                       return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Hybrid_Weapon:                       return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Projectile_Weapon:                   return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Drones:                              return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Launcher:                            return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Electronics:                         return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Energy_Grid:                         return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Astronautic:                         return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Electronics_Superiority:             return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Mining:                              return new RigModule(item, ship);
            case EVEDB::invGroups::Rig_Security_Transponder:                return new RigModule(item, ship);


            /************************************/
            /*        SubSystem Modules         */
            /************************************/
            case EVEDB::invGroups::Defensive_Systems:                       return NULL;
            case EVEDB::invGroups::Electronic_Systems:                      return NULL;
            case EVEDB::invGroups::Offensive_Systems:                       return NULL;
            case EVEDB::invGroups::Propulsion_Systems:                      return NULL;
            case EVEDB::invGroups::Engineering_Systems:                     return NULL;


            default:
                break;
        }
    }

    return NULL;
}

#endif /* __MODULEFACTORY_H__ */
