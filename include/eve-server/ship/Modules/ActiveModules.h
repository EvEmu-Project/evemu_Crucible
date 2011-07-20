
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

};


class Afterburner : public ActiveModule
{
public:

	Afterburner( InventoryItemRef item, ShipRef ship )
	{
		m_Item = item;
		m_Ship = ship;
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

	 void SetAttribute(uint32 attrID, EvilNumber val) 
	 {

	 }

	 EvilNumber GetAttribute(uint32 attrID)
	 {
		 return EvilNumber(1); //idk if this should be new'd or not
	 }


	//access functions
	uint32 itemID() { return m_Item->itemID(); }

	EVEItemFlags flag() { return m_Item->flag(); }



private:
	InventoryItemRef m_Item;      //we do not own this
	ShipRef m_Ship;                     //we do not own this

};

#endif