
#ifndef PASSIVE_MODULES_H
#define PASSIVE_MODULES_H

#include "ship/Modules/Modules.h"
#include "ship/Modules/components/ModifyShipAttributesComponent.h"

class PassiveModule : public GenericModule
{
public:
	PassiveModule() { }
	~PassiveModule() { }

	void Process()
	{
		//does nothing for passive modules
	}

	void Activate()
	{
		//does nothing for passive modules
	}

	void Deactivate()
	{
		//does nothing for passive modules
	}

	void Load()
	{
		//does nothing
	}

	void Unload()
	{
		//does nothing
	}

	void Overload()
	{
		//does nothing
	}

	void DeOverload()
	{
		//does nothing
	}

	void DestroyRig()
	{
		//not a rig
	}


	//access functions
	ModulePowerLevel GetModulePowerLevel()					{ return isHighPower() ? HIGH_POWER : ( isMediumPower() ? MEDIUM_POWER : LOW_POWER); }

	bool isHighPower()										{ return m_Effects->isHighSlot(); }
	bool isMediumPower()									{ return m_Effects->isMediumSlot(); }
	bool isLowPower()										{ return m_Effects->isLowSlot(); }
	bool isRig()											{ return false; }
	bool isSubSystem()										{ return false;	}

	

protected:
	ModifyShipAttributesComponent * m_ShipAttrComp;

};

class ArmorReinforcer : public PassiveModule
{
public:
	ArmorReinforcer(InventoryItemRef item, ShipRef ship)
	{
		m_Item = item;
		m_Ship = ship;
		m_Effects = new ModuleEffects(m_Item->typeID());
		m_ShipAttrComp = new ModifyShipAttributesComponent(this, m_Ship);
	}

	~ArmorReinforcer()
	{
		//delete members
		delete m_Effects;

		//null ptrs
		m_Effects = NULL;
	}

	void Offline() 
	{

	}

	void Online()
	{

	}

	void Repair()
	{

	}

};

#endif