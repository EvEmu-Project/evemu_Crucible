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

#include "eve-server.h"

#include "PyCallable.h"
#include "ship/Ship.h"
#include "ship/ModuleManager.h"
#include "ship/ShipOperatorInterface.h"
#include "ship/modules/ModuleFactory.h"


//////////////////////////////////////////////////////////////////////////////////
// Modifier class definitions
#pragma region Modifier

// No functions defined here at this time.

#pragma endregion
/////////////////////////////// END MODIFIER /////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// ModuleContainer class definitions
#pragma region ModuleContainerClass
ModuleContainer::ModuleContainer(uint32 lowSlots, uint32 medSlots, uint32 highSlots, uint32 rigSlots, uint32 subSystemSlots,
    uint32 turretSlots, uint32 launcherSlots, ModuleManager * myManager)
{
    m_LowSlots = lowSlots;
    m_MediumSlots = medSlots;
    m_HighSlots = highSlots;
    m_RigSlots = rigSlots;
    m_SubSystemSlots = subSystemSlots;
    m_TurretSlots = turretSlots;
    m_LauncherSlots = launcherSlots;

    _initializeModuleContainers();

    m_TotalTurretsFitted = 0;
    m_TotalLaunchersFitted = 0;

    m_MyManager = myManager;
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
        case NaT:
            sLog.Error("ModuleContainer::AddModule()","Flag Out of bounds");
            break;
        case slotTypeSubSystem:
            m_SubSystemModules[flag - flagSubSystem0] = mod;
            break;
        case slotTypeRig:
            m_RigModules[flag - flagRigSlot0] = mod;
            break;
        case slotTypeLowPower:
            m_LowSlotModules[flag - flagLowSlot0] = mod;
            break;
        case slotTypeMedPower:
            m_MediumSlotModules[flag - flagMedSlot0] = mod;
            break;
        case slotTypeHiPower:
            m_HighSlotModules[flag - flagHiSlot0] = mod;
            break;
    }

    // Maintain Turret and Launcher Fitted module counts:
    if( mod->isTurretFitted() )
        m_TotalTurretsFitted++;
    if( mod->isLauncherFitted() )
        m_TotalLaunchersFitted++;

    // Maintain the Modules Fitted By Group counter for this module group:
    if( m_ModulesFittedByGroupID.find(mod->getItem()->groupID()) != m_ModulesFittedByGroupID.end() )
        m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second += 1;
    else
        m_ModulesFittedByGroupID.insert(std::pair<uint32,uint32>(mod->getItem()->groupID(), 1));
}

void ModuleContainer::RemoveModule(EVEItemFlags flag)
{
    GenericModule * mod = GetModule(flag);

    _removeModule(mod->flag(), mod);

    //delete the module
    delete mod;
    mod = NULL;
}

void ModuleContainer::RemoveModule(uint32 itemID)
{
    GenericModule * mod = GetModule(itemID);

    _removeModule(mod->flag(), mod);

    //delete the module
    delete mod;
    mod = NULL;
}

GenericModule * ModuleContainer::GetModule(EVEItemFlags flag)
{
    switch(_checkBounds(flag))
    {
        case NaT:
            sLog.Error("ModuleContainer::AddModule()","Flag Out of bounds");
            break;
        case slotTypeSubSystem:
            return m_SubSystemModules[flag - flagSubSystem0];
            break;
        case slotTypeRig:
            return m_RigModules[flag - flagRigSlot0];
            break;
        case slotTypeLowPower:
            return m_LowSlotModules[flag - flagLowSlot0];
            break;
        case slotTypeMedPower:
            return m_MediumSlotModules[flag - flagMedSlot0];
            break;
        case slotTypeHiPower:
            return m_HighSlotModules[flag - flagHiSlot0];
            break;
    }

    return NULL;
}

GenericModule * ModuleContainer::GetModule(uint32 itemID)
{
    //iterate through the list and see if we have it
    uint8 r;
    for(r = 0; r < MAX_HIGH_SLOT_COUNT; r++)
    {
        if( !(m_HighSlotModules[r] == NULL) )
            if(m_HighSlotModules[r]->itemID() == itemID)
                return m_HighSlotModules[r];
    }

    for(r = 0; r < MAX_MEDIUM_SLOT_COUNT; r++)
    {
        if( !(m_MediumSlotModules[r] == NULL) )
            if(m_MediumSlotModules[r]->itemID() == itemID)
                return m_MediumSlotModules[r];
    }

    for(r = 0; r < MAX_LOW_SLOT_COUNT; r++)
    {
        if( !(m_LowSlotModules[r] == NULL) )
            if(m_LowSlotModules[r]->itemID() == itemID)
                return m_LowSlotModules[r];
    }

    for(r = 0; r < MAX_ASSEMBLY_COUNT; r++)
    {
        if( !(m_SubSystemModules[r] == NULL) )
            if(m_SubSystemModules[r]->itemID() == itemID)
                return m_SubSystemModules[r];
    }

    for(r = 0; r < MAX_RIG_COUNT; r++)
    {
        if( !(m_RigModules[r] == NULL) )
            if(m_RigModules[r]->itemID() == itemID)
                return m_RigModules[r];
    }

    sLog.Warning("ModuleContainer::GetModule()","Search for itemID: %u yielded no results", itemID);

    return NULL;  //we don't
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

uint32 ModuleContainer::GetFittedModuleCountByGroup(uint32 groupID)
{
    uint32 moduleCountByGroup = 0;
    if( m_ModulesFittedByGroupID.find(groupID) == m_ModulesFittedByGroupID.end() )
        return 0;
    else
        return m_ModulesFittedByGroupID.find(groupID)->second;
}

void ModuleContainer::SaveModules()
{
    uint8 r;
    for(r = 0; r < MAX_HIGH_SLOT_COUNT; r++)
    {
        if( !(m_HighSlotModules[r] == NULL) )
            m_HighSlotModules[r]->getItem()->SaveItem();
    }

    for(r = 0; r < MAX_MEDIUM_SLOT_COUNT; r++)
    {
        if( !(m_MediumSlotModules[r] == NULL) )
            m_MediumSlotModules[r]->getItem()->SaveItem();
    }

    for(r = 0; r < MAX_LOW_SLOT_COUNT; r++)
    {
        if( !(m_LowSlotModules[r] == NULL) )
            m_LowSlotModules[r]->getItem()->SaveItem();
    }

    for(r = 0; r < MAX_ASSEMBLY_COUNT; r++)
    {
        if( !(m_SubSystemModules[r] == NULL) )
            m_SubSystemModules[r]->getItem()->SaveItem();
    }

    for(r = 0; r < MAX_RIG_COUNT; r++)
    {
        if( !(m_RigModules[r] == NULL) )
            m_RigModules[r]->getItem()->SaveItem();
    }

}

void ModuleContainer::_removeModule(EVEItemFlags flag, GenericModule * mod)
{
    switch(_checkBounds(flag))
    {
    case NaT:
        sLog.Error("ModuleContainer::_removeModule()","Flag Out of bounds");
        break;
    case slotTypeSubSystem:
        m_SubSystemModules[flag-flagSubSystem0] = NULL;
        break;
    case slotTypeRig:
        m_RigModules[flag-flagRigSlot0] = NULL;
        break;
    case slotTypeLowPower:
        m_LowSlotModules[flag-flagLowSlot0] = NULL;
        break;
    case slotTypeMedPower:
        m_MediumSlotModules[flag-flagMedSlot0] = NULL;
        break;
    case slotTypeHiPower:
        m_HighSlotModules[flag-flagHiSlot0] = NULL;
        break;
    }

    // Maintain Turret and Launcher Fitted module counts:
    if( mod->isTurretFitted() )
        m_TotalTurretsFitted--;
    if( mod->isLauncherFitted() )
        m_TotalLaunchersFitted--;

    // Maintain the Modules Fitted By Group counter for this module group:
    if( m_ModulesFittedByGroupID.find(mod->getItem()->groupID()) != m_ModulesFittedByGroupID.end() )
    {
        uint32 moduleCount = 0;
        if( (moduleCount = m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second) > 1)
            // We still have more than one module of this group fitted, so just reduce number fitted by 1:
            m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second -= 1;
        else
            // This was the last module of this group fitted, so remove the entry from the map:
            m_ModulesFittedByGroupID.erase(mod->getItem()->groupID());
    }
    else
        sLog.Error( "ModuleContainer::_removeModule()", "Removing Module from ship fit when it had NO entry in m_ModulesFittedByGroup !" );
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
            sLog.Error("ModuleContainer::_isLowSlot()", "this shouldn't happen");
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
            sLog.Error("ModuleContainer::_isMediumSlot()", "this shouldn't happen");
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
            sLog.Error("ModuleContainer::_isHighSlot()", "this shouldn't happen");
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
            sLog.Error("ModuleContainer::_isRigSlot()", "this shouldn't happen");
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
            sLog.Error("ModuleContainer::_isSubSystemSlot()", "this shouldn't happen");
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
/////////////////////////// END MODULECONTAINER //////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// ModuleManager class definitions
#pragma region ModuleManagerClass
ModuleManager::ModuleManager(Ship *const ship)
{
    // Create ModuleContainer object and initialize with sizes for all slot banks for this ship:
    m_Modules = new ModuleContainer((uint32)ship->GetAttribute(AttrLowSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrMedSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrHiSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrRigSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrSubSystemSlot).get_int(),
                                    (uint32)ship->GetAttribute(AttrTurretSlotsLeft).get_int(),
                                    (uint32)ship->GetAttribute(AttrLauncherSlotsLeft).get_int(),
                                    this);

    // Store reference to the Ship object to which the ModuleManager belongs:
    m_Ship = ship;

    // Load modules, rigs and subsystems from Ship's inventory into ModuleContainer:
    uint32 flagIndex;
    for(flagIndex=flagLowSlot0; flagIndex<=flagLowSlot7; flagIndex++)
    {
        InventoryItemRef itemRef;
        m_Ship->FindSingleByFlag( (EVEItemFlags)flagIndex, itemRef );
        if( !(itemRef == NULL) )
            _fitModule( itemRef, (EVEItemFlags)flagIndex );
    }

    for(flagIndex=flagMedSlot0; flagIndex<=flagMedSlot7; flagIndex++)
    {
        InventoryItemRef itemRef;
        m_Ship->FindSingleByFlag( (EVEItemFlags)flagIndex, itemRef );
        if( !(itemRef == NULL) )
            _fitModule( itemRef, (EVEItemFlags)flagIndex );
    }

    for(flagIndex=flagHiSlot0; flagIndex<=flagHiSlot7; flagIndex++)
    {
        InventoryItemRef itemRef;
        m_Ship->FindSingleByFlag( (EVEItemFlags)flagIndex, itemRef );
        if( !(itemRef == NULL) )
            _fitModule( itemRef, (EVEItemFlags)flagIndex );
    }

    for(flagIndex=flagRigSlot0; flagIndex<=flagRigSlot7; flagIndex++)
    {
        InventoryItemRef itemRef;
        m_Ship->FindSingleByFlag( (EVEItemFlags)flagIndex, itemRef );
        if( !(itemRef == NULL) )
            _fitModule( itemRef, (EVEItemFlags)flagIndex );
    }

    for(flagIndex=flagSubSystem0; flagIndex<=flagSubSystem7; flagIndex++)
    {
        InventoryItemRef itemRef;
        m_Ship->FindSingleByFlag( (EVEItemFlags)flagIndex, itemRef );
        if( !(itemRef == NULL) )
            _fitModule( itemRef, (EVEItemFlags)flagIndex );
    }

    //modifier maps, we own these
    m_LocalSubsystemModifierMaps = new ModifierMaps;
    m_LocalShipSkillModifierMaps = new ModifierMaps;
    m_LocalModuleRigModifierMaps = new ModifierMaps;
    m_LocalImplantModifierMaps = new ModifierMaps;
    m_RemoteModifierMaps = new ModifierMaps;
}

ModuleManager::~ModuleManager()
{
    //module cleanup is handled in the ModuleContainer destructor
    delete m_Modules;
    m_Modules = NULL;

    //modifier map cleanup is handled in the std::map destructor
    delete m_LocalSubsystemModifierMaps;
    delete m_LocalShipSkillModifierMaps;
    delete m_LocalModuleRigModifierMaps;
    delete m_LocalImplantModifierMaps;
    delete m_RemoteModifierMaps;
    m_LocalSubsystemModifierMaps = NULL;
    m_LocalShipSkillModifierMaps = NULL;
    m_LocalModuleRigModifierMaps = NULL;
    m_LocalImplantModifierMaps = NULL;
    m_RemoteModifierMaps = NULL;
}

bool ModuleManager::IsSlotOccupied(uint32 flag)
{
    if( this->m_Modules->GetModule(EVEItemFlags(flag)) == NULL )
        return false;
    else
        return true;
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

bool ModuleManager::InstallRig(InventoryItemRef item, EVEItemFlags flag)
{
    if(item->groupID() >= 773 && item->groupID() <= 786 && item->groupID() != 783)
    {
        _fitModule(item,flag);
        return true;
    }
    else
        sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a rig", m_Ship->GetOperator()->GetName(), item->itemID());

    return false;
}

void ModuleManager::UninstallRig(uint32 itemID)
{
    GenericModule * mod = m_Modules->GetModule(itemID);
    if( mod != NULL )
        mod->DestroyRig();
}

bool ModuleManager::SwapSubSystem(InventoryItemRef item, EVEItemFlags flag)
{
    if(item->groupID() >= 954 && item->groupID() <= 958)
    {
        _fitModule(item,flag);
        return true;
    }
    else
        sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Ship->GetOperator()->GetName(), item->itemID());

    return false;
}

bool ModuleManager::FitModule(InventoryItemRef item, EVEItemFlags flag)
{
    if(item->categoryID() == EVEDB::invCategories::Module)
    {
        // Attempt to fit the module
        if( _fitModule(item, flag) )
        {
            // Now that module is successfully fitted, attempt to put it Online:
            Online(item->itemID());
            return true;
        }
    }
    else
        sLog.Debug("ModuleManager","%s tried to fit item %u, which is not a module", m_Ship->GetOperator()->GetName(), item->itemID());

    return false;
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

bool ModuleManager::_fitModule(InventoryItemRef item, EVEItemFlags flag)
{
    bool verifyFailed = false;
    GenericModule * mod = ModuleFactory(item, ShipRef(m_Ship));

    // Check for max turret modules allowed:
    if( mod->isTurretFitted() && (m_Modules->GetFittedTurretCount() == m_Ship->GetMaxTurrentHardpoints().get_int()) )
    {
        //std::map<std::string, PyRep *> args;
        //args["typename"] = new PyString(item->itemName().c_str());
        //args["portion"] = new PyInt(item->type().portionSize());

        throw PyException( MakeUserError( "NotEnoughTurretSlots" ) );
        verifyFailed = true;
    }
    // Check for max launcher modules allowed:
    if( mod->isLauncherFitted() && (m_Modules->GetFittedLauncherCount() == m_Ship->GetMaxLauncherHardpoints().get_int()) )
    {
        //std::map<std::string, PyRep *> args;
        //args["typename"] = new PyString(item->itemName().c_str());
        //args["portion"] = new PyInt(item->type().portionSize());

        throw PyException( MakeUserError( "NotEnoughLauncherSlots" ) );
        verifyFailed = true;
    }
    // Check for max modules of group allowed:
    else if( mod->isMaxGroupFitLimited() && (m_Modules->GetFittedModuleCountByGroup(item->groupID()) == mod->getItem()->GetAttribute(AttrMaxGroupFitted).get_int()) )
    {
        //std::map<std::string, PyRep *> args;
        //args["typename"] = new PyString(item->itemName().c_str());
        //args["portion"] = new PyInt(item->type().portionSize());

        throw PyException( MakeUserError( "CantFitTooManyByGroup" ) );
        verifyFailed = true;
    }
    else
    {
        // Fit Module now that all checks have passed:
        //m_Modules->AddModule(mod->flag(), mod);
        m_Modules->AddModule(flag, mod);
    }

    if( verifyFailed )
    {
        delete mod;
        return false;
    }
    else
        return true;
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
    //return 1;

    GenericModule * mod = m_Modules->GetModule(itemID);
    if( mod != NULL )
    {
		if(effectName == "online")
		{
			//ModuleCommand cmd = _translateEffectName(effectName);		// GET RID of this function, effectName should be passed into the module's calls
			mod->Online();

			// We should check for "online" here or something else, then either call the mod->Online() or mod->Activate()
			//if(cmd == ONLINE)
		    //    mod->Online();     // this currently fails since m_selectedEffect and m_defaultEffect in the ModuleEffect class are undefined
			//there needs to be more cases here i just don't know what they're called yet
		}
		else
		{
			mod->Activate(targetID);
		}
    }

    return 1;
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
    GenericModule * mod = m_Modules->GetModule(itemID);
    if( mod != NULL )
    {
		if(effectName == "online")
		{
			//ModuleCommand cmd = _translateEffectName(effectName);		// GET RID of this function, effectName should be passed into the module's calls
			mod->Offline();

			// We should check for "online" here or something else, then either call the mod->Offline() or mod->Deactivate()
			//if(cmd == OFFLINE)
			//    mod->Offline();     // this currently fails since m_selectedEffect and m_defaultEffect in the ModuleEffect class are undefined
			//there needs to be more cases here i just don't know what they're called yet
		}
		else
		{
			mod->Deactivate();
		}
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
    case MODULE_BANK_HIGH_POWER:
        for(int i = flagHiSlot0; i < flagHiSlot7 + 1; i++)
        {
            tmp = m_Modules->GetModule((EVEItemFlags)i);
            if( tmp->typeID() == typeID && tmp->isOnline() )
                mods.push_back(tmp);
        }
        break;
    case MODULE_BANK_MEDIUM_POWER:
        for(int i = flagMedSlot0; i < flagMedSlot7 + 1; i++)
        {
            tmp = m_Modules->GetModule((EVEItemFlags)i);
            if( tmp->typeID() == typeID && tmp->isOnline() )
                mods.push_back(tmp);
        }
        break;
    case MODULE_BANK_LOW_POWER:
        for(int i = flagLowSlot0; i < flagLowSlot7 + 1; i++)
        {
            tmp = m_Modules->GetModule((EVEItemFlags)i);
            if( tmp->typeID() == typeID && tmp->isOnline() )
                mods.push_back(tmp);
        }
        break;
    case MODULE_BANK_RIG:
        for(int i = flagRigSlot0; i < flagRigSlot7 + 1; i++)
        {
            tmp = m_Modules->GetModule((EVEItemFlags)i);
            if( tmp->typeID() == typeID && tmp->isOnline() )
                mods.push_back(tmp);
        }
        break;
    case MODULE_BANK_SUBSYSTEM:
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

void ModuleManager::SaveModules()
{
    m_Modules->SaveModules();
}

int32 ModuleManager::ApplyRemoteEffect(uint32 attributeID, uint32 originatorID, SystemEntity * systemEntity, ModifierRef modifierRef)
{
    //

    return 1;
}

int32 ModuleManager::RemoveRemoteEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    //

    return 1;
}

int32 ModuleManager::ApplySubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    ModifierMap * modMap = NULL;

    // Make sure the ModifierRef passed in is not NULL:
    if(modifierRef.get() == NULL)
        return -1;

    // Check to see if this attributeID does not have a ModifierMap in the Map of ModifierMaps
    if( m_LocalSubsystemModifierMaps->find(attributeID) == m_LocalSubsystemModifierMaps->end() )
    {
        // A Modifier Map for this attributeID does not exist, create a new one:
        modMap = new ModifierMap();
        if(modMap == NULL)
            return -1;
    }
    else
    {
        // A Modifier Map for this attributeID already exists, find it and get its pointer:
        modMap = m_LocalSubsystemModifierMaps->find(attributeID)->second;
    }

    // Check to see if the modifier map has any entries corresponding to the passed-in modifier's value:
    if( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
    {
        // Modifier entry in this attributeID's Modifier Map already exists (modifierRef->GetModifierValue() found a match),
        // so check its originatorID and if that matches, DO NOT add this Modifier object to the map as the reference
        // already exists, the Module class can modify the contents of the Modifier object without really calling this function,
        // however, to maintain consistent code, the Module classes will always call this function to notify the map class
        // that the contents of the map was changed, or made 'dirty':
        modMap->m_MapIsDirty = true;
        ModifierMapType::iterator cur;
        std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
        range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being added
        for (cur=range.first; cur!=range.second; ++cur)
            if( cur->second->GetOriginatorID() == originatorID )
                return 1;   // Yep, we found the Modifier owned by this originatorID, so we return "success" because the Module
                            // class object already updated this Modifier through its own ModifierRef, we don't need to do anything
                            // else here except return and prevent ADDING to the ModifierMap

        // For loop searching existing modifiers completed, so this originatorID's Modifier
        // is NOT in the map yet... Let's add it:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
    }
    else
    {
        // Modifier entry in this attributeID's Modifier Map does not exist yet, so lets insert it for the first time:
        // Insert the (modifierValue, ModifierRef) pair into the Modifier Map for this attributeID:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
        modMap->m_MapIsDirty = true;
        m_LocalSubsystemModifierMaps->insert(std::pair<uint32, ModifierMap *>(attributeID, modMap));
    }

    return 1;
}

int32 ModuleManager::RemoveSubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    bool bModifierFound = false;
    ModifierMap * modMap = NULL;

    if( m_LocalSubsystemModifierMaps->find(attributeID) != m_LocalSubsystemModifierMaps->end() )
    {
        modMap = m_LocalSubsystemModifierMaps->find(attributeID)->second;
        modMap->m_MapIsDirty = true;
        
        if( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
        {
            modMap->m_MapIsDirty = true;
            ModifierMapType::iterator cur;
            std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
            range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being removed
            for (cur=range.first; cur!=range.second; ++cur)
                if( cur->second->GetOriginatorID() == originatorID )
                {
                    bModifierFound = true;  // Yep, we found the Modifier owned by this originatorID, so we break out of the for()
                                            // so we can now remove this exact Modifier object from the multimap
                    break;
                }

            if( bModifierFound == true )
            {
                // For loop searching existing modifiers completed, so this originatorID's Modifier
                // was found in the map
                modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
            }
            else
                return -1;  // This modifier's originatorID was not found in the map, so return error code
        }
        else
            return -1;  // This modifier's modifier value was not even found in the map, so return error code
    }
    else
        return -1;  // Modifier Map for supplied attributeID does not exist, return error value

    return 1;
}

int32 ModuleManager::ApplyShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    ModifierMap * modMap = NULL;

    // Make sure the ModifierRef passed in is not NULL:
    if(modifierRef.get() == NULL)
        return -1;

    // Check to see if this attributeID does not have a ModifierMap in the Map of ModifierMaps
    if( m_LocalShipSkillModifierMaps->find(attributeID) == m_LocalShipSkillModifierMaps->end() )
    {
        // A Modifier Map for this attributeID does not exist, create a new one:
        modMap = new ModifierMap();
        if(modMap == NULL)
            return -1;
    }
    else
    {
        // A Modifier Map for this attributeID already exists, find it and get its pointer:
        modMap = m_LocalShipSkillModifierMaps->find(attributeID)->second;
    }

    // Check to see if the modifier map has any entries corresponding to the passed-in modifier's value:
    if( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
    {
        // Modifier entry in this attributeID's Modifier Map already exists (modifierRef->GetModifierValue() found a match),
        // so check its originatorID and if that matches, DO NOT add this Modifier object to the map as the reference
        // already exists, the Module class can modify the contents of the Modifier object without really calling this function,
        // however, to maintain consistent code, the Module classes will always call this function to notify the map class
        // that the contents of the map was changed, or made 'dirty':
        modMap->m_MapIsDirty = true;
        ModifierMapType::iterator cur;
        std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
        range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being added
        for (cur=range.first; cur!=range.second; ++cur)
            if( cur->second->GetOriginatorID() == originatorID )
                return 1;   // Yep, we found the Modifier owned by this originatorID, so we return "success" because the Module
                            // class object already updated this Modifier through its own ModifierRef, we don't need to do anything
                            // else here except return and prevent ADDING to the ModifierMap

        // For loop searching existing modifiers completed, so this originatorID's Modifier
        // is NOT in the map yet... Let's add it:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
    }
    else
    {
        // Modifier entry in this attributeID's Modifier Map does not exist yet, so lets insert it for the first time:
        // Insert the (modifierValue, ModifierRef) pair into the Modifier Map for this attributeID:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
        modMap->m_MapIsDirty = true;
        m_LocalShipSkillModifierMaps->insert(std::pair<uint32, ModifierMap *>(attributeID, modMap));
    }

    return 1;
}

int32 ModuleManager::RemoveShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    bool bModifierFound = false;
    ModifierMap * modMap = NULL;

    if( m_LocalShipSkillModifierMaps->find(attributeID) != m_LocalShipSkillModifierMaps->end() )
    {
        modMap = m_LocalShipSkillModifierMaps->find(attributeID)->second;
        modMap->m_MapIsDirty = true;
        
        if( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
        {
            modMap->m_MapIsDirty = true;
            ModifierMapType::iterator cur;
            std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
            range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being removed
            for (cur=range.first; cur!=range.second; ++cur)
                if( cur->second->GetOriginatorID() == originatorID )
                {
                    bModifierFound = true;  // Yep, we found the Modifier owned by this originatorID, so we break out of the for()
                                            // so we can now remove this exact Modifier object from the multimap
                    break;
                }

            if( bModifierFound == true )
            {
                // For loop searching existing modifiers completed, so this originatorID's Modifier
                // was found in the map
                modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
            }
            else
                return -1;  // This modifier's originatorID was not found in the map, so return error code
        }
        else
            return -1;  // This modifier's modifier value was not even found in the map, so return error code
    }
    else
        return -1;  // Modifier Map for supplied attributeID does not exist, return error value

    return 1;
}

int32 ModuleManager::ApplyModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    return 1;
}

int32 ModuleManager::RemoveModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    return 1;
}

int32 ModuleManager::ApplyImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    return 1;
}

int32 ModuleManager::RemoveImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    return 1;
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
    case 'a': return ACTIVATE;
    case 'd': return s[2] == 'a' ? DEACTIVATE : DEOVERLOAD;
    case 'o': return s[1] == 'n' ? ONLINE : (s[1] == 'f' ? OFFLINE : OVERLOAD); //compound booleans ftw
    }

    return CMD_ERROR;
}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////
