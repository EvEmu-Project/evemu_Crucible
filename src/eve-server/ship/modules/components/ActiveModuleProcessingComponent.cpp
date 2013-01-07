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

#include "ship/Ship.h"
#include "ship/modules/components/ActiveModuleProcessingComponent.h"

ActiveModuleProcessingComponent::ActiveModuleProcessingComponent(InventoryItemRef item, GenericModule * mod, ShipRef ship, ModifyShipAttributesComponent * shipAttrMod)
: m_Item( item ), m_Mod( mod ), m_Ship( ship ), m_ShipAttrModComp( shipAttrMod ), m_timer(0)
{
	m_Stop = false;
}

ActiveModuleProcessingComponent::~ActiveModuleProcessingComponent()
{
    //nothing to do yet
}

/****************************************************
	A little note about the timer:
		Timer.Check() has two functions:
			1. It checks if the timer has runed out 
			2. It subtracts from the start time
	Don't be fooled by it's name because if you don't
	call it in a loop, you won't get the time moving.
*****************************************************/


void ActiveModuleProcessingComponent::Process()
{
	//check if we have signal to stop the cycle
	if(!m_Stop)
	{
		//check if the timer runed out & subtract time
		if(m_timer.Check())
		{
			if(ShouldProcessActiveCycle())
			{
				//time passed and we can drain cap and make/maintain changes to the attributes
				sLog.Debug("ActiveModuleProcessingComponent", "Cycle finished, processing...");
				ProcessActiveCycle();
			}
			else
			{
				m_Item->SetActive(false, 1253, 0, false);
				m_Stop = true;
			}
		}
	}
	else
	{
		if(m_timer.Check())
		{
			//wait for time to run out and send deactivate to client
			m_timer.Disable();
			m_Item->SetActive(false, 1253, 0, false);
		}
	}
		
}

void ActiveModuleProcessingComponent::ActivateCycle()
{
	m_Stop = false;
	m_timer.Start(m_Mod->GetAttribute(AttrDuration).get_int());
}

void ActiveModuleProcessingComponent::DeactivateCycle()
{
    m_Stop = true;
}

//timing and verification function
bool ActiveModuleProcessingComponent::ShouldProcessActiveCycle()
{
    //check that we have enough capacitor avaiable
	if(m_Ship->GetAttribute(AttrCharge) > m_Mod->GetAttribute(AttrCapacitorNeed))
	{
		//having enough capacitor to activate the module
		return true;
	}
	else
	{
		return false;
	}

    //finally check if we have been told to deactivate
	if(m_Stop)
		return false;
}

void ActiveModuleProcessingComponent::ProcessActiveCycle()
{
	//TO-DO: Check to see if capacitor is drained at activation or after the cycle

    //check for stop signal
    if(m_Stop)
        return;

    //else consume capacitor
	EvilNumber capCapacity = m_Ship->GetAttribute(AttrCharge);
	EvilNumber capNeed = m_Mod->GetAttribute(AttrCapacitorNeed);
	capCapacity -= capNeed;

	m_Ship->SetAttribute(AttrCharge, capCapacity.get_float());

    //then check if we are targeting another ship or not and apply attribute changes
	//maybe we can have a check for modules that repeat the same attributes so we
	//send the changes just once at activation and at deactivation

	//--pseudocode--
	//if(target != self)
	//	m_ShipAttrComp->ModifyTargetShipAttribute();
	//else
	//	m_ShipAttrComp->ModifyShipAttribute();
}

void ActiveModuleProcessingComponent::ProcessDeactivateCycle()
{
	//check to see who is the target
	
	//--pseudocode--
	//if(target != self)
	//	m_ShipAttrComp->ModifyTargetShipAttribute();
	//else
	//	m_ShipAttrComp->ModifyShipAttribute();
}
