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

#ifndef MAILING_LIST_MGR_SERVICE_H
#define MAILING_LIST_MGR_SERVICE_H

#include "services/Service.h"
#include "mail/MailDB.h"

class MailingListMgrService : public Service <MailingListMgrService>
{
public:
    MailingListMgrService();

protected:
    MailDB m_db;

    PyResult GetJoinedLists(PyCallArgs& call);
    PyResult Create(PyCallArgs& call, PyWString* name, PyInt* defaultAccess, PyInt* defaultMemberAccess, std::optional<PyInt*> mailCost);
    PyResult Join(PyCallArgs& call, PyRep* listName);
    PyResult Leave(PyCallArgs& call, PyInt* listID);
    PyResult Delete(PyCallArgs& call, PyInt* listID);
    PyResult KickMembers(PyCallArgs& call, PyInt* listID, PyList* memberIDs);
    PyResult GetMembers(PyCallArgs& call, PyInt* listID);
    PyResult SetEntityAccess(PyCallArgs& call, PyInt* listID, PyInt* entityID, PyInt* access);
    PyResult ClearEntityAccess(PyCallArgs& call, PyInt* listID, PyInt* entityID);
    PyResult SetMembersMuted(PyCallArgs& call, PyInt* listID, PyList* memberIDs);
    PyResult SetMembersOperator(PyCallArgs& call, PyInt* listID, PyList* memberIDs);
    PyResult SetMembersClear(PyCallArgs& call, PyInt* listID, PyList* memberIDs);
    PyResult SetDefaultAccess(PyCallArgs& call, PyInt* listID, PyInt* defaultAccess, PyInt* defaultMemberAccess, std::optional<PyInt*> mailCost);
    PyResult GetInfo(PyCallArgs& call, PyInt* listID);
    PyResult GetSettings(PyCallArgs& call, PyInt* listID);
    PyResult GetWelcomeMail(PyCallArgs& call, PyInt* listID);
    PyResult SaveWelcomeMail(PyCallArgs& call, PyInt* listID, PyWString* title, PyWString* body);
    PyResult SendWelcomeMail(PyCallArgs& call, PyInt* listID, PyWString* title, PyWString* body);
    PyResult ClearWelcomeMail(PyCallArgs& call, PyInt* listID);
};

#endif /* MAILING_LIST_MGR_SERVICE_H */
