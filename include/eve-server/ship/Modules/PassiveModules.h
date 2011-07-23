
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



	void SetAttribute(uint32 attrID, EvilNumber val)	{ m_Item->SetAttribute(attrID, val); }
	EvilNumber GetAttribute(uint32 attrID)				{ return m_Item->GetAttribute(attrID); }

	//access functions
	uint32 itemID()										{ return m_Item->itemID(); }
	EVEItemFlags flag()									{ return m_Item->flag(); }
	uint32 typeID()										{ return m_Item->typeID(); }
	bool isHighPower()									{ return m_Effects->isHighSlot(); }
	bool isMediumPower()								{ return m_Effects->isMediumSlot(); }
	bool isLowPower()									{ return m_Effects->isLowSlot(); }
	bool isRig()										{ return false; }
	bool isSubSystem()									{ return false;	}

protected:


};

class ArmorReinforcer : public PassiveModule
{
public:
	ArmorReinforcer(InventoryItemRef item, ShipRef ship)
	{
		m_Item = item;
		m_Ship = ship;
		m_Effects = new ModuleEffects(m_Item->typeID());
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