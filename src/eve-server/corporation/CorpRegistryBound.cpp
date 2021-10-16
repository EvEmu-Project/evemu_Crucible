/*      CorpRegistryBound.cpp
 *      bound object to access functions for specific corp
 *
 *
 */

#include <string>

#include "EVE_Corp.h"
#include "EVE_Mail.h"
#include "CorpData.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "corporation/CorpRegistryBound.h"
#include "corporation/SparseBound.h"
#include "station/StationDB.h"
#include "station/StationDataMgr.h"
#include "alliance/AllianceDB.h"
#include "alliance/AllianceBound.h"

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

CorpRegistryBound::CorpRegistryBound(PyServiceMgr *mgr, CorporationDB& db, uint32 corpID)
: PyBoundObject(mgr),
 m_db(db),
 m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "CorpRegistryBound";

    PyCallable_REG_CALL(CorpRegistryBound, GetEveOwners);
    PyCallable_REG_CALL(CorpRegistryBound, GetCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetCorporations);
    PyCallable_REG_CALL(CorpRegistryBound, GetInfoWindowDataForChar);
    PyCallable_REG_CALL(CorpRegistryBound, GetLockedItemLocations);
    PyCallable_REG_CALL(CorpRegistryBound, AddCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedTickerNames);
    PyCallable_REG_CALL(CorpRegistryBound, GetOffices);
    PyCallable_REG_CALL(CorpRegistryBound, GetStations);

    PyCallable_REG_CALL(CorpRegistryBound, CreateRecruitmentAd);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateRecruitmentAd);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteRecruitmentAd);
    PyCallable_REG_CALL(CorpRegistryBound, GetRecruiters);
    PyCallable_REG_CALL(CorpRegistryBound, GetRecruitmentAdsForCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetMyApplications);
    PyCallable_REG_CALL(CorpRegistryBound, InsertApplication);
    PyCallable_REG_CALL(CorpRegistryBound, GetApplications);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateApplicationOffer);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteApplication);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateDivisionNames);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateLogo);
    PyCallable_REG_CALL(CorpRegistryBound, SetAccountKey);
    PyCallable_REG_CALL(CorpRegistryBound, GetMember);
    PyCallable_REG_CALL(CorpRegistryBound, GetMembers);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateMember);

    PyCallable_REG_CALL(CorpRegistryBound, MoveCompanyShares);
    PyCallable_REG_CALL(CorpRegistryBound, MovePrivateShares);
    PyCallable_REG_CALL(CorpRegistryBound, GetSharesByShareholder);
    PyCallable_REG_CALL(CorpRegistryBound, GetShareholders);
    PyCallable_REG_CALL(CorpRegistryBound, PayoutDividend);

    PyCallable_REG_CALL(CorpRegistryBound, CanViewVotes);
    PyCallable_REG_CALL(CorpRegistryBound, InsertVoteCase);
    PyCallable_REG_CALL(CorpRegistryBound, GetVotes);
    PyCallable_REG_CALL(CorpRegistryBound, CanVote);
    PyCallable_REG_CALL(CorpRegistryBound, InsertVote);
    PyCallable_REG_CALL(CorpRegistryBound, GetVoteCasesByCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetVoteCaseOptions);
    PyCallable_REG_CALL(CorpRegistryBound, GetSanctionedActionsByCorporation);

    PyCallable_REG_CALL(CorpRegistryBound, AddBulletin);
    PyCallable_REG_CALL(CorpRegistryBound, GetBulletins);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteBulletin);

    PyCallable_REG_CALL(CorpRegistryBound, CreateLabel);
    PyCallable_REG_CALL(CorpRegistryBound, GetLabels);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteLabel);
    PyCallable_REG_CALL(CorpRegistryBound, EditLabel);
    PyCallable_REG_CALL(CorpRegistryBound, AssignLabels);
    PyCallable_REG_CALL(CorpRegistryBound, RemoveLabels);

    PyCallable_REG_CALL(CorpRegistryBound, GetRecentKillsAndLosses);
    PyCallable_REG_CALL(CorpRegistryBound, GetRoleGroups);
    PyCallable_REG_CALL(CorpRegistryBound, GetRoles);
    PyCallable_REG_CALL(CorpRegistryBound, GetLocationalRoles);

    PyCallable_REG_CALL(CorpRegistryBound, GetTitles);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateTitle);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateTitles);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteTitle);
    PyCallable_REG_CALL(CorpRegistryBound, ExecuteActions);

    PyCallable_REG_CALL(CorpRegistryBound, GetCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryBound, AddCorporateContact);
    PyCallable_REG_CALL(CorpRegistryBound, EditContactsRelationshipID);
    PyCallable_REG_CALL(CorpRegistryBound, RemoveCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryBound, EditCorporateContact);

    PyCallable_REG_CALL(CorpRegistryBound, CreateAlliance);
    PyCallable_REG_CALL(CorpRegistryBound, ApplyToJoinAlliance);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteAllianceApplication);
    PyCallable_REG_CALL(CorpRegistryBound, GetAllianceApplications);
    PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedAllianceShortNames);

    PyCallable_REG_CALL(CorpRegistryBound, GetMembersPaged);
    PyCallable_REG_CALL(CorpRegistryBound, GetMembersByIds);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberIDsWithMoreThanAvgShares);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberIDsByQuery);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberTrackingInfo);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberTrackingInfoSimple);
    PyCallable_REG_CALL(CorpRegistryBound, GetRentalDetailsPlayer);
    PyCallable_REG_CALL(CorpRegistryBound, GetRentalDetailsCorp);

    PyCallable_REG_CALL(CorpRegistryBound, UpdateCorporationAbilities);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateStationManagementSettings);
    PyCallable_REG_CALL(CorpRegistryBound, GetNumberOfPotentialCEOs);

    PyCallable_REG_CALL(CorpRegistryBound, CanLeaveCurrentCorporation);

    m_corpID = corpID;
}

PyResult CorpRegistryBound::Handle_GetLocationalRoles(PyCallArgs &call)
{       // not sure if this is used...
    PyList* list = new PyList();
    list->AddItem(new PyInt(Corp::Role::HangarCanTake1));
    list->AddItem(new PyInt(Corp::Role::HangarCanTake2));
    list->AddItem(new PyInt(Corp::Role::HangarCanTake3));
    list->AddItem(new PyInt(Corp::Role::HangarCanTake4));
    list->AddItem(new PyInt(Corp::Role::HangarCanTake5));
    list->AddItem(new PyInt(Corp::Role::HangarCanTake6));
    list->AddItem(new PyInt(Corp::Role::HangarCanTake7));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery1));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery2));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery3));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery4));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery5));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery6));
    list->AddItem(new PyInt(Corp::Role::HangarCanQuery7));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake1));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake2));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake3));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake4));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake5));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake6));
    list->AddItem(new PyLong(Corp::Role::ContainerCanTake7));

    return list;
}

PyResult CorpRegistryBound::Handle_GetEveOwners(PyCallArgs &call) {
    /* this is a method-chaining call.
     * it comes with the bind request for a particular corp.
     *
     * the client wants a member list for given corp
     */
    return m_db.GetEveOwners(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetInfoWindowDataForChar( PyCallArgs& call )
{
    // takes characterID
    // returns corpID, allianceID, title
    Call_SingleIntegerArg args;

    // if no characterID is specified, just return information for current character
    if (args.Decode (call.tuple) == false)
        return CharacterDB::GetInfoWindowDataForChar (call.client->GetCharacterID ());

    return CharacterDB::GetInfoWindowDataForChar (args.arg);
}

PyResult CorpRegistryBound::Handle_GetCorporation(PyCallArgs &call) {
    // called by member of this corp
    return m_db.GetCorporation(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetRoles(PyCallArgs &call)
{   // working
    return m_db.GetCorpRoles();
}

PyResult CorpRegistryBound::Handle_GetRoleGroups(PyCallArgs &call)
{   // working
    return m_db.GetCorpRoleGroups();
}

PyResult CorpRegistryBound::Handle_GetTitles(PyCallArgs &call)
{   // working
    return m_db.GetTitles(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetBulletins(PyCallArgs &call)
{   // working
    return m_db.GetBulletins(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetCorporateContacts(PyCallArgs &call)
{   // working
    return m_db.GetContacts(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetApplications(PyCallArgs &call)
{   // working
    return m_db.GetApplications(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetRecruitmentAdsForCorporation(PyCallArgs &call)
{   // recruitments = self.GetCorpRegistry().GetRecruitmentAdsForCorporation()
    return m_db.GetAdRegistryData();
}

PyResult CorpRegistryBound::Handle_GetMyApplications(PyCallArgs &call)
{   // working
    return m_db.GetMyApplications(call.client->GetCharacterID());
}

PyResult CorpRegistryBound::Handle_GetMemberTrackingInfo(PyCallArgs &call)
{   // working
    return m_db.GetMemberTrackingInfo(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetMemberTrackingInfoSimple(PyCallArgs &call)
{   // working
    return m_db.GetMemberTrackingInfoSimple(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetShareholders(PyCallArgs &call)
{   // working
    return m_db.GetShares(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetLabels(PyCallArgs &call)
{   // working
    return m_db.GetLabels(call.client->GetCorporationID());
}

PyResult CorpRegistryBound::Handle_CanViewVotes(PyCallArgs &call)
{   // working
    return m_db.PyHasShares(call.client->GetCharacterID(), m_corpID);
}

PyResult CorpRegistryBound::Handle_GetMemberIDsWithMoreThanAvgShares(PyCallArgs &call)
{   // working
    return m_db.GetSharesForCorp(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetRecruiters(PyCallArgs &call)
{   // working
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetRecruiters(args.arg1, args.arg2);
}

PyResult CorpRegistryBound::Handle_DeleteTitle(PyCallArgs &call)
{   // working
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.DeleteTitle(call.client->GetCorporationID(), arg.arg);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteRecruitmentAd(PyCallArgs &call)
{   // working
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.DeleteAdvert(arg.arg);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetSharesByShareholder(PyCallArgs &call)
{   // working
    Call_SingleBoolArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 owner = (arg.arg ? call.client->GetCorporationID() : call.client->GetCharacterID());
    return m_db.GetMyShares(owner);
}

PyResult CorpRegistryBound::Handle_GetCorporations(PyCallArgs &call) {
    // working
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    return m_db.GetCorporations(args.arg);
}

PyResult CorpRegistryBound::Handle_GetRecentKillsAndLosses(PyCallArgs &call)
{   // working
    Call_GetRecentKillsAndLosses args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetKillsAndLosses(m_corpID, args.number, args.offset);
}

PyResult CorpRegistryBound::Handle_GetMember(PyCallArgs &call)
{   // not working
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetMember()");
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyRep* rep = m_db.GetMember(args.arg);

    if (is_log_enabled(CORP__RSP_DUMP))
        rep->Dump(CORP__RSP_DUMP, "");

    return rep;
}

PyResult CorpRegistryBound::Handle_SetAccountKey(PyCallArgs &call)
{   // working
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    call.client->GetChar()->SetAccountKey(args.arg);
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult CorpRegistryBound::Handle_GetSuggestedTickerNames(PyCallArgs &call)
{
    Call_SingleWStringArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyList* result = new PyList();
    SuggestedTickerName sTN;
    sTN.tName.clear();
    uint32 cnLen = args.arg.length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0; i < cnLen; ++i)
        if (args.arg[i] >= 'A' && args.arg[i] <= 'Z')
            sTN.tName += args.arg[i];

    result->AddItem( sTN.Encode() );
    return result;
}

PyResult CorpRegistryBound::Handle_GetSuggestedAllianceShortNames(PyCallArgs &call)
{
    Call_SingleWStringArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    PyList* result = new PyList();
    SuggestedShortName sSN;
    sSN.sName.clear();
    uint32 cnLen = args.arg.length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0; i < cnLen; ++i)
        if (args.arg[i] >= 'A' && args.arg[i] <= 'Z')
            sSN.sName += args.arg[i];

    result->AddItem( sSN.Encode() );
    return result;
}

PyResult CorpRegistryBound::Handle_GetMembers(PyCallArgs &call)
{   // working
    // this just wants a member count and time (and headers)
    uint16 rowCount = m_db.GetMemberCount(m_corpID);

    /*  update....do we need to send header also??
     *
        [PyObjectData Name: util.SparseRowset]
          [PyTuple 3 items]
            [PyList 19 items]
              [PyString "characterID"]
              [PyString "corporationID"]
              [PyString "divisionID"]
              [PyString "squadronID"]
              [PyString "title"]
              [PyString "roles"]
              [PyString "grantableRoles"]
              [PyString "startDateTime"]
              [PyString "baseID"]
              [PyString "rolesAtHQ"]
              [PyString "grantableRolesAtHQ"]
              [PyString "rolesAtBase"]
              [PyString "grantableRolesAtBase"]
              [PyString "rolesAtOther"]
              [PyString "grantableRolesAtOther"]
              [PyString "titleMask"]
              [PyString "accountKey"]
              [PyString "rowDate"]
              [PyString "blockRoles"]
            [PySubStruct]
              [PySubStream 51 bytes]
                [PyTuple 3 items]
                  [PyString "N=700149:17018"]
                  [PyDict 1 kvp]
                    [PyString "realRowCount"]
                    [PyInt 3]
                  [PyIntegerVar 129515350203058462]
            [PyInt 3]
    */

    PyDict *dict = new PyDict();
        dict->SetItemString("realRowCount", new PyInt(rowCount));   // this is current member count
    PyTuple* boundObject = new PyTuple(3);
        boundObject->SetItem(0, new PyString(GetBindStr()));    // node info here
        boundObject->SetItem(1, dict);
        boundObject->SetItem(2, new PyLong(GetFileTimeNow()));

    GetMembersSparseRowset ret;
        ret.boundObject = boundObject;
        ret.realRowCount = rowCount;
    return ret.Encode();
}

PyResult CorpRegistryBound::Handle_UpdateDivisionNames(PyCallArgs &call)
{   // working
    Call_UpdateDivisionNames args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    Notify_IntRaw notif;
        notif.key = m_corpID;
        notif.data = new PyDict();

    if (!m_db.UpdateDivisionNames(notif.key, args, notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update division names for corp %u", call.client->GetName(), notif.key);
        PyDecRef( notif.data );
        return PyStatic.NewNone();
    }

    // Only send notification if it is needed...
    if (notif.data->items.size()) {
        /** @todo update this to use CorpNotify() */
        MulticastTarget mct;
            mct.corporations.insert(notif.key);
        PyTuple * answer = notif.Encode();
        sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);
        call.client->SendNotification("OnCorporationChanged", "clientID", &answer);
    }

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult CorpRegistryBound::Handle_GetMembersPaged(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetMembersPaged(page)

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    DBQueryResult res;
    m_db.GetMembersPaged(m_corpID, arg.arg, res);
    //uint32 rowCount = (uint32)res.GetRowCount();

    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        //SELECT characterID, corporationID, title, rolesAtAll, grantableRoles, startDateTime, rolesAtHQ, grantableRolesAtHQ, \
          rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther, titleMask, corpAccountKey, baseID, blockRoles, name
        PyDict* dict = new PyDict();
        dict->SetItemString( "characterID",             new PyInt(row.GetInt(0)));
        dict->SetItemString( "corporationID",           new PyInt(row.GetInt(1)));
        dict->SetItemString( "divisionID",              new PyInt(0));
        dict->SetItemString( "squadronID",              new PyInt(0));
        dict->SetItemString( "title",                   new PyInt(row.GetInt(2)));
        dict->SetItemString( "roles",                   new PyLong(row.GetInt64(3)));
        dict->SetItemString( "grantableRoles",          new PyInt(row.GetInt(4)));
        dict->SetItemString( "startDateTime",           new PyLong(row.GetInt64(5)));
        dict->SetItemString( "rolesAtHQ",               new PyLong(row.GetInt64(6)));
        dict->SetItemString( "grantableRolesAtHQ",      new PyLong(row.GetInt64(7)));
        dict->SetItemString( "rolesAtBase",             new PyLong(row.GetInt64(8)));
        dict->SetItemString( "grantableRolesAtBase",    new PyLong(row.GetInt64(9)));
        dict->SetItemString( "rolesAtOther",            new PyLong(row.GetInt64(10)));
        dict->SetItemString( "grantableRolesAtOther",   new PyLong(row.GetInt64(11)));
        dict->SetItemString( "titleMask",               new PyLong(row.GetInt64(12))); // titleID
        dict->SetItemString( "accountKey",              new PyInt(row.GetInt(13)));
        dict->SetItemString( "rowDate",                 new PyLong(GetFileTimeNow())); //may not be right
        dict->SetItemString( "baseID",                  new PyInt(row.GetInt(14))); /** @todo update this */
        dict->SetItemString( "blockRoles",              new PyBool(row.GetInt(15)));
        dict->SetItemString( "ownerName",               new PyString(row.GetText(16)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    if (is_log_enabled(CORP__RSP_DUMP))
        list->Dump(CORP__RSP_DUMP, "");

    return list;
}

PyResult CorpRegistryBound::Handle_GetMembersByIds(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetMembersByIds(memberIDs)
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetMembersByIds()");
    call.Dump(CORP__CALL_DUMP);

    Call_GetMembersByID args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyList* list = new PyList();
    for (PyList::const_iterator itr = args.memberIDs->begin(); itr != args.memberIDs->end(); ++itr)
        list->AddItem(new PyObject("util.KeyVal", m_db.GetMember(PyRep::IntegerValueU32(*itr))));

    if (is_log_enabled(CORP__RSP_DUMP))
        list->Dump(CORP__RSP_DUMP, "");

    return list;
}

PyResult CorpRegistryBound::Handle_AddCorporation(PyCallArgs &call) {
    Client* pClient(call.client);
    Call_AddCorporation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // verify they're not using bad words in their corp name
    for (const auto cur : badWords)
        if (EvE::icontains(args.corpName, cur))
            throw CustomError ("Corp Name contains banned words");

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(args.corpName, cur))
            throw CustomError ("Corp Name contains invalid characters");


    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(args.corpTicker, cur))
            throw CustomError ("Corp Ticker contains invalid characters");

    // verify ticker is available
    if (m_db.IsTickerTaken(args.corpTicker))
        throw UserError ("CorpTickerNameInvalidTaken");

    double corp_cost = sConfig.rates.corpCost;
    if (pClient->GetBalance(Account::CreditType::ISK) < corp_cost) {
        _log(SERVICE__ERROR, "%s: Cannot afford corporation startup costs!", pClient->GetName());
        // send error to client here?
        return nullptr;
    }

    //adding a corporation will affect eveStaticOwners, so we gotta invalidate the cache...
    //  call to db.AddCorporation() will update eveStaticOwners with new corp
    PyString* cache_name = new PyString( "config.StaticOwners" );
    m_manager->cache_service->InvalidateCache( cache_name );
    PySafeDecRef( cache_name );

    // Register new corp
    uint32 corpID(0);
    if (!m_db.AddCorporation(args, pClient, corpID)) {
        codelog(SERVICE__ERROR, "New corporation creation failed.");
        return nullptr;
    }

    // make sure AI is set correctly in db.  sometimes it's not (cause i forget)
    if (!IsPlayerCorp(corpID)) {
        // not sure what to do here yet....delete everything, make sure db autoincrement is set right then reapply
        _log(CORP__ERROR, "Auto Increment improperly set in DB.");
        pClient->SendErrorMsg("Server Error when creating corp.  CorpID invalid.");
        return nullptr;
    }

    // create default role title data
    m_db.CreateTitleData(corpID);

    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC: Creating new corporation: ";
    reason += args.corpName;
    reason += " (";
    reason += args.corpTicker;
    reason += ")";
    AccountService::TranserFunds(
                                pClient->GetCharacterID(),
                                m_db.GetStationOwner(pClient->GetStationID()),  // station owner files paperwork, this is fee for that
                                corp_cost,
                                reason.c_str(),
                                Journal::EntryType::CorporationRegistrationFee,
                                pClient->GetStationID(),
                                Account::KeyType::Cash);

    // add corp event for creating new corp
    m_db.AddItemEvent(corpID, pClient->GetCharacterID(), Corp::EventType::CreatedCorporation);

    // create corp channel
    m_manager->lsc_service->CreateSystemChannel(corpID);

    CorpData data = CorpData();
        data.name = args.corpName;
        data.ticker = args.corpTicker;
        data.taxRate = args.taxRate;
        data.baseID = pClient->GetLocationID();
        data.corpHQ = pClient->GetLocationID();
        data.corporationID = corpID;
        data.corpAccountKey = Account::KeyType::Cash;
        data.corpRole = Corp::Role::Admin;
        data.rolesAtAll = Corp::Role::Admin;
        data.rolesAtBase = Corp::Role::Admin;
        data.rolesAtHQ = Corp::Role::Admin;
        data.rolesAtOther = Corp::Role::Admin;
        data.grantableRoles = Corp::Role::Admin;
        data.grantableRolesAtBase = Corp::Role::Admin;
        data.grantableRolesAtHQ = Corp::Role::Admin;
        data.grantableRolesAtOther = Corp::Role::Admin;
    // update corp data and refresh session data.
    pClient->GetChar()->JoinCorporation(data);

    // Here we send a notification about creating a new corporation...
    OnCorporationChanged cc;
    cc.corpID = corpID;
    if (!m_db.CreateCorporationCreatePacket(cc, m_corpID, corpID)) {
        codelog(SERVICE__ERROR, "Failed to create OnCorpChanged notification stream.");
        // This is a big problem, because this way we won't be able to see the difference...
        pClient->SendErrorMsg("Unable to notify about corp creation.");
        return nullptr;
    }
    PyTuple* a1 = cc.Encode();
    // send single to client
    pClient->SendNotification("OnCorporationChanged", "clientID", &a1);
    // send multi to station guests
    PyIncRef(a1);
    sEntityList.Multicast("OnCorporationChanged", "stationid", &a1, NOTIF_DEST__LOCATION, pClient->GetLocationID());

    return m_db.GetCorporations(corpID);
}

PyResult CorpRegistryBound::Handle_UpdateTitle(PyCallArgs &call) {
    // self.GetCorpRegistry().UpdateTitle(titleID, titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther)
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateTitle()");
    //call.Dump(CORP__CALL_DUMP);

    Call_UpdateTitleData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyDict* updates = new PyDict();
    if (m_db.UpdateTitle(m_corpID, args, updates)) {
        /** @todo update this to use CorpNotify() */
        OnTitleChanged change;
            change.corpID = m_corpID;
            change.titleID = args.titleID;
            change.changes = updates;
        PyTuple* notif = change.Encode();
        MulticastTarget mct;
            mct.corporations.insert(m_corpID);
        sEntityList.Multicast("OnTitleChanged", "corpid", &notif, mct);
    }

    //OnTitleChanged
    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateTitles(PyCallArgs &call) {
    //    self.GetCorpRegistry().UpdateTitles(titles)
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateTitles()");
    //call.Dump(CORP__CALL_DUMP);
    if (!call.tuple->GetItem(0)->IsObject()) {
        codelog(CORP__ERROR, "Tuple Item is wrong type: %s.  Expected PyObject.", call.tuple->GetItem(0)->TypeString());
        return nullptr;
    }
    Call_UpdateTitle arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!arg.object->arguments()->IsDict()) {
        codelog(CORP__ERROR, "Object Argument is wrong type: %s.  Expected PyDict.", arg.object->arguments()->TypeString());
        return nullptr;
    }

    PyDict* dict = arg.object->arguments()->AsDict();
    //dict->Dump(CORP__TRACE, "    ");
    PyRep* rep = dict->GetItemString("lines");
    if (!rep->IsList()) {
        codelog(CORP__ERROR, "'lines' item is not PyList: %s", rep->TypeString());
        return nullptr;
    }

    Call_UpdateTitleData args;
    PyList* list = rep->AsList();
    for (PyList::const_iterator itr = list->begin(); itr != list->end(); ++itr) {
        if (!(*itr)->IsList()) {
            codelog(CORP__ERROR, "itr item is not PyList: %s", (*itr)->TypeString());
            continue;
        }
        PyDict* updates = new PyDict();
        PyList* list2 = (*itr)->AsList();
        args.titleID = list2->GetItem(0)->AsInt()->value();
        args.titleName = PyRep::StringContent(list2->GetItem(1));
        args.roles = PyRep::IntegerValue(list2->GetItem(2));
        args.grantableRoles = PyRep::IntegerValue(list2->GetItem(3));
        args.rolesAtHQ = PyRep::IntegerValue(list2->GetItem(4));
        args.grantableRolesAtHQ = PyRep::IntegerValue(list2->GetItem(5));
        args.rolesAtBase = PyRep::IntegerValue(list2->GetItem(6));
        args.grantableRolesAtBase = PyRep::IntegerValue(list2->GetItem(7));
        args.rolesAtOther = PyRep::IntegerValue(list2->GetItem(8));
        args.grantableRolesAtOther = PyRep::IntegerValue(list2->GetItem(9));
        if (m_db.UpdateTitle(m_corpID, args, updates)) {
            /** @todo update this to use CorpNotify() */
            OnTitleChanged change;
                change.corpID = m_corpID;
                change.titleID = args.titleID;
                change.changes = updates;
            PyTuple* notif = change.Encode();
            MulticastTarget mct;
                mct.corporations.insert(m_corpID);
            sEntityList.Multicast("OnTitleChanged", "corpid", &notif, mct);
        }
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateCorporation(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateCorporation() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateCorporation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Notify_IntRaw notif;
    notif.key = m_corpID;
    notif.data = new PyDict();

    if (!m_db.UpdateCorporation(notif.key, args, notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update corporation data for corp %u", call.client->GetName(), notif.key);
        PyDecRef( notif.data );
        return PyStatic.NewNone();
    }

    // Only send notification if it is needed...
    if (notif.data->items.size()) {
        /** @todo update this to use CorpNotify() */
        MulticastTarget mct;
        mct.corporations.insert(notif.key);
        PyTuple * answer = notif.Encode();
        sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);
        call.client->SendNotification("OnCorporationChanged", "clientID", &answer);
    }

    return PyStatic.NewNone();
}

PyResult CorpRegistryBound::Handle_UpdateLogo(PyCallArgs &call)
{
    Call_UpdateLogo args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Notify_IntRaw notif;
    notif.key = m_corpID;

    // Check if we have enough money
    double logo_change = 100;
    if (AccountDB::GetCorpBalance(notif.key, Account::KeyType::Cash) < logo_change) {
        _log( SERVICE__ERROR, "%s: Cannot afford corporation logo change costs", call.client->GetName());
        call.client->SendErrorMsg("Your corporation doesn't have enough money (%u ISK) to change it's logo.", logo_change);
        return nullptr;
    }

    notif.data = new PyDict();
    if (!m_db.UpdateLogo(notif.key, args, notif.data))    {
        codelog( SERVICE__ERROR, "Corporation logo change failed..." );
        PyDecRef( notif.data );
        return nullptr;
    }

    std::string reason = "Change Corporation Logo.";
    // move cash and record the transaction.
    AccountService::TranserFunds(
        call.client->GetCharacterID(),
                                 m_db.GetStationOwner(call.client->GetStationID()),
                                 logo_change,
                                 reason,
                                 Journal::EntryType::CorporationLogoChangeCost,
                                 Account::KeyType::Cash);   // main wallet

    // Send notification to those in the station
    /** @todo update this to use CorpNotify() */
    MulticastTarget mct;
    mct.locations.insert(call.client->GetLocationID());
    PyTuple *answer = notif.Encode();
    sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);

    return m_db.GetCorporation(notif.key);
}

PyResult CorpRegistryBound::Handle_AddBulletin(PyCallArgs &call) {
    // self.GetCorpRegistry().AddBulletin(title, body)
    // self.GetCorpRegistry().AddBulletin(title, body, bulletinID=bulletinID, editDateTime=editDateTime)    <-- this is to update bulletin
    _log(CORP__CALL, "CorpRegistryBound::Handle_AddBulletin()");
    call.Dump(CORP__CALL_DUMP);

    Call_AddBulletin args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    bool edit = false;
    int64 editDateTime = 0;
    if (call.byname.find("editDateTime") != call.byname.end()) {
        if (call.byname.find("editDateTime")->second->IsLong()) {
            edit = true;
            editDateTime = PyInt::IntegerValue(call.byname.find("editDateTime")->second);
        } else {
            _log(CORP__ERROR, "Handle_AddBulletin - editDateTime is of the wrong type: '%s'.  Expected PyString or PyWString.", \
                            call.byname.find("editDateTime")->second->TypeString());
        }
    }

    int64 bulletinID = 0;
    if (call.byname.find("bulletinID") != call.byname.end()) {
        if (call.byname.find("bulletinID")->second->IsInt()) {
            edit = true;
            bulletinID = PyInt::IntegerValue(call.byname.find("bulletinID")->second);
        } else {
            _log(CORP__ERROR, "Handle_AddBulletin - bulletinID is of the wrong type: '%s'.  Expected PyInt.", \
                            call.byname.find("bulletinID")->second->TypeString());
        }
    }

    if (edit) {
        if (bulletinID >= 100000) {
            AllianceDB::EditBulletin(bulletinID, call.client->GetCharacterID(), editDateTime, args.title, args.body);
        } else {
            m_db.EditBulletin(bulletinID, call.client->GetCharacterID(), editDateTime, args.title, args.body);
        }

    } else {
        m_db.AddBulletin(m_corpID, m_corpID, call.client->GetCharacterID(), args.title, args.body);
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteBulletin(PyCallArgs &call) {
    //self.GetCorpRegistry().DeleteBulletin(id)
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteBulletin() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    uint32 bulletinID = PyRep::IntegerValue(call.tuple->GetItem(0));
    if (bulletinID >= 100000) {
        AllianceDB::DeleteBulletin(bulletinID);
    } else {
        m_db.DeleteBulletin(bulletinID);
    }
    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateRecruitmentAd(PyCallArgs &call) {
    // return self.GetCorpRegistry().CreateRecruitmentAd(days, typeMask, allianceID, description, channelID, recruiters, title)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CreateRecruitmentAd()");
    call.Dump(CORP__CALL_DUMP);
    /*
     * 00:41:50 [SvcCall] Service CorpRegistryBound::CreateRecruitmentAd()
     * 00:41:50 W CorpRegistryBound::Handle_CreateRecruitmentAd(): size= 7
     * 00:41:50 [CorpCallDump]   Call Arguments:
     * 00:41:50 [CorpCallDump]      Tuple: 7 elements
     * 00:41:50 [CorpCallDump]       [ 0]    Integer: 14
     * 00:41:50 [CorpCallDump]       [ 1]    Integer: 34672663
     * 00:41:50 [CorpCallDump]       [ 2]       None
     * 00:41:50 [CorpCallDump]       [ 3]    WString: 'test'
     * 00:41:50 [CorpCallDump]       [ 4]    Integer: 0
     * 00:41:50 [CorpCallDump]       [ 5]   List: 1 elements
     * 00:41:50 [CorpCallDump]       [ 5]   [ 0]    Integer: 90000000
     * 00:41:50 [CorpCallDump]       [ 6]    WString: 'test'
     */

    Call_CreateRecruitmentAd args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 amount = 1250000;
    switch (args.days) {
        case 7:    amount = 2250000;  break;
        case 14:   amount = 4000000;  break;
        case 28:   amount = 7500000;  break;
    }

    AccountService::TranserFunds(m_corpID, call.client->GetCorpHQ(), amount, "Initial Advert Time for Corp Recruit Advert", Journal::EntryType::CorporationAdvertisementFee, call.client->GetCharacterID());

    int32 adID = m_db.CreateAdvert(call.client, m_corpID, args.typeMask, args.days, m_db.GetCorpMemberCount(m_corpID), args.description, args.channelID, args.title);

    std::vector<int32> recruiters;
    recruiters.clear();
    for (PyList::const_iterator itr = args.recruiters->begin(); itr != args.recruiters->end(); ++itr)
        recruiters.push_back(PyRep::IntegerValue(*itr));
    //recruiters.push_back((*itr)->AsInt()->value());

    // if no recruiters defined, default to creating character
    if (recruiters.empty())
        recruiters.push_back(call.client->GetCharacterID());

    m_db.AddRecruiters(adID, (int32)m_corpID, recruiters);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateRecruitmentAd(PyCallArgs &call) {
    // return self.GetCorpRegistry().UpdateRecruitmentAd(adID, typeMask, description, channelID, recruiters, title, addedDays)
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateRecruitmentAd() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    /*
     * 04:47:40 [CorpCall] CorpRegistryBound::Handle_UpdateRecruitmentAd() size=7
     * 04:47:40 [CorpCallDump]   Call Arguments:
     * 04:47:40 [CorpCallDump]      Tuple: 7 elements
     * 04:47:40 [CorpCallDump]       [ 0]    Integer: 3
     * 04:47:40 [CorpCallDump]       [ 1]    Integer: 34869363
     * 04:47:40 [CorpCallDump]       [ 2]    WString: 'test advert msg'
     * 04:47:40 [CorpCallDump]       [ 3]    Integer: 0
     * 04:47:40 [CorpCallDump]       [ 4]   List: 3 elements
     * 04:47:40 [CorpCallDump]       [ 4]   [ 0]    Integer: 90000000
     * 04:47:40 [CorpCallDump]       [ 4]   [ 1]    Integer: 91000001
     * 04:47:40 [CorpCallDump]       [ 4]   [ 2]    Integer: 91000003
     * 04:47:40 [CorpCallDump]       [ 5]    WString: 'mining corp'
     * 04:47:40 [CorpCallDump]       [ 6]    Integer: 0
     */

    Call_UpdateRecruitmentAd args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint8 days = 1;
    // check that corp can afford added time, if requested
    if (args.addedDays) {
        // 3d = .75misk, 7d = 1.75misk, 14d = 3.5misk
        uint32 amount = 750000;
        switch (args.addedDays) {
            case 7:    amount = 1750000;  break;
            case 14:   amount = 3500000;  break;
        }

        AccountService::TranserFunds(m_corpID, call.client->GetCorpHQ(), amount, "Added Advert Time to Corp Recruit Advert", Journal::EntryType::CorporationAdvertisementFee);

        // do some funky shit to determine days left for advert then add additional time if requested
        int64 time = m_db.GetAdvertTime(args.adID, m_corpID);
        if (time > 0) {
            time -= GetFileTimeNow();
            days = time / EvE::Time::Day;
            days += args.addedDays;
        }
    }

    m_db.UpdateAdvert(args.adID, m_corpID, args.typeMask, days, m_db.GetCorpMemberCount(m_corpID), args.description, args.channelID, args.title);

    std::vector<int32> recruiters;
    recruiters.clear();
    for (PyList::const_iterator itr = args.recruiters->begin(); itr != args.recruiters->end(); ++itr)
        recruiters.push_back(PyRep::IntegerValue(*itr));

    // if no recruiters defined, default to creating character
    if (recruiters.empty())
        recruiters.push_back(call.client->GetCharacterID());

    m_db.AddRecruiters(args.adID, (int32)m_corpID, recruiters);

    return m_db.GetAdvert(args.adID);
}

PyResult CorpRegistryBound::Handle_MoveCompanyShares(PyCallArgs &call) {
    // return self.GetCorpRegistry().MoveCompanyShares(corporationID, toShareholderID, numberOfShares)
    _log(CORP__CALL, "CorpRegistryBound::Handle_MoveCompanyShares()");
    call.Dump(CORP__CALL_DUMP);

    Call_MoveShares args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (IsCorp(args.toShareholderID)) {
        call.client->SendInfoModalMsg("You cannot give shares to a corporation.");
        return nullptr;
    }

    uint32 corpID = 0;
    Client* pClient = sEntityList.FindClientByCharID(args.toShareholderID);
    if (pClient == nullptr) {
        corpID = CharacterDB::GetCorpID(args.toShareholderID);
    } else {
        corpID = pClient->GetCorporationID();
    }

    /** @todo  test for moving shares between players.  can we do that? */
    m_db.MoveShares(m_corpID, corpID, args);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_MovePrivateShares(PyCallArgs &call) {
    // return self.GetCorpRegistry().MovePrivateShares(corporationID, toShareholderID, numberOfShares)
    _log(CORP__CALL, "CorpRegistryBound::Handle_MovePrivateShares()");
    call.Dump(CORP__CALL_DUMP);

    Call_MoveShares args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 corpID = 0;
    Client* pClient = sEntityList.FindClientByCharID(args.toShareholderID);
    if (pClient == nullptr) {
        corpID = CharacterDB::GetCorpID(args.toShareholderID);
    } else {
        corpID = pClient->GetCorporationID();
    }

    // gonna have to do this one different...
    //  will need shares OF WHAT corpID also.
    m_db.MoveShares(call.client->GetCharacterID(), corpID, args);
    return nullptr;
}


PyResult CorpRegistryBound::Handle_GetMemberIDsByQuery(PyCallArgs &call) {
    /*this is performed thru corp window using a multitude of options, to get specific members based on quite variable criteria
     * not as complicated as i had originally thought.
     */

    //return self.GetCorpRegistry().GetMemberIDsByQuery(query, includeImplied, searchTitles)
    call.Dump(CORP__CALL_DUMP);
    Call_GetMemberIDsByQuery_Main args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (args.data->empty()) {
        call.client->SendErrorMsg("You must choose a role to search for.");
        return nullptr;
    }

    /*
     *                query.append([property, operator, value])
     *                query.append([joinOperator, property, operator, value])
     *
     * 04:28:06 [CorpCallDump]   Call Arguments:
     * 04:28:06 [CorpCallDump]      Tuple: 3 elements
     * 04:28:06 [CorpCallDump]       [ 0]   List: 3 elements
     * 04:28:06 [CorpCallDump]       [ 0]   [ 0]   List: 3 elements
     * 04:28:06 [CorpCallDump]       [ 0]   [ 0]   [ 0]     String: 'roles'
     * 04:28:06 [CorpCallDump]       [ 0]   [ 0]   [ 1]    Integer: 7
     * 04:28:06 [CorpCallDump]       [ 0]   [ 0]   [ 2]    Integer: 134217728
     * 04:28:06 [CorpCallDump]       [ 0]   [ 1]   List: 4 elements
     * 04:28:06 [CorpCallDump]       [ 0]   [ 1]   [ 0]    Integer: 2           <-- joinOp  (AND/OR)
     * 04:28:06 [CorpCallDump]       [ 0]   [ 1]   [ 1]     String: 'roles'     <-- queryType   (Corp::QueryType {roles/charID/baseID/joinDate})
     * 04:28:06 [CorpCallDump]       [ 0]   [ 1]   [ 2]    Integer: 8           <-- searchOp (Corp::SearchOp)
     * 04:28:06 [CorpCallDump]       [ 0]   [ 1]   [ 3]    Integer: 4096        <-- value   (depends on searchOp, int, int64)
     * 04:28:06 [CorpCallDump]       [ 0]   [ 2]   List: 4 elements
     * 04:28:06 [CorpCallDump]       [ 0]   [ 2]   [ 0]    Integer: 2
     * 04:28:06 [CorpCallDump]       [ 0]   [ 2]   [ 1]     String: 'roles'
     * 04:28:06 [CorpCallDump]       [ 0]   [ 2]   [ 2]    Integer: 8
     * 04:28:06 [CorpCallDump]       [ 0]   [ 2]   [ 3]    Integer: 1024
     * 04:28:06 [CorpCallDump]       [ 1]    Integer: 1
     * 04:28:06 [CorpCallDump]       [ 2]    Integer: 1
     *
     */
    /*
     *    int8 joinOp = Corp::JoinOp::None;
     *    uint8 queryType = Corp::QueryType::Roles;
     *    uint8 searchOp = Corp::SearchOp::EQUAL;
     *    int64 value = 0;
     *    std::string searchString = "";
     */
    // query holder
    std::ostringstream query;
    query << "SELECT characterID FROM chrCharacters WHERE corporationID = ";
    query << m_corpID << " AND ";

    bool set = false;
    // decode query format
    PyList* list(nullptr);
    for (PyList::const_iterator itr = args.data->begin(); itr != args.data->end(); ++itr) {
        list = (*itr)->AsList();
        if (list == nullptr)
            continue;
        if (list->size() == 3) {
            Call_GetMemberIDsByQuery_List3 args3;
            if (!args3.Decode(&list)) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                return nullptr;
            }

            if (args3.queryType.compare("roles") == 0) {
                query << "corpRole";
            } else {
                query << args3.queryType;
            }

            if (GetSearchValues(args3.searchOp, args3.valueRaw, query)) {
                set = true;
            } else {
                call.client->SendErrorMsg("Search data invalid. Ref: ServerError xxxxx");
                return nullptr;
            }
        } else if (list->size() == 4) {
            // db will not do bitwise for multiple operators.
            //   will have to hit db for first call then query result with remaining conditionals to get result
            if (!set)
                return nullptr;
            Call_GetMemberIDsByQuery_List4 args4;
            if (!args4.Decode(&list)) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                return nullptr;
            }

            switch (args4.joinOp) {
                case Corp::JoinOp::AND: {
                    query << " AND ";
                } break;
                case Corp::JoinOp::OR: {
                    query << " OR ";
                } break;
                case Corp::JoinOp::None:
                default: {
                    // error
                    _log(CORP__ERROR, "CorpRegistryBound::Handle_GetMemberIDsByQuery() sent invalid JoinOp %i", args4.joinOp);
                    return nullptr;
                } break;
            }

            if (args4.queryType.compare("roles") == 0) {
                query << "corpRole";
            } else {
                query << args4.queryType;
            }

            if (GetSearchValues(args4.searchOp, args4.valueRaw, query)) {
                set = true;
            } else {
                call.client->SendErrorMsg("Search data invalid. Ref: ServerError xxxxx");
                return nullptr;
            }
        } else {
            _log(CORP__ERROR, "CorpRegistryBound::Handle_GetMemberIDsByQuery() - Invalid data size: %u.  Expected 3 or 4.", list->size());
            return nullptr;
        }

    }

    // get corp memberlist based on query
    std::vector<uint32> result;
    // make sure we have a valid query before sending to db method
    if (set)
        m_db.GetMembersForQuery(query, result);

    // create/clear list as needed
    if (list == nullptr) {
        list = new PyList();
    } else {
        list->clear();
    }

    // populate results
    for (auto cur : result)
        list->AddItem(new PyInt(cur));

    if (is_log_enabled(CORP__RSP_DUMP))
        list->Dump(CORP__RSP_DUMP, "");

    // return results
    return list;
}

//SELECT `characterID`\
`corpRole`, `rolesAtAll`, `rolesAtHQ`, `rolesAtBase`, `rolesAtOther`, \
`grantableRoles`, `grantableRolesAtHQ`, `grantableRolesAtBase`, `grantableRolesAtOther`,\
`titleMask`, `blockRoles`, `baseID`, `startDateTime` FROM `chrCharacters` WHERE 1
bool CorpRegistryBound::GetSearchValues(int8 op, PyRep* rep, std::ostringstream& query)
{
    using namespace Corp;
    switch (op) {
        case SearchOp::EQUAL: {
            query << " = ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::GREATER: {
            query << " > ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::GREATER_OR_EQUAL: {
            query << " >= ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::LESS: {
            query << " < ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::LESS_OR_EQUAL: {
            query << " <= ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::NOT_EQUAL: {
            query << " != ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::HAS_BIT: {
            query << " &";
            query << PyRep::IntegerValue(rep);
            query << " = ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::NOT_HAS_BIT: {
            query << " ~";
            query << PyRep::IntegerValue(rep);
            query << " = ";
            query << PyRep::IntegerValue(rep);
        } break;
        case SearchOp::STR_CONTAINS:
        case SearchOp::STR_LIKE: {
            query << "%";
            query << PyRep::StringContent(rep);
            query << "% ";
        } break;
        case SearchOp::STR_STARTS_WITH: {
            query << PyRep::StringContent(rep);
            query << "%";
        } break;
        case SearchOp::STR_ENDS_WITH: {
            query << "%";
            query << PyRep::StringContent(rep);
        } break;
        case SearchOp::STR_IS: {
            query << " = ";
            query << PyRep::StringContent(rep);
        } break;
        default: {
            _log(CORP__ERROR, "CorpRegistryBound::GetSearchValues() sent invalid searchOp %i", op);
            return false;
        };
    }
    return true;
}

// no longer used with new query decoding
uint8 CorpRegistryBound::GetQueryType(std::string queryType)
{
    if (queryType.compare("roles") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("rolesAtHQ") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("rolesAtBase") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("rolesAtOther") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("grantableRoles") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("grantableRolesAtHQ") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("grantableRolesAtBase") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("grantableRolesAtOther") == 0) {
        return Corp::QueryType::Roles;
    } else if (queryType.compare("baseID") == 0) {
        return Corp::QueryType::BaseID;
    } else if (queryType.compare("startDateTime") == 0) {
        return Corp::QueryType::StartDateTime;
    } else if (queryType.compare("characterID") == 0) {     //this is actually string.  check searchOp for details
        return Corp::QueryType::CharID;
    } else if (queryType.compare("titleMask") == 0) {
        return Corp::QueryType::TitleMask;
    }

    _log(CORP__ERROR, "CorpRegistryBound::GetQueryType() - Invalid QueryType: %s", queryType.c_str());
    return 0;
}


/**     ***********************************************************************
 * @note   these below are partially coded
 */

PyResult CorpRegistryBound::Handle_PayoutDividend(PyCallArgs &call) {
    //self.GetCorpRegistry().PayoutDividend(payShareholders, payoutAmount)
    /*** shareholders
     * 04:42:43 W CorpRegistryBound::Handle_PayoutDividend(): size= 2
     * 04:42:43 [CorpCallDump]   Call Arguments:
     * 04:42:43 [CorpCallDump]       Tuple: 2 elements
     * 04:42:43 [CorpCallDump]         [ 0] Integer field: 1            <-- int bool?
     * 04:42:43 [CorpCallDump]         [ 1] Real field: 1.000000        <-- amount
     *** members
     * 04:42:50 W CorpRegistryBound::Handle_PayoutDividend(): size= 2
     * 04:42:50 [CorpCallDump]   Call Arguments:
     * 04:42:50 [CorpCallDump]       Tuple: 2 elements
     * 04:42:50 [CorpCallDump]         [ 0] Integer field: 0
     * 04:42:50 [CorpCallDump]         [ 1] Real field: 1.000000
     */
    _log(CORP__CALL, "CorpRegistryBound::Handle_PayoutDividend()");
    call.Dump(CORP__CALL_DUMP);

    Call_PayoutDividend args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    /** @todo finish this... */

    // get list of ids to pay.  this includes corp shareholders if paying to shares
    std::vector<uint32> toIDs;
    if (args.payShareholders) {

    } else {

    }

    // get total amount and divide by # of ids to pay
    float amount = args.payoutAmount / toIDs.size();
    if (amount < 0.01)
        return nullptr;  //make error here?

        // pay each id and record xfer
        std::string reason = "Dividend Payment from ";
    reason += ""; //corp name here
    for (auto cur : toIDs)
        AccountService::TranserFunds(m_corpID, cur, amount, reason.c_str(), Journal::EntryType::CorporationDividendPayment, call.client->GetCharacterID());

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateMember(PyCallArgs &call) {
    //return self.GetCorpRegistry().UpdateMember(charIDToUpdate, title, divisionID, squadronID, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther, baseID, titleMask, blockRoles)
    /** @todo there is more to this call......havent fully figured it out yet.  */
    /*  called when clicking on "member details - roles tab - apply"
     * 21:14:49 [SvcCall] Service CorpRegistryBound::UpdateMember()
     * 21:14:49 [CorpCallDump]   Call Arguments:
     * 21:14:49 [CorpCallDump]      Tuple: 15 elements
     * 21:14:49 [CorpCallDump]       [ 0]    Integer: 91000034
     * 21:14:49 [CorpCallDump]       [ 1]     String: ''
     * 21:14:49 [CorpCallDump]       [ 2]    Integer: 0
     * 21:14:49 [CorpCallDump]       [ 3]    Integer: 0
     * 21:14:49 [CorpCallDump]       [ 4]       Long: 288230376151711744
     * 21:14:49 [CorpCallDump]       [ 5]    Integer: 0
     * 21:14:49 [CorpCallDump]       [ 6]    Integer: 0
     * 21:14:49 [CorpCallDump]       [ 7]    Integer: 0
     * 21:14:49 [CorpCallDump]       [ 8]    Integer: 0
     * 21:14:49 [CorpCallDump]       [ 9]    Integer: 0
     * 21:14:49 [CorpCallDump]       [10]    Integer: 0
     * 21:14:49 [CorpCallDump]       [11]    Integer: 0
     * 21:14:49 [CorpCallDump]       [12]    Integer: 60014137
     * 21:14:49 [CorpCallDump]       [13]       None
     * 21:14:49 [CorpCallDump]       [14]       None
     * 21:14:49 [CorpCallDump]  Named Arguments:
     * 21:14:49 [CorpCallDump]   machoVersion
     * 21:14:49 [CorpCallDump]        Integer: 1
     * 21:14:49 [DecodeError] Decode Call_UpdateMember failed: grantableRoles is not a long Integer: Integer
     * 21:14:49 [SvcError] Handle_UpdateMember(/usr/local/src/eve/EVEmu/src/eve-server/corporation/CorpRegistryBound.cpp:1270): allan: Failed to decode arguments.
     */
    /*  called when clicking on "member details - titles tab - apply"
     * 21:17:46 [SvcCall] Service CorpRegistryBound::UpdateMember()
     * 21:17:46 [CorpCallDump]   Call Arguments:
     * 21:17:46 [CorpCallDump]      Tuple: 15 elements
     * 21:17:46 [CorpCallDump]       [ 0]    Integer: 91000034
     * 21:17:46 [CorpCallDump]       [ 1]    WString: ''
     * 21:17:46 [CorpCallDump]       [ 2]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 3]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 4]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 5]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 6]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 7]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 8]    Integer: 0
     * 21:17:46 [CorpCallDump]       [ 9]    Integer: 0
     * 21:17:46 [CorpCallDump]       [10]    Integer: 0
     * 21:17:46 [CorpCallDump]       [11]    Integer: 0
     * 21:17:46 [CorpCallDump]       [12]    Integer: 60014137
     * 21:17:46 [CorpCallDump]       [13]    Integer: 1
     * 21:17:46 [CorpCallDump]       [14]       None
     * 21:17:46 [CorpCallDump]  Named Arguments:
     * 21:17:46 [CorpCallDump]   machoVersion
     * 21:17:46 [CorpCallDump]        Integer: 1
     * 21:17:46 [DecodeWarning]  Safe is enabled. title was decoded as PyWString
     * 21:17:46 [DecodeError] Decode Call_UpdateMember failed: roles is not a long Integer: Integer
     * 21:17:46 [SvcError] Handle_UpdateMember(/usr/local/src/eve/EVE/src/eve-server/corporation/CorpRegistryBound.cpp:1270): allan: Failed to decode arguments.
     */
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateMember()");
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateMember args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    if (!IsCharacter(args.charID))
        return nullptr;

    int64 oldRole = 0;
    bool grantable = false;  // boolean - do new roles have grantable privs?  they may.

    Client* pClient = sEntityList.FindClientByCharID(args.charID);
    if (pClient == nullptr) {
        oldRole = CharacterDB::GetCorpRole(args.charID);
        CharacterDB::SetCorpRole(args.charID, args.roles);
    } else {
        oldRole = pClient->GetCorpRole();
        CorpData data = pClient->GetChar()->GetCorpData();
        data.rolesAtAll = args.roles;
        data.rolesAtBase = args.rolesAtBase;
        data.rolesAtHQ = args.rolesAtHQ;
        data.rolesAtOther = args.rolesAtOther;
        data.grantableRolesAtBase = args.grantableRolesAtBase;
        data.grantableRolesAtHQ = args.grantableRolesAtHQ;
        data.grantableRolesAtOther = args.grantableRolesAtOther;
        data.grantableRoles = args.grantableRoles;
        // update corp data and refresh session data.
        pClient->GetChar()->UpdateCorpData(data);
    }

    // check roleGroups data for bitmaks for grantable roles
    //      use bitmaks to set 'grantable' bool

    m_db.AddRoleHistory(m_corpID, args.charID, call.client->GetCharacterID(), oldRole, args.roles, grantable);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_InsertApplication(PyCallArgs &call)
{
    Call_InsertApplication args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // should "corp jumpers" be time-limited?   config option?   parameters?
    //  if so, put check here and return msg to player on hit

    // can we send app to non-player corp?
    if (!IsPlayerCorp(args.corpID))
        return nullptr;
    // check member limit
    if (m_db.GetCorpMemberCount(args.corpID) + 1 > m_db.GetCorpMemberLimit(args.corpID)) {
        call.client->SendNotifyMsg("%s cannot accept any new members at this time.", m_db.GetCorpName(args.corpID).c_str());
        return nullptr;
    }

    // check member race
    //  ceo's ethnic relation skill allows +20%/lvl members of other races, based on member limit
    //  not sure how im gonna do this yet....db column in corp table of off-race count maybe?

    uint32 charID = call.client->GetCharacterID();
    /// Insert query into the db
    Corp::ApplicationInfo aInfo = Corp::ApplicationInfo();
        aInfo.valid = true;
        aInfo.charID = charID;
        aInfo.corpID = args.corpID;
        aInfo.appText = args.message;
        aInfo.role = Corp::Role::Member;
        aInfo.grantRole = Corp::Role::Member;  // this is "None"
        aInfo.status = Corp::AppStatus::AppliedByCharacter;
        aInfo.appTime = GetFileTimeNow();
        aInfo.deleted = false;
        aInfo.lastCID = 0;

    if (!m_db.InsertApplication(aInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to insert application request", call.client->GetName());
        return nullptr;
    }

    m_db.AddItemEvent(args.corpID, charID, Corp::EventType::AppliedForMembershipOfCorporation);

    // this is sent for any change, including new
    Corp::ApplicationInfo oldInfo = Corp::ApplicationInfo();
        oldInfo.valid = false;
    OnCorporationApplicationChanged OCAC;
    FillOCApplicationChange(OCAC, oldInfo, aInfo);
    // would this be for another corp??
    //  could you bind/req this call from a diff corp?
    OCAC.corpID = args.corpID;
    OCAC.charID = charID;
    sEntityList.CorpNotify(args.corpID, Notify::Types::CorpAppNew, "OnCorporationApplicationChanged", "*corpid&corprole", OCAC.Encode());
    // this is also sent to applicant
    call.client->SendNotification("OnCorporationApplicationChanged", "*corpid&corprole", OCAC.Encode(), false);

    /// need to find out what happens on the other side, if there's anything at all on the other side

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string subject = "New application from ";
    subject += call.client->GetName();
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(args.corpID));
    m_manager->lsc_service->SendMail(charID, recipients, subject, args.message);

    // should this be sent from mail system?  maybe not...cannot determine type from mail.
    // for now, this notification will need to be created/sent from same method sending mail.
    PyDict* dict = new PyDict();
        dict->SetItemString("applicationText", new PyString(args.message));
        dict->SetItemString("corpID", new PyInt(args.corpID));
        dict->SetItemString("charID", new PyInt(charID));
    OnNotify onn;
        onn.created = GetFileTimeNow();
        onn.data    = dict;
        onn.notifyID = 0;
        onn.senderID = charID;
        onn.typeID  = Notify::Types::CorpAppNew;
    sEntityList.CorpNotify(args.corpID, Notify::Types::CorpAppNew, "OnNotificationReceived", "clientID", onn.Encode());

    /// Reply: ~\x00\x00\x00\x00\x01
    //returns none
    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateApplicationOffer(PyCallArgs &call) {
    //     return self.GetCorpRegistry().UpdateApplicationOffer(characterID, applicationText, status, applicationDateTime = None) NOTE: time not used.
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateApplicationOffer() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateApplicationOffer args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // can we send app to non-player corp?
    if (!IsPlayerCorp(m_corpID))
        return nullptr;

    Corp::ApplicationInfo oldInfo = Corp::ApplicationInfo();
        oldInfo.valid = true;
        oldInfo.appText = args.appText;
        oldInfo.charID = args.charID;
        oldInfo.corpID = m_corpID;  // is this applicants current corpID or applied-to corpID??
    if (!m_db.GetCurrentApplicationInfo(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application for char %u corp %u", call.client->GetName(), args.charID, m_corpID);
        return nullptr;
    }

    Corp::ApplicationInfo newInfo = oldInfo;
        newInfo.valid = true;  // allow saving of this update. we'll null it later.
        // these are populated as we're still saving new status in db.  subject to change as system progresses
        newInfo.status = args.newStatus;
        newInfo.lastCID = call.client->GetCharacterID();
        //newInfo.appTime = GetFileTimeNow();  // not used here.
    if (!m_db.UpdateApplication(newInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), args.charID, m_corpID);
        return nullptr;
    }

    // sets 'new' info to PyNone in OnCorporationApplicationChanged packet.  this removes app from char/corp page
    //newInfo.valid = false;

    OnCorporationApplicationChanged ocac;
        ocac.charID = args.charID;
        ocac.corpID = m_corpID;
    FillOCApplicationChange(ocac, oldInfo, newInfo);

    if (args.newStatus == Corp::AppStatus::AcceptedByCorporation) {
        MemberAttributeUpdate change;
        if (!m_db.CreateMemberAttributeUpdate(oldInfo.corpID, args.charID, change)) {
            codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), args.charID, m_corpID);
            return nullptr;
        }

        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpAppAccept, "OnCorporationApplicationChanged", "*corpid&corprole", ocac.Encode());

        // OnObjectPublicAttributesUpdated event        <<<---  needs to be updated. do search in packet logs
        OnObjectPublicAttributesUpdated opau;
            opau.realRowCount = m_db.GetMemberCount(m_corpID); // for this call, this is corp membership
            opau.bindID = GetBindStr();
            opau.changePKIndexValue = args.charID;    // logs show this as charID, but cant find anything about it in code as to why.
            opau.changes = change.Encode();
        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpNews, "OnObjectPublicAttributesUpdated", "objectID", opau.Encode());

        // OnCorporationMemberChanged event
        OnCorpMemberChange ocmc;
            ocmc.charID = args.charID;
            ocmc.newCorpID = PyRep::IntegerValue(change.corporationIDNew);
            ocmc.oldCorpID = PyRep::IntegerValue(change.corporationIDOld);
            ocmc.newDate = (int64)GetFileTimeNow(); //PyRep::IntegerValue(OCAC.applicationDateTimeNew);
            ocmc.oldDate = oldInfo.appTime; // PyRep::IntegerValue(OCAC.applicationDateTimeOld);

        // both corporations' members will be notified about the change
        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpNews, "OnCorporationMemberChanged", "corpid", ocmc.Encode());
        // old corp MAY be NPC corp...
        if (IsPlayerCorp(ocmc.oldCorpID))
            sEntityList.CorpNotify(ocmc.oldCorpID, Notify::Types::CorpNews, "OnCorporationMemberChanged", "corpid", ocmc.Encode());

        CorpData data = CorpData();
            sItemFactory.db()->GetCorpData(args.charID, data);
            data.corpAccountKey = Account::KeyType::Cash;
            data.corpRole = Corp::Role::Member;
            data.rolesAtAll = Corp::Role::Member;
            data.rolesAtBase = Corp::Role::Member;
            data.rolesAtHQ = Corp::Role::Member;
            data.rolesAtOther = Corp::Role::Member;
            data.grantableRoles = Corp::Role::Member;
            data.grantableRolesAtBase = Corp::Role::Member;
            data.grantableRolesAtHQ = Corp::Role::Member;
            data.grantableRolesAtOther = Corp::Role::Member;
            data.corporationID = m_corpID;
        Client* recruit = sEntityList.FindClientByCharID(ocmc.charID);   // this returns nullptr for offline chars
        if (recruit != nullptr) {
            recruit->GetChar()->JoinCorporation(data);
        } else {
            CharacterDB::AddEmployment(args.charID, m_corpID, ocmc.oldCorpID);
        }

        // add corp events for changing both corps
        m_db.AddItemEvent(m_corpID, args.charID, Corp::EventType::JoinedCorporation);
        if (IsPlayerCorp(ocmc.oldCorpID))
            m_db.AddItemEvent(ocmc.oldCorpID, args.charID, Corp::EventType::LeftCorporation);
    } else if (args.newStatus == Corp::AppStatus::RejectedByCorporation) {
        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpAppReject, "OnCorporationApplicationChanged", "*corpid&corprole", ocac.Encode());
    } else if (args.newStatus == Corp::AppStatus::RenegotiatedByCorporation) {
        // i dont see this option in client code...may not be able to reneg app.
        _log(CORP__MESSAGE, "CorpRegistryBound::Handle_UpdateApplicationOffer() hit AppStatus::RenegotiatedByCorporation by %s ", call.client->GetName() );
    } else {
        codelog(SERVICE__ERROR, "%s: Sent unhandled status %u ", call.client->GetName(), args.newStatus);
        // error...what to do here?
    }

    // update applicant, if online
    Client* pClient = sEntityList.FindClientByCharID(args.charID);
    if (pClient != nullptr)
        pClient->SendNotification("OnCorporationApplicationChanged", "clientID", ocac.Encode(), false);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteApplication(PyCallArgs & call)
{
    //  self.GetCorpRegistry().DeleteApplication(corporationID, characterID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteApplication() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewFalse();
    }

    OnCorporationApplicationChanged ocac;
        ocac.corpID = args.arg1;
        ocac.charID = args.arg2;
    Corp::ApplicationInfo newInfo = Corp::ApplicationInfo();
        newInfo.valid = false;
    Corp::ApplicationInfo oldInfo = Corp::ApplicationInfo();
        oldInfo.valid = true;
        oldInfo.corpID = args.arg1;
        oldInfo.charID = args.arg2;
    if (!m_db.GetCurrentApplicationInfo(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), ocac.charID, ocac.corpID);
        return PyStatic.NewFalse();
    }

    FillOCApplicationChange(ocac, oldInfo, newInfo);
    if (!m_db.DeleteApplication(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to delete application info for char %u corp %u", call.client->GetName(), ocac.charID, ocac.corpID);
        return PyStatic.NewFalse();
    }

    sEntityList.CorpNotify(m_corpID, Notify::Types::CorpAppNew, "OnCorporationApplicationChanged", "*corpid&corprole", ocac.Encode());

    return PyStatic.NewTrue();
}

void CorpRegistryBound::FillOCApplicationChange(OnCorporationApplicationChanged& OCAC, const Corp::ApplicationInfo& Old, const Corp::ApplicationInfo& New)
{
    //  ? (PyRep*)new PyInt(x) : PyStatic.NewNone();
    if (Old.valid) {
        OCAC.applicationIDOld = new PyInt(Old.appID);
        //OCAC.applicationDateTimeOld = new PyLong(Old.appTime);
        OCAC.applicationTextOld = new PyString(Old.appText);
        OCAC.characterIDOld = new PyInt(Old.charID);
        OCAC.corporationIDOld = new PyInt(Old.corpID);
        //OCAC.deletedOld = new PyInt(Old.deleted);
        //OCAC.grantableRolesOld = new PyLong(Old.grantRole);
        if (Old.lastCID) {
            OCAC.lastCorpUpdaterIDOld = new PyInt(Old.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDOld = PyStatic.NewNone();
        }
        //OCAC.rolesOld = new PyLong(Old.role);
        OCAC.statusOld = new PyInt(Old.status);
    } else {
        OCAC.applicationIDOld = PyStatic.NewNone();
        //OCAC.applicationDateTimeOld = PyStatic.NewNone();
        OCAC.applicationTextOld = PyStatic.NewNone();
        OCAC.characterIDOld = PyStatic.NewNone();
        OCAC.corporationIDOld = PyStatic.NewNone();
        //OCAC.deletedOld = PyStatic.NewNone();
        //OCAC.grantableRolesOld = PyStatic.NewNone();
        OCAC.lastCorpUpdaterIDOld = PyStatic.NewNone();
        //OCAC.rolesOld = PyStatic.NewNone();
        OCAC.statusOld = PyStatic.NewNone();
    }

    if (New.valid) {
        OCAC.applicationIDNew = new PyInt(New.appID);
        //OCAC.applicationDateTimeNew = new PyLong(New.appTime);
        OCAC.applicationTextNew = new PyString(New.appText);
        OCAC.characterIDNew = new PyInt(New.charID);
        OCAC.corporationIDNew = new PyInt(New.corpID);
        //OCAC.deletedNew = new PyInt(New.deleted);
        //OCAC.grantableRolesNew = new PyLong(New.grantRole);
        if (New.lastCID) {
            OCAC.lastCorpUpdaterIDNew = new PyInt(New.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDNew = PyStatic.NewNone();
        }
        //OCAC.rolesNew = new PyLong(New.role);
        OCAC.statusNew = new PyInt(New.status);
    } else {
        OCAC.applicationIDNew = PyStatic.NewNone();
        //OCAC.applicationDateTimeNew = PyStatic.NewNone();
        OCAC.applicationTextNew = PyStatic.NewNone();
        OCAC.characterIDNew = PyStatic.NewNone();
        OCAC.corporationIDNew = PyStatic.NewNone();
        //OCAC.deletedNew = PyStatic.NewNone();
        //OCAC.grantableRolesNew = PyStatic.NewNone();
        OCAC.lastCorpUpdaterIDNew = PyStatic.NewNone();
        //OCAC.rolesNew = PyStatic.NewNone();
        OCAC.statusNew = PyStatic.NewNone();
    }
}

PyResult CorpRegistryBound::Handle_GetStations(PyCallArgs &call)
{   // not working
    //  logs show this should be SparseRowset, but handled by bound corp registry object
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetStations() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    /*
     *    [PyTuple 1 items]
     *      [PySubStream 88 bytes]
     *        [PyObjectData Name: util.SparseRowset]
     *          [PyTuple 3 items]
     *            [PyList 2 items]
     *              [PyString "stationID"]
     *              [PyString "typeID"]
     *            [PySubStruct]
     *              [PySubStream 51 bytes]
     *                [PyTuple 3 items]
     *                  [PyString "N=698477:223512"]
     *                  [PyDict 1 kvp]
     *                    [PyString "realRowCount"]
     *                    [PyInt 0]
     *                  [PyIntegerVar 129492959106785392]
     *            [PyInt 0]
     *    [PyDict 1 kvp]
     *      [PyString "OID+"]
     *      [PyDict 1 kvp]
     *        [PyString "N=698477:223512"]
     *        [PyIntegerVar 129492959106785392]
     */

    /** @todo  need more data/info on this return to make it work right..
    PyBoundObject* bObj = new SparseCorpOfficeListBound(m_manager, m_db);
    if (bObj == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to create bound object for:", call.client->GetName()); //errors here
        return nullptr;
    }

    // this sends header info and # offices in station
    // Data will be fetched from the SparseRowset
    CorpOfficeSparseRowset rsp;

    rsp.officeNumber = StationDB::GetOfficeCount(m_corpID);

    PyDict *dict = new PyDict();
    dict->SetItemString("realRowCount", new PyInt(rsp.officeNumber));

    rsp.boundObject = m_manager->BindObject(call.client, bObj, dict);

    PyObject* obj = rsp.Encode();
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
    */
    PyObject* obj = m_db.GetStations(m_corpID);

    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

PyResult CorpRegistryBound::Handle_GetOffices(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetOffices() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    PyBoundObject* bObj = new SparseBound(m_manager, m_db, m_corpID);
    if (bObj == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to create bound object for:", call.client->GetName()); //errors here
        return nullptr;
    }

    // this sends header info and # offices rented by corp
    // Data will be fetched from the subsequent call to SparseRowset (using self.sr.offices in client)
    CorpOfficeSparseRowset rsp;
    rsp.officeCount = StationDB::GetOfficeCount(m_corpID);

    PyDict* dict = new PyDict();
        dict->SetItemString("realRowCount", new PyInt(rsp.officeCount));

    PyDict* oid = new PyDict();
    rsp.boundObject = m_manager->BindObject(call.client, bObj, dict, oid);

    PyObject* obj = rsp.Encode();
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return PyResult(obj, oid);
    /*
    [PyTuple 1 items]
      [PySubStream 114 bytes]
        [PyObjectData Name: util.SparseRowset]
          [PyTuple 3 items]
            [PyList 4 items]
              [PyString "stationID"]
              [PyString "typeID"]
              [PyString "officeID"]
              [PyString "officeFolderID"]
            [PySubStruct]
              [PySubStream 50 bytes]
                [PyTuple 3 items]
                  [PyString "N=789442:2172"]
                  [PyDict 1 kvp]
                    [PyString "realRowCount"]
                    [PyInt 8]
                  [PyIntegerVar 129753802088805346]
            [PyInt 8]
    [PyDict 1 kvp]
      [PyString "OID+"]
      [PyDict 1 kvp]
        [PyString "N=789442:2172"]
        [PyIntegerVar 129753802088805346]
    */
    /*  newish  (dv packet)
    [PyTuple 1 items]
      [PySubStream 139 bytes]
        [PyObjectData Name: eve.common.script.sys.rowset.SparseRowset]
          [PyTuple 3 items]
            [PyList 5 items]
              [PyString "locationID"]
              [PyString "solarSystemID"]
              [PyString "typeID"]
              [PyString "officeID"]
              [PyString "officeFolderID"]
            [PySubStruct]
              [PySubStream 50 bytes]
                [PyTuple 3 items]
                  [PyString "N=1222739:1759"]
                  [PyDict 1 kvp]
                    [PyString "realRowCount"]
                    [PyInt 0]
                  [PyIntegerVar 131101118262531055]
            [PyInt 0]
    [PyDict 1 kvp]
      [PyString "OID+"]
      [PyDict 1 kvp]
        [PyString "N=1222739:1759"]
        [PyIntegerVar 131101118262531055]
    */
}

PyResult CorpRegistryBound::Handle_InsertVoteCase(PyCallArgs &call) {
    //  return self.GetCorpRegistry().InsertVoteCase(voteCaseText, description, corporationID, voteType, voteCaseOptions, startDateTime, endDateTime)
    // see notes in m_db.AddVoteCase() for more info on data

    _log(CORP__CALL, "CorpRegistryBound::Handle_InsertVoteCase()");
    call.Dump(CORP__CALL_DUMP);

    /*
     * 10:10:55 W CorpRegistryBound::Handle_InsertVoteCase(): size= 7
     * 10:10:55 [CorpCallDump]   Call Arguments:
     * 10:10:55 [CorpCallDump]       Tuple: 7 elements
     * 10:10:55 [CorpCallDump]       [ 0]    WString: 'test'         << voteCaseText
     * 10:10:55 [CorpCallDump]       [ 1]    WString: 'test vote'    << description
     * 10:10:55 [CorpCallDump]       [ 2]    Integer: 98000000       << corporationID
     * 10:10:55 [CorpCallDump]       [ 3]    Integer: 4              << voteType (Corp::VoteType::xx)
     * 10:10:55 [CorpCallDump]       [ 4] Object:
     * 10:10:55 [CorpCallDump]       [ 4] Type:     String: 'util.Rowset'
     * 10:10:55 [CorpCallDump]       [ 4] Args: Dictionary: 3 entries   << voteCaseOptions
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Key:     String: 'lines'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: List: 2 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] List: 4 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 0]    WString: 'yay'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 1]    Integer: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 2]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 3]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] List: 4 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 0]    WString: 'nay'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 1]    Integer: 1
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 2]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 3]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 1] Key:     String: 'RowClass'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 1] Value: Token: 'util.Row'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Key:     String: 'header'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: List: 4 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 0]     String: 'choice'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 1]     String: 'itemID'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 2]     String: 'typeID'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 3]     String: 'locationID'
     * 10:10:55 [CorpCallDump]       [ 5]       Long: 131575686552352973    << startDateTime
     * 10:10:55 [CorpCallDump]       [ 6]       Long: 131576550552352973    << endDateTime
     * 10:10:55 [CorpCallDump]   Call Named Arguments:
     * 10:10:55 [CorpCallDump]     Argument 'machoVersion':
     * 10:10:55 [CorpCallDump]            Integer: 1
     */

    /*
     * 13:54:27 [CorpCall] CorpRegistryBound::Handle_InsertVoteCase()
     * 13:54:27 [DB Error] DoQuery_locked(/backups/local/src/eve/EVE/src/eve-core/database/dbcore.cpp:345):
     * DBCore Query - #1064 in
     * 'INSERT INTO crpVoteOptions (voteCaseID, optionID, optionText, parameter, parameter1, parameter2)
     * VALUES
     * (1,-104,'Lockdown the 125mm Gatling AutoCannon I Blueprint',140024216,819,60014140),
     * (1,0,'Don't lockdown the 125mm Gatling AutoCannon I Blueprint',0,0,0)':
     *
     * You have an error in your SQL syntax; check the manual that corresponds to your MariaDB server version
     * for the right syntax to use near 't lockdown the 125mm Gatling AutoCannon I Blueprint',0,0,0)' at line 1
     */


    if (!call.tuple->GetItem(4)->IsObject()) {
        codelog(CORP__ERROR, "Tuple Item is wrong type: %s.  Expected PyObject.", call.tuple->GetItem(0)->TypeString());
        return nullptr;
    }
    Call_InsertVoteCase args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!args.voteCaseOptions->arguments()->IsDict()) {
        codelog(CORP__ERROR, "voteCaseOptions Argument is wrong type: %s.  Expected PyDict.", args.voteCaseOptions->arguments()->TypeString());
        return nullptr;
    }

    m_db.AddVoteCase(m_corpID, call.client->GetCharacterID(), args);

    //  OnCorporationVoteCaseChanged

    // send notification to online corp memebers
    //def OnNotificationReceived(self, notificationID, typeID, senderID, created, data = {}):
    /*
     * ==================== Sent from Server 161 bytes
     *
     * [PyObjectData Name: macho.Notification]
     *  [PyTuple 6 items]
     *    [PyInt 12]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 1]
     *        [PyInt 698477]
     *        [PyNone]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 4]
     *        [PyString "OnNotificationReceived"]
     *        [PyList 0 items]
     *        [PyString "clientID"]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 85 bytes]
     *          [PyTuple 2 items]
     *            [PyInt 0]
     *            [PyTuple 2 items]
     *              [PyInt 1]
     *              [PyTuple 5 items]
     *                [PyInt 342405655]     << notifyID (sequential?  random?)
     *                [PyInt 25]           << Notify::Types::CorpVote
     *                [PyInt 1661059544]    << senderID (CharID)
     *                [PyIntegerVar 129492998400000000] << created timestamp
     *                [PyDict 2 kvp]        << data (varies for each notify type)
     *                  [PyString "body"]
     *                  [PyString "please vote for me"]
     *                  [PyString "subject"]
     *                  [PyString "proposed vote"]
     *    [PyDict 1 kvp]
     *      [PyString "sn"]
     *      [PyIntegerVar 9]
     */
    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetVoteCasesByCorporation(PyCallArgs &call)
{
    //  xxx  = self.GetCorpRegistry().GetVoteCasesByCorporation(corpid)
    //return self.GetCorpRegistry().GetVoteCasesByCorporation(corpid, status, maxLen)

    /*** from closed
     * 04:24:15 W CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 3
     * 04:24:15 [CorpCallDump]   Call Arguments:
     * 04:24:15 [CorpCallDump]       Tuple: 3 elements
     * 04:24:15 [CorpCallDump]         [ 0] Integer field: 98000000
     * 04:24:15 [CorpCallDump]         [ 1] Integer field: 1
     * 04:24:15 [CorpCallDump]         [ 2] Integer field: 20
     *** from open
     * 04:24:20 W CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 3
     * 04:24:20 [CorpCallDump]   Call Arguments:
     * 04:24:20 [CorpCallDump]       Tuple: 3 elements
     * 04:24:20 [CorpCallDump]         [ 0] Integer field: 98000000
     * 04:24:20 [CorpCallDump]         [ 1] Integer field: 2
     * 04:24:20 [CorpCallDump]         [ 2] Integer field: 0
     *** from sanctionable actions
     * 12:48:19 [CorpCall] CorpRegistryBound::Handle_GetVoteCasesByCorporation()
     * 12:48:19 [CorpCallDump]   Call Arguments:
     * 12:48:19 [CorpCallDump]      Tuple: 1 elements
     * 12:48:19 [CorpCallDump]       [ 0]    Integer: 98000000
     *
     */
    /*
    [PyTuple 1 items]
      [PySubStream 193 bytes]
        [PyObjectData Name: util.IndexRowset]
          [PyDict 4 kvp]
            [PyString "items"]
            [PyDict 1 kvp]
              [PyInt 2167602]
              [PyList 8 items]
                [PyInt 2167602]
                [PyString "proposed vote"]
                [PyString "please vote for me"]
                [PyInt 98038978]
                [PyInt 1661059544]
                [PyInt 4]
                [PyIntegerVar 129492998400000000]
                [PyIntegerVar 129493862400000000]
            [PyString "RowClass"]
            [PyToken util.Row]
            [PyString "idName"]
            [PyString "voteCaseID"]
            [PyString "header"]
            [PyList 8 items]
              [PyString "voteCaseID"]
              [PyString "voteCaseText"]
              [PyString "description"]
              [PyString "corporationID"]
              [PyString "characterID"]
              [PyString "voteType"]
              [PyString "startDateTime"]
              [PyString "endDateTime"]
              */
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetVoteCasesByCorporation()");
    call.Dump(CORP__CALL_DUMP);

    if (call.tuple->size() == 3) {
        Call_GetVoteCases args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return nullptr;
        }
        return m_db.GetVoteItems(args.corpid, args.status, args.maxLen);
    } else if (call.tuple->size() == 1) {
        return m_db.GetVoteItems(m_corpID);
    } else {
        ; // error?
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetVoteCaseOptions(PyCallArgs &call) {
    // options = self.GetCorpRegistry().GetVoteCaseOptions(corpID, voteCaseID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetVoteCaseOptions()");
    call.Dump(CORP__CALL_DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyRep* rsp = m_db.GetVoteOptions(args.arg2);
    if (is_log_enabled(CORP__RSP_DUMP))
        rsp->Dump(CORP__RSP_DUMP, "");

    return rsp;
}

PyResult CorpRegistryBound::Handle_GetVotes(PyCallArgs &call) {
    // charVotes = sm.GetService('corp').GetVotes(self.corpID, vote.voteCaseID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetVotes() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    // this will need a bit more thought....
    /*
                charVotes = sm.GetService('corp').GetVotes(self.corpID, vote.voteCaseID)
                hasVoted = 0
                votedFor = -1
                if charVotes and charVotes.has_key(session.charid):
                    hasVoted = 1
                    votedFor = charVotes[session.charid].optionID
            */

    PyRep* rsp = m_db.GetVotes(args.arg2);
    if (is_log_enabled(CORP__RSP_DUMP))
        rsp->Dump(CORP__RSP_DUMP, "");

    return rsp;
}

/*  GetVoteCaseOptions
    [PyTuple 1 items]
      [PySubStream 243 bytes]
        [PyObjectData Name: util.IndexRowset]
          [PyDict 4 kvp]
            [PyString "items"]
            [PyDict 3 kvp]
              [PyInt 0]
              [PyList 9 items]
                [PyInt 2167602]
                [PyInt 0]
                [PyString "vote choice 1"]
                [PyIntegerVar 0]
                [PyFloat 0]
                [PyInt 0]
                [PyInt 0]
                [PyNone]
                [PyNone]
              [PyInt 1]
              [PyList 9 items]
                [PyInt 2167602]
                [PyInt 1]
                [PyString "vote choice 2"]
                [PyIntegerVar 1]
                [PyFloat 0]
                [PyInt 0]
                [PyInt 0]
                [PyNone]
                [PyNone]
              [PyInt 2]
              [PyList 9 items]
                [PyInt 2167602]
                [PyInt 2]
                [PyString "vote choice 3"]
                [PyIntegerVar 2]
                [PyFloat 0]
                [PyInt 0]
                [PyInt 0]
                [PyNone]
                [PyNone]
            [PyString "RowClass"]
            [PyToken util.Row]
            [PyString "idName"]
            [PyString "optionID"]
            [PyString "header"]
            [PyList 9 items]
              [PyString "voteCaseID"]
              [PyString "optionID"]
              [PyString "optionText"]
              [PyString "parameter"]
              [PyString "votesFor"]
              [PyString "votesMade"]
              [PyString "votesProxied"]
              [PyString "parameter1"]
              [PyString "parameter2"]

              */


PyResult CorpRegistryBound::Handle_GetSanctionedActionsByCorporation(PyCallArgs &call) {
    //  rows = sm.GetService('corp').GetSanctionedActionsByCorporation(eve.session.corpid, state)
    /*
05:09:00 [CorpCallDump]   Call Arguments:
05:09:00 [CorpCallDump]      Tuple: 2 elements
05:09:00 [CorpCallDump]       [ 0]    Integer: 98000000
05:09:00 [CorpCallDump]       [ 1]    Integer: 1
*/
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetSanctionedActionsByCorporation()");
    call.Dump(CORP__CALL_DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /*  this still needs work...used to show voting items in corp that 'mean something'
     *
     * if row.voteType in [const.voteItemUnlock,
     *  const.voteCEO,
     *  const.voteWar,
     *  const.voteShares,
     *  const.voteKickMember,
     *  const.voteItemLockdown] and row.parameter == 0:
     *     continue
     * voteType = self.voteTypes[row.voteType]
     * title = voteCases[row.voteCaseID].voteCaseText
     * description = voteCases[row.voteCaseID].description
     * expires = util.FmtDate(row.expires)
     * actedUpon = [localization.GetByLabel('UI/Generic/No'), localization.GetByLabel('UI/Generic/Yes')][row.actedUpon]
     * inEffect = [localization.GetByLabel('UI/Generic/No'), localization.GetByLabel('UI/Generic/Yes')][row.inEffect]
     * rescended = localization.GetByLabel('UI/Generic/No')
     * if row.timeRescended:
     *     rescended = localization.GetByLabel('UI/Corporations/CorpSanctionableActions/AnswerWithTimestamp', answer=rescended, timestamp=util.FmtDate(row.timeRescended))
     * if row.timeActedUpon:
     *     actedUpon = localization.GetByLabel('UI/Corporations/CorpSanctionableActions/AnswerWithTimestamp', answer=actedUpon, timestamp=util.FmtDate(row.timeActedUpon))
     */


    return m_db.GetSanctionedItems(args.arg1, args.arg2);
}

PyResult CorpRegistryBound::Handle_CanVote(PyCallArgs &call) {
    // canVote = sm.GetService('corp').CanVote(self.corpID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CanVote() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return PyStatic.NewFalse();
}


PyResult CorpRegistryBound::Handle_InsertVote(PyCallArgs &call) {
    // return self.GetCorpRegistry().InsertVote(corporationID, voteCaseID, voteValue)
    _log(CORP__CALL, "CorpRegistryBound::Handle_InsertVote() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_InsertVote args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // can you change your vote?  will we have to check for votes already cast and update them?
    m_db.CastVote(args.corpID, call.client->GetCharacterID(), args.voteCaseID, args.voteValue);

    // returns none
    return PyStatic.NewNone();
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult CorpRegistryBound::Handle_GetLockedItemLocations( PyCallArgs& call )
{    /*
    03:15:28 W CorpRegistryBound::Handle_GetLockedItemLocations(): size= 0
    03:15:28 [CorpCallDump]   Call Arguments:
    03:15:28 [CorpCallDump]       Tuple: Empty
    */
    //            locationIDs = self.GetCorpRegistry().GetLockedItemLocations()

    // called from corp.assets.lockdown
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetLockedItemLocations()");
    call.Dump(CORP__CALL_DUMP);

    //this returns an empty list for me on live.
    //  ...because there are no locked items for your corp. :/
    return new PyList();
}

PyResult CorpRegistryBound::Handle_AddCorporateContact(PyCallArgs &call) {
    //self.GetCorpRegistry().AddCorporateContact(contactID, relationshipID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_AddCorporateContact()");
    call.Dump(CORP__CALL_DUMP);

    Call_CorporateContactData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.AddContact(m_corpID, args);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditCorporateContact(PyCallArgs &call) {
    //self.GetCorpRegistry().EditCorporateContact(contactID, relationshipID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_EditCorporateContact()");
    call.Dump(CORP__CALL_DUMP);

    Call_CorporateContactData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.UpdateContact(args.relationshipID, args.contactID, m_corpID);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditContactsRelationshipID(PyCallArgs &call) {
    //self.GetCorpRegistry().EditContactsRelationshipID(contactIDs, relationshipID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_EditContactsRelationshipID()");
    call.Dump(CORP__CALL_DUMP);

    Call_EditCorporateContacts args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (PyList::const_iterator itr = args.contactIDs->begin(); itr != args.contactIDs->end(); ++itr) {
        m_db.UpdateContact(args.relationshipID, PyRep::IntegerValueU32(*itr), m_corpID);
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_RemoveCorporateContacts(PyCallArgs &call) {
    // self.GetCorpRegistry().RemoveCorporateContacts(contactIDs)
    _log(CORP__CALL, "CorpRegistryBound::Handle_RemoveCorporateContacts()");
    call.Dump(CORP__CALL_DUMP);

    Call_RemoveCorporateContacts args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (PyList::const_iterator itr = args.contactIDs->begin(); itr != args.contactIDs->end(); ++itr) {
        m_db.RemoveContact(PyRep::IntegerValueU32(*itr), m_corpID);
    }

    return nullptr;
}

// this is a member role/title update by memberIDs called from corp->members->find in role->task mgmt
PyResult CorpRegistryBound::Handle_ExecuteActions(PyCallArgs &call) {
    //      verb, property, value = action
    //      remoteActions.append(action)
    //  return self.GetCorpRegistry().ExecuteActions(targetIDs, remoteActions)
    _log(CORP__CALL, "CorpRegistryBound::Handle_ExecuteActions() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_ExecuteActions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(CORP__TRACE);

    /*
     *   args.memberIDs = list of charIDs to update
     *   args.verb      = one of add, remove, set, give
     *   args.queryType = all role* types or title
     *   args.value     = roleMask or titleID
     */

    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateLabel(PyCallArgs &call) {
    // return self.GetCorpRegistry().CreateLabel(name, color)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CreateLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteLabel(PyCallArgs &call) {
    // self.GetCorpRegistry().DeleteLabel(labelID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditLabel(PyCallArgs &call) {
    // self.GetCorpRegistry().EditLabel(labelID, name, color)
    _log(CORP__CALL, "CorpRegistryBound::Handle_EditLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_AssignLabels(PyCallArgs &call) {
    // self.GetCorpRegistry().AssignLabels(contactIDs, labelMask)
    _log(CORP__CALL, "CorpRegistryBound::Handle_AssignLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_RemoveLabels(PyCallArgs &call) {
    // self.GetCorpRegistry().RemoveLabels(contactIDs, labelMask)
    _log(CORP__CALL, "CorpRegistryBound::Handle_RemoveLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateAlliance(PyCallArgs &call) {
    //self.GetCorpRegistry().CreateAlliance(allianceName, shortName, description, url)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CreateAlliance() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    AllianceDB a_db;
    Client* pClient(call.client);

    Call_CreateAlliance args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(CORP__TRACE);

    // verify they're not using bad words in their alliance name
    for (const auto cur : badWords)
        if (EvE::icontains(args.allianceName, cur))
            throw CustomError ("Alliance Name contains banned words");

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(args.allianceName, cur))
            throw CustomError ("Alliance Name contains invalid characters");

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(args.shortName, cur))
            throw CustomError ("Alliance short name contains invalid characters");

    // verify short name is available
    if (a_db.IsShortNameTaken(args.shortName))
        throw CustomError ("Alliance short name is taken.");

    double ally_cost = sConfig.rates.allyCost;
    if (pClient->GetBalance(Account::CreditType::ISK) < ally_cost) {
        _log(SERVICE__ERROR, "%s: Cannot afford alliance startup costs!", pClient->GetName());

        // send error to client here
        throw CustomError ("You must have %f ISK to create an alliance.", ally_cost);
        return nullptr;
    }

    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC: Creating new alliance: ";
    reason += args.allianceName;
    reason += " (";
    reason += args.shortName;
    reason += ")";
    AccountService::TranserFunds(
                                pClient->GetCharacterID(),
                                m_db.GetStationOwner(pClient->GetStationID()),  // station owner files paperwork, this is fee for that
                                ally_cost,
                                reason.c_str(),
                                Journal::EntryType::AllianceRegistrationFee,
                                pClient->GetStationID(),
                                Account::KeyType::Cash);

    //creating an alliance will affect eveStaticOwners, so we gotta invalidate the cache...
    //  call to db.AddCorporation() will update eveStaticOwners with new corp
    PyString* cache_name = new PyString( "config.StaticOwners" );
    m_manager->cache_service->InvalidateCache( cache_name );
    PySafeDecRef( cache_name );

    // Register new alliance (also gather current corpID and new allyID at same time)
    uint32 allyID(0);
    uint32 corpID(0);
    if (!a_db.CreateAlliance(args, pClient, allyID, corpID)) {
        codelog(SERVICE__ERROR, "New alliance creation failed.");
        return nullptr;
    }

    // create alliance channel
    m_manager->lsc_service->CreateSystemChannel(allyID);

    // join corporation to alliance
    if (!a_db.UpdateCorpAlliance(allyID, corpID)) {
        codelog(SERVICE__ERROR, "Alliance join failed.");
        return nullptr;
    }

    // Add alliance membership record to corporation
    if (!a_db.AddEmployment(allyID, corpID)) {
        codelog(SERVICE__ERROR, "Add corp employment failed.");
        return nullptr;
    }

    // Update alliance ID on client session
    pClient->UpdateSessionInt("allianceid", allyID);

    //Return alliance we just created
    return a_db.GetAlliance(allyID);
}

PyResult CorpRegistryBound::Handle_ApplyToJoinAlliance(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_ApplyToJoinAlliance()");
    call.Dump(CORP__CALL_DUMP);

    AllianceDB a_db;

    Call_ApplyToJoinAlliance args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(CORP__TRACE);

    Alliance::ApplicationInfo app;
        app.appText = args.applicationText;
        app.allyID = args.allianceID;
        app.corpID = m_corpID;
        app.state = EveAlliance::AppStatus::AppNew;
        app.valid = true;

    if (!a_db.InsertApplication(app)) {
        codelog(SERVICE__ERROR, "New alliance application failed.");
        return nullptr;
    }

    Alliance::ApplicationInfo oldInfo = Alliance::ApplicationInfo();
        oldInfo.valid = false;

    OnAllianceApplicationChanged oaac;
        oaac.corpID = m_corpID;
        oaac.allianceID = app.allyID;

    AllianceBound::FillOAApplicationChange(oaac, oldInfo, app);

    //Send to everyone who needs to see it in the applying corp and in the alliance executor corp
    uint32 executorID = AllianceDB::GetExecutorID(app.allyID);

    std::vector<Client *> list;
    sEntityList.GetCorpClients(list, oaac.corpID);
    for (auto cur : list)
    {
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to client %u", cur->GetClientID());
        }
    }

    list.clear();
    sEntityList.GetCorpClients(list, executorID);
    for (auto cur : list)
    {
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to client %u", cur->GetClientID());
        }
    }

    //Get sending corp's CEO ID:
    uint32 charID = m_db.GetCorporationCEO(m_corpID);

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string subject = "New application from ";
    subject += call.client->GetName();
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(AllianceDB::GetExecutorID(app.allyID)));
    m_manager->lsc_service->SendMail(charID, recipients, subject, args.applicationText);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetAllianceApplications(PyCallArgs &call) {
    //application = sm.GetService('corp').GetAllianceApplications()[allianceID]
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetAllianceApplications()");
    call.Dump(CORP__CALL_DUMP);

    AllianceDB a_db;
    return a_db.GetMyApplications(m_corpID);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteAllianceApplication(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteAllianceApplication()");
    call.Dump(CORP__CALL_DUMP);

    //This is not implemented by client, no context menu option

    AllianceDB a_db;

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(CORP__TRACE);

    //Old application info
    Alliance::ApplicationInfo oldInfo = Alliance::ApplicationInfo();

    if (!a_db.GetCurrentApplicationInfo(args.arg, call.client->GetCorporationID(), oldInfo))
    {
        _log(SERVICE__ERROR, "%s: Failed to query application for corp %u alliance %u", call.client->GetName(), call.client->GetCorporationID(), args.arg);
        return nullptr;
    }

    //New application info (set deleted flag to true here)
    Alliance::ApplicationInfo newInfo = oldInfo;
    newInfo.valid = true;
    newInfo.deleted = true;

    OnAllianceApplicationChanged oaac;
    oaac.allianceID = newInfo.allyID;
    oaac.corpID = newInfo.corpID;
    AllianceBound::FillOAApplicationChange(oaac, oldInfo, newInfo);

    if (!a_db.DeleteApplication(newInfo)) {
        codelog(SERVICE__ERROR, "Deletion failed.");
        return nullptr;
    }

    //Send to everyone who needs to see it in the applying corp and in the alliance executor corp
    uint32 executorID = AllianceDB::GetExecutorID(oaac.allianceID);

    std::vector<Client *> list;
    sEntityList.GetCorpClients(list, oaac.corpID);
    for (auto cur : list)
    {
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to client %u", cur->GetClientID());
        }
    }

    list.clear();
    sEntityList.GetCorpClients(list, executorID);
    for (auto cur : list)
    {
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to client %u", cur->GetClientID());
        }
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetRentalDetailsPlayer(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetRentalDetailsPlayer()
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetRentalDetailsPlayer()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetRentalDetailsCorp(PyCallArgs &call) {
    // return self.GetCorpRegistry().GetRentalDetailsCorp()
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetRentalDetailsCorp()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateCorporationAbilities(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateCorporationAbilities()");
    call.Dump(CORP__CALL_DUMP);

    //this will need to update corp memberlimit, allowed races, and then update all members with new data

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateStationManagementSettings(PyCallArgs &call) {
    //  self.corpStationMgr.UpdateStationManagementSettings(self.modifiedServiceAccessRulesByServiceID, self.modifiedServiceCostModifiers, self.modifiedRentableItems, self.station.stationName, self.station.description, self.station.dockingCostPerVolume, self.station.officeRentalCost, self.station.reprocessingStationsTake, self.station.reprocessingHangarFlag, self.station.exitTime, self.station.standingOwnerID)

    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateStationManagementSettings()");
    call.Dump(CORP__CALL_DUMP);

    // not real sure what this does yet....outpost shit maybe?

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetNumberOfPotentialCEOs(PyCallArgs &call) {
    //  return self.GetCorpRegistry().GetNumberOfPotentialCEOs()
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetNumberOfPotentialCEOs()");
    call.Dump(CORP__CALL_DUMP);


    return nullptr;
}

PyResult CorpRegistryBound::Handle_CanLeaveCurrentCorporation(PyCallArgs &call) {
    //  canLeave, error, errorDetails = corpSvc.CanLeaveCurrentCorporation()
    // error:  CrpCantQuitNotInStasis  and canLeave=false for member that has roles

    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, PyStatic.NewTrue());  //canLeave - set this to timer or w/e to deter corp jumpers
        tuple->SetItem(1, PyStatic.NewNone());
        tuple->SetItem(2, PyStatic.NewNone());

    return tuple;
}



