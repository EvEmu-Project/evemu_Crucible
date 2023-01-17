
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



#include "planet/Moon.h"
#include "pos/PosMgr.h"
#include "pos/Array.h"
#include "pos/Battery.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "pos/Weapon.h"
#include "system/SystemManager.h"

PosMgr::PosMgr(EVEServiceManager &mgr) :
    BindableService("posMgr", mgr)
{
    this->Add("GetJumpArrays", &PosMgr::GetJumpArrays);
    this->Add("GetControlTowers", &PosMgr::GetControlTowers);
    this->Add("GetControlTowerFuelRequirements", &PosMgr::GetControlTowerFuelRequirements);
}

BoundDispatcher* PosMgr::BindObject(Client* client, PyRep* bindParameters) {
    _log( POS__DUMP, "PosMgr bind request for:" );
    bindParameters->Dump( POS__DUMP, "    " );

    if (!bindParameters->IsInt()){
        sLog.Error( "PosMgr::CreateBoundObject", "%s: bind_args is not int: '%s'. ", client->GetName(), bindParameters->TypeString() );
        return nullptr;
    }

    uint32 systemID = bindParameters->AsInt()->value();
    auto it = this->m_instances.find (systemID);

    if (it != this->m_instances.end ())
        return it->second;

    PosMgrBound* bound = new PosMgrBound(this->GetServiceManager(), *this, systemID);

    this->m_instances.insert_or_assign (systemID, bound);

    return bound;
}

void PosMgr::BoundReleased (PosMgrBound* bound) {
    auto it = this->m_instances.find (bound->GetSystemID());

    if (it == this->m_instances.end ())
        return;

    this->m_instances.erase (it);
}

PyResult PosMgr::GetControlTowerFuelRequirements(PyCallArgs &call) {
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

PyResult PosMgr::GetControlTowers(PyCallArgs &call) {
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

PyResult PosMgr::GetJumpArrays(PyCallArgs &call) {
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

PosMgrBound::PosMgrBound(EVEServiceManager& mgr, PosMgr& parent, uint32 systemID) :
    EVEBoundObject(mgr, parent)
{
    m_systemID = systemID;

    this->Add("GetMoonForTower", &PosMgrBound::GetMoonForTower);
    this->Add("SetTowerPassword", static_cast <PyResult (PosMgrBound::*)(PyCallArgs&, PyInt*, PyRep*, PyBool*, PyBool*)> (&PosMgrBound::SetTowerPassword));
    this->Add("SetTowerPassword", static_cast <PyResult(PosMgrBound::*)(PyCallArgs&, PyInt*, PyRep*)> (&PosMgrBound::SetTowerPassword));
    this->Add("SetShipPassword", &PosMgrBound::SetShipPassword);
    this->Add("GetSiloCapacityByItemID", &PosMgrBound::GetSiloCapacityByItemID);
    this->Add("AnchorOrbital", &PosMgrBound::AnchorOrbital);
    this->Add("UnanchorOrbital", &PosMgrBound::UnanchorOrbital);
    this->Add("OnlineOrbital", &PosMgrBound::OnlineOrbital);
    this->Add("GMUpgradeOrbital", &PosMgrBound::GMUpgradeOrbital);
    this->Add("AnchorStructure", &PosMgrBound::AnchorStructure);
    this->Add("UnanchorStructure", &PosMgrBound::UnanchorStructure);
    this->Add("AssumeStructureControl", &PosMgrBound::AssumeStructureControl);
    this->Add("RelinquishStructureControl", &PosMgrBound::RelinquishStructureControl);
    this->Add("ChangeStructureProvisionType", &PosMgrBound::ChangeStructureProvisionType);
    this->Add("CompleteOrbitalStateChange", &PosMgrBound::CompleteOrbitalStateChange);
    this->Add("GetMoonProcessInfoForTower", &PosMgrBound::GetMoonProcessInfoForTower);
    this->Add("LinkResourceForTower", &PosMgrBound::LinkResourceForTower);
    this->Add("RunMoonProcessCycleforTower", &PosMgrBound::RunMoonProcessCycleforTower);
    this->Add("GetStarbasePermissions", &PosMgrBound::GetStarbasePermissions);
    this->Add("SetStarbasePermissions", &PosMgrBound::SetStarbasePermissions);
    this->Add("GetTowerNotificationSettings", &PosMgrBound::GetTowerNotificationSettings);
    this->Add("SetTowerNotifications", &PosMgrBound::SetTowerNotifications);
    this->Add("GetTowerSentrySettings", &PosMgrBound::GetTowerSentrySettings);
    this->Add("SetTowerSentrySettings", &PosMgrBound::SetTowerSentrySettings);
    this->Add("InstallJumpBridgeLink", &PosMgrBound::InstallJumpBridgeLink);
    this->Add("UninstallJumpBridgeLink", &PosMgrBound::UninstallJumpBridgeLink);
}

PyResult PosMgrBound::InstallJumpBridgeLink(PyCallArgs &call, PyInt* localItemID, PyInt* remoteSolarSystemID, PyInt* remoteItemID) {
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

    // Install jump bridge link both ways
    m_db.InstallBridgeLink(localItemID->value(), remoteSolarSystemID->value(), remoteItemID->value());
    m_db.InstallBridgeLink(remoteItemID->value(), call.client->GetSystemID(), localItemID->value());

    return PyStatic.NewNone();
}

PyResult PosMgrBound::UninstallJumpBridgeLink(PyCallArgs &call, PyInt* itemID) {
    /**
     *    def UnbridgePortal(self, itemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.UninstallJumpBridgeLink(itemID)
     *
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_UninstallJumpBridgeLink()");
    call.Dump(POS__DUMP);

    m_db.UninstallRemoteBridgeLink(itemID->value());
    m_db.UninstallBridgeLink(itemID->value());

    return PyStatic.NewNone();
}

PyResult PosMgrBound::GetSiloCapacityByItemID(PyCallArgs &call, PyInt* itemID) {
    _log(POS__TRACE,  "PosMgrBound::Handle_GetSiloCapacityByItemID()");
    call.Dump(POS__DUMP);

    uint16 typeID = 0;
    /** @todo  put this in static data */

    return m_db.GetSiloCapacityForType(typeID);
}

PyResult PosMgrBound::GetTowerNotificationSettings(PyCallArgs &call, PyInt* itemID) {
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

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return new PyObject("util.Row", dict);

    PyList* line = new PyList(2);
        line->SetItem(0, new PyBool(pTSE->SendFuelNotifications()));
        line->SetItem(1, new PyBool(pTSE->ShowInCalendar()));
    dict->SetItemString("line", line);

    return new PyObject("util.Row", dict);
}

PyResult PosMgrBound::SetTowerNotifications(PyCallArgs &call, PyInt* itemID, PyBool* showInCalendar, PyBool* sendFuelNotifications) {
    //self.posMgr.SetTowerNotifications(self.slimItem.itemID, showInCalendar, sendFuelNotifications)

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    pTSE->SetSendFuelNotifications(sendFuelNotifications->value());
    pTSE->SetShowInCalendar(showInCalendar->value());
    pTSE->UpdateNotify();
    return PyStatic.NewNone();
}

PyResult PosMgrBound::GetTowerSentrySettings(PyCallArgs &call, PyInt* itemID) {
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

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
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

PyResult PosMgrBound::SetTowerSentrySettings(PyCallArgs &call, PyInt* itemID, PyFloat* standing, PyFloat* status, PyBool* statusDrop, PyBool* corpWar, PyBool* useAllianceStandings) {
    //  self.posMgr.SetTowerSentrySettings(self.slimItem.itemID, standing, status, statusDrop, war, useAllianceStandings)
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    pTSE->SetStanding(standing->value());
    pTSE->SetStatus(status->value());
    pTSE->SetStatusDrop(statusDrop->value());
    pTSE->SetCorpWar(corpWar->value());
    pTSE->SetUseAllyStandings(useAllianceStandings->value());
    pTSE->UpdateSentry();

    return PyStatic.NewNone();
}

PyResult PosMgrBound::GetStarbasePermissions(PyCallArgs &call, PyInt* itemID) {
    //  deployFlags, usageFlagsList = self.posMgr.GetStarbasePermissions(self.slimItem.itemID)
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, pTSE->GetDeployFlags());  // deployFlags
        tuple->SetItem(1, pTSE->GetUsageFlagList());  // usageFlagsList
    return tuple;
}

PyResult PosMgrBound::SetStarbasePermissions(PyCallArgs &call, PyInt* itemID, PyObject* deployFlags, PyObject* usageFlagsList) {
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

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    // decode deployFlags object
    PyRep* rep = deployFlags->arguments()->AsDict()->GetItemString("line");
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
    PyRep* rep2 = usageFlagsList->arguments()->AsDict()->GetItemString("lines");
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

PyResult PosMgrBound::GetMoonForTower(PyCallArgs &call, PyInt* itemID) {
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

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
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

PyResult PosMgrBound::SetShipPassword(PyCallArgs &call, PyWString* password) {
    /*
     * 13:16:17 L PosMgrBound::Handle_SetShipPassword(): size= 1
     * 13:16:17 [SvcCall]   Call Arguments:
     * 13:16:17 [SvcCall]       Tuple: 1 elements
     * 13:16:17 [SvcCall]         [ 0] WString: 'test'             << password
     */

    // havent been able to call this while docked, but there is an error msg for it.
    if (call.client->IsDocked())
        throw UserError("CannotSetShieldHarmonicPassword");

    call.client->GetShipSE()->SetPassword(PyRep::StringContent(password));

    return PyStatic.NewNone();
}

PyResult PosMgrBound::SetTowerPassword(PyCallArgs& call, PyInt* itemID, PyRep* password) {
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    if (password->IsString() or password->IsWString())
        pTSE->SetPassword(PyRep::StringContent(password));
    pTSE->UpdatePassword();
}

PyResult PosMgrBound::SetTowerPassword(PyCallArgs &call, PyInt* itemID, PyRep* password, PyBool* allowCorp, PyBool* allowAlliance) {
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    if (password->IsString() or password->IsWString()) {
        pTSE->SetPassword(PyRep::StringContent(password));
        pTSE->UpdatePassword();
    }

    pTSE->SetCorpAccess(allowCorp->value());
    pTSE->SetAllyAccess(allowAlliance->value());
    pTSE->UpdateAccess();

    // set harmonic for ship to 'offline' (0)   -according to packet data
    call.client->GetShipSE()->SetHarmonic(EVEPOS::Harmonic::Offline);

    // at this point, if ship password isnt updated, it should be kicked out of tower's ff.
    //  not sure how we're gonna do this yet.  will have to wait till system matures

    return PyStatic.NewNone();
}

PyResult PosMgrBound::UnanchorStructure(PyCallArgs &call, PyInt* itemID) {
    _log(POS__TRACE,  "PosMgrBound::Handle_UnanchorStructure()");
    call.Dump(POS__DUMP);

    StructureSE* pTSE(nullptr);

    //pTSE->PullAnchor();

    return PyStatic.NewNone();
}

PyResult PosMgrBound::AnchorStructure(PyCallArgs &call, PyInt* structureID, PyTuple* position) {
    _log(POS__TRACE, "POS Mgr::Anchor()");
    call.Dump(POS__DUMP);
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    double posX = PyRep::IntegerValue(position->GetItem(0));
    double posY = PyRep::IntegerValue(position->GetItem(1));
    double posZ = PyRep::IntegerValue(position->GetItem(2));

    StructureSE* pTSE = pSystem->GetSE(structureID->value())->GetPOSSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    GPoint pos(posX, posY, posZ);
    pTSE->SetAnchor(call.client, pos);

    // auto warp to new POS position?  config option?
    if (pTSE->IsTowerSE()) {
        uint32 dist = pTSE->GetSelf()->radius() + call.client->GetShip()->radius();
        call.client->GetShipSE()->DestinyMgr()->WarpTo(pos, dist);
        /** @todo add tower anchor position to bookmark */
    }
    // returns nodeID and timestamp
    return this->GetOID();
}

PyResult PosMgrBound::GetMoonProcessInfoForTower(PyCallArgs &call, PyInt* itemID) {
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    TowerSE* pTSE = pSystem->GetSE(itemID->value())->GetTowerSE();
    if (pTSE == nullptr)
        return PyStatic.NewNone();

    return pTSE->GetProcessInfo();
}

PyResult PosMgrBound::AssumeStructureControl(PyCallArgs &call, PyInt* itemID) {
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

PyResult PosMgrBound::RelinquishStructureControl(PyCallArgs &call, PyInt* itemID) {
    /*
        posMgr = moniker.GetPOSMgr()
        posMgr.RelinquishStructureControl(item.itemID)
    */
    _log(POS__TRACE,  "PosMgrBound::Handle_RelinquishStructureControl()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::AnchorOrbital(PyCallArgs &call, PyInt* itemID) {
    /*
     *  def AnchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.AnchorOrbital(itemID)
     */

    _log(POS__TRACE,  "PosMgrBound::Handle_()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::UnanchorOrbital(PyCallArgs &call, PyInt* itemID) {
    /*
     *  def UnanchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.UnanchorOrbital(itemID)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_UnanchorOrbital()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::OnlineOrbital(PyCallArgs &call, PyInt* itemID) {
    _log(POS__TRACE,  "PosMgrBound::Handle_OnlineOrbital()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::ChangeStructureProvisionType(PyCallArgs &call, PyInt* towerID, PyInt* itemID, PyInt* typeID) {
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

    /** @todo  finish this.. */

    return PyStatic.NewNone();
}

PyResult PosMgrBound::LinkResourceForTower(PyCallArgs &call, PyInt* itemID, PyList* connections) {
    _log(POS__TRACE,  "PosMgrBound::Handle_LinkResourceForTower()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::RunMoonProcessCycleforTower(PyCallArgs &call, PyInt* itemID) {
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

PyResult PosMgrBound::GMUpgradeOrbital(PyCallArgs &call, PyInt* itemID) {
    /*
     *  def GMUpgradeOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.GMUpgradeOrbital(itemID)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_GMUpgradeOrbital()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

PyResult PosMgrBound::CompleteOrbitalStateChange(PyCallArgs &call, PyInt* itemID) {
    /*
     *  def CompleteOrbitalStateChange(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.CompleteOrbitalStateChange(itemID)
     */
    _log(POS__TRACE,  "PosMgrBound::Handle_CompleteOrbitalStateChange()");
    call.Dump(POS__DUMP);

    return PyStatic.NewNone();
}

