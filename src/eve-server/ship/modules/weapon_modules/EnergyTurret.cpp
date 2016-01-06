/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Reve, AknorJaden
*/

#include "eve-server.h"

#include "EntityList.h"
#include "system/SystemBubble.h"
#include "system/Damage.h"
#include "ship/modules/weapon_modules/EnergyTurret.h"

EnergyTurret::EnergyTurret( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
	m_chargeLoaded = false;
}

EnergyTurret::~EnergyTurret()
{

}

void EnergyTurret::Process()
{
	m_ActiveModuleProc->Process();
}

void EnergyTurret::Load(InventoryItemRef charge)
{
	ActiveModule::Load(charge);
}

void EnergyTurret::Unload()
{
	ActiveModule::Unload();
}

void EnergyTurret::Repair()
{

}

void EnergyTurret::Overload()
{

}

void EnergyTurret::DeOverload()
{

}

void EnergyTurret::DestroyRig()
{

}

void EnergyTurret::Activate(SystemEntity * targetEntity)
{
	if( this->m_chargeRef )
	{
		m_targetEntity = targetEntity;
		m_targetID = targetEntity->Item()->itemID();

		// Activate active processing component timer:
		m_ActiveModuleProc->ActivateCycle();
		m_ModuleState = MOD_ACTIVATED;
		//_ShowCycle();
		m_ActiveModuleProc->ProcessActiveCycle();
	}
	else
	{
		sLog.Error( "EnergyTurret::Activate()", "ERROR: Cannot find charge that is supposed to be loaded into this module!" );
		throw PyException( MakeCustomError( "ERROR!  Cannot find charge that is supposed to be loaded into this module!" ) );
	}
}

void EnergyTurret::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void EnergyTurret::StopCycle()
{
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectTargetAttack;
	shipEff.when = Win32TimeNow();
	shipEff.start = 0;
	shipEff.active = 0;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyInt(m_targetID));
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
		m_targetID,
		m_chargeRef->itemID(),
		"effects.Laser",
		1,
		0,
		0,
		1.0,
		0
	);
}

void EnergyTurret::DoCycle()
{
	if( m_ActiveModuleProc->ShouldProcessActiveCycle() )
	{
		// Check to see if our target is still in this bubble or has left or been destroyed:
		if( m_Ship->GetOperator()->GetSystemEntity()->Bubble() == NULL )
		{
			// Target has left our bubble or been destroyed, deactivate this module:
			Deactivate();
			return;
		}
		else
		{
			if( !(m_Ship->GetOperator()->GetSystemEntity()->Bubble()->GetEntity(m_targetID)) )
			{
				// Target has left our bubble or been destroyed, deactivate this module:
				Deactivate();
				return;
			}
		}

		_ShowCycle();

		// Create Damage action:
		//Damage( SystemEntity *_source,
        //    InventoryItemRef _weapon,
        //    double _kinetic,
        //    double _thermal,
        //    double _em,
        //    double _explosive,
        //    EVEEffectID _effect );
		double kinetic_damage = 0.0;
		double thermal_damage = 0.0;
		double em_damage = 0.0;
		double explosive_damage = 0.0;

		// This still somehow needs skill, ship, module, and implant bonuses to be applied:
		// This still somehow needs to have optimal range and falloff attributes applied as a damage modification factor:
		if( m_chargeRef->HasAttribute(AttrKineticDamage) )
			kinetic_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrKineticDamage)).get_float();
		if( m_chargeRef->HasAttribute(AttrThermalDamage) )
			thermal_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrThermalDamage)).get_float();
		if( m_chargeRef->HasAttribute(AttrEmDamage) )
			em_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrEmDamage)).get_float();
		if( m_chargeRef->HasAttribute(AttrExplosiveDamage) )
			explosive_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrExplosiveDamage)).get_float();

		Damage damageDealt
		(
			m_Ship->GetOperator()->GetSystemEntity(),
			m_Item,
			kinetic_damage,			// kinetic damage
			thermal_damage,			// thermal damage
			em_damage,				// em damage
			explosive_damage,		// explosive damage
			effectTargetAttack		// from EVEEffectID::
		);
		
		m_targetEntity->ApplyDamage( damageDealt );
	}
}

void EnergyTurret::_ShowCycle()
{
	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectTargetAttack;		// From EVEEffectID::
	shipEff.when = Win32TimeNow();
	shipEff.start = 1;
	shipEff.active = 1;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyInt(m_targetID));
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
		m_targetID,
		m_chargeRef->typeID(),
		"effects.Laser",
		1,
		1,
		1,
		m_Item->GetAttribute(AttrSpeed).get_float(),
		1
	);
}
