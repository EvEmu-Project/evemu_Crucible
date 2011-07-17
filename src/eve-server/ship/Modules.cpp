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

//GenericModule class definitions
#pragma region GenericModuleClass
GenericModule::GenericModule(InventoryItemRef item, ShipRef ship)
{
	m_Item = item;
	m_Ship = ship;
}

void GenericModule::Process()
{

}

void GenericModule::Online()
{

}

void GenericModule::Offline()
{

}

void GenericModule::Deactivate()
{

}

void GenericModule::Repair()
{

}

void GenericModule::Unload()
{

}


void GenericModule::Overload()
{

}

void GenericModule::DeOverload()
{

}

void GenericModule::Load()
{

}

void GenericModule::DestroyRig()
{

}

uint32 GenericModule::itemID()
{
	return m_Item->itemID();
}

EVEItemFlags GenericModule::flag()
{
	return m_Ship->flag();
}

EvilNumber GenericModule::CalculateNewAttributeValue(EvilNumber attrVal, EvilNumber attrMod, EVECalculationType type)
{
	switch(type)
	{
	case ADD :					return Add(attrVal, attrMod);  
	case SUBTRACT :				return Subtract(attrVal, attrMod);
	case DIVIDE :				return Divide(attrVal, attrMod);
	case MULTIPLY :				return Multiply(attrVal, attrMod);
	case ADD_PERCENT :			return AddPercent(attrVal, attrMod);
	case ADD_AS_PERCENT :		return AddAsPercent(attrVal, attrMod);
	case SUBTRACT_PERCENT :		return SubtractPercent(attrVal, attrMod);
	case SUBTRACT_AS_PERCENT :  return SubtractAsPercent(attrVal, attrMod);
	}

}

void GenericModule::SetAttribute(uint32 attrID, EvilNumber val)
{
	m_Item->SetAttribute(attrID, val);
}

EvilNumber GenericModule::GetAttribute(uint32 attrID)
{
	return m_Item->GetAttribute(attrID);
}

#pragma endregion