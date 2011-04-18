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
		Deactivating,
		Deactive
	} State;

	State m_state;				//state of the module

	Client *const m_pilot;		//owner of the ship the module is fitted to

	InventoryItemRef m_item;	//the module itself
	InventoryItemRef m_charge;	//charge in the module
	InventoryItemRef m_targetItem;

	Timer m_activationTimer;
	Timer m_effectTimer;

	uint32 m_repeatCount;
	uint32 m_target;

	SystemEntity *target;
	
	//ModuleEffect *m_effect;

	std::string m_effectName;

	uint32 m_effectInterval;
	uint32 m_activationInterval;

	//static factory method
	static ShipModule *CreateModule(Client *owner, InventoryItemRef self, InventoryItemRef charge); 
	static ShipModule *CreateRig(Client *owner, InventoryItemRef self);
	
	//ShipModule building blocks
	InventoryItemRef item() const { return(m_item); }
	InventoryItemRef charge() const { return(m_charge); }

	//ShipModule Processors
	virtual void Process();												//functional ship module process loop
	virtual void Deactivate(const std::string &effectName = "online" );
	virtual int  Activate(const std::string &effectName = "online", uint32 target = 0, uint32 repeat = 0);
	virtual int	 Upgrade();//adding rigs
	virtual void Downgrade();//removing rigs

	//ShipModule CoProcessors
	virtual bool ValidateOnline();
	virtual bool ValidateOffline();
	virtual bool ValidateActive();
	virtual bool ValidateDeactive();
	virtual bool ValidateEffect(bool active);
	virtual void DoEffect(bool active);
	
	//Effects Functions
	virtual void DoPassiveEffects(bool add, bool notify);
	virtual void DoActiveModulePassiveEffects(bool add, bool notify);
	virtual void DoUpgradeLoad(bool add, bool notify);
	virtual void DoCapacitorNeed(bool startup, bool notify);

 /*	virtual void DoArmorHPBonus(bool add, bool notify) = 0;
	virtual void DoArmorHPBonusAdd(bool add, bool notify) = 0;
	virtual void DoArmorHPMultiplier(bool add, bool notify) = 0;
	virtual void DoArmorEmDamageResistanceBonus(bool add, bool notify) = 0;
	virtual void DoArmorExplosiveDamageResistanceBonus(bool add, bool notify) = 0;
	virtual void DoArmorKineticDamageResistanceBonus(bool add, bool notify) = 0;
	virtual void DoArmorThermalDamageResistanceBonu(bool add, bool notify) = 0;

    virtual void DoShieldCapacity(bool add, bool notify) = 0;
	virtual void DoShieldCapacityBonus(bool add, bool notify) = 0;
	virtual void DoShieldCapacityMultiplier(bool add, bool notify);
	virtual void DoShieldEmDamageResistanceBonus(bool add, bool notify);
	virtual void DoShieldExplosiveDamageResistanceBonus(bool add, bool notify);
	virtual void DoShieldKineticDamageResistanceBonus(bool add, bool notify);
	virtual void DoShieldThermalDamageResistanceBonu(bool add, bool notify);
	virtual void DoShieldRechargeRateMultiplier(bool add, bool notify);

    virtual void DoHullHpBonus(bool add, bool notify);

    virtual void DoStructureHPMultiplier(bool add, bool notify);
	virtual void DoStructureEmDamageResistanceBonus(bool add, bool notify);
	virtual void DoStructureExplosiveDamageResistanceBonus(bool add, bool notify);
	virtual void DoStructureKineticDamageResistanceBonus(bool add, bool notify);
	virtual void DoStructureThermalDamageResistanceBonu(bool add, bool notify);                                     */

    virtual void DoImplantBonusVelocity(bool add, bool notify);

    virtual void DoSpeedBonus(bool add, bool notify);
	virtual void DoMaxVelocityBonus(bool add, bool notify);

 /*	virtual void DoCpuMultiplier(bool add, bool notify);

    virtual void DoPowerIncrease(bool add, bool notify);
	virtual void DoPowerOutputBonus(bool add, bool notify);
	virtual void DoPowerOutputMultiplier(bool add, bool notify);

    virtual void DoCapacitorBonus(bool add, bool notify);
	virtual void DoCapacitorCapacityMultiplier(bool add, bool notify);
	virtual void DoCapacitorRechargeRateMultiplier(bool add, bool notify);

    virtual void DoCargoCapacityMultiplier(bool add, bool notify);

    virtual void DoScanStrengthBonus(bool add, bool notify);
	virtual void DoScanGravimetricStrengthPercent(bool add, bool notify);
	virtual void DoScanLadarStrengthPercent(bool add, bool notify);
	virtual void DoScanMagnetometricStrengthPercent(bool add, bool notify);
	virtual void DoScanRadarStrengthPercent(bool add, bool notify);

    virtual void DoSignatureRadiusBonus(bool add, bool notify);
	virtual void DoDroneCapacityBonus(bool add, bool notify);
	virtual void DoAgilityMultiplier(bool add, bool notify);
	virtual void DoScanResultionBonus(bool add, bool notify);
	virtual void DoScanResultionMultiplier(bool add, bool notify);
	virtual void DoMaxTargetRangeBonus(bool add, bool notify);
	virtual void DoMaxLockedTargetsBonus(bool add, bool notify);
	virtual void DoWarpScrambleStrength(bool add, bool notify);

    virtual void DoPropulsionFusionStrength(bool add, bool notify);
	virtual void DoPropulsionIonStrength(bool add, bool notify);
	virtual void DoPropulsionMagpulseStrength(bool add, bool notify);
	virtual void DoPropulsionPlasmaStrength(bool add, bool notify);

    virtual void DoPassiveArmorEmDamageResistanceBonus(bool add, bool notify);
	virtual void DoPassiveArmorExplosiveDamageResistanceBonus(bool add, bool notify);
	virtual void DoPassiveArmorKineticDamageResistanceBonus(bool add, bool notify);
	virtual void DoPassiveArmorThermalDamageResistanceBonu(bool add, bool notify);
	virtual void DoPassiveShieldEmDamageResistanceBonus(bool add, bool notify);
	virtual void DoPassiveShieldExplosiveDamageResistanceBonus(bool add, bool notify);
	virtual void DoPassiveShieldKineticDamageResistanceBonus(bool add, bool notify);
	virtual void DoPassiveShieldThermalDamageResistanceBonus(bool add, bool notify);
*/

    //Helper Functions
	virtual void ChangeMState(State new_state);							//changes the m_state variable to the appropriate value
	virtual bool Equals(double a, int b, double percision = 0.0001);	//used to determine if floats are equal to ints within a certain percision (less than the eve client uses)
	virtual int	 ToInt(double a);										//used to return an integer to the client.  Rounds down
	virtual bool TypeCast(double a);									//used to determine if the value is an integer or a float
	virtual int GetState() { return m_state;}
	virtual void GetActivationInterval();
	virtual bool AffectsArmor();
	virtual bool AffectsShield();
	virtual bool AffectsHull();

	//Godma Handlers
	virtual void DoGodmaEffects(bool active);

	//Graphics Handlers
	virtual void DoSpecialFX(bool startup);
	virtual int SFXEffectInterval();
		
	void ChangeCharge(InventoryItemRef new_charge);

protected:
	//Constructor
	ShipModule(Client *owner, InventoryItemRef self, InventoryItemRef charge);
	ShipModule(Client *owner, InventoryItemRef self);

public:
	//Deconstructor
	virtual ~ShipModule();
};

class ModuleEffect {
public:
	ModuleEffect();
	~ModuleEffect();

	uint32			effectID;
	std::string		effectName;
	uint32			effectCategory;
	uint32			preExpression;
	uint32			postExpression;
	std::string		description;
	std::string		guid;
	uint32			graphicID;
	uint32			isOffensive;
	uint32			isAssistance;
	uint32			durationAttributeID;
	uint32			trackingSpeedAttributeID;
	uint32			dischargeAttributeID;
	uint32			rangeAttributeID;
	uint32			falloffAttributeID;
	uint32			disallowAutoRepeat;
	uint32			published;
	uint32			isWarpSafe;
	uint32			rangeChance;
	uint32			electronicChance;
	uint32			propulsionChance;
	uint32			distribution;
	std::string		sfxName;
};

class ModuleManager {
public:
	ModuleManager(Client *pilot);
	~ModuleManager();
	
	void Process();
	void UpdateModules();
	
	int Activate(uint32 itemID, const std::string &effectName, uint32 target, uint32 repeat);
	int Upgrade(uint32 itemID);
	void Downgrade(uint32 itemID);
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
	std::map<uint32, uint8> m_rigByID;
	ShipModule *m_modules[MAX_MODULE_COUNT];
	ShipModule *m_rigs[MAX_RIG_COUNT];
};

#endif


