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
    Author:     Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/CalendarDB.h"
#include "system/CalendarProxy.h"

PyCallable_Make_InnerDispatcher(CalendarProxy)

CalendarProxy::CalendarProxy(PyServiceMgr *mgr)
: PyService(mgr, "calendarProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CalendarProxy, GetEventList);
    PyCallable_REG_CALL(CalendarProxy, GetEventDetails);
}

CalendarProxy::~CalendarProxy()
{
    delete m_dispatch;
}


PyResult CalendarProxy::Handle_GetEventList( PyCallArgs& call )
{
    Call_TwoIntegerArgs args;   //(month, year)
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyList *list = new PyList();
    PyRep* res(nullptr);

    // get system events
    res = CalendarDB::GetEventList(ownerSystem, args.arg1, args.arg2);
    if (res != nullptr)
        list->AddItem(res);

    // get personal events
    res = CalendarDB::GetEventList(call.client->GetCharacterID(), args.arg1, args.arg2);
    if (res != nullptr)
        list->AddItem(res);

    // get corp events
    res = CalendarDB::GetEventList(call.client->GetCorporationID(), args.arg1, args.arg2);
    if (res != nullptr)
        list->AddItem(res);

    // get alliance events
    if (IsAlliance(call.client->GetAllianceID())) {
        res = CalendarDB::GetEventList(call.client->GetAllianceID(), args.arg1, args.arg2);
        if (res != nullptr)
            list->AddItem(res);
    }

    if (list->empty())
        list->AddItem(PyStatic.NewNone());

    return list;
}

PyResult CalendarProxy::Handle_GetEventDetails( PyCallArgs& call )
{
    // self.eventDetails[eventID] = self.GetCalendarProxy().GetEventDetails(eventID, ownerID)
    Call_TwoIntegerArgs args;   //(eventID, ownerID)
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return CalendarDB::GetEventDetails(args.arg1);
}

