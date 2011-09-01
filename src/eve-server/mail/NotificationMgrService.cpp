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
    Author:     caytchen
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(NotificationMgrService)

NotificationMgrService::NotificationMgrService(PyServiceMgr* mgr)
: PyService(mgr, "notificationMgr"), m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(NotificationMgrService, GetByGroupID)
	PyCallable_REG_CALL(NotificationMgrService, GetUnprocessed)
	PyCallable_REG_CALL(NotificationMgrService, MarkGroupAsProcessed)
	PyCallable_REG_CALL(NotificationMgrService, MarkAllAsProcessed)
	PyCallable_REG_CALL(NotificationMgrService, MarkAsProcessed)
	PyCallable_REG_CALL(NotificationMgrService, DeleteGroupNotifications)
	PyCallable_REG_CALL(NotificationMgrService, DeleteAllNotifications)
	PyCallable_REG_CALL(NotificationMgrService, DeleteNotifications)
}

NotificationMgrService::~NotificationMgrService() {
	delete m_dispatch;
}

PyResult NotificationMgrService::Handle_GetByGroupID(PyCallArgs &call)
{
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for GetByGroupID call");
		return NULL;
	}
	int groupID = args.arg;
	return new PyTuple(0);
}

PyResult NotificationMgrService::Handle_GetUnprocessed(PyCallArgs &call)
{
	return new PyTuple(0);
}

PyResult NotificationMgrService::Handle_MarkGroupAsProcessed(PyCallArgs &call)
{
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for MarkGroupAsProcessed call");
		return NULL;
	}
	int groupID = args.arg;
	return NULL;
}

PyResult NotificationMgrService::Handle_MarkAllAsProcessed(PyCallArgs &call)
{
	return NULL;
}

PyResult NotificationMgrService::Handle_MarkAsProcessed(PyCallArgs &call)
{
	Call_SingleArg args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for MarkAsProcessed call");
		return NULL;
	}
	PyRep* notificationsList = args.arg;
	return NULL;
}

PyResult NotificationMgrService::Handle_DeleteGroupNotifications(PyCallArgs &call)
{
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for DeleteGroupNotifications call");
		return NULL;
	}
	int groupID = args.arg;
	return NULL;
}

PyResult NotificationMgrService::Handle_DeleteAllNotifications(PyCallArgs &call)
{
	return NULL;
}

PyResult NotificationMgrService::Handle_DeleteNotifications(PyCallArgs &call)
{
	Call_SingleArg args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for DeleteNotifications call");
		return NULL;
	}
	PyRep* notificationsIDs = args.arg;
	return NULL;
}