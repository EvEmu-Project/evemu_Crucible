
#ifndef ACTIVE_MODULES_H
#define ACTIVE_MODULES_H

#include "Modules.h"

class ActiveModule : public GenericModule
{
public:
	ActiveModule()
	{

	}

	virtual ~ActiveModule() { }

	//should define process

	
	

	//access functions
	ModulePowerLevel GetModulePowerLevel()					{ return isHighPower() ? HIGH_POWER : ( isMediumPower() ? MEDIUM_POWER : LOW_POWER); }

	bool isHighPower()										{ return m_Effects->isHighSlot(); }
	bool isMediumPower()									{ return m_Effects->isMediumSlot(); }
	bool isLowPower()										{ return m_Effects->isLowSlot(); }
	bool isRig()											{ return false; }
	bool isSubSystem()										{ return false; }

};


class Afterburner : public ActiveModule
{
public:

	Afterburner( InventoryItemRef item, ShipRef ship )
	{
		m_Item = item;
		m_Ship = ship;
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