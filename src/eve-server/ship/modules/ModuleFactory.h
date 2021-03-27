/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Updates:    Allan   13 March 2017    Implemented TRUE effects class and code that processess and applies all item effect using ccp's effects data
*/


#ifndef __MODULEFACTORY_H__
#define __MODULEFACTORY_H__

#include "ship/modules/ActiveModule.h"
#include "ship/modules/MiningLaser.h"
#include "ship/modules/PassiveModule.h"
#include "ship/modules/RigModule.h"
#include "ship/modules/Prospector.h"
#include "ship/modules/SubSystemModule.h"
#include "ship/modules/ModuleItem.h"
#include "ship/modules/TurretModule.h"


static GenericModule* ModuleFactory(ModuleItemRef mRef, ShipItemRef sRef)
{
    if ((mRef->categoryID() != EVEDB::invCategories::Module) and (mRef->categoryID() != EVEDB::invCategories::Subsystem)) {
        sLog.Error("ModuleFactory","Cannot create module from non module type item");
        assert(false);
    } else {
        switch(mRef->groupID()) {
            // Weapon Modules:
            case EVEDB::invGroups::Energy_Weapon:                           return (new TurretModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Projectile_Weapon:                       return (new TurretModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Gyrostabilizer:                          return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Energy_Vampire:                          return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Energy_Destabilizer:                     return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Smart_Bomb:                              return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Hybrid_Weapon:                           return (new TurretModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Heat_Sink:                               return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Magnetic_Field_Stabilizer:               return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Ballistic_Control_system:                return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Super_Weapon:                            return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Interdiction_Sphere_Launcher:            return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Warp_Disrupt_Field_Generator:            return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Snowball:               return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Cruise:                 return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Rocket:                 return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Siege:                  return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Standard:               return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Heavy:                  return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Assault:                return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Defender:               return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Citadel:                return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Heavy_Assault:          return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Missile_Launcher_Bomb:                   return (new ActiveModule(mRef, sRef));    // Active - external

            // Armor Modules:
            case EVEDB::invGroups::Damage_Control:                          return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Armor_Repair_Unit:                       return (new ActiveModule(mRef, sRef));	  // Active  -  SELF
            case EVEDB::invGroups::Hull_Repair_Unit:                        return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Reinforced_Bulkheads:                    return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Armor_Coating:                           return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Armor_Repair_Projector:                  return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Armor_Plating_Energized:                 return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Armor_Hardener:                          return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Armor_Reinforcer:                        return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Remote_Hull_Repairer:                    return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Expanded_Cargohold:                      return (new PassiveModule(mRef, sRef));

            // Shield Modules:
            case EVEDB::invGroups::Shield_Extender:                         return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Shield_Recharger:                        return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Shield_Booster:                          return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Shield_Transporter:                      return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Shield_Power_Relay:                      return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Shield_Hardener:                         return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Shield_Amplifier:                        return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Shield_Boost_Amplifier:                  return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Shield_Disruptor:                        return (new ActiveModule(mRef, sRef));    // Active - external

            // Scanner Modules
            case EVEDB::invGroups::Cargo_Scanner:                           return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Ship_Scanner:                            return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Survey_Scanner:                          return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::System_Scanner:                          return (new ActiveModule(mRef, sRef));    // Active - external

            // Electronics Modules:
            case EVEDB::invGroups::Cloaking_Device:                         return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Target_Painter:                          return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Drone_Control_Unit:                      return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Scan_Probe_Launcher:                     return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Drone_Navigation_Computer:               return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Drone_Tracking_Modules:                  return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Drone_Control_Range_Module:              return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Tractor_Beam:                            return (new ActiveModule(mRef, sRef));    // Active - external

            // Prospecting Modules
            case EVEDB::invGroups::Salvager:                                return (new Prospector(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Data_Miner:                              return (new Prospector(mRef, sRef));    // Active - external

            // Engineering Modules:
            case EVEDB::invGroups::Capacitor_Recharger:                     return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Capacitor_Battery:                       return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Energy_Transfer_Array:                   return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Capacitor_Booster:                       return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Auxiliary_Power_Core:                    return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Power_Diagnostic_System:                 return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Capacitor_Power_Relay:                   return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Capacitor_Flux_Coil:                     return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Reactor_Control_Unit:                    return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Shield_Flux_Coil:                        return (new PassiveModule(mRef, sRef));

            // EWAR Modules:
            case EVEDB::invGroups::Warp_Scrambler:                          return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Stasis_Web:                              return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::ECM_Burst:                               return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Passive_Targeting_System:                return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Automated_Targeting_System:              return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::ECM:                                     return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::ECCM:                                    return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Sensor_Backup_Array:                     return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Remote_Sensor_Damper:                    return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Tracking_Link:                           return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Signal_Amplifier:                        return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Tracking_Enhancer:                       return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Sensor_Booster:                          return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::Tracking_Computer:                       return (new ActiveModule(mRef, sRef));    // Active  -  SELF
            case EVEDB::invGroups::CPU_Enhancer:                            return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Projected_ECCM:                          return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Remote_Sensor_Booster:                   return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Tracking_Disruptor:                      return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::ECM_Stabilizer:                          return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Remote_ECM_Burst:                        return (new ActiveModule(mRef, sRef));    // Active - external

            // Gang Assist Modules:
            case EVEDB::invGroups::Gang_Coordinator:                        return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Siege_Module:                            return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Jump_Portal_Generator:                   return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Cynosural_Field:                         return (new ActiveModule(mRef, sRef));    // Active - external
            case EVEDB::invGroups::Clone_Vat_Bay:                           return (new PassiveModule(mRef, sRef));

            // Mining Modules:
            case EVEDB::invGroups::Mining_Laser:                            return (new MiningLaser(mRef, sRef));     // Active - external
            case EVEDB::invGroups::Strip_Miner:                             return (new MiningLaser(mRef, sRef));     // Active - external
            case EVEDB::invGroups::Frequency_Mining_Laser:                  return (new MiningLaser(mRef, sRef));     // Active - external
            case EVEDB::invGroups::Mining_Upgrade:                          return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Gas_Cloud_Harvester:                     return (new MiningLaser(mRef, sRef));     // Active - external

            // Propulsion Modules:
            case EVEDB::invGroups::Afterburner:                             return (new ActiveModule(mRef, sRef));     // Active
            case EVEDB::invGroups::Microwarpdrive:                          return (new ActiveModule(mRef, sRef));     // Active
            case EVEDB::invGroups::Warp_Core_Stabilizer:                    return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Inertial_Stabilizer:                     return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Nanofiber_Internal_Structure:            return (new PassiveModule(mRef, sRef));
            case EVEDB::invGroups::Overdrive_Injector_System:               return (new PassiveModule(mRef, sRef));

            // Rigs
            case EVEDB::invGroups::Rig_Armor:                               return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Shield:                              return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Energy_Weapon:                       return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Hybrid_Weapon:                       return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Projectile_Weapon:                   return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Drones:                              return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Launcher:                            return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Electronics:                         return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Energy_Grid:                         return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Astronautic:                         return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Electronics_Superiority:             return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Mining:                              return (new RigModule(mRef, sRef));
            case EVEDB::invGroups::Rig_Security_Transponder:                return (new RigModule(mRef, sRef));

            // SubSystems
            case EVEDB::invGroups::Defensive_Systems:                       return (new SubSystemModule(mRef, sRef));
            case EVEDB::invGroups::Electronic_Systems:                      return (new SubSystemModule(mRef, sRef));
            case EVEDB::invGroups::Offensive_Systems:                       return (new SubSystemModule(mRef, sRef));
            case EVEDB::invGroups::Propulsion_Systems:                      return (new SubSystemModule(mRef, sRef));
            case EVEDB::invGroups::Engineering_Systems:                     return (new SubSystemModule(mRef, sRef));

            // may need specific code for these gm modules
            //case EVEDB::invGroups::GM_Modules:
            case EVEDB::invGroups::Cheat_Module_Group:                      return (new ActiveModule(mRef, sRef));

            // Uncategorized and Unknown Modules Groups (some of these groups contain NO REAL typeIDs in the 'invTypes' table:
            /**  @note  let these make an error to show they are used
            case EVEDB::invGroups::Computer_Interface_Node:
            case EVEDB::invGroups::Cruise_Control:
            case EVEDB::invGroups::Smartbomb_Supercharger:
            case EVEDB::invGroups::Anti_Ballistic_Defense_System:
            case EVEDB::invGroups::New_EW_Testing:
            case EVEDB::invGroups::Missile_Launcher:
            case EVEDB::invGroups::Countermeasure_Launcher:
            case EVEDB::invGroups::Anti_Cloaking_Pulse:
            case EVEDB::invGroups::Signature_Scrambling:
            case EVEDB::invGroups::Energy_Vampire_Slayer:
            case EVEDB::invGroups::Autopilot:
            case EVEDB::invGroups::DroneBayExpander:
            case EVEDB::invGroups::Drone_Modules:
            case EVEDB::invGroups::Navigation_Computer:
            case EVEDB::invGroups::Super_Gang_Enhancer:
            case EVEDB::invGroups::Drone_Damage_Modules:
            case EVEDB::invGroups::ECM_Enhancer:
            case EVEDB::invGroups::Cloak_Enhancements:
            case EVEDB::invGroups::Mining_Enhancer:
            case EVEDB::invGroups::Covert_Cynosural_Field_Generator: {
                return nullptr;
            }
            */
        }
    }

    _log(MODULE__ERROR, "ModuleFactory - Module Group not found for %s - groupID: %u, typeID: %u", mRef->name(), mRef->groupID(), mRef->typeID());
    return nullptr;
}

#endif /* __MODULEFACTORY_H__ */
