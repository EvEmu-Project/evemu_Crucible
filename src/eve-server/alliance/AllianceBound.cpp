
/**
  * @name AllianceBound.cpp
  *     Alliance Bound code
  *
  * @Author:        Allan
  * Updated:        James
  * @date:          16 January 2018
  */

#include <string>

#include "EVE_Corp.h"
#include "EVE_Mail.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "alliance/AllianceBound.h"
#include "alliance/AllianceDB.h"
#include "alliance/AllianceData.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "packets/CorporationPkts.h"
#include "corporation/CorporationDB.h"
#include "station/StationDB.h"
#include "station/StationDataMgr.h"

/*
 * ALLY__ERROR
 * ALLY__WARNING
 * ALLY__INFO
 * ALLY__MESSAGE
 * ALLY__TRACE
 * ALLY__CALL
 * ALLY__CALL_DUMP
 * ALLY__RSP_DUMP
 */

AllianceBound::AllianceBound(PyServiceMgr *mgr, AllianceDB &db, uint32 allyID)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "AllianceBound";

    PyCallable_REG_CALL(AllianceBound, GetAlliance);
    PyCallable_REG_CALL(AllianceBound, AddBulletin);
    PyCallable_REG_CALL(AllianceBound, GetBulletins);
    PyCallable_REG_CALL(AllianceBound, DeleteBulletin);
    PyCallable_REG_CALL(AllianceBound, CreateLabel);
    PyCallable_REG_CALL(AllianceBound, GetLabels);
    PyCallable_REG_CALL(AllianceBound, DeleteLabel);
    PyCallable_REG_CALL(AllianceBound, EditLabel);
    PyCallable_REG_CALL(AllianceBound, AssignLabels);
    PyCallable_REG_CALL(AllianceBound, RemoveLabels);
    PyCallable_REG_CALL(AllianceBound, GetMembers);
    PyCallable_REG_CALL(AllianceBound, DeclareExecutorSupport);
    PyCallable_REG_CALL(AllianceBound, DeleteMember);
    PyCallable_REG_CALL(AllianceBound, GetApplications);
    PyCallable_REG_CALL(AllianceBound, UpdateApplication);
    PyCallable_REG_CALL(AllianceBound, AddToVoiceChat);
    PyCallable_REG_CALL(AllianceBound, PayBill);
    PyCallable_REG_CALL(AllianceBound, GetBills);
    PyCallable_REG_CALL(AllianceBound, GetBillsReceivable);
    PyCallable_REG_CALL(AllianceBound, GetAllianceContacts);
    PyCallable_REG_CALL(AllianceBound, AddAllianceContact);
    PyCallable_REG_CALL(AllianceBound, EditAllianceContact);
    PyCallable_REG_CALL(AllianceBound, RemoveAllianceContacts);
    PyCallable_REG_CALL(AllianceBound, EditContactsRelationshipID);
    PyCallable_REG_CALL(AllianceBound, UpdateAlliance);

    m_allyID = allyID;
}

PyResult AllianceBound::Handle_GetAlliance(PyCallArgs &call)
{
    // Works
    //   self.members = self.GetMoniker().GetAlliance()
    _log(ALLY__CALL, "AllianceBound::Handle_GetAlliance() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    // called by member of this alliance
    return m_db.GetAlliance(m_allyID);
}

PyResult AllianceBound::Handle_GetMembers(PyCallArgs &call)
{
    // Works
    //   self.members = self.GetMoniker().GetMembers()
    _log(ALLY__CALL, "AllianceBound::Handle_GetMembers() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetMembers(m_allyID);

    return nullptr;
}

PyResult AllianceBound::Handle_DeclareExecutorSupport(PyCallArgs &call)
{
    //   self.GetMoniker().DeclareExecutorSupport(corpID)
    _log(ALLY__CALL, "AllianceBound::Handle_DeclareExecutorSupport() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.DeclareExecutorSupport(call.client->GetCorporationID(), args.arg);

    return nullptr;
}

PyResult AllianceBound::Handle_DeleteMember(PyCallArgs &call)
{
    //  self.GetMoniker().DeleteMember(corpID)
    _log(ALLY__CALL, "AllianceBound::Handle_DeleteMember() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.DeleteMember(m_allyID, args.arg);

    return nullptr;
}

PyResult AllianceBound::Handle_GetApplications(PyCallArgs &call)
{
    //   self.applications = self.GetMoniker().GetApplications()
    // Get all applications to our alliance
    _log(ALLY__CALL, "AllianceBound::Handle_GetApplications() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetApplications(m_allyID);

    return nullptr;
}

PyResult AllianceBound::Handle_UpdateApplication(PyCallArgs &call)
{
    //    return self.GetMoniker().UpdateApplication(corpID, applicationText, state)
    _log(ALLY__CALL, "AllianceBound::Handle_UpdateApplication() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_UpdateAllianceApplication args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(ALLY__TRACE);

    //We don't delay joining an alliance so we will simply change AppAccepted to AppEffective
    if (args.applicationStatus == EveAlliance::AppStatus::AppAccepted)
    {
        args.applicationStatus = EveAlliance::AppStatus::AppEffective;
    }

    //Old application info
    Alliance::ApplicationInfo oldInfo = Alliance::ApplicationInfo();

    if (!m_db.GetCurrentApplicationInfo(m_allyID, args.corporationID, oldInfo))
    {
        _log(SERVICE__ERROR, "%s: Failed to query application for corp %u alliance %u", call.client->GetName(), args.corporationID, m_allyID);
        return nullptr;
    }

    //New application info
    Alliance::ApplicationInfo newInfo = oldInfo;
    newInfo.valid = true;
    newInfo.state = args.applicationStatus;
    if (!m_db.UpdateApplication(newInfo))
    {
        _log(SERVICE__ERROR, "%s: Failed to update application for corp %u alliance %u", call.client->GetName(), args.corporationID, m_allyID);
        return nullptr;
    }

    OnAllianceApplicationChanged oaac;
    oaac.allianceID = newInfo.allyID;
    oaac.corpID = newInfo.corpID;
    FillOAApplicationChange(oaac, oldInfo, newInfo);

    //Client list for notifications
    std::vector<Client *> list;

    //If we are accepting an application
    if (args.applicationStatus == EveAlliance::AppStatus::AppEffective)
    {
        // OnAllianceMemberChanged event
        OnAllianceMemberChange oamc;
        oamc.allyID = newInfo.allyID;
        oamc.corpID = newInfo.corpID;
        FillOAMemberChange(oamc, oldInfo, newInfo);

        //Send to everyone who needs to see it in the applying corp and in the alliance executor corp
        uint32 executorID = AllianceDB::GetExecutorID(newInfo.allyID);

        list.clear();
        sEntityList.GetCorpClients(list, oamc.corpID);
        for (auto cur : list)
        {
            if (cur->GetChar().get() != nullptr)
            {
                cur->SendNotification("OnAllianceMemberChanged", "clientID", oamc.Encode(), false);
                _log(ALLY__TRACE, "OnAllianceMemberChanged sent to client %u", cur->GetClientID());
            }
        }

        list.clear();
        sEntityList.GetCorpClients(list, executorID);
        for (auto cur : list)
        {
            if (cur->GetChar().get() != nullptr)
            {
                cur->SendNotification("OnAllianceMemberChanged", "clientID", oamc.Encode(), false);
                _log(ALLY__TRACE, "OnAllianceMemberChanged sent to client %u", cur->GetClientID());
            }
        }

        //  call to db.AddCorporation() will update eveStaticOwners with new corp
        PyString *cache_name = new PyString("config.StaticOwners");
        m_manager->cache_service->InvalidateCache(cache_name);
        PySafeDecRef(cache_name);

        // join corporation to alliance
        if (!m_db.UpdateCorpAlliance(m_allyID, args.corporationID))
        {
            codelog(SERVICE__ERROR, "Alliance join failed.");
            return nullptr;
        }

        // Add alliance membership record to corporation
        if (!m_db.AddEmployment(m_allyID, args.corporationID))
        {
            codelog(SERVICE__ERROR, "Add corp employment failed.");
            return nullptr;
        }

        // Send an evemail to the corporation in question that the application has been accepted
        std::string subject = "Application to alliance accepted";
        std::string message = "Your alliance application has been accepted.";
        subject += call.client->GetName();
        std::vector<int32> recipients;
        recipients.push_back(CorporationDB::GetCorporationCEO(args.corporationID));
        m_manager->lsc_service->SendMail(CorporationDB::GetCorporationCEO(m_db.GetExecutorID(m_allyID)), recipients, subject, message);

        //Send OnAllianceChanged packet
        OnAllianceChanged ac;
        ac.allianceID = m_allyID;
        if (!m_db.CreateAllianceChangePacket(ac, 0, m_allyID))
        {
            codelog(SERVICE__ERROR, "Failed to create OnAllianceChanged notification stream.");
            call.client->SendErrorMsg("Unable to notify about alliance change.");
            return nullptr;
        }

        //Send to everyone who needs to see it in the applying corp
        list.clear();
        sEntityList.GetCorpClients(list, args.corporationID);
        for (auto cur : list)
        {
            if (cur->GetChar().get() != nullptr)
            {
                cur->SendNotification("OnAllianceChanged", "clientID", ac.Encode(), false);
                _log(ALLY__TRACE, "OnAllianceChanged sent to client %u", cur->GetClientID());
            }
        }
    }

    else if (args.applicationStatus == EveAlliance::AppStatus::AppRejected)
    {
        _log(ALLY__TRACE, "Application of %u:%u has been rejected, sending notification...", m_allyID, args.corporationID);
    }

    else
    {
        _log(SERVICE__ERROR, "%s: Sent unhandled status %u ", call.client->GetName(), args.applicationStatus);
    }

    //Send to everyone who needs to see it in the applying corp and in the alliance executor corp
    uint32 executorID = AllianceDB::GetExecutorID(oaac.allianceID);

    list.clear();
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

void AllianceBound::FillOAApplicationChange(OnAllianceApplicationChanged &OAAC, const Alliance::ApplicationInfo &Old, const Alliance::ApplicationInfo &New)
{
    if (Old.valid)
    {
        OAAC.applicationIDOld = new PyInt(Old.appID);
        OAAC.applicationTextOld = new PyString(Old.appText);
        OAAC.corporationIDOld = new PyInt(Old.corpID);
        OAAC.allianceIDOld = new PyInt(Old.allyID);
        OAAC.stateOld = new PyInt(Old.state);
        OAAC.applicationDateTimeOld = new PyLong(Old.appTime);
        if (Old.deleted == true)
        {
            OAAC.deletedOld = new PyBool(true);
        }
    }
    if (New.valid)
    {
        OAAC.applicationIDNew = new PyInt(New.appID);
        OAAC.applicationTextNew = new PyString(New.appText);
        OAAC.corporationIDNew = new PyInt(New.corpID);
        OAAC.allianceIDNew = new PyInt(New.allyID);
        OAAC.stateNew = new PyInt(New.state);
        OAAC.applicationDateTimeNew = new PyLong(New.appTime);
        if (New.deleted == true)
        {
            OAAC.deletedNew = new PyBool(true);
        }
    }
}

void AllianceBound::FillOAMemberChange(OnAllianceMemberChange &oamc, const Alliance::ApplicationInfo &Old, const Alliance::ApplicationInfo &New)
{
    if (Old.valid)
    {
        oamc.corporationIDOld = new PyInt(Old.corpID);
        oamc.corporationNameOld = new PyString(CorporationDB::GetCorpName(Old.corpID));
        oamc.chosenExecutorIDOld = new PyInt(AllianceDB::GetExecutorID(Old.allyID));
    }
    if (New.valid)
    {
        oamc.corporationIDNew = new PyInt(New.corpID);
        oamc.corporationNameNew = new PyString(CorporationDB::GetCorpName(New.corpID));
        oamc.chosenExecutorIDNew = new PyInt(AllianceDB::GetExecutorID(New.allyID));
    }
}


PyResult AllianceBound::Handle_AddToVoiceChat(PyCallArgs &call)
{
    //    success = moniker.GetAlliance().AddToVoiceChat(vivoxChannelName)
    _log(ALLY__CALL, "AllianceBound::Handle_AddToVoiceChat() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_PayBill(PyCallArgs &call)
{
    //   return self.GetMoniker().PayBill(billID, fromAccountKey)
    _log(ALLY__CALL, "AllianceBound::Handle_PayBill() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceBound::Handle_GetBillBalance(PyCallArgs &call)
{
    //   return self.GetMoniker().GetBillBalance(billID)
    _log(ALLY__CALL, "AllianceBound::Handle_GetBillBalance() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceBound::Handle_GetBills(PyCallArgs &call)
{
    //   return self.GetMoniker().GetBills()
    _log(ALLY__CALL, "AllianceBound::Handle_GetBills() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceBound::Handle_GetBillsReceivable(PyCallArgs &call)
{
    //   return self.GetMoniker().GetBillsReceivable()
    _log(ALLY__CALL, "AllianceBound::Handle_GetBillsReceivable() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceBound::Handle_AddBulletin(PyCallArgs &call)
{
    //   sm.GetService('alliance').GetMoniker().AddBulletin(title, body)
    //  sm.GetService('alliance').GetMoniker().AddBulletin(title, body, bulletinID=bulletinID, editDateTime=editDateTime)  <-- this is to update bulletin
    _log(ALLY__CALL, "AllianceBound::Handle_AddBulletin() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_AddBulletin args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.AddBulletin(m_allyID, call.client->GetCorporationID(), call.client->GetCharacterID(), args.title, args.body);

    return nullptr;
}

PyResult AllianceBound::Handle_DeleteBulletin(PyCallArgs &call)
{
    //   sm.GetService('alliance').GetMoniker().DeleteBulletin(id)
    _log(ALLY__CALL, "AllianceBound::Handle_DeleteBulletin() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.DeleteBulletin(args.arg);
    return nullptr;
}

PyResult AllianceBound::Handle_GetBulletins(PyCallArgs &call)
{
    //   self.bulletins = self.GetMoniker().GetBulletins()
    _log(ALLY__CALL, "AllianceBound::Handle_GetBulletins() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return m_db.GetBulletins(m_allyID);
}

PyResult AllianceBound::Handle_GetAllianceContacts(PyCallArgs &call)
{
    //    return self.GetMoniker().GetAllianceContacts()
    _log(ALLY__CALL, "AllianceBound::Handle_GetAllianceContacts() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetContacts(m_allyID);
}

PyResult AllianceBound::Handle_AddAllianceContact(PyCallArgs &call)
{
    //   self.GetMoniker().AddAllianceContact(contactID, relationshipID)
    _log(ALLY__CALL, "AllianceBound::Handle_AddAllianceContact() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_CorporateContactData args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.AddContact(m_allyID, args);

    return nullptr;
}

PyResult AllianceBound::Handle_EditAllianceContact(PyCallArgs &call)
{
    //   self.GetMoniker().EditAllianceContact(contactID, relationshipID)
    _log(ALLY__CALL, "AllianceBound::Handle_EditAllianceContact() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_CorporateContactData args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.UpdateContact(args.relationshipID, args.contactID, m_allyID);

    return nullptr;
}

PyResult AllianceBound::Handle_RemoveAllianceContacts(PyCallArgs &call)
{
    //   self.GetMoniker().RemoveAllianceContacts(contactIDs)
    _log(ALLY__CALL, "AllianceBound::Handle_RemoveAllianceContacts() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_RemoveCorporateContacts args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (PyList::const_iterator itr = args.contactIDs->begin(); itr != args.contactIDs->end(); ++itr)
    {
        m_db.RemoveContact(PyRep::IntegerValueU32(*itr), m_allyID);
    }

    return nullptr;
}

PyResult AllianceBound::Handle_EditContactsRelationshipID(PyCallArgs &call)
{
    //    self.GetMoniker().EditContactsRelationshipID(contactIDs, relationshipID)
    _log(ALLY__CALL, "AllianceBound::Handle_EditContactsRelationshipID() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_EditCorporateContacts args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (PyList::const_iterator itr = args.contactIDs->begin(); itr != args.contactIDs->end(); ++itr)
    {
        m_db.UpdateContact(args.relationshipID, PyRep::IntegerValueU32(*itr), m_allyID);
    }

    return nullptr;
}

PyResult AllianceBound::Handle_GetLabels(PyCallArgs &call)
{
    //   return self.GetMoniker().GetLabels()
    _log(ALLY__CALL, "AllianceBound::Handle_GetLabels() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_CreateLabel(PyCallArgs &call)
{
    //   return self.GetMoniker().CreateLabel(name, color)
    _log(ALLY__CALL, "AllianceBound::Handle_CreateLabel() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_DeleteLabel(PyCallArgs &call)
{
    //   self.GetMoniker().DeleteLabel(labelID)
    _log(ALLY__CALL, "AllianceBound::Handle_DeleteLabel() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_EditLabel(PyCallArgs &call)
{
    //   self.GetMoniker().EditLabel(labelID, name, color)
    _log(ALLY__CALL, "AllianceBound::Handle_EditLabel() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_AssignLabels(PyCallArgs &call)
{
    //   self.GetMoniker().AssignLabels(contactIDs, labelMask)
    _log(ALLY__CALL, "AllianceBound::Handle_AssignLabels() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_RemoveLabels(PyCallArgs &call)
{
    //   self.GetMoniker().RemoveLabels(contactIDs, labelMask)
    _log(ALLY__CALL, "AllianceBound::Handle_RemoveLabels() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_UpdateAlliance(PyCallArgs &call)
{
    //    return self.GetMoniker().UpdateAlliance(description, url)
    _log(ALLY__CALL, "AllianceBound::Handle_UpdateAlliance() size=%u", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    Call_UpdateAlliance args;
    if (!args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.UpdateAlliance(m_allyID, args.description, args.url);

    //Send OnAllianceChanged packet
    OnAllianceChanged ac;
    ac.allianceID = m_allyID;
    if (!m_db.CreateAllianceChangePacket(ac, 0, m_allyID))
    {
        codelog(SERVICE__ERROR, "Failed to create OnAllianceChanged notification stream.");
        call.client->SendErrorMsg("Unable to notify about alliance change.");
        return nullptr;
    }

    //Send notification to the client about the alliance change which just occurred
    call.client->SendNotification("OnAllianceChanged", "clientID", ac.Encode(), false);
    _log(SOV__DEBUG, "OnAllianceChanged sent to client %u", call.client->GetClientID());

    return nullptr;
}
