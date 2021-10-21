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

#include "eve-server.h"

#include "Client.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "Probes.h"
#include "exploration/ScanMgrService.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"

/* SCAN__ERROR
 * SCAN__WARNING
 * SCAN__MESSAGE
 * SCAN__DEBUG
 * SCAN__INFO
 * SCAN__TRACE
 * SCAN__DUMP
 */

class ScanBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(ScanBound)

    ScanBound(PyServiceMgr *mgr, Client *pClient)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_client(pClient)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "ScanBound";

        PyCallable_REG_CALL(ScanBound, ConeScan);
        PyCallable_REG_CALL(ScanBound, RequestScans);
        PyCallable_REG_CALL(ScanBound, RecoverProbes);
        PyCallable_REG_CALL(ScanBound, ReconnectToLostProbes);
        PyCallable_REG_CALL(ScanBound, DestroyProbe);

    }
        /**

        return sm.RemoteSvc('scanMgr').GetSystemScanMgr().ReconnectToLostProbes()

        successProbeIDs = sm.RemoteSvc('scanMgr').GetSystemScanMgr().RecoverProbes(probeIDs)

        sm.RemoteSvc('scanMgr').GetSystemScanMgr().RequestScans(probes)

        sm.RemoteSvc('scanMgr').GetSystemScanMgr().DestroyProbe(probeID)

        */

    virtual ~ScanBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(RequestScans);
    PyCallable_DECL_CALL(ConeScan);
    PyCallable_DECL_CALL(ReconnectToLostProbes);
    PyCallable_DECL_CALL(RecoverProbes);
    PyCallable_DECL_CALL(DestroyProbe);

protected:
    Dispatcher *const m_dispatch;
    Scan* m_scan;
    ScanningDB* m_db;
    Client* m_client;
};

PyCallable_Make_InnerDispatcher(ScanMgrService)

ScanMgrService::ScanMgrService(PyServiceMgr *mgr)
: PyService(mgr, "scanMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ScanMgrService, GetSystemScanMgr);

}

ScanMgrService::~ScanMgrService() {
    delete m_dispatch;
}

PyResult ScanMgrService::Handle_GetSystemScanMgr( PyCallArgs& call ) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    ScanBound* pSB = new ScanBound(m_manager, call.client);
    PyRep* result = m_manager->BindObject(call.client, pSB);
    return result;
}

PyResult ScanBound::Handle_ConeScan( PyCallArgs& call ) {
    //result = sm.GetService('scanSvc').ConeScan(self.scanangle, rnge * 1000, vec.x, vec.y, vec.z)
    //return sm.RemoteSvc('scanMgr').GetSystemScanMgr().ConeScan(scanangle, scanRange, x, y, z)
    //_log(SCAN__TRACE, "ScanBound::Handle_ConeScan() - size=%li", call.tuple->size());
    //call.Dump(SCAN__DUMP);

    Call_ConeScan args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        //TODO: throw exception
        return PyStatic.NewNone();
    }

    DestinyManager* pDestiny = m_client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", m_client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't scan while warping");
        return PyStatic.NewNone();
    }

    if (m_client->GetShipSE()->SysBubble() == nullptr)
        sBubbleMgr.Add(m_client->GetShipSE());

    if (m_client->scan() == nullptr)
        m_client->SetScan(new Scan(m_client));

    /** @todo  2s reset time on dscan */

    return m_client->scan()->ConeScan(args);
}

PyResult ScanBound::Handle_RequestScans( PyCallArgs& call ) {
    _log(SCAN__TRACE, "ScanBound::Handle_RequestScans() - size=%li", call.tuple->size());
    call.Dump(SCAN__DUMP);

    DestinyManager* pDestiny = m_client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't scan while warping");
        return PyStatic.NewNone();
    }

    if (m_client->GetShipSE()->SysBubble() == nullptr)
        sBubbleMgr.Add(m_client->GetShipSE());

    if (m_client->scan() == nullptr)
        m_client->SetScan(new Scan(m_client));

    PyDict* dict(nullptr);
    if (call.tuple->GetItem( 0 )->IsDict())
        dict = call.tuple->GetItem(0)->AsDict();

    m_client->scan()->RequestScans(dict);

    // this call returns a PyNone
    return PyStatic.NewNone();
}

PyResult ScanBound::Handle_RecoverProbes( PyCallArgs& call ) {
    //successProbeIDs = sm.RemoteSvc('scanMgr').GetSystemScanMgr().RecoverProbes(probeIDs)
    // list of probes successfully scooped to cargo
    // this is tested and added in Probe::RecoverProbe()
    PyList* list = new PyList();

    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        //TODO: throw exception
        return list;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : args.ints) {
        pSE = m_client->SystemMgr()->GetSE(cur);
        if (pSE == nullptr)
            continue;
        if (pSE->IsProbeSE())
            pSE->GetProbeSE()->RecoverProbe(list);
    }

    return list;
}

PyResult ScanBound::Handle_DestroyProbe( PyCallArgs& call ) {
    //scanMan = sm.RemoteSvc('scanMgr').GetSystemScanMgr()
    //scanMan.DestroyProbe(probeID)
    _log(SCAN__TRACE, "ScanBound::Handle_DestroyProbe() - size=%li", call.tuple->size());
    call.Dump(SCAN__DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        //TODO: throw exception
        return nullptr;
    }

    SystemEntity* pSE(m_client->SystemMgr()->GetSE(arg.arg));
    if (pSE != nullptr)
        pSE->Delete();
    SafeDelete(pSE);

    return nullptr;
}

PyResult ScanBound::Handle_ReconnectToLostProbes( PyCallArgs& call ) {
    // no args
    //  will have to test against client launcher vs probe m_moduleID
    // will have to write *something* to loop thru active probes in system for this....

    //  search inactive probes in system for ownerID, reset variables as per launch, add to all maps, then send client update
    // will need separate map for probes to use this?  or just loop thru EVERYTHING in system in the off chance we'll find a lost probe?
    //    i dunno...both kinda suck at this point
    return nullptr;
}
