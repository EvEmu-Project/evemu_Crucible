/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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

/** @todo  this entire file needs rework to update with current code  */


#include "eve-server.h"

#include "ship/Ship.h"
#include "ship/modules/SuperWeapon.h"
#include "system/SystemBubble.h"
#include "system/Damage.h"

SuperWeapon::SuperWeapon( InventoryItemRef item, ShipItemRef ship )
:
	m_buildUpTimer(0), m_effectDurationTimer(0)
{
}

void SuperWeapon::Activate(SystemEntity* pSE)
{
	// TODO:
	// 1. Prevent activation on certain targets: asteroids, NPC stations, ice, clouds...  Perhaps restrict to ships and structures.
	// 2. Check for minimum qty of consumable materials needed according to the 'consumptionType' attribute value in appropriate cargo bay on board the ship

    m_targetSE = pSE;
    m_targetID = pSE->GetID();

	// Activate active processing component timer:
	ActiveModule::Activate(pSE);
	//_ShowCycle();
	//m_ActiveModuleProc->ProcessActiveCycle();
}

void SuperWeapon::StopCycle(bool abort)
{
	// TODO:
	// 1. Enable warping and jump drive

    std::string effectString = "";
    uint32 effectID = 0;
	switch (m_modRef->typeID())
	{
		case 24550:
			effectString = "effects.SuperWeaponAmarr";
            effectID = effectSuperWeaponAmarr;
			break;

		case 24552:
			effectString = "effects.SuperWeaponCaldari";
            effectID = effectSuperWeaponCaldari;
			break;

		case 24554:
			effectString = "effects.SuperWeaponGallente";
            effectID = effectSuperWeaponGallente;
			break;

		case 23674:
			effectString = "effects.SuperWeaponMinmatar";
            effectID = effectSuperWeaponMinmatar;
			break;
	}

    uint32 timeLeft = GetRemainingCycleTimeMS();
    timeLeft /= 1000;

	// Create Special Effect:
	m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
	(
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        0,
        0,
        effectString,
        1,
        0,
        0,
        timeLeft,
        0
    );

	GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = m_shipRef->itemID();
        ge.target = PyStatic.NewNone();
        ge.subLoc = PyStatic.NewNone();
        ge.area = new PyList;
        ge.effectID = effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = (shipEff.timeNow + (timeLeft * EvE::Time::Second));
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = PyStatic.NewNone();

    PyTuple* tuple = shipEff.Encode();

    if (m_destinyMgr->IsWarping() or (m_bubble == nullptr))
        m_shipRef->GetPilot()->QueueDestinyEvent(&tuple);
    else
        m_bubble->BubblecastDestinyEvent(&tuple, "destiny");
}

double SuperWeapon::DoCycle()
{
        if ((!m_shipRef->GetPilot()->GetShipSE()->SysBubble())
            || (!m_shipRef->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID))
            /*|| (!m_chargeLoaded) || (!m_chargeRef)*/ )
        {
            Deactivate();
            return 0;
        }

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

        Damage d(m_shipRef->GetPilot()->GetShipSE(),
                 m_modRef,
                 m_chargeRef->GetAttribute(AttrKineticDamage).get_float(),     // kinetic damage
                 m_chargeRef->GetAttribute(AttrThermalDamage).get_float(),     // thermal damage
                 m_chargeRef->GetAttribute(AttrEmDamage).get_float(),     // em damage
                 m_chargeRef->GetAttribute(AttrExplosiveDamage).get_float(),     // explosive damage
                 1,
                 effectProjectileFired       // from EVEEffectID::
        );

        m_targetSE->ApplyDamage(d);

		// Reduce consumable in cargo:
		// TODO
        return m_cycleTime;
}

void SuperWeapon::_ShowCycle()
{
    std::string effectString = "";
    uint32 effectID = 0;
	switch (m_modRef->typeID())
	{
	case 24550:
		effectString = "effects.SuperWeaponAmarr";
        effectID = effectSuperWeaponAmarr;
		break;

	case 24552:
		effectString = "effects.SuperWeaponCaldari";
        effectID = effectSuperWeaponCaldari;
		break;

	case 24554:
		effectString = "effects.SuperWeaponGallente";
        effectID = effectSuperWeaponGallente;
		break;

	case 23674:
		effectString = "effects.SuperWeaponMinmatar";
        effectID = effectSuperWeaponMinmatar;
		break;
	}

	// Create Special Effect:
	m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
	(
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        0,
        0,
        effectString,
        1,
        1,
        1,
        m_cycleTime,
        1
    );

    // Create Destiny Updates:
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = m_shipRef->itemID();;
        ge.targetID = 0;
        ge.subLoc = sDataMgr.NewNone();
        ge.area = new PyList;
        ge.effectID = effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = m_cycleTime;
        shipEff.repeat = m_repeat;
        shipEff.error = sDataMgr.NewNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
    //m_shipRef->GetPilot()->QueueDestinyEvent(&tup);
}

void SuperWeapon::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
