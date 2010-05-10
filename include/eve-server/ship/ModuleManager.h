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

#ifndef __MODULEMANAGER_H_INCL__
#define __MODULEMANAGER_H_INCL__

class InventoryItem;
class SystemEntity;
class Client;

class ShipModule {
public:
	//factory method:
	static ShipModule *CreateModule(Client *owner, InventoryItemRef self, InventoryItemRef charge);
	
protected:
	ShipModule(Client *owner, InventoryItemRef self, InventoryItemRef charge);
public:
	virtual ~ShipModule();
	
	InventoryItemRef item() const { return(m_item); }
	InventoryItemRef charge() const { return(m_charge); }
	virtual void Process();
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual	bool OnlineModule();
	virtual void OfflineModule();
	
	void ChangeCharge(InventoryItemRef new_charge);

protected:
	virtual uint32 _ActivationInterval() const;	//some items use speed (default), others use duration.. freakin strange..

	typedef enum {
		Offline,
		PuttingOnline,
		Online,
		Active,	//if we are in this state, it is a repeating effect.
		Deactivating
	} State;
	State m_state;
	
	Client *const m_pilot;

	InventoryItemRef m_item;
	InventoryItemRef m_charge;

	Timer m_timer;
};

class PassiveModule : public ShipModule {
public:
	PassiveModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ShipModule(owner, self, charge_) {}
	virtual ~PassiveModule() {}
};

class ActivatableModule : public ShipModule {
public:
	ActivatableModule(const char *effectName, Client *owner, InventoryItemRef self, InventoryItemRef charge_);
	virtual ~ActivatableModule() {}

	//ShipModule interface:
	virtual void Process();
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	
	void DeactivateModule(bool update=true);
	
protected:
	void _SendGodmaShipEffect(EVEEffectID effect, bool active);
	void _SendWeaponEffect(const char *effect, SystemEntity *target);

	const char *_GetEffectName() const { return(m_effectName); }
	
	virtual void StartEffect() = 0;
	virtual void DoEffect() = 0;
	virtual void StopEffect() = 0;
	
	uint32 m_repeatCount;
	uint32 m_target;
	
//private:
	// Transformed in protected to get access in derived classes
protected:
	//its possible that a module offers multiple active effects, but
	//I am not sure.. this will not support it if that is the case...
	const char *const m_effectName;	//must point into static string data.
};

class HybridWeaponModule : public ActivatableModule {
public:
	HybridWeaponModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("projectileFired", owner, self, charge_) {}
	virtual ~HybridWeaponModule() {}
protected:
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
};

class LaserWeaponModule : public ActivatableModule {
public:
	LaserWeaponModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("targetAttack", owner, self, charge_) {}
	virtual ~LaserWeaponModule() {}
protected:
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
};

class MiningLaserModule : public ActivatableModule {
public:
	MiningLaserModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("miningLaser", owner, self, charge_) {}
	virtual ~MiningLaserModule() {}
protected:
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;
};

class GenericShipModule : public PassiveModule {
public:
	GenericShipModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: PassiveModule(owner, self, charge_) {}
	virtual ~GenericShipModule() {}
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


protected:
	Client *const m_pilot;
	
	std::map<uint32, uint8> m_moduleByID;	//maps itemID to m_modules index
	ShipModule *m_modules[MAX_MODULE_COUNT];
};


/**
 * \class MiningStripLaserModule
 *
 * @brief control the module Strip miner.
 *
 * The Strip Miner module used in Mining Barges
 *
 * This miner type is diferent from normal one because its time is
 * greater, for this we need to make sure that the effects.Laser
 * is sent to client several times, maybe 24 to 26 times.
 *
 * @author Stranho
 * @date 06/30/2009.
 */
class MiningStripLaserModule : public ActivatableModule {
public:
	MiningStripLaserModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("miningLaser", owner, self, charge_), m_EffectTimer(0), m_MiningDuration(0) {}
	virtual ~MiningStripLaserModule() {}
protected:
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

	Timer m_EffectTimer;
	int32 m_MiningDuration;		// used when the user cancels the mining before completation
};


/**
 * \class ShieldBoostingModule
 *
 * @brief control the shield boosting module.
 *
 * Gives a boost in the shield charge at cost
 * of capacitor power.
 *
 * @author Stranho
 * @date 06/30/2009.
 */
class ShieldBoostingModule : public ActivatableModule {
public:
	ShieldBoostingModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("shieldBoosting", owner, self, charge_) {}
	virtual ~ShieldBoostingModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};


/**
 * \class ShieldExtenderModule
 *
 * @brief control the shield total strength.
 *
 * Increase the ship shield capacity,
 * giving it more hit points.
 *
 * @author Stranho
 * @date 06/30/2009.
 */
class ShieldExtenderModule : public PassiveModule {
public:
	ShieldExtenderModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: PassiveModule(owner, self, charge_) {}
	virtual ~ShieldExtenderModule() {}

	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
};


/**
 * \class ArmorRepairModule
 *
 * @brief Repair damage to the ships armor.
 *
 * Gives a boost in the shield charge at cost
 * of capacitor power.
 *
 * @author Stranho
 * @date 06/30/2009.
 */
class ArmorRepairModule : public ActivatableModule {
public:
	ArmorRepairModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("armorRepair", owner, self, charge_) {}
	virtual ~ArmorRepairModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};


/**
 * \class ArmorPlatesModule
 *
 * @brief Armor Plates
 *
 * Increases the maximum strength of the Armor.
 *
 * @author Stranho
 * @date 07/02/2009.
 */
class ArmorPlatesModule : public PassiveModule {
public:
	ArmorPlatesModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: PassiveModule(owner, self, charge_) {}
	virtual ~ArmorPlatesModule() {}

	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
};


/**
 * \class PowerDiagnosticsModule
 *
 * @brief Power Diagnostics System
 *
 * Monitors and optimizes the power grid. Gives a boost to 
 * power core output and gives an increase in shield
 * and capacitor recharge rate.
 *
 * @author Stranho
 * @date 07/02/2009.
 */
class PowerDiagnosticsModule : public PassiveModule {
public:
	PowerDiagnosticsModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: PassiveModule(owner, self, charge_) 
	{
		m_ShieldRechargeRateMultiplier = 0.0;
		m_CapacitorRechargeRateMultiplier = 0.0;
		m_ShieldRechargeRate = 0;
		m_RechargeRate = 0;
		m_PowerOutput = 0;
		m_ShieldCapacity = 0;
		m_CapacitorCapacity = 0;
	}
	virtual ~PowerDiagnosticsModule() {}

	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);

protected:
	double m_ShieldRechargeRateMultiplier;
	double m_CapacitorRechargeRateMultiplier;
	uint32 m_ShieldRechargeRate;
	uint32 m_RechargeRate;
	uint32 m_PowerOutput;
	uint32 m_ShieldCapacity;
	uint32 m_CapacitorCapacity;
};


/**
 * \class SmartBombModule
 *
 * @brief Do damage to nearby ships
 *
 * Sends a wave of blast that hit any
 * nearby ships in the range.
 *
 * @author Stranho
 * @date 06/30/2009.
 */
class SmartBombModule : public ActivatableModule {
public:
	SmartBombModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~SmartBombModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

//God this sucks... and yes there is definately a better way to do this, but for now, im writting them out to get them working,
//then we'll see which ones can be combined, or even if this can be incorporated into xmlpacketgen (it probably should be)


class AfterburnerModule : public ActivatableModule {
public:
	AfterburnerModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("speedBoostMassAddition", owner, self, charge_) {}
	virtual ~AfterburnerModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};
/*
class WarpStabilizerModule : public ActivatableModule {
public:
	WarpStabilizerModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~WarpStabilizerModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class ArmorHardenerModule : public ActivatableModule {
public:
	ArmorHardenerModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~ArmorHardenerModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class AutomatedTargetingModule : public ActivatableModule {
public:
	AutomatedTargetingModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~AutomatedTargetingModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class AuxiliaryPowerCoreModule : public ActivatableModule {
public:
	AuxiliaryPowerCoreModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~AuxiliaryPowerCoreModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class BallisticControlModule : public ActivatableModule {
public:
	BallisticControlModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~BallisticControlModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CapacitorBatteryModule : public ActivatableModule {
public:
	CapacitorBatteryModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CapacitorBatteryModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CapacitorBoosterModule : public ActivatableModule {
public:
	CapacitorBoosterModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CapacitorBoosterModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CapacitorRechargerModule : public ActivatableModule {
public:
	CapacitorRechargerModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CapacitorRechargerModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CargoScannerModule : public ActivatableModule {
public:
	CargoScannerModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CargoScannerModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CloakingDeviceModule : public ActivatableModule {
public:
	CloakingDeviceModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CloakingDeviceModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CountermeasureLauncherModule : public ActivatableModule {
public:
	CountermeasureLauncherModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CountermeasureLauncherModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class CPUEnhancerModule : public ActivatableModule {
public:
	CPUEnhancerModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~CPUEnhancerModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class DamageControlModule : public ActivatableModule {
public:
	DamageControlModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~DamageControlModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class DroneControlRangeModule : public ActivatableModule {
public:
	DroneControlRangeModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~DroneControlRangeModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class DroneModule : public ActivatableModule {
public:
	DroneModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~DroneModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

class DroneControlModule : public ActivatableModule {
public:
	DroneControlModule(Client *owner, InventoryItemRef self, InventoryItemRef charge_)
		: ActivatableModule("empWave", owner, self, charge_) {}
	virtual ~DroneControlModule() {}
protected:
	void _SendGodmaShipEffect(uint32 effect, bool active);
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	virtual void Process();
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
	virtual uint32 _ActivationInterval() const;

};

*/





#endif


