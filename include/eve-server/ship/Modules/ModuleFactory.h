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
			/************************************/
			/*      Active Modules              */
			/************************************/

		case EVEDB::invGroups::Afterburner:							return new Afterburner(item, ship); break;
		case EVEDB::invGroups::Cargo_Scanner:						
		case EVEDB::invGroups::Ship_Scanner:
		case EVEDB::invGroups::Survey_Scanner:
		case EVEDB::invGroups::Warp_Scrambler:
		case EVEDB::invGroups::Energy_Weapon:
		case EVEDB::invGroups::Mining_Laser:
		case EVEDB::invGroups::Projectile_Weapon:
		case EVEDB::invGroups::Missile_Launcher:
		case EVEDB::invGroups::Armor_Repair_Unit:
		case EVEDB::invGroups::Hull_Repair_Unit:
		case EVEDB::invGroups::Stasis_Web:
		case EVEDB::invGroups::Energy_Transfer_Array:
		case EVEDB::invGroups::Energy_Vampire:
		case EVEDB::invGroups::Energy_Destabilizer:
		case EVEDB::invGroups::Smart_Bomb:
		case EVEDB::invGroups::Hybrid_Weapon:
		case EVEDB::invGroups::Shield_Booster:
		case EVEDB::invGroups::Capacitor_Booster:
		case EVEDB::invGroups::ECM_Burst:
		case EVEDB::invGroups::Remote_Sensor_Booster:
		case EVEDB::invGroups::Sensor_Booster:
		case EVEDB::invGroups::Missile_Launcher_Snowball:
		case EVEDB::invGroups::Missile_Launcher_Cruise:
		case EVEDB::invGroups::Missile_Launcher_Rocket:
		case EVEDB::invGroups::Missile_Launcher_Siege:
		case EVEDB::invGroups::Missile_Launcher_Standard:
		case EVEDB::invGroups::Missile_Launcher_Heavy:
		case EVEDB::invGroups::Missile_Launcher_Assault:
		case EVEDB::invGroups::Missile_Launcher_Defender:
		case EVEDB::invGroups::Countermeasure_Launcher:
		case EVEDB::invGroups::Shield_Disruptor:
		case EVEDB::invGroups::Cloaking_Device:
		case EVEDB::invGroups::Armor_Hardener:
		case EVEDB::invGroups::Armor_Repair_Projector:
		case EVEDB::invGroups::Anti_Cloaking_Pulse:
		case EVEDB::invGroups::Signature_Scrambling:
		case EVEDB::invGroups::Target_Painter:
		case EVEDB::invGroups::Strip_Miner:
		case EVEDB::invGroups::Frequency_Mining_Laser:
		case EVEDB::invGroups::Siege_Module:
		case EVEDB::invGroups::Data_Miners:
		case EVEDB::invGroups::Remote_Hull_Repairer:
		case EVEDB::invGroups::Super_Weapon:
		case EVEDB::invGroups::Interdiction_Sphere_Launcher:
		case EVEDB::invGroups::Jump_Portal_Generator:
		case EVEDB::invGroups::Tractor_Beam:
		case EVEDB::invGroups::Cynosural_Field:
		case EVEDB::invGroups::Energy_Vampire_Slayer:
		case EVEDB::invGroups::Gas_Cloud_Harvester:
		case EVEDB::invGroups::Remote_ECM_Burst:
		case EVEDB::invGroups::Missile_Launcher_Bomb:
		case EVEDB::invGroups::Warp_Disrupt_Field_Generator:
		case EVEDB::invGroups::Covert_Cynosural_Field_Generator:


			/************************************/
			/*      Passive Modules             */
			/************************************/

		case EVEDB::invGroups::Shield_Extender:
		case EVEDB::invGroups::Shield_Recharger:
		case EVEDB::invGroups::Shield_Transporter:
		case EVEDB::invGroups::Capacitor_Recharger:
		case EVEDB::invGroups::Shield_Power_Relay:
		case EVEDB::invGroups::Gyrostabilizer:
		case EVEDB::invGroups::Damage_Control:								
		case EVEDB::invGroups::Capacitor_Battery:
		case EVEDB::invGroups::Shield_Hardener:
		case EVEDB::invGroups::Reinforced_Bulkheads:
		case EVEDB::invGroups::Passive_Targeting_System:
		case EVEDB::invGroups::Automated_Targeting_System:
		case EVEDB::invGroups::Armor_Coating:
		case EVEDB::invGroups::ECM:
		case EVEDB::invGroups::ECCM:
		case EVEDB::invGroups::Sensor_Backup_Array:
		case EVEDB::invGroups::Heat_Sink:
		case EVEDB::invGroups::Tracking_Link:
		case EVEDB::invGroups::Signal_Amplifier:
		case EVEDB::invGroups::Tracking_Enhancer:
		case EVEDB::invGroups::Tracking_Computer:
		case EVEDB::invGroups::Cheat_Module_Group:
		case EVEDB::invGroups::CPU_Enhancer:
		case EVEDB::invGroups::Projected_ECCM:
		case EVEDB::invGroups::Tracking_Disrupter:
		case EVEDB::invGroups::Shield_Amplifier:
		case EVEDB::invGroups::Magnetic_Field_Stabilizer:
		case EVEDB::invGroups::Autopilot:
		case EVEDB::invGroups::Warp_Core_Stabilizer:
		case EVEDB::invGroups::Gang_Coordinator:
		case EVEDB::invGroups::Computer_Interface_Node:
		case EVEDB::invGroups::Armor_Plating_Energized:
		case EVEDB::invGroups::Armor_Reinforcer:  new PassiveModule(item, ship); break;
		case EVEDB::invGroups::Shield_Boost_Amplifier:
		case EVEDB::invGroups::Auxiliary_Power_Core: new PassiveModule(item, ship); break;
		case EVEDB::invGroups::GM_Modules:
		case EVEDB::invGroups::DroneBayExpander:
		case EVEDB::invGroups::Ballistic_Control_system: new PassiveModule(item, ship); break;
		case EVEDB::invGroups::Cruise_Control:		
		case EVEDB::invGroups::Smartbomb_Supercharger:
		case EVEDB::invGroups::Drone_Control_Unit:		
		case EVEDB::invGroups::System_Scanner:
		case EVEDB::invGroups::Microwarpdrive:
		case EVEDB::invGroups::Scan_Probe_Launcher:		
		case EVEDB::invGroups::New_EW_Testing:
		case EVEDB::invGroups::ECM_Stabilizer:		
		case EVEDB::invGroups::Anti_Ballistic_Defense_System:
		case EVEDB::invGroups::Missile_Launcher_Citadel:		
		case EVEDB::invGroups::Mining_Upgrade:		
		case EVEDB::invGroups::Drone_Modules:		
		case EVEDB::invGroups::Navigation_Computer:
		case EVEDB::invGroups::Super_Gang_Enhancer:
		case EVEDB::invGroups::Drone_Navigation_Computer:
		case EVEDB::invGroups::Drone_Damage_Modules:
		case EVEDB::invGroups::Drone_Tracking_Modules:
		case EVEDB::invGroups::Drone_Control_Range_Module:
		case EVEDB::invGroups::ECM_Enhancer:
		case EVEDB::invGroups::Inertial_Stabilizer:
		case EVEDB::invGroups::Nanofiber_Internal_Structure:
		case EVEDB::invGroups::Overdrive_Injector_System:
		case EVEDB::invGroups::Expanded_Cargohold:
		case EVEDB::invGroups::Power_Diagnostic_System:
		case EVEDB::invGroups::Capacitor_Power_Relay:
		case EVEDB::invGroups::Capacitor_Flux_Coil:
		case EVEDB::invGroups::Reactor_Control_Unit:
		case EVEDB::invGroups::Shield_Flux_Coil:
		case EVEDB::invGroups::Clone_Vat_Bay:
		case EVEDB::invGroups::Cloak_Enhancements:	
		case EVEDB::invGroups::Mining_Enhancer:

		/************************************/
		/*			   Rigs                 */
		/************************************/

		case EVEDB::invGroups::Rig_Armor:									return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Shield:									return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Energy_Weapon:							return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Hybrid_Weapon:							return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Projectile_Weapon:						return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Drones:									return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Launcher:								return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Electronics:								return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Energy_Grid:								return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Astronautic:								return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Electronics_Superiority:					return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Mining:									return new RigModule(item, ship);
		case EVEDB::invGroups::Rig_Security_Transponder:					return new RigModule(item, ship);

			break;
		}
	}
}
