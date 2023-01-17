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
    Author:     caytchen
*/

#include "eve-server.h"


#include "mail/NotificationMgrService.h"

// this service is part of mail and used with the 'notifications' tab of mail window

NotificationMgrService::NotificationMgrService() :
    Service("notificationMgr")
{
    this->Add("GetByGroupID", &NotificationMgrService::GetByGroupID);
    this->Add("GetUnprocessed", &NotificationMgrService::GetUnprocessed);
    this->Add("MarkGroupAsProcessed", &NotificationMgrService::MarkGroupAsProcessed);
    this->Add("MarkAllAsProcessed", &NotificationMgrService::MarkAllAsProcessed);
    this->Add("MarkAsProcessed", &NotificationMgrService::MarkAsProcessed);
    this->Add("DeleteGroupNotifications", &NotificationMgrService::DeleteGroupNotifications);
    this->Add("DeleteAllNotifications", &NotificationMgrService::DeleteAllNotifications);
    this->Add("DeleteNotifications", &NotificationMgrService::DeleteNotifications);
}

PyResult NotificationMgrService::GetByGroupID(PyCallArgs &call, PyInt* groupID)
{

    sLog.White("NotificationMgrService", "Handle_GetByGroupID() size=%li", call.tuple->size());
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
    return new PyTuple(0);
}

PyResult NotificationMgrService::GetUnprocessed(PyCallArgs &call)
{
    // called when mail window's notifications tab opened
    // see /journal/GetUnprocessed for info..

    return new PyTuple(0);
}

PyResult NotificationMgrService::MarkGroupAsProcessed(PyCallArgs &call, PyInt* groupID)
{
    return nullptr;
}

PyResult NotificationMgrService::MarkAllAsProcessed(PyCallArgs &call)
{
    return nullptr;
}

PyResult NotificationMgrService::MarkAsProcessed(PyCallArgs &call, PyList* notificationIDsToMarkAsRead)
{
    return nullptr;
}

PyResult NotificationMgrService::DeleteGroupNotifications(PyCallArgs &call, PyInt* groupID)
{
    return nullptr;
}

PyResult NotificationMgrService::DeleteAllNotifications(PyCallArgs &call)
{
    return nullptr;
}

PyResult NotificationMgrService::DeleteNotifications(PyCallArgs &call, PyList* notificatinIDs)
{
    return nullptr;
}
