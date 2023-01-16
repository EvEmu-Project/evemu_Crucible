
 /**
  * @name FleetBound.h
  *     Fleet Bound code for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */


#ifndef EVEMU_SRC_FLEET_BOUND_H_
#define EVEMU_SRC_FLEET_BOUND_H_


#include "services/BoundService.h"
#include "fleet/FleetManager.h"

class FleetObject;

class FleetBound : public EVEBoundObject <FleetBound>
{
public:
    FleetBound(EVEServiceManager& mgr, FleetObject& parent, uint32 fleetID);

    uint32 GetFleetID() { return this->m_fleetID; }

protected:
    bool CanClientCall(Client* client) override;

    PyResult Init(PyCallArgs& call, std::optional <PyInt*> shipTypeID);
    PyResult GetInitState(PyCallArgs& call);
    PyResult GetFleetID(PyCallArgs& call);
    PyResult Invite(PyCallArgs& call, PyInt* characterID, std::optional<PyInt*> wingID, std::optional <PyInt*> squadID, PyLong* role);
    PyResult AcceptInvite(PyCallArgs& call, std::optional <PyInt*> shipTypeID);
    PyResult RejectInvite(PyCallArgs& call);
    PyResult ChangeWingName(PyCallArgs& call, PyInt* wingID, PyRep* name);
    PyResult ChangeSquadName(PyCallArgs& call, PyInt* squadID, PyRep* name);
    PyResult SetOptions(PyCallArgs& call, PyObject* options);
    PyResult GetJoinRequests(PyCallArgs& call);
    PyResult RejectJoinRequest(PyCallArgs& call, PyInt* characterID);
    PyResult AddToVoiceChat(PyCallArgs& call, PyRep* channelName);
    PyResult SetVoiceMuteStatus(PyCallArgs& call, PyRep* status, PyRep* channel);
    PyResult ExcludeFromVoiceMute(PyCallArgs& call, PyInt* characterID, PyRep* channel);
    PyResult GetFleetComposition(PyCallArgs& call);
    PyResult GetWings(PyCallArgs& call);
    PyResult SendBroadcast(PyCallArgs& call, PyRep* message, PyInt* group, PyInt* itemID);
    PyResult UpdateMemberInfo(PyCallArgs& call, std::optional <PyInt*> shipTypeID);
    PyResult SetMotdEx(PyCallArgs& call, PyRep* motd);
    PyResult GetMotd(PyCallArgs& call);
    PyResult LeaveFleet(PyCallArgs& call);
    PyResult MakeLeader(PyCallArgs& call, PyInt* characterID);
    PyResult SetBooster(PyCallArgs& call, PyInt* characterID, std::optional <PyInt*> roleBooster);
    PyResult MoveMember(PyCallArgs& call, PyInt* characterID, std::optional <PyInt*> wingID, std::optional <PyInt*> squadID, std::optional <PyInt*> role, std::optional <PyInt*> booster);
    PyResult KickMember(PyCallArgs& call, PyInt* characterID);
    PyResult CreateWing(PyCallArgs& call);
    PyResult CreateSquad(PyCallArgs& call, PyInt* wingID);
    PyResult DeleteWing(PyCallArgs& call, PyInt* wingID);
    PyResult DeleteSquad(PyCallArgs& call, PyInt* squadID);
    PyResult Reconnect(PyCallArgs& call);

private:
    uint32 m_fleetID;

};

#endif  // EVEMU_SRC_FLEET_BOUND_H_
