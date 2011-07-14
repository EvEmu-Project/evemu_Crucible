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

GenericModule::GenericModule()
{

}

void GenericModule::Process()
{

}


//ModuleContainer class definitions
#pragma region ModuleContainerClass
ModuleContainer::ModuleContainer(uint32 lowSlots, uint32 medSlots, uint32 highSlots, uint32 rigSlots, uint32 subSystemSlots)
{
	m_LowSlots = lowSlots;
	m_MediumSlots = medSlots;
	m_HighSlots = highSlots;
	m_RigSlots = rigSlots;
	m_SubSystemSlots = subSystemSlots;

	_initializeModuleContainers();
}

ModuleContainer::~ModuleContainer()
{
	//need to clean up the arrays
}

void ModuleContainer::AddModule(uint32 flag, GenericModule * mod)
{
	switch(_checkBounds(flag))
	{
	case NaT:					sLog.Error("AddModule","Out of bounds");	break; 
	case slotTypeSubSystem:		_addSubSystemModule(flag, mod);				break;
	case slotTypeRig:			_addRigModule(flag, mod);					break;
	case slotTypeLowPower:		_addLowSlotModule(flag, mod);				break;
	case slotTypeMedPower:		_addMediumSlotModule(flag, mod);			break;
	case slotTypeHiPower:		_addHighSlotModule(flag, mod);				break;
	}

}

GenericModule * ModuleContainer::GetModule(uint32 flag)
{
	switch(_checkBounds(flag))
	{
	case NaT:					sLog.Error("AddModule","Out of bounds");	break; 
	case slotTypeSubSystem:		return _getSubSystemModule(flag);			break;
	case slotTypeRig:			return _getRigModule(flag);					break;
	case slotTypeLowPower:		return _getLowSlotModule(flag);				break;
	case slotTypeMedPower:		return _getMediumSlotModule(flag);			break;
	case slotTypeHiPower:		return _getHighSlotModule(flag);			break;					
	}
	
	return NULL;
}

void ModuleContainer::Process()
{
	//Process High Slots
	_processHigh();

	//Process Medium Slots
	_processMedium();

	//Process Low Slots
	_processLow();

}

void ModuleContainer::_processHigh()
{
	uint8 r;

	GenericModule **cur = m_HighSlotModules;
	for(r = 0; r < MAX_HIGH_SLOT_COUNT; r++, cur++)
	{
		if(*cur == NULL)
			continue;

		(*cur)->Process();
	}
}

void ModuleContainer::_processMedium()
{
	uint8 r;

	GenericModule **cur = m_MediumSlotModules;
	for(r = 0; r < MAX_MEDIUM_SLOT_COUNT; r++, cur++)
	{
		if(*cur == NULL)
			continue;

		(*cur)->Process();
	}
}

void ModuleContainer::_processLow()
{
	uint8 r;

	GenericModule **cur = m_LowSlotModules;
	for(r = 0; r < MAX_LOW_SLOT_COUNT; r++, cur++)
	{
		if(*cur == NULL)
			continue;

		(*cur)->Process();
	}
}

void ModuleContainer::_addSubSystemModule(uint32 flag, GenericModule * mod)
{
	m_SubSystemModules[flag - flagSubSystem0] = mod;
}

void ModuleContainer::_addRigModule(uint32 flag, GenericModule * mod)
{
	m_RigModules[flag - flagRigSlot0] = mod;
}

void ModuleContainer::_addLowSlotModule(uint32 flag, GenericModule * mod)
{
	m_LowSlotModules[flag - flagLowSlot0] = mod;
}

void ModuleContainer::_addMediumSlotModule(uint32 flag, GenericModule * mod)
{
	m_MediumSlotModules[flag - flagMedSlot0] = mod;
}

void ModuleContainer::_addHighSlotModule(uint32 flag, GenericModule * mod)
{
	m_HighSlotModules[flag - flagHiSlot0] = mod;
}

GenericModule * ModuleContainer::_getSubSystemModule(uint32 flag)
{
	return m_SubSystemModules[flag - flagSubSystem0];
}

GenericModule * ModuleContainer::_getRigModule(uint32 flag)
{
	return m_RigModules[flag - flagRigSlot0];
}

GenericModule * ModuleContainer::_getLowSlotModule(uint32 flag)
{
	return m_LowSlotModules[flag - flagLowSlot0];
}

GenericModule * ModuleContainer::_getMediumSlotModule(uint32 flag)
{
	return m_MediumSlotModules[flag - flagMedSlot0];
}

GenericModule * ModuleContainer::_getHighSlotModule(uint32 flag)
{
	return m_HighSlotModules[flag - flagHiSlot0];
}

//TODO - make this better -Luck
EVEItemSlotType ModuleContainer::_checkBounds(uint32 flag)
{
	//this could be done better
	if( _isLowSlot(flag) )
		return slotTypeLowPower;

	if( _isMediumSlot(flag) )
		return slotTypeMedPower;

	if( _isHighSlot(flag) )
		return slotTypeHiPower;

	//check rigs and subsystems last because they are much less common
	if( _isRigSlot(flag) )
		return slotTypeRig;

	if( _isSubSystemSlot(flag) )
		return slotTypeSubSystem;

	return NaT;  //Not a Type
}

bool ModuleContainer::_isLowSlot(uint32 flag)
{
	if( flag >= flagLowSlot0 && flag <= flagLowSlot7 )
	{
		if( flag < m_LowSlots )
			return true;
		else
			sLog.Error("_isLowSlot", "this shouldn't happen");
	}

	return false;
}

bool ModuleContainer::_isMediumSlot(uint32 flag)
{
	if( flag >= flagMedSlot0 && flag <= flagMedSlot7 )
	{
		if( flag < m_MediumSlots )
			return true;
		else
			sLog.Error("_isMediumSlot", "this shouldn't happen");
	}

	return false;
}

bool ModuleContainer::_isHighSlot(uint32 flag)
{
	if( flag >= flagHiSlot0 && flag <= flagHiSlot7 )
	{
		if( flag < m_HighSlots )
			return true;
		else
			sLog.Error("_isHighSlot", "this shouldn't happen");
	}

	return false;
}

bool ModuleContainer::_isRigSlot(uint32 flag)
{
	if( flag >= flagRigSlot0 && flag <= flagRigSlot7 )
	{
		if( flag < m_RigSlots )
			return true;
		else
			sLog.Error("_isRigSlot", "this shouldn't happen");
	}

	return false;
}

bool ModuleContainer::_isSubSystemSlot(uint32 flag)
{
	if( flag >= flagSubSystem0 && flag <= flagSubSystem7 )
	{
		if( flag < m_SubSystemSlots )
			return true;
		else
			sLog.Error("_isSubSystemSlot", "this shouldn't happen");
	}

	return false;
}

void ModuleContainer::_initializeModuleContainers()
{
	memset(m_HighSlotModules, 0, sizeof(m_HighSlotModules));
	memset(m_MediumSlotModules, 0, sizeof(m_MediumSlotModules));
	memset(m_LowSlotModules, 0, sizeof(m_LowSlotModules));
}
#pragma endregion

//ModuleManager class definitions
#pragma region ModuleManagerClass
ModuleManager::ModuleManager(Ship *const ship)
{
	m_Modules = new ModuleContainer((uint32)ship->GetAttribute(AttrLowSlots).get_int(),
									(uint32)ship->GetAttribute(AttrMedSlots).get_int(),
									(uint32)ship->GetAttribute(AttrHiSlots).get_int(),
									(uint32)ship->GetAttribute(AttrRigSlots).get_int(),
									(uint32)ship->GetAttribute(AttrSubSystemSlot).get_int());

	m_Ship = ship;
	m_Client = NULL; //this is null until the ship is picked up by someone
}

ModuleManager::~ModuleManager()
{
	//TODO: cleanup
}

//necessary function to avoid complications in the 
//ship constructor
void ModuleManager::SetClient(Client * client)
{
	sLog.Debug("SetClient","Recieved client pointer for %s", client->GetCharacterName());
	m_Client = client;
}

void ModuleManager::_SendInfoMessage(const char *fmt, ...)
{
	if( m_Client == NULL )
		sLog.Error("SendMessage","message should have been sent to character, but *m_Client is null.  Did you forget to call GetShip()->SetOwner(Client *c)?");
	else
	{
		va_list args;
		va_start(args,fmt);
		m_Client->SendNotifyMsg(fmt,args);
		va_end(args);

	}
}

void ModuleManager::_SendErrorMessage(const char *fmt, ...)
{
	if( m_Client == NULL )
		sLog.Error("SendMessage","message should have been sent to character, but *m_Client is null.  Did you forget to call GetShip()->SetOwner(Client *c)?");
	else
	{
		va_list args;
		va_start(args,fmt);
		m_Client->SendErrorMsg(fmt,args);
		va_end(args);
	}
}

void ModuleManager::InstallRig(InventoryItemRef item)
{
	sLog.Debug("InstallRig","Needs to be implemented");
}

void ModuleManager::UninstallRig(uint32 itemID)
{
	sLog.Debug("UninstallRig","Needs to be implemented");
}

void ModuleManager::SwapSubSystem()
{
	sLog.Debug("SwapSubSystem","Needs to be implemented");
}

void ModuleManager::FitModule(InventoryItemRef item)
{
	sLog.Debug("FitModule","Needs to be implemented");
}

void ModuleManager::UnfitModule(uint32 itemID)
{
	sLog.Debug("UnfitModule","Needs to be implemented");
}

void ModuleManager::Online(uint32 itemID)
{
	sLog.Debug("Online","Needs to be implemented");
}

void ModuleManager::OnlineAll()
{
	sLog.Debug("OnlineAll","Needs to be implemented");
}

void ModuleManager::Offline(uint32 itemID)
{
	sLog.Debug("Offline","Needs to be implemented");
}

void ModuleManager::OfflineAll()
{
	sLog.Debug("OfflineAll","Needs to be implemented");
}

int32 ModuleManager::Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat)
{
	sLog.Debug("Activate","Needs to be implemented");
	return 1;
}

void ModuleManager::Deactivate(int32 itemID, std::string effecetName)
{
	sLog.Debug("Deactivate","Needs to be implemented");
}

void ModuleManager::DeactivateAllModules()
{
	sLog.Debug("DeactivateAllModules","Needs to be implemented");
}

void ModuleManager::Overload()
{
	sLog.Debug("Overload","Needs to be implemented");
}

void ModuleManager::DeOverload()
{
	sLog.Debug("DeOverload","Needs to be implemented");
}

void ModuleManager::DamageModule(uint32 itemID)
{
	sLog.Debug("DamageModule","Needs to be implemented");
}

void ModuleManager::RepairModule(uint32 itemID)
{
	sLog.Debug("RepairModule","Needs to be implemented");
}

void ModuleManager::ReplaceCharges()
{
	sLog.Debug("ReplaceCharges","Needs to be implemented");
}

void ModuleManager::UnloadAllModules()
{
	sLog.Debug("UnloadAllModules","Needs to be implemented");
}

void ModuleManager::CharacterLeavingShip()
{
	sLog.Debug("CharacterLeavingShip","Needs to be implemented");
}

void ModuleManager::CharacterBoardingShip()
{
	sLog.Debug("CharacterBoardingShip","Needs to be implemented");
}

void ModuleManager::ShipWarping()
{
	sLog.Debug("ShipWarping","Needs to be implemented");
}

void ModuleManager::ShipJumping()
{
	sLog.Debug("ShipJumping","Needs to be implemented");
}

void ModuleManager::Process()
{
	m_Modules->Process();
}


#pragma endregion