/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "corporation/BillMgr.h"

PyCallable_Make_InnerDispatcher(BillMgr)

BillMgr::BillMgr(PyServiceMgr *mgr)
: PyService(mgr, "billMgr"),
  m_dispatch(new Dispatcher(this)) {
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BillMgr, CharPayBill);
    PyCallable_REG_CALL(BillMgr, CharGetBills);
    PyCallable_REG_CALL(BillMgr, CharGetBillsReceivable);
    PyCallable_REG_CALL(BillMgr, GetBillTypes);
    PyCallable_REG_CALL(BillMgr, PayCorporationBill);
    PyCallable_REG_CALL(BillMgr, GetCorporationBills);
    PyCallable_REG_CALL(BillMgr, GetCorporationBillsReceivable);
    PyCallable_REG_CALL(BillMgr, GetAutomaticPaySettings);
    PyCallable_REG_CALL(BillMgr, SendAutomaticPaySettings);
}

BillMgr::~BillMgr() {
    delete m_dispatch;
}

PyResult BillMgr::Handle_GetBillTypes(PyCallArgs& call) {
    return sDataMgr.GetBillTypes();
}

PyResult BillMgr::Handle_GetCorporationBills(PyCallArgs &call) {
    return m_db.GetCorporationBills(call.client->GetCorporationID(), true);
}

PyResult BillMgr::Handle_GetCorporationBillsReceivable(PyCallArgs &call) {
    return m_db.GetCorporationBills(call.client->GetCorporationID(), false);
}


PyResult BillMgr::Handle_CharPayBill(PyCallArgs &call) {
    //   sm.RemoteSvc('billMgr').CharPayBill(bill.billID)
    sLog.Warning("BillMgr", "Handle_CharPayBill() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    // returns nothing
    return nullptr;
}

PyResult BillMgr::Handle_CharGetBills(PyCallArgs &call) {
    //   return sm.RemoteSvc('billMgr').CharGetBills()
    sLog.Warning("BillMgr", "Handle_CharGetBills() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    // returns nothing
    return nullptr;
}

PyResult BillMgr::Handle_CharGetBillsReceivable(PyCallArgs &call) {
    //   bills = sm.RemoteSvc('billMgr').CharGetBillsReceivable()
    sLog.Warning("BillMgr", "Handle_CharGetBillsReceivable() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    // returns nothing
    return nullptr;
}

PyResult BillMgr::Handle_PayCorporationBill(PyCallArgs &call) {
    //  sm.RemoteSvc('billMgr').PayCorporationBill(bill.billID, fromAccountKey=eve.session.corpAccountKey)
    sLog.Warning("BillMgr", "Handle_PayCorporationBill() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    // returns nothing
    return nullptr;
}


PyResult BillMgr::Handle_SendAutomaticPaySettings(PyCallArgs &call) {
    //    sm.RemoteSvc('billMgr').SendAutomaticPaySettings(self.automaticPaymentSettings)

    // if corp in alliance, get settings for all 6, else ignore AllianceMaintainanceBill (5)

    /*
     * 19:07:55 [CorpCallDump]   Call Arguments:
     * 19:07:55 [CorpCallDump]      Tuple: 1 elements
     * 19:07:55 [CorpCallDump]       [ 0]  Dictionary: 1 entries
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0]   Key:    Integer: 98000001
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:  Dictionary: 6 entries
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 0]   Key:    Integer: 6
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 0] Value:    Boolean: false
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 1]   Key:    Integer: 4
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 1] Value:    Boolean: true
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 2]   Key:     String: 'divisionID'  << wallet to use
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 2] Value:    Integer: 1003
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 3]   Key:    Integer: 3
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 3] Value:    Boolean: true
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 4]   Key:    Integer: 2
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 4] Value:    Boolean: true
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 5]   Key:    Integer: 1
     * 19:07:55 [CorpCallDump]       [ 0]   [ 0] Value:   [ 5] Value:    Boolean: false
     */

    // returns nothing
    return nullptr;
}

PyResult BillMgr::Handle_GetAutomaticPaySettings(PyCallArgs &call) {
    //    ambSettings = sm.RemoteSvc('billMgr').GetAutomaticPaySettings()
    // returns t/f for bill types
    DBQueryResult res;
    m_db.GetAutoPay(call.client->GetCorporationID(), res);

    DBResultRow row;
    PyDict* sets = new PyDict();
    if (res.GetRow(row)) {
        sets->SetItem(new PyInt(Corp::BillType::MarketFine), new PyBool(row.GetBool(0)));
        sets->SetItem(new PyInt(Corp::BillType::RentalBill), new PyBool(row.GetBool(1)));
        sets->SetItem(new PyInt(Corp::BillType::BrokerBill), new PyBool(row.GetBool(2)));
        sets->SetItem(new PyInt(Corp::BillType::WarBill), new PyBool(row.GetBool(3)));
        if (call.client->GetAllianceID())
            sets->SetItem(new PyInt(Corp::BillType::AllianceMaintainanceBill), new PyBool(row.GetBool(4)));
        sets->SetItem(new PyInt(Corp::BillType::SovereigntyMarker), new PyBool(row.GetBool(5)));
    }

    PyDict* dict = new PyDict();
        dict->SetItem(new PyInt(call.client->GetCorporationID()), sets);

    if (is_log_enabled(CORP__RSP_DUMP))
        dict->Dump(CORP__RSP_DUMP, "");

    return dict;
}


/*
 *        [PyString "OnNotificationReceived"]
 *        [PyList 0 items]
 *        [PyString "clientID"]
 *    [PyInt 5654387]
 *    [PyTuple 1 items]
 *      [PyTuple 2 items]
 *        [PyInt 0]
 *        [PySubStream 168 bytes]
 *          [PyTuple 2 items]
 *            [PyInt 0]
 *            [PyTuple 2 items]
 *              [PyInt 1]
 *              [PyTuple 5 items]
 *                [PyInt 342402174]
 *                [PyInt 10]            << Notify::Types::CorpAllBill
 *                [PyInt 1000167]
 *                [PyIntegerVar 129492968400000000]
 *                [PyDict 8 kvp]
 *                  [PyString "debtorID"]
 *                  [PyInt 98038978]
 *                  [PyString "creditorID"]
 *                  [PyInt 1000167]
 *                  [PyString "billTypeID"]
 *                  [PyInt 2]
 *                  [PyString "amount"]
 *                  [PyInt 981907]
 *                  [PyString "externalID2"]
 *                  [PyInt 60014683]
 *                  [PyString "externalID"]
 *                  [PyInt 27]
 *                  [PyString "currentDate"]
 *                  [PyIntegerVar 129492968683459696]
 *                  [PyString "dueDate"]
 *                  [PyIntegerVar 129518888683422295]
 *    [PyDict 1 kvp]
 *      [PyString "sn"]
 *      [PyIntegerVar 4]
 */

/*
// OnBillReceived, an essentially empty tuple, just to tell the client that there is something,
// maybe for blinking purpose?
OnBillReceived N_obr;   // this is in Wallet.xmlp
PyTuple * res5 = N_obr.Encode();
//call.client->SendNotification("OnBillReceived", "*corpid&corprole", &res5, false);
// Why do we create a bill, when the office is already paid? Maybe that's why it's empty...


// OnMessage notification, the LSC packet NotifyOnMessage can be used, along with the StoreNewEVEMail
// Who to send notification? corpRoleJuniorAccountant and equiv? atm it's enough to send it to the renter
// TODO: get the correct evemail content from somewhere
// TODO: send it to every corp member who's affected by it. corpRoleAccountant, corpRoleJuniorAccountant or equiv
m_manager->lsc_service->SendMail(
    m_db.GetStationCorporationCEO(oInfo.stationID),
                                 call.client->GetCharacterID(),
                                 "Bill issued",
                                 "Bill issued for renting an office");

*/