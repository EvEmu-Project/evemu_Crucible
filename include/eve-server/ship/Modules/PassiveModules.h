
#ifndef PASSIVE_MODULES_H
#define PASSIVE_MODULES_H

#include "ship/Modules/Modules.h"
#include "ship/Modules/components/ModifyShipAttributesComponent.h"

class PassiveModule : public GenericModule
{
public:
	PassiveModule() { }
	~PassiveModule() { }


	//access functions
	


	

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
		delete m_ShipAttrComp;

		//null ptrs
		m_Effects = NULL;
		m_ShipAttrComp = NULL;
	}

	void Offline() 
	{
		//m_Effects->SetActiveEffect(/*Find out what goes here */);
		m_Effects->SetDefaultEffectAsActive();
		for(int i = 0; i < m_Effects->GetSizeOfAttributeList(); i++)
		{
			m_ShipAttrComp->ModifyShipAttribute(m_Effects->GetTargetAttributeID(i), m_Effects->GetSourceAttributeID(i), m_Effects->GetReverseCalculationType(i));
		}
	}

	void Online()
	{
		//m_Effects->SetActiveEffect(/*Find out what goes here */);
		m_Effects->SetDefaultEffectAsActive();
		for(int i = 0; i < m_Effects->GetSizeOfAttributeList(); i++)
		{
			m_ShipAttrComp->ModifyShipAttribute(m_Effects->GetTargetAttributeID(i), m_Effects->GetSourceAttributeID(i), m_Effects->GetCalculationType(i));
		}
	}

};

#endif