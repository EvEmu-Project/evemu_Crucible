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
    Author:        Allan
*/

//work in progress

#include "eve-server.h"

#include "PyServiceCD.h"
#include "fleet/FleetManager.h"

PyCallable_Make_InnerDispatcher(FleetManager)

FleetManager::FleetManager(PyServiceMgr *mgr)
: PyService(mgr, "fleetMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FleetManager, ForceLeaveFleet);
    PyCallable_REG_CALL(FleetManager, AddToWatchlist);
    PyCallable_REG_CALL(FleetManager, RemoveFromWatchlist);
    PyCallable_REG_CALL(FleetManager, RegisterForDamageUpdates);
    PyCallable_REG_CALL(FleetManager, GetActiveStatus);
    PyCallable_REG_CALL(FleetManager, BroadcastToBubble);
    PyCallable_REG_CALL(FleetManager, BroadcastToSystem);
}

FleetManager::~FleetManager()
{
    delete m_dispatch;
}

PyResult FleetManager::Handle_ForceLeaveFleet(PyCallArgs &call) {
    sLog.Log("FleetManager", "Handle_ForceLeaveFleet() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetManager::Handle_AddToWatchlist(PyCallArgs &call) {
  /**
        sm.RemoteSvc('fleetMgr').AddToWatchlist(charID, fav)
        */
    sLog.Log("FleetManager", "Handle_AddToWatchlist() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetManager::Handle_RemoveFromWatchlist(PyCallArgs &call) {
  /**
        sm.RemoteSvc('fleetMgr').RemoveFromWatchlist(charID, fav)
        */
    sLog.Log("FleetManager", "Handle_RemoveFromWatchlist() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetManager::Handle_RegisterForDamageUpdates(PyCallArgs &call) {
  /**
        sm.RemoteSvc('fleetMgr').RegisterForDamageUpdates(fav)

        17:38:00 [SvcCall] Service fleetMgr: calling RegisterForDamageUpdates
        17:38:00 [SvcCall]   Call Arguments:
        17:38:00 [SvcCall]       Tuple: 1 elements
        17:38:00 [SvcCall]         [ 0] List: Empty
        17:38:00 [SvcCall]   Call Named Arguments:
        17:38:00 [SvcCall]     Argument 'machoVersion':
        17:38:00 [SvcCall]         Integer field: 1
        */
    sLog.Log("FleetManager", "Handle_RegisterForDamageUpdates() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}


PyResult FleetManager::Handle_GetActiveStatus(PyCallArgs &call) {
  /**
            self.activeStatus = sm.RemoteSvc('fleetMgr').GetActiveStatus()

            17:38:00 [SvcCall] Service fleetMgr: calling GetActiveStatus
            17:38:00 [SvcCall]   Call Arguments:
            17:38:00 [SvcCall]       Tuple: Empty
            17:38:00 [SvcCall]   Call Named Arguments:
            17:38:00 [SvcCall]     Argument 'machoVersion':
            17:38:00 [SvcCall]         Integer field: 1
        */
  /*
    [PyTuple 1 items]  <- from server
      [PySubStream 67 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 3 kvp]
            [PyString "fleet"]
            [PyInt 254]             <- spaces left (out of 256)
            [PyString "wings"]
            [PyDict 1 kvp]
              [PyIntegerVar 2013710700300]      <- wing id
              [PyInt 0]             <- current member count
            [PyString "squads"]
            [PyDict 1 kvp]
              [PyIntegerVar 3014610700300]      <- squad id
              [PyInt -1]            <- current member count

    */
    sLog.Log("FleetManager", "Handle_GetActiveStatus() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetManager::Handle_BroadcastToBubble(PyCallArgs &call) {
  /**
        sm.RemoteSvc('fleetMgr').BroadcastToBubble(name, self.broadcastScope, itemID)
        */
    sLog.Log("FleetManager", "Handle_BroadcastToBubble() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

PyResult FleetManager::Handle_BroadcastToSystem(PyCallArgs &call) {
  /**
        sm.RemoteSvc('fleetMgr').BroadcastToSystem(name, self.broadcastScope, itemID)
        */
    sLog.Log("FleetManager", "Handle_BroadcastToSystem() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return NULL;
}

