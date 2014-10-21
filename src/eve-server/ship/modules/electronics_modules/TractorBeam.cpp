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
#include "ship/modules/electronics_modules/TractorBeam.h"

TractorBeam::TractorBeam( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
	m_chargeLoaded = false;

	m_ModuleState = MOD_UNFITTED;
	m_ChargeState = MOD_UNLOADED;
}

TractorBeam::~TractorBeam()
{

}

void TractorBeam::Process()
{
	m_ActiveModuleProc->Process();
}

void TractorBeam::Load(InventoryItemRef charge)
{
	ActiveModule::Load(charge);
	m_ChargeState = MOD_LOADED;
}

void TractorBeam::Unload()
{
	ActiveModule::Unload();
	m_ChargeState = MOD_UNLOADED;
}

void TractorBeam::Repair()
{

}

void TractorBeam::Overload()
{

}

void TractorBeam::DeOverload()
{

}

void TractorBeam::DestroyRig()
{

}

void TractorBeam::Activate(SystemEntity * targetEntity)
{
	// Check to make sure target is NOT a static entity:
	// TODO: Check for target = asteroid, ice, or gas cloud then only allow tractoring if ship = Orca
	// TODO: DO NOT allow tractoring of Client-connected player ships
	if (!(targetEntity->IsStaticEntity()))
	{
		if (
		     (
				(m_Ship->typeID() == 28606)		// Orca is the only ship allowed to tractor asteroids and ice chunks
				&&
				(
				((getItem()->typeID() == 16278 || getItem()->typeID() == 22229) && (targetEntity->Item()->groupID() == EVEDB::invGroups::Ice))
				||
				(targetEntity->Item()->categoryID() == EVEDB::invCategories::Asteroid)
				||
				((targetEntity->Item()->groupID() == EVEDB::invGroups::Harvestable_Cloud) && (getItem()->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester))
				)
				)
				||
				(targetEntity->Item()->groupID() == EVEDB::invGroups::Cargo_Container)
				||
				(targetEntity->Item()->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
				||
				(targetEntity->Item()->groupID() == EVEDB::invGroups::Wreck)
			)
		{
			m_targetEntity = targetEntity;
			m_targetID = targetEntity->Item()->itemID();

			// Activate active processing component timer:
			m_ActiveModuleProc->ActivateCycle();
			m_ModuleState = MOD_ACTIVATED;
			//_ShowCycle();
			m_ActiveModuleProc->ProcessActiveCycle();
		}
	}
}

void TractorBeam::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void TractorBeam::StopCycle(bool abort)
{
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectTractorBeam;		// From EVEEffectID::
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
		0,
		"effects.TractorBeam",
		0,
		0,
		0,
		m_Item->GetAttribute(AttrDuration).get_float(),
		0
	);
}

void TractorBeam::DoCycle()
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

		// Initiate continued Destiny Action to move tractored object toward ship
		DynamicSystemEntity * targetEntity = static_cast<DynamicSystemEntity *>(m_targetEntity);
		// Check for distance to target > 5000m + ship radius
		GVector distanceToTarget(targetEntity->GetPosition(), m_Ship->position());
		if (distanceToTarget.length() > (5000.0 + m_Ship->GetAttribute(AttrRadius).get_float()))
		{
			// Range higher?  Then start it moving toward ship @ 200m/s
			targetEntity->Destiny()->SetMaxVelocity(1000.0);
			targetEntity->Destiny()->SetSpeedFraction(1.0);
			// Tractor objects at 1000m/s:
			targetEntity->Destiny()->TractorBeamFollow(m_Ship->GetOperator()->GetSystemEntity(), 10, 1000, (5000.0 + m_Ship->GetAttribute(AttrRadius).get_float()));
		}
		else
		{
			targetEntity->Destiny()->TractorBeamHalt();
			Deactivate();
		}
	}
}

void TractorBeam::_ShowCycle()
{
	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectTractorBeam;		// From EVEEffectID::
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
		0,
		"effects.TractorBeam",
		0,
		1,
		1,
		m_Item->GetAttribute(AttrDuration).get_float(),
		1000
	);
}
