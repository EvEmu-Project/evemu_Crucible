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
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(this, ship ,m_ShipAttrComp);
}

EnergyTurret::~EnergyTurret()
{

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
	
	Notify_OnDamageMessages dmgMsg;
	
}

void EnergyTurret::Deactivate() 
{
	m_Ship->SetAttribute(AttrMaxVelocity, 300);
	m_Ship->SetAttribute(AttrSpeedBoostFactorCalc, 1.41811846689895);
	m_Ship->SetAttribute(AttrSpeedBoostFactorCalc2, 0.418118466898955);
	m_Ship->SetAttribute(AttrMass, 1148000);

	m_Item->SetActive(false, 1253, 10000, false);

	DoDestiny_SetMaxSpeed speed;
	speed.entityID = m_Ship->itemID();
	speed.speedValue = 300;

	DoDestiny_SetBallMass mass;
	mass.entityID = m_Ship->itemID();
	mass.mass = 1148000;

	std::vector<PyTuple *> updates;
	updates.push_back(speed.Encode());
	updates.push_back(mass.Encode());

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, true);
}
