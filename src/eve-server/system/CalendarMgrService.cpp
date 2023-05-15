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


#include "system/CalendarDB.h"
#include "system/CalendarMgrService.h"


CalendarMgrService::CalendarMgrService() :
    Service("calendarMgr", eAccessLevel_Character)
{
    this->Add("CreatePersonalEvent", &CalendarMgrService::CreatePersonalEvent);
    this->Add("CreateCorporationEvent", &CalendarMgrService::CreateCorporationEvent);
    this->Add("CreateAllianceEvent", &CalendarMgrService::CreateAllianceEvent);
    this->Add("UpdateEventParticipants", &CalendarMgrService::UpdateEventParticipants);
    this->Add("EditPersonalEvent", &CalendarMgrService::EditPersonalEvent);
    this->Add("EditCorporationEvent", &CalendarMgrService::EditCorporationEvent);
    this->Add("EditAllianceEvent", &CalendarMgrService::EditAllianceEvent);
    this->Add("GetResponsesForCharacter", &CalendarMgrService::GetResponsesForCharacter);
    this->Add("SendEventResponse", &CalendarMgrService::SendEventResponse);
    this->Add("DeleteEvent", &CalendarMgrService::DeleteEvent);
    this->Add("GetResponsesToEvent", &CalendarMgrService::GetResponsesToEvent);
}

PyResult CalendarMgrService::GetResponsesForCharacter(PyCallArgs& call) {
    return CalendarDB::GetResponsesForCharacter(call.client->GetCharacterID());
}

PyResult CalendarMgrService::GetResponsesToEvent(PyCallArgs& call, PyInt* eventID, PyInt* ownerID)
{
    return CalendarDB::GetResponsesToEvent(eventID->value()); // eventID
}

PyResult CalendarMgrService::DeleteEvent(PyCallArgs& call, PyInt* eventID, PyInt* ownerID)
{
    CalendarDB::DeleteEvent(eventID->value()); // eventID

    // Calendar must be reloaded or the event won't actually show as deleted.
    call.client->SendNotification("OnReloadCalendar", "charid", new PyTuple(0), false);

    return nullptr;
}

PyResult CalendarMgrService::SendEventResponse(PyCallArgs& call, PyInt* eventID, PyInt* ownerID, PyInt* response)
{
    CalendarDB::SaveEventResponse(call.client->GetCharacterID(), eventID->value(), response->value());

    // if this is an invitation, update calendar for non-denial responses

    return nullptr;
}

PyResult CalendarMgrService::CreatePersonalEvent(PyCallArgs& call, PyLong* dateTime, PyInt* duration, PyWString* title, PyWString* description, PyRep* important, PyList* invitees)
{
    // newEventID = self.calendarMgr.CreatePersonalEvent(dateTime, duration, title, description, important, invitees)

    sLog.Cyan( "CalendarMgrService::Handle_CreatePersonalEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    // TODO: update this to not use xmlpktgen, too many changes just for the services update
    Call_CreateEventWithInvites args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // returns eventID
    return CalendarDB::SaveNewEvent(call.client->GetCharacterID(), args);
}

PyResult CalendarMgrService::CreateCorporationEvent(PyCallArgs& call, PyLong* dateTime, PyInt* duration, PyWString* title, PyWString* description, PyRep* important)
{
    // TODO: update this to not use xmlpktgen, too many changes just for the services update
    Call_CreateEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // returns eventID
    return CalendarDB::SaveNewEvent(call.client->GetCorporationID(), call.client->GetCharacterID(), args);
}

PyResult CalendarMgrService::CreateAllianceEvent(PyCallArgs& call, PyLong* dateTime, PyInt* duration, PyWString* title, PyWString* description, PyRep* important)
{
    // TODO: update this to not use xmlpktgen, too many changes just for the services update
    Call_CreateEvent args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    // returns eventID
    return CalendarDB::SaveNewEvent(call.client->GetAllianceID(), call.client->GetCharacterID(), args);
}

PyResult CalendarMgrService::EditPersonalEvent(PyCallArgs& call, PyInt* eventID, PyLong* oldDateTime, PyLong* dateTime, PyInt* duration, PyWString* title, PyWString* description, PyRep* important)
{
    //self.calendarMgr.EditPersonalEvent(eventID, oldDateTime, dateTime, duration, title, description, important)

    sLog.Cyan( "CalendarMgrService::Handle_EditPersonalEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult CalendarMgrService::EditCorporationEvent(PyCallArgs& call, PyInt* eventID, PyLong* oldDateTime, PyLong* dateTime, PyInt* duration, PyWString* title, PyWString* description, PyRep* important)
{
    // self.calendarMgr.EditCorporationEvent(eventID, oldDateTime, dateTime, duration, title, description, important)

    sLog.Cyan( "CalendarMgrService::Handle_EditCorporationEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult CalendarMgrService::EditAllianceEvent(PyCallArgs& call, PyInt* eventID, PyLong* oldDateTime, PyLong* dateTime, PyInt* duration, PyWString* title, PyWString* description, PyRep* important)
{
    //self.calendarMgr.EditAllianceEvent(eventID, oldDateTime, dateTime, duration, title, description, important)

    sLog.Cyan( "CalendarMgrService::Handle_EditAllianceEvent()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult CalendarMgrService::UpdateEventParticipants(PyCallArgs& call, PyInt* eventID, PyList* charsToAdd, PyList* charsToRemove)
{
    // self.calendarMgr.UpdateEventParticipants(eventID, charsToAdd, charsToRemove)

    sLog.Cyan( "CalendarMgrService::Handle_UpdateEventParticipants()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    // TODO: implement this
    CalendarDB::UpdateEventParticipants();

    //  this will need to update invitees and inform them of the invitation
    // their calendar is updated based on their response (SendEventResponse)

    return nullptr;
}

