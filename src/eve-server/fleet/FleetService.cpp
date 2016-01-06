/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Allan
*/

//work in progress

#include "eve-server.h"
#include "fleet/FleetService.h"

//may need to adjust this later for persistant fleets
uint32 FleetService::m_fleetID = 10;

//  Manager class functions and methods...

PyObject* FleetService::CreateFleet(Client *pClient)
{
    uint32 fleetID = m_fleetID, wingID = fleetID + 1, squadID = wingID + 1;
    FleetMemberInfo fleet;

    fleet.fleetID = fleetID;        //this is also lsc channel #
    fleet.wingID = wingID;
    fleet.squadID = squadID;
    fleet.fleetRole = fleetRoleLeader;
    fleet.fleetBooster = fleetBoosterFleet;
    fleet.fleetJob = fleetJobCreator;

    //call updates on fleet session data
    pClient->GetChar().get()->SetFleetData(fleet);

    //update the fleet list with new fleet id
    m_fleets.push_back(fleetID);
    // update the fleet member map with new member for this fleet.
    m_fleetMembers.emplace(fleetID, pClient->GetChar().get());
    m_fleetID++;

    PyList* list = new PyList;
    list->AddItemInt(pClient->GetChar()->GetSkillLevel(skillLeadership, true));     //skill in Leadership
    list->AddItemInt(pClient->GetChar()->GetSkillLevel(skillWingCommand, true));     //skill in Wing Command
    list->AddItemInt(pClient->GetChar()->GetSkillLevel(skillFleetCommand, true));     //skill in Fleet Command

    PyDict* dict = new PyDict;
    //dict->SetItemString( "charID", new PyInt(pClient->GetCharacterID()) );
    dict->SetItemString( "solarSystemID", new PyInt(pClient->GetSystemID()) );
    dict->SetItemString( "shipTypeID", new PyInt(pClient->GetShip()->typeID()) );
    dict->SetItemString( "role", new PyInt(fleet.fleetRole) );
    dict->SetItemString( "job", new PyInt(fleet.fleetJob) );
    //dict->SetItemString( "clientID", new PyInt(pClient->GetCharacterID()) );
    dict->SetItemString( "timestamp", new PyLong(Win32TimeNow()) );
    dict->SetItemString( "wingID", new PyInt(fleet.wingID) );
    dict->SetItemString( "squadID", new PyInt(fleet.squadID) );
    dict->SetItemString( "roleBooster", new PyInt(fleet.fleetBooster) );
    dict->SetItemString( "skills", list );
    //dict->SetItemString( "Init", new PyInt(fleetID) );

    PyObject* res = new PyObject( "util.KeyVal", dict );
    return res;
}

PyObject* FleetService::Init(Client *pClient) {

    return NULL;
}

PyObject* FleetService::CreateWing(Client *pClient)
{

    return NULL;
}


PyObject* FleetService::CreateSquad(Client *pClient)
{

    return NULL;
}

PyRep* FleetService::GetAvailableFleets()
{
    //  more to this than just a list of fleetIDs
    PyList* list = new PyList;
    for (auto p : m_fleets)
        list->AddItemInt(p);

    return list;
}

