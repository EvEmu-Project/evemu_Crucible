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

//GenericModule class definitions
#pragma region GenericModuleClass
GenericModule::GenericModule(InventoryItemRef item, ShipRef ship)
{
	m_Item = item;
	m_Ship = ship;
}

void GenericModule::Process()
{

}

void GenericModule::Online()
{

}

void GenericModule::Offline()
{

}

void GenericModule::Deactivate()
{

}

void GenericModule::Repair()
{

}

void GenericModule::Unload()
{

}


void GenericModule::Overload()
{

}

void GenericModule::DeOverload()
{

}

void GenericModule::Load()
{

}

uint32 GenericModule::itemID()
{
	return m_Item->itemID();
}

EVEItemFlags GenericModule::flag()
{
	return m_Ship->flag();
}
#pragma endregion

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

void ModuleContainer::RemoveModule(EVEItemFlags flag)
{
	GenericModule * mod = GetModule(flag);

}

void ModuleContainer::RemoveModule(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);
}

GenericModule * ModuleContainer::GetModule(EVEItemFlags flag)
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

GenericModule * ModuleContainer::GetModule(uint32 itemID)
{
	//iterate through the list and see if we have it
	uint8 r;
	for(r = 0; r < MAX_HIGH_SLOT_COUNT; r++)
	{
		if(m_HighSlotModules[r]->itemID() == itemID)
			return m_HighSlotModules[r];
	}

	for(r = 0; r < MAX_MEDIUM_SLOT_COUNT; r++)
	{
		if(m_MediumSlotModules[r]->itemID() == itemID)
			return m_MediumSlotModules[r];
	}

	for(r = 0; r < MAX_LOW_SLOT_COUNT; r++)
	{
		if(m_LowSlotModules[r]->itemID() == itemID)
			return m_LowSlotModules[r];
	}

	for(r = 0; r < MAX_ASSEMBLY_COUNT; r++)
	{
		if(m_SubSystemModules[r]->itemID() == itemID)
			return m_SubSystemModules[r];
	}

	for(r = 0; r < MAX_RIG_COUNT; r++)
	{
		if(m_RigModules[r]->itemID() == itemID)
			return m_RigModules[r];
	}

	sLog.Warning("ModuleContainer","Search for itemID: %u yielded no results", itemID);

	return NULL;  //we don't
}

void ModuleContainer::_removeModule(EVEItemFlags flag)
{
	switch(_checkBounds(flag))
	{
	case NaT:					sLog.Error("AddModule","Out of bounds");	break; 
	case slotTypeSubSystem:		_removeSubSystemModule(flag);				break;
	case slotTypeRig:			_removeRigSlotModule(flag);					break;
	case slotTypeLowPower:		_removeLowSlotModule(flag);					break;
	case slotTypeMedPower:		_removeMediumSlotModule(flag);					break;
	case slotTypeHiPower:		_removeHighSlotModule(flag);					break;
	}
}

void ModuleContainer::Process()
{
	_process(typeProcessAll);
}

void ModuleContainer::OnlineAll()
{
	_process(typeOnlineAll);
}

void ModuleContainer::OfflineAll()
{
	_process(typeOfflineAll);
}

void ModuleContainer::DeactivateAll()
{
	_process(typeDeactivateAll);
}

void ModuleContainer::UnloadAll()
{
	_process(typeUnloadAll);
}

void ModuleContainer::_process(processType p)
{
	//high slots
	_processEx(p, highSlot);

	//med slots
	_processEx(p, mediumSlot);

	//low slots
	_processEx(p, lowSlot);
}

void ModuleContainer::_processEx(processType p, slotType t)
{
	uint8 r, COUNT;

	GenericModule **cur = m_HighSlotModules;

	switch(t)
	{
	case highSlot:		COUNT = MAX_HIGH_SLOT_COUNT;		break;
	case mediumSlot:	COUNT = MAX_MEDIUM_SLOT_COUNT;		break;
	case lowSlot:		COUNT = MAX_LOW_SLOT_COUNT;			break;
	//case rigSlot:		COUNT = MAX_RIG_COUNT;				break;  /* not needed now, but possible functionality extension */
	//case subSystemSlot:	COUNT = MAX_ASSEMBLY_COUNT;		break;
	}

	switch(p)
	{
	case typeOnlineAll:
		for(r = 0; r < COUNT; r++, cur++)
		{
			if(*cur == NULL)
				continue;

			(*cur)->Online();
		}
		break;

	case typeOfflineAll:
		for(r = 0; r < COUNT; r++, cur++)
		{
			if(*cur == NULL)
				continue;

			(*cur)->Offline();
		}
		break;

	case typeDeactivateAll:
		for(r = 0; r < COUNT; r++, cur++)
		{
			if(*cur == NULL)
				continue;

			(*cur)->Deactivate();
		}
		break;

	case typeUnloadAll:
		for(r = 0; r < COUNT; r++, cur++)
		{
			if(*cur == NULL)
				continue;

			(*cur)->Unload();
		}
		break;

	case typeProcessAll:
		for(r = 0; r < COUNT; r++, cur++)
		{
			if(*cur == NULL)
				continue;

			(*cur)->Process();
		}
		break;
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

void ModuleContainer::_removeSubSystemModule(uint32 flag)
{
	m_SubSystemModules[flag-flagSubSystem0] = NULL;
}

void ModuleContainer::_removeRigSlotModule(uint32 flag)
{
	m_RigModules[flag-flagRigSlot0] = NULL;
}

void ModuleContainer::_removeLowSlotModule(uint32 flag)
{
	m_LowSlotModules[flag-flagLowSlot0] = NULL;
}

void ModuleContainer::_removeMediumSlotModule(uint32 flag)
{
	m_MediumSlotModules[flag-flagMedSlot0] = NULL;
}

void ModuleContainer::_removeHighSlotModule(uint32 flag)
{
	m_HighSlotModules[flag-flagHiSlot0] = NULL;
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
	memset(m_RigModules, 0, sizeof(m_RigModules));
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
	//module cleanup is handled in the ModuleContainer destructor
}

//necessary function to avoid complications in the 
//ship constructor
void ModuleManager::SetClient(Client * client)
{
	sLog.Debug("SetClient","Received client pointer for %s", client->GetCharacterName());
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
	if(item->groupID() >= 773 && item->groupID() <= 786 && item->groupID() != 783)
		_fitModule(item);
	else
		sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a rig", m_Client->GetName(), item->itemID());
}

void ModuleManager::UninstallRig(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )

}

void ModuleManager::SwapSubSystem(InventoryItemRef item)
{
	if(item->groupID() >= 954 && item->groupID() <= 958)
		_fitModule(item);
	else
		sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Client->GetName(), item->itemID());
}

void ModuleManager::FitModule(InventoryItemRef item)
{
	if(item->categoryID() == EVEItemCategories::Module)
		_fitModule(item);
	else
		sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a module", m_Client->GetName(), item->itemID());
}

void ModuleManager::UnfitModule(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
	{
		mod->Offline();
		m_Modules->RemoveModule(itemID);
	}
}

void ModuleManager::_fitModule(InventoryItemRef item)
{
	GenericModule * mod = new GenericModule(item, m_Client->GetShip());

	m_Modules->AddModule(mod->flag(), mod);
}

void ModuleManager::Online(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
		mod->Online();
}

void ModuleManager::OnlineAll()
{
	m_Modules->OnlineAll();
}

void ModuleManager::Offline(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
		mod->Offline();
}

void ModuleManager::OfflineAll()
{
	m_Modules->OfflineAll();
}

int32 ModuleManager::Activate(uint32 itemID, std::string effectName, int32 targetID, int32 repeat)
{
	sLog.Debug("Activate","Needs to be implemented");
	return 1;
}

void ModuleManager::Deactivate(uint32 itemID, std::string effecetName)
{
	sLog.Debug("Deactivate","Needs to be implemented");
}

void ModuleManager::DeactivateAllModules()
{
	m_Modules->DeactivateAll();
}

void ModuleManager::Overload(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
	{
		mod->Overload();
	}
}

void ModuleManager::DeOverload(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
	{
		mod->DeOverload();
	}
}

void ModuleManager::DamageModule(uint32 itemID)
{
	sLog.Debug("DamageModule","Needs to be implemented");
}

void ModuleManager::RepairModule(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
	{
		mod->Repair();
	}
}

void ModuleManager::ReplaceCharges()
{
	sLog.Debug("ReplaceCharges","Needs to be implemented");
}

void ModuleManager::UnloadAllModules()
{
	m_Modules->UnloadAll();
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