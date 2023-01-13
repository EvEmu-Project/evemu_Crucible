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
    Author:        Zhur
    Rewrite:    Allan
*/

/*
 * AgentMgr bound(agentID):
 *  -> DoAction(actionID or None)
 *  -> WarpToLocation(locationType, locationNumber, warpRange, is_gang)
 *
 *   Also sent an OnRemoteMessage(AgtMissionOfferWarning)
 *
 *   and various OnAgentMissionChange()
 *
*/

/*
 * # Agent Logging:
 * AGENT__ERROR
 * AGENT__WARNING
 * AGENT__MESSAGE
 * AGENT__DEBUG
 * AGENT__INFO
 * AGENT__TRACE
 * AGENT__DUMP
 * AGENT__RSP_DUMP
 */

#include "eve-server.h"

#include "EntityList.h"
#include "StaticDataMgr.h"
#include "agents/Agent.h"
#include "agents/AgentBound.h"
#include "agents/AgentMgrService.h"
#include "missions/MissionDataMgr.h"

AgentMgrService::AgentMgrService(EVEServiceManager& mgr) :
    BindableService("agentMgr", mgr)
{
    this->Add("GetAgents", &AgentMgrService::GetAgents);
    this->Add("GetCareerAgents", &AgentMgrService::GetCareerAgents);
    this->Add("GetMyJournalDetails", &AgentMgrService::GetMyJournalDetails);
    this->Add("GetSolarSystemOfAgent", &AgentMgrService::GetSolarSystemOfAgent);
    this->Add("GetMyEpicJournalDetails", &AgentMgrService::GetMyEpicJournalDetails);
}

// need a way to check created objects for client/agent combinations to avoid duplicates.
//  also need a way to check/delete released objects/agents
BoundDispatcher* AgentMgrService::BindObject(Client* client, PyRep* bindParameters) {
    if (!bindParameters->IsInt()) {
        _log(SERVICE__ERROR, "%s: Non-integer argument '%s'", client->GetName(), bindParameters->TypeString());
        return nullptr;
    }

    uint32 agentID(bindParameters->AsInt()->value());
    Agent* pAgent(sEntityList.GetAgent(agentID));
    if (pAgent == nullptr) {
        _log(AGENT__ERROR, "%s: Unable to obtain agent %u", client->GetName(), agentID);
        return nullptr;
    }

    return new AgentBound(this->GetServiceManager(), bindParameters, pAgent);
}

PyResult AgentMgrService::GetAgents(PyCallArgs &call) {
    // this is cached on client side...
    return sDataMgr.GetAgents();
}

PyResult AgentMgrService::GetSolarSystemOfAgent(PyCallArgs &call, PyInt* agentID)
{
    return sDataMgr.GetAgentSystemID(agentID->value());
}

PyResult AgentMgrService::GetMyJournalDetails(PyCallArgs &call) {
// note:  this will show mission data in journal AND "offered" msg in agent data bloc on agent tab in station

    _log(AGENT__INFO, "AgentMgrService::Handle_GetMyJournalDetails() - size=%li", call.tuple->size());
    call.Dump(AGENT__DUMP);

    /** @todo  journal details
     * found in eve/client/script/ui/shared/neocom/journal.py
     *
     *  missions = self.GetMyAgentJournalDetails()[0]
     *    missionState, importantMission, missionType, missionName, agentID, expirationTime, bookmarks, remoteOfferable, remoteCompletable = missions[i]
     *
     *    research = sm.GetService('journal').GetMyAgentJournalDetails()[1]
     *    agentID, typeID, ppd, points, level, quality, stationID = research[i]
     *
     *            self.agentjournal = sm.RemoteSvc('agentMgr').GetMyJournalDetails()
     *            for mission in self.agentjournal[0]:
     *                if mission[4] == agentID:
     *            for research in self.agentjournal[1]:
     *                if research[0] == agentID:
     */

    PyTuple *tuple = new PyTuple(2);
    //missions:
    PyList* missions = new PyList();
    std::vector<MissionOffer> data;
    sMissionDataMgr.LoadMissionOffers(call.client->GetCharacterID(), data);
    for (auto cur : data) {
        PyTuple* mData = new PyTuple(9);
        mData->SetItem(0, new PyInt(cur.stateID)); //missionState  .. these may be wrong also.
        mData->SetItem(1, new PyInt(cur.important?1:0)); //importantMission  -- integer boolean
        mData->SetItem(2, new PyString(sMissionDataMgr.GetTypeLabel(cur.typeID))); //missionTypeLabel
        mData->SetItem(3, new PyString(cur.name)); //missionName
        mData->SetItem(4, new PyInt(cur.agentID)); //agentID
        mData->SetItem(5, new PyLong(cur.expiryTime)); //expirationTime
        mData->SetItem(6, cur.bookmarks->Clone()); //bookmarks -- if populated, this is PyList of PyDicts as defined below...
        mData->SetItem(7, new PyBool(cur.remoteOfferable)); //remoteOfferable
        mData->SetItem(8, new PyBool(cur.remoteCompletable)); //remoteCompletable
        missions->AddItem(mData);
    }
    tuple->SetItem(0, missions);

    //research:
    PyList* research = new PyList();
    tuple->SetItem(1, research);

    if (is_log_enabled(AGENT__RSP_DUMP))
        tuple->Dump(AGENT__RSP_DUMP, "   ");
    return tuple;
  /*
      [PySubStream 59 bytes]
        [PyTuple 2 items]
          [PyList 1 items]
            [PyTuple 9 items]
              [PyInt 1]
              [PyInt 0]
              [PyString "Encounter"]
              [PyString "Seek and Destroy"]
              [PyInt 3010819]
              [PyIntegerVar 129495559223373466]
              [PyList 0 items]
              [PyBool False]
              [PyBool False]
          [PyList 0 items]

    [PyTuple 1 items]
      [PySubStream 53 bytes]
        [PyTuple 2 items]
          [PyList 1 items]
            [PyTuple 9 items]
              [PyInt 1]
              [PyInt 0]
              [PyString "Courier"]
              [PyString "Good Harvest"]
              [PyInt 3010819]
              [PyIntegerVar 129495553039999957]
              [PyList 0 items]
              [PyBool False]
              [PyBool False]
          [PyList 0 items]


      [PySubStream 513 bytes]
        [PyTuple 2 items]
          [PyList 1 items]
            [PyTuple 9 items]
              [PyInt 2]
              [PyInt 0]
              [PyString "Courier"]
              [PyString "Good Harvest"]
              [PyInt 3010819]
              [PyIntegerVar 129495553802043094]
              [PyList 2 items]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 15 kvp]
                    [PyString "itemID"]
                    [PyInt 60014683]
                    [PyString "typeID"]
                    [PyInt 1529]
                    [PyString "agentID"]
                    [PyInt 3010819]
                    [PyString "hint"]
                    [PyString "Objective (Pick Up) & Agent Base - Annaro VIII - Moon 4 - State War Academy School"]
                    [PyString "locationType"]
                    [PyString "objective.source"]
                    [PyString "memo"]
                    [PyString ""]
                    [PyString "created"]
                    [PyIntegerVar 129489505802043094]
                    [PyString "locationNumber"]
                    [PyInt 0]
                    [PyString "flag"]
                    [PyNone]
                    [PyString "locationID"]
                    [PyInt 30002776]
                    [PyString "ownerID"]
                    [PyInt 1661059544]
                    [PyString "y"]
                    [PyInt 0]
                    [PyString "x"]
                    [PyInt 0]
                    [PyString "solarsystemID"]
                    [PyInt 30002776]
                    [PyString "z"]
                    [PyInt 0]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 15 kvp]
                    [PyString "itemID"]
                    [PyInt 60000016]
                    [PyString "typeID"]
                    [PyInt 1531]
                    [PyString "agentID"]
                    [PyInt 3010819]
                    [PyString "hint"]
                    [PyString "Objective (Drop Off) - Tasabeshi VIII - Moon 13 - CBD Corporation Storage"]
                    [PyString "locationType"]
                    [PyString "objective.destination"]
                    [PyString "memo"]
                    [PyString ""]
                    [PyString "created"]
                    [PyIntegerVar 129489505802043094]
                    [PyString "locationNumber"]
                    [PyInt 0]
                    [PyString "flag"]
                    [PyNone]
                    [PyString "locationID"]
                    [PyInt 30002778]
                    [PyString "solarsystemID"]
                    [PyInt 30002778]
                    [PyString "ownerID"]
                    [PyInt 1661059544]
                    [PyString "y"]
                    [PyInt 0]
                    [PyString "x"]
                    [PyInt 0]
                    [PyString "z"]
                    [PyInt 0]
              [PyBool False]
              [PyBool False]
          [PyList 0 items]
    */

}


/** not handled */
PyResult AgentMgrService::GetMyEpicJournalDetails(PyCallArgs& call)
{
    //no args
  _log(AGENT__INFO, "AgentMgrBound::Handle_GetMyEpicJournalDetails() - size=%li", call.tuple->size());

    return new PyList();
}

PyResult AgentMgrService::GetCareerAgents(PyCallArgs &call)
{
  _log(AGENT__INFO, "AgentMgrBound::Handle_GetCareerAgents() - size=%li", call.tuple->size());
    call.Dump(AGENT__DUMP);

    return PyStatic.NewZero();
}

EpicArcService::EpicArcService() :
    Service("epicArcStatus")
{
    this->Add("AgentHasEpicMissionsForCharacter", &EpicArcService::AgentHasEpicMissionsForCharacter);
}

PyResult EpicArcService::AgentHasEpicMissionsForCharacter(PyCallArgs &call, PyInt* agentID) {
  /**
     epicArcStatusSvc = sm.RemoteSvc('epicArcStatus').AgentHasEpicMissionsForCharacter(agent.agentID):
     */
    _log(AGENT__INFO, "EpicArcService::Handle_AgentHasEpicMissionsForCharacter() - size=%li", call.tuple->size());
    call.Dump(AGENT__DUMP);

    // return boolean
    return PyStatic.NewFalse();

}
