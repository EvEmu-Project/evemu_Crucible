
 /**
  * @name EntityService.cpp
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */


#include "eve-server.h"




#include "EVEServerConfig.h"
#include "EntityList.h"
#include "inventory/AttributeEnum.h"
#include "npc/Drone.h"
#include "npc/EntityService.h"
#include "ship/Ship.h"
#include "system/TargetManager.h"
#include "system/SystemManager.h"
#include "services/ServiceManager.h"

EntityService::EntityService(EVEServiceManager& mgr) :
    BindableService("entity", mgr)
{
}

/*  drone states...
namespace DroneAI {
    namespace State {
        enum {
            Invalid           = -1,
            // defined in client
            Idle              = 0,  // not doing anything....idle.
            Combat            = 1,  // fighting - needs targetID
            Mining            = 2,  // unsure - needs targetID
            Approaching       = 3,  // too close to chase, but to far to engage
            Departing         = 4,  // return to ship
            Departing2        = 5,  // leaving.  different from Departing
            Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Fleeing           = 7,  // running away
            Operating         = 9,  // whats diff from engaged here?
            Engaged           = 10, // non-combat? - needs targetID
            // internal only
            Unknown           = 8,  // as stated
            Guarding          = 11,
            Assisting         = 12,
            Incapacicated     = 13  //
        };
    }
}
*/

/*
DRONE__ERROR
DRONE__WARNING
DRONE__MESSAGE
DRONE__INFO
DRONE__TRACE
DRONE__DUMP
DRONE__AI_TRACE
*/

/** @todo  will need to make sure this object is deleted when changing systems  */
BoundDispatcher *EntityService::BindObject(Client* client, PyRep* bindParameters) {
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

/* Drone commands.
 *
 * The client sends these to the 'entity' service bound with the solar system ID.  Every command but reconnect returns
 *  {droneID: (userErrorName, args)} for the drones that could not carry it out, or {} when they all did.  The client
 *  shows one of the errors (or MultiDroneCmdResult if they differ).  Returning None would break the client.
 * The client checks for an active target itself; everything else (control, ranges, limits) is checked here.
 */

namespace {
    // control range of a pilot's drones
    double PilotControlRange(Client* pClient) {
        double range(pClient->GetChar()->GetAttribute(AttrDroneControlDistance).get_double());
        return (range > 0.0 ? range : 20000.0);
    }

    double Gap(SystemEntity* pA, SystemEntity* pB) {
        return pA->GetPosition().distance(pB->GetPosition()) - pA->GetRadius() - pB->GetRadius();
    }

    PyTuple* MakeError(const char* msgName, std::map<std::string, PyRep*> args) {
        PyDict* dict = new PyDict();
        for (auto cur : args)
            dict->SetItemString(cur.first.c_str(), cur.second);
        PyTuple* err = new PyTuple(2);
            err->SetItem(0, new PyString(msgName));
            err->SetItem(1, dict);
        return err;
    }

    // {targetTypeName} is the drone's name in most of the entity messages
    PyTuple* DroneError(const char* msgName, DroneSE* pDrone) {
        return MakeError(msgName, {{"targetTypeName", new PyString(pDrone->GetName())}});
    }

    std::vector<uint32> GetIDs(PyList* list) {
        std::vector<uint32> ids;
        for (PyList::const_iterator itr = list->begin(); itr != list->end(); ++itr)
            if ((*itr)->IsInt() or (*itr)->IsLong())
                ids.push_back(PyRep::IntegerValueU32(*itr));
        return ids;
    }

    void SendActivity(Client* pClient, uint32 droneID, uint32 charID, const char* activity) {
        if (pClient == nullptr)
            return;
        PyTuple* ev = new PyTuple(3);
            ev->SetItem(0, new PyInt(droneID));
            ev->SetItem(1, (charID == 0 ? PyStatic.NewNone() : new PyInt(charID)));
            ev->SetItem(2, (activity == nullptr ? PyStatic.NewNone() : new PyString(activity)));
        pClient->SendNotification("OnDroneActivityChange", "charid", &ev);
    }

    /* checks every command makes.  returns the error for this drone, or nullptr if the pilot can command it.
     *  pDrone is set whenever the drone exists
     */
    PyTuple* CheckDrone(Client* pClient, SystemManager* pSystem, uint32 droneID, DroneSE*& pDrone) {
        pDrone = nullptr;
        SystemEntity* pSE(pSystem->GetSE(droneID));
        if ((pSE == nullptr) or !pSE->IsDroneSE())
            //{'FullPath': u'UI/Messages', 'label': u'EntityNotPresentBody'}(u'{targetTypeName} cannot be commanded as it is not actually present.')
            return MakeError("EntityNotPresent", {{"targetTypeName", new PyString("The drone")}});
        pDrone = pSE->GetDroneSE();
        ShipSE* pShip(pClient->GetShipSE());
        //{'FullPath': u'UI/Messages', 'label': u'EntityNotYoursToCommandBody'}(u'{targetTypeName} does not respond to your commands.')
        if ((pShip == nullptr) or (pDrone->GetControllerID() != pShip->GetID())
        or  (pDrone->GetControllerOwnerID() != static_cast<uint32>(pClient->GetCharacterID())))
            return DroneError("EntityNotYoursToCommand", pDrone);
        //{'FullPath': u'UI/Messages', 'label': u'EntityDistantCommandBody'}(u'{targetTypeName} is too far away and will not respond to the command you are giving it (must be within {distance} meters from it).')
        double range(PilotControlRange(pClient));
        if ((pDrone->SysBubble() != pShip->SysBubble()) or (Gap(pDrone, pShip) > range))
            return MakeError("EntityDistantCommand", {{"targetTypeName", new PyString(pDrone->GetName())}, {"distance", new PyInt(static_cast<int32>(range))}});
        return nullptr;
    }

    // a target for engage/mine.  returns the error, or nullptr if the target can be used
    PyTuple* CheckTarget(Client* pClient, DroneSE* pDrone, SystemEntity* pTarget) {
        //{'FullPath': u'UI/Messages', 'label': u'EntityTargetNotPresentBody'}(u'{targetTypeName} cannot be commanded to work on a target that is no longer present.')
        if ((pTarget == nullptr) or pTarget->IsDead() or (pTarget->SysBubble() != pDrone->SysBubble()))
            return DroneError("EntityTargetNotPresent", pDrone);
        //{'FullPath': u'UI/Messages', 'label': u'EntityTargetTooDistantBody'}(u'The drones fail to execute your commands as the target {targetTypeName} is not within your {distance} m drone command range.')
        double range(PilotControlRange(pClient));
        if (Gap(pClient->GetShipSE(), pTarget) > range)
            return MakeError("EntityTargetTooDistant", {{"targetTypeName", new PyString(pTarget->GetName())}, {"distance", new PyInt(static_cast<int32>(range))}});
        //{'FullPath': u'UI/Messages', 'label': u'EntityCurrentlyMiningBody'}(u'{targetTypeName} is currently mining an asteroid and cannot take commands until it has finished. It will return home and await further orders when it is finished.')
        if (pDrone->GetAI()->IsMining())
            return DroneError("EntityCurrentlyMining", pDrone);
        return nullptr;
    }

    // a guard/assist order ends when the drone is given another
    void EndActivity(Client* pClient, DroneSE* pDrone) {
        uint8 order(pDrone->GetAI()->GetOrder());
        if ((order == DroneAI::Order::Guard) or (order == DroneAI::Order::Assist))
            SendActivity(pClient, pDrone->GetID(), 0, nullptr);
    }

    // the pilot a guard/assist/delegate command is about.  returns the error (for all drones), or nullptr
    PyTuple* CheckPilot(Client* pClient, SystemManager* pSystem, uint32 charID, Client*& pOther) {
        pOther = sEntityList.FindClientByCharID(charID);
        //{'FullPath': u'UI/Messages', 'label': u'EntityTargetCharNotPresentBody'}(u'The drone cannot be commanded with respect to {[character]targetChar.name} because they are not present in this solar system.')
        if ((pOther == nullptr) or (pOther->SystemMgr() != pSystem) or !pOther->IsInSpace() or (pOther->GetShipSE() == nullptr))
            return MakeError("EntityTargetCharNotPresent", {{"targetChar", new PyInt(charID)}});
        //{'FullPath': u'UI/Messages', 'label': u'EntityTargetCharInCapsuleBody'}(u'The drone cannot be commanded with respect to {targetChar} because the pilot is in a capsule.')
        if (pOther->InPod())
            return MakeError("EntityTargetCharInCapsule", {{"targetChar", new PyString(pOther->GetName())}});
        //{'FullPath': u'UI/Messages', 'label': u'EntityTargetMustBeFleetMemberBody'}(u'Drones can only accept that command if {targetOwner} is a member of your fleet, which they are not.')
        if ((pOther != pClient) and ((pClient->GetFleetID() == 0) or (pClient->GetFleetID() != pOther->GetFleetID())))
            return MakeError("EntityTargetMustBeFleetMember", {{"targetOwner", new PyString(pOther->GetName())}});
        return nullptr;
    }

    bool IsCombatAction(uint8 action) {
        return ((action == DroneAI::Action::Attack) or (action == DroneAI::Action::Web) or (action == DroneAI::Action::Ewar));
    }
}

PyResult EntityBound::CmdEngage(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    // ret = entity.CmdEngage(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdEngage()");
    call.Dump(DRONE__DUMP);
    Client* pClient(call.client);
    PyDict* errors = new PyDict();
    SystemEntity* pTarget(call.client->SystemMgr()->GetSE(targetID->value()));
    ShipSE* pShip(pClient->GetShipSE());
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(pClient, call.client->SystemMgr(), droneID, pDrone));
        if (err == nullptr)
            err = CheckTarget(pClient, pDrone, pTarget);
        if (err == nullptr) {
            uint8 action(pDrone->Stats().action);
            //{'FullPath': u'UI/Messages', 'label': u'EntityUnknownCommandBody'}(u'{targetTypeName} does not recognize the command you are trying to give it.')
            if ((action == DroneAI::Action::None) or (action == DroneAI::Action::Mine)) {
                err = DroneError("EntityUnknownCommand", pDrone);
            } else if (pShip->TargetMgr()->GetTarget(pTarget->GetID(), true) == nullptr) {
                //{'FullPath': u'UI/Messages', 'label': u'EntityTargetMustBeTargetedBody'}(u'{targetTypeName} requires the target be locked onto by you, which it is not.')
                err = DroneError("EntityTargetMustBeTargeted", pDrone);
            }
        }
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        EndActivity(pClient, pDrone);
        if ((pDrone->GetAI()->GetOrder() == DroneAI::Order::Guard) or (pDrone->GetAI()->GetOrder() == DroneAI::Order::Assist))
            pDrone->GetAI()->SetIdle();
        pDrone->GetAI()->Engage(pTarget);
    }
    return errors;
}

PyResult EntityBound::CmdMine(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    // ret = entity.CmdMine(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdMine()");
    return Mine(call, droneIDs, targetID, false);
}

PyResult EntityBound::CmdMineRepeatedly(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    // ret = entity.CmdMineRepeatedly(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdMineRepeatedly()");
    return Mine(call, droneIDs, targetID, true);
}

PyResult EntityBound::Mine(PyCallArgs &call, PyList* droneIDs, PyInt* targetID, bool repeat) {
    call.Dump(DRONE__DUMP);
    Client* pClient(call.client);
    PyDict* errors = new PyDict();
    SystemEntity* pTarget(call.client->SystemMgr()->GetSE(targetID->value()));
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(pClient, call.client->SystemMgr(), droneID, pDrone));
        if (err == nullptr)
            err = CheckTarget(pClient, pDrone, pTarget);
        if (err == nullptr) {
            if (pDrone->Stats().action != DroneAI::Action::Mine) {
                err = DroneError("EntityUnknownCommand", pDrone);
            } else if (!pTarget->IsAsteroidSE()) {
                //{'FullPath': u'UI/Messages', 'label': u'EntityInvalidTargetBody'}(u'{targetTypeName} can only perform that action on an item of group  {desiredTarget}.')
                err = MakeError("EntityInvalidTarget", {{"targetTypeName", new PyString(pDrone->GetName())}, {"desiredTarget", new PyString("Asteroid")}});
            }
        }
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        EndActivity(pClient, pDrone);
        pDrone->GetAI()->Mine(pTarget, repeat);
    }
    return errors;
}

PyResult EntityBound::CmdUnanchor(PyCallArgs &call, PyList* droneIDs, PyInt* targetID) {
    // ret = entity.CmdUnanchor(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdUnanchor()");
    call.Dump(DRONE__DUMP);
    // no unanchoring drones yet
    PyDict* errors = new PyDict();
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(call.client, call.client->SystemMgr(), droneID, pDrone));
        errors->SetItem(new PyInt(droneID), (err != nullptr ? err : DroneError("EntityUnknownCommand", pDrone)));
    }
    return errors;
}

PyResult EntityBound::CmdReturnHome(PyCallArgs &call, PyList* droneIDs) {
    // ret = entity.CmdReturnHome(droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReturnHome()");
    call.Dump(DRONE__DUMP);
    PyDict* errors = new PyDict();
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(call.client, call.client->SystemMgr(), droneID, pDrone));
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        EndActivity(call.client, pDrone);
        pDrone->GetAI()->ReturnHome();
    }
    return errors;
}

PyResult EntityBound::CmdReturnBay(PyCallArgs &call, PyList* droneIDs) {
    // ret = entity.CmdReturnBay(droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReturnBay()");
    call.Dump(DRONE__DUMP);
    PyDict* errors = new PyDict();
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(call.client, call.client->SystemMgr(), droneID, pDrone));
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        EndActivity(call.client, pDrone);
        pDrone->GetAI()->ReturnToBay();
    }
    return errors;
}

PyResult EntityBound::CmdAbandonDrone(PyCallArgs &call, PyList* droneIDs) {
    // ret = entity.CmdAbandonDrone(droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAbandonDrone()");
    call.Dump(DRONE__DUMP);
    PyDict* errors = new PyDict();
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(call.client, call.client->SystemMgr(), droneID, pDrone));
        if ((err == nullptr) and (pDrone->GetOwnerID() != static_cast<uint32>(call.client->GetCharacterID())))
            err = DroneError("EntityNotYoursToCommand", pDrone);   // only the owner can abandon it
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        pDrone->Abandon();
    }
    return errors;
}

// returns {errorName: args} rather than per-drone errors
PyResult EntityBound::CmdReconnectToDrones(PyCallArgs &call, PyList* droneCandidates) {
    // ret = entity.CmdReconnectToDrones(droneCandidates)
    //     for errStr, dicty in ret.iteritems():
    // this sends a list of drones in local space owned by calling character
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReconnectToDrones()");
    call.Dump(DRONE__DUMP);
    Client* pClient(call.client);
    PyDict* errors = new PyDict();
    ShipSE* pShip(pClient->GetShipSE());
    if ((pShip == nullptr) or pClient->InPod())
        return errors;
    uint32 charID(pClient->GetCharacterID());
    uint32 maxDrones(pClient->GetChar()->GetAttribute(AttrMaxActiveDrones).get_uint32());
    double range(PilotControlRange(pClient));
    double bandwidth(pShip->GetShipItemRef()->GetAttribute(AttrDroneBandwidth).get_double());

    for (auto droneID : GetIDs(droneCandidates)) {
        SystemEntity* pSE(call.client->SystemMgr()->GetSE(droneID));
        if ((pSE == nullptr) or !pSE->IsDroneSE())
            continue;
        DroneSE* pDrone(pSE->GetDroneSE());
        if ((pDrone->GetOwnerID() != charID) or pDrone->IsControlled())
            continue;
        if ((pDrone->SysBubble() != pShip->SysBubble()) or (Gap(pDrone, pShip) > range)) {
            PyDict* args = new PyDict();
                args->SetItemString("targetTypeName", new PyString(pDrone->GetName()));
                args->SetItemString("distance", new PyInt(static_cast<int32>(range)));
            errors->SetItemString("EntityDistantCommand", args);
            continue;
        }
        if (DroneSE::CountControlledBy(call.client->SystemMgr(), charID) >= maxDrones) {
            PyDict* args = new PyDict();
                args->SetItemString("item", new PyInt(pDrone->GetSelf()->typeID()));
                args->SetItemString("limit", new PyInt(maxDrones));
            errors->SetItemString("NoDroneManagementAbilitiesLeft", args);
            break;
        }
        double used(DroneSE::BandwidthUsedBy(call.client->SystemMgr(), pShip->GetID()));
        if (used + pDrone->Stats().bandwidth > bandwidth) {
            PyDict* args = new PyDict();
                args->SetItemString("droneName", new PyString(pDrone->GetName()));
                args->SetItemString("droneBandwidthUsed", new PyInt(static_cast<int32>(pDrone->Stats().bandwidth)));
                args->SetItemString("bandwidthLeft", new PyInt(static_cast<int32>(bandwidth - used)));
            errors->SetItemString("MaxBandwidthExceeded2", args);
            continue;
        }
        pDrone->SetController(pShip);
    }
    return errors;
}

PyResult EntityBound::CmdRelinquishControl(PyCallArgs &call, PyList* IDs) {
    // ret = entity.CmdRelinquishControl(droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdRelinquishControl()");
    call.Dump(DRONE__DUMP);
    PyDict* errors = new PyDict();
    for (auto droneID : GetIDs(IDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(call.client, call.client->SystemMgr(), droneID, pDrone));
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        // control goes back to the owner if they can take it (see DroneSE::ReleaseControl())
        if (pDrone->GetOwnerID() != static_cast<uint32>(call.client->GetCharacterID())) {
            EndActivity(call.client, pDrone);
            pDrone->ReleaseControl();
        }
    }
    return errors;
}

PyResult EntityBound::CmdDelegateControl(PyCallArgs &call, PyList* droneIDs, PyInt* controllerID) {
    // ret = entity.CmdDelegateControl(droneIDs, charID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdDelegateControl()");
    call.Dump(DRONE__DUMP);
    Client* pClient(call.client);
    PyDict* errors = new PyDict();
    Client* pOther(nullptr);
    PyTuple* pilotErr(CheckPilot(pClient, call.client->SystemMgr(), controllerID->value(), pOther));
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(pClient, call.client->SystemMgr(), droneID, pDrone));
        if ((err == nullptr) and (pilotErr != nullptr)) {
            err = pilotErr;
            PyIncRef(pilotErr);
        }
        if (err == nullptr) {
            uint32 maxDrones(pOther->GetChar()->GetAttribute(AttrMaxActiveDrones).get_uint32());
            if (pDrone->GetOwnerID() != static_cast<uint32>(pClient->GetCharacterID())) {
                err = DroneError("EntityNotYoursToCommand", pDrone);
            } else if (pOther == pClient) {
                //{'FullPath': u'UI/Messages', 'label': u'EntityTargetAlreadyHasControlBody'}(u'Control of the {item} cannot be delegated to {whom} because they already have control of it.')
                err = MakeError("EntityTargetAlreadyHasControl", {{"item", new PyString(pDrone->GetName())}, {"whom", new PyString(pOther->GetName())}});
            } else if (maxDrones < 1) {
                //{'FullPath': u'UI/Messages', 'label': u'EntityNoTargetDroneManagementAbilitiesBody'}(u'Control of the {item} cannot be delegated to {whom} because they do not have the skill to control any drones.')
                err = MakeError("EntityNoTargetDroneManagementAbilities", {{"item", new PyString(pDrone->GetName())}, {"whom", new PyString(pOther->GetName())}});
            } else if (DroneSE::CountControlledBy(call.client->SystemMgr(), pOther->GetCharacterID()) >= maxDrones) {
                //{'FullPath': u'UI/Messages', 'label': u'EntityNoTargetDroneManagementAbilitiesLeftBody'}(u'Control of the {item} cannot be delegated to {whom} because they only have the skill to control {[numeric]limit} drones and they are already controlling that many.')
                err = MakeError("EntityNoTargetDroneManagementAbilitiesLeft", {{"item", new PyString(pDrone->GetName())}, {"whom", new PyString(pOther->GetName())},
                                                                               {"limit", new PyInt(maxDrones)}});
            }
        }
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        EndActivity(pClient, pDrone);
        pDrone->SetController(pOther->GetShipSE());
    }
    if (pilotErr != nullptr)
        PyDecRef(pilotErr);
    return errors;
}

PyResult EntityBound::CmdAssist(PyCallArgs &call, PyInt* assistID, PyList* droneIDs) {
    // ret = entity.CmdAssist(charID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAssist()");
    return GuardOrAssist(call, assistID->value(), droneIDs, false);
}

PyResult EntityBound::CmdGuard(PyCallArgs &call, PyInt* guardID, PyList* droneIDs) {
    // ret = entity.CmdGuard(charID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdGuard()");
    return GuardOrAssist(call, guardID->value(), droneIDs, true);
}

PyResult EntityBound::GuardOrAssist(PyCallArgs &call, uint32 charID, PyList* droneIDs, bool guard) {
    call.Dump(DRONE__DUMP);
    Client* pClient(call.client);
    PyDict* errors = new PyDict();
    Client* pOther(nullptr);
    PyTuple* pilotErr(CheckPilot(pClient, call.client->SystemMgr(), charID, pOther));
    for (auto droneID : GetIDs(droneIDs)) {
        DroneSE* pDrone(nullptr);
        PyTuple* err(CheckDrone(pClient, call.client->SystemMgr(), droneID, pDrone));
        if ((err == nullptr) and (pilotErr != nullptr)) {
            err = pilotErr;
            PyIncRef(pilotErr);
        }
        if ((err == nullptr) and !IsCombatAction(pDrone->Stats().action))
            err = DroneError("EntityUnknownCommand", pDrone);
        if (err != nullptr) {
            errors->SetItem(new PyInt(droneID), err);
            continue;
        }
        if (guard) {
            pDrone->GetAI()->Guard(charID);
        } else {
            pDrone->GetAI()->Assist(charID);
        }
        SendActivity(pClient, droneID, charID, (guard ? "guard" : "assist"));
    }
    if (pilotErr != nullptr)
        PyDecRef(pilotErr);
    return errors;
}
