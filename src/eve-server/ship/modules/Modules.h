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


#ifndef __MODULES_H__
#define __MODULES_H__

#include "inventory/EffectsEnum.h"
#include "inventory/InventoryItem.h"
#include "inventory/ItemRef.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleDefs.h"
#include "ship/modules/ModuleEffects.h"
#include "ship/ShipOperatorInterface.h"
#include "ship/DestinyManager.h"
#include "ship/ModuleManager.h"

//generic module base class - possibly should inherit from RefPtr...
class GenericModule
{
public:
    GenericModule()
    {
        m_Module_State = MOD_UNFITTED;
        m_Charge_State = MOD_UNLOADED;
    }
    virtual ~GenericModule()
    {
    // ALL DERIVED CLASSES SHOULD OVERRIDE THIS
    //    //warn user - yes be obnoxious
    //    sLog.Error("GenericModule","MEMORY LEAK!");

    //    //force the users to override the inherited destructor
    //    assert(false); //crash if they don't
    }

    virtual void Process()											{ /* Do nothing here */ }
    virtual void Offline()											{ /* Do nothing here */ }
    virtual void Online()											{ /* Do nothing here */ }
	virtual void Activate(uint32 targetID)							{ /* Do nothing here */ }
    virtual void Deactivate()										{ /* Do nothing here */ }
    virtual void Load(InventoryItemRef charge)						{ /* Do nothing here */ }
    virtual void Unload()											{ /* Do nothing here */ }
    virtual void Overload()											{ /* Do nothing here */ }
    virtual void DeOverload()										{ /* Do nothing here */ }
    virtual void DestroyRig()										{ /* Do nothing here */ }


    virtual void Repair()                                        { m_Item->ResetAttribute(AttrHp, true); }
    virtual void Repair(EvilNumber amount)                        { m_Item->SetAttribute(AttrHp, m_Item->GetAttribute(AttrHp) + amount); }

    virtual void SetAttribute(uint32 attrID, EvilNumber val)    { m_Item->SetAttribute(attrID, val); }
    virtual EvilNumber GetAttribute(uint32 attrID)                { return m_Item->GetAttribute(attrID); }

    //access functions
    InventoryItemRef getItem()                                  { return m_Item; }
    virtual uint32 itemID()                                        { return m_Item->itemID(); }
    virtual EVEItemFlags flag()                                    { return m_Item->flag(); }
    virtual uint32 typeID()                                        { return m_Item->typeID(); }
    virtual bool isOnline()                                        { return (m_Item->GetAttribute(AttrIsOnline) == 1); }
    virtual bool isHighPower()                                    { return m_Effects->isHighSlot(); }
    virtual bool isMediumPower()                                { return m_Effects->isMediumSlot(); }
    virtual bool isLowPower()                                    { return m_Effects->isLowSlot(); }

    virtual bool isTurretFitted()
    {
        // Try to make the effect called 'turretFitted' active, if it exists, to test for module being a turret:
        if( m_Effects->HasEffect(Effect_turretFitted) )     // Effect_turretFitted from enum EveAttrEnum::Effect_turretFitted
            return true;
        else
            return false;
    }

    virtual bool isLauncherFitted()
    {
        // Try to make the effect called 'launcherFitted' active, if it exists, to test for module being a launcher:
        if( m_Effects->HasEffect(Effect_launcherFitted) )   // Effect_launcherFitted from enum EveAttrEnum::Effect_launcherFitted
            return true;
        else
            return false;
    }

    virtual bool isMaxGroupFitLimited()
    {
        if( m_Item->HasAttribute(AttrMaxGroupFitted) )  // AttrMaxGroupFitted from enum EveAttrEnum::AttrMaxGroupFitted
            return true;
        else
            return false;
    }

    virtual bool isRig()
    {
        uint32 i = m_Item->categoryID();
        return ( (i >= 773 && i <= 782) || (i == 786) || (i == 787) || (i == 896) || (i == 904) );  //need to use enums, but the enum system is a huge mess
    }

    virtual bool isSubSystem()                                    { return (m_Item->categoryID() == EVEDB::invCategories::Subsystem); }

    //override for rigs and subsystems
    virtual ModulePowerLevel GetModulePowerLevel()                { return isHighPower() ? MODULE_BANK_HIGH_POWER : ( isMediumPower() ? MODULE_BANK_MEDIUM_POWER : MODULE_BANK_LOW_POWER); }

	void SetLog(Basic_Log * pLog) { m_pMM_Log = pLog; }
	Basic_Log * GetLog() { return m_pMM_Log; }

protected:
    InventoryItemRef m_Item;
    ShipRef m_Ship;
    ModuleEffects * m_Effects;

    ModuleStates m_Module_State;
    ChargeStates m_Charge_State;

	Basic_Log * m_pMM_Log;		// We do not own this
};

#endif /* __MODULES_H__ */
