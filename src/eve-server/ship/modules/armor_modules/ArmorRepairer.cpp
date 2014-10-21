/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        AknorJaden
*/

#include "eve-server.h"

#include "EntityList.h"
#include "system/SystemBubble.h"
#include "system/Damage.h"
#include "ship/modules/armor_modules/ArmorRepairer.h"

ArmorRepairer::ArmorRepairer( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
	m_chargeLoaded = false;
}

ArmorRepairer::~ArmorRepairer()
{

}

void ArmorRepairer::Process()
{
	m_ActiveModuleProc->Process();
}

void ArmorRepairer::Load(InventoryItemRef charge)
{

}

void ArmorRepairer::Unload()
{

}

void ArmorRepairer::Repair()
{

}

void ArmorRepairer::Overload()
{

}

void ArmorRepairer::DeOverload()
{

}

void ArmorRepairer::DestroyRig()
{

}

void ArmorRepairer::Activate(SystemEntity * targetEntity)
{
	// Activate active processing component timer:
	m_ActiveModuleProc->ActivateCycle();
	m_ModuleState = MOD_ACTIVATED;
	//_ShowCycle();
	m_ActiveModuleProc->ProcessActiveCycle();
}

void ArmorRepairer::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void ArmorRepairer::StopCycle(bool abort)
{
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectArmorRepair;
	shipEff.when = Win32TimeNow();
	shipEff.start = 0;
	shipEff.active = 0;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyInt(shipEff.effectID));

	shipEff.environment = env;
	shipEff.startTime = shipEff.when;
	shipEff.duration = 1.0;		//m_ActiveModuleProc->GetRemainingCycleTimeMS();		// At least, I'm assuming this is the remaining time left in the cycle
	shipEff.repeat = new PyInt(0);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyList* events = new PyList;
	events->AddItem(shipEff.Encode());

	Notify_OnMultiEvent multi;
	multi.events = events;

	PyTuple* tmp = multi.Encode();

	m_Ship->GetOperator()->SendDogmaNotification("OnMultiEvent", "clientID", &tmp);

	m_ActiveModuleProc->DeactivateCycle();

	// Create Special Effect:
	m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
	(
		m_Ship,
		m_Item->itemID(),
		m_Item->typeID(),
		0,
		0,
		"effects.ArmorRepair",
		0,
		0,
		0,
		1.0,
		0
	);
}

void ArmorRepairer::DoCycle()
{
	if( m_ActiveModuleProc->ShouldProcessActiveCycle() )
	{
		_ShowCycle();

		// Apply repair amount:
		EvilNumber newDamageAmount;
		if( m_Item->GetAttribute(AttrArmorDamageAmount) <= m_Ship->GetAttribute(AttrArmorDamage) )
			newDamageAmount = m_Ship->GetAttribute(AttrArmorDamage) - m_Item->GetAttribute(AttrArmorDamageAmount);
		else
			newDamageAmount = 0.0;

        m_Ship->SetAttribute(AttrArmorDamage, newDamageAmount);
	}
}

void ArmorRepairer::_ShowCycle()
{
	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectArmorRepair;		// From EVEEffectID::
	shipEff.when = Win32TimeNow();
	shipEff.start = 1;
	shipEff.active = 1;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyInt(shipEff.effectID));

	shipEff.environment = env;
	shipEff.startTime = shipEff.when;
	shipEff.duration = m_Item->GetAttribute(AttrSpeed).get_float();
	shipEff.repeat = new PyInt(1000);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyTuple* tmp = new PyTuple(3);
	//tmp->SetItem(1, dmgMsg.Encode());
	tmp->SetItem(2, shipEff.Encode());

	std::vector<PyTuple*> events;
	//events.push_back(dmgMsg.Encode());
	events.push_back(shipEff.Encode());

	std::vector<PyTuple*> updates;
	//updates.push_back(dmgChange.Encode());

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);

	// Create Special Effect:
	m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
	(
		m_Ship,
		m_Item->itemID(),
		m_Item->typeID(),
		0,
		0,
		"effects.ArmorRepair",
		0,
		1,
		1,
		m_Item->GetAttribute(AttrDuration).get_float(),
		1000
	);
}
