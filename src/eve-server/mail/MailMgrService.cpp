/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
#include "mail/MailDB.h"
#include "mail/MailMgrService.h"

PyCallable_Make_InnerDispatcher(MailMgrService)

MailMgrService::MailMgrService(PyServiceMgr *mgr)
: PyService(mgr, "mailMgr"),
  m_dispatch(new Dispatcher(this)),
  m_db(new MailDB())
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MailMgrService, SendMail);
    PyCallable_REG_CALL(MailMgrService, PrimeOwners);
    PyCallable_REG_CALL(MailMgrService, SyncMail);
    PyCallable_REG_CALL(MailMgrService, GetMailHeaders);
    PyCallable_REG_CALL(MailMgrService, MoveToTrash);
    PyCallable_REG_CALL(MailMgrService, MoveFromTrash);
    PyCallable_REG_CALL(MailMgrService, MarkAsUnread);
    PyCallable_REG_CALL(MailMgrService, MarkAsRead);
    PyCallable_REG_CALL(MailMgrService, MoveAllToTrash);
    PyCallable_REG_CALL(MailMgrService, MoveToTrashByLabel);
    PyCallable_REG_CALL(MailMgrService, MoveToTrashByList);
    PyCallable_REG_CALL(MailMgrService, MarkAllAsUnread);
    PyCallable_REG_CALL(MailMgrService, MarkAsUnreadByLabel);
    PyCallable_REG_CALL(MailMgrService, MarkAsUnreadByList);
    PyCallable_REG_CALL(MailMgrService, MarkAllAsRead);
    PyCallable_REG_CALL(MailMgrService, MarkAsReadByLabel);
    PyCallable_REG_CALL(MailMgrService, MarkAsReadByList);
    PyCallable_REG_CALL(MailMgrService, MoveAllFromTrash);
    PyCallable_REG_CALL(MailMgrService, EmptyTrash);
    PyCallable_REG_CALL(MailMgrService, DeleteMail);
    PyCallable_REG_CALL(MailMgrService, GetBody);
    PyCallable_REG_CALL(MailMgrService, AssignLabels);
    PyCallable_REG_CALL(MailMgrService, RemoveLabels);

    // implemented
    PyCallable_REG_CALL(MailMgrService, GetLabels);
    PyCallable_REG_CALL(MailMgrService, EditLabel);
    PyCallable_REG_CALL(MailMgrService, CreateLabel);
    PyCallable_REG_CALL(MailMgrService, DeleteLabel);
}

MailMgrService::~MailMgrService() {
    delete m_dispatch;
    delete m_db;
}

PyResult MailMgrService::Handle_SendMail(PyCallArgs &call)
{
    Call_SendMail args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode SendMail args");
        return NULL;
    }

    int sender = call.client->GetCharacterID();
    return new PyInt(m_db->SendMail(sender, args.toCharacterIDs, args.toListID, args.toCorpOrAllianceID, args.title, args.body, args.isReplyTo, args.isForwardedFrom));
}

PyResult MailMgrService::Handle_PrimeOwners(PyCallArgs &call)
{
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode PrimeOwners args");
        return NULL;
    }

    return NULL;
}

PyResult MailMgrService::Handle_SyncMail(PyCallArgs &call)
{
    int firstId = 0, secondId = 0;

    if (call.tuple->size() == 2 && !call.tuple->GetItem(0)->IsNone() && !call.tuple->GetItem(1)->IsNone())
    {
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple))
        {
            codelog(CLIENT__ERROR, "Failed to decode SyncMail args");
            return NULL;
        }

        // referring to a mail id range
        int firstId = args.arg1, secondId = args.arg2;
    }

    PyDict* dummy = new PyDict;
    dummy->SetItemString("oldMail", new PyNone());
    dummy->SetItemString("newMail", m_db->GetNewMail(call.client->GetCharacterID()));
    dummy->SetItemString("mailStatus", m_db->GetMailStatus(call.client->GetCharacterID()));
    return new PyObject("util.KeyVal", dummy);
}

PyResult MailMgrService::Handle_AssignLabels(PyCallArgs &call)
{
    Call_AssignLabels args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode AssignLabels args");
        return NULL;
    }
    return NULL;
}

PyResult MailMgrService::Handle_CreateLabel(PyCallArgs &call)
{
    Call_CreateLabel args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode CreateLabel args");
        return NULL;
    }

    uint32 ret;
    if (m_db->CreateLabel(call.client->GetCharacterID(), args, ret))
        return new PyInt(ret);
    return NULL;
}

PyResult MailMgrService::Handle_DeleteLabel(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode DeleteLabel args");
        return NULL;
    }

    m_db->DeleteLabel(call.client->GetCharacterID(), args.arg /*labelID*/);

    return NULL;
}

PyResult MailMgrService::Handle_DeleteMail(PyCallArgs &call)
{
    // contains message ids
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode DeleteMail args");
        return NULL;
    }

    return NULL;
}

PyResult MailMgrService::Handle_EditLabel(PyCallArgs &call)
{
    Call_EditLabel args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode EditLabel args");
        return NULL;
    }

    m_db->EditLabel(call.client->GetCharacterID(), args);
    return NULL;
}

PyResult MailMgrService::Handle_EmptyTrash(PyCallArgs &call)
{
    // no arguments
    return NULL;
}

PyResult MailMgrService::Handle_GetBody(PyCallArgs &call)
{
    Call_MailGetBody args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode GetBody args");
        return NULL;
    }

    m_db->SetMailUnread(args.messageId, args.isUnread);
    return m_db->GetMailBody(args.messageId);
}

PyResult MailMgrService::Handle_GetLabels(PyCallArgs &call)
{
    return m_db->GetLabels(call.client->GetCharacterID());
}

PyResult MailMgrService::Handle_GetMailHeaders(PyCallArgs &call)
{
    // contains message ids
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode GetMailHeaders args");
        return NULL;
    }

    return NULL;
}

PyResult MailMgrService::Handle_MarkAllAsRead(PyCallArgs &call)
{
    // no arguments
    return NULL;
}

PyResult MailMgrService::Handle_MarkAllAsUnread(PyCallArgs &call)
{
    // no arguments
    return NULL;
}

PyResult MailMgrService::Handle_MarkAsRead(PyCallArgs &call)
{
    // message id list
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MarkAsRead args");
        return NULL;
    }

    for (size_t i = 0; i < args.ints.size(); i++)
        m_db->SetMailUnread(args.ints[i], false);

    return NULL;
}

PyResult MailMgrService::Handle_MarkAsReadByLabel(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MarkAsReadByLabel args");
        return NULL;
    }

    int labelId = args.arg;

    return NULL;
}

PyResult MailMgrService::Handle_MarkAsReadByList(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MarkAsReadByList args");
        return NULL;
    }

    int listId = args.arg;

    return NULL;
}

PyResult MailMgrService::Handle_MarkAsUnread(PyCallArgs &call)
{
    // message id list
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MarkAsUnread args");
        return NULL;
    }

    for (size_t i = 0; i < args.ints.size(); i++)
        m_db->SetMailUnread(args.ints[i], true);

    return NULL;
}

PyResult MailMgrService::Handle_MarkAsUnreadByLabel(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MarkAsUnreadByLabel args");
        return NULL;
    }

    int labelId = args.arg;

    return NULL;
}

PyResult MailMgrService::Handle_MarkAsUnreadByList(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MarkAsUnreadByList args");
        return NULL;
    }

    int listId = args.arg;

    return NULL;
}

PyResult MailMgrService::Handle_MoveAllFromTrash(PyCallArgs &call)
{
    // no arguments
    return NULL;
}

PyResult MailMgrService::Handle_MoveAllToTrash(PyCallArgs &call)
{
    // no arguments
    return NULL;
}

PyResult MailMgrService::Handle_MoveFromTrash(PyCallArgs &call)
{
    // message id list
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MoveFromTrash args");
        return NULL;
    }


    return NULL;
}

PyResult MailMgrService::Handle_MoveToTrash(PyCallArgs &call)
{
    // message id list
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MoveToTrash args");
        return NULL;
    }

    return NULL;
}

PyResult MailMgrService::Handle_MoveToTrashByLabel(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MoveToTrashByLabel args");
        return NULL;
    }

    int labelId = args.arg;

    return NULL;
}

PyResult MailMgrService::Handle_MoveToTrashByList(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode MoveToTrashByList args");
        return NULL;
    }

    int listId = args.arg;

    return NULL;
}

PyResult MailMgrService::Handle_RemoveLabels(PyCallArgs &call)
{
    // we reuse Call_AssignLabels here because the arguments match
    Call_AssignLabels args;
    if (!args.Decode(&call.tuple))
    {
        codelog(CLIENT__ERROR, "Failed to decode RemoveLabels args");
        return NULL;
    }
    return NULL;
}
