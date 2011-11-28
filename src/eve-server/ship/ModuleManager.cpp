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
	//clean up array objects
	for(int i = 0; i < MAX_HIGH_SLOT_COUNT; i++)
		delete[] m_HighSlotModules[i];

	for(int i = 0; i < MAX_MEDIUM_SLOT_COUNT; i++)
		delete[] m_MediumSlotModules[i];

	for(int i = 0; i < MAX_LOW_SLOT_COUNT; i++)
		delete[] m_LowSlotModules[i];

	for(int i = 0; i < MAX_RIG_COUNT; i++)
		delete[] m_RigModules[i];

	for(int i = 0; i < MAX_ASSEMBLY_COUNT; i++)
		delete[] m_SubSystemModules[i];

	//clean up arrays of module pointers
	delete[] m_LowSlotModules;
	delete[] m_MediumSlotModules;
	delete[] m_HighSlotModules;
	delete[] m_RigModules;
	delete[] m_SubSystemModules;

	//nullify pointers
	m_LowSlotModules = NULL;
	m_MediumSlotModules = NULL;
	m_HighSlotModules = NULL;
	m_RigModules = NULL;
	m_SubSystemModules = NULL;
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

	_removeModule(mod->flag());

	//delete the module
	delete mod;
	mod = NULL;
}

void ModuleContainer::RemoveModule(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);

	_removeModule(mod->flag());

	//delete the module
	delete mod;
	mod = NULL;
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
	case slotTypeMedPower:		_removeMediumSlotModule(flag);				break;
	case slotTypeHiPower:		_removeHighSlotModule(flag);				break;
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

bool ModuleContainer::isHighPower(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);

	return mod->isHighPower();
}

bool ModuleContainer::isMediumPower(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);

	return mod->isMediumPower();
}

bool ModuleContainer::isLowPower(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);

	return mod->isLowPower();
}

bool ModuleContainer::isRig(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);

	return mod->isRig();
}

bool ModuleContainer::isSubSystem(uint32 itemID)
{
	GenericModule * mod = GetModule(itemID);

	return mod->isSubSystem();
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

	GenericModule **cur;

	switch(t)
	{
	case highSlot:		
		COUNT = MAX_HIGH_SLOT_COUNT;
		cur = m_HighSlotModules;
		break;
	case mediumSlot:	
		COUNT = MAX_MEDIUM_SLOT_COUNT;
		cur = m_MediumSlotModules;
		break;
	case lowSlot:		
		COUNT = MAX_LOW_SLOT_COUNT;		
		cur = m_LowSlotModules;
		break;
	case rigSlot:		
		COUNT = MAX_RIG_COUNT;
		cur = m_RigModules;
		break;  
	case subSystemSlot:	
		COUNT = MAX_ASSEMBLY_COUNT;
		cur = m_SubSystemModules;
		break;
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
			if(cur == NULL)
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
		if( (flag - flagLowSlot0) < m_LowSlots )
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
		if( (flag - flagMedSlot0) < m_MediumSlots )
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
		if( (flag - flagHiSlot0) < m_HighSlots )
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
		if( (flag - flagRigSlot0) < m_RigSlots )
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
		if( (flag - flagSubSystem0) < m_SubSystemSlots )
			return true;
		else
			sLog.Error("_isSubSystemSlot", "this shouldn't happen");
	}

	return false;
}

void ModuleContainer::_initializeModuleContainers()
{
	//initialize our arrays of pointers
	m_HighSlotModules = new GenericModule*[MAX_HIGH_SLOT_COUNT];
	m_MediumSlotModules = new GenericModule*[MAX_MEDIUM_SLOT_COUNT];
	m_LowSlotModules = new GenericModule*[MAX_LOW_SLOT_COUNT];
	m_RigModules = new GenericModule*[MAX_RIG_COUNT];
	m_SubSystemModules = new GenericModule*[MAX_ASSEMBLY_COUNT];

	for(int i = 0; i < MAX_HIGH_SLOT_COUNT; i++)
		m_HighSlotModules[i] = NULL;

	for(int i = 0; i < MAX_MEDIUM_SLOT_COUNT; i++)
		m_MediumSlotModules[i] = NULL;

	for(int i = 0; i < MAX_LOW_SLOT_COUNT; i++)
		m_LowSlotModules[i] = NULL;

	for(int i = 0; i < MAX_RIG_COUNT; i++)
		m_RigModules[i] = NULL;

	for( int i = 0; i < MAX_ASSEMBLY_COUNT; i++)
		m_SubSystemModules[i] = NULL;

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
}

ModuleManager::~ModuleManager()
{
	//module cleanup is handled in the ModuleContainer destructor
	delete m_Modules;
	m_Modules = NULL;
}

void ModuleManager::_SendInfoMessage(const char *fmt, ...)
{
    if( m_Ship->GetOperator() == NULL )     // Operator assumed to be Client *
		sLog.Error("SendMessage","message should have been sent to character, but *m_Client is null.  Did you forget to call GetShip()->SetOwner(Client *c)?");
	else
	{
		va_list args;
		va_start(args,fmt);
        m_Ship->GetOperator()->SendNotifyMsg(fmt,args);
		va_end(args);

	}
}

void ModuleManager::_SendErrorMessage(const char *fmt, ...)
{
    if( m_Ship->GetOperator() == NULL )     // Operator assumed to be Client *
		sLog.Error("SendMessage","message should have been sent to character, but *m_Client is null.  Did you forget to call GetShip()->SetOwner(Client *c)?");
	else
	{
		va_list args;
		va_start(args,fmt);
        m_Ship->GetOperator()->SendErrorMsg(fmt,args);
		va_end(args);
	}
}

void ModuleManager::InstallRig(InventoryItemRef item)
{
	if(item->groupID() >= 773 && item->groupID() <= 786 && item->groupID() != 783)
		_fitModule(item);
	else
		sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a rig", m_Ship->GetOperator()->GetName(), item->itemID());
}

void ModuleManager::UninstallRig(uint32 itemID)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
		mod->DestroyRig();
}

void ModuleManager::SwapSubSystem(InventoryItemRef item)
{
	if(item->groupID() >= 954 && item->groupID() <= 958)
		_fitModule(item);
	else
		sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Ship->GetOperator()->GetName(), item->itemID());
}

void ModuleManager::FitModule(InventoryItemRef item)
{
	if(item->categoryID() == EVEDB::invCategories::Module)
		_fitModule(item);
	else
		sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a module", m_Ship->GetOperator()->GetName(), item->itemID());
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
	GenericModule * mod = ModuleFactory(item, m_Ship->GetOperator()->GetShip());

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

int32 ModuleManager::Activate(uint32 itemID, std::string effectName, uint32 targetID, uint32 repeat)
{
	sLog.Debug("Activate","Needs to be implemented");
	return 1;
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL )
	{
		ModuleCommand cmd = _translateEffectName(effectName);
		if(cmd  == OFFLINE)
			mod->Offline();
		//there needs to be more cases here i just don't know what they're called yet
	}
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

void ModuleManager::DamageModule(uint32 itemID, EvilNumber val)
{
	GenericModule * mod = m_Modules->GetModule(itemID);
	if( mod != NULL)
	{
		mod->SetAttribute(AttrHp, val);
	}
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
	//this is complicated and im gonna leave it alone for now until
	//a few things become more clear
}

void ModuleManager::CharacterBoardingShip()
{
	sLog.Debug("CharacterBoardingShip","Needs to be implemented");
	//this is complicated and im gonna leave it alone for now until
	//a few things become more clear
}

void ModuleManager::ShipWarping()
{
	sLog.Debug("ShipWarping","Needs to be implemented");
	//need to remove targets and such
}

void ModuleManager::ShipJumping()
{
	//disable non-warpsafe modules

	//probably should to send a message to the client
}


void ModuleManager::Process()
{
	m_Modules->Process();
}

void ModuleManager::ProcessExternalEffect(Effect * e)
{
	while(e->hasEffect())
	{
		_processExternalEffect(e->next());
	}
}

std::vector<GenericModule *> ModuleManager::GetStackedItems(uint32 typeID, ModulePowerLevel level)
{
	std::vector<GenericModule *> mods;
	GenericModule * tmp;

	switch(level)
	{
	case HIGH_POWER:
		for(int i = flagHiSlot0; i < flagHiSlot7 + 1; i++)
		{
			tmp = m_Modules->GetModule((EVEItemFlags)i);
			if( tmp->typeID() == typeID && tmp->isOnline() )
				mods.push_back(tmp);
		}
		break;
	case MEDIUM_POWER:
		for(int i = flagMedSlot0; i < flagMedSlot7 + 1; i++)
		{
			tmp = m_Modules->GetModule((EVEItemFlags)i);
			if( tmp->typeID() == typeID && tmp->isOnline() )
				mods.push_back(tmp);
		}
		break;
	case LOW_POWER:
		for(int i = flagLowSlot0; i < flagLowSlot7 + 1; i++)
		{
			tmp = m_Modules->GetModule((EVEItemFlags)i);
			if( tmp->typeID() == typeID && tmp->isOnline() )
				mods.push_back(tmp);
		}
		break;
	case RIG:
		for(int i = flagRigSlot0; i < flagRigSlot7 + 1; i++)
		{
			tmp = m_Modules->GetModule((EVEItemFlags)i);
			if( tmp->typeID() == typeID && tmp->isOnline() )
				mods.push_back(tmp);
		}
		break;
	case SUBSYSTEM:
		for(int i = flagSubSystem0; i < flagSubSystem7 + 1; i++)
		{
			tmp = m_Modules->GetModule((EVEItemFlags)i);
			if( tmp->typeID() == typeID && tmp->isOnline() )
				mods.push_back(tmp);
		}
		break;
	}

	return mods;
}

void ModuleManager::_processExternalEffect(SubEffect * s)
{
	//50-50 it's targeting a specific module ( i'm assuming here )
	GenericModule * mod = m_Modules->GetModule(s->TargetItemID());
	if( mod != NULL )
	{
		//calculate new attribute
		mod->SetAttribute(s->AttributeID(), 
			              CalculateNewAttributeValue(mod->GetAttribute(s->AttributeID()),
																	   s->AppliedValue(), s->CalculationType()));
	}
	else if( s->TargetItemID() == m_Ship->itemID() ) //guess it's not, but that means it should be targeting our ship itself
	{
		//calculate new attribute
		m_Ship->SetAttribute(s->AttributeID(), 
			                 CalculateNewAttributeValue(m_Ship->GetAttribute(s->AttributeID()), 
							                                                 s->AppliedValue(), s->CalculationType()));
	}
	else //i have no idea what their targeting X_X
		sLog.Error("ModuleManager", "Process external effect inconsistency.  This shouldn't happen");

}

ModuleCommand ModuleManager::_translateEffectName(std::string s)
{
	//slow but it's better to do it once then many times as it gets passed around in modules or w/e
	//all modules should expect a ModuleCommand instead of a string

	//slightly faster version for when I know what things are really called
	//might as well use, but will definately not be right
	
	switch(s[0])
	{
	case 'A': return ACTIVATE;
	case 'D': return s[2] == 'a' ? DEACTIVATE : DEOVERLOAD;
	case 'O': return s[1] == 'n' ? ONLINE : (s[1] == 'f' ? OFFLINE : OVERLOAD); //compound booleans ftw
	}
	
	return CMD_ERROR;
}

#pragma endregion