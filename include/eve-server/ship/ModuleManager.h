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
	Author:		ItWasLuck
*/

#ifndef __MODULEMANAGER_H_INCL__
#define __MODULEMANAGER_H_INCL__

class InventoryItem;
class SystemEntity;
class Client;


class ShipModule {
public:

	//Declarations
		typedef enum {
		Offline, 
		Online,
		Active,
		Overloaded,
		Deactive
	} State;

	typedef enum {
		AlreadyFitted,			//module is has already been validated for fit. This is only true of modules that were online when you logged off
		NewModule				//module that has just been created or just turned online
	} Fit;

	typedef enum {
		ActiveModule,					//modules that only have active effecs
		PassiveModule,				//modules that only have passive effects
		Both					//some modules have both, i think?
	} APGroup;

	typedef enum {
		Turret,
		Launcher,
		Miner,
		StripMiner
	} Type;

	State m_state;				//state of the module
	Fit m_fit;					//whether or not it was already on the ship
	Type m_type;				//whether it's a missle launcher, turret, or whatever
	APGroup m_apGroup;			//active, passive, or both
	
	Client *const m_pilot;		//owner of the ship the module is fitted to

	InventoryItemRef m_item;	//the module itself
	InventoryItemRef m_charge;	//charge in the module
	InventoryItemRef m_targetItem;

	Timer m_activationTimer;
	Timer m_effectTimer;

	uint32 m_repeatCount;
	uint32 m_target;
	bool m_active;

	SystemEntity *target;

	std::string m_effectName;

	uint32 m_effectInterval;
	uint32 m_interval;

	EVEEffectID m_effectID;

	//static factory method
	static ShipModule *CreateModule(Client *owner, InventoryItemRef self, InventoryItemRef charge); 
	
	//ShipModule building blocks
	InventoryItemRef item() const { return(m_item); }
	InventoryItemRef charge() const { return(m_charge); }

	//ShipModule Processors
	virtual void Process();												//functional ship module process loop
	virtual void Deactivate(const std::string &effectName);
	virtual int  Activate(const std::string &effectName = "online", uint32 target = 0, uint32 repeat = 0);

	//ShipModule CoProcessors
	virtual bool Validate_Online();
	virtual void Handle_Online();
	virtual bool Validate_Offline();
	virtual void Handle_Offline();
	virtual bool Validate_Active();
	virtual void Handle_Active(bool start);
	virtual void DoPassiveEffects(bool online);
	virtual void DoActiveEffects(bool online);

	//Effects Functions
	virtual void ActiveShipVelocityEffects(bool online);
	virtual void ArmorHPEffects(bool online);
	virtual void ArmorResistanceEffects(bool online);
	virtual void ShieldHPEffects(bool online);
	virtual void ShieldResistanceEffects(bool online);
	virtual void ShieldRechargeEffects(bool online);
	virtual void HullHPEffects(bool online);
	virtual void HullResistanceEffects(bool online);
	virtual void ShipVelocityEffects(bool online);
	virtual void ShipCpuEffects(bool online);
	virtual void ShipPowerEffects(bool online);
	virtual void ShipCapacitorChargeEffects(bool online);
	virtual void ShipCapacitorRechargeRateEffects(bool online);
	virtual void ShipCargoCapacityEffects(bool online);
	virtual void ShipScanStrengthEffects(bool online);
	virtual void ShipSignatureRadiusEffects(bool online);
	virtual void ShipDroneCapacityEffects(bool online);
	virtual void ShipAgilityEffects(bool online);
	virtual void ShipScanResolutionEffects(bool online);
	virtual void ShipMaxTargetsEffects(bool online);
	virtual void ShipMaxTargetRangeEffects(bool online);
	virtual void ShipWarpScrambleStrengthEffects(bool online);
	virtual void ShipPropulsionStrengthEffects(bool online);
	virtual void ActiveModulePassiveShieldResists(bool online);
	virtual void ActiveModulePassiveArmorResists(bool online);

	//Helper Functions
	virtual bool IsTurret();											//checks if the new module is a turret
	virtual bool IsLauncher();											//checks if the new module is a launcher
	virtual bool IsMiner();												//checks if the new module is a miner
	virtual bool IsStripMiner();										//checks if the new module is a strip miner
	virtual bool IsShieldExtender();									//checks if the new module is a shield extender
	virtual bool IsAutomatedTargetingSystem();							//checks if the new module is an automated targeting system
	virtual bool AffectsArmor();										//checks if the new module affects armor
	virtual bool AffectsShield();										//checks if the new module affects shield
	virtual bool AffectsHull();											//checks if the new module affects hull
	virtual bool AffectsDrones();										//checks if the new module affects your drones
	virtual bool IsInertialStabilizer();								//checks if the new modules is an inertial stabilizer
	virtual void ChangeMState(State new_state);							//changes the m_state variable to the appropriate value
	virtual bool Equals(double a, int b, double percision = 0.0001);	//used to determine if floats are equal to ints within a certain percision (less than the eve client uses)
	virtual int	 ToInt(double a);										//used to return an integer to the client.  Rounds down
	virtual int GetState() { return m_state;}
	
	void ChangeCharge(InventoryItemRef new_charge);


protected:
	//Constructor
	ShipModule(Client *owner, InventoryItemRef self, InventoryItemRef charge);

public:
	//Deconstructor
	virtual ~ShipModule();
};

class ModuleManager {
public:
	ModuleManager(Client *pilot);
	~ModuleManager();
	
	void Process();
	void UpdateModules();
	
	int Activate(uint32 itemID, const std::string &effectName, uint32 target, uint32 repeat);
	void Deactivate(uint32 itemID, const std::string &effectName);
	void ReplaceCharges(EVEItemFlags flag, InventoryItemRef charge);
 	void DeactivateAllModules();
	
	void OnlineAll();
	void UnloadModule(uint32 itemID);
	void UnloadAllModules();
	void RepairModules();


protected:
	Client *const m_pilot;
	
	std::map<uint32, uint8> m_moduleByID;	//maps itemID to m_modules index
	ShipModule *m_modules[MAX_MODULE_COUNT];
};

#endif


