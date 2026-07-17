/**
  * @name EntityService.h
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */

#ifndef __EVEMU_NPC_ENTITY_H
#define __EVEMU_NPC_ENTITY_H

#include "../eve-server.h"
#include "services/BoundService.h"
#include "Client.h"
#include "ship/Ship.h"
#include "system/SystemEntity.h"

class SystemManager;
class EntityBound;
class DroneSE;
class ShipSE;

class EntityService : public BindableService <EntityService, EntityBound>
{
public:
    EntityService(EVEServiceManager& mgr);

    void BoundReleased (EntityBound* bound) override;
protected:
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters);

private:
    std::map <uint32, EntityBound*> m_instances;
};

class EntityBound : public EVEBoundObject <EntityBound>
{
public:
    EntityBound(EVEServiceManager& mgr, EntityService& parent, SystemManager* systemMgr, uint32 systemID);

    uint32 GetSystemID() { return this->m_systemID; }



private:
    DroneSE* GetValidDrone(Client* client, uint32 droneID);
    SystemEntity* GetValidTarget(Client* client, uint32 targetID);
    ShipSE* GetPlayerShip(Client* client);

protected:
    PyResult CmdEngage(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdRelinquishControl(PyCallArgs& call, PyList* IDs);
    PyResult CmdDelegateControl(PyCallArgs& call, PyList* droneIDs, PyInt* controllerID);
    PyResult CmdAssist(PyCallArgs& call, PyInt* assistID, PyList* droneIDs);
    PyResult CmdGuard(PyCallArgs& call, PyInt* guardID, PyList* droneIDs);
    PyResult CmdMine(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdMineRepeatedly(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdUnanchor(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdReturnHome(PyCallArgs& call, PyList* droneIDs);
    PyResult CmdReturnBay(PyCallArgs& call, PyList* droneIDs);
    PyResult CmdAbandonDrone(PyCallArgs& call, PyList* droneIDs);
    PyResult CmdReconnectToDrones(PyCallArgs& call, PyList* droneCandidates);

protected:
    SystemManager* m_sysMgr;
    uint32 m_systemID;
};

#endif  // __EVEMU_NPC_ENTITY_H