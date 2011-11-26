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


#ifndef __MODULEMANAGER_H_INCL__
#define __MODULEMANAGER_H_INCL__

class InventoryItem;
class GenericModule;
class SystemEntity;
class Client;

#include "ship/Modules/Modules.h"
#include "ship/Modules/ModuleDefs.h"


//container for all ships modules
#pragma region ModuleContainer
class ModuleContainer
{
public:
	ModuleContainer(uint32 lowSlots, uint32 medSlots, uint32 highSlots, uint32 rigSlots, uint32 subSystemSlots);
	~ModuleContainer();

	void AddModule(uint32 flag, GenericModule * mod);
	void RemoveModule(EVEItemFlags flag);
	void RemoveModule(uint32 itemID);
	GenericModule * GetModule(EVEItemFlags flag);
	GenericModule * GetModule(uint32 itemID);

	int NumberOfSameType(uint32 typeID);

	//batch processes handlers
	void Process();
	void OfflineAll();
	void OnlineAll();
	void DeactivateAll();
	void UnloadAll();

	//useful accessors
	bool isHighPower(uint32 itemID);
	bool isMediumPower(uint32 itemID);
	bool isLowPower(uint32 itemID);
	bool isRig(uint32 itemID);
	bool isSubSystem(uint32 itemID);

private:

	//internal enums
	enum processType
	{
		typeOnlineAll,
		typeOfflineAll,
		typeDeactivateAll,
		typeUnloadAll,
		typeProcessAll
	};

	enum slotType
	{
		highSlot,
		mediumSlot,
		lowSlot,
		rigSlot,
		subSystemSlot
	};

	void _removeModule(EVEItemFlags flag);

	void _process(processType p);
	void _processEx(processType p, slotType t);

	void _addHighSlotModule(uint32 flag, GenericModule * mod);
	void _addMediumSlotModule(uint32 flag, GenericModule * mod);
	void _addLowSlotModule(uint32 flag, GenericModule * mod);
	void _addRigModule(uint32 flag, GenericModule * mod);
	void _addSubSystemModule(uint32 flag, GenericModule * mod);

	GenericModule * _getHighSlotModule(uint32 flag);
	GenericModule * _getMediumSlotModule(uint32 flag);
	GenericModule * _getLowSlotModule(uint32 flag);
	GenericModule * _getRigModule(uint32 flag);
	GenericModule * _getSubSystemModule(uint32 flag);

	void _removeHighSlotModule(uint32 flag);
	void _removeMediumSlotModule(uint32 flag);
	void _removeLowSlotModule(uint32 flag);
	void _removeRigSlotModule(uint32 flag);
	void _removeSubSystemModule(uint32 flag);

	EVEItemSlotType _checkBounds(uint32 flag);

	bool _isLowSlot(uint32 flag);
	bool _isMediumSlot(uint32 flag);
	bool _isHighSlot(uint32 flag);
	bool _isRigSlot(uint32 flag);
	bool _isSubSystemSlot(uint32 flag);

	void _initializeModuleContainers();

	//we own these
	GenericModule ** m_HighSlotModules;
	GenericModule ** m_MediumSlotModules;
	GenericModule ** m_LowSlotModules;
	GenericModule ** m_RigModules;
	GenericModule ** m_SubSystemModules;

	uint32 m_LowSlots;
	uint32 m_MediumSlots;
	uint32 m_HighSlots;
	uint32 m_RigSlots;
	uint32 m_SubSystemSlots;

};

#pragma endregion

//classes for passing effects around to targets
#pragma region Effect Passing

static const uint8 MAX_EFFECT_COUNT = 5;  //arbitrary, lazy etc.  The bigger this number, the larger these message classes will be

class SubEffect
{
public:

	SubEffect(uint32 attrID, EVECalculationType type, EvilNumber val, uint32 targetItemID = 0)
	: m_AttrID( attrID ), m_TargetItemID( targetItemID ), m_CalcType( type ), m_Val( val )
	{

	}

	~SubEffect() { }

	//gets
	uint32 AttributeID()                    { return m_AttrID; }
	uint32 TargetItemID()                   { return m_TargetItemID; }
	EVECalculationType CalculationType()    { return m_CalcType; }
	EvilNumber AppliedValue()               { return m_Val; }

private:
	uint32 m_AttrID;
	uint32 m_TargetItemID;
	EVECalculationType m_CalcType;
	EvilNumber m_Val;

};



class Effect
{
public:
	Effect()
	: m_Count( 0 )
	{

	}

	~Effect()
	{
		delete[] m_SubEffects;
	}

	void AddEffect(uint32 attributeID, EVECalculationType type, EvilNumber val, uint32 targetItemID = 0)
	{
		SubEffect * s = new SubEffect(attributeID, type, val, targetItemID);
		if( m_Count + 1 < MAX_EFFECT_COUNT )
		{
			m_SubEffects[m_Count] = s;
			m_Count++;
		}
	}

	bool hasEffect() { return (m_Count > 0);  }

	SubEffect * next()
	{
		m_Count--;
		return m_SubEffects[m_Count];

	}

private:
	SubEffect * m_SubEffects[MAX_EFFECT_COUNT];
	int m_Count;

};

#pragma endregion

//Primary Module Manager
#pragma region ModuleManager

class ModuleManager
{
public:
	ModuleManager(Ship *const ship);
	~ModuleManager();

	void InstallRig(InventoryItemRef item);
	void UninstallRig(uint32 itemID);
	void InstallSubSystem(InventoryItemRef item);
	void SwapSubSystem(InventoryItemRef item);
	void FitModule(InventoryItemRef item);
	void UnfitModule(uint32 itemID);
	void Online(uint32 itemID);
	void OnlineAll();
	void Offline(uint32 itemID);
	void OfflineAll();
	int32 Activate(uint32 itemID, std::string effectName, uint32 targetID, uint32 repeat);
	void Deactivate(uint32 itemID, std::string effecetName);
	void DeactivateAllModules();
	void Overload(uint32 itemID);
	void DeOverload(uint32 itemID);
	void DamageModule(uint32 itemID, EvilNumber val);
	void RepairModule(uint32 itemID);
	void ReplaceCharges();
	void UnloadAllModules();
	void CharacterLeavingShip();
	void CharacterBoardingShip();
	void ShipWarping();
	void ShipJumping();
	void Process();
	void ProcessExternalEffect(Effect * e);
	std::vector<GenericModule *> GetStackedItems(uint32 typeID, ModulePowerLevel level);  //should only be used by components

private:
	void _fitModule(InventoryItemRef item);

	void _processExternalEffect(SubEffect * e);

	ModuleCommand _translateEffectName(std::string s);


	void _SendInfoMessage(const char* fmt, ...);
	void _SendErrorMessage(const char* fmt, ...);

	//access to the ship that owns us.  We do not own this
	Ship * m_Ship;

	//access to destiny.  We do not own this
	DestinyManager * m_Destiny;

	//modules storage, we own this
	ModuleContainer * m_Modules;


};

#pragma endregion

#endif  /* MODULE_MANAGER_H */


