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
    Author:     caytchen
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "mail/NotificationMgrService.h"

PyCallable_Make_InnerDispatcher(NotificationMgrService)

// this service is part of mail and used with the 'notifications' tab of mail window

NotificationMgrService::NotificationMgrService(PyServiceMgr* mgr)
: PyService(mgr, "notificationMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(NotificationMgrService, GetByGroupID);
    PyCallable_REG_CALL(NotificationMgrService, GetUnprocessed);
    PyCallable_REG_CALL(NotificationMgrService, MarkGroupAsProcessed);
    PyCallable_REG_CALL(NotificationMgrService, MarkAllAsProcessed);
    PyCallable_REG_CALL(NotificationMgrService, MarkAsProcessed);
    PyCallable_REG_CALL(NotificationMgrService, DeleteGroupNotifications);
    PyCallable_REG_CALL(NotificationMgrService, DeleteAllNotifications);
    PyCallable_REG_CALL(NotificationMgrService, DeleteNotifications);
}

NotificationMgrService::~NotificationMgrService() {
    delete m_dispatch;
}

PyResult NotificationMgrService::Handle_GetByGroupID(PyCallArgs &call)
{

    sLog.White("NotificationMgrService", "Handle_GetByGroupID() size=%u", call.tuple->size() );
    call.Dump(MAIL__DUMP);
    /*
            [PyString "GetByGroupID"]
            [PyTuple 1 items]
              [PyInt 9]
        {returns}
        [PyObjectEx Type2]
          [PyTuple 2 items]
            [PyTuple 1 items]
              [PyToken dbutil.CRowset]
            [PyDict 1 kvp]
              [PyString "header"]
              [PyObjectEx Normal]
                [PyTuple 2 items]
                  [PyToken blue.DBRowDescriptor]
                  [PyTuple 1 items]
                    [PyTuple 8 items]
                      [PyTuple 2 items]
                        [PyString "notificationID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "typeID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "senderID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "receiverID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "processed"]
                        [PyInt 11]
                      [PyTuple 2 items]
                        [PyString "created"]
                        [PyInt 64]
                      [PyTuple 2 items]
                        [PyString "data"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "deleted"]
                        [PyInt 11]
              */
              // yes, i have the packet, but wtf is this?
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    int groupID = args.arg;
    return new PyTuple(0);
}

PyResult NotificationMgrService::Handle_GetUnprocessed(PyCallArgs &call)
{
    // called when mail window's notifications tab opened
    // see /journal/GetUnprocessed for info..

    return new PyTuple(0);
}

PyResult NotificationMgrService::Handle_MarkGroupAsProcessed(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
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
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyRep* notificationsList = args.arg;
    return NULL;
}

PyResult NotificationMgrService::Handle_DeleteGroupNotifications(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
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
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    
    PyRep* notificationsIDs = args.arg;
    return NULL;
}
