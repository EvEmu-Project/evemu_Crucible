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

#ifndef MAILING_LIST_MGR_SERVICE_H
#define MAILING_LIST_MGR_SERVICE_H

#include "PyService.h"

class MailingListMgrService
: public PyService
{
public:
    MailingListMgrService(PyServiceMgr *mgr);
    virtual ~MailingListMgrService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetJoinedLists)
    PyCallable_DECL_CALL(Create)
    PyCallable_DECL_CALL(Join)
    PyCallable_DECL_CALL(Leave)
    PyCallable_DECL_CALL(Delete)
    PyCallable_DECL_CALL(KickMembers)
    PyCallable_DECL_CALL(GetMembers)
    PyCallable_DECL_CALL(SetEntityAccess)
    PyCallable_DECL_CALL(ClearEntityAccess)
    PyCallable_DECL_CALL(SetMembersMuted)
    PyCallable_DECL_CALL(SetMembersOperator)
    PyCallable_DECL_CALL(SetMembersClear)
    PyCallable_DECL_CALL(SetDefaultAccess)
    PyCallable_DECL_CALL(GetInfo)
    PyCallable_DECL_CALL(GetSettings)
    PyCallable_DECL_CALL(GetWelcomeMail)
    PyCallable_DECL_CALL(SaveWelcomeMail)
    PyCallable_DECL_CALL(SendWelcomeMail)
    PyCallable_DECL_CALL(ClearWelcomeMail)
};

#endif /* MAILING_LIST_MGR_SERVICE_H */
