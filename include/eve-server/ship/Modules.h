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

class InventoryItem;
class SystemEntity;
class Client;


//Module inheritance definition
#pragma region Modules
//generic module base class - possibly should inherit from RefPtr...
class GenericModule
{
public:
	GenericModule(InventoryItemRef item, ShipRef ship);
	~GenericModule();

	virtual void Process();
	virtual void Offline();
	virtual void Online();
	virtual void Deactivate();
	virtual void Load();
	virtual void Unload();
	virtual void Repair();
	virtual void Overload();
	virtual void DeOverload();
	virtual void DestroyRig();

	virtual void SetAttribute(uint32 attrID, EvilNumber val);
	virtual EvilNumber GetAttribute(uint32 attrID);

	EvilNumber CalculateNewAttributeValue(EvilNumber attrVal, EvilNumber attrMod, EVECalculationType type);

	//access functions
	uint32 itemID();
	EVEItemFlags flag();


private:
	InventoryItemRef m_Item;
	ShipRef m_Ship;

};

class PassiveModule : public GenericModule
{

};

class RigModule : public PassiveModule
{

};

class SubSystemModule : public PassiveModule
{

};

#pragma endregion

#endif /* MODULES_H */