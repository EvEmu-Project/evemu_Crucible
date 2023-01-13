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

#include "../../eve-common/EVE_Calendar.h"
#include "system/CalendarDB.h"


void CalendarDB::DeleteEvent(uint32 eventID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE sysCalendarEvents SET isDeleted = 1 WHERE eventID = %u", eventID);
}

// for personal char events
PyRep* CalendarDB::SaveNewEvent(uint32 ownerID, Call_CreateEventWithInvites& args)
{
    EvE::TimeParts data = EvE::TimeParts();
    data = GetTimeParts(args.startDateTime);

    DBerror err;
    if (!args.invitees->empty()) {
        bool comma(false);
        std::ostringstream str;
        PyList* list(args.invitees->AsList());
        PyList::const_iterator itr = list->begin(), end = list->end();
        while (itr != end) {
            if (comma) {
                str << ",";
            } else {
                comma = true;
            }
            str << *itr;
            ++itr;
        }

        sDatabase.RunQuery(err,
                "INSERT INTO `sysCalendarInvitees`(`eventID`, `inviteeList`)"
                " VALUES %s", str.str().c_str());
    }

    uint32 eventID(0);
    if (args.duration) {
        if (!sDatabase.RunQueryLID(err, eventID,
            "INSERT INTO sysCalendarEvents(ownerID, creatorID, eventDateTime, eventDuration, importance,"
            " eventTitle, eventText, flag, month, year)"
            " VALUES (%u, %u, %li, %u, %u, '%s', '%s', %u, %u, %u)",
            ownerID, ownerID, args.startDateTime, args.duration, args.important, args.title.c_str(),
            args.description.c_str(), Calendar::Flag::Personal, data.month, data.year))
        {
            codelog(DATABASE__ERROR, "Error in SaveNewEvent query: %s", err.c_str());
            return PyStatic.NewZero();
        }
    } else {
        if (!sDatabase.RunQueryLID(err, eventID,
            "INSERT INTO sysCalendarEvents(ownerID, creatorID, eventDateTime, importance,"
            " eventTitle, eventText, flag, month, year)"
            " VALUES (%u, %u, %li, %u, '%s', '%s', %u, %u, %u)",
            ownerID, ownerID, args.startDateTime, args.important, args.title.c_str(),
            args.description.c_str(), Calendar::Flag::Personal, data.month, data.year))
        {
            codelog(DATABASE__ERROR, "Error in SaveNewEvent query: %s", err.c_str());
            return PyStatic.NewZero();
        }
    }

    return new PyInt(eventID);
}

// for corp/alliance events
PyRep* CalendarDB::SaveNewEvent(uint32 ownerID, uint32 creatorID, Call_CreateEvent &args)
{
    uint8 flag(Calendar::Flag::Invalid);
    if (IsCharacterID(ownerID)) {
     flag = Calendar::Flag::Personal;
    } else if (IsCorp(ownerID)) { // this would also be Automated for pos fuel
        flag = Calendar::Flag::Corp;
    } else if (IsAlliance(ownerID)) {
        flag = Calendar::Flag::Alliance;
    } else if (ownerID == 1) {
        flag = Calendar::Flag::CCP;
    } else {
        flag = Calendar::Flag::Automated;
    }

    EvE::TimeParts data = EvE::TimeParts();
    data = GetTimeParts(args.startDateTime);

    uint32 eventID(0);
    DBerror err;
    if (args.duration) {
        if (!sDatabase.RunQueryLID(err, eventID,
            "INSERT INTO sysCalendarEvents(ownerID, creatorID, eventDateTime, eventDuration, importance,"
            " eventTitle, eventText, flag, month, year)"
            " VALUES (%u, %u, %li, %u, %u, '%s', '%s', %u, %u, %u)",
            ownerID, creatorID, args.startDateTime, args.duration, args.important,
            args.title.c_str(), args.description.c_str(), flag, data.month, data.year))
        {
            codelog(DATABASE__ERROR, "Error in SaveNewEvent query: %s", err.c_str());
            return PyStatic.NewZero();
        }
    } else {
        if (!sDatabase.RunQueryLID(err, eventID,
            "INSERT INTO sysCalendarEvents(ownerID, creatorID, eventDateTime, importance,"
            " eventTitle, eventText, flag, month, year)"
            " VALUES (%u, %u, %li, %u, '%s', '%s', %u, %u, %u)",
            ownerID, creatorID, args.startDateTime, args.important,
            args.title.c_str(), args.description.c_str(), flag, data.month, data.year))
        {
            codelog(DATABASE__ERROR, "Error in SaveNewEvent query: %s", err.c_str());
            return PyStatic.NewZero();
        }
    }

    return new PyInt(eventID);
}

// for system/auto events
uint32 CalendarDB::SaveSystemEvent(uint32 ownerID, uint32 creatorID, int64 startDateTime, uint8 autoEventType,
                                   std::string title, std::string description, bool important/*false*/)
{
    EvE::TimeParts data = EvE::TimeParts();
    data = GetTimeParts(startDateTime);

    uint32 eventID(0);
    DBerror err;
    sDatabase.RunQueryLID(err, eventID,
        "INSERT INTO sysCalendarEvents(ownerID, creatorID, eventDateTime, autoEventType,"
        " eventTitle, eventText, flag, month, year, importance)"
        " VALUES (%u, %u, %li, %u, '%s', '%s', %u, %u, %u, %u)",
        ownerID, creatorID, startDateTime, autoEventType, title.c_str(), description.c_str(),
        Calendar::Flag::Automated, data.month, data.year, important?1:0);

    return eventID;
}


PyRep* CalendarDB::GetEventList(uint32 ownerID, uint32 month, uint32 year)
{
    if (ownerID == 0)
        return nullptr;

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT eventID, ownerID, eventDateTime, dateModified, eventDuration, importance, eventTitle, flag,"
        " autoEventType, isDeleted"
        " FROM sysCalendarEvents WHERE ownerID = %u AND month = %u AND year = %u", ownerID, month, year))
    {
        codelog(DATABASE__ERROR, "Error in GetEventList query: %s", res.error.c_str());
        return nullptr;
    }

    if (res.GetRowCount() < 1)
        return nullptr;

    DBResultRow row;
    PyList* list = new PyList();
    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
            dict->SetItemString("eventID",              new PyInt(row.GetInt(0)));
            dict->SetItemString("ownerID",              new PyInt(row.GetInt(1)));
            dict->SetItemString("eventDateTime",        new PyLong(row.GetInt64(2)));
            dict->SetItemString("dateModified",         row.IsNull(3) ? PyStatic.NewNone() : new PyLong(row.GetInt64(3)));
            dict->SetItemString("eventDuration",        row.IsNull(4) ? PyStatic.NewNone() : new PyInt(row.GetInt(4)));
            dict->SetItemString("importance",           new PyBool(row.GetBool(5)));
            dict->SetItemString("eventTitle",           new PyString(row.GetText(6)));
            dict->SetItemString("flag",                 new PyInt(row.GetInt(7)));
            // client patch to allow non-corp automated events for ram jobs
            if (row.GetInt(7) == Calendar::Flag::Automated)
                dict->SetItemString("autoEventType",    new PyInt(row.GetInt(8)));
            dict->SetItemString("isDeleted",            new PyBool(row.GetBool(9)));
        list->AddItem(new PyObject("util.KeyVal",       dict));
    }

    return list;
}

PyRep* CalendarDB::GetEventDetails(uint32 eventID)
{
    if (eventID == 0)
        return nullptr;

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT ownerID, creatorID, eventText"
        " FROM sysCalendarEvents WHERE eventID = %u", eventID))
    {
        codelog(DATABASE__ERROR, "Error in GetEventList query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    PyDict* dict = new PyDict();
        dict->SetItemString("eventID",          new PyInt(eventID));
        dict->SetItemString("ownerID",          new PyInt(row.GetInt(0)));
        dict->SetItemString("creatorID",        new PyInt(row.GetInt(1)));
        dict->SetItemString("eventText",        new PyString(row.GetText(2)));

    return new PyObject("util.KeyVal", dict);
}

void CalendarDB::SaveEventResponse(uint32 charID, uint32 eventID, uint32 response)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO `sysCalendarResponses`(`eventID`, `charID`, `response`)"
        " VALUES (%u, %u, %u)", eventID, charID, response);
}

PyRep* CalendarDB::GetResponsesForCharacter(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,"SELECT eventID, response FROM sysCalendarResponses WHERE charID = %u", charID)) {
        codelog(DATABASE__ERROR, "Error in GetEventList query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    PyList *list = new PyList();
    while (res.GetRow(row)) {
        // list char response for each event
        PyDict* dict = new PyDict();
            dict->SetItemString("eventID", new PyInt(row.GetInt(0)));
            dict->SetItemString("status",  new PyInt(row.GetInt(1)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

PyRep* CalendarDB::GetResponsesToEvent(uint32 eventID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,"SELECT charID, response FROM sysCalendarResponses WHERE eventID = %u", eventID)) {
        codelog(DATABASE__ERROR, "Error in GetEventList query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    PyList *list = new PyList();
    while (res.GetRow(row)) {
        // list char response for each event
        PyDict* dict = new PyDict();
            dict->SetItemString("characterID", new PyInt(row.GetInt(0)));
            dict->SetItemString("status",  new PyInt(row.GetInt(1)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

void CalendarDB::UpdateEventParticipants(Call_UpdateEventParticipants& args)
{
    DBerror err;
}



