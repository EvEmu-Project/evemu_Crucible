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
//#include "ship/Modules/components/ActiveModuleProcessingComponent.h"


ActiveModuleProcessingComponent::ActiveModuleProcessingComponent(GenericModule * mod, ShipRef ship, ModifyShipAttributesComponent * shipAttrMod)
: m_Stop( false ), m_Mod( mod ), m_Ship( ship ), m_ShipAttrModComp( shipAttrMod )
{

}

ActiveModuleProcessingComponent::~ActiveModuleProcessingComponent()
{
	//nothing to do yet
}

void ActiveModuleProcessingComponent::DeactivateCycle()
{
	m_Stop = true;
}

//timing and verification function
bool ActiveModuleProcessingComponent::ShouldProcessActiveCycle()
{
	//first check time for cycle timer

	//next check that we have enough capacitor avaiable

	//finally check if we have been told to deactivate

    return false;
}

void ActiveModuleProcessingComponent::ProcessActiveCycle()
{
	//check for stop signal
	if(m_Stop)
		return;

	//else consume capacitor

		
	//then check if we are targeting another ship or not
}
