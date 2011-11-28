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


#ifndef __MODULES_H__
#define __MODULES_H__

#include "ship/Modules/ModuleDefs.h"
#include "ship/Modules/ModuleEffects.h"


//generic module base class - possibly should inherit from RefPtr...

class GenericModule
{
public:
	GenericModule() { }
	virtual ~GenericModule() 
	{
		//warn user - yes be obnoxious
		sLog.Error("GenericModule","MEMORY LEAK!");

		//force the users to override the inherited destructor
		assert(false); //crash if they don't
	}

	virtual void Process()										{ /* Do nothing here */ }
	virtual void Offline()										{ /* Do nothing here */ }
	virtual void Online()										{ /* Do nothing here */ }
	virtual void Activate(uint32 targetID)						{ /* Do nothing here */ }
	virtual void Deactivate()									{ /* Do nothing here */ }
	virtual void Load(InventoryItemRef charge)					{ /* Do nothing here */ }
	virtual void Unload()										{ /* Do nothing here */ } 
	virtual void Overload()										{ /* Do nothing here */ }
	virtual void DeOverload()									{ /* Do nothing here */ }
	virtual void DestroyRig()									{ /* Do nothing here */ }


	virtual void Repair()										{ m_Item->ResetAttribute(AttrHp, true); }
	virtual void Repair(EvilNumber amount)						{ m_Item->SetAttribute(AttrHp, m_Item->GetAttribute(AttrHp) + amount); }

	virtual void SetAttribute(uint32 attrID, EvilNumber val)	{ m_Item->SetAttribute(attrID, val); }
	virtual EvilNumber GetAttribute(uint32 attrID)				{ return m_Item->GetAttribute(attrID); }

	//access functions
	virtual uint32 itemID()										{ return m_Item->itemID(); }
	virtual EVEItemFlags flag()									{ return m_Item->flag(); }
	virtual uint32 typeID()										{ return m_Item->typeID(); }
	virtual bool isOnline()										{ return (m_Item->GetAttribute(AttrIsOnline) == 1); }
	virtual bool isHighPower()									{ return m_Effects->isHighSlot(); }
	virtual bool isMediumPower()								{ return m_Effects->isMediumSlot(); }
	virtual bool isLowPower()									{ return m_Effects->isLowSlot(); }

	virtual bool isRig() 
	{
		uint32 i = m_Item->categoryID();
		return ( (i >= 773 && i <= 782) || (i == 786) || (i == 787) || (i == 896) || (i == 904) );  //need to use enums, but the enum system is a huge mess
	}

	virtual bool isSubSystem()									{ return (m_Item->categoryID() == EVEDB::invCategories::Subsystem); }

	//override for rigs and subsystems
	virtual ModulePowerLevel GetModulePowerLevel()				{ return isHighPower() ? HIGH_POWER : ( isMediumPower() ? MEDIUM_POWER : LOW_POWER); }  

protected:
	InventoryItemRef m_Item;
	ShipRef m_Ship;
	ModuleEffects * m_Effects;

};

#endif /* __MODULES_H__ */