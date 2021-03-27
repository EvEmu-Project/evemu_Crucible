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
    Author:        Luck, caytchen
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "mail/MailingListMgrService.h"

PyCallable_Make_InnerDispatcher(MailingListMgrService)

MailingListMgrService::MailingListMgrService(PyServiceMgr *mgr)
: PyService(mgr, "mailingListsMgr"),
    m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MailingListMgrService, GetJoinedLists);
    PyCallable_REG_CALL(MailingListMgrService, Create);
    PyCallable_REG_CALL(MailingListMgrService, Join);
    PyCallable_REG_CALL(MailingListMgrService, Leave);
    PyCallable_REG_CALL(MailingListMgrService, Delete);
    PyCallable_REG_CALL(MailingListMgrService, KickMembers);
    PyCallable_REG_CALL(MailingListMgrService, GetMembers);
    PyCallable_REG_CALL(MailingListMgrService, SetEntityAccess);
    PyCallable_REG_CALL(MailingListMgrService, ClearEntityAccess);
    PyCallable_REG_CALL(MailingListMgrService, SetMembersMuted);
    PyCallable_REG_CALL(MailingListMgrService, SetMembersOperator);
    PyCallable_REG_CALL(MailingListMgrService, SetMembersClear);
    PyCallable_REG_CALL(MailingListMgrService, SetDefaultAccess);
    PyCallable_REG_CALL(MailingListMgrService, GetInfo);
    PyCallable_REG_CALL(MailingListMgrService, GetSettings);
    PyCallable_REG_CALL(MailingListMgrService, GetWelcomeMail);
    PyCallable_REG_CALL(MailingListMgrService, SaveWelcomeMail);
    PyCallable_REG_CALL(MailingListMgrService, SendWelcomeMail);
    PyCallable_REG_CALL(MailingListMgrService, ClearWelcomeMail);
}

MailingListMgrService::~MailingListMgrService()
{
    delete m_dispatch;
}

PyResult MailingListMgrService::Handle_GetJoinedLists(PyCallArgs& call)
{
    // @TODO: Test
    // no args
    sLog.Debug("MailingListMgrService", "Called GetJoinedLists stub" );

    return m_db.GetJoinedMailingLists(call.client->GetCharacterID());
}

PyResult MailingListMgrService::Handle_Create(PyCallArgs& call)
{
    // @TODO: Test
    sLog.Debug("MailingListMgrService", "Called Create stub" );
    Call_CreateMailingList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 r = m_db.CreateMailingList(call.client->GetCharacterID(), args.name, args.defaultAccess,
                                   args.defaultMemberAccess, args.mailCost);
    if (r >= 0) {
        return new PyInt(r);
    }
    return nullptr;
}

PyResult MailingListMgrService::Handle_Join(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called Join stub" );
    Call_SingleStringArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    std::string listName = args.arg;
    // returns mailing list object
    return nullptr;
}

PyResult MailingListMgrService::Handle_Leave(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called Leave stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int listID = args.arg;
    // no return values
    return nullptr;
}

PyResult MailingListMgrService::Handle_Delete(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called Delete stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int listID = args.arg;
    // no return values
    return nullptr;
}

PyResult MailingListMgrService::Handle_KickMembers(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called KickMembers stub" );
    Call_MemberList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    for (int i = 0; i < args.memberIDs->size(); i++) {
        PyRep *member = args.memberIDs->GetItem(i);
        member->Dump(SERVICE__ERROR, "member item");
    }

    // no return values
    return nullptr;
}

PyResult MailingListMgrService::Handle_GetMembers(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetMembers stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int listID = args.arg;
    return m_db.GetMailingListMembers(listID);
}

PyResult MailingListMgrService::Handle_SetEntityAccess(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetEntityAccess stub" );
    Call_SetEntityAccess args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // no return values
    return nullptr;
}

PyResult MailingListMgrService::Handle_ClearEntityAccess(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called ClearEntityAccess stub" );
    Call_ClearEntityAccess args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // no return values
    return nullptr;
}

PyResult MailingListMgrService::Handle_SetMembersMuted(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetMembersMuted stub" );
    Call_MemberList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult MailingListMgrService::Handle_SetMembersOperator(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetMembersOperator stub" );
    Call_MemberList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult MailingListMgrService::Handle_SetMembersClear(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetMembersClear stub" );
    Call_MemberList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult MailingListMgrService::Handle_SetDefaultAccess(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetDefaultAccess stub" );
    Call_SetDefaultAccess args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.SetMailingListDefaultAccess(args.listID, args.defaultAccess,
                                      args.defaultMemberAccess, args.mailCost);

    return nullptr;
}

PyResult MailingListMgrService::Handle_GetInfo(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetInfo stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int listID = args.arg;
    if (listID == 0) {
        PyDict *ret = new PyDict();
        ret->SetItem("displayName", new PyString("Test"));
        return new PyObject("util.KeyVal", ret);
    }
    return nullptr;
}

PyResult MailingListMgrService::Handle_GetSettings(PyCallArgs& call)
{
    // @TODO: Test
    sLog.Debug("MailingListMgrService", "Called GetSettings stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // return:
    // .access: list (ownerID, accessLevel)
    // .defaultAccess
    // .defaultMemberAccess

    int listID = args.arg;
    return m_db.MailingListGetSettings(listID);
}

PyResult MailingListMgrService::Handle_GetWelcomeMail(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetWelcomeMail stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int listID = args.arg;
    return nullptr;
}

PyResult MailingListMgrService::Handle_SaveWelcomeMail(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SaveWelcomeMail stub" );
    Call_SaveWelcomeMail args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult MailingListMgrService::Handle_SendWelcomeMail(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SendWelcomeMail stub" );
    Call_SaveWelcomeMail args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult MailingListMgrService::Handle_ClearWelcomeMail(PyCallArgs& call)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called ClearWelcomeMail stub" );
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int listID = args.arg;
    // no return value
    return nullptr;
}
