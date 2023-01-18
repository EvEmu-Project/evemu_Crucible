
 /**
  * @name FleetBound.cpp
  *     Fleet Bound code for EVEmu
  *     This code handles specific fleet functions
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */


//work in progress

#include "eve-server.h"



#include "fleet/FleetBound.h"
#include "fleet/FleetObject.h"
#include "services/ServiceManager.h"

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

FleetBound::FleetBound(EVEServiceManager& mgr, FleetObject& parent, uint32 fleetID) :
    EVEBoundObject(mgr, parent),
    m_fleetID (fleetID)
{
    this->Add("Init", &FleetBound::Init);
    this->Add("GetInitState", &FleetBound::GetInitState);
    this->Add("GetFleetID", &FleetBound::GetFleetID);
    this->Add("Invite", &FleetBound::Invite);
    this->Add("AcceptInvite", &FleetBound::AcceptInvite);
    this->Add("RejectInvite", &FleetBound::RejectInvite);
    this->Add("ChangeWingName", &FleetBound::ChangeWingName);
    this->Add("ChangeSquadName", &FleetBound::ChangeSquadName);
    this->Add("SetOptions", &FleetBound::SetOptions);
    this->Add("GetWings", &FleetBound::GetWings);
    this->Add("MakeLeader", &FleetBound::MakeLeader);
    this->Add("SetBooster", &FleetBound::SetBooster);
    this->Add("MoveMember", &FleetBound::MoveMember);
    this->Add("KickMember", &FleetBound::KickMember);
    this->Add("CreateWing", &FleetBound::CreateWing);
    this->Add("CreateSquad", &FleetBound::CreateSquad);
    this->Add("DeleteWing", &FleetBound::DeleteWing);
    this->Add("DeleteSquad", &FleetBound::DeleteSquad);
    this->Add("LeaveFleet", &FleetBound::LeaveFleet);
    this->Add("GetFleetComposition", &FleetBound::GetFleetComposition);
    this->Add("GetJoinRequests", &FleetBound::GetJoinRequests);
    this->Add("RejectJoinRequest", &FleetBound::RejectJoinRequest);
    this->Add("SendBroadcast", &FleetBound::SendBroadcast);
    this->Add("SetMotdEx", &FleetBound::SetMotdEx);
    this->Add("GetMotd", &FleetBound::GetMotd);
    this->Add("UpdateMemberInfo", &FleetBound::UpdateMemberInfo);

    // stubs
    this->Add("Reconnect", &FleetBound::Reconnect);
    this->Add("AddToVoiceChat", &FleetBound::AddToVoiceChat);
    this->Add("SetVoiceMuteStatus", &FleetBound::SetVoiceMuteStatus);
    this->Add("ExcludeFromVoiceMute", &FleetBound::ExcludeFromVoiceMute);
}

PyResult FleetBound::GetFleetID(PyCallArgs &call) {
    return new PyInt(m_fleetID);
}

PyResult FleetBound::Init(PyCallArgs &call, std::optional <PyInt*> shipTypeID) {
    //self.fleet.Init(self.GetMyShipTypeID())
    // this only sends ship typeID when inspace.
    sLog.Warning("FleetBound", "Handle_Init() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

/*
    FleetInitCall args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        // this should throw an error here...
        return nullptr;
    }
    args.myShipTypeID
    args.unknownDict
    */

    Client* pClient = call.client;
    if (pClient == nullptr)
        return new PyLong(GetFileTimeNow());

    Character* pChar = pClient->GetChar().get();
    if (pChar == nullptr)
        return new PyLong(GetFileTimeNow());

    // member is already added here, so cannot use sFltSvc.AddMember()
    JoinFleetRSP join;
        join.charID = pClient->GetCharacterID();
        join.clientID = pClient->GetClientID();
        join.job = pChar->fleetJob();
        join.role = pChar->fleetRole();
        join.shipTypeID = pClient->GetShip()->typeID();
        join.skillFleetCommand = pChar->GetSkillLevel(EvESkill::FleetCommand);
        join.skillLeadership = pChar->GetSkillLevel(EvESkill::Leadership);
        join.skillWingCommand = pChar->GetSkillLevel(EvESkill::WingCommand);
        join.squadID = pChar->squadID();
        join.roleBooster = pChar->fleetBooster();
        join.solarSystemID = pClient->GetSystemID();
        join.wingID = pChar->wingID();
        join.timestamp = pChar->fleetJoinTime();
    PyTuple* res = new PyTuple(1);
        res->SetItem(0, join.Encode());
    if (is_log_enabled(FLEET__DEBUG)) {
        _log(FLEET__UPDATE_DUMP, "OnFleetJoin");
        res->Dump(FLEET__UPDATE_DUMP, "   ");
    }
    pClient->SendNotification("OnFleetJoin", "clientid", &res, true);

    PyDict* dict = new PyDict();
        dict->SetItemString("targetTags", new PyDict());
    PyTuple* obj = new PyTuple(1);
        obj->SetItem(0, new PyObject("util.KeyVal", dict));
    _log(FLEET__UPDATE_DUMP, "OnFleetStateChange");
    obj->Dump(FLEET__UPDATE_DUMP, "   ");
    pClient->SendNotification("OnFleetStateChange", "clientid", obj, true);

    PyTuple* count = new PyTuple(1);
        count->SetItem(0, new PyInt(254));
    _log(FLEET__UPDATE_DUMP, "OnFleetActive");
    count->Dump(FLEET__UPDATE_DUMP, "   ");
    pClient->SendNotification("OnFleetActive", "clientid", count, true);

    //response should be OID
    /*
    [PyDict 1 kvp]
      [PyString "OID+"]
      [PyDict 1 kvp]
        [PyString "N=790423:223898"]  "N=%u:%u", &nodeID, &bindID
        [PyIntegerVar 129756560170416597]
        */
    return new PyLong(GetFileTimeNow());
}

PyResult FleetBound::GetInitState(PyCallArgs &call) {
    FleetData fData = FleetData();
    sFltSvc.GetFleetData(m_fleetID, fData);

    GetInitStateRSP rsp;
        rsp.fleetID = m_fleetID;
        rsp.isFreeMove = fData.isFreeMove;
        rsp.isLootLogging = fData.isLootLogging;
        rsp.isRegistered = fData.isRegistered;
        rsp.isVoiceEnabled = fData.isVoiceEnabled;
        rsp.motd = fData.motd;
        rsp.name = fData.name;

    PyDict* muteDict = new PyDict();
    for (auto cur : fData.isExcludedFromMuting)
        muteDict->SetItem(new PyInt(cur.first), new PyInt(cur.second));
    PySafeDecRef(rsp.isExcludedFromMuting);
    rsp.isExcludedFromMuting = muteDict;

    PyDict* lMuteDict = new PyDict();
    for (auto cur : fData.isMutedByLeader)
        lMuteDict->SetItem(new PyInt(cur.first), new PyInt(cur.second));
    PySafeDecRef(rsp.isMutedByLeader);
    rsp.isMutedByLeader = lMuteDict;

    std::vector< uint32 > wingIDs, squadIDs;
    wingIDs.clear();
    sFltSvc.GetWingIDs(m_fleetID, wingIDs);
    PyDict* dict1 = new PyDict();
    for (auto wingID : wingIDs) {
        WingData wData = WingData();
        sFltSvc.GetWingData(wingID, wData);
        WingRSP wing;
        wing.name = wData.name;
        wing.wingID = wingID;

        squadIDs.clear();
        sFltSvc.GetSquadIDs(wingID, squadIDs);

        PyDict* dict = new PyDict();
        for (auto squadID : squadIDs) {
            SquadData sData = SquadData();
            sFltSvc.GetSquadData(squadID, sData);
            SquadRSP squad;
            squad.name = sData.name;
            squad.squadID = squadID;
            dict->SetItem(new PyInt(squadID), squad.Encode());
        }
        PySafeDecRef(wing.squads);
        wing.squads = dict;
        dict1->SetItem(new PyInt(wingID), wing.Encode());
    }
    PySafeDecRef(rsp.wings);
    rsp.wings = dict1;

    std::vector<Client*> members;
    sFltSvc.GetMemeberVec(m_fleetID, members);
    PyDict* dict2 = new PyDict();
    for (auto cur : members) {
        MemberRSP member;
            member.charID = cur->GetCharacterID();
            member.clientID = cur->GetClientID();
            member.job = cur->GetChar()->fleetJob();
            member.role = cur->GetChar()->fleetRole();
            member.shipTypeID = cur->GetShip()->typeID();
            member.skillFleetCommand = cur->GetChar()->GetSkillLevel(EvESkill::FleetCommand);
            member.skillLeadership = cur->GetChar()->GetSkillLevel(EvESkill::Leadership);
            member.skillWingCommand = cur->GetChar()->GetSkillLevel(EvESkill::WingCommand);
            member.squadID = cur->GetChar()->squadID();
            member.roleBooster = cur->GetChar()->fleetBooster();
            member.solarSystemID = cur->GetSystemID();
            member.wingID = cur->GetChar()->wingID();
            member.timestamp = cur->GetChar()->fleetJoinTime();
        dict2->SetItem(new PyInt(member.charID), member.Encode());
    }

    rsp.members = dict2;
    if (is_log_enabled(FLEET__DEBUG))
        rsp.Dump(FLEET__DEBUG);

    //response should be sequenced
    return rsp.Encode();
}

PyResult FleetBound::Invite(PyCallArgs &call, PyInt* characterID, std::optional<PyInt*> wingID, std::optional <PyInt*> squadID, PyLong* role) {
    sLog.Warning("FleetBound", "Handle_Invite() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Client* pClient = sEntityList.FindClientByCharID(characterID->value());
    if (pClient == nullptr)
        return PyStatic.NewNone();
    if (pClient->GetChar()->fleetID()) {
        call.client->SendNotifyMsg("%s is already in a fleet.  Denying Fleet Invite issue.", pClient->GetChar()->name());
        return PyStatic.NewNone();
    }

    InviteData data = InviteData();
        data.inviteBy = call.client;
        data.invited = pClient;
        data.wingID = wingID.has_value() ? wingID.value()->value() : 0;  // default sends 0
        data.squadID = squadID.has_value() ? squadID.value()->value() : 0;  // default sends 0
        data.role = role->value();

    if (data.role == Fleet::Role::FleetLeader) {
        data.wingID = -1;
        data.squadID = -1;
        _log(FLEET__MESSAGE, "Fleet Invite:  Role: %s, WingID: %i, SquadID: %i", sFltSvc.GetRoleName(data.role).c_str(), data.wingID, data.squadID);
    } else if (data.role == Fleet::Role::WingLeader) {
        // wing should be populated at this point
        data.squadID = -1;
        _log(FLEET__MESSAGE, "Fleet Invite:  Role: %s, WingID: %i, SquadID: %i", sFltSvc.GetRoleName(data.role).c_str(), data.wingID, data.squadID);
    } else if (data.role == Fleet::Role::SquadLeader) {
        // wing and squad should both be populated at this point
        _log(FLEET__MESSAGE, "Fleet Invite:  Role: %s, WingID: %i, SquadID: %i", sFltSvc.GetRoleName(data.role).c_str(), data.wingID, data.squadID);
    } else if ((data.wingID == 0) and (data.squadID == 0)) {// char invited as generic squad member, no specific position
        sFltSvc.GetRandUnitIDs(m_fleetID, data.wingID, data.squadID);
        _log(FLEET__MESSAGE, "Fleet Invite:  Role: %s, WingID: %i, SquadID: %i", sFltSvc.GetRoleName(data.role).c_str(), data.wingID, data.squadID);
    } else {
        _log(FLEET__ERROR, "Fleet Invite Role/UnitID error.  Role: %s, WingID: %i, SquadID: %i", sFltSvc.GetRoleName(data.role).c_str(), data.wingID, data.squadID);
    }

    if (!sFltSvc.SaveInviteData(characterID->value(), data)) {
        call.client->SendNotifyMsg("%s is invited to another fleet.  That invite must be rejected before another can be issued.", pClient->GetCharName().c_str());
        return PyStatic.NewNone();
    }

    // join requests are accepted via invite.  delete request on invite
    sFltSvc.RemoveJoinRequest(m_fleetID, pClient);

    PyTuple* tuple = new PyTuple(4);
        tuple->SetItem(0, new PyInt(m_fleetID));
        tuple->SetItem(1, new PyInt(data.inviteBy->GetCharacterID()));
        tuple->SetItem(2, new PyString("AskJoinFleet"));
    PyTuple* tuple2 = new PyTuple(2);
        tuple2->SetItem(0, new PyInt(call.client->GetChar()->fleetJob()));
        tuple2->SetItem(1, new PyInt(data.inviteBy->GetCharacterID()));
    PyDict* dict = new PyDict();
        dict->SetItemString("name", tuple2);
        tuple->SetItem(3, dict);
    if (is_log_enabled(FLEET__UPDATE_DUMP)) {
        _log(FLEET__UPDATE_DUMP, "OnFleetInvite");
        tuple->Dump(FLEET__UPDATE_DUMP, "   ");
    }
    pClient->SendNotification("OnFleetInvite", "clientID", &tuple, true);

    // returns nothing
    return nullptr;
}

PyResult FleetBound::AcceptInvite(PyCallArgs &call, std::optional <PyInt*> shipTypeID) {
    sLog.Warning("FleetBound", "Handle_AcceptInvite() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Character* pChar = call.client->GetChar().get();
    if (pChar == nullptr)
        return new PyLong(GetFileTimeNow());

    InviteData data = InviteData();
    if (!sFltSvc.GetInviteData(pChar->itemID(), data)){
        call.client->SendNotifyMsg("You do not have an outstanding Fleet Invite on issue.");
        return new PyLong(GetFileTimeNow());
    }

    int8 booster = Fleet::Booster::No;
    switch (data.role) {
        case Fleet::Role::FleetLeader: {
            FleetData fData = FleetData();
            sFltSvc.GetFleetData(m_fleetID, fData);
            if (fData.booster == nullptr)
                booster = Fleet::Booster::Fleet;
        } break;
        case Fleet::Role::WingLeader: {
            WingData wData = WingData();
            sFltSvc.GetWingData(data.wingID, wData);
            if (wData.booster == nullptr)
                booster = Fleet::Booster::Wing;
        } break;
        case Fleet::Role::SquadLeader: {
            SquadData sData = SquadData();
            sFltSvc.GetSquadData(data.squadID, sData);
            if (sData.booster == nullptr)
                booster = Fleet::Booster::Squad;
        } break;
    }

    sFltSvc.AddMember(call.client, m_fleetID, data.wingID, data.squadID, Fleet::Job::None, data.role, booster);
    sFltSvc.RemoveInviteData(pChar->itemID());

    // returns nodeID and timestamp
    return this->GetOID();
}

PyResult FleetBound::RejectInvite(PyCallArgs &call) {
    sLog.Warning("FleetBound", "Handle_RejectInvite() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Character* pChar = call.client->GetChar().get();
    if (pChar == nullptr)
        return new PyLong(GetFileTimeNow());

    bool rejected = false;
    if (call.tuple->AsTuple()->GetItem(0)->IsBool())
        rejected = call.tuple->AsTuple()->GetItem(0)->AsBool()->value();

    InviteData data = InviteData();
    if (!sFltSvc.GetInviteData(pChar->itemID(), data)) {
        call.client->SendNotifyMsg("You do not have an outstanding Fleet Invite on issue.");
        return new PyLong(GetFileTimeNow());
    }

    if (rejected) {
        data.inviteBy->SendNotifyMsg("%s has rejected your fleet invite.", data.invited->GetChar()->name());
    } else {
        data.inviteBy->SendNotifyMsg("%s has auto-rejected your fleet invite.", data.invited->GetChar()->name());
    }
    sFltSvc.RemoveInviteData(pChar->itemID());

    // returns nodeID and timestamp
    return this->GetOID();
}


PyResult FleetBound::ChangeWingName(PyCallArgs &call, PyInt* wingID, PyRep* name) {
          //   self.fleet.ChangeWingName(wingID, ret[:MAX_NAME_LENGTH])
    sLog.Warning("FleetBound", "Handle_ChangeWingName() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.RenameWing(wingID->value(), PyRep::StringContent (name));

    // returns nothing
    return nullptr;
}

PyResult FleetBound::ChangeSquadName(PyCallArgs &call, PyInt* squadID, PyRep* name) {
          //   self.fleet.ChangeSquadName(squadID, ret[:MAX_NAME_LENGTH])
    sLog.Warning("FleetBound", "Handle_ChangeSquadName() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.RenameSquad(squadID->value(), PyRep::StringContent(name));

    // returns nothing
    return nullptr;
}

PyResult FleetBound::SetOptions(PyCallArgs &call, PyObject* options) {
          //   self.fleet.SetOptions(options)
    sLog.Warning("FleetBound", "Handle_SetOptions() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    PyDict* dict = options->arguments()->AsDict();
    bool isFreeMove = dict->GetItemString("isFreeMove")->AsBool()->value();
    bool isRegistered = dict->GetItemString("isRegistered")->AsBool()->value();
    bool isVoiceEnabled = (dict->GetItemString("isVoiceEnabled")->AsBool()->value() ? sConfig.chat.EnableVoiceChat : false);

    sFltSvc.UpdateOptions(m_fleetID, isFreeMove, isRegistered, isVoiceEnabled);

    // returns nothing
    return nullptr;
}

PyResult FleetBound::GetJoinRequests(PyCallArgs &call) {
          //   self.fleet.GetJoinRequests()
    /* 20:12:59 W FleetBound: Handle_GetJoinRequests() size=0
     * 20:12:59 [FleetDump]   Call Arguments:
     * 20:12:59 [FleetDump]       Tuple: Empty
     */
    sLog.Warning("FleetBound", "Handle_GetJoinRequests() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    std::vector<Client*> cVec;
    sFltSvc.GetJoinRequests(call.client->GetChar()->fleetID(), cVec);

    PyDict* rsp = new PyDict();
    for (auto cur : cVec) {
        PyDict* dict = new PyDict();
        dict->SetItemString("charID", new PyInt(cur->GetCharacterID()));
        rsp->SetItem(new PyInt(cur->GetCharacterID()), new PyObject("util.KeyVal", dict));
    }

    if (is_log_enabled(FLEET__DEBUG))
        rsp->Dump(FLEET__DEBUG, "    ");
    return rsp;
}

PyResult FleetBound::RejectJoinRequest(PyCallArgs &call, PyInt* characterID) {
    //    self.fleet.RejectJoinRequest(charID)
    sLog.Warning("FleetBound", "Handle_RejectJoinRequest() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Client* pClient = sEntityList.FindClientByCharID(characterID->value());
    sFltSvc.RemoveJoinRequest(call.client->GetChar()->fleetID(), pClient);

    pClient->SendInfoModalMsg("Your fleet join request was denied by %s", call.client->GetName());

    // returns nothing
    return nullptr;
}

PyResult FleetBound::GetFleetComposition(PyCallArgs &call) {
    sLog.Warning("FleetBound", "Handle_GetFleetComposition() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Character* pChar(nullptr);
    PyList* list = new PyList();
    std::vector<Client*> members;
    sFltSvc.GetMemeberVec(m_fleetID, members);
    for (auto cur : members) {
        pChar = cur->GetChar().get();
        if (pChar == nullptr)
            continue;
        PyDict* dict = new PyDict();
        dict->SetItemString("characterID", new PyInt(cur->GetCharacterID()));
        dict->SetItemString("shipTypeID", new PyInt(cur->GetShip()->typeID()));
        dict->SetItemString("solarSystemID", new PyInt(cur->GetSystemID()));
        if (pChar->HasSkill(EvESkill::Leadership) or pChar->HasSkill(EvESkill::FleetCommand) or pChar->HasSkill(EvESkill::WingCommand)) {
            PyTuple* skills = new PyTuple(3);
                skills->SetItem(0, new PyInt(pChar->GetSkillLevel(EvESkill::Leadership)));
                skills->SetItem(1, new PyInt(pChar->GetSkillLevel(EvESkill::FleetCommand)));
                skills->SetItem(2, new PyInt(pChar->GetSkillLevel(EvESkill::WingCommand)));
            dict->SetItemString("skills", skills);
            PyTuple* skillIDs = new PyTuple(3);
                skillIDs->SetItem(0, new PyInt(EvESkill::Leadership));
                skillIDs->SetItem(1, new PyInt(EvESkill::FleetCommand));
                skillIDs->SetItem(2, new PyInt(EvESkill::WingCommand));
            dict->SetItemString("skillIDs", skillIDs);
        }
        list->AddItem(new PyObject("util.KeyVal", dict));
        pChar = nullptr;
    }

    if (is_log_enabled(FLEET__DEBUG))
        list->Dump(FLEET__DEBUG, "    ");
    return list;
}

PyResult FleetBound::GetWings(PyCallArgs &call) {
         //    self.fleet.GetWings()
    sLog.Warning("FleetBound", "Handle_GetWings() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    return sFltSvc.GetWings(m_fleetID);
}

// this is fleet-wide
PyResult FleetBound::SendBroadcast(PyCallArgs &call, PyRep* message, PyInt* group, PyInt* itemID) {
         //    self.fleet.SendBroadcast(name, self.broadcastScope, itemID)
    sLog.Warning("FleetBound", "Handle_SendBroadcast() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.FleetBroadcast(call.client, itemID->value(), Fleet::BCast::Scope::Universe, group->value(), PyRep::StringContent (message));

    // returns nothing
    return nullptr;
}

PyResult FleetBound::UpdateMemberInfo(PyCallArgs &call, std::optional <PyInt*> shipTypeID) {
         //    self.fleet.UpdateMemberInfo(self.GetMyShipTypeID())
    /*
     * 13:26:54 W FleetBound: Handle_UpdateMemberInfo() size=1
     * 13:26:54 [FleetDump]   Call Arguments:
     * 13:26:54 [FleetDump]       Tuple: 1 elements
     * 13:26:54 [FleetDump]         [ 0] (None)
     */
    sLog.Warning("FleetBound", "Handle_UpdateMemberInfo() size=%lli", call.tuple->size());
    //call.Dump(FLEET__DUMP);

    // returns nothing
    return nullptr;
}

PyResult FleetBound::SetMotdEx(PyCallArgs &call, PyRep* motd) {
         //    self.fleet.SetMotdEx(motd)
    sLog.Warning("FleetBound", "Handle_SetMotdEx() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.SetMOTD(m_fleetID, PyRep::StringContent(motd));

    // returns nothing
    return nullptr;
}

PyResult FleetBound::GetMotd(PyCallArgs &call) {
    //   self.motd = self.fleet.GetMotd()
    sLog.Warning("FleetBound", "Handle_GetMotd() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    return sFltSvc.GetMOTD(m_fleetID);
}

PyResult FleetBound::LeaveFleet(PyCallArgs &call) {
    //    self.fleet.LeaveFleet()
    sLog.Warning("FleetBound", "Handle_LeaveFleet() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.LeaveFleet(call.client);

    // returns nothing
    return nullptr;
}

PyResult FleetBound::MakeLeader(PyCallArgs &call, PyInt* characterID) {
    /* self.fleet.MakeLeader(charID)  */
    sLog.Warning("FleetBound", "Handle_MakeLeader() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    // leaders will keep existing fleetJob.

    // get existing leader to remove current Role, if applicable
    Client* pClient = sFltSvc.GetFleetLeader(m_fleetID);
    if (pClient != nullptr) {
        Character* pCharOld = pClient->GetChar().get();
        if (pCharOld == nullptr)
            return PyStatic.NewNone();
        int32 wingID = 0, squadID = 0;
        sFltSvc.GetRandUnitIDs(m_fleetID, wingID, squadID);
        sFltSvc.UpdateMember(pCharOld->itemID(), m_fleetID, wingID, squadID, pCharOld->fleetJob(), Fleet::Role::Member, pCharOld->fleetBooster());
    }

    // update new leader
    Character* pCharNew = sEntityList.FindClientByCharID(characterID->value())->GetChar().get();
    if (pCharNew == nullptr)
        return PyStatic.NewNone();
    sFltSvc.UpdateMember(characterID->value(), m_fleetID, -1, -1, pCharNew->fleetJob(), Fleet::Role::FleetLeader, pCharNew->fleetBooster());

    // returns nothing
    return nullptr;
}

PyResult FleetBound::SetBooster(PyCallArgs &call, PyInt* characterID, std::optional <PyInt*> roleBooster) {
    /*self.fleet.SetBooster(charID, roleBooster):  */
    sLog.Warning("FleetBound", "Handle_SetBooster() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Character* pOldChar(nullptr);
    Character* pChar = sEntityList.FindClientByCharID(characterID->value())->GetChar().get();
    if (pChar == nullptr)
        return PyStatic.NewFalse();

    // there can be only one booster per each fleet/wing/squad
    switch (roleBooster.has_value() ? roleBooster.value()->value() : 0) {
        case Fleet::Booster::Fleet: {
            FleetData fData = FleetData();
            sFltSvc.GetFleetData(m_fleetID, fData);
            if ((fData.booster != nullptr) and ((pOldChar = fData.booster->GetChar().get()) != nullptr)) {
                if (!sFltSvc.UpdateMember(pOldChar->itemID(), m_fleetID, pOldChar->wingID(), pOldChar->squadID(), pOldChar->fleetJob(), pOldChar->fleetRole(), Fleet::Booster::No))
                    return PyStatic.NewFalse();
            }
        } break;
        case Fleet::Booster::Wing: {
            WingData wData = WingData();
            sFltSvc.GetWingData(pChar->wingID(), wData);
            if ((wData.booster != nullptr) and ((pOldChar = wData.booster->GetChar().get()) != nullptr)) {
                if (!sFltSvc.UpdateMember(pOldChar->itemID(), m_fleetID, pOldChar->wingID(), pOldChar->squadID(), pOldChar->fleetJob(), pOldChar->fleetRole(), Fleet::Booster::No))
                    return PyStatic.NewFalse();
            }
        } break;
        case Fleet::Booster::Squad: {
            SquadData sData = SquadData();
            sFltSvc.GetSquadData(pChar->squadID(), sData);
            if ((sData.booster != nullptr) and ((pOldChar = sData.booster->GetChar().get()) != nullptr)) {
                if (!sFltSvc.UpdateMember(pOldChar->itemID(), m_fleetID, pOldChar->wingID(), pOldChar->squadID(), pOldChar->fleetJob(), pOldChar->fleetRole(), Fleet::Booster::No))
                    return PyStatic.NewFalse();
            }
        } break;
    }

    if (sFltSvc.UpdateMember(characterID->value(), m_fleetID, pChar->wingID(), pChar->squadID(), pChar->fleetJob(), pChar->fleetRole(), roleBooster.has_value() ? roleBooster.value()->value() : 0))
        return PyStatic.NewTrue();

    // returns boolean
    return PyStatic.NewFalse();
}

PyResult FleetBound::MoveMember(PyCallArgs &call, PyInt* characterID, std::optional <PyInt*> wingID, std::optional <PyInt*> squadID, std::optional <PyInt*> role, std::optional <PyInt*> booster) {
    sLog.Warning("FleetBound", "Handle_MoveMember() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    Character* pChar = sEntityList.FindClientByCharID(characterID->value())->GetChar().get();
    if (pChar == nullptr)
        return PyStatic.NewFalse();

    // if voice is enabled for this fleet, SendNotification OnFleetMove to enable changing channels (on client side)

    if (sFltSvc.UpdateMember(characterID->value(), m_fleetID, wingID.has_value() ? wingID.value()->value() : 0, squadID.has_value() ? squadID.value()->value() : 0, pChar->fleetJob(), role.has_value() ? role.value()->value() : 0, booster.has_value() ? booster.value()->value() : 0))
        return PyStatic.NewTrue();

    // returns boolean
    return PyStatic.NewFalse();
}

PyResult FleetBound::KickMember(PyCallArgs &call, PyInt* characterID) {
    /*
     *        if charID == eve.session.charid:
     *            self.LeaveFleet()
     *        else:
     *            self.fleet.KickMember(charID)
     */
    sLog.Warning("FleetBound", "Handle_KickMember() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.LeaveFleet(sEntityList.FindClientByCharID(characterID->value()));

    // returns boolean
    return PyStatic.NewTrue();
}

PyResult FleetBound::CreateWing(PyCallArgs &call) {
    /*  wingID = self.fleet.CreateWing()  */
    sLog.Warning("FleetBound", "Handle_CreateWing() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    return sFltSvc.CreateWing(m_fleetID);
}

PyResult FleetBound::CreateSquad(PyCallArgs &call, PyInt* wingID) {
    /* self.fleet.CreateSquad(wingID)  */
    sLog.Warning("FleetBound", "Handle_CreateSquad() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.CreateSquad(m_fleetID, wingID->value());

    // returns nothing
    return nullptr;
}

PyResult FleetBound::DeleteWing(PyCallArgs &call, PyInt* wingID) {
    /*    self.fleet.DeleteWing(wingID)  */
    sLog.Warning("FleetBound", "Handle_DeleteWing() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.DeleteWing(wingID->value());

    // returns nothing
    return nullptr;
}

PyResult FleetBound::DeleteSquad(PyCallArgs &call, PyInt* squadID) {
    /* self.fleet.DeleteSquad(squadID)  */
    sLog.Warning("FleetBound", "Handle_DeleteSquad() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    sFltSvc.DeleteSquad(squadID->value());   //call.tuple->AsTuple()->GetItem(0)->AsInt()->value()

    // returns nothing
    return nullptr;
}

PyResult FleetBound::AddToVoiceChat(PyCallArgs &call, PyRep* channelName) {
    //    self.fleet.AddToVoiceChat(channelName)
    sLog.Warning("FleetBound", "Handle_AddToVoiceChat() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    //sConfig.chat.EnableVoiceChat;

    // returns nothing
    return nullptr;
}

PyResult FleetBound::SetVoiceMuteStatus(PyCallArgs &call, PyRep* status, PyRep* channel) {
    //    self.fleet.SetVoiceMuteStatus(status, channel)
    sLog.Warning("FleetBound", "Handle_SetVoiceMuteStatus() size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    // returns nothing
    return nullptr;
}

PyResult FleetBound::ExcludeFromVoiceMute(PyCallArgs &call, PyInt* characterID, PyRep* channel) {
    //    self.fleet.ExcludeFromVoiceMute(charid, channel)
    sLog.Warning("FleetBound", "Handle_ExcludeFromVoiceMute()) size=%lli", call.tuple->size());
    call.Dump(FLEET__DUMP);

    // returns nothing
    return nullptr;
}

PyResult FleetBound::Reconnect(PyCallArgs &call) {
    // no args
    sLog.Warning("FleetBound", "Handle_Reconnect()) size=%lli", call.tuple->size());

    // returns nothing
    return nullptr;
}
