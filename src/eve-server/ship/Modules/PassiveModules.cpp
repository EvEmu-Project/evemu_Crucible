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
	Author:		Luck
*/

#include "EVEServerPCH.h"
//#include "ship/Modules/Modules.h"
//#include "ship/Modules/components/ModifyShipAttributesComponent.h"


PassiveModule::PassiveModule(InventoryItemRef item, ShipRef ship)
{
	m_Item = item;
	m_Ship = ship;
	m_Effects = new ModuleEffects(m_Item->typeID());
	m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
}

PassiveModule::~PassiveModule()
{
	//delete members
	delete m_Effects;
	delete m_ShipAttrComp;

	//null ptrs
	m_Effects = NULL;
	m_ShipAttrComp = NULL;
}

void PassiveModule::Offline() 
{
	//remove item attributes
	//m_Effects->SetDefaultEffectAsActive();
	//for(uint32 i = 0; i < m_Effects->GetSizeOfAttributeList(); i++)
	//{
	//	m_ShipAttrComp->ModifyShipAttribute(m_Effects->GetTargetAttributeID(i), m_Effects->GetSourceAttributeID(i), m_Effects->GetReverseCalculationType(i));
	//}

	//change item state
	m_Item->PutOffline();
}

void PassiveModule::Online()
{
	//add item attributes
	//m_Effects->SetDefaultEffectAsActive();
	//for(uint32 i = 0; i < m_Effects->GetSizeOfAttributeList(); i++)
	//{
	//	m_ShipAttrComp->ModifyShipAttribute(m_Effects->GetTargetAttributeID(i), m_Effects->GetSourceAttributeID(i), m_Effects->GetCalculationType(i));
	//}

	//change item state
	m_Item->PutOnline();
}
