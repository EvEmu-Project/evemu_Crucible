
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

	//accessors
	void SetAttribute(uint32 attrID, EvilNumber val)		{ m_Item->SetAttribute(attrID, val); }
	EvilNumber GetAttribute(uint32 attrID)					{ return m_Item->GetAttribute(attrID); }

	uint32 itemID()											{ return m_Item->itemID(); }
	EVEItemFlags flag()										{ return m_Item->flag(); }
	uint32 typeID()											{ return m_Item->typeID(); }
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

	 //accessors
	 bool isHighPower()					{ return false; }
	 bool isMediumPower()				{ return  true; }
	 bool isLowPower()					{ return false; }

};

#endif