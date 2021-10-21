
 /**
  * @name FleetProxy.cpp
  *     Fleet Proxy code for EVEmu
  *     This file deals with very generic fleet functions
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  * @finished:      02 December 2017
  *
  */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "fleet/FleetProxy.h"

PyCallable_Make_InnerDispatcher(FleetProxy)

FleetProxy::FleetProxy(PyServiceMgr *mgr)
: PyService(mgr, "fleetProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FleetProxy, GetAvailableFleets);
    PyCallable_REG_CALL(FleetProxy, AddFleetFinderAdvert);
    PyCallable_REG_CALL(FleetProxy, RemoveFleetFinderAdvert);
    PyCallable_REG_CALL(FleetProxy, GetMyFleetFinderAdvert);
    PyCallable_REG_CALL(FleetProxy, UpdateAdvertInfo);
    PyCallable_REG_CALL(FleetProxy, ApplyToJoinFleet);
}

FleetProxy::~FleetProxy()
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
PyResult FleetProxy::Handle_GetAvailableFleets(PyCallArgs &call) {
    return sFltSvc.GetAvailableFleets();
}

PyResult FleetProxy::Handle_ApplyToJoinFleet(PyCallArgs &call) {
  // ret = sm.ProxySvc('fleetProxy').ApplyToJoinFleet(fleetID)
   // sLog.White("FleetProxy", "Handle_ApplyToJoinFleet() size=%li", call.tuple->size());
   // call.Dump(FLEET__DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode args.", call.client->GetChar()->name());
        return new PyBool(false);
    }

    // returns boolean
    return new PyBool(sFltSvc.AddJoinRequest(arg.arg, call.client));
}

    // this is also used to update advert info
PyResult FleetProxy::Handle_AddFleetFinderAdvert(PyCallArgs &call) {
 //  sm.ProxySvc('fleetProxy').AddFleetFinderAdvert(info)
    //sLog.White("FleetProxy", "Handle_AddFleetFinderAdvert() size=%li", call.tuple->size());
    //call.Dump(FLEET__DUMP);

    /** @todo  this needs to be updated....check standings and verifly scope */

    FleetAdvertCall args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode args.", call.client->GetChar()->name());
        return nullptr;
    }

    int32 fleetID = call.client->GetChar()->fleetID();

    FleetData fData = FleetData();
    sFltSvc.GetFleetData(fleetID, fData);

    FleetAdvert adata = FleetAdvert();
        adata.fleetID = fleetID;
        adata.hideInfo = args.hideInfo;
        adata.inviteScope = args.inviteScope;
        adata.leader = call.client;
        adata.fleetName = PyRep::StringContent(args.fleetName);
        adata.advertTime = GetFileTimeNow();
        adata.dateCreated = fData.dateCreated;
        adata.description = PyRep::StringContent(args.description);
        adata.solarSystemID = call.client->GetSystemID();
        adata.joinNeedsApproval = args.joinNeedsApproval;
        adata.local_minSecurity = args.local_minSecurity;
        adata.local_minStanding = args.local_minStanding;
        adata.public_minSecurity = args.public_minSecurity;
        adata.public_minStanding = args.public_minStanding;
    PyList* localList = args.local_allowedEntities->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();
        adata.local_allowedEntities.clear();
    for (PyList::const_iterator itr = localList->begin(); itr != localList->end(); ++itr)
        adata.local_allowedEntities.push_back(PyRep::IntegerValueU32(*itr));
    PyList* publicList = args.public_allowedEntities->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();
        adata.public_allowedEntities.clear();
    for (PyList::const_iterator itr = publicList->begin(); itr != publicList->end(); ++itr)
        adata.public_allowedEntities.push_back(PyRep::IntegerValueU32(*itr));
    sFltSvc.CreateFleetAdvert(fleetID, adata);

    return nullptr;
}

PyResult FleetProxy::Handle_RemoveFleetFinderAdvert(PyCallArgs &call) {
   // sLog.White("FleetProxy", "Handle_RemoveFleetFinderAdvert() size=%li", call.tuple->size());
    //call.Dump(FLEET__DUMP);

    sFltSvc.RemoveFleetAdvert(call.client->GetChar()->fleetID());

    return nullptr;
}

PyResult FleetProxy::Handle_GetMyFleetFinderAdvert(PyCallArgs &call) {
   // sLog.White("FleetProxy", "Handle_GetMyFleetFinderAdvert() size=%li", call.tuple->size());
   // call.Dump(FLEET__DUMP);

    return sFltSvc.GetFleetAdvert(call.client->GetChar()->fleetID());
}

PyResult FleetProxy::Handle_UpdateAdvertInfo(PyCallArgs &call) {
  //   sm.ProxySvc('fleetProxy').UpdateAdvertInfo(numMembers)
    // this call just updates member count in fleet advert.  not needed here, as that is dynamic data.
    //sLog.White("FleetProxy", "Handle_UpdateAdvertInfo() size=%li", call.tuple->size());
    //call.Dump(FLEET__DUMP);

    return nullptr;
}
