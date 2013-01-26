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

#ifndef ACTIVE_MODULES_H
#define ACTIVE_MODULES_H

#include "Modules.h"
#include "ship/modules/components/ModifyShipAttributesComponent.h"
#include "ship/modules/components/ActiveModuleProcessingComponent.h"

class ActiveModule : public GenericModule
{
public:
    ActiveModule(InventoryItemRef item, ShipRef ship);
    ~ActiveModule();

	virtual void Process()						{/*do nothing*/}
    void Offline();
    void Online();
	void Activate(uint32 targetID);
    void Deactivate();

    //access functions
    ModulePowerLevel GetModulePowerLevel()                    { return isHighPower() ? MODULE_BANK_HIGH_POWER : ( isMediumPower() ? MODULE_BANK_MEDIUM_POWER : MODULE_BANK_LOW_POWER); }

    bool isHighPower()                                        { return m_Effects->isHighSlot(); }
    bool isMediumPower()                                    { return m_Effects->isMediumSlot(); }
    bool isLowPower()                                        { return m_Effects->isLowSlot(); }
    bool isRig()                                            { return false; }
    bool isSubSystem()                                        { return false; }
    bool requiresTarget()
    {
        if( m_Effects->HasDefaultEffect() )
            return m_Effects->GetDefaultEffect()->GetIsAssistance() || m_Effects->GetDefaultEffect()->GetIsOffensive();
        else
            return false;
    }

protected:
    ModifyShipAttributesComponent * m_ShipAttrComp;
	ActiveModuleProcessingComponent * m_ActiveModuleProc;
    uint32 targetID;  //passed to us by activate

    //inheritance crap
    ActiveModule() {}
};


#endif