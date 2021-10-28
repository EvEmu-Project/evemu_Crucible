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
    Author:        Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/CalendarDB.h"
#include "system/CalendarMgrService.h"
#include "packets/Calendar.h"

PyCallable_Make_InnerDispatcher(CalendarMgrService)

CalendarMgrService::CalendarMgrService(PyServiceMgr *mgr)
: PyService(mgr, "calendarMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CalendarMgrService, CreatePersonalEvent);
    PyCallable_REG_CALL(CalendarMgrService, CreateCorporationEvent);
    PyCallable_REG_CALL(CalendarMgrService, CreateAllianceEvent);
    PyCallable_REG_CALL(CalendarMgrService, UpdateEventParticipants);
    PyCallable_REG_CALL(CalendarMgrService, EditPersonalEvent);
    PyCallable_REG_CALL(CalendarMgrService, EditCorporationEvent);
    PyCallable_REG_CALL(CalendarMgrService, EditAllianceEvent);
    PyCallable_REG_CALL(CalendarMgrService, GetResponsesForCharacter);
    PyCallable_REG_CALL(CalendarMgrService, SendEventResponse);
    PyCallable_REG_CALL(CalendarMgrService, DeleteEvent);
    PyCallable_REG_CALL(CalendarMgrService, GetResponsesToEvent);
}

CalendarMgrService::~CalendarMgrService()
{
    delete m_dispatch;
}

PyResult CalendarMgrService::Handle_GetResponsesForCharacter(PyCallArgs& call) {
    return CalendarDB::GetResponsesForCharacter(call.client->GetCharacterID());
}

PyResult CalendarMgrService::Handle_GetResponsesToEvent(PyCallArgs& call)
{
    Call_TwoIntegerArgs args;   //(eventID, ownerID)
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return CalendarDB::GetResponsesToEvent(args.arg1); // eventID
}

PyResult CalendarMgrService::Handle_DeleteEvent( PyCallArgs& call )
{
    Call_TwoIntegerArgs args;   //(eventID, ownerID)
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CalendarDB::DeleteEvent(args.arg1); // eventID

    // Calendar must be reloaded or the event won't actually show as deleted.
    call.client->SendNotification("OnReloadCalendar", "charid", new PyTuple(0), false);

    return nullptr;
}

PyResult CalendarMgrService::Handle_SendEventResponse( PyCallArgs& call )
{
    Call_SendEventResponse args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    CalendarDB::SaveEventResponse(call.client->GetCharacterID(), args);

    // if this is an invitation, update calendar for non-denial responses

    return nullptr;
}

PyResult CalendarMgrService::Handle_CreatePersonalEvent( PyCallArgs& call )
{
    // newEventID = self.calendarMgr.CreatePersonalEvent(dateTime, duration, title, description, important, invitees)

    sLog.Cyan( "CalendarMgrService::Handle_CreatePersonalEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_CreateEventWithInvites args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // returns eventID
    return CalendarDB::SaveNewEvent(call.client->GetCharacterID(), args);
}

PyResult CalendarMgrService::Handle_CreateCorporationEvent( PyCallArgs& call )
{
    Call_CreateEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // returns eventID
    return CalendarDB::SaveNewEvent(call.client->GetCorporationID(), call.client->GetCharacterID(), args);
}

PyResult CalendarMgrService::Handle_CreateAllianceEvent( PyCallArgs& call )
{
    Call_CreateEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // returns eventID
    return CalendarDB::SaveNewEvent(call.client->GetAllianceID(), call.client->GetCharacterID(), args);
}

PyResult CalendarMgrService::Handle_EditPersonalEvent( PyCallArgs& call )
{
    //self.calendarMgr.EditPersonalEvent(eventID, oldDateTime, dateTime, duration, title, description, important)

    sLog.Cyan( "CalendarMgrService::Handle_EditPersonalEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_EditEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    return nullptr;
}

PyResult CalendarMgrService::Handle_EditCorporationEvent( PyCallArgs& call )
{
    // self.calendarMgr.EditCorporationEvent(eventID, oldDateTime, dateTime, duration, title, description, important)

    sLog.Cyan( "CalendarMgrService::Handle_EditCorporationEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_EditEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    return nullptr;
}

PyResult CalendarMgrService::Handle_EditAllianceEvent( PyCallArgs& call )
{
    //self.calendarMgr.EditAllianceEvent(eventID, oldDateTime, dateTime, duration, title, description, important)

    sLog.Cyan( "CalendarMgrService::Handle_EditAllianceEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_EditEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    return nullptr;
}

PyResult CalendarMgrService::Handle_UpdateEventParticipants( PyCallArgs& call )
{
    // self.calendarMgr.UpdateEventParticipants(eventID, charsToAdd, charsToRemove)

    sLog.Cyan( "CalendarMgrService::Handle_UpdateEventParticipants()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_UpdateEventParticipants args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    CalendarDB::UpdateEventParticipants(args);

    //  this will need to update invitees and inform them of the invitation
    // their calendar is updated based on their response (SendEventResponse)

    return nullptr;
}

