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
    Author:        Zhur
    Rewrite:    Allan
*/

/*
 * CORP__ERROR
 * CORP__WARNING
 * CORP__INFO
 * CORP__MESSAGE
 * CORP__TRACE
 * CORP__CALL
 * CORP__CALL_DUMP
 * CORP__RSP_DUMP
 * CORP__DB_ERROR
 * CORP__DB_WARNING
 * CORP__DB_INFO
 * CORP__DB_MESSAGE
 */

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "chat/LSCService.h"
#include "corporation/CorpStationMgr.h"
#include "station/Station.h"
#include "station/StationDataMgr.h"

class CorpStationMgrIMBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(CorpStationMgrIMBound)

    CorpStationMgrIMBound(PyServiceMgr *mgr, CorporationDB& db, uint32 station_id)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)),
      m_db(db),
      m_stationID(station_id)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "CorpStationMgrIMBound";

        //PyCallable_REG_CALL(CorpStationMgrIMBound, GetEveOwners);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetCorporateStationInfo);
        PyCallable_REG_CALL(CorpStationMgrIMBound, DoStandingCheckForStationService);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetPotentialHomeStations);
        PyCallable_REG_CALL(CorpStationMgrIMBound, SetHomeStation);
        PyCallable_REG_CALL(CorpStationMgrIMBound, SetCloneTypeID);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetQuoteForRentingAnOffice);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetNumberOfUnrentedOffices);
        PyCallable_REG_CALL(CorpStationMgrIMBound, MoveCorpHQHere);
        PyCallable_REG_CALL(CorpStationMgrIMBound, RentOffice);
        PyCallable_REG_CALL(CorpStationMgrIMBound, CancelRentOfOffice);
        //testing
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetStationOffices);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetCorporateStationOffice);
        PyCallable_REG_CALL(CorpStationMgrIMBound, DoesPlayersCorpHaveJunkAtStation);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetQuoteForGettingCorpJunkBack);
        PyCallable_REG_CALL(CorpStationMgrIMBound, PayForReturnOfCorpJunk);

        pStationItem = sEntityList.GetStationByID(station_id).get();
    }
    virtual ~CorpStationMgrIMBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    //PyCallable_DECL_CALL(GetEveOwners);
    PyCallable_DECL_CALL(GetCorporateStationInfo);
    PyCallable_DECL_CALL(DoStandingCheckForStationService);
    PyCallable_DECL_CALL(GetPotentialHomeStations);
    PyCallable_DECL_CALL(SetHomeStation);
    PyCallable_DECL_CALL(SetCloneTypeID);
    PyCallable_DECL_CALL(GetQuoteForRentingAnOffice);
    PyCallable_DECL_CALL(RentOffice);
    PyCallable_DECL_CALL(CancelRentOfOffice);
    PyCallable_DECL_CALL(GetStationOffices);
    PyCallable_DECL_CALL(GetNumberOfUnrentedOffices);
    PyCallable_DECL_CALL(MoveCorpHQHere);
    //testing
    PyCallable_DECL_CALL(GetCorporateStationOffice);
    PyCallable_DECL_CALL(DoesPlayersCorpHaveJunkAtStation);
    PyCallable_DECL_CALL(GetQuoteForGettingCorpJunkBack);
    PyCallable_DECL_CALL(PayForReturnOfCorpJunk);


protected:
    Dispatcher *const m_dispatch;

    StationItem* pStationItem;    //we do not own this

    CorporationDB& m_db;
    const uint32 m_stationID;
};

PyCallable_Make_InnerDispatcher(CorpStationMgr)

CorpStationMgr::CorpStationMgr(PyServiceMgr *mgr)
: PyService(mgr, "corpStationMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpStationMgr, GetStationServiceStates)
    PyCallable_REG_CALL(CorpStationMgr, GetImprovementStaticData)
}

CorpStationMgr::~CorpStationMgr() {
    delete m_dispatch;
}


PyBoundObject *CorpStationMgr::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    if (!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }
    return new CorpStationMgrIMBound( m_manager, m_db, bind_args->AsInt()->value() );
}

PyResult CorpStationMgrIMBound::Handle_GetStationOffices( PyCallArgs& call )
{
    return pStationItem->GetOffices();
}

PyResult CorpStationMgrIMBound::Handle_GetNumberOfUnrentedOffices( PyCallArgs &call )
{
    return new PyInt(pStationItem->GetAvalibleOfficeCount());
}

PyResult CorpStationMgrIMBound::Handle_GetQuoteForRentingAnOffice(PyCallArgs &call)
{
    return new PyLong(pStationItem->GetOfficeRentalFee());
}

// cannot find this call in client code
PyResult CorpStationMgrIMBound::Handle_GetCorporateStationInfo(PyCallArgs &call) {
    // is this right?
    /* returns:
     *  list(
     *      eveowners:
     *          rowset: ownerID,ownerName,typeID
     *      corporations:
     *          rowset:corporationID,corporationName,description,shares,graphicID,memberCount,ceoID,stationID,raceID,corporationType,creatorID,hasPlayerPersonnelManager,tickerName,sendCharTerminationMessage,shape1,shape2,shape3,color1,color2,color3,typeface,memberLimit,allowedMemberRaceIDs,url,taxRate,minimumJoinStanding,division1,division2,division3,division4,division5,division6,division7,allianceID
     *      offices (may be None):
     *          rowset: corporationID,itemID,officeFolderID
     *      )
     */

    PyList *list = new PyList();
        list->AddItem(m_db.ListStationOwners(m_stationID));
        list->AddItem(m_db.ListStationCorps(m_stationID));
        list->AddItem(StationDB::GetOffices(m_stationID));/*m_db.ListStationOffices*/
    return list;
}

PyResult CorpStationMgrIMBound::Handle_SetCloneTypeID(PyCallArgs &call) {

    Call_SetCloneTypeID arg;
    if (!arg.Decode(&call.tuple)){
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
    }

    //Get cost of clone
    double cost = m_db.GetCloneTypeCostByID(arg.CloneTypeID);

    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC: Updating Clone in ";
    // make config option for station name or system name here?
    reason += call.client->GetSystemName();
    reason += " at ";
    reason += stDataMgr.GetStationName(call.client->GetStationID());
    AccountService::TranserFunds(
                    call.client->GetCharacterID(),
                    call.client->GetStationID(),
                    cost,
                    reason.c_str(),
                    Journal::EntryType::CloneActivation,
                    call.client->GetStationID(),
                    Account::KeyType::Cash);

    //update type of clone
    CharacterDB c_db;
    c_db.ChangeCloneType(call.client->GetCharacterID(), arg.CloneTypeID);
    return PyStatic.NewNone();
}

PyResult CorpStationMgrIMBound::Handle_RentOffice(PyCallArgs &call) {
    // corp role is checked in client before this button is shown.  no need to check here.
    // 1 param, corp rent price
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Client* pClient = call.client;

    // see if corp has office in station already.
    if (pStationItem->GetOfficeID(pClient->GetCorporationID()))
        throw UserError ("RentingYouHaveAnOfficeHere");

    // this may not be needed, as rental fee is queried immediately prior to this call
    if (arg.arg != pStationItem->GetOfficeRentalFee())
        _log(CORP__WARNING, "RentOffice() - Was quoted %i but station reports %u for office rental at %s", \
                arg.arg, pStationItem->GetOfficeRentalFee(), pStationItem->name());

    // check if the corp has enough money
    // remove the money and record the transaction
    std::string reason = "Office Rental at ";
    reason += pStationItem->itemName();
    reason += " by ";
    reason += pClient->GetCharName();
    AccountService::TranserFunds(pClient->GetCorporationID(), pStationItem->GetOwnerID(), arg.arg, reason.c_str(), Journal::EntryType::OfficeRentalFee);
/** @note  why is this disabled?
    int64 balance = AccountDB::GetCorpBalance(pClient->GetCorporationID(), Account::KeyType::Cash);
    if (balance < arg.arg) {
        std::map<std::string, PyRep *> args;
        args["amount"] = new PyFloat(arg.arg);
        args["balance"] = new PyFloat(balance);
        throw PyException(MakeUserError("NotEnoughMoney", args));
    }
*/
    OfficeData odata = OfficeData();
        odata.name = CorporationDB::GetCorpName(pClient->GetCorporationID());
        odata.ticker = pClient->GetChar()->corpTicker();
        odata.corporationID = pClient->GetCorporationID();
        odata.expiryTime = Win32TimeNow() + EvE::Time::Month;
        odata.lockDown = false;
        odata.rentalFee = arg.arg;
        odata.typeID = 27;  // office typeID
    pStationItem->RentOffice(odata);

    if (!odata.officeID) {
        codelog(CORP__ERROR, "%s: Error at renting a new office", pClient->GetName());
        return nullptr;
    }

    // send data to bill mgr for creating bill and notifications

    /*
     *        [PyString "OnNotificationReceived"]
     *        [PyList 0 items]
     *        [PyString "clientID"]           << Notify::Types::
     */

    /** @todo update this to use corpNotify */
    // This has to be sent to everyone in the station
    //  are corp members notified?
    OfficeAttributeUpdate change;
        change.oldOfficeFolderID = PyStatic.NewNone();
        change.oldOfficeID = PyStatic.NewNone();
        change.oldStationID = PyStatic.NewNone();
        change.oldTypeID = PyStatic.NewNone();
        change.newOfficeFolderID = odata.folderID;
        change.newOfficeID = odata.officeID;
        change.newStationID = odata.stationID;
        change.newTypeID = odata.typeID;
    OnObjectPublicAttributesUpdated update;
        update.notificationParams = new PyDict();
        update.realRowCount = 1; // what is this?
        update.bindID = GetBindStr();
        update.changePKIndexValue = odata.stationID; // primary key
        update.changes = change.Encode();
    PyTuple* payload = update.Encode();
    std::vector<Client*> cVec;
    cVec.clear();
    pStationItem->GetGuestList(cVec);
    for (auto cur : cVec) {
        PyIncRef(payload);
        cur->SendNotification("OnObjectPublicAttributesUpdated", "stationid", &payload, false);
    }
    PyDecRef( payload );

    // returns officeID
    return new PyInt(odata.officeID);
}

PyResult CorpStationMgrIMBound::Handle_GetCorporateStationOffice(PyCallArgs &call) {
  /*
   *        if not session.stationid2:
   *            return
   *        if self.itemIDOfficeFolderIDByCorporationID is None:
   *            self.itemIDOfficeFolderIDByCorporationID = util.IndexRowset(['corporationID', 'itemID', 'officeFolderID'], [], 'corporationID')
   *            corpStationMgr = self.GetCorpStationManager()
   *            corpStationMgr.Bind()
   *            offices = corpStationMgr.GetCorporateStationOffice()
   *            for office in offices:
   *                self.itemIDOfficeFolderIDByCorporationID[office.corporationID] = [office.corporationID, office.itemID, office.officeFolderID]
   */
  // this gets all corp offices in current station
    return pStationItem->GetOffices();
}

PyResult CorpStationMgrIMBound::Handle_MoveCorpHQHere(PyCallArgs &call)
{
    if (call.client->GetCorpHQ() == m_stationID)
        throw UserError ("CorpHQIsAtThisStation");

    uint32 corpID = call.client->GetCorporationID();
    CorporationDB::UpdateCorpHQ(corpID, m_stationID);

    // update all online members with new hq change.  no OnCorporationChanged bcast
    /** @todo update this */
    Client* pClient(nullptr);
    std::vector<uint32> ids;
    CorporationDB::GetMemberIDs(corpID, ids, true);
    for (auto cur : ids) {
        pClient = sEntityList.FindClientByCharID(cur);
        if (pClient != nullptr)
            pClient->GetChar()->SetCorpHQ(m_stationID);
    }

    return nullptr;
}

/**     ***********************************************************************
 * @note   these below are partially coded
 */

PyResult CorpStationMgrIMBound::Handle_GetPotentialHomeStations(PyCallArgs &call) {
    // this is for station options for xfering clone
    //returns stationID,typeID,serviceMask

    Client* pClient(call.client);
    std::vector<uint32> stVec;
    stVec.push_back(m_stationID);   // current station.  is this right?
    // first char home station always an option
    stVec.push_back(CharacterDB::GetStartingStationByCareer(pClient->GetChar()->careerID()));
    // get corp stations and add to vector
    m_db.GetCorpStations(pClient->GetCorporationID(), stVec);

    // TODO:  not sure how to determine possible stations yet...
    /*  ideas:
     *      get all corp offices.
     *      determine standings of requesting character to specific stations/locations
     *
     *      pc corp members - get corp standings vs. potential stations/corps/locations
     *      npc corp members - get faction standings for potential locations
     *
     *  other possible stations?  test for min/max potential station count?
     */

    PyDict* dict = new PyDict();
    PyList* list = new PyList();
    StationData data = StationData();
    for (auto cur : stVec) {
        stDataMgr.GetStationData(cur, data);
        dict->SetItemString("stationID", new PyInt(cur));
        dict->SetItemString("typeID", new PyInt(data.typeID));
        dict->SetItemString("serviceMask", new PyLong(data.serviceMask));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

PyResult CorpStationMgrIMBound::Handle_GetQuoteForGettingCorpJunkBack(PyCallArgs &call)
{   //cost = corpStationMgr.GetQuoteForGettingCorpJunkBack()
    _log(CORP__CALL, "CorpStationMgrIMBound::Handle_GetQuoteForGettingCorpJunkBack()");
    call.Dump(CORP__CALL_DUMP);

    // office rental fee (with multiplier?  config option for multiplier?)
    //stDataMgr.GetOfficeRentalFee(m_stationID);
    return new PyInt(pStationItem->GetOfficeRentalFee());
}

PyResult CorpStationMgrIMBound::Handle_DoesPlayersCorpHaveJunkAtStation(PyCallArgs &call)
{   //if corpStationMgr.DoesPlayersCorpHaveJunkAtStation():
    _log(CORP__CALL, "CorpStationMgrIMBound::Handle_DoesPlayersCorpHaveJunkAtStation()");
    call.Dump(CORP__CALL_DUMP);

    // query station for (officeID:flagimpounded)

    // returns boolean
    return PyStatic.NewFalse();
}

PyResult CorpStationMgrIMBound::Handle_SetHomeStation(PyCallArgs &call) {
    // sm.GetService('corp').GetCorpStationManager().SetHomeStation(newHomeStationID)
    /** @todo this is once a year on live, unless a new char changes corps.
     *  we will need to make other checks when this is called, as i dont think client checks anything.
     */
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    call.client->GetChar()->SetBaseID(arg.arg);
    CharacterDB::ChangeCloneLocation(call.client->GetCharID(), arg.arg);
    return nullptr;
}


/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(PyCallArgs &call) {
    // not sure what this actually does...
    //   corpStationMgr.DoStandingCheckForStationService(stationServiceID)
    /*
     * 01:09:09 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 01:09:09 [SvcCallDump]   Call Arguments:
     * 01:09:09 [SvcCallDump]       Tuple: 1 elements
     * 01:09:09 [SvcCallDump]         [ 0] Integer field: 4096  (repair)
     * 23:09:41 L Server: DoStandingCheckForStationService call made to
     * 23:09:41 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 23:09:41 [SvcCall]   Call Arguments:
     * 23:09:41 [SvcCall]       Tuple: 1 elements
     * 23:09:41 [SvcCall]         [ 0] Integer field: 8192
     * 23:09:41 L Server: DoStandingCheckForStationService call made to
     * 23:09:41 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 23:09:41 [SvcCall]   Call Arguments:
     * 23:09:41 [SvcCall]       Tuple: 1 elements
     * 23:09:41 [SvcCall]         [ 0] Integer field: 16384
     * 18:49:22 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 18:49:22 [SvcCall]   Call Arguments:
     * 18:49:22 [SvcCall]       Tuple: 1 elements
     * 18:49:22 [SvcCall]         [ 0] Integer field: 65536  (fitting)
     * 01:11:32 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 01:11:32 [SvcCallDump]   Call Arguments:
     * 01:11:32 [SvcCallDump]       Tuple: 1 elements
     * 01:11:32 [SvcCallDump]         [ 0] Integer field: 1048576  (insurance)
     */

    _log(CORP__CALL, "CorpStationMgrIMBound::Handle_DoStandingCheckForStationService()");
    call.Dump(CORP__CALL_DUMP);

    // returns None
    return PyStatic.NewNone();
}

PyResult CorpStationMgrIMBound::Handle_CancelRentOfOffice(PyCallArgs &call)
{   //  corpStationMgr.CancelRentOfOffice()
    _log(CORP__CALL, "CorpStationMgrIMBound::Handle_CancelRentOfOffice()");
    call.Dump(CORP__CALL_DUMP);

    /* this will need to search for items in office hangar.
     *   if none, then no worries
     *   if items, move them to 'impounded' (flagImpounded - corp junk)
     *
     *  not sure if the items change locations from corp officeID to stationID
     *   if so, cannot remove officeID from office map, but will need a flag to show it as 'not rented'
     */
    return nullptr;
}

PyResult CorpStationMgrIMBound::Handle_PayForReturnOfCorpJunk(PyCallArgs &call)
{   //    corpStationMgr.PayForReturnOfCorpJunk(cost)
    _log(CORP__CALL, "CorpStationMgrIMBound::Handle_PayForReturnOfCorpJunk()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpStationMgr::Handle_GetImprovementStaticData(PyCallArgs &call)
{
    //  more outpost shit.  we're nowhere near ready for this yet.
    //  more info in  client/script/ui/station/stationmanagement/base_stationmanagement.py
    /*
     *        outpostData = self.GetOutpostData()
     *        isd = sm.RemoteSvc('corpStationMgr').GetImprovementStaticData()
     *        outpostRaceID = cfg.invtypes.Get(outpostData.typeID).raceID
     *        outpostAsmLines = set([ each.assemblyLineTypeID for each in sm.ProxySvc('ramProxy').AssemblyLinesGet(eve.session.stationid) ])
     */
    _log(CORP__CALL, "CorpStationMgr::Handle_GetImprovementStaticData()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpStationMgr::Handle_GetStationServiceStates(PyCallArgs &call)
{
    /*   i *THINK* this is only sent for outposts.....stationID is 61m (above static stations)
     * since it has NOT been called yet on evemu, it very well could be outposts only (cause we dont have any)
     *
     *  **UPDATE**
     *   i was right.  these are outposts or conqurable stations.   found this in code...
     * if util.IsOutpost(eve.session.stationid) or sm.GetService('godma').GetType(eve.stationItem.stationTypeID).isPlayerOwnable == 1:
     *     self.serviceItemsState = sm.RemoteSvc('corpStationMgr').GetStationServiceStates()
     *
     *
     * ==================== Sent from Client 98 bytes
     *
     * [PyObjectData Name: macho.CallReq]
     *  [PyTuple 6 items]
     *    [PyInt 6]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 2]
     *        [PyInt 0]
     *        [PyIntegerVar 45]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 3 items]
     *        [PyInt 8]
     *        [PyString "corpStationMgr"]
     *        [PyNone]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 39 bytes]
     *          [PyTuple 4 items]
     *            [PyInt 1]
     *            [PyString "GetStationServiceStates"]
     *            [PyTuple 0 items]
     *            [PyDict 1 kvp]
     *              [PyString "machoVersion"]
     *              [PyInt 1]
     *    [PyNone]
     *
     *
     *
     * ==================== Sent from Server 347 bytes
     *
     * [PyObjectData Name: macho.CallRsp]
     *  [PyTuple 6 items]
     *    [PyInt 7]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 3 items]
     *        [PyInt 8]
     *        [PyString "corpStationMgr"]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 2]
     *        [PyIntegerVar 15001000001023]
     *        [PyIntegerVar 45]
     *        [PyNone]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PySubStream 279 bytes]
     *        [PyDict 6 kvp]
     *          [PyInt 512]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]              << stationID 61m = outpost
     *                [PyInt 512]
     *                [PyIntegerVar 318021030]
     *                [PyInt 1]
     *          [PyInt 16384]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 16384]
     *                [PyIntegerVar 318021027]
     *                [PyInt 1]
     *          [PyInt 4096]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 4096]
     *                [PyIntegerVar 318021029]
     *                [PyInt 1]
     *          [PyInt 8192]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 8192]
     *                [PyIntegerVar 318021028]
     *                [PyInt 1]
     *          [PyInt 16]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 16]
     *                [PyIntegerVar 1002331174723]
     *                [PyInt 1]
     *          [PyInt 65536]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 65536]
     *                [PyIntegerVar 318021026]
     *                [PyInt 1]
     *    [PyNone]
     */
    _log(CORP__CALL, "CorpStationMgr::Handle_GetStationServiceStates()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}
