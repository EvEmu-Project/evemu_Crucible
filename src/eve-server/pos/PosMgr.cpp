
/**
 * @name PosMgr.cpp
 *   Class for pos manager calls.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */

/*
 * POS__ERROR
 * POS__WARNING
 * POS__MESSAGE
 * POS__DUMP
 * POS__DEBUG
 * POS__DESTINY
 * POS__SLIMITEM
 * POS__TRACE
 */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "planet/Moon.h"
#include "pos/PosMgr.h"
#include "pos/Array.h"
#include "pos/Battery.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "pos/Weapon.h"
#include "system/SystemManager.h"


class PosMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PosMgrBound);

    PosMgrBound(PyServiceMgr* mgr, uint32 systemID)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PosMgrBound";

        m_systemID = systemID;

        PyCallable_REG_CALL(PosMgrBound, GetMoonForTower);
        PyCallable_REG_CALL(PosMgrBound, SetTowerPassword);
        PyCallable_REG_CALL(PosMgrBound, SetShipPassword);
        PyCallable_REG_CALL(PosMgrBound, GetSiloCapacityByItemID);
        PyCallable_REG_CALL(PosMgrBound, AnchorOrbital);
        PyCallable_REG_CALL(PosMgrBound, UnanchorOrbital);
        PyCallable_REG_CALL(PosMgrBound, OnlineOrbital);
        PyCallable_REG_CALL(PosMgrBound, GMUpgradeOrbital);
        PyCallable_REG_CALL(PosMgrBound, AnchorStructure);
        PyCallable_REG_CALL(PosMgrBound, UnanchorStructure);
        PyCallable_REG_CALL(PosMgrBound, AssumeStructureControl);
        PyCallable_REG_CALL(PosMgrBound, RelinquishStructureControl);
        PyCallable_REG_CALL(PosMgrBound, ChangeStructureProvisionType);
        PyCallable_REG_CALL(PosMgrBound, CompleteOrbitalStateChange);
        PyCallable_REG_CALL(PosMgrBound, GetMoonProcessInfoForTower);
        PyCallable_REG_CALL(PosMgrBound, LinkResourceForTower);
        PyCallable_REG_CALL(PosMgrBound, RunMoonProcessCycleforTower);
        PyCallable_REG_CALL(PosMgrBound, GetStarbasePermissions);
        PyCallable_REG_CALL(PosMgrBound, SetStarbasePermissions);
        PyCallable_REG_CALL(PosMgrBound, GetTowerNotificationSettings);
        PyCallable_REG_CALL(PosMgrBound, SetTowerNotifications);
        PyCallable_REG_CALL(PosMgrBound, GetTowerSentrySettings);
        PyCallable_REG_CALL(PosMgrBound, SetTowerSentrySettings);
        PyCallable_REG_CALL(PosMgrBound, InstallJumpBridgeLink);
        PyCallable_REG_CALL(PosMgrBound, UninstallJumpBridgeLink);
    }

    virtual ~PosMgrBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(SetTowerPassword);
    PyCallable_DECL_CALL(SetShipPassword);
    PyCallable_DECL_CALL(GetMoonForTower);
    PyCallable_DECL_CALL(GetSiloCapacityByItemID);
    PyCallable_DECL_CALL(AnchorOrbital);
    PyCallable_DECL_CALL(UnanchorOrbital);
    PyCallable_DECL_CALL(OnlineOrbital);
    PyCallable_DECL_CALL(GMUpgradeOrbital);
    PyCallable_DECL_CALL(AnchorStructure);
    PyCallable_DECL_CALL(UnanchorStructure);
    PyCallable_DECL_CALL(AssumeStructureControl);
    PyCallable_DECL_CALL(RelinquishStructureControl);
    PyCallable_DECL_CALL(ChangeStructureProvisionType);
    PyCallable_DECL_CALL(CompleteOrbitalStateChange);
    PyCallable_DECL_CALL(GetMoonProcessInfoForTower);
    PyCallable_DECL_CALL(LinkResourceForTower);
    PyCallable_DECL_CALL(RunMoonProcessCycleforTower);
    PyCallable_DECL_CALL(GetStarbasePermissions);
    PyCallable_DECL_CALL(SetStarbasePermissions);
    PyCallable_DECL_CALL(GetTowerNotificationSettings);
    PyCallable_DECL_CALL(SetTowerNotifications);
    PyCallable_DECL_CALL(GetTowerSentrySettings);
    PyCallable_DECL_CALL(SetTowerSentrySettings);
    PyCallable_DECL_CALL(InstallJumpBridgeLink);
    PyCallable_DECL_CALL(UninstallJumpBridgeLink);

protected:
    Dispatcher* const m_dispatch;
    PosMgrDB m_db;
    uint32 m_systemID;
};


PyCallable_Make_InnerDispatcher(PosMgr)

PosMgr::PosMgr(PyServiceMgr *mgr)
: PyService(mgr, "posMgr"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PosMgr, GetJumpArrays);
    PyCallable_REG_CALL(PosMgr, GetControlTowers);
    PyCallable_REG_CALL(PosMgr, GetControlTowerFuelRequirements);
}

PosMgr::~PosMgr() {
    delete m_dispatch;
}

PyBoundObject* PosMgr::CreateBoundObject( Client* pClient, const PyRep* bind_args ) {
    _log( POS__DUMP, "PosMgr bind request for:" );
    bind_args->Dump( POS__DUMP, "    " );

    if (!bind_args->IsInt()){
        sLog.Error( "PosMgr::CreateBoundObject", "%s: bind_args is not int: '%s'. ", pClient->GetName(), bind_args->TypeString() );
        return nullptr;
    }

    return new PosMgrBound(m_manager, bind_args->AsInt()->value());
}

PyResult PosMgr::Handle_GetControlTowerFuelRequirements(PyCallArgs &call) {
    /** @todo put this in static data */
    DBQueryResult res;
    m_db.GetControlTowerFuelRequirements(res);

    DBResultRow row;
    PyList* list = new PyList();
    while (res.GetRow(row)) {
        //SELECT controlTowerTypeID, resourceTypeID, purpose, quantity, minSecurityLevel, factionID, wormholeClassID
        PyDict* dict = new PyDict();
        dict->SetItemString("controlTowerTypeID",   new PyInt(row.GetInt(0)));
        dict->SetItemString("resourceTypeID",       new PyInt(row.GetInt(1)));
        dict->SetItemString("purpose",              new PyInt(row.GetInt(2)));
        dict->SetItemString("quantity",             new PyInt(row.GetInt(3)));
        dict->SetItemString("minSecurityLevel",     (row.IsNull(4) ? PyStatic.NewNone() : new PyFloat(row.GetDouble(4))));
        dict->SetItemString("factionID",            (row.IsNull(5) ? PyStatic.NewNone() : new PyInt(row.GetInt(5))));
        dict->SetItemString("wormholeClassID",      (row.IsNull(6) ? PyStatic.NewNone() : new PyInt(row.GetInt(6))));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

PyResult PosMgr::Handle_GetControlTowers(PyCallArgs &call) {
    /*  ct = sm.RemoteSvc('posMgr').GetControlTowers()
     *        for row in ct:
     *            typeID, structureID, solarSystemID = row[0:3]
     *
     *                        [CRowSet]
     *                            [DBRowDescriptor]
     *                                ["typeID" =>  [I4] ]
     *                                ["itemID" =>  [I8] ]
     *                                ["locationID" =>  [I8] ]
     *                            Rows:
     *                                [PyPackedRow 21 bytes]
     *                                    ["typeID" =>  [I4] <20062>]
     *                                    ["itemID" =>  [I8] <1013581844534>]
     *                                    ["locationID" =>  [I8] <30002206>]
     *                                [PyPackedRow 21 bytes]
     *                                    ["typeID" =>  [I4] <27606>]
     *                                    ["itemID" =>  [I8] <1010759458081>]
     *                                    ["locationID" =>  [I8] <30002225>]
     */

    _log(POS__TRACE,  "PosMgr::Handle_GetControlTowers()");
    call.Dump(POS__DUMP);

    return m_db.GetCorpControlTowers(call.client->GetCorporationID());
}

PyResult PosMgr::Handle_GetJumpArrays(PyCallArgs &call) {
    /*        jb = sm.RemoteSvc('posMgr').GetJumpArrays()
     *
     *            for data in jb:
     *                solarSystemID, subData = data (fromSystem, solarSystemData)     <-- tuple (to solarSystemID, StructureData)
     *                    ssid = subData.keys()[0]                                    <-- jb itemID (from)
     *                    tsid = subData.values()[0][1]                               <-- jb itemID (to)
     *
     *                    fromStructure = solarSystemData.keys()[0]                   <-- indexed dict of tuple
     *                    toSystem = cfg.evelocations.Get(solarSystemData.values()[0][0])     <-- to solarSystemID
     *                    toStructure = solarSystemData.values()[0][1]                       <-- jb itemID (to)
     *                    toStructureType = solarSystemData.values()[0][2]                   <-- jb typeID
     */
    DBQueryResult res;
    m_db.GetCorpJumpArrays(call.client->GetCorporationID(), res);

    DBResultRow row;
    PyList* list = new PyList();
    while (res.GetRow(row)) {
        //SELECT itemID, systemID, toItemID, toTypeID, toSystemID
        PyTuple* rsp = new PyTuple(2);
            rsp->SetItem(0, new PyInt(row.GetInt(1)));
        if (row.GetInt(2) > 0) {
        PyDict* dict = new PyDict();
            PyTuple* tuple = new PyTuple(3);
                tuple->SetItem(0, new PyInt(row.GetInt(4)));
                tuple->SetItem(1, new PyInt(row.GetInt(2)));
                tuple->SetItem(2, new PyInt(row.GetInt(3)));
            dict->SetItem(new PyInt(row.GetInt(0)), tuple);
            rsp->SetItem(1, dict);
        } else {
            rsp->SetItem(1, PyStatic.NewNone());
        }

        list->AddItem(rsp);
    }

    list->Dump(POS__RSP_DUMP, "    ");
    return list;
}

PyResult PosMgrBound::Handle_InstallJumpBridgeLink(PyCallArgs &call) {
    /**
     *    def BridgePortals(self, localItemID, remoteSolarSystemID, remoteItemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.InstallJumpBridgeLink(localItemID, remoteSolarSystemID, remoteItemID)
     *
     * 01:15:52 W PosMgrBound::Handle_InstallJumpBridgeLink(): size=3
     * 01:15:52 [POS:Dump]   Call Arguments:
     * 01:15:52 [POS:Dump]      Tuple: 3 elements
     * 01:15:52 [POS:Dump]       [ 0]    Integer: 140000332     <-- itemID
     * 01:15:52 [POS:Dump]       [ 1]    Integer: 30000053      <-- toSystemID
     * 01:15:52 [POS:Dump]       [ 2]    Integer: 140000064     <-- toItemID
     *
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_InstallJumpBridgeLink()");
    call.Dump(POS__DUMP);

    InstallJumpBridgeLink args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // Install jump bridge link both ways
    m_db.InstallBridgeLink(args.itemID, args.toSystemID, args.toItemID);
    m_db.InstallBridgeLink(args.toItemID, call.client->GetSystemID(), args.itemID);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_UninstallJumpBridgeLink(PyCallArgs &call) {
    /**
     *    def UnbridgePortal(self, itemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.UninstallJumpBridgeLink(itemID)
     *
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_UninstallJumpBridgeLink()");
    call.Dump(POS__DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    m_db.UninstallRemoteBridgeLink(arg.arg);
    m_db.UninstallBridgeLink(arg.arg);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_GetSiloCapacityByItemID(PyCallArgs &call) {
    _log(POS__TRACE,  "PosMgrBound::Handle_GetSiloCapacityByItemID()");
    call.Dump(POS__DUMP);

    uint16 typeID = 0;
    /** @todo  put this in static data */

    return m_db.GetSiloCapacityForType(typeID);
}

PyResult PosMgrBound::Handle_GetTowerNotificationSettings(PyCallArgs &call) {
    /*
     *        notifySettings = self.posMgr.GetTowerNotificationSettings(self.slimItem.itemID)
     *        self.fuelNotifyCheckbox.SetChecked(notifySettings.sendFuelNotifications, 0)
     *        self.calendarCheckbox.SetChecked(notifySettings.showInCalendar, 0)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_GetTowerNotificationSettings()");
    call.Dump(POS__DUMP);

    PyList* header = new PyList(2);
        header->SetItemString(0, "sendFuelNotifications");
        header->SetItemString(1, "showInCalendar");
    PyDict* dict = new PyDict();
        dict->SetItemString("header", header);

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyObject("util.Row", dict);
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return new PyObject("util.Row", dict);
    }

    TowerSE* pTSE = pSystem->GetSE(arg.arg)->GetTowerSE();
    if (pTSE == nullptr)
        return new PyObject("util.Row", dict);

    PyList* line = new PyList(2);
        line->SetItem(0, new PyBool(pTSE->SendFuelNotifications()));
        line->SetItem(1, new PyBool(pTSE->ShowInCalendar()));
    dict->SetItemString("line", line);

    return new PyObject("util.Row", dict);
}

PyResult PosMgrBound::Handle_SetTowerNotifications(PyCallArgs &call) {
    //self.posMgr.SetTowerNotifications(self.slimItem.itemID, showInCalendar, sendFuelNotifications)

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    SetTowerNotification args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(args.structureID)->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    pTSE->SetSendFuelNotifications(args.sendFuelNotifications);
    pTSE->SetShowInCalendar(args.showInCalendar);
    pTSE->UpdateNotify();
    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_GetTowerSentrySettings(PyCallArgs &call) {
    //  standing, status, statusDrop, war, standingOwnerID = self.sentrySettings = self.posMgr.GetTowerSentrySettings(self.slimItem.itemID)
    PyDict* data = new PyDict();
    PyList* header = new PyList(5);
        header->SetItemString(0, "standing");
        header->SetItemString(1, "status");
        header->SetItemString(2, "statusDrop");
        header->SetItemString(3, "corpWar");
        header->SetItemString(4, "standingOwnerID");
    data->SetItemString("header", header);

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyObject("util.Row", data);
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return new PyObject("util.Row", data);
    }

    TowerSE* pTSE = pSystem->GetSE(arg.arg)->GetTowerSE();
    if (pTSE == nullptr)
        return new PyObject("util.Row", data);

    PyList* line = new PyList(5);
        line->SetItem(0, new PyFloat(pTSE->GetStanding()));
        line->SetItem(1, new PyFloat(pTSE->GetStatus()));
        line->SetItem(2, new PyBool(pTSE->GetStatusDrop()));
        line->SetItem(3, new PyBool(pTSE->GetCorpWar()));
        line->SetItem(4, new PyInt(pTSE->GetStandingOwnerID()));
    data->SetItemString("line", line);

    return new PyObject("util.Row", data);
}

PyResult PosMgrBound::Handle_SetTowerSentrySettings(PyCallArgs &call) {
    //  self.posMgr.SetTowerSentrySettings(self.slimItem.itemID, standing, status, statusDrop, war, useAllianceStandings)
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    SetTowerSentrySettings args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(args.structureID)->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    pTSE->SetStanding(args.standing);
    pTSE->SetStatus(args.status);
    pTSE->SetStatusDrop(args.statusDrop);
    pTSE->SetCorpWar(args.corpWar);
    pTSE->SetUseAllyStandings(args.useAllianceStandings);
    pTSE->UpdateSentry();

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_GetStarbasePermissions(PyCallArgs &call) {
    //  deployFlags, usageFlagsList = self.posMgr.GetStarbasePermissions(self.slimItem.itemID)
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(arg.arg)->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, pTSE->GetDeployFlags());  // deployFlags
        tuple->SetItem(1, pTSE->GetUsageFlagList());  // usageFlagsList
    return tuple;
}

PyResult PosMgrBound::Handle_SetStarbasePermissions(PyCallArgs &call) {
    //  self.posMgr.SetStarbasePermissions(self.slimItem.itemID, self.sr.deployFlags, self.sr.usageFlagsList)
    /* values....
     * 0 - corp role config equip.
     * 1 - corp members
     * 2 - ally members
     * 3 - corp role fuel tech
     */

    _log(POS__TRACE,  "PosMgrBound::Handle_SetStarbasePermissions()");
    call.Dump(POS__DUMP);

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    SetStarbasePermissions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(args.towerID)->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    // decode deployFlags object
    PyRep* rep = args.deployFlags->arguments()->AsDict()->GetItemString("line");
    if (!rep->IsList()) {
        codelog(POS__ERROR, "deployFlags 'line' item is not PyList: %s", rep->TypeString());
        return nullptr;
    }

    PyList* list = rep->AsList();
    //list->Dump(POS__RSP_DUMP, "    ");
    pTSE->SetDeployFlags(list->GetItem(0)->AsInt()->value(),
                         list->GetItem(1)->AsInt()->value(),
                         list->GetItem(2)->AsInt()->value(),
                         list->GetItem(3)->AsInt()->value() );


    // decode usageFlagsList object
    PyRep* rep2 = args.usageFlagsList->arguments()->AsDict()->GetItemString("lines");
    if (!rep2->IsList()) {
        codelog(POS__ERROR, "usageFlagsList 'lines' item is not PyList: %s", rep2->TypeString());
        return nullptr;
    }

    PyList* list2 = rep2->AsList();
    //list2->Dump(POS__RSP_DUMP, "    ");
    for (PyList::const_iterator itr = list2->begin(); itr != list2->end(); ++itr) {
        if (!(*itr)->IsList()) {
            codelog(POS__ERROR, "usageFlagsList - itr item is not PyList: %s", (*itr)->TypeString());
            continue;
        }
        pTSE->SetUseFlags((*itr)->AsList()->GetItem(0)->AsInt()->value(),
                          (*itr)->AsList()->GetItem(1)->AsInt()->value(),
                          (*itr)->AsList()->GetItem(2)->AsInt()->value(),
                          (*itr)->AsList()->GetItem(3)->AsInt()->value());
    }

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_GetMoonForTower( PyCallArgs &call ) {
  /*
13:13:06 L PosMgrBound::Handle_GetMoonForTower(): size= 1
13:13:06 [SvcCall]   Call Arguments:
13:13:06 [SvcCall]       Tuple: 1 elements
13:13:06 [SvcCall]         [ 0] Integer field: 140001260

self.moonID = self.moon[0]
if self.moon[1] is not None:
    for typeID, quantity in self.moon[1]:

    returns
        tuple
            moonID
            tuple
                resource typeID
                resource quantity

  _log(POS__TRACE,  "PosMgrBound::Handle_GetMoonForTower()");
  call.Dump(POS__DUMP);
  */

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(arg.arg)->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();
    MoonSE* pMSE = pTSE->GetMoonSE();
    if (pMSE == nullptr)
        return PyStatic.NewNone();

    std::map<uint16, uint8>::iterator itr = pMSE->GooBegin(), end = pMSE->GooEnd();
    PyList* list = new PyList();
    while (itr != end) {
        PyTuple* resource = new PyTuple(2);
            resource->SetItem(0, new PyInt(itr->first));
            resource->SetItem(1, new PyInt(itr->second));
        list->AddItem(resource);
        ++itr;
    }

    PyTuple* item = new PyTuple(2);
        item->SetItem(0, new PyInt(pMSE->GetID()));
        item->SetItem(1, list);
    return item;
}

PyResult PosMgrBound::Handle_SetShipPassword( PyCallArgs &call ) {
    /*
     * 13:16:17 L PosMgrBound::Handle_SetShipPassword(): size= 1
     * 13:16:17 [SvcCall]   Call Arguments:
     * 13:16:17 [SvcCall]       Tuple: 1 elements
     * 13:16:17 [SvcCall]         [ 0] WString: 'test'             << password
     */

    // havent been able to call this while docked, but there is an error msg for it.
    if (call.client->IsDocked())
        throw UserError("CannotSetShieldHarmonicPassword");

    call.client->GetShipSE()->SetPassword(PyRep::StringContent(call.tuple->GetItem(0)));

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_SetTowerPassword( PyCallArgs &call ) {
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    if (call.tuple->size() == 2) {
        SetTowerPassword2 args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewNone();
        }

        TowerSE* pTSE = pSystem->GetSE(args.towerID)->GetTowerSE();
        if (pTSE == nullptr)
            return PyStatic.NewNone();

        if (args.password->IsString() or args.password->IsWString())
            pTSE->SetPassword(PyRep::StringContent(args.password));
        pTSE->UpdatePassword();
    } else if (call.tuple->size() == 4) {
        SetTowerPassword4 args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewNone();
        }

        TowerSE* pTSE = pSystem->GetSE(args.towerID)->GetTowerSE();
        if (pTSE == nullptr)
            return PyStatic.NewNone();

        if (args.password->IsString() or args.password->IsWString()) {
            pTSE->SetPassword(PyRep::StringContent(args.password));
            pTSE->UpdatePassword();
        }

        pTSE->SetCorpAccess(args.allowCorp);
        pTSE->SetAllyAccess(args.allowAlliance);
        pTSE->UpdateAccess();
    } else {
        // make error here?
    }

    // set harmonic for ship to 'offline' (0)   -according to packet data
    call.client->GetShipSE()->SetHarmonic(EVEPOS::Harmonic::Offline);

    // at this point, if ship password isnt updated, it should be kicked out of tower's ff.
    //  not sure how we're gonna do this yet.  will have to wait till system matures

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_UnanchorStructure(PyCallArgs &call) {
    _log(POS__TRACE,  "PosMgrBound::Handle_UnanchorStructure()");
    call.Dump(POS__DUMP);

    StructureSE* pTSE(nullptr);

    //pTSE->PullAnchor();

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_AnchorStructure(PyCallArgs &call) {
    _log(POS__TRACE, "POS Mgr::Anchor()");
    call.Dump(POS__DUMP);
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    AnchorStructure args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    StructureSE* pTSE = pSystem->GetSE(args.structureID)->GetPOSSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    GPoint pos(args.posX, args.posY, args.posZ);
    pTSE->SetAnchor(call.client, pos);

    // auto warp to new POS position?  config option?
    if (pTSE->IsTowerSE()) {
        uint32 dist = pTSE->GetSelf()->radius() + call.client->GetShip()->radius();
        call.client->GetShipSE()->DestinyMgr()->WarpTo(pos, dist);
        /** @todo add tower anchor position to bookmark */
    }
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult PosMgrBound::Handle_GetMoonProcessInfoForTower(PyCallArgs &call) {
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(arg.arg)->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    return pTSE->GetProcessInfo();
}

PyResult PosMgrBound::Handle_AssumeStructureControl(PyCallArgs &call) {
    // NOTE:  this is for controlling pos defences
    /*
        posMgr = moniker.GetPOSMgr()
        posMgr.AssumeStructureControl(item.itemID)

        02:02:07 W PosMgrBound::Handle_AssumeStructureControl(): size=1
        02:02:07 [POS:Dump]   Call Arguments:
        02:02:07 [POS:Dump]      Tuple: 1 elements
        02:02:07 [POS:Dump]       [ 0]    Integer: 140000311

        'OnAssumeStructureControl',
        'OnRelinquishStructureControl',

    */
    _log(POS__TRACE,  "PosMgrBound::Handle_AssumeStructureControl()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_RelinquishStructureControl(PyCallArgs &call) {
    /*
        posMgr = moniker.GetPOSMgr()
        posMgr.RelinquishStructureControl(item.itemID)
    */
    _log(POS__TRACE,  "PosMgrBound::Handle_RelinquishStructureControl()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_AnchorOrbital(PyCallArgs &call) {
    /*
     *  def AnchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.AnchorOrbital(itemID)
     */

    _log(POS__TRACE,  "PosMgrBound::Handle_()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_UnanchorOrbital(PyCallArgs &call) {
    /*
     *  def UnanchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.UnanchorOrbital(itemID)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_UnanchorOrbital()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_OnlineOrbital(PyCallArgs &call) {
    _log(POS__TRACE,  "PosMgrBound::Handle_OnlineOrbital()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_ChangeStructureProvisionType(PyCallArgs &call) {
    // this changes silo content or mining product
    /*
     * 03:11:32 W PosMgrBound::Handle_ChangeStructureProvisionType(): size=3
     * 03:11:32 [POS:Dump]   Call Arguments:
     * 03:11:32 [POS:Dump]      Tuple: 3 elements
     * 03:11:32 [POS:Dump]       [ 0]    Integer: 140000061     <-- towerID
     * 03:11:32 [POS:Dump]       [ 1]    Integer: 140000224     <-- itemID
     * 03:11:32 [POS:Dump]       [ 2]    Integer: 16634         <-- typeID
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_ChangeStructureProvisionType()");
    call.Dump(POS__DUMP);

    ChangeStructureProvisionType args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    /** @todo  finish this.. */

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_LinkResourceForTower(PyCallArgs &call) {
    _log(POS__TRACE,  "PosMgrBound::Handle_LinkResourceForTower()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_RunMoonProcessCycleforTower(PyCallArgs &call) {
    // this sets cycle 'on' for given towerID
    /*
     * 03:14:25 W PosMgrBound::Handle_RunMoonProcessCycleforTower(): size=1
     * 03:14:25 [POS:Dump]   Call Arguments:
     * 03:14:25 [POS:Dump]      Tuple: 1 elements
     * 03:14:25 [POS:Dump]       [ 0]    Integer: 140000061     <-- towerID
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_RunMoonProcessCycleforTower()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_GMUpgradeOrbital(PyCallArgs &call) {
    /*
     *  def GMUpgradeOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.GMUpgradeOrbital(itemID)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_GMUpgradeOrbital()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::Handle_CompleteOrbitalStateChange(PyCallArgs &call) {
    /*
     *  def CompleteOrbitalStateChange(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.CompleteOrbitalStateChange(itemID)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_CompleteOrbitalStateChange()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

