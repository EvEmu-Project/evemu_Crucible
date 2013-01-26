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
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship ,m_ShipAttrComp);
}

Afterburner::~Afterburner()
{

}

void Afterburner::Process()
{
	m_ActiveModuleProc->Process();
}

void Afterburner::Load()
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

void Afterburner::Activate(uint32 targetID)
{
	m_ActiveModuleProc->ActivateCycle();

	m_Ship->SetAttribute(AttrCharge, 134.999996046585);
	m_Ship->SetAttribute(AttrMaxVelocity, 387);
	m_Ship->SetAttribute(AttrSpeedBoostFactorCalc, 1.29126213592233);
	m_Ship->SetAttribute(AttrSpeedBoostFactorCalc2, 0.29126213592233);
	m_Ship->SetAttribute(AttrMass, 1648000);

	m_Item->SetActive(true, 1253, 10000, true);

	DoDestiny_SetBallMass mass;
	mass.entityID = m_Ship->itemID();
	mass.mass = 1648000;

	DoDestiny_SetMaxSpeed speed;
	speed.entityID = m_Ship->itemID();
	speed.speedValue = 387.378640776699;

	std::vector<PyTuple *> updates;
	updates.push_back(mass.Encode());
	updates.push_back(speed.Encode());

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, true);
}

void Afterburner::Deactivate() 
{
	m_ActiveModuleProc->DeactivateCycle();

	m_Ship->SetAttribute(AttrMaxVelocity, 300);
	m_Ship->SetAttribute(AttrSpeedBoostFactorCalc, 1.41811846689895);
	m_Ship->SetAttribute(AttrSpeedBoostFactorCalc2, 0.418118466898955);
	m_Ship->SetAttribute(AttrMass, 1148000);

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
