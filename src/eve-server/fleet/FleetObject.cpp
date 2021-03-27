/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Allan
*/

//work in progress

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "fleet/FleetObject.h"

class FleetBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(FleetBound)

    FleetBound(PyServiceMgr *mgr, Client *c)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "FleetBound";

        PyCallable_REG_CALL(FleetBound, Init);

    }
    virtual ~FleetBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(Init);

protected:
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(FleetObject)

FleetObject::FleetObject(PyServiceMgr *mgr)
: PyService(mgr, "fleetObjectHandler"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FleetObject, CreateFleet);
    PyCallable_REG_CALL(FleetObject, CreateWing);
    PyCallable_REG_CALL(FleetObject, CreateSquad);
    PyCallable_REG_CALL(FleetObject, SetMotdEx);
    PyCallable_REG_CALL(FleetObject, GetMotd);
    PyCallable_REG_CALL(FleetObject, CheckIsInFleet);
    PyCallable_REG_CALL(FleetObject, MakeLeader);
    PyCallable_REG_CALL(FleetObject, SetBooster);
    PyCallable_REG_CALL(FleetObject, MoveMember);
    PyCallable_REG_CALL(FleetObject, KickMember);
    PyCallable_REG_CALL(FleetObject, DeleteWing);
    PyCallable_REG_CALL(FleetObject, DeleteSquad);
    PyCallable_REG_CALL(FleetObject, LeaveFleet);
}

FleetObject::~FleetObject()
{
    delete m_dispatch;
}

PyBoundObject* FleetObject::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "FleetObjectHandler bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, "    " );

    return new FleetBound( m_manager, c );
}

PyResult FleetBound::Handle_Init(PyCallArgs &call) {

    sLog.Log("FleetBound", "Handle_Init() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_CreateFleet(PyCallArgs &call) {
    /*  object manager here sets fleet IDs (squad, wing, fleet)
     *   and tracks player movement within the fleet.
     *
     */
    //self.fleet = sm.RemoteSvc('fleetObjectHandler').CreateFleet()
    return m_Svc.CreateFleet(call.client);
}

PyResult FleetObject::Handle_CreateWing(PyCallArgs &call) {
    /*  wingID = self.fleet.CreateWing()  */

    sLog.Log("FleetObjectHandler", "Handle_CreateWing() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return m_Svc.CreateWing(call.client);
}

PyResult FleetObject::Handle_CreateSquad(PyCallArgs &call) {
    /* self.fleet.CreateSquad(wingID)  */

    sLog.Log("FleetObjectHandler", "Handle_CreateSquad() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return m_Svc.CreateSquad(call.client);
}

PyResult FleetObject::Handle_SetMotdEx(PyCallArgs &call) {
    /*  self.fleet.SetMotdEx(motd)  */

    sLog.Log("FleetObjectHandler", "Handle_SetMotdEx() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_GetMotd(PyCallArgs &call) {
    /*  self.fleet.GetMotd()  */

    sLog.Log("FleetObjectHandler", "Handle_GetMotd() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_CheckIsInFleet(PyCallArgs &call) {
    /* self.CheckIsInFleet()  */

    sLog.Log("FleetObjectHandler", "Handle_CheckIsInFleet() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_MakeLeader(PyCallArgs &call) {
    /* self.fleet.MakeLeader(charID)  */

    sLog.Log("FleetObjectHandler", "Handle_MakeLeader() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_SetBooster(PyCallArgs &call) {
    /*self.fleet.SetBooster(charID, roleBooster):
     *sm.ScatterEvent('OnFleetMemberChanging', charID)  */

    sLog.Log("FleetObjectHandler", "Handle_SetBooster() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_MoveMember(PyCallArgs &call) {
    /*  MoveMember(charID, wingID, squadID, role, roleBooster):  */

    sLog.Log("FleetObjectHandler", "Handle_MoveMember() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_KickMember(PyCallArgs &call) {
    /*
     *        if charID == eve.session.charid:
     *            self.LeaveFleet()
     *        else:
     *            self.fleet.KickMember(charID)
     */

    sLog.Log("FleetObjectHandler", "Handle_KickMember() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_DeleteWing(PyCallArgs &call) {
    /*    self.fleet.DeleteWing(wingID)  */

    sLog.Log("FleetObjectHandler", "Handle_DeleteWing() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_DeleteSquad(PyCallArgs &call) {
    /* self.fleet.DeleteSquad(wingID)  */

    sLog.Log("FleetObjectHandler", "Handle_DeleteSquad() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetObject::Handle_LeaveFleet(PyCallArgs &call) {
    /*
        if charID == eve.session.charid:
            self.LeaveFleet()
        else:
            self.fleet.KickMember(charID)
    */

    sLog.Log("FleetObjectHandler", "Handle_LeaveFleet() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    FleetMemberInfo fleet;

    fleet.fleetID = 0;
    fleet.wingID = 0;
    fleet.squadID = 0;
    fleet.fleetRole = 0;
    fleet.fleetBooster = 0;
    fleet.fleetJob = 0;

    //call updates on fleet session data
    call.client->GetChar().get()->SetFleetData(fleet);
    return NULL;
}


/**
typedef enum {
fleetJobNone = 0,
fleetJobScout = 1,
fleetJobCreator = 2
} FleetJobs;

typedef enum {
fleetRoleLeader = 1,
fleetRoleWingCmdr = 2,
fleetRoleSquadCmdr = 3,
fleetRoleMember = 4
} FleetRoles;

typedef enum {
fleetBoosterNone = 0,
fleetBoosterFleet = 1,
fleetBoosterWing = 2,
fleetBoosterSquad = 3
} FleetBoosters;

fleetGroupingRange = 300
rejectFleetInviteTimeout = 1
rejectFleetInviteAlreadyInFleet = 2
*/
