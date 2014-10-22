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
#include "ship/modules/weapon_modules/SuperWeapon.h"

SuperWeapon::SuperWeapon( InventoryItemRef item, ShipRef ship )
	: m_buildUpTimer(0), m_effectDurationTimer(0)
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
	m_chargeLoaded = false;
	m_effectID = 0;
}

SuperWeapon::~SuperWeapon()
{

}

void SuperWeapon::Process()
{
	m_ActiveModuleProc->Process();
}

void SuperWeapon::Load(InventoryItemRef charge)
{

}

void SuperWeapon::Unload()
{

}

void SuperWeapon::Repair()
{

}

void SuperWeapon::Overload()
{

}

void SuperWeapon::DeOverload()
{

}

void SuperWeapon::DestroyRig()
{

}

void SuperWeapon::Activate(SystemEntity * targetEntity)
{
	// TODO:
	// 1. Prevent activation on certain targets: asteroids, NPC stations, ice, clouds...  Perhaps restrict to ships and structures.
	// 2. Check for minimum qty of consumable materials needed according to the 'consumptionType' attribute value in appropriate cargo bay on board the ship

	m_targetEntity = targetEntity;
	m_targetID = targetEntity->Item()->itemID();

	// Activate active processing component timer:
	m_ActiveModuleProc->ActivateCycle();
	m_ModuleState = MOD_ACTIVATED;
	//_ShowCycle();
	m_ActiveModuleProc->ProcessActiveCycle();
}

void SuperWeapon::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void SuperWeapon::StopCycle(bool abort)
{
	// TODO:
	// 1. Enable warping and jump drive

	std::string effectString;
	switch (m_Item->typeID())
	{
		case 24550:
			effectString = "effects.SuperWeaponAmarr";
			m_effectID = effectSuperWeaponAmarr;
			break;

		case 24552:
			effectString = "effects.SuperWeaponCaldari";
			m_effectID = effectSuperWeaponCaldari;
			break;

		case 24554:
			effectString = "effects.SuperWeaponGallente";
			m_effectID = effectSuperWeaponGallente;
			break;

		case 23674:
			effectString = "effects.SuperWeaponMinmatar";
			m_effectID = effectSuperWeaponMinmatar;
			break;

		default:
			effectString = "";
			m_effectID = 0;
			break;
	}

	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = m_effectID;
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
		effectString,
		1,
		0,
		0,
		1.0,
		0
		);
}

void SuperWeapon::DoCycle()
{
	if( m_ActiveModuleProc->ShouldProcessActiveCycle() )
	{
		_ShowCycle();

		// Do massive, crazy damage to targets within a sphere of damage, screw CCP and their 'this can only target capital ships' junk ;)
		// NOTE: due to the small bit of time to "build up" the weapon firing action, we need a timer to delay application of damage to the target(s)
		// Avatar:  5 seconds to build after activation,  5 seconds duration, instant reach to target
		// Erebus:  6 seconds to build after activation,  6 seconds duration, instant reach to target
		// Leviathan:  range to target 124km,  6 seconds to launch after activation, 9 seconds flight time
		// Ragnarok:  range to target 125km,  6 seconds to build after activation, 4 seconds duration, instant reach to target
		//
		// TODO:
		// 0. Somehow disable warping and jump drive until deactivated
		// 1. Set 'm_buildUpTimer' to the "build up time"
		// 2. Set 'm_effectDurationTimer' to the "duration/flight time" of the super weapon effect
		// 3. Move Damage action to SuperWeapon::Process() function and check for timer expirations before applying damage
		// 4. Consider applying damage in increments, such as 5 stages of the 2 million hp (400,000 hp per stage, once per second)
		// 5. Loop through all entities in the bubble, filtering on dynamic entities that are ships, drones, structures, cans
		//    and calculate distance from target to calculate area-of-effect damage using some 1/x formula, where damage sustained
		//    drops off the further away from the primary target.

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
		if (m_Item->HasAttribute(AttrKineticDamage))
			kinetic_damage = (m_Item->GetAttribute(AttrKineticDamage)).get_float();
		if (m_Item->HasAttribute(AttrThermalDamage))
			thermal_damage = (m_Item->GetAttribute(AttrThermalDamage)).get_float();
		if (m_Item->HasAttribute(AttrEmDamage))
			em_damage = (m_Item->GetAttribute(AttrEmDamage)).get_float();
		if (m_Item->HasAttribute(AttrExplosiveDamage))
			explosive_damage = (m_Item->GetAttribute(AttrExplosiveDamage)).get_float();

		Damage damageDealt
			(
			m_Ship->GetOperator()->GetSystemEntity(),
			m_Item,
			kinetic_damage,			// kinetic damage
			thermal_damage,			// thermal damage
			em_damage,				// em damage
			explosive_damage,		// explosive damage
			(EVEEffectID)(m_effectID)	// from EVEEffectID::
			);

		m_targetEntity->ApplyDamage(damageDealt);

		// Reduce consumable in cargo:
		// TODO
	}
}

void SuperWeapon::_ShowCycle()
{
	std::string effectString;
	switch (m_Item->typeID())
	{
	case 24550:
		effectString = "effects.SuperWeaponAmarr";
		m_effectID = effectSuperWeaponAmarr;
		break;

	case 24552:
		effectString = "effects.SuperWeaponCaldari";
		m_effectID = effectSuperWeaponCaldari;
		break;

	case 24554:
		effectString = "effects.SuperWeaponGallente";
		m_effectID = effectSuperWeaponGallente;
		break;

	case 23674:
		effectString = "effects.SuperWeaponMinmatar";
		m_effectID = effectSuperWeaponMinmatar;
		break;

	default:
		effectString = "";
		m_effectID = 0;
		break;
	}

	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = m_effectID;		// From EVEEffectID::
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
		effectString,
		1,
		1,
		1,
		m_Item->GetAttribute(AttrDuration).get_float(),
		1000
		);
}
