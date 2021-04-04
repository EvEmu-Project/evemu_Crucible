
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


#include "PyBoundObject.h"
#include "fleet/FleetManager.h"

class FleetBound
 : public PyBoundObject
{
  public:

     FleetBound(PyServiceMgr* mgr, uint32 fleetID);

     virtual ~FleetBound();

     virtual void Release() {
         //I hate this statement
         delete this;
     }

     PyCallable_DECL_CALL(Init);
     PyCallable_DECL_CALL(GetInitState);
     PyCallable_DECL_CALL(GetFleetID);
     PyCallable_DECL_CALL(Invite);
     PyCallable_DECL_CALL(AcceptInvite);
     PyCallable_DECL_CALL(RejectInvite);
     PyCallable_DECL_CALL(ChangeWingName);
     PyCallable_DECL_CALL(ChangeSquadName);
     PyCallable_DECL_CALL(SetOptions);
     PyCallable_DECL_CALL(GetJoinRequests);
     PyCallable_DECL_CALL(RejectJoinRequest);
     PyCallable_DECL_CALL(AddToVoiceChat);
     PyCallable_DECL_CALL(SetVoiceMuteStatus);
     PyCallable_DECL_CALL(ExcludeFromVoiceMute);
     PyCallable_DECL_CALL(GetFleetComposition);
     PyCallable_DECL_CALL(GetWings);
     PyCallable_DECL_CALL(SendBroadcast);
     PyCallable_DECL_CALL(UpdateMemberInfo);
     PyCallable_DECL_CALL(SetMotdEx);
     PyCallable_DECL_CALL(GetMotd);
     PyCallable_DECL_CALL(LeaveFleet);
     PyCallable_DECL_CALL(MakeLeader);
     PyCallable_DECL_CALL(SetBooster);
     PyCallable_DECL_CALL(MoveMember);
     PyCallable_DECL_CALL(KickMember);
     PyCallable_DECL_CALL(CreateWing);
     PyCallable_DECL_CALL(CreateSquad);
     PyCallable_DECL_CALL(DeleteWing);
     PyCallable_DECL_CALL(DeleteSquad);
     PyCallable_DECL_CALL(Reconnect);

  protected:
     class Dispatcher;
     Dispatcher *const m_dispatch;
     Client* m_client;

  private:
    uint32 m_fleetID;

};

#endif  // EVEMU_SRC_FLEET_BOUND_H_
