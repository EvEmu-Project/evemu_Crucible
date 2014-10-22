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
    Author:        Luck
*/


#ifndef __MODULEFACTORY_H__
#define __MODULEFACTORY_H__

#include "ActiveModules.h"
#include "PassiveModules.h"
#include "RigModule.h"
#include "SubSystemModules.h"
#include "ship/modules/armor_modules/ArmorRepairer.h"
#include "ship/modules/electronics_modules/TractorBeam.h"
#include "ship/modules/mining_modules/MiningLaser.h"
#include "ship/modules/propulsion_modules/Afterburner.h"
#include "ship/modules/weapon_modules/EnergyTurret.h"
#include "ship/modules/weapon_modules/HybridTurret.h"
#include "ship/modules/weapon_modules/ProjectileTurret.h"
#include "ship/modules/weapon_modules/MissileLauncher.h"
#include "ship/modules/weapon_modules/SuperWeapon.h"

//how you should access the modules
static GenericModule* ModuleFactory(InventoryItemRef item, ShipRef ship)
{

    if( (item->categoryID() != EVEDB::invCategories::Module) && (item->categoryID() != EVEDB::invCategories::Subsystem) )
    {
        sLog.Error("ModuleFactory","Cannot create module from non module type item");
        assert(false);
    }
    else
    {
        switch(item->groupID())
        {
            // Armor Modules Subgroup:
            case EVEDB::invGroups::Damage_Control:                          return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Armor_Repair_Unit:                       return (new ArmorRepairer(item, ship)); break;	 // Active  -  SELF
            case EVEDB::invGroups::Hull_Repair_Unit:                        return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Reinforced_Bulkheads:                    return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Armor_Coating:                           return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Armor_Repair_Projector:                  return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Armor_Plating_Energized:                 return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Armor_Hardener:                          return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Armor_Reinforcer:                        return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Remote_Hull_Repairer:                    return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Expanded_Cargohold:                      return (new PassiveModule(item, ship)); break;

            // Electronics Modules Subgroup:
            case EVEDB::invGroups::Cargo_Scanner:                           return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Ship_Scanner:                            return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Survey_Scanner:                          return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Cloaking_Device:                         return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Target_Painter:                          return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Drone_Control_Unit:                      return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::System_Scanner:                          return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Scan_Probe_Launcher:                     return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Drone_Navigation_Computer:               return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Drone_Tracking_Modules:                  return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Drone_Control_Range_Module:              return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Tractor_Beam:                            return (new TractorBeam(item, ship)); break;    // Active - external

            // Engineering Modules Subgroup:
            case EVEDB::invGroups::Capacitor_Recharger:                     return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Capacitor_Battery:                       return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Energy_Transfer_Array:                   return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Capacitor_Booster:                       return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Auxiliary_Power_Core:                    return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Power_Diagnostic_System:                 return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Capacitor_Power_Relay:                   return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Capacitor_Flux_Coil:                     return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Reactor_Control_Unit:                    return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Shield_Flux_Coil:                        return (new PassiveModule(item, ship)); break;

            // EWAR Modules Subgroup:
            case EVEDB::invGroups::Warp_Scrambler:                          return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Stasis_Web:                              return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::ECM_Burst:                               return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Passive_Targeting_System:                return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Automated_Targeting_System:              return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::ECM:                                     return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::ECCM:                                    return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Sensor_Backup_Array:                     return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Remote_Sensor_Damper:                    return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Tracking_Link:                           return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Signal_Amplifier:                        return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Tracking_Enhancer:                       return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Sensor_Booster:                          return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Tracking_Computer:                       return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::CPU_Enhancer:                            return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Projected_ECCM:                          return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Remote_Sensor_Booster:                   return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Tracking_Disruptor:                      return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::ECM_Stabilizer:                          return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Remote_ECM_Burst:                        return (new ActiveModule(item, ship)); break;    // Active - external

            // Gang Assist Modules Subgroup:
            case EVEDB::invGroups::Gang_Coordinator:                        return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Siege_Module:                            return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Data_Miners:                             return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Jump_Portal_Generator:                   return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Cynosural_Field:                         return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Clone_Vat_Bay:                           return (new PassiveModule(item, ship)); break;

            // Mining Modules Subgroup:
            case EVEDB::invGroups::Mining_Laser:                            return (new MiningLaser(item, ship)); break;     // Active - external
            case EVEDB::invGroups::Strip_Miner:                             return (new MiningLaser(item, ship)); break;     // Active - external
            case EVEDB::invGroups::Frequency_Mining_Laser:                  return (new MiningLaser(item, ship)); break;     // Active - external
            case EVEDB::invGroups::Mining_Upgrade:                          return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Gas_Cloud_Harvester:                     return (new MiningLaser(item, ship)); break;     // Active - external

            // Propulsion Modules Subgroup:
            case EVEDB::invGroups::Afterburner:                             return (new Afterburner(item, ship)); break;
            case EVEDB::invGroups::Warp_Core_Stabilizer:                    return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Inertial_Stabilizer:                     return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Nanofiber_Internal_Structure:            return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Overdrive_Injector_System:               return (new PassiveModule(item, ship)); break;

            // Shield Modules Subgroup:
            case EVEDB::invGroups::Shield_Extender:                         return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Shield_Recharger:                        return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Shield_Booster:                          return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Shield_Transporter:                      return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Shield_Power_Relay:                      return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Shield_Hardener:                         return (new ActiveModule(item, ship)); break;    // Active  -  SELF
            case EVEDB::invGroups::Shield_Amplifier:                        return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Shield_Boost_Amplifier:                  return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Shield_Disruptor:                        return (new ActiveModule(item, ship)); break;    // Active - external

            // Weapon Modules Subgroup:
            case EVEDB::invGroups::Energy_Weapon:                           return (new EnergyTurret(item, ship)); break;        // Active - external
            case EVEDB::invGroups::Projectile_Weapon:                       return (new ProjectileTurret(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Gyrostabilizer:                          return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Energy_Vampire:                          return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Energy_Destabilizer:                     return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Smart_Bomb:                              return (new ActiveModule(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Hybrid_Weapon:                           return (new HybridTurret(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Heat_Sink:                               return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Magnetic_Field_Stabilizer:               return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Ballistic_Control_system:                return (new PassiveModule(item, ship)); break;
            case EVEDB::invGroups::Missile_Launcher_Snowball:               return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Cruise:                 return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Rocket:                 return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Siege:                  return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Standard:               return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Heavy:                  return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Assault:                return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Defender:               return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Citadel:                return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Super_Weapon:                            return (new SuperWeapon(item, ship)); break;       // Active - external
            case EVEDB::invGroups::Interdiction_Sphere_Launcher:            return (new ActiveModule(item, ship)); break;       // Active - external
            case EVEDB::invGroups::Missile_Launcher_Heavy_Assault:          return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Bomb:                   return (new MissileLauncher(item, ship)); break;    // Active - external
            case EVEDB::invGroups::Warp_Disrupt_Field_Generator:            return (new ActiveModule(item, ship)); break;       // Active - external


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

            case EVEDB::invGroups::Rig_Armor:                               return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Shield:                              return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Energy_Weapon:                       return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Hybrid_Weapon:                       return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Projectile_Weapon:                   return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Drones:                              return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Launcher:                            return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Electronics:                         return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Energy_Grid:                         return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Astronautic:                         return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Electronics_Superiority:             return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Mining:                              return (new RigModule(item, ship)); break;
            case EVEDB::invGroups::Rig_Security_Transponder:                return (new RigModule(item, ship)); break;


            /************************************/
            /*        SubSystem Modules         */
            /************************************/
            case EVEDB::invGroups::Defensive_Systems:                       return (new SubSystemModule(item, ship)); break;
            case EVEDB::invGroups::Electronic_Systems:                      return (new SubSystemModule(item, ship)); break;
            case EVEDB::invGroups::Offensive_Systems:                       return (new SubSystemModule(item, ship)); break;
            case EVEDB::invGroups::Propulsion_Systems:                      return (new SubSystemModule(item, ship)); break;
            case EVEDB::invGroups::Engineering_Systems:                     return (new SubSystemModule(item, ship)); break;


            default:
                break;
        }
    }

    // THIS IS A MAJOR PROBLEM IF CODE EXECUTION MAKES IT TO THIS POINT,
    // IT WILL CRASH THE SERVER IF NULL IS RETURNED:
    sLog.Error( "ModuleFactory::ModuleFactory()", "CRITICAL!  NO Module Group was found for item '%s' (itemID = %u)", item->itemName().c_str(), item->itemID() );
    return NULL;
}

#endif /* __MODULEFACTORY_H__ */
