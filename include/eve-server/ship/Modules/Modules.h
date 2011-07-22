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

	virtual void Process() = 0;
	virtual void Offline() = 0;
	virtual void Online() = 0;
	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
	virtual void Load() = 0;
	virtual void Unload() = 0;
	virtual void Repair() = 0;
	virtual void Overload() = 0;
	virtual void DeOverload() = 0;
	virtual void DestroyRig() = 0;

	virtual void SetAttribute(uint32 attrID, EvilNumber val) = 0;
	virtual EvilNumber GetAttribute(uint32 attrID) = 0;

	//access functions
	virtual uint32 itemID() = 0;
	virtual EVEItemFlags flag() = 0;
	virtual uint32 typeID() = 0;
	virtual bool isHighPower() = 0;
	virtual bool isMediumPower() = 0;
	virtual bool isLowPower() = 0;
	virtual bool isRig() = 0;
	virtual bool isSubSystem() = 0;

protected:
	InventoryItemRef m_Item;
	ShipRef m_Ship;
	ModuleEffects * m_Effects;

};

#endif /* MODULES_H */