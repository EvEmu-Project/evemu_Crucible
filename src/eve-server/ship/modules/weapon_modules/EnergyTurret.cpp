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
    Author:        Reve
*/

#include "eve-server.h"

#include "ship/modules/weapon_modules/EnergyTurret.h"

EnergyTurret::EnergyTurret( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship ,m_ShipAttrComp);
}

EnergyTurret::~EnergyTurret()
{

}

void EnergyTurret::Process()
{
	m_ActiveModuleProc->Process();
}

void EnergyTurret::Load()
{

}

void EnergyTurret::Unload()
{

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

void EnergyTurret::Activate(uint32 targetID)
{

	DoDestiny_OnDamageStateChange dmgChange;
	dmgChange.entityID = targetID;

	PyList *states = new PyList;
	states->AddItem(new PyFloat(0));
	states->AddItem(new PyFloat(0));
	states->AddItem(new PyFloat(0.99));
	dmgChange.state = states;

	m_Ship->SetAttribute(AttrCharge, 139);
	
	Notify_OnEffectHit effHit;
	effHit.itemID = m_Item->itemID();
	effHit.effectID = 10;
	effHit.targetID = targetID;
	effHit.damage = 5;

	Notify_OnDamageMessage dmgMsg;
	dmgMsg.messageID = "AttackHit3";
	dmgMsg.weapon = 3634;
	dmgMsg.splash = "";
	dmgMsg.target = targetID;
	dmgMsg.damage = 5;

	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = 10;
	shipEff.when = Win32TimeNow();
	shipEff.start = 1;
	shipEff.active = 1;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyInt(targetID));
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyInt(10));

	shipEff.environment = env;
	shipEff.startTime = shipEff.when;
	shipEff.duration = 1584;
	shipEff.repeat = new PyInt(1000);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyTuple* tmp = new PyTuple(3);
	tmp->SetItem(0, effHit.Encode());
	tmp->SetItem(1, dmgMsg.Encode());
	tmp->SetItem(2, shipEff.Encode());

	std::vector<PyTuple*> events;
	events.push_back(effHit.Encode());
	events.push_back(dmgMsg.Encode());
	events.push_back(shipEff.Encode());

	std::vector<PyTuple*> updates;
	updates.push_back(dmgChange.Encode());

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, true);
}

void EnergyTurret::Deactivate() 
{
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = 10;
	shipEff.when = Win32TimeNow();
	shipEff.start = 0;
	shipEff.active = 0;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyInt(targetID));
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyInt(10));

	shipEff.environment = env;
	shipEff.startTime = shipEff.when;
	shipEff.duration = 1584;
	shipEff.repeat = new PyInt(0);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyList* events = new PyList;
	events->AddItem(shipEff.Encode());

	Notify_OnMultiEvent multi;
	multi.events = events;

	PyTuple* tmp = multi.Encode();

	m_Ship->GetOperator()->SendDogmaNotification("OnMultiEvent", "clientID", &tmp);
}
