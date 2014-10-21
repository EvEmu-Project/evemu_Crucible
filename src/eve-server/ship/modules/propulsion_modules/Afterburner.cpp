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
    Author:        Luck
*/

#include "eve-server.h"

#include "ship/modules/propulsion_modules/Afterburner.h"

Afterburner::Afterburner( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

//	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
//	m_chargeLoaded = false;
}

Afterburner::~Afterburner()
{

}

void Afterburner::Process()
{
	m_ActiveModuleProc->Process();
}

void Afterburner::Load(InventoryItemRef charge)
{

}

void Afterburner::Unload()
{

}

void Afterburner::Repair()
{

}

void Afterburner::Overload()
{

}

void Afterburner::DeOverload()
{

}

void Afterburner::DestroyRig()
{

}

void Afterburner::Activate(SystemEntity * targetEntity)
{
	// Activate active processing component timer:
	m_ActiveModuleProc->ActivateCycle();
	m_ModuleState = MOD_ACTIVATED;
	//_ShowCycle();
	m_ActiveModuleProc->ProcessActiveCycle();
}

void Afterburner::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void Afterburner::StopCycle(bool abort)
{
	//m_Item->SetActive(false, 1253, 0.0, false);

	// Tell Destiny Manager about our new speed so it properly tracks ship movement:
	m_Ship->GetOperator()->GetDestiny()->SetMaxVelocity(m_Ship->GetDefaultAttribute(AttrMaxVelocity).get_float());
	m_Ship->GetOperator()->GetDestiny()->SetSpeedFraction(1.0);

	DoDestiny_SetMaxSpeed speed;
	speed.entityID = m_Ship->itemID();
	speed.speedValue = m_Ship->GetDefaultAttribute(AttrMaxVelocity).get_float();

	DoDestiny_SetBallMass mass;
	mass.entityID = m_Ship->itemID();
	mass.mass = m_Ship->GetAttribute(AttrMass).get_float();

	std::vector<PyTuple *> updates;
	updates.push_back(speed.Encode());
	updates.push_back(mass.Encode());

	m_ActiveModuleProc->DeactivateCycle();

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, false);

	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = 1254;
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
		"effects.SpeedBoost",
		0,
		0,
		0,
		1.0,
		0
		);
}

void Afterburner::DoCycle()
{
	if (m_ActiveModuleProc->ShouldProcessActiveCycle())
	{
		_ShowCycle();
	}
}

void Afterburner::_ShowCycle()
{
	//m_Item->SetActive(true, 1253, m_Item->GetAttribute(AttrDuration).get_float(), true);

	double implantBonuses = 1.0;	// TODO: gather and accumulate all implant bonuses for MWDs/Afterburners
	double accelerationControlSkillLevel = 0.0;	// TODO: Figure out how to get access to skills list of character running this ship and get this value
	double boostSpeed = m_Ship->GetAttribute(AttrMaxVelocity).get_float() * (1.0 + (m_Item->GetAttribute(AttrSpeedFactor).get_float() / 100.0 * (1 + accelerationControlSkillLevel * 0.05) * (implantBonuses) * (m_Item->GetAttribute(AttrSpeedBoostFactor).get_float() / (m_Ship->GetAttribute(AttrMass).get_float()))));

	// Tell Destiny Manager about our new speed so it properly tracks ship movement:
	m_Ship->GetOperator()->GetDestiny()->SetMaxVelocity(boostSpeed);
	m_Ship->GetOperator()->GetDestiny()->SetSpeedFraction(1.0);

	DoDestiny_SetBallMass mass;
	mass.entityID = m_Ship->itemID();
	mass.mass = m_Ship->GetAttribute(AttrMass).get_float();

	DoDestiny_SetMaxSpeed speed;
	speed.entityID = m_Ship->itemID();
	speed.speedValue = boostSpeed;

	std::vector<PyTuple *> updates;
	updates.push_back(mass.Encode());
	updates.push_back(speed.Encode());

	//m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, false);

	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = 1254;		// From EVEEffectID::
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
	shipEff.duration = m_Item->GetAttribute(AttrDuration).get_float();
	shipEff.repeat = new PyInt(1000);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyTuple* tmp = new PyTuple(3);
	//tmp->SetItem(1, dmgMsg.Encode());
	tmp->SetItem(2, shipEff.Encode());

	std::vector<PyTuple*> events;
	//events.push_back(dmgMsg.Encode());
	events.push_back(shipEff.Encode());

	//std::vector<PyTuple*> updates;
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
		"effects.SpeedBoost",
		0,
		1,
		1,
		m_Item->GetAttribute(AttrDuration).get_float(),
		1000
		);
}
