
 /**
  * @name EntityService.cpp
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"

#include "EVEServerConfig.h"
#include "npc/EntityService.h"
#include "system/SystemManager.h"


PyCallable_Make_InnerDispatcher(EntityService)

EntityService::EntityService(PyServiceMgr *mgr)
: PyService(mgr, "entity"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

}

EntityService::~EntityService() {
    delete m_dispatch;
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
PyBoundObject *EntityService::CreateBoundObject(Client* pClient, const PyRep* bind_args) {
    _log(DRONE__DUMP, "EntityService bind request");
    bind_args->Dump(DRONE__DUMP, "    ");
    if (!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s: Non-integer bind argument '%s'", pClient->GetName(), bind_args->TypeString());
        return nullptr;
    }

    uint32 systemID = bind_args->AsInt()->value();
    if (!IsSolarSystem(systemID)) {
        codelog(SERVICE__ERROR, "%s: Expected systemID, but got %u.", pClient->GetName(), systemID);
        return nullptr;
    }

    return new EntityBound(m_manager, pClient->SystemMgr(), systemID);
}

EntityBound::EntityBound(PyServiceMgr *mgr, SystemManager* systemMgr, uint32 systemID)
: PyBoundObject(mgr),
m_sysMgr(systemMgr),
m_systemID(systemID),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "EntityBound";

    PyCallable_REG_CALL(EntityBound, CmdEngage);
    PyCallable_REG_CALL(EntityBound, CmdRelinquishControl);
    PyCallable_REG_CALL(EntityBound, CmdDelegateControl);
    PyCallable_REG_CALL(EntityBound, CmdAssist);
    PyCallable_REG_CALL(EntityBound, CmdGuard);
    PyCallable_REG_CALL(EntityBound, CmdMine);
    PyCallable_REG_CALL(EntityBound, CmdMineRepeatedly);
    PyCallable_REG_CALL(EntityBound, CmdUnanchor);
    PyCallable_REG_CALL(EntityBound, CmdReturnHome);
    PyCallable_REG_CALL(EntityBound, CmdReturnBay);
    PyCallable_REG_CALL(EntityBound, CmdAbandonDrone);
    PyCallable_REG_CALL(EntityBound, CmdReconnectToDrones);
}

PyResult EntityBound::Handle_CmdEngage(PyCallArgs &call) {
 // ret = entity.CmdEngage(droneIDs, targetID)
    /*
        [PySubStream 104 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoBindObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyTuple 3 items]
                [PyString "CmdEngage"]
                [PyTuple 2 items]
                  [PyList 5 items]
                    [PyIntegerVar 1005909162494]
                    [PyIntegerVar 1005902743336]
                    [PyIntegerVar 1005909162497]
                    [PyIntegerVar 1005909162499]
                    [PyIntegerVar 1005909162492]
                  [PyIntegerVar 9000000000001190095]
                [PyDict 0 kvp]
                */
    /*
      [PySubStream 258 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 31 bytes]
              [PyTuple 2 items]
                [PyString "N=790408:2886"]
                [PyIntegerVar 129756560501538126]
          [PyDict 5 kvp]
            [PyIntegerVar 1005909162494]
            [PyTuple 2 items]
              [PyString "EntityTargetTooDistant"]
              [PyDict 2 kvp]
                [PyString "targetTypeName"]
                [PyTuple 2 items]
                  [PyInt 7]
                  [PyInt 561]
                [PyString "distance"]
                [PyFloat 45000]
                */
    /*
      [PySubStream 42 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 31 bytes]
              [PyTuple 2 items]
                [PyString "N=790408:2886"]
                [PyIntegerVar 129756560847182701]
          [PyDict 0 kvp]
          */
    /*
    if (tSE->SysBubble()->HasTower()) {
        TowerSE* ptSE = tSE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField())
            if (tSE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI()) {
                std::map<std::string, PyRep *> arg;
                arg["target"] = new PyInt(args.arg);
                throw PyException( MakeUserError("DeniedDroneTargetForceField", arg ));
            }
    }
        */

    _log(DRONE__TRACE, "EntityBound::Handle_CmdEngage()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdRelinquishControl(PyCallArgs &call) {
 // ret = entity.CmdRelinquishControl(IDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdRelinquishControl()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdDelegateControl(PyCallArgs &call) {
 // ret = entity.CmdDelegateControl(droneIDs, controllerID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdDelegateControl()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdAssist(PyCallArgs &call) {
 // ret = entity.CmdAssist(assistID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAssist()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdGuard(PyCallArgs &call) {
 // ret = entity.CmdGuard(guardID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdGuard()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdMine(PyCallArgs &call) {
 // ret = entity.CmdMine(droneIDs, targetID)
    /*
     * 16:19:14 [DroneTrace] EntityBound::Handle_CmdMine()
     * 16:19:14 [DroneDump]   Call Arguments:
     * 16:19:14 [DroneDump]      Tuple: 2 elements
     * 16:19:14 [DroneDump]       [ 0]   List: 5 elements
     * 16:19:14 [DroneDump]       [ 0]   [ 0]    Integer: 140024264
     * 16:19:14 [DroneDump]       [ 0]   [ 1]    Integer: 140024265
     * 16:19:14 [DroneDump]       [ 0]   [ 2]    Integer: 140024261
     * 16:19:14 [DroneDump]       [ 0]   [ 3]    Integer: 140024262
     * 16:19:14 [DroneDump]       [ 0]   [ 4]    Integer: 140024263
     * 16:19:14 [DroneDump]       [ 1]    Integer: 450000587
     */

    _log(DRONE__TRACE, "EntityBound::Handle_CmdMine()");
    call.Dump(DRONE__DUMP);

    /** @todo MAKE CHECKS IN MINING LASER FOR DRONES BEFORE COMPLETING THIS FUNCTION  **/

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdMineRepeatedly(PyCallArgs &call) {
 // ret = entity.CmdMineRepeatedly(droneIDs, targetID)
    /*)
     * 16:20:28 [DroneTrace] EntityBound::Handle_CmdMineRepeatedly()
     * 16:20:28 [DroneDump]   Call Arguments:
     * 16:20:28 [DroneDump]      Tuple: 2 elements
     * 16:20:28 [DroneDump]       [ 0]   List: 5 elements
     * 16:20:28 [DroneDump]       [ 0]   [ 0]    Integer: 140024264
     * 16:20:28 [DroneDump]       [ 0]   [ 1]    Integer: 140024265
     * 16:20:28 [DroneDump]       [ 0]   [ 2]    Integer: 140024261
     * 16:20:28 [DroneDump]       [ 0]   [ 3]    Integer: 140024262
     * 16:20:28 [DroneDump]       [ 0]   [ 4]    Integer: 140024263
     * 16:20:28 [DroneDump]       [ 1]    Integer: 450000587
     */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdMineRepeatedly()");
    call.Dump(DRONE__DUMP);

    /** @todo MAKE CHECKS IN MINING LASER FOR DRONES BEFORE COMPLETING THIS FUNCTION  **/

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdUnanchor(PyCallArgs &call) {
 // ret = entity.CmdUnanchor(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdUnanchor()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdReturnHome(PyCallArgs &call) {
 // ret = entity.CmdReturnHome(droneIDs)
    // this is return and orbit command
    /*
02:18:26 [DroneTrace] EntityBound::Handle_CmdReturnHome()
02:18:26 [DroneDump]   Call Arguments:
02:18:26 [DroneDump]      Tuple: 1 elements
02:18:26 [DroneDump]       [ 0]   List: 1 elements
02:18:26 [DroneDump]       [ 0]   [ 0]    Integer: 140001219
*/
    call.Dump(DRONE__DUMP);

    //Drone* pDrone = m_sysMgr->GetSE()->GetDroneSE();
    //pDrone->DestinyMgr()->Orbit(pShipSE, 800);


    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdReturnBay(PyCallArgs &call) {
 // ret = entity.CmdReturnBay(droneIDs)
    /*
        [PySubStream 97 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoBindObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyTuple 3 items]
                [PyString "CmdReturnBay"]
                [PyTuple 1 items]
                  [PyList 5 items]
                    [PyIntegerVar 1005909162494]
                    [PyIntegerVar 1005902743336]
                    [PyIntegerVar 1005909162497]
                    [PyIntegerVar 1005909162499]
                    [PyIntegerVar 1005909162492]
                [PyDict 0 kvp]

    [PyTuple 1 items]
      [PySubStream 42 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 31 bytes]
              [PyTuple 2 items]
                [PyString "N=790408:2886"]
                [PyIntegerVar 129756563162318175]
          [PyDict 0 kvp]
          */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReturnBay()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");

    // returns nodeID and timestamp and dict of error msg
    /*
    PyDict* dict = new PyDict();
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    PySubStruct* str = new PySubStruct(new PySubStream(tuple));
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, str);
        tuple1->SetItem(1, dict);
    return tuple1;
    */
    return new PyDict();
}

PyResult EntityBound::Handle_CmdAbandonDrone(PyCallArgs &call) {
 // ret = entity.CmdAbandonDrone(droneIDs)
    /*
     * 16:23:23 [DroneTrace] EntityBound::Handle_CmdAbandonDrone()
     * 16:23:23 [DroneDump]   Call Arguments:
     * 16:23:23 [DroneDump]      Tuple: 1 elements
     * 16:23:23 [DroneDump]       [ 0]   List: 1 elements
     * 16:23:23 [DroneDump]       [ 0]   [ 0]    Integer: 140024263
     */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAbandonDrone()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdReconnectToDrones(PyCallArgs &call) {
    // ret = entity.CmdReconnectToDrones(droneCandidates)
    //     for errStr, dicty in ret.iteritems():
    // this sends a list of drones in local space owned by calling character
    /*
     * 09:09:48 [DroneDump]   Call Arguments:
     * 09:09:48 [DroneDump]      Tuple: 1 elements
     * 09:09:48 [DroneDump]       [ 0]   List: 1 elements
     * 09:09:48 [DroneDump]       [ 0]   [ 0]    Integer: 140007055
     */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReconnectToDrones()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}
