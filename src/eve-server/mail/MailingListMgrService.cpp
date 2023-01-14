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
#include "mail/MailingListMgrService.h"

MailingListMgrService::MailingListMgrService() :
    Service("mailingListsMgr")
{
    this->Add("GetJoinedLists", &MailingListMgrService::GetJoinedLists);
    this->Add("Create", &MailingListMgrService::Create);
    this->Add("Join", &MailingListMgrService::Join);
    this->Add("Leave", &MailingListMgrService::Leave);
    this->Add("Delete", &MailingListMgrService::Delete);
    this->Add("KickMembers", &MailingListMgrService::KickMembers);
    this->Add("GetMembers", &MailingListMgrService::GetMembers);
    this->Add("SetEntityAccess", &MailingListMgrService::SetEntityAccess);
    this->Add("ClearEntityAccess", &MailingListMgrService::ClearEntityAccess);
    this->Add("SetMembersMuted", &MailingListMgrService::SetMembersMuted);
    this->Add("SetMembersOperator", &MailingListMgrService::SetMembersOperator);
    this->Add("SetMembersClear", &MailingListMgrService::SetMembersClear);
    this->Add("SetDefaultAccess", &MailingListMgrService::SetDefaultAccess);
    this->Add("GetInfo", &MailingListMgrService::GetInfo);
    this->Add("GetSettings", &MailingListMgrService::GetSettings);
    this->Add("GetWelcomeMail", &MailingListMgrService::GetWelcomeMail);
    this->Add("SaveWelcomeMail", &MailingListMgrService::SaveWelcomeMail);
    this->Add("SendWelcomeMail", &MailingListMgrService::SendWelcomeMail);
    this->Add("ClearWelcomeMail", &MailingListMgrService::ClearWelcomeMail);
}

PyResult MailingListMgrService::GetJoinedLists(PyCallArgs& call)
{
    // @TODO: Test
    // no args
    sLog.Debug("MailingListMgrService", "Called GetJoinedLists stub" );

    return m_db.GetJoinedMailingLists(call.client->GetCharacterID());
}

PyResult MailingListMgrService::Create(PyCallArgs& call, PyWString* name, PyInt* defaultAccess, PyInt* defaultMemberAccess, std::optional<PyInt*> mailCost)
{
    // @TODO: Test
    sLog.Debug("MailingListMgrService", "Called Create stub" );
    uint32 r = m_db.CreateMailingList(call.client->GetCharacterID(), name->content(), defaultAccess->value(),
                                   defaultMemberAccess->value(), mailCost.has_value() ? mailCost.value()->value() : 0);
    if (r >= 0) {
        return new PyInt(r);
    }
    return nullptr;
}

PyResult MailingListMgrService::Join(PyCallArgs& call, PyRep* listName)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called Join stub" );

    std::string listNameStr = PyRep::StringContent (listName);
    // returns mailing list object
    return nullptr;
}

PyResult MailingListMgrService::Leave(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called Leave stub" );
    // no return values
    return nullptr;
}

PyResult MailingListMgrService::Delete(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called Delete stub" );
    // no return values
    return nullptr;
}

PyResult MailingListMgrService::KickMembers(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called KickMembers stub" );
    for (int i = 0; i < memberIDs->size(); i++) {
        PyRep *member = memberIDs->GetItem(i);
        member->Dump(SERVICE__ERROR, "member item");
    }

    // no return values
    return nullptr;
}

PyResult MailingListMgrService::GetMembers(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetMembers stub" );
    return m_db.GetMailingListMembers(listID->value());
}

PyResult MailingListMgrService::SetEntityAccess(PyCallArgs& call, PyInt* listID, PyInt* entityID, PyInt* access)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetEntityAccess stub" );

    // no return values
    return nullptr;
}

PyResult MailingListMgrService::ClearEntityAccess(PyCallArgs& call, PyInt* listID, PyInt* entityID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called ClearEntityAccess stub" );
    // no return values
    return nullptr;
}

PyResult MailingListMgrService::SetMembersMuted(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetMembersMuted stub" );
    return nullptr;
}

PyResult MailingListMgrService::SetMembersOperator(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetMembersOperator stub" );
    return nullptr;
}

PyResult MailingListMgrService::SetMembersClear(PyCallArgs& call, PyInt* listID, PyList* memberIDs)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetMembersClear stub" );
    return nullptr;
}

PyResult MailingListMgrService::SetDefaultAccess(PyCallArgs& call, PyInt* listID, PyInt* defaultAccess, PyInt* defaultMemberAccess, std::optional<PyInt*> mailCost)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SetDefaultAccess stub" );
    m_db.SetMailingListDefaultAccess(listID->value(), defaultAccess->value(),
                                      defaultMemberAccess->value(), mailCost.has_value() ? mailCost.value()->value() : 0);

    return nullptr;
}

PyResult MailingListMgrService::GetInfo(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetInfo stub" );

    if (listID->value() == 0) {
        PyDict *ret = new PyDict();
        ret->SetItem("displayName", new PyString("Test"));
        return new PyObject("util.KeyVal", ret);
    }
    return nullptr;
}

PyResult MailingListMgrService::GetSettings(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Test
    sLog.Debug("MailingListMgrService", "Called GetSettings stub" );
    // return:
    // .access: list (ownerID, accessLevel)
    // .defaultAccess
    // .defaultMemberAccess

    return m_db.MailingListGetSettings(listID->value());
}

PyResult MailingListMgrService::GetWelcomeMail(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called GetWelcomeMail stub" );
    return nullptr;
}

PyResult MailingListMgrService::SaveWelcomeMail(PyCallArgs& call, PyInt* listID, PyWString* title, PyWString* body)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SaveWelcomeMail stub" );

    return nullptr;
}

PyResult MailingListMgrService::SendWelcomeMail(PyCallArgs& call, PyInt* listID, PyWString* title, PyWString* body)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called SendWelcomeMail stub" );
    return nullptr;
}

PyResult MailingListMgrService::ClearWelcomeMail(PyCallArgs& call, PyInt* listID)
{
    // @TODO: Stub
    sLog.Debug("MailingListMgrService", "Called ClearWelcomeMail stub" );

    // no return value
    return nullptr;
}
