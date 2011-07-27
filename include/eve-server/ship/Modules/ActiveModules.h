
#ifndef ACTIVE_MODULES_H
#define ACTIVE_MODULES_H

#include "Modules.h"

class ActiveModule : public GenericModule
{
public:
	ActiveModule()
	{

	}

	virtual ~ActiveModule() 
	{
		//delete members
		delete m_Effects;
		delete m_ShipAttrComp;

		//null ptrs
		m_Effects = NULL;
		m_ShipAttrComp = NULL;
	}

	//should define process

	
	

	//access functions
	ModulePowerLevel GetModulePowerLevel()					{ return isHighPower() ? HIGH_POWER : ( isMediumPower() ? MEDIUM_POWER : LOW_POWER); }

	bool isHighPower()										{ return m_Effects->isHighSlot(); }
	bool isMediumPower()									{ return m_Effects->isMediumSlot(); }
	bool isLowPower()										{ return m_Effects->isLowSlot(); }
	bool isRig()											{ return false; }
	bool isSubSystem()										{ return false; }
	bool requiresTarget()									{ return m_Effects->GetIsAssistance() || m_Effects->GetIsOffensive(); }

protected:
	ModifyShipAttributesComponent m_ShipAttrComp;
};


class Afterburner : public ActiveModule
{
public:

	Afterburner( InventoryItemRef item, ShipRef ship )
	{
		m_Item = item;
		m_Ship = ship;
		m_Effects = new ModuleEffects(m_Item->typeID());
		m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	}

	~Afterburner()
	{

	}

	 void Process() 
	 {

	 }

	 void Offline() 
	 {

	 }

	 void Online()
	 {

	 }

	 void Activate()
	 {

	 }

	 void Deactivate()
	 {

	 }

	 void Load()
	 {

	 }

	 void Unload()
	 {

	 }

	 void Repair()
	 {

	 }

	 void Overload()
	 {

	 }

	 void DeOverload()
	 {

	 }

	 void DestroyRig()
	 {

	 }



};

#endif