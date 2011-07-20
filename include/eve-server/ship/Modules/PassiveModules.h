
#ifndef PASSIVE_MODULES_H
#define PASSIVE_MODULES_H

#include "ship/Modules/Modules.h"

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

	void Load()
	{
		//does nothing
	}

	void Unload()
	{
		//does nothing
	}


};

class DamageControl : public PassiveModule
{
public:
	DamageControl(InventoryItemRef item, ShipRef ship)
	{
		m_Item = item;
		m_Ship = ship;
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

	void SetAttribute(uint32 attrID, EvilNumber val) 
	{
		m_Item->SetAttribute(attrID, val);
	}

	EvilNumber GetAttribute(uint32 attrID)
	{
		return m_Item->GetAttribute(attrID);
	}


	//access functions
	uint32 itemID() { return m_Item->itemID(); }

	EVEItemFlags flag() { return m_Item->flag(); }

private:
	InventoryItemRef m_Item;
	ShipRef m_Ship;

};

#endif