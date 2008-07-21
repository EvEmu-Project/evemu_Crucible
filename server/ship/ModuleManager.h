/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __MODULEMANAGER_H_INCL__
#define __MODULEMANAGER_H_INCL__

#include "../common/timer.h"
#include "../common/packet_types.h"
#include <string>
#include <map>

class InventoryItem;
class SystemEntity;
class Client;

class ShipModule {
public:
	//factory method:
	static ShipModule *CreateModule(Client *owner, InventoryItem *self, InventoryItem *charge);
	
protected:
	ShipModule(Client *owner, InventoryItem *self, InventoryItem *charge);
public:
	virtual ~ShipModule();
	
	InventoryItem *item() const { return(m_item); }
	InventoryItem *charge() const { return(m_charge); }
	virtual void Process();
	virtual int Activate(const std::string &effectName, uint32 target, uint32 repeat);
	virtual void Deactivate(const std::string &effectName);
	
	void ChangeCharge(InventoryItem *new_charge);

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
	InventoryItem *const m_item;	//we own a ref to this.
	InventoryItem *m_charge;	//we own a ref to this. may be NULL.
	Timer m_timer;
};

class PassiveModule : public ShipModule {
public:
	PassiveModule(Client *owner, InventoryItem *self, InventoryItem *charge_)
		: ShipModule(owner, self, charge_) {}
	virtual ~PassiveModule() {}
};

class ActivatableModule : public ShipModule {
public:
	ActivatableModule(const char *effectName, Client *owner, InventoryItem *self, InventoryItem *charge_);
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
	
private:
	//its possible that a module offers multiple active effects, but
	//I am not sure.. this will not support it if that is the case...
	const char *const m_effectName;	//must point into static string data.
};

class HybridWeaponModule : public ActivatableModule {
public:
	HybridWeaponModule(Client *owner, InventoryItem *self, InventoryItem *charge_)
		: ActivatableModule("projectileFired", owner, self, charge_) {}
	virtual ~HybridWeaponModule() {}
protected:
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
};

class LaserWeaponModule : public ActivatableModule {
public:
	LaserWeaponModule(Client *owner, InventoryItem *self, InventoryItem *charge_)
		: ActivatableModule("targetAttack", owner, self, charge_) {}
	virtual ~LaserWeaponModule() {}
protected:
	virtual void StartEffect();
	virtual void DoEffect();
	virtual void StopEffect();
};

class MiningLaserModule : public ActivatableModule {
public:
	MiningLaserModule(Client *owner, InventoryItem *self, InventoryItem *charge_)
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
	GenericShipModule(Client *owner, InventoryItem *self, InventoryItem *charge_)
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
	void ReplaceCharges(EVEItemFlags flag, InventoryItem *charge);
	
	
protected:
	Client *const m_pilot;
	
	static uint8 FlagToSlot(EVEItemFlags flag);
	static EVEItemFlags SlotToFlag(uint8 slot);
	
	std::map<uint32, uint8> m_moduleByID;	//maps itemID to m_modules index
	ShipModule *m_modules[MAX_MODULE_COUNT];
};





#endif


