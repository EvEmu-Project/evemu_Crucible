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
    Author:        Luck, caytchen
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "mail/MailMgrService.h"
#include "EVE_Mail.h"

MailMgrService::MailMgrService() :
    Service("mailMgr")
{
    this->Add("SendMail", &MailMgrService::SendMail);
    this->Add("PrimeOwners", &MailMgrService::PrimeOwners);
    this->Add("SyncMail", &MailMgrService::SyncMail);
    this->Add("GetMailHeaders", &MailMgrService::GetMailHeaders);
    this->Add("MoveToTrash", &MailMgrService::MoveToTrash);
    this->Add("MoveFromTrash", &MailMgrService::MoveFromTrash);
    this->Add("MarkAsUnread", &MailMgrService::MarkAsUnread);
    this->Add("MarkAsRead", &MailMgrService::MarkAsRead);
    this->Add("MoveAllToTrash", &MailMgrService::MoveAllToTrash);
    this->Add("MoveToTrashByLabel", &MailMgrService::MoveToTrashByLabel);
    this->Add("MoveToTrashByList", &MailMgrService::MoveToTrashByList);
    this->Add("MarkAllAsUnread", &MailMgrService::MarkAllAsUnread);
    this->Add("MarkAsUnreadByLabel", &MailMgrService::MarkAsUnreadByLabel);
    this->Add("MarkAsUnreadByList", &MailMgrService::MarkAsUnreadByList);
    this->Add("MarkAllAsRead", &MailMgrService::MarkAllAsRead);
    this->Add("MarkAsReadByLabel", &MailMgrService::MarkAsReadByLabel);
    this->Add("MarkAsReadByList", &MailMgrService::MarkAsReadByList);
    this->Add("MoveAllFromTrash", &MailMgrService::MoveAllFromTrash);
    this->Add("EmptyTrash", &MailMgrService::EmptyTrash);
    this->Add("DeleteMail", &MailMgrService::DeleteMail);
    this->Add("GetBody", &MailMgrService::GetBody);
    this->Add("AssignLabels", &MailMgrService::AssignLabels);
    this->Add("RemoveLabels", &MailMgrService::RemoveLabels);

    // implemented
    this->Add("GetLabels", &MailMgrService::GetLabels);
    this->Add("EditLabel", &MailMgrService::EditLabel);
    this->Add("CreateLabel", &MailMgrService::CreateLabel);
    this->Add("DeleteLabel", &MailMgrService::DeleteLabel);
}

PyResult MailMgrService::SendMail(PyCallArgs &call, PyList* toCharacterIDs, std::optional<PyInt*> listID, std::optional<PyInt*> toCorpOrAllianceID, PyWString* title, PyWString* body, PyInt* isReplyTo, PyInt* isForwardedFrom)
{
    std::vector<int32> characters;

    PyList::const_iterator list_2_cur = toCharacterIDs->begin();
    for (size_t list_2_index(0); list_2_cur != toCharacterIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SendMail failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        characters.push_back(t->value());
    }

    int sender = call.client->GetCharacterID();
    return new PyInt(
        m_db.SendMail(
            sender, characters,
            listID.has_value() ? listID.value()->value() : -1,
            toCorpOrAllianceID.has_value() ? toCorpOrAllianceID.value()->value() : -1,
            title->content(), body->content(),
            isReplyTo->value(),
            isForwardedFrom->value()
        )
    );
}

PyResult MailMgrService::PrimeOwners(PyCallArgs &call, PyList* ownerIDs)
{
    std::vector<int32> owners;

    PyList::const_iterator list_2_cur = ownerIDs->begin();
    for (size_t list_2_index(0); list_2_cur != ownerIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SendMail failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        owners.push_back(t->value());
    }
    return ServiceDB::PrimeOwners(owners);
}

PyResult MailMgrService::SyncMail(PyCallArgs &call, std::optional<PyInt*> first, std::optional<PyInt*> second)
{
    int firstId = 0, secondId = 0;

    if (first.has_value() && second.has_value())
    {
        // referring to a mail id range
        int firstId = first.value()->value(), secondId = second.value()->value();
    }

    PyDict* dummy = new PyDict;
    dummy->SetItemString("oldMail", PyStatic.NewNone());
    dummy->SetItemString("newMail", m_db.GetNewMail(call.client->GetCharacterID()));
    dummy->SetItemString("mailStatus", m_db.GetMailStatus(call.client->GetCharacterID()));
    return new PyObject("util.KeyVal", dummy);
}

PyResult MailMgrService::AssignLabels(PyCallArgs &call, PyList* messageIDs, PyInt* labelID)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    m_db.ApplyLabels(messageIds, labelID->value());

    return nullptr;
}

PyResult MailMgrService::CreateLabel(PyCallArgs &call, PyWString* name, std::optional<PyInt*> color)
{
    uint32 ret;
    if (m_db.CreateLabel(call.client->GetCharacterID(), name->content(), color.has_value() ? color.value()->value() : -1, ret))
        return new PyInt(ret);
    return nullptr;
}

PyResult MailMgrService::DeleteLabel(PyCallArgs &call, PyInt* labelID)
{
    m_db.DeleteLabel(call.client->GetCharacterID(), labelID->value());

    return nullptr;
}

PyResult MailMgrService::DeleteMail(PyCallArgs &call, PyList* messageIDs)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    for (int i = 0; i < messageIds.size(); i++) {
        int32 messageID = messageIds[i];
        m_db.DeleteMail(messageID);
    }

    return nullptr;
}

PyResult MailMgrService::EditLabel(PyCallArgs &call, PyInt* labelID, PyWString* name, std::optional<PyInt*> color)
{
    m_db.EditLabel(call.client->GetCharacterID(), labelID->value(), name->content(), color.has_value() ? color.value()->value() : -1);
    return nullptr;
}

PyResult MailMgrService::EmptyTrash(PyCallArgs &call)
{
    // @TODO: TEST
    m_db.EmptyTrash(call.client->GetCharacterID());
    return nullptr;
}

PyResult MailMgrService::GetBody(PyCallArgs &call, PyInt* messageID, PyInt* isUnread)
{
    if (!isUnread->value()) {
        m_db.SetMailUnread(messageID->value());
    } else {
        m_db.SetMailRead(messageID->value());
    }

    return m_db.GetMailBody(messageID->value());
}

PyResult MailMgrService::GetLabels(PyCallArgs &call)
{
    return m_db.GetLabels(call.client->GetCharacterID());
}

PyResult MailMgrService::GetMailHeaders(PyCallArgs &call, PyList* messageIDs)
{
    // @TODO: Stub
    // contains message ids
    return nullptr;
}

PyResult MailMgrService::MarkAllAsRead(PyCallArgs &call)
{
    m_db.MarkAllAsRead(call.client->GetCharacterID());

    return nullptr;
}

PyResult MailMgrService::MarkAllAsUnread(PyCallArgs &call)
{
    m_db.MarkAllAsUnread(call.client->GetCharacterID());
    return nullptr;
}

PyResult MailMgrService::MarkAsRead(PyCallArgs &call, PyList* messageIDs)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    m_db.SetMailsRead(messageIds);

    return nullptr;
}

PyResult MailMgrService::MarkAsReadByLabel(PyCallArgs &call, PyInt* labelID)
{
    m_db.MarkAllAsReadByLabel(call.client->GetCharacterID(), labelID->value());

    return nullptr;
}

PyResult MailMgrService::MarkAsReadByList(PyCallArgs &call, PyInt* listID)
{
    return nullptr;
}

PyResult MailMgrService::MarkAsUnread(PyCallArgs &call, PyList* messageIDs)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    m_db.SetMailsUnread(messageIds);

    return nullptr;
}

PyResult MailMgrService::MarkAsUnreadByLabel(PyCallArgs &call, PyInt* labelID)
{
    m_db.MarkAllAsUnreadByLabel(call.client->GetCharacterID(), labelID->value());

    return nullptr;
}

PyResult MailMgrService::MarkAsUnreadByList(PyCallArgs &call, PyList* messageIDs)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    return nullptr;
}

PyResult MailMgrService::MoveAllFromTrash(PyCallArgs &call)
{
    m_db.MoveAllFromTrash(call.client->GetCharacterID());
    return nullptr;
}

PyResult MailMgrService::MoveAllToTrash(PyCallArgs &call)
{
    m_db.MoveAllToTrash(call.client->GetCharacterID());
    return nullptr;
}

PyResult MailMgrService::MoveFromTrash(PyCallArgs &call, PyList* messageIDs)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    m_db.RemoveStatusMasks(messageIds, mailStatusMaskTrashed);

    return nullptr;
}

PyResult MailMgrService::MoveToTrash(PyCallArgs &call, PyList* messageIDs)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    m_db.ApplyStatusMasks(messageIds, mailStatusMaskTrashed);

    return nullptr;
}

PyResult MailMgrService::MoveToTrashByLabel(PyCallArgs &call, PyInt* labelID)
{
    return nullptr;
}

PyResult MailMgrService::MoveToTrashByList(PyCallArgs &call, PyInt* listID)
{
    return nullptr;
}

PyResult MailMgrService::RemoveLabels(PyCallArgs &call, PyList* messageIDs, PyInt* labelID)
{
    std::vector<int32> messageIds;

    PyList::const_iterator list_2_cur = messageIDs->begin();
    for (size_t list_2_index(0); list_2_cur != messageIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_AssignLabels failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        messageIds.push_back(t->value());
    }

    m_db.RemoveLabels(messageIds, labelID->value());

    return nullptr;
}
