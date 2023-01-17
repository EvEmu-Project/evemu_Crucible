
 /**
  * @name AgentBound.h
  *   agent specific code
  *    removed from AgentMgrService.cpp
  *
  * @Author:        Allan
  * @date:      26 June 2018
  *
  */


#ifndef _EVE_SERVER_AGENTBOUND_H
#define _EVE_SERVER_AGENTBOUND_H


#include "services/BoundService.h"

#include "agents/Agent.h"

class AgentMgrService;

class AgentBound : public EVEBoundObject <AgentBound>
{
public:
    AgentBound(EVEServiceManager& mgr, AgentMgrService& parent, Agent *agt);

    Agent* GetAgent () { return this->m_agent; }

protected:
    Agent* m_agent;    //we do not own this.

    PyResult GetAgentLocationWrap(PyCallArgs& call);
    PyResult GetInfoServiceDetails(PyCallArgs& call);
    PyResult DoAction(PyCallArgs& call, std::optional <PyInt*> actionID);
    PyResult GetMissionBriefingInfo(PyCallArgs& call);
    PyResult GetMissionKeywords(PyCallArgs& call, PyInt* contentID);
    PyResult GetMissionObjectiveInfo(PyCallArgs& call, std::optional <PyInt*> characterID, std::optional <PyInt*> contentID);
    PyResult GetMyJournalDetails(PyCallArgs& call);
    PyResult GetMissionJournalInfo(PyCallArgs& call, PyInt* characterID, PyInt* contentID);
    PyResult GetDungeonShipRestrictions(PyCallArgs& call, PyInt* dungeonID);
    PyResult RemoveOfferFromJournal(PyCallArgs& call);
    PyResult GetOfferJournalInfo(PyCallArgs& call);
    PyResult GetEntryPoint(PyCallArgs& call);
    PyResult GotoLocation(PyCallArgs& call, PyInt* locationType, PyInt* locationNumber, PyInt* referringAgentID);
    PyResult WarpToLocation(PyCallArgs& call, PyInt* locationType, PyInt* locationNumber, PyFloat* warpRange, PyBool* fleet, PyInt* referringAgentID);

private:
    PyTuple* GetMissionObjectives(Client* pClient, MissionOffer& offer);
    PyDict* GetMissionObjectiveInfo(Client* pClient, MissionOffer& offer);
};

#endif  // _EVE_SERVER_AGENTBOUND_H

/*
 ( 239875, `Current* RP`)
 (239876, `RP/day`)
 (239877, `Level`)
(232833, `Agent Conversation`)
(232834, `Agent Conversation - {[character]agentID.name}`)
(251135, `To accept this mission, your ship would have to have space for {[numeric]cargoUnits, decimalPlaces=2} more cargo units in its cargo hold.`)

(235484, `Decline Important Mission?`)
(235485, `Cargo Capacity Warning`)
(235486, `This mission involves objectives requiring a total capacity of {[numeric]requiredSpace, decimalPlaces=2} cargo units, but your active ship only has space for {[numeric]availableSpace, decimalPlaces=2} more cargo units in its cargo hold.  Accept anyway?`)
(235487, `One or more mission objectives have not been completed.  Please check your mission journal for further information.`)
(235488, `If you decline this important mission will lose a lot of standings with this agent.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to decline this important mission?`)
(235489, `To accept this mission, your ship would have to have space for {[numeric]neededCapacity, decimalPlaces=2} more cargo units in its cargo hold.`)
(235490, `One or more mission objectives have not been completed. For example, you must deliver {[item]objectiveTypeID.quantityName, quantity=objectiveQuantity} to complete this mission.  Please check your mission journal for further information.`)
(235491, `Declining a mission from a particular agent more than once every 4 hours will result in a loss of standing with that agent.`)
(235492, `You have to be at the drop off location to deliver the items in person`)
(235493, `One or more mission objectives have not been completed.  The item(s) must be located in your cargo hold or in your personal hangar (if the objective was within a station).  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  If a specific item was requested as opposed to any item of the specified type, please be sure that the correct specific item is indeed being provided.  Otherwise, please make sure that the item is not assembled, packaged or damaged.  Please check your mission journal for further information.`)
(235494, `Cannot Accept Mission`)
(235495, `Ahem... there seems to have been a problem giving out your rewards.  Well, at least if you see this, all the other stuff should still work (standings, LP, next mission, storyline counter, etc)...`)
(235496, `Declining a mission from this agent within the next {timeRemaining} will result in a loss of standing with this agent.`)
(235497, `You are currently in a fleet. Do you want to share the rewards of this mission with the other members of the group?`)
(235498, `If you decline a mission before {[datetime]when} you will lose standings with this agent, as well as his corp and faction.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to decline this mission?`)
(235499, `You do not have enough available cargo space to accept my generous reward.  The reward requires {[numeric]requiredSpace} cargo units but you ony have {[numeric]availableSpace} available.  Please either free up some cargo space or talk to me while docked in a station so I can transfer the reward to you there.`)
(235500, `Cannot Complete Mission`)
(235501, `You must provide the following as collateral prior to accepting this mission: {[item]typeID.quantityName, quantity=amount}`)
(235502, `Decline Mission?`)
(235503, `Yes, split the mission rewards between all members (up to 10) equally.`)
(235504, `One or more mission objectives have not been completed. For example, you must deliver {[item]objectiveTypeID.quantityName, quantity=objectiveQuantity} to {[location]location.name} to complete this mission.  The item(s) must be located in your cargo hold or in your personal hangar.  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  Please check your mission journal for further information.`)
(235505, `No, I want to claim the reward for myself.`)
(235506, `{[character]agentID.name} has no other missions to offer right now. Are you sure you want to decline?`)
(235507, `{declineMessageText}

However, believe it or not, I have another assignment prepared for you already.`)
(235508, `This mission cannot be accepted remotely; go to {[character]agentID.nameWithPossessive} location and ask {[character]agentID.gender -> "him", "her"} in person.`)
(235509, `Ahem... there seems to have been a problem giving out your rewards.  Well, at least if you see this, all the other stuff should still work (standings, LP, next mission, storyline counter, etc)...  I've got another mission ready for you already however...`)
(235510, `If you decline a mission before {[datetime]when} you will lose standings with this agent's faction. If you lose enough standings, you will no longer be able to talk to the agent. Are you sure you would like to decline this mission?`)
(235511, `No Item`)
(235512, `Report to {[character]agentID.name}`)
(235513, `The following rewards will be awarded to you as a bonus if you complete the mission within {[timeinterval]timeRemaining.writtenForm, to=minute}`)
(235514, `Bonus Rewards`)
(235515, `Delaying your decision on a mission will end your conversation with the agent, but the mission offer will remain in your journal.  If you are not ready to accept this mission right now, you may defer it and come back to the agent later, until the offer expires.  Expired offers will automatically be removed from your journal.`)
(235516, `<font color=#E3170D>Bonus no longer available.  The bonus time interval has passed.</font>`)
(235517, `Delay Mission`)
(235518, `Agent Location`)
(235519, `The following item was granted to you when the mission was accepted`)
(235520, `<font color="#E0FF0000">Objective failed.</font>`)
(235521, `Error:  mission briefing corrupt.  This has been logged server side, and will undoubtedly be fixed as soon as possible.  Sorry for the inconvenience.`)
(235522, `Objective`)
(235523, `<font color="#E000FF00">Objective completed.</font>`)
(235524, `Prior to accepting this mission, the following must be provided by you as collateral, to be returned to you upon successful completion of the mission:`)
(235525, `Destroy these targets`)
(235526, `Collateral`)
(235527, `S_Pale_01`)
(235528, `Location`)
(235529, `S_Green_01`)
(235530, `Vapor`)
(235537, `Granted Items`)
(235538, `Mission Expiration`)
(235539, `Optionally, destroy these targets`)
(235540, `This site contains normal {startHttpLink}ship restrictions{endHttpLink}.`)
(235541, `This site contains special {startHttpLink}ship restrictions{endHttpLink}.`)
(235542, `Acquire these goods:`)
(235543, `Item`)
(235544, `Optional Objective`)
(235545, `Bring Item Objective`)
(235546, `Drop-off Location`)
(235547, `The following item will be granted to you when the mission is accepted`)
(235548, `Mission briefing`)
(235549, `{missionName} Objectives`)
(235550, `{missionName} Objectives Complete`)
(235551, `The following objectives must be completed to finish the mission:`)
(235552, `{[numeric]rpAmount} Research Points.`)
(235553, `{[numeric]lpAmount} Loyalty Points.`)
(235554, `Referral to {[character]agentID.name}`)
(235555, `<b><i>This is an important mission, which will have significant impact on your faction standings.</i></b>`)
(235556, `Noise`)
(235557, `Collision`)
(235558, `Sparks`)
(235559, `Smoke_Atlas`)
(235560, `If you quit this important mission you will lose a lot of standings with your agent, as well as his corp and faction. If you lose enough standings, you will no longer be able to talk to the agent. Are you sure you would like to quit this mission?`)
(235561, `Quit Mission?`)
(235562, `If you quit this mission you will lose standings with your agent, as well as his corp and faction.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to quit this mission?`)
(235563, `Rewards`)
(235564, `Pickup Location`)
(235565, `{[character]agentID.name} has no other missions to offer right now. Are you sure you want to quit?`)
(235566, `Drop-off Location`)
(235567, `Transport Objective`)
(235568, `You cannot quit this mission remotely; go to {[character]agentID.nameWithPossessive} location and talk to {[character]agentID.gender -> "him", "her"}  in person.`)
(235569, `The following rewards will be yours if you complete this mission:`)
(235570, `Quit Important Mission?`)
(235571, `Transport these goods:`)
(235572, `Cargo`)
(235573, `This offer expires at {[datetime]expireTime}`)
(235574, `This mission expires at {[datetime]expireTime}`)
(235575, `Warning`)
*/