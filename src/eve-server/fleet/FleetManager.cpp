
 /**
  * @name FleetManager.cpp
  *     Fleet Manager code for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
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

    PyCallable_REG_CALL(FleetManager, GetActiveStatus);
    PyCallable_REG_CALL(FleetManager, ForceLeaveFleet);
    PyCallable_REG_CALL(FleetManager, BroadcastToBubble);
    PyCallable_REG_CALL(FleetManager, BroadcastToSystem);

    // stubs
    PyCallable_REG_CALL(FleetManager, AddToWatchlist);
    PyCallable_REG_CALL(FleetManager, RemoveFromWatchlist);
    PyCallable_REG_CALL(FleetManager, RegisterForDamageUpdates);
}

FleetManager::~FleetManager()
{
    delete m_dispatch;
}

/*
FLEET__ERROR
FLEET__WARNING
FLEET__MESSAGE
FLEET__DEBUG
FLEET__INFO
FLEET__TRACE
FLEET__DUMP
FLEET__BIND_DUMP
*/
PyResult FleetManager::Handle_ForceLeaveFleet(PyCallArgs &call) {
    sLog.Warning("FleetManager", "Handle_ForceLeaveFleet() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.LeaveFleet(call.client);

    // returns nothing
    return nullptr;
}

PyResult FleetManager::Handle_GetActiveStatus(PyCallArgs &call) {
  //   self.activeStatus = sm.RemoteSvc('fleetMgr').GetActiveStatus()
    // have seen this return PyNone in logs.  dont know why...bad fleetID maybe?
    sLog.Warning("FleetManager", "Handle_GetActiveStatus() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

  /*
    [PyTuple 1 items]                       <- from server
      [PySubStream 67 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 3 kvp]
            [PyString "fleet"]
            [PyInt 254]                     <- spaces left (out of 256)
            [PyString "wings"]
            [PyDict 1 kvp]
              [PyIntegerVar 2013710700300]  <- wing id
              [PyInt 0]                     <- isActive (integer bool)
            [PyString "squads"]
            [PyDict 1 kvp]
              [PyIntegerVar 3014610700300]  <- squad id
              [PyInt 1]                     <- isActive (integer bool)

    */

    int32 fleetID = call.client->GetChar()->fleetID();
    if (fleetID == 0)
        return PyStatic.NewNone();

    ActiveStatusRSP rsp;
    rsp.fleetCount = (255 - sFltSvc.GetFleetMemberCount(fleetID));     // this is slots left from max (256)

    PyDict* wings = new PyDict();
    PyDict* squads = new PyDict();

    std::vector< uint32 > wingIDs, squadIDs;
    sFltSvc.GetWingIDs(fleetID, wingIDs);
    for (auto wingID : wingIDs) {
        WingData wData = WingData();
        sFltSvc.GetWingData(wingID, wData);
        wings->SetItem(new PyInt(wingID), new PyInt((sFltSvc.IsWingActive(wingID) ? 1 : 0)));

        sFltSvc.GetSquadIDs(wingID, squadIDs);
        for (auto squadID : squadIDs) {
            SquadData sData = SquadData();
            sFltSvc.GetSquadData(squadID, sData);
            squads->SetItem(new PyInt(squadID), new PyInt(sData.members.size() > 0 ? 1 : 0));
        }
    }

    PySafeDecRef(rsp.wings);
    rsp.wings = wings;

    PySafeDecRef(rsp.squads);
    rsp.squads = squads;

    if (is_log_enabled(FLEET__DEBUG))
        rsp.Dump(FLEET__DEBUG);

    return rsp.Encode();
}

PyResult FleetManager::Handle_BroadcastToBubble(PyCallArgs &call) {
  //     sm.RemoteSvc('fleetMgr').BroadcastToBubble(name, self.broadcastScope, itemID)
    /*
     * 00:06:49 W FleetManager: Handle_BroadcastToSysBubble() size=3
     * 00:06:49 [FleetDump]   Call Arguments:
     * 00:06:49 [FleetDump]       Tuple: 3 elements
     * 00:06:49 [FleetDump]         [ 0] String: 'HealCapacitor'
     * 00:06:49 [FleetDump]         [ 1] Integer field: 3           <-- groupID
     * 00:06:49 [FleetDump]         [ 2] Integer field: 140006694   <-- charID
     */
    sLog.Warning("FleetManager", "Handle_BroadcastToSysBubble() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

    SendBroadCastCall args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode args.", call.client->GetChar()->name());
        return nullptr;
    }

    sFltSvc.FleetBroadcast(call.client, args.itemID, Fleet::BCast::Scope::Bubble, args.group, args.msg);

    return nullptr;
}

PyResult FleetManager::Handle_BroadcastToSystem(PyCallArgs &call) {
  //     sm.RemoteSvc('fleetMgr').BroadcastToSystem(name, self.broadcastScope, itemID)
    sLog.Warning("FleetManager", "Handle_BroadcastToSystem() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

    SendBroadCastCall args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode args.", call.client->GetChar()->name());
        return nullptr;
    }

    sFltSvc.FleetBroadcast(call.client, args.itemID, Fleet::BCast::Scope::System, args.group, args.msg);

    return nullptr;
}

PyResult FleetManager::Handle_AddToWatchlist(PyCallArgs &call) {
    /**
     *        sm.RemoteSvc('fleetMgr').AddToWatchlist(charID, fav)
     */
    sLog.Warning("FleetManager", "Handle_AddToWatchlist() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

    return nullptr;
}

PyResult FleetManager::Handle_RemoveFromWatchlist(PyCallArgs &call) {
    /**
     *        sm.RemoteSvc('fleetMgr').RemoveFromWatchlist(charID, fav)
     */
    sLog.Warning("FleetManager", "Handle_RemoveFromWatchlist() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

    return nullptr;
}

PyResult FleetManager::Handle_RegisterForDamageUpdates(PyCallArgs &call) {
    /**
        fav = self.GetWatchlistMembers()
        sm.RemoteSvc('fleetMgr').RegisterForDamageUpdates(fav)
     *
     *        17:38:00 [SvcCall] Service fleetMgr: calling RegisterForDamageUpdates
     *        17:38:00 [SvcCall]   Call Arguments:
     *        17:38:00 [SvcCall]       Tuple: 1 elements
     *        17:38:00 [SvcCall]         [ 0] List: Empty
     */
    sLog.Warning("FleetManager", "Handle_RegisterForDamageUpdates() size=%li", call.tuple->size());
    call.Dump(FLEET__DUMP);

    // returns nothing
    return nullptr;
}
