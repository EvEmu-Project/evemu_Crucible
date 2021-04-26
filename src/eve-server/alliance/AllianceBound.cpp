
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
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "alliance/AllianceBound.h"
#include "alliance/AllianceDB.h"
#include "alliance/AllianceData.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "packets/CorporationPkts.h"
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
    args.Dump(CORP__TRACE);

    //Instantiate an applicationinfo struct to store our call arguments in
    Alliance::ApplicationInfo app;
    app.appText = args.applicationText;
    app.corpID = args.corporationID;
    app.state = EveAlliance::AppStatus::AppEffective;
    app.valid = true;

    //If we are accepting an application
    if (app.state == EveAlliance::AppStatus::AppEffective)
    {
        //creating an alliance will affect eveStaticOwners, so we gotta invalidate the cache...
        //  call to db.AddCorporation() will update eveStaticOwners with new corp
        PyString *cache_name = new PyString("config.StaticOwners");
        m_manager->cache_service->InvalidateCache(cache_name);
        PySafeDecRef(cache_name);

        // join corporation to alliance
        if (!m_db.UpdateCorpAlliance(m_allyID, app.corpID))
        {
            codelog(SERVICE__ERROR, "Alliance join failed.");
            return nullptr;
        }

        // Add alliance membership record to corporation
        if (!m_db.AddEmployment(m_allyID, app.corpID))
        {
            codelog(SERVICE__ERROR, "Add corp employment failed.");
            return nullptr;
        }
    }

    if (!m_db.UpdateApplication(app))
    {
        codelog(SERVICE__ERROR, "Updating alliance application failed.");
        return nullptr;
    }

    return nullptr;
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
    if (!args.Decode(&call.tuple)) {
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
    if (!args.Decode(&call.tuple)) {
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
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (PyList::const_iterator itr = args.contactIDs->begin(); itr != args.contactIDs->end(); ++itr) {
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
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (PyList::const_iterator itr = args.contactIDs->begin(); itr != args.contactIDs->end(); ++itr) {
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
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.UpdateAlliance(m_allyID, args.description, args.url);
    return nullptr;
}
