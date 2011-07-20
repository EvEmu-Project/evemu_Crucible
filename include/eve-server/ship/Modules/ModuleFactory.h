
#include "ActiveModules.h"
#include "PassiveModules.h"

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
		case EVEDB::invGroups::Damage_Control:								return new DamageControl(item,ship);
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
		case EVEDB::invGroups::Armor_Reinforcer:
		case EVEDB::invGroups::Shield_Boost_Amplifier:
		case EVEDB::invGroups::Auxiliary_Power_Core:
		case EVEDB::invGroups::GM_Modules:
		case EVEDB::invGroups::DroneBayExpander:
		case EVEDB::invGroups::Ballistic_Control_system:
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

		case EVEDB::invGroups::Rig_Armor:
		case EVEDB::invGroups::Rig_Shield:
		case EVEDB::invGroups::Rig_Energy_Weapon:
		case EVEDB::invGroups::Rig_Hybrid_Weapon:
		case EVEDB::invGroups::Rig_Projectile_Weapon:
		case EVEDB::invGroups::Rig_Drones:
		case EVEDB::invGroups::Rig_Launcher:
		case EVEDB::invGroups::Rig_Electronics:
		case EVEDB::invGroups::Rig_Energy_Grid:
		case EVEDB::invGroups::Rig_Astronautic:
		case EVEDB::invGroups::Rig_Electronics_Superiority:
		case EVEDB::invGroups::Rig_Mining:
		case EVEDB::invGroups::Rig_Security_Transponder:

			break;
		}
	}
}
