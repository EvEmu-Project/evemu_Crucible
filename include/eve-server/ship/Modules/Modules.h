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


#ifndef MODULES_H
#define MODULES_H

#include "ship/Modules/ModuleDefs.h"
#include "ship/Modules/ModuleEffects.h"

//generic module base class - possibly should inherit from RefPtr...

class GenericModule
{
public:
	GenericModule() { }
	virtual ~GenericModule() { }

	virtual void Process() { }
	virtual void Offline() { }
	virtual void Online() { }
	virtual void Activate() { }
	virtual void Deactivate() { }
	virtual void Load() { }
	virtual void Unload() { } 
	virtual void Overload() { }
	virtual void DeOverload() { }
	virtual void DestroyRig() { }


	virtual void Repair()										{ m_Item->ResetAttribute(AttrHp, true); }
	virtual void Repair(EvilNumber amount)						{ m_Item->SetAttribute(AttrHp, m_Item->GetAttribute(AttrHp) + amount); }

	virtual void SetAttribute(uint32 attrID, EvilNumber val)	{ m_Item->SetAttribute(attrID, val); }
	virtual EvilNumber GetAttribute(uint32 attrID)				{ return m_Item->GetAttribute(attrID); }

	//access functions
	virtual uint32 itemID()										{ return m_Item->itemID(); }
	virtual EVEItemFlags flag()									{ return m_Item->flag(); }
	virtual uint32 typeID()										{ return m_Item->typeID(); }
	virtual bool isOnline()										{ return (m_Item->GetAttribute(AttrIsOnline) == 1); }
	bool isHighPower()											{ return m_Effects->isHighSlot(); }
	bool isMediumPower()										{ return m_Effects->isMediumSlot(); }
	bool isLowPower()											{ return m_Effects->isLowSlot(); }
	bool isRig() 
	{
		uint32 i = m_Item->categoryID();
		return ( (i >= 773 && i <= 782) || (i == 786) || (i == 787) || (i == 896) || (i == 904) );
	}
	bool isSubSystem()											{ return (m_Item->categoryID() == EVEItemCategories::Subsystem); }

	//override for rigs and subsystems
	virtual ModulePowerLevel GetModulePowerLevel()				{ return isHighPower() ? HIGH_POWER : ( isMediumPower() ? MEDIUM_POWER : LOW_POWER); }  

protected:
	InventoryItemRef m_Item;
	ShipRef m_Ship;
	ModuleEffects * m_Effects;

};

#endif /* MODULES_H */