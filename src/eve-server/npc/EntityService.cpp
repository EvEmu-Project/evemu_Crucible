/**
  * @name EntityService.cpp
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */

#include "eve-server.h"
#include "EVEServerConfig.h"
#include "npc/EntityService.h"
#include "system/SystemManager.h"
#include "services/ServiceManager.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "system/SystemEntity.h"
#include "ship/Ship.h"
#include <iostream>

EntityService::EntityService(EVEServiceManager& mgr) :
    BindableService("entity", mgr)
{
}

BoundDispatcher *EntityService::BindObject(Client* client, PyRep* bindParameters) {
    std::cout << "[DRONE] === ENTITY BIND OBJECT CALLED ===" << std::endl;
    if (bindParameters) {
        std::cout << "[DRONE] Bind parameters type: " << bindParameters->TypeString() << std::endl;
    }
    
    _log(DRONE__DUMP, "EntityService bind request");
    bindParameters->Dump(DRONE__DUMP, "    ");
    if (!bindParameters->IsInt()) {
        codelog(SERVICE__ERROR, "%s: Non-integer bind argument '%s'", client->GetName(), bindParameters->TypeString());
        return nullptr;
    }

    uint32 systemID = bindParameters->AsInt()->value();
    if (!sDataMgr.IsSolarSystem(systemID)) {
        codelog(SERVICE__ERROR, "%s: Expected systemID, but got %u.", client->GetName(), systemID);
        return nullptr;
    }

    auto it = this->m_instances.find (systemID);
    if (it != this->m_instances.end ())
        return it->second;

    EntityBound* bound = new EntityBound(this->GetServiceManager(), *this, client->SystemMgr(), systemID);
    this->m_instances.insert_or_assign (systemID, bound);
    
    std::cout << "[DRONE] === ENTITY BIND OBJECT CREATED: systemID=" << systemID << std::endl;
    return bound;
}

void EntityService::BoundReleased (EntityBound* bound) {
    auto it = this->m_instances.find (bound->GetSystemID());
    if (it == this->m_instances.end ())
        return;
    this->m_instances.erase (it);
}

EntityBound::EntityBound(EVEServiceManager &mgr, EntityService& parent, SystemManager* systemMgr, uint32 systemID) :
    EVEBoundObject(mgr, parent),
    m_sysMgr(systemMgr),
    m_systemID(systemID)
{
    std::cout << "[DRONE] === ENTITY BOUND CREATED ===" << std::endl;
    
    this->Add("CmdEngage", &EntityBound::CmdEngage);
    this->Add("CmdRelinquishControl", &EntityBound::CmdRelinquishControl);
    this->Add("CmdDelegateControl", &EntityBound::CmdDelegateControl);
    this->Add("CmdAssist", &EntityBound::CmdAssist);
    this->Add("CmdGuard", &EntityBound::CmdGuard);
    this->Add("CmdMine", &EntityBound::CmdMine);
    this->Add("CmdMineRepeatedly", &EntityBound::CmdMineRepeatedly);
    this->Add("CmdUnanchor", &EntityBound::CmdUnanchor);
    this->Add("CmdReturnHome", &EntityBound::CmdReturnHome);
    this->Add("CmdReturnBay", &EntityBound::CmdReturnBay);
    this->Add("CmdAbandonDrone", &EntityBound::CmdAbandonDrone);
    this->Add("CmdReconnectToDrones", &EntityBound::CmdReconnectToDrones);
}

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// ============================================================================

DroneSE* EntityBound::GetValidDrone(Client* client, uint32 droneID) {
    SystemEntity* pEntity = m_sysMgr->GetSE(droneID);
    if (!pEntity) {
        std::cout << "[DRONE] GetValidDrone: Drone " << droneID << " not found in system" << std::endl;
        return nullptr;
    }
    
    DroneSE* pDrone = dynamic_cast<DroneSE*>(pEntity);
    if (!pDrone) {
        std::cout << "[DRONE] GetValidDrone: Entity " << droneID << " is not a drone" << std::endl;
        return nullptr;
    }
    
    Client* owner = pDrone->GetOwner();
    if (!owner) {
        std::cout << "[DRONE] GetValidDrone: Drone " << droneID << " has no owner" << std::endl;
        return nullptr;
    }
    
    if (owner->GetCharacterID() != client->GetCharacterID()) {
        std::cout << "[DRONE] GetValidDrone: Drone " << droneID << " not owned by player" << std::endl;
        return nullptr;
    }
    
    DroneAIMgr* pAI = pDrone->GetAI();
    if (pAI && pAI->GetState() == DroneAI::State::Incapacitated) {
        std::cout << "[DRONE] GetValidDrone: Drone " << droneID << " is incapacitated" << std::endl;
        return nullptr;
    }
    
    return pDrone;
}

SystemEntity* EntityBound::GetValidTarget(Client* client, uint32 targetID) {
    SystemEntity* pTarget = m_sysMgr->GetSE(targetID);
    if (!pTarget) {
        std::cout << "[DRONE] GetValidTarget: Target " << targetID << " not found in system" << std::endl;
        return nullptr;
    }
    return pTarget;
}

// ============================================================================
// ПОЛУЧЕНИЕ КОРАБЛЯ ИГРОКА
// ============================================================================

ShipSE* EntityBound::GetPlayerShip(Client* client) {
    uint32 shipID = client->GetShipID();
    if (shipID != 0) {
        SystemEntity* pEntity = m_sysMgr->GetSE(shipID);
        if (pEntity) {
            ShipSE* pShip = dynamic_cast<ShipSE*>(pEntity);
            if (pShip) {
                std::cout << "[DRONE] GetPlayerShip: Found ship via SystemManager (ID: " << shipID << ")" << std::endl;
                return pShip;
            }
        }
    }
    
    ShipSE* pShip = client->GetShipSE();
    if (pShip) {
        std::cout << "[DRONE] GetPlayerShip: Found ship via GetShipSE()" << std::endl;
        return pShip;
    }
    
    std::cout << "[DRONE] GetPlayerShip: No ship found for player " << client->GetName() << std::endl;
    return nullptr;
}

// ============================================================================
// ОСНОВНЫЕ КОМАНДЫ УПРАВЛЕНИЯ ДРОНАМИ
// ============================================================================

PyResult EntityBound::CmdEngage(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    std::cout << "[DRONE] === METHOD CALLED: CmdEngage ===" << std::endl;
    std::cout << "[DRONE] === CMD ENGAGE RECEIVED ===" << std::endl;
    call.Dump(DRONE__DUMP);
    
    if (!droneIDs || !targetID || droneIDs->size() == 0) {
        call.client->SendNotifyMsg("Invalid drone command parameters");
        return new PyDict();
    }
    
    std::cout << "[DRONE] === CMD ENGAGE: Player " << call.client->GetName() 
              << ", target ID: " << targetID->value() 
              << ", drones count: " << droneIDs->size() << " ===" << std::endl;
    
    ShipSE* pShip = GetPlayerShip(call.client);
    if (!pShip) {
        call.client->SendNotifyMsg("You must be in a ship to control drones");
        return new PyDict();
    }
    
    // ============================================================
    // ПОЛУЧАЕМ ЦЕЛЬ
    // ============================================================
    SystemEntity* pTarget = GetValidTarget(call.client, targetID->value());
    if (!pTarget) {
        call.client->SendNotifyMsg("Target not found");
        return new PyDict();
    }
    
    std::cout << "[DRONE] CmdEngage: Target found: " << pTarget->GetName() << "(" << pTarget->GetID() << ")" << std::endl;
    
    // ============================================================
    // РАСЧЕТ ДАЛЬНОСТИ УПРАВЛЕНИЯ ДРОНАМИ
    // ============================================================
    
    float droneRange = 5000.0f;
    
    InventoryItemRef pShipItem = pShip->GetSelf();
    if (pShipItem) {
        if (pShipItem->HasAttribute(AttrDroneControlDistance)) {
            droneRange = pShipItem->GetAttribute(AttrDroneControlDistance).get_float();
            std::cout << "[DRONE] Drone control range from ship attribute: " << droneRange << std::endl;
        }
    }
    
    CharacterRef pChar = call.client->GetChar();
    if (pChar) {
        uint32 droneControlLevel = pChar->GetSkillLevel(3324);
        if (droneControlLevel > 0) {
            droneRange += droneControlLevel * 5000.0f;
            std::cout << "[DRONE] Drone Control skill level " << droneControlLevel << ": +" << (droneControlLevel * 5000.0f) << "m" << std::endl;
        }
        
        uint32 advDroneControlLevel = pChar->GetSkillLevel(3334);
        if (advDroneControlLevel > 0) {
            droneRange += advDroneControlLevel * 5000.0f;
            std::cout << "[DRONE] Advanced Drone Control skill level " << advDroneControlLevel << ": +" << (advDroneControlLevel * 5000.0f) << "m" << std::endl;
        }
        
        uint32 droneInterfacingLevel = pChar->GetSkillLevel(3338);
        if (droneInterfacingLevel > 0) {
            float bonus = 1.0f + (droneInterfacingLevel * 0.2f);
            droneRange *= bonus;
            std::cout << "[DRONE] Drone Interfacing skill level " << droneInterfacingLevel << ": x" << bonus << " range" << std::endl;
        }
    }
    
    GPoint shipPos = pShip->GetPosition();
    GPoint targetPos = pTarget->GetPosition();
    float distance = shipPos.distance(targetPos);
    
    std::cout << "[DRONE] Distance to target: " << distance << "m, Drone range: " << droneRange << "m" << std::endl;
    
    if (distance > droneRange) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Target is out of drone control range (%.0fm / %.0fm)", distance, droneRange);
        call.client->SendNotifyMsg(msg);
        return new PyDict();
    }
    
    uint32 validDrones = 0;
    
    for (PyRep* item : *droneIDs) {
        if (!item->IsInt()) continue;
        uint32 droneID = item->AsInt()->value();
        
        std::cout << "[DRONE] CmdEngage: Processing drone ID: " << droneID << std::endl;
        
        DroneSE* pDrone = GetValidDrone(call.client, droneID);
        if (!pDrone) {
            std::cout << "[DRONE] CmdEngage: Drone " << droneID << " is not valid" << std::endl;
            continue;
        }
        
        DroneAIMgr* pAI = pDrone->GetAI();
        if (!pAI) {
            std::cout << "[DRONE] CmdEngage: Drone " << droneID << " has no AI manager" << std::endl;
            continue;
        }
        
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - Current state: " << (int)pAI->GetState() << std::endl;
        
        if (!pDrone->IsEnabled()) {
            std::cout << "[DRONE] CmdEngage: Drone " << droneID << " is offline, enabling..." << std::endl;
            pDrone->Online(pShip);
        }
        
        // ============================================================
        // ОБХОД TargetMgr: добавляем цель напрямую в структуру дрона
        // ============================================================
        pDrone->TargetMgr()->ClearTargets();
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - Cleared old targets" << std::endl;
        
        // 1. Сохраняем цель в самом дроне (используем существующий метод SetTarget)
        pDrone->SetTarget(pTarget);
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - SetTarget called, target ID: " << pTarget->GetID() << std::endl;
        
        // 2. Проверяем, что цель установилась
        uint32 targetIDStored = pDrone->GetTargetID();
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - GetTargetID: " << targetIDStored << std::endl;
        
        // 3. Добавляем цель через TargetMgr (пытаемся обойти ограничения)
        pDrone->TargetMgr()->TargetAdded(pTarget);
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - TargetAdded called" << std::endl;
        
        // 4. Проверяем
        SystemEntity* pCheckTarget = pDrone->TargetMgr()->GetFirstTarget(true);
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - TargetMgr has target: " << (pCheckTarget ? "YES" : "NO") << std::endl;
        
        // 5. Если TargetMgr всё ещё не видит цель - устанавливаем флаг canAttack вручную
        if (!pCheckTarget) {
            std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - WARNING: TargetMgr does not see target!" << std::endl;
        }
        
        pAI->SetEngaged(pTarget);
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - SetEngaged called" << std::endl;
        
        pDrone->Enable();
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " - Enabled" << std::endl;
        
        validDrones++;
        std::cout << "[DRONE] CmdEngage: Drone " << droneID << " successfully engaged target " << targetID->value() << std::endl;
    }
    
    if (validDrones == 0) {
        call.client->SendNotifyMsg("No valid drones available to engage");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg), "%u drones engaging target", validDrones);
        call.client->SendNotifyMsg(msg);
    }
    
    std::cout << "[DRONE] === CMD ENGAGE COMPLETED: " << validDrones << " drones engaged ===" << std::endl;
    return new PyDict();
}

PyResult EntityBound::CmdReturnBay(PyCallArgs &call, PyList* droneIDs) {
    std::cout << "[DRONE] === METHOD CALLED: CmdReturnBay ===" << std::endl;
    std::cout << "[DRONE] === CMD RETURN BAY ===" << std::endl;
    call.Dump(DRONE__DUMP);
    
    if (!droneIDs || droneIDs->size() == 0) {
        return new PyDict();
    }
    
    ShipSE* pShip = GetPlayerShip(call.client);
    if (!pShip) {
        call.client->SendNotifyMsg("You must be in a ship");
        return new PyDict();
    }
    
    uint32 returnedDrones = 0;
    
    for (PyRep* item : *droneIDs) {
        if (!item->IsInt()) continue;
        uint32 droneID = item->AsInt()->value();
        
        DroneSE* pDrone = GetValidDrone(call.client, droneID);
        if (!pDrone) {
            std::cout << "[DRONE] CmdReturnBay: Drone " << droneID << " is not valid" << std::endl;
            continue;
        }
        
        DroneAIMgr* pAI = pDrone->GetAI();
        if (!pAI) continue;
        
        pAI->StopAttack();
        pAI->Return();
        
        returnedDrones++;
        std::cout << "[DRONE] CmdReturnBay: Drone " << droneID << " returning to bay" << std::endl;
    }
    
    if (returnedDrones > 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%u drones returning to bay", returnedDrones);
        call.client->SendNotifyMsg(msg);
    }
    
    return new PyDict();
}

PyResult EntityBound::CmdReturnHome(PyCallArgs &call, PyList* droneIDs) {
    std::cout << "[DRONE] === METHOD CALLED: CmdReturnHome ===" << std::endl;
    std::cout << "[DRONE] === CMD RETURN HOME ===" << std::endl;
    call.Dump(DRONE__DUMP);
    
    if (!droneIDs || droneIDs->size() == 0) {
        return new PyDict();
    }
    
    ShipSE* pShip = GetPlayerShip(call.client);
    if (!pShip) {
        call.client->SendNotifyMsg("You must be in a ship");
        return new PyDict();
    }
    
    uint32 returnedDrones = 0;
    
    for (PyRep* item : *droneIDs) {
        if (!item->IsInt()) continue;
        uint32 droneID = item->AsInt()->value();
        
        DroneSE* pDrone = GetValidDrone(call.client, droneID);
        if (!pDrone) {
            std::cout << "[DRONE] CmdReturnHome: Drone " << droneID << " is not valid" << std::endl;
            continue;
        }
        
        DroneAIMgr* pAI = pDrone->GetAI();
        if (!pAI) continue;
        
        pAI->StopAttack();
        pAI->SetIdle();
        pDrone->TargetMgr()->ClearAllTargets();
        
        returnedDrones++;
        std::cout << "[DRONE] CmdReturnHome: Drone " << droneID << " returning home" << std::endl;
    }
    
    if (returnedDrones > 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%u drones returning home", returnedDrones);
        call.client->SendNotifyMsg(msg);
    }
    
    return new PyDict();
}

PyResult EntityBound::CmdAbandonDrone(PyCallArgs &call, PyList* droneIDs) {
    std::cout << "[DRONE] === METHOD CALLED: CmdAbandonDrone ===" << std::endl;
    std::cout << "[DRONE] === CMD ABANDON DRONE ===" << std::endl;
    call.Dump(DRONE__DUMP);
    
    if (!droneIDs || droneIDs->size() == 0) {
        return new PyDict();
    }
    
    uint32 abandonedDrones = 0;
    
    for (PyRep* item : *droneIDs) {
        if (!item->IsInt()) continue;
        uint32 droneID = item->AsInt()->value();
        
        DroneSE* pDrone = GetValidDrone(call.client, droneID);
        if (!pDrone) {
            std::cout << "[DRONE] CmdAbandonDrone: Drone " << droneID << " is not valid" << std::endl;
            continue;
        }
        
        pDrone->Abandon();
        abandonedDrones++;
        std::cout << "[DRONE] CmdAbandonDrone: Drone " << droneID << " abandoned" << std::endl;
    }
    
    if (abandonedDrones > 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%u drones abandoned", abandonedDrones);
        call.client->SendNotifyMsg(msg);
    }
    
    return new PyDict();
}

PyResult EntityBound::CmdReconnectToDrones(PyCallArgs &call, PyList* droneCandidates) {
    std::cout << "[DRONE] === METHOD CALLED: CmdReconnectToDrones ===" << std::endl;
    std::cout << "[DRONE] === CMD RECONNECT ===" << std::endl;
    call.Dump(DRONE__DUMP);
    
    uint32 playerID = call.client->GetCharacterID();
    PyList* resultList = new PyList();
    uint32 foundDrones = 0;
    
    for (uint32 i = 1; i < 1000000; ++i) {
        SystemEntity* pEntity = m_sysMgr->GetSE(i);
        if (!pEntity) continue;
        
        DroneSE* pDrone = dynamic_cast<DroneSE*>(pEntity);
        if (!pDrone) continue;
        
        Client* owner = pDrone->GetOwner();
        if (!owner || owner->GetCharacterID() != playerID) continue;
        
        DroneAIMgr* pAI = pDrone->GetAI();
        if (pAI && pAI->GetState() != DroneAI::State::Incapacitated) {
            resultList->AddItem(new PyInt(pDrone->GetID()));
            foundDrones++;
            std::cout << "[DRONE] CmdReconnectToDrones: Found reconnectable drone " << pDrone->GetID() << std::endl;
        }
    }
    
    std::cout << "[DRONE] CmdReconnectToDrones: Found " << foundDrones << " reconnectable drones" << std::endl;
    return resultList;
}

// ============================================================================
// ЗАГОТОВКИ ДЛЯ ДРУГИХ КОМАНД
// ============================================================================

PyResult EntityBound::CmdRelinquishControl(PyCallArgs &call, PyList* IDs) {
    std::cout << "[DRONE] === METHOD CALLED: CmdRelinquishControl ===" << std::endl;
    call.client->SendNotifyMsg("Drone control relinquish is not implemented yet");
    return new PyDict();
}

PyResult EntityBound::CmdDelegateControl(PyCallArgs &call, PyList* droneIDs, PyInt* controllerID) {
    std::cout << "[DRONE] === METHOD CALLED: CmdDelegateControl ===" << std::endl;
    call.client->SendNotifyMsg("Drone control delegation is not implemented yet");
    return new PyDict();
}

PyResult EntityBound::CmdAssist(PyCallArgs &call, PyInt* assistID, PyList* droneIDs) {
    std::cout << "[DRONE] === METHOD CALLED: CmdAssist ===" << std::endl;
    call.client->SendNotifyMsg("Drone assist is not implemented yet");
    return new PyDict();
}

PyResult EntityBound::CmdGuard(PyCallArgs &call, PyInt* guardID, PyList* droneIDs) {
    std::cout << "[DRONE] === METHOD CALLED: CmdGuard ===" << std::endl;
    call.client->SendNotifyMsg("Drone guard is not implemented yet");
    return new PyDict();
}

PyResult EntityBound::CmdMine(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    std::cout << "[DRONE] === METHOD CALLED: CmdMine ===" << std::endl;
    call.client->SendNotifyMsg("Drone mining is not implemented yet");
    return new PyDict();
}

PyResult EntityBound::CmdMineRepeatedly(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    std::cout << "[DRONE] === METHOD CALLED: CmdMineRepeatedly ===" << std::endl;
    call.client->SendNotifyMsg("Drone mining repeated is not implemented yet");
    return new PyDict();
}

PyResult EntityBound::CmdUnanchor(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    std::cout << "[DRONE] === METHOD CALLED: CmdUnanchor ===" << std::endl;
    call.client->SendNotifyMsg("Drone unanchor is not implemented yet");
    return new PyDict();
}