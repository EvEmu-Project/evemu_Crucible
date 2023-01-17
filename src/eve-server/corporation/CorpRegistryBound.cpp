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
#include "corporation/CorpRegistryService.h"
#include "corporation/CorpRegistryBound.h"
#include "corporation/OfficeSparseBound.h"
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

CorpRegistryBound::CorpRegistryBound(EVEServiceManager& mgr, CorpRegistryService& parent, CorporationDB& db, uint32 corpID) :
    EVEBoundObject(mgr, parent),
    m_db(db)
{
    this->Add("GetEveOwners", &CorpRegistryBound::GetEveOwners);
    this->Add("GetCorporation", &CorpRegistryBound::GetCorporation);
    this->Add("GetCorporations", &CorpRegistryBound::GetCorporations);
    this->Add("GetInfoWindowDataForChar", &CorpRegistryBound::GetInfoWindowDataForChar);
    this->Add("GetLockedItemLocations", &CorpRegistryBound::GetLockedItemLocations);
    this->Add("AddCorporation", &CorpRegistryBound::AddCorporation);
    this->Add("GetSuggestedTickerNames", &CorpRegistryBound::GetSuggestedTickerNames);
    this->Add("GetOffices", &CorpRegistryBound::GetOffices);
    this->Add("GetStations", &CorpRegistryBound::GetStations);

    this->Add("CreateRecruitmentAd", &CorpRegistryBound::CreateRecruitmentAd);
    this->Add("UpdateRecruitmentAd", &CorpRegistryBound::UpdateRecruitmentAd);
    this->Add("DeleteRecruitmentAd", &CorpRegistryBound::DeleteRecruitmentAd);
    this->Add("GetRecruiters", &CorpRegistryBound::GetRecruiters);
    this->Add("GetRecruitmentAdsForCorporation", &CorpRegistryBound::GetRecruitmentAdsForCorporation);
    this->Add("GetMyApplications", &CorpRegistryBound::GetMyApplications);
    this->Add("InsertApplication", &CorpRegistryBound::InsertApplication);
    this->Add("GetApplications", &CorpRegistryBound::GetApplications);
    this->Add("UpdateApplicationOffer", &CorpRegistryBound::UpdateApplicationOffer);
    this->Add("DeleteApplication", &CorpRegistryBound::DeleteApplication);
    this->Add("UpdateDivisionNames", &CorpRegistryBound::UpdateDivisionNames);
    this->Add("UpdateCorporation", &CorpRegistryBound::UpdateCorporation);
    this->Add("UpdateLogo", &CorpRegistryBound::UpdateLogo);
    this->Add("SetAccountKey", &CorpRegistryBound::SetAccountKey);
    this->Add("GetMember", &CorpRegistryBound::GetMember);
    this->Add("GetMembers", &CorpRegistryBound::GetMembers);
    this->Add("UpdateMember", &CorpRegistryBound::UpdateMember);

    this->Add("MoveCompanyShares", &CorpRegistryBound::MoveCompanyShares);
    this->Add("MovePrivateShares", &CorpRegistryBound::MovePrivateShares);
    this->Add("GetSharesByShareholder", &CorpRegistryBound::GetSharesByShareholder);
    this->Add("GetShareholders", &CorpRegistryBound::GetShareholders);
    this->Add("PayoutDividend", &CorpRegistryBound::PayoutDividend);

    this->Add("CanViewVotes", &CorpRegistryBound::CanViewVotes);
    this->Add("InsertVoteCase", &CorpRegistryBound::InsertVoteCase);
    this->Add("GetVotes", &CorpRegistryBound::GetVotes);
    this->Add("CanVote", &CorpRegistryBound::CanVote);
    this->Add("InsertVote", &CorpRegistryBound::InsertVote);
    this->Add("GetVoteCasesByCorporation", &CorpRegistryBound::GetVoteCasesByCorporation);
    this->Add("GetVoteCaseOptions", &CorpRegistryBound::GetVoteCaseOptions);
    this->Add("GetSanctionedActionsByCorporation", &CorpRegistryBound::GetSanctionedActionsByCorporation);

    this->Add("AddBulletin", &CorpRegistryBound::AddBulletin);
    this->Add("GetBulletins", &CorpRegistryBound::GetBulletins);
    this->Add("DeleteBulletin", &CorpRegistryBound::DeleteBulletin);

    this->Add("CreateLabel", &CorpRegistryBound::CreateLabel);
    this->Add("GetLabels", &CorpRegistryBound::GetLabels);
    this->Add("DeleteLabel", &CorpRegistryBound::DeleteLabel);
    this->Add("EditLabel", &CorpRegistryBound::EditLabel);
    this->Add("AssignLabels", &CorpRegistryBound::AssignLabels);
    this->Add("RemoveLabels", &CorpRegistryBound::RemoveLabels);

    this->Add("GetRecentKillsAndLosses", &CorpRegistryBound::GetRecentKillsAndLosses);
    this->Add("GetRoleGroups", &CorpRegistryBound::GetRoleGroups);
    this->Add("GetRoles", &CorpRegistryBound::GetRoles);
    this->Add("GetLocationalRoles", &CorpRegistryBound::GetLocationalRoles);

    this->Add("GetTitles", &CorpRegistryBound::GetTitles);
    this->Add("UpdateTitle", &CorpRegistryBound::UpdateTitle);
    this->Add("UpdateTitles", &CorpRegistryBound::UpdateTitles);
    this->Add("DeleteTitle", &CorpRegistryBound::DeleteTitle);
    this->Add("ExecuteActions", &CorpRegistryBound::ExecuteActions);

    this->Add("GetCorporateContacts", &CorpRegistryBound::GetCorporateContacts);
    this->Add("AddCorporateContact", &CorpRegistryBound::AddCorporateContact);
    this->Add("EditContactsRelationshipID", &CorpRegistryBound::EditContactsRelationshipID);
    this->Add("RemoveCorporateContacts", &CorpRegistryBound::RemoveCorporateContacts);
    this->Add("EditCorporateContact", &CorpRegistryBound::EditCorporateContact);

    this->Add("CreateAlliance", &CorpRegistryBound::CreateAlliance);
    this->Add("ApplyToJoinAlliance", &CorpRegistryBound::ApplyToJoinAlliance);
    this->Add("DeleteAllianceApplication", &CorpRegistryBound::DeleteAllianceApplication);
    this->Add("GetAllianceApplications", &CorpRegistryBound::GetAllianceApplications);
    this->Add("GetSuggestedAllianceShortNames", &CorpRegistryBound::GetSuggestedAllianceShortNames);

    this->Add("GetMembersPaged", &CorpRegistryBound::GetMembersPaged);
    this->Add("GetMembersByIds", &CorpRegistryBound::GetMembersByIds);
    this->Add("GetMemberIDsWithMoreThanAvgShares", &CorpRegistryBound::GetMemberIDsWithMoreThanAvgShares);
    this->Add("GetMemberIDsByQuery", &CorpRegistryBound::GetMemberIDsByQuery);
    this->Add("GetMemberTrackingInfo", &CorpRegistryBound::GetMemberTrackingInfo);
    this->Add("GetMemberTrackingInfoSimple", &CorpRegistryBound::GetMemberTrackingInfoSimple);
    this->Add("GetRentalDetailsPlayer", &CorpRegistryBound::GetRentalDetailsPlayer);
    this->Add("GetRentalDetailsCorp", &CorpRegistryBound::GetRentalDetailsCorp);

    this->Add("UpdateCorporationAbilities", &CorpRegistryBound::UpdateCorporationAbilities);
    this->Add("UpdateStationManagementSettings", &CorpRegistryBound::UpdateStationManagementSettings);
    this->Add("GetNumberOfPotentialCEOs", &CorpRegistryBound::GetNumberOfPotentialCEOs);

    this->Add("CanLeaveCurrentCorporation", &CorpRegistryBound::CanLeaveCurrentCorporation);

    m_corpID = corpID;

    this->m_cache = this->GetServiceManager().Lookup <ObjCacheService>("objectCaching");
    this->m_lsc = this->GetServiceManager().Lookup <LSCService>("LSC");
}

PyResult CorpRegistryBound::GetLocationalRoles(PyCallArgs &call)
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

PyResult CorpRegistryBound::GetEveOwners(PyCallArgs &call) {
    /* this is a method-chaining call.
     * it comes with the bind request for a particular corp.
     *
     * the client wants a member list for given corp
     */
    return m_db.GetEveOwners(m_corpID);
}

PyResult CorpRegistryBound::GetInfoWindowDataForChar(PyCallArgs& call, std::optional <PyInt*> characterID)
{
    // if no characterID is specified, just return information for current character
    if (characterID.has_value() == false)
        return CharacterDB::GetInfoWindowDataForChar (call.client->GetCharacterID ());

    return CharacterDB::GetInfoWindowDataForChar (characterID.value()->value());
}

PyResult CorpRegistryBound::GetCorporation(PyCallArgs &call) {
    // called by member of this corp
    return m_db.GetCorporation(m_corpID);
}

PyResult CorpRegistryBound::GetRoles(PyCallArgs &call)
{   // working
    return m_db.GetCorpRoles();
}

PyResult CorpRegistryBound::GetRoleGroups(PyCallArgs &call)
{   // working
    return m_db.GetCorpRoleGroups();
}

PyResult CorpRegistryBound::GetTitles(PyCallArgs &call)
{   // working
    return m_db.GetTitles(m_corpID);
}

PyResult CorpRegistryBound::GetBulletins(PyCallArgs &call)
{   // working
    return m_db.GetBulletins(m_corpID);
}

PyResult CorpRegistryBound::GetCorporateContacts(PyCallArgs &call)
{   // working
    return m_db.GetContacts(m_corpID);
}

PyResult CorpRegistryBound::GetApplications(PyCallArgs &call)
{   // working
    return m_db.GetApplications(m_corpID);
}

PyResult CorpRegistryBound::GetRecruitmentAdsForCorporation(PyCallArgs &call)
{   // recruitments = self.GetCorpRegistry().GetRecruitmentAdsForCorporation()
    return m_db.GetAdRegistryData();
}

PyResult CorpRegistryBound::GetMyApplications(PyCallArgs &call)
{   // working
    return m_db.GetMyApplications(call.client->GetCharacterID());
}

PyResult CorpRegistryBound::GetMemberTrackingInfo(PyCallArgs &call)
{   // working
    return m_db.GetMemberTrackingInfo(m_corpID);
}

PyResult CorpRegistryBound::GetMemberTrackingInfoSimple(PyCallArgs &call)
{   // working
    return m_db.GetMemberTrackingInfoSimple(m_corpID);
}

PyResult CorpRegistryBound::GetShareholders(PyCallArgs &call)
{   // working
    return m_db.GetShares(m_corpID);
}

PyResult CorpRegistryBound::GetLabels(PyCallArgs &call)
{   // working
    return m_db.GetLabels(call.client->GetCorporationID());
}

PyResult CorpRegistryBound::CanViewVotes(PyCallArgs &call)
{   // working
    return m_db.PyHasShares(call.client->GetCharacterID(), m_corpID);
}

PyResult CorpRegistryBound::GetMemberIDsWithMoreThanAvgShares(PyCallArgs &call)
{   // working
    return m_db.GetSharesForCorp(m_corpID);
}

PyResult CorpRegistryBound::GetRecruiters(PyCallArgs &call, PyInt* corpID, PyInt* adID)
{   // working
    return m_db.GetRecruiters(corpID->value(), adID->value());
}

PyResult CorpRegistryBound::DeleteTitle(PyCallArgs &call, PyInt* titleID)
{   // working
    m_db.DeleteTitle(call.client->GetCorporationID(), titleID->value());
    return nullptr;
}

PyResult CorpRegistryBound::DeleteRecruitmentAd(PyCallArgs &call, PyInt* adID)
{   // working
    m_db.DeleteAdvert(adID->value());
    return nullptr;
}

PyResult CorpRegistryBound::GetSharesByShareholder(PyCallArgs &call, PyBool* corpShares)
{   // working
    return m_db.GetMyShares(corpShares->value() ? call.client->GetCorporationID() : call.client->GetCharacterID());
}

PyResult CorpRegistryBound::GetCorporations(PyCallArgs &call, PyInt* corporationID) {
    // working
    return m_db.GetCorporations(corporationID->value());
}

PyResult CorpRegistryBound::GetRecentKillsAndLosses(PyCallArgs &call, PyInt* number, PyInt* offset)
{   // working
    return m_db.GetKillsAndLosses(m_corpID, number->value(), offset->value());
}

PyResult CorpRegistryBound::GetMember(PyCallArgs &call, PyInt* characterID)
{   // not working
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetMember()");
    PyRep* rep = m_db.GetMember(characterID->value());

    if (is_log_enabled(CORP__RSP_DUMP))
        rep->Dump(CORP__RSP_DUMP, "");

    return rep;
}

PyResult CorpRegistryBound::SetAccountKey(PyCallArgs &call, PyInt* accountKey)
{   // working
    call.client->GetChar()->SetAccountKey(accountKey->value());
    return this->GetOID();
}

PyResult CorpRegistryBound::GetSuggestedTickerNames(PyCallArgs &call, PyWString* name)
{
    PyList* result = new PyList();
    SuggestedTickerName sTN;
    sTN.tName.clear();
    uint32 cnLen = name->content().length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0; i < cnLen; ++i)
        if (name->content()[i] >= 'A' && name->content()[i] <= 'Z')
            sTN.tName += name->content()[i];

    result->AddItem( sTN.Encode() );
    return result;
}

PyResult CorpRegistryBound::GetSuggestedAllianceShortNames(PyCallArgs &call, PyWString* name)
{
    PyList* result = new PyList();
    SuggestedShortName sSN;
    sSN.sName.clear();
    uint32 cnLen = name->content().length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0; i < cnLen; ++i)
        if (name->content()[i] >= 'A' && name->content()[i] <= 'Z')
            sSN.sName += name->content()[i];

    result->AddItem( sSN.Encode() );
    return result;
}

PyResult CorpRegistryBound::GetMembers(PyCallArgs &call)
{   // working
    // this just wants a member count and time (and headers)
    uint16 rowCount = m_db.GetMemberCount(m_corpID);

    // TODO: properly implement this call, an example is provided on GetOffices of what's needed

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


    PyList* headers = new PyList;

    headers->AddItemString ("characterID");
    headers->AddItemString ("corporationID");
    headers->AddItemString ("divisionID");
    headers->AddItemString ("squadronID");
    headers->AddItemString ("title");
    headers->AddItemString ("roles");
    headers->AddItemString ("grantableRoles");
    headers->AddItemString ("startDateTime");
    headers->AddItemString ("baseID");
    headers->AddItemString ("rolesAtHQ");
    headers->AddItemString ("grantableRolesAtHQ");
    headers->AddItemString ("rolesAtBase");
    headers->AddItemString ("grantableRolesAtBase");
    headers->AddItemString ("rolesAtOther");
    headers->AddItemString ("grantableRolesAtOther");
    headers->AddItemString ("titleMask");
    headers->AddItemString ("accountKey");
    headers->AddItemString ("rowDate");
    headers->AddItemString ("blockRoles");

    // TODO: rewrite this to properly use different services, right now any calls to the MembersSparseRowset will be directed here
    PyDict *dict = new PyDict();
        dict->SetItemString("realRowCount", new PyInt(rowCount));   // this is current member count
    PyTuple* boundObject = new PyTuple(3);
        boundObject->SetItem(0, new PyString (this->GetIDString()));    // node info here
        boundObject->SetItem(1, dict);
        boundObject->SetItem(2, new PyLong(GetFileTimeNow()));

    GetMembersSparseRowset ret;
        ret.boundObject = boundObject;
        ret.realRowCount = rowCount;
    return ret.Encode();
}

PyResult CorpRegistryBound::UpdateDivisionNames(PyCallArgs &call,
    PyRep* div1, PyRep* div2, PyRep* div3, PyRep* div4, PyRep* div5, PyRep* div6, PyRep* div8,
    PyRep* wal1, PyRep* wal2, PyRep* wal3, PyRep* wal4, PyRep* wal5, PyRep* wal6, PyRep* wal7)
{   // working
    // TODO: stop using this and make use of the parameters, changing this is way too out of the scope of the service manager changes
    Call_UpdateDivisionNames args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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
    return this->GetOID();
}

PyResult CorpRegistryBound::GetMembersPaged(PyCallArgs &call, PyInt* page) {
    //return self.GetCorpRegistry().GetMembersPaged(page)
    DBQueryResult res;
    m_db.GetMembersPaged(m_corpID, page->value(), res);
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

PyResult CorpRegistryBound::GetMembersByIds(PyCallArgs &call, PyList* memberIDs) {
    //return self.GetCorpRegistry().GetMembersByIds(memberIDs)
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetMembersByIds()");
    call.Dump(CORP__CALL_DUMP);

    PyList* list = new PyList();
    for (PyList::const_iterator itr = memberIDs->begin(); itr != memberIDs->end(); ++itr)
        list->AddItem(new PyObject("util.KeyVal", m_db.GetMember(PyRep::IntegerValueU32(*itr))));

    if (is_log_enabled(CORP__RSP_DUMP))
        list->Dump(CORP__RSP_DUMP, "");

    return list;
}

PyResult CorpRegistryBound::AddCorporation(PyCallArgs &call,
    PyRep* name, PyRep* ticker, PyRep* description, PyRep* url, PyFloat* taxRate,
    PyRep* shape1, PyRep* shape2, PyRep* shape3,
    PyRep* color1, PyRep* color2, PyRep* color3,
    PyRep* typeface, PyInt* applicationEnabled) {

    Client* pClient(call.client);
    // TODO: stop using this and make use of the parameters, changing this is way too out of the scope of the service manager changes
    Call_AddCorporation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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
    this->m_cache->InvalidateCache( cache_name );
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
    this->m_lsc->CreateSystemChannel(corpID);

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

PyResult CorpRegistryBound::UpdateTitle(PyCallArgs &call,
    PyRep* titleID, PyRep* titleName,
    PyRep* roles, PyRep* grantableRoles,
    PyRep* rolesAtHQ, PyRep* grantableRolesAtHQ, 
    PyRep* rolesAtBase, PyRep* grantableRolesAtBase, 
    PyRep* rolesAtOther, PyRep* grantableRolesAtOther) {
    // self.GetCorpRegistry().UpdateTitle(titleID, titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther)
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateTitle()");
    //call.Dump(CORP__CALL_DUMP);

    // TODO: stop using this and make use of the parameters, changing this is way too out of the scope of the service manager changes
    Call_UpdateTitleData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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

PyResult CorpRegistryBound::UpdateTitles(PyCallArgs &call, PyObject* titles) {
    //    self.GetCorpRegistry().UpdateTitles(titles)
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateTitles()");
    //call.Dump(CORP__CALL_DUMP);
    if (!call.tuple->GetItem(0)->IsObject()) {
        codelog(CORP__ERROR, "Tuple Item is wrong type: %s.  Expected PyObject.", call.tuple->GetItem(0)->TypeString());
        return nullptr;
    }

    if (!titles->arguments()->IsDict()) {
        codelog(CORP__ERROR, "Object Argument is wrong type: %s.  Expected PyDict.", titles->arguments()->TypeString());
        return nullptr;
    }

    PyDict* dict = titles->arguments()->AsDict();
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

PyResult CorpRegistryBound::UpdateCorporation(PyCallArgs &call, PyRep* description, PyRep* url, PyFloat* tax) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateCorporation() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);
    Notify_IntRaw notif;
    notif.key = m_corpID;
    notif.data = new PyDict();

    if (!m_db.UpdateCorporation(notif.key, PyRep::StringContent(description), PyRep::StringContent (url), tax->value(), notif.data)) {
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

PyResult CorpRegistryBound::UpdateLogo(PyCallArgs &call,
    PyRep* shape1, PyRep* shape2, PyRep* shape3,
    PyRep* color1, PyRep* color2, PyRep* color3,
    PyRep* typeface)
{
    // TODO: stop using this and make use of the parameters, changing this is way too out of the scope of the service manager changes
    Call_UpdateLogo args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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

PyResult CorpRegistryBound::AddBulletin(PyCallArgs &call, PyRep* title, PyRep* body) {
    // self.GetCorpRegistry().AddBulletin(title, body)
    // self.GetCorpRegistry().AddBulletin(title, body, bulletinID=bulletinID, editDateTime=editDateTime)    <-- this is to update bulletin
    _log(CORP__CALL, "CorpRegistryBound::Handle_AddBulletin()");
    call.Dump(CORP__CALL_DUMP);

    std::string titleStr = PyRep::StringContent(title);
    std::string bodyStr = PyRep::StringContent(body);

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
            AllianceDB::EditBulletin(bulletinID, call.client->GetCharacterID(), editDateTime, titleStr, bodyStr);
        } else {
            m_db.EditBulletin(bulletinID, call.client->GetCharacterID(), editDateTime, titleStr, bodyStr);
        }

    } else {
        m_db.AddBulletin(m_corpID, m_corpID, call.client->GetCharacterID(), titleStr, bodyStr);
    }

    return nullptr;
}

PyResult CorpRegistryBound::DeleteBulletin(PyCallArgs &call, PyInt* bulletinID) {
    //self.GetCorpRegistry().DeleteBulletin(id)
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteBulletin() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    if (bulletinID->value() >= 100000) {
        AllianceDB::DeleteBulletin(bulletinID->value());
    } else {
        m_db.DeleteBulletin(bulletinID->value());
    }
    return nullptr;
}

PyResult CorpRegistryBound::CreateRecruitmentAd(PyCallArgs &call, PyInt* days, PyInt* typeMask, std::optional <PyInt*> allianceID, PyWString* description, PyInt* channelID, PyList* recruiterIDs, PyWString* title) {
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

    uint32 amount = 1250000;
    switch (days->value()) {
        case 7:    amount = 2250000;  break;
        case 14:   amount = 4000000;  break;
        case 28:   amount = 7500000;  break;
    }

    AccountService::TranserFunds(m_corpID, call.client->GetCorpHQ(), amount, "Initial Advert Time for Corp Recruit Advert", Journal::EntryType::CorporationAdvertisementFee, call.client->GetCharacterID());

    int32 adID = m_db.CreateAdvert(call.client, m_corpID, typeMask->value(), days->value(), m_db.GetCorpMemberCount(m_corpID), description->content(), channelID->value(), title->content());

    std::vector<int32> recruiters;
    recruiters.clear();
    for (PyList::const_iterator itr = recruiterIDs->begin(); itr != recruiterIDs->end(); ++itr)
        recruiters.push_back(PyRep::IntegerValue(*itr));
    //recruiters.push_back((*itr)->AsInt()->value());

    // if no recruiters defined, default to creating character
    if (recruiters.empty())
        recruiters.push_back(call.client->GetCharacterID());

    m_db.AddRecruiters(adID, (int32)m_corpID, recruiters);

    return nullptr;
}

PyResult CorpRegistryBound::UpdateRecruitmentAd(PyCallArgs &call, PyInt* adID, PyInt* typeMask, PyWString* description, PyInt* channelID, PyList* recruiterIDs, PyWString* title, PyInt* addedDays) {
    // return self.GetCorpRegistry().UpdateRecruitmentAd(adID, typeMask, description, channelID, recruiters, title, addedDays)
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateRecruitmentAd() size=%li", call.tuple->size());
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

    uint8 days = 1;
    // check that corp can afford added time, if requested
    if (addedDays->value()) {
        // 3d = .75misk, 7d = 1.75misk, 14d = 3.5misk
        uint32 amount = 750000;
        switch (addedDays->value()) {
            case 7:    amount = 1750000;  break;
            case 14:   amount = 3500000;  break;
        }

        AccountService::TranserFunds(m_corpID, call.client->GetCorpHQ(), amount, "Added Advert Time to Corp Recruit Advert", Journal::EntryType::CorporationAdvertisementFee);

        // do some funky shit to determine days left for advert then add additional time if requested
        int64 time = m_db.GetAdvertTime(adID->value(), m_corpID);
        if (time > 0) {
            time -= GetFileTimeNow();
            days = time / EvE::Time::Day;
            days += addedDays->value();
        }
    }

    m_db.UpdateAdvert(adID->value(), m_corpID, typeMask->value(), days, m_db.GetCorpMemberCount(m_corpID), description->content(), channelID->value(), title->content());

    std::vector<int32> recruiters;
    recruiters.clear();
    for (PyList::const_iterator itr = recruiterIDs->begin(); itr != recruiterIDs->end(); ++itr)
        recruiters.push_back(PyRep::IntegerValue(*itr));

    // if no recruiters defined, default to creating character
    if (recruiters.empty())
        recruiters.push_back(call.client->GetCharacterID());

    m_db.AddRecruiters(adID->value(), (int32)m_corpID, recruiters);

    return m_db.GetAdvert(adID->value());
}

PyResult CorpRegistryBound::MoveCompanyShares(PyCallArgs &call, PyInt* corporationID, PyInt* toShareholderID, PyInt* numberOfShares) {
    // return self.GetCorpRegistry().MoveCompanyShares(corporationID, toShareholderID, numberOfShares)
    _log(CORP__CALL, "CorpRegistryBound::Handle_MoveCompanyShares()");
    call.Dump(CORP__CALL_DUMP);

    if (IsCorp(toShareholderID->value())) {
        call.client->SendInfoModalMsg("You cannot give shares to a corporation.");
        return nullptr;
    }

    uint32 corpID = 0;
    Client* pClient = sEntityList.FindClientByCharID(toShareholderID->value());
    if (pClient == nullptr) {
        corpID = CharacterDB::GetCorpID(toShareholderID->value());
    } else {
        corpID = pClient->GetCorporationID();
    }

    /** @todo  test for moving shares between players.  can we do that? */
    m_db.MoveShares(m_corpID, corpID, corporationID->value(), toShareholderID->value(), numberOfShares->value());
    return nullptr;
}

PyResult CorpRegistryBound::MovePrivateShares(PyCallArgs &call, PyInt* corporationID, PyInt* toShareholderID, PyInt* numberOfShares) {
    // return self.GetCorpRegistry().MovePrivateShares(corporationID, toShareholderID, numberOfShares)
    _log(CORP__CALL, "CorpRegistryBound::Handle_MovePrivateShares()");
    call.Dump(CORP__CALL_DUMP);

    uint32 corpID = 0;
    Client* pClient = sEntityList.FindClientByCharID(toShareholderID->value());
    if (pClient == nullptr) {
        corpID = CharacterDB::GetCorpID(toShareholderID->value());
    } else {
        corpID = pClient->GetCorporationID();
    }

    // gonna have to do this one different...
    //  will need shares OF WHAT corpID also.
    m_db.MoveShares(call.client->GetCharacterID(), corpID, corporationID->value(), toShareholderID->value(), numberOfShares->value());
    return nullptr;
}


PyResult CorpRegistryBound::GetMemberIDsByQuery(PyCallArgs &call, PyList* queryList, std::optional <PyInt*> includeImplied, PyInt* searchTitles) {
    /*this is performed thru corp window using a multitude of options, to get specific members based on quite variable criteria
     * not as complicated as i had originally thought.
     */

    //return self.GetCorpRegistry().GetMemberIDsByQuery(query, includeImplied, searchTitles)
    call.Dump(CORP__CALL_DUMP);

    if (queryList->empty()) {
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
    for (PyList::const_iterator itr = queryList->begin(); itr != queryList->end(); ++itr) {
        list = (*itr)->AsList();
        if (list == nullptr)
            continue;
        if (list->size() == 3) {
            Call_GetMemberIDsByQuery_List3 args3;
            if (!args3.Decode(&list)) {
                codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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
                codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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

PyResult CorpRegistryBound::PayoutDividend(PyCallArgs &call, PyBool* payShareholders, PyFloat* payoutAmount) {
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

    /** @todo finish this... */

    // get list of ids to pay.  this includes corp shareholders if paying to shares
    std::vector<uint32> toIDs;
    if (payShareholders->value()) {
        // not used yet
    } else {
        // not used yet
    }

    // get total amount and divide by # of ids to pay
    float amount = payoutAmount->value() / toIDs.size();
    if (amount < 0.01)
        return nullptr;  //make error here?

        // pay each id and record xfer
        std::string reason = "Dividend Payment from ";
    reason += ""; //corp name here
    for (auto cur : toIDs)
        AccountService::TranserFunds(m_corpID, cur, amount, reason.c_str(), Journal::EntryType::CorporationDividendPayment, call.client->GetCharacterID());

    return nullptr;
}

PyResult CorpRegistryBound::UpdateMember(PyCallArgs &call,
    PyInt* characterID, PyRep* title, PyRep* divisionID, PyRep* squadronID,
    PyLong* roles, PyLong* grantableRoles, PyLong* rolesAtHQ, PyLong* grantableRolesAtHQ,
    PyLong* rolesAtBase, PyLong* grantableRolesAtBase, PyLong* rolesAtOther, PyLong* grantableRolesAtOther,
    PyRep* baseID, PyRep* titleMask, PyRep* blockRoles) {
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

    // TODO: stop using this and make use of the parameters, changing this is way too out of the scope of the service manager changes
    Call_UpdateMember args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
        return nullptr;
    }
    if (!IsCharacterID(args.charID))
        return nullptr;

    int64 oldRole(0);
    bool grantable(false);  // boolean - do new roles have grantable privs?  they may.

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

PyResult CorpRegistryBound::InsertApplication(PyCallArgs &call, PyInt* corporationID, PyRep* message)
{
    // should "corp jumpers" be time-limited?   config option?   parameters?
    //  if so, put check here and return msg to player on hit

    // can we send app to non-player corp?
    if (!IsPlayerCorp(corporationID->value()))
        return nullptr;
    // check member limit
    if (m_db.GetCorpMemberCount(corporationID->value()) + 1 > m_db.GetCorpMemberLimit(corporationID->value())) {
        call.client->SendNotifyMsg("%s cannot accept any new members at this time.", m_db.GetCorpName(corporationID->value()).c_str());
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
        aInfo.corpID = corporationID->value();
        aInfo.appText = PyRep::StringContent (message);
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

    m_db.AddItemEvent(corporationID->value(), charID, Corp::EventType::AppliedForMembershipOfCorporation);

    // this is sent for any change, including new
    Corp::ApplicationInfo oldInfo = Corp::ApplicationInfo();
        oldInfo.valid = false;
    OnCorporationApplicationChanged OCAC;
    FillOCApplicationChange(OCAC, oldInfo, aInfo);
    // would this be for another corp??
    //  could you bind/req this call from a diff corp?
    OCAC.corpID = corporationID->value();
    OCAC.charID = charID;
    sEntityList.CorpNotify(corporationID->value(), Notify::Types::CorpAppNew, "OnCorporationApplicationChanged", "*corpid&corprole", OCAC.Encode());
    // this is also sent to applicant
    call.client->SendNotification("OnCorporationApplicationChanged", "*corpid&corprole", OCAC.Encode(), false);

    /// need to find out what happens on the other side, if there's anything at all on the other side

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string subject = "New application from ";
    subject += call.client->GetName();
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(corporationID->value()));
    this->m_lsc->SendMail(charID, recipients, subject, aInfo.appText);

    // should this be sent from mail system?  maybe not...cannot determine type from mail.
    // for now, this notification will need to be created/sent from same method sending mail.
    PyDict* dict = new PyDict();
        dict->SetItemString("applicationText", new PyString(aInfo.appText));
        dict->SetItemString("corpID", new PyInt(corporationID->value()));
        dict->SetItemString("charID", new PyInt(charID));
    OnNotify onn;
        onn.created = GetFileTimeNow();
        onn.data    = dict;
        onn.notifyID = 0;
        onn.senderID = charID;
        onn.typeID  = Notify::Types::CorpAppNew;
    sEntityList.CorpNotify(corporationID->value(), Notify::Types::CorpAppNew, "OnNotificationReceived", "clientID", onn.Encode());

    /// Reply: ~\x00\x00\x00\x00\x01
    //returns none
    return nullptr;
}

PyResult CorpRegistryBound::UpdateApplicationOffer(PyCallArgs &call, PyInt* characterID, PyRep* applicationText, PyInt* status, PyLong* applicationDateTime) {
    //     return self.GetCorpRegistry().UpdateApplicationOffer(characterID, applicationText, status, applicationDateTime = None) NOTE: time not used.
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateApplicationOffer() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    // can we send app to non-player corp?
    if (!IsPlayerCorp(m_corpID))
        return nullptr;

    Corp::ApplicationInfo oldInfo = Corp::ApplicationInfo();
        oldInfo.valid = true;
        oldInfo.appText = PyRep::StringContent(applicationText);
        oldInfo.charID = characterID->value();
        oldInfo.corpID = m_corpID;  // is this applicants current corpID or applied-to corpID??
    if (!m_db.GetCurrentApplicationInfo(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application for char %u corp %u", call.client->GetName(), characterID->value(), m_corpID);
        return nullptr;
    }

    Corp::ApplicationInfo newInfo = oldInfo;
        newInfo.valid = true;  // allow saving of this update. we'll null it later.
        // these are populated as we're still saving new status in db.  subject to change as system progresses
        newInfo.status = status->value();
        newInfo.lastCID = call.client->GetCharacterID();
        //newInfo.appTime = GetFileTimeNow();  // not used here.
    if (!m_db.UpdateApplication(newInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), characterID->value(), m_corpID);
        return nullptr;
    }

    // sets 'new' info to PyNone in OnCorporationApplicationChanged packet.  this removes app from char/corp page
    //newInfo.valid = false;

    OnCorporationApplicationChanged ocac;
        ocac.charID = characterID->value();
        ocac.corpID = m_corpID;
    FillOCApplicationChange(ocac, oldInfo, newInfo);

    if (status->value() == Corp::AppStatus::AcceptedByCorporation) {
        MemberAttributeUpdate change;
        if (!m_db.CreateMemberAttributeUpdate(oldInfo.corpID, characterID->value(), change)) {
            codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), characterID->value(), m_corpID);
            return nullptr;
        }

        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpAppAccept, "OnCorporationApplicationChanged", "*corpid&corprole", ocac.Encode());

        // this should come from the SparseRowset instance, not from here...
        // OnObjectPublicAttributesUpdated event        <<<---  needs to be updated. do search in packet logs
        OnObjectPublicAttributesUpdated opau;
            opau.realRowCount = m_db.GetMemberCount(m_corpID); // for this call, this is corp membership
            opau.bindID = this->GetIDString();
            opau.changePKIndexValue = characterID->value();    // logs show this as charID, but cant find anything about it in code as to why.
            opau.changes = change.Encode();
        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpNews, "OnObjectPublicAttributesUpdated", "objectID", opau.Encode());

        // OnCorporationMemberChanged event
        OnCorpMemberChange ocmc;
            ocmc.charID = characterID->value();
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
        CharacterDB::GetCharCorpData(characterID->value(), data);
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
            CharacterDB::AddEmployment(characterID->value(), m_corpID, ocmc.oldCorpID);
        }

        // add corp events for changing both corps
        m_db.AddItemEvent(m_corpID, characterID->value(), Corp::EventType::JoinedCorporation);
        if (IsPlayerCorp(ocmc.oldCorpID))
            m_db.AddItemEvent(ocmc.oldCorpID, characterID->value(), Corp::EventType::LeftCorporation);
    } else if (status->value() == Corp::AppStatus::RejectedByCorporation) {
        sEntityList.CorpNotify(m_corpID, Notify::Types::CorpAppReject, "OnCorporationApplicationChanged", "*corpid&corprole", ocac.Encode());
    } else if (status->value() == Corp::AppStatus::RenegotiatedByCorporation) {
        // i dont see this option in client code...may not be able to reneg app.
        _log(CORP__MESSAGE, "CorpRegistryBound::Handle_UpdateApplicationOffer() hit AppStatus::RenegotiatedByCorporation by %s ", call.client->GetName() );
    } else {
        codelog(SERVICE__ERROR, "%s: Sent unhandled status %u ", call.client->GetName(), status->value());
        // error...what to do here?
    }

    // update applicant, if online
    Client* pClient = sEntityList.FindClientByCharID(characterID->value());
    if (pClient != nullptr)
        pClient->SendNotification("OnCorporationApplicationChanged", "clientID", ocac.Encode(), false);

    return nullptr;
}

PyResult CorpRegistryBound::DeleteApplication(PyCallArgs & call, PyInt* corporationID, PyInt* characterID)
{
    //  self.GetCorpRegistry().DeleteApplication(corporationID, characterID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteApplication() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    OnCorporationApplicationChanged ocac;
        ocac.corpID = corporationID->value();
        ocac.charID = characterID->value();
    Corp::ApplicationInfo newInfo = Corp::ApplicationInfo();
        newInfo.valid = false;
    Corp::ApplicationInfo oldInfo = Corp::ApplicationInfo();
        oldInfo.valid = true;
        oldInfo.corpID = corporationID->value();
        oldInfo.charID = characterID->value();
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

PyResult CorpRegistryBound::GetStations(PyCallArgs &call)
{   // not working
    //  logs show this should be SparseRowset, but handled by bound corp registry object
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetStations() size=%li", call.tuple->size());
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

void CorpRegistryBound::BoundReleased (OfficeSparseBound* bound) {
    this->m_offices = nullptr;
}

PyResult CorpRegistryBound::GetOffices(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetOffices() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    PyList* headers = new PyList;

    headers->AddItemString ("itemID");
    headers->AddItemString ("stationID");
    headers->AddItemString ("stationTypeID");
    headers->AddItemString ("officeFolderID");

    if (this->m_offices == nullptr) {
        this->m_offices = new OfficeSparseBound (this->GetServiceManager (), *this, m_db, m_corpID, headers);
    }

    if (this->m_offices == nullptr) {
        _log(SERVICE__ERROR, "%s: Unable to create bound object for:", call.client->GetName()); //errors here
        return nullptr;
    }

    this->m_offices->NewReference (call.client);

    return this->m_offices->GetHeader ();
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

PyResult CorpRegistryBound::InsertVoteCase(PyCallArgs &call, PyRep* voteCaseText, PyRep* description, PyInt* corporationID, PyInt* voteType, std::optional <PyObject*> voteCaseOptions, PyLong* startDateTime, PyLong* endDateTime) {
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
        codelog(SERVICE__ERROR, "CorpRegistryBound: Failed to decode arguments.");
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

PyResult CorpRegistryBound::GetVoteCasesByCorporation(PyCallArgs &call, PyInt* corporationID, std::optional<PyInt*> status, std::optional <PyInt*> maxLen)
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

    if (status.has_value() && maxLen.has_value()) {
        return m_db.GetVoteItems(corporationID->value(), status.value()->value(), maxLen.value()->value());
    } else if (call.tuple->size() == 1) {
        return m_db.GetVoteItems(m_corpID);
    } else {
        ; // error?
    }

    return nullptr;
}

PyResult CorpRegistryBound::GetVoteCaseOptions(PyCallArgs &call, PyInt* corporationID, PyInt* voteCaseID) {
    // options = self.GetCorpRegistry().GetVoteCaseOptions(corpID, voteCaseID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetVoteCaseOptions()");
    call.Dump(CORP__CALL_DUMP);

    // TODO: also support specifying corporationID as parameters so shareholders can vote
    PyRep* rsp = m_db.GetVoteOptions(voteCaseID->value());
    if (is_log_enabled(CORP__RSP_DUMP))
        rsp->Dump(CORP__RSP_DUMP, "");

    return rsp;
}

PyResult CorpRegistryBound::GetVotes(PyCallArgs &call, PyInt* corporationId, PyInt* voteCaseID) {
    // charVotes = sm.GetService('corp').GetVotes(self.corpID, vote.voteCaseID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetVotes() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    // this will need a bit more thought....
    /*
                charVotes = sm.GetService('corp').GetVotes(self.corpID, vote.voteCaseID)
                hasVoted = 0
                votedFor = -1
                if charVotes and charVotes.has_key(session.charid):
                    hasVoted = 1
                    votedFor = charVotes[session.charid].optionID
            */

    // TODO: also support specifying corporationID as parameters so shareholders can vote
    PyRep* rsp = m_db.GetVotes(voteCaseID->value());
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


PyResult CorpRegistryBound::GetSanctionedActionsByCorporation(PyCallArgs &call, PyInt* corporationID, PyInt* state) {
    //  rows = sm.GetService('corp').GetSanctionedActionsByCorporation(eve.session.corpid, state)
    /*
05:09:00 [CorpCallDump]   Call Arguments:
05:09:00 [CorpCallDump]      Tuple: 2 elements
05:09:00 [CorpCallDump]       [ 0]    Integer: 98000000
05:09:00 [CorpCallDump]       [ 1]    Integer: 1
*/
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetSanctionedActionsByCorporation()");
    call.Dump(CORP__CALL_DUMP);

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


    return m_db.GetSanctionedItems(corporationID->value(), state->value());
}

PyResult CorpRegistryBound::CanVote(PyCallArgs &call, PyInt* corporationID) {
    // canVote = sm.GetService('corp').CanVote(self.corpID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CanVote() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    return PyStatic.NewFalse();
}


PyResult CorpRegistryBound::InsertVote(PyCallArgs &call, PyInt* corporationID, PyInt* voteCaseID, PyInt* voteValue) {
    // return self.GetCorpRegistry().InsertVote(corporationID, voteCaseID, voteValue)
    _log(CORP__CALL, "CorpRegistryBound::Handle_InsertVote() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    // can you change your vote?  will we have to check for votes already cast and update them?
    m_db.CastVote(corporationID->value(), call.client->GetCharacterID(), voteCaseID->value(), voteValue->value());

    // returns none
    return PyStatic.NewNone();
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult CorpRegistryBound::GetLockedItemLocations(PyCallArgs& call)
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

PyResult CorpRegistryBound::AddCorporateContact(PyCallArgs &call, PyInt* contactID, PyInt* relationshipID) {
    //self.GetCorpRegistry().AddCorporateContact(contactID, relationshipID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_AddCorporateContact()");
    call.Dump(CORP__CALL_DUMP);

    m_db.AddContact(m_corpID, contactID->value(), relationshipID->value());

    return nullptr;
}

PyResult CorpRegistryBound::EditCorporateContact(PyCallArgs &call, PyInt* contactID, PyInt* relationshipID) {
    //self.GetCorpRegistry().EditCorporateContact(contactID, relationshipID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_EditCorporateContact()");
    call.Dump(CORP__CALL_DUMP);

    m_db.UpdateContact(relationshipID->value(), contactID->value(), m_corpID);

    return nullptr;
}

PyResult CorpRegistryBound::EditContactsRelationshipID(PyCallArgs &call, PyList* contactIDs, PyInt* relationshipID) {
    //self.GetCorpRegistry().EditContactsRelationshipID(contactIDs, relationshipID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_EditContactsRelationshipID()");
    call.Dump(CORP__CALL_DUMP);

    for (PyList::const_iterator itr = contactIDs->begin(); itr != contactIDs->end(); ++itr) {
        m_db.UpdateContact(relationshipID->value(), PyRep::IntegerValueU32(*itr), m_corpID);
    }

    return nullptr;
}

PyResult CorpRegistryBound::RemoveCorporateContacts(PyCallArgs &call, PyList* contactIDs) {
    // self.GetCorpRegistry().RemoveCorporateContacts(contactIDs)
    _log(CORP__CALL, "CorpRegistryBound::Handle_RemoveCorporateContacts()");
    call.Dump(CORP__CALL_DUMP);

    for (PyList::const_iterator itr = contactIDs->begin(); itr != contactIDs->end(); ++itr) {
        m_db.RemoveContact(PyRep::IntegerValueU32(*itr), m_corpID);
    }

    return nullptr;
}

// this is a member role/title update by memberIDs called from corp->members->find in role->task mgmt
PyResult CorpRegistryBound::ExecuteActions(PyCallArgs &call, PyList* targetIDs, PyList* remoteActions) {
    //      verb, property, value = action
    //      remoteActions.append(action)
    //  return self.GetCorpRegistry().ExecuteActions(targetIDs, remoteActions)
    _log(CORP__CALL, "CorpRegistryBound::Handle_ExecuteActions() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    /*
     *   args.memberIDs = list of charIDs to update
     *   args.verb      = one of add, remove, set, give
     *   args.queryType = all role* types or title
     *   args.value     = roleMask or titleID
     */

    return nullptr;
}

PyResult CorpRegistryBound::CreateLabel(PyCallArgs &call, PyRep* name, PyRep* color) {
    // return self.GetCorpRegistry().CreateLabel(name, color)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CreateLabel() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::DeleteLabel(PyCallArgs &call, PyInt* labelID) {
    // self.GetCorpRegistry().DeleteLabel(labelID)
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteLabel() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::EditLabel(PyCallArgs &call, PyInt* labelID, PyRep* name, PyRep* color) {
    // self.GetCorpRegistry().EditLabel(labelID, name, color)
    _log(CORP__CALL, "CorpRegistryBound::Handle_EditLabel() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::AssignLabels(PyCallArgs &call, PyList* contactIDs, PyInt* labelMask) {
    // self.GetCorpRegistry().AssignLabels(contactIDs, labelMask)
    _log(CORP__CALL, "CorpRegistryBound::Handle_AssignLabels() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::RemoveLabels(PyCallArgs &call, PyList* contactIDs, PyInt* labelMask) {
    // self.GetCorpRegistry().RemoveLabels(contactIDs, labelMask)
    _log(CORP__CALL, "CorpRegistryBound::Handle_RemoveLabels() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::CreateAlliance(PyCallArgs &call, PyRep* allianceName, PyRep* shortName, PyRep* description, PyRep* url) {
    //self.GetCorpRegistry().CreateAlliance(allianceName, shortName, description, url)
    _log(CORP__CALL, "CorpRegistryBound::Handle_CreateAlliance() size=%li", call.tuple->size());
    call.Dump(CORP__CALL_DUMP);

    AllianceDB a_db;
    Client* pClient(call.client);
    
    std::string allianceNameStr = PyRep::StringContent(allianceName);
    std::string shortNameStr = PyRep::StringContent(shortName);
    std::string descriptionStr = PyRep::StringContent(description);
    std::string urlStr = PyRep::StringContent(url);

    // verify they're not using bad words in their alliance name
    for (const auto cur : badWords)
        if (EvE::icontains(allianceNameStr, cur))
            throw CustomError ("Alliance Name contains banned words");

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(allianceNameStr, cur))
            throw CustomError ("Alliance Name contains invalid characters");

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(shortNameStr, cur))
            throw CustomError ("Alliance short name contains invalid characters");

    // verify short name is available
    if (a_db.IsShortNameTaken(shortNameStr))
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
    reason += allianceNameStr;
    reason += " (";
    reason += shortNameStr;
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
    this->m_cache->InvalidateCache( cache_name );
    PySafeDecRef( cache_name );

    // Register new alliance (also gather current corpID and new allyID at same time)
    uint32 allyID(0);
    uint32 corpID(0);
    if (!a_db.CreateAlliance(allianceNameStr, shortNameStr, descriptionStr, urlStr, pClient, allyID, corpID)) {
        codelog(SERVICE__ERROR, "New alliance creation failed.");
        return nullptr;
    }

    // create alliance channel
    this->m_lsc->CreateSystemChannel(allyID);

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

PyResult CorpRegistryBound::ApplyToJoinAlliance(PyCallArgs &call, PyInt* allianceID, PyWString* applicationText) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_ApplyToJoinAlliance()");
    call.Dump(CORP__CALL_DUMP);

    AllianceDB a_db;

    Alliance::ApplicationInfo app;
        app.appText = applicationText->content();
        app.allyID = allianceID->value();
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

    std::vector<Client*> list;
    sEntityList.GetCorpClients(list, oaac.corpID);
    for (auto cur : list)
    {
        if (cur != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to %s (%u)", cur->GetName(), cur->GetCharID());
        }
    }

    list.clear();
    sEntityList.GetCorpClients(list, executorID);
    for (auto cur : list)
    {
        if (cur != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to %s (%u)", cur->GetName(), cur->GetCharID());
        }
    }

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string subject = "New application from ";
    subject += call.client->GetName();
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(AllianceDB::GetExecutorID(app.allyID)));
    this->m_lsc->SendMail(m_db.GetCorporationCEO(m_corpID), recipients, subject, applicationText->content());

    return nullptr;
}

PyResult CorpRegistryBound::GetAllianceApplications(PyCallArgs &call) {
    //application = sm.GetService('corp').GetAllianceApplications()[allianceID]
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetAllianceApplications()");
    call.Dump(CORP__CALL_DUMP);

    return AllianceDB::GetMyApplications(m_corpID);
}

PyResult CorpRegistryBound::DeleteAllianceApplication(PyCallArgs &call, PyInt* allianceID) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_DeleteAllianceApplication()");
    call.Dump(CORP__CALL_DUMP);

    //This is not implemented by client, no context menu option

    AllianceDB a_db;

    //Old application info
    Alliance::ApplicationInfo oldInfo = Alliance::ApplicationInfo();

    if (!a_db.GetCurrentApplicationInfo(allianceID->value(), call.client->GetCorporationID(), oldInfo))
    {
        _log(SERVICE__ERROR, "%s: Failed to query application for corp %u alliance %u", call.client->GetName(), call.client->GetCorporationID(), allianceID->value());
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
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to %s(%u)", cur->GetName(), cur->GetCharID());
        }
    }

    list.clear();
    sEntityList.GetCorpClients(list, executorID);
    for (auto cur : list)
    {
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("OnAllianceApplicationChanged", "clientID", oaac.Encode(), false);
            _log(ALLY__TRACE, "OnAllianceApplicationChanged sent to %s(%u)", cur->GetName(), cur->GetCharID());
        }
    }

    return nullptr;
}

PyResult CorpRegistryBound::GetRentalDetailsPlayer(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetRentalDetailsPlayer()
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetRentalDetailsPlayer()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::GetRentalDetailsCorp(PyCallArgs &call) {
    // return self.GetCorpRegistry().GetRentalDetailsCorp()
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetRentalDetailsCorp()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::UpdateCorporationAbilities(PyCallArgs &call) {
    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateCorporationAbilities()");
    call.Dump(CORP__CALL_DUMP);

    //this will need to update corp memberlimit, allowed races, and then update all members with new data

    return nullptr;
}

PyResult CorpRegistryBound::UpdateStationManagementSettings(PyCallArgs &call,
    PyRep* modifiedServiceAccessRulesByServiceID, PyRep* modifiedServiceCostModifiers,
    PyRep* modifiedRentableItems, PyRep* stationName, PyRep* description,
    PyRep* dockingCostPerVolume, PyRep* officeRentalCost, PyRep* reprocessingStationsTake,
    PyRep* reprocessingHangarFlag, PyRep* exitTime, PyRep* standingOwnerID) {
    //  self.corpStationMgr.UpdateStationManagementSettings(self.modifiedServiceAccessRulesByServiceID, self.modifiedServiceCostModifiers, self.modifiedRentableItems, self.station.stationName, self.station.description, self.station.dockingCostPerVolume, self.station.officeRentalCost, self.station.reprocessingStationsTake, self.station.reprocessingHangarFlag, self.station.exitTime, self.station.standingOwnerID)

    _log(CORP__CALL, "CorpRegistryBound::Handle_UpdateStationManagementSettings()");
    call.Dump(CORP__CALL_DUMP);

    // not real sure what this does yet....outpost shit maybe?

    return nullptr;
}

PyResult CorpRegistryBound::GetNumberOfPotentialCEOs(PyCallArgs &call) {
    //  return self.GetCorpRegistry().GetNumberOfPotentialCEOs()
    _log(CORP__CALL, "CorpRegistryBound::Handle_GetNumberOfPotentialCEOs()");
    call.Dump(CORP__CALL_DUMP);


    return nullptr;
}

PyResult CorpRegistryBound::CanLeaveCurrentCorporation(PyCallArgs &call) {
    //  canLeave, error, errorDetails = corpSvc.CanLeaveCurrentCorporation()
    // error:  CrpCantQuitNotInStasis  and canLeave=false for member that has roles

    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, PyStatic.NewTrue());  //canLeave - set this to timer or w/e to deter corp jumpers
        tuple->SetItem(1, PyStatic.NewNone());
        tuple->SetItem(2, PyStatic.NewNone());

    return tuple;
}



