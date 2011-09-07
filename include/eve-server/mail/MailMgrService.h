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
	Author:		Luck, caytchen
*/

#ifndef MAIL_MGR_SERIVCE_H
#define MAIL_MGR_SERVICE_H

#include "PyService.h"

class MailDB;

class MailMgrService : public PyService
{
public:
	MailMgrService(PyServiceMgr *mgr);
	virtual ~MailMgrService();

protected:
	MailDB* m_db;

	class Dispatcher;
	Dispatcher *const m_dispatch;

	PyCallable_DECL_CALL(PrimeOwners);
	PyCallable_DECL_CALL(SyncMail);
	PyCallable_DECL_CALL(GetMailHeaders);
	PyCallable_DECL_CALL(MoveToTrash);
	PyCallable_DECL_CALL(MoveFromTrash);
	PyCallable_DECL_CALL(MarkAsUnread);
	PyCallable_DECL_CALL(MarkAsRead);
	PyCallable_DECL_CALL(MoveAllToTrash);
	PyCallable_DECL_CALL(MoveToTrashByLabel);
	PyCallable_DECL_CALL(MoveToTrashByList);
	PyCallable_DECL_CALL(MarkAllAsUnread);
	PyCallable_DECL_CALL(MarkAsUnreadByLabel);
	PyCallable_DECL_CALL(MarkAsUnreadByList);
	PyCallable_DECL_CALL(MarkAllAsRead);
	PyCallable_DECL_CALL(MarkAsReadByLabel);
	PyCallable_DECL_CALL(MarkAsReadByList);
	PyCallable_DECL_CALL(MoveAllFromTrash);
	PyCallable_DECL_CALL(EmptyTrash);
	PyCallable_DECL_CALL(DeleteMail);
	PyCallable_DECL_CALL(GetBody);
	PyCallable_DECL_CALL(AssignLabels);
	PyCallable_DECL_CALL(RemoveLabels);

	// implemented
	PyCallable_DECL_CALL(GetLabels);
	PyCallable_DECL_CALL(EditLabel);
	PyCallable_DECL_CALL(CreateLabel);
	PyCallable_DECL_CALL(DeleteLabel);
};


#endif /* MAIL_MGR_SERVICE_H */