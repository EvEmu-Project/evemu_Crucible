
/*
 *  EVE_Missions.h
 *   mission-specific data
 *
 */

#ifndef EVE_MISSIONS_H
#define EVE_MISSIONS_H

struct MissionData {
        bool important;
        uint8 level;
        uint8 typeID;
        uint8 range;
        uint16 missionID;
        uint32 briefingID;
        uint32 constellationID;
        uint32 corporationID;
        uint32 dungeonID;
        std::string name;
};

class PyList;
struct MissionOffer {
    bool important;
    bool storyline;
    bool remoteOfferable;
    bool remoteCompletable;
    uint8 stateID;
    uint8 typeID;
    uint8 range;
    uint16 bonusTime;           // time in minutes
    uint16 missionID;           // this is mission title messageID for locale
    uint16 rewardLP;
    uint16 rewardItemID;
    uint16 rewardItemQty;
    uint16 courierTypeID;
    uint16 courierAmount;
    uint16 destinationTypeID;
    uint32 offerID;
    uint32 agentID;
    uint32 briefingID;          // this is mission briefing messageID for locale
    //uint32 contentID;           // on live, this is specific char data for mission keywords.  we're not using it
    uint32 characterID;
    uint32 rewardISK;
    uint32 bonusISK;
    uint32 originID;
    uint32 originOwnerID;
    uint32 originSystemID;
    uint32 destinationID;
    uint32 destinationOwnerID;
    uint32 destinationSystemID;
    uint32 dungeonLocationID;
    uint32 dungeonSolarSystemID;
    uint32 acceptFee;
    float courierItemVolume;
    double expiryTime;
    double dateIssued;
    double dateAccepted;
    double dateCompleted;
    std::string name;
    PyList* bookmarks;
};

struct CourierData {
    bool important;
    bool storyline;
    uint8 level;
    uint8 typeID;
    uint8 range;
    uint8 raceID;
    uint16 bonusTime;
    uint16 missionID;
    uint16 itemTypeID;
    uint16 itemQty;
    uint16 rewardItemID;
    uint16 rewardItemQty;
    uint32 briefingID;
    uint32 rewardISK;
    uint32 bonusISK;
    float itemVolume;
    std::string name;
};

namespace Mission {
    namespace State {
        enum {
            Allocated   = 0,    // also listed as 'Offered' in some places
            Offered     = 1,
            Accepted    = 2,
            Failed      = 3,
            //added - cannot send these to client.
            Completed   = 4,
            Rejected    = 5,
            Defered     = 6,    // send as Allocated
            Expired     = 7     // send as Failed
        };
    }

    namespace Status {
        enum {
            Incomplete  = 0,
            Complete    = 1,
            Cheat       = 2
        };
    }

    namespace Type {
        enum {
            // i think these are arbitrary
            Tutorial    = 1,
            Encounter   = 2,
            Courier     = 3,
            Trade       = 4,
            Mining      = 5,
            Research    = 6,
            Data        = 7,
            Storyline   = 8,  // After every 15 regular missions completed you will be offered a storyline mission.
            Cosmos      = 9,
            Arc         = 10, //Throughout the arc, you will be offered choices which will branch the arc in one or more directions, and thus the arcs have different outcomes depending on your choices. The missions that make up these arcs typically have very good ISK rewards and the last mission of the arc typically carries a handsome reward. There are seven Epic Mission Arcs. Most players begin with The Blood-Stained Stars, an arc that can be completed in a T1 frigate and gives a boost in standings withe Sisters of Eve. Seasoned L4 runners will be doing the four empire epic arcs while the fearless pilots can do the two pirate epic arcs. Epic arcs can be repeated once every three months.
            Anomic      = 11, //optional security missions that are given out by level 4 agents. They can always be declined without penalty. Anomic missions present a different and higher challenge compared to other security missions. You will encounter a small number of very powerful adversaries and you are restricted in ship size.
            Burner      = 12  //Miscellanous offers that can be completed in T1 frig/dessy, that have no bearing on corp/ally standings.  these are purely personal agent requests.  all agents have a chance to give these "courier" missions, which can be decliened without penalty.
        };
    }

}

#endif  // EVE_MISSIONS_H


/* {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235484, 'label': u'DeclineImportantMessageTitle'}(u'Decline Important Mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235485, 'label': u'CargoCapacityWarningTitle'}(u'Cargo Capacity Warning', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235486, 'label': u'CargoCapacityWarningMessage'}(u'This mission involves objectives requiring a total capacity of {[numeric]requiredSpace, decimalPlaces=2} cargo units, but your active ship only has space for {[numeric]availableSpace, decimalPlaces=2} more cargo units in its cargo hold.  Accept anyway?', None, {u'{[numeric]requiredSpace, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'requiredSpace'}, u'{[numeric]availableSpace, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'availableSpace'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235487, 'label': u'MissingMissionObjectives'}(u'One or more mission objectives have not been completed.  Please check your mission journal for further information.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235488, 'label': u'DeclineImportantMessageBody'}(u'If you decline this important mission will lose a lot of standings with this agent.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to decline this important mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235489, 'label': u'CargoCapacityWarningAccept'}(u'To accept this mission, your ship would have to have space for {[numeric]neededCapacity, decimalPlaces=2} more cargo units in its cargo hold.', None, {u'{[numeric]neededCapacity, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'neededCapacity'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235490, 'label': u'MissingMissionObjectiveItem'}(u'One or more mission objectives have not been completed. For example, you must deliver {[item]objectiveTypeID.quantityName, quantity=objectiveQuantity} to complete this mission.  Please check your mission journal for further information.', None, {u'{[item]objectiveTypeID.quantityName, quantity=objectiveQuantity}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'quantityName', 'args': 0, 'kwargs': {'quantity': 'objectiveQuantity'}, 'variableName': 'objectiveTypeID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235491, 'label': u'DeclineMessageGeneric'}(u'Declining a mission from a particular agent more than once every 4 hours will result in a loss of standing with that agent.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235492, 'label': u'MissingMissionObjectivePlayerLocation'}(u'You have to be at the drop off location to deliver the items in person', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235493, 'label': u'MissingMissionObjectiveNonItem'}(u'One or more mission objectives have not been completed.  The item(s) must be located in your cargo hold or in your personal hangar (if the objective was within a station).  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  If a specific item was requested as opposed to any item of the specified type, please be sure that the correct specific item is indeed being provided.  Otherwise, please make sure that the item is not assembled, packaged or damaged.  Please check your mission journal for further information.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235494, 'label': u'AcceptFailureMessageTitle'}(u'Cannot Accept Mission', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235495, 'label': u'CompletionError'}(u'Ahem... there seems to have been a problem giving out your rewards.  Well, at least if you see this, all the other stuff should still work (standings, LP, next mission, storyline counter, etc)...', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235496, 'label': u'DeclineMessageTimeLeft'}(u'Declining a mission from this agent within the next {timeRemaining} will result in a loss of standing with this agent.', None, {u'{timeRemaining}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'timeRemaining'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235497, 'label': u'CompleteShareWithFleetMessage'}(u'You are currently in a fleet. Do you want to share the rewards of this mission with the other members of the group?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235498, 'label': u'DeclineMessage'}(u'If you decline a mission before {[datetime]when} you will lose standings with this agent, as well as his corp and faction.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to decline this mission?', None, {u'{[datetime]when}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'when'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235499, 'label': u'CompleteRewardBadCapacity'}(u'You do not have enough available cargo space to accept my generous reward.  The reward requires {[numeric]requiredSpace} cargo units but you ony have {[numeric]availableSpace} available.  Please either free up some cargo space or talk to me while docked in a station so I can transfer the reward to you there.', None, {u'{[numeric]requiredSpace}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSpace'}, u'{[numeric]availableSpace}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'availableSpace'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235500, 'label': u'CompletionFailure'}(u'Cannot Complete Mission', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235501, 'label': u'AcceptFailureMissingCollateral'}(u'You must provide the following as collateral prior to accepting this mission: {[item]typeID.quantityName, quantity=amount}', None, {u'{[item]typeID.quantityName, quantity=amount}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'quantityName', 'args': 0, 'kwargs': {'quantity': 'amount'}, 'variableName': 'typeID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235502, 'label': u'DeclineMissionTitle'}(u'Decline Mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235503, 'label': u'ShareWithFleetYes'}(u'Yes, split the mission rewards between all members (up to 10) equally.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235504, 'label': u'MissingMissionObjectiveItemLocation'}(u'One or more mission objectives have not been completed. For example, you must deliver {[item]objectiveTypeID.quantityName, quantity=objectiveQuantity} to {[location]location.name} to complete this mission.  The item(s) must be located in your cargo hold or in your personal hangar.  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  Please check your mission journal for further information.', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[item]objectiveTypeID.quantityName, quantity=objectiveQuantity}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'quantityName', 'args': 0, 'kwargs': {'quantity': 'objectiveQuantity'}, 'variableName': 'objectiveTypeID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235505, 'label': u'ShareWithFleetNo'}(u'No, I want to claim the reward for myself.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235506, 'label': u'DeclineNoMoreMissions'}(u'{[character]agentID.name} has no other missions to offer right now. Are you sure you want to decline?', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235507, 'label': u'DeclineNextMissionSuffix'}(u'{declineMessageText}\n\nHowever, believe it or not, I have another assignment prepared for you already.', None, {u'{declineMessageText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'declineMessageText'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235508, 'label': u'CantAcceptRemotely'}(u'This mission cannot be accepted remotely; go to {[character]agentID.nameWithPossessive} location and ask {[character]agentID.gender -> "him", "her"} in person.', None, {u'{[character]agentID.nameWithPossessive}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'nameWithPossessive', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}, u'{[character]agentID.gender -> "him", "her"}': {'conditionalValues': [u'him', u'her'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235509, 'label': u'CompletionErrorNewMission'}(u"Ahem... there seems to have been a problem giving out your rewards.  Well, at least if you see this, all the other stuff should still work (standings, LP, next mission, storyline counter, etc)...  I've got another mission ready for you already however...", None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235510, 'label': u'DeclineFactionPenaltyWarningOnly'}(u"If you decline a mission before {[datetime]when} you will lose standings with this agent's faction. If you lose enough standings, you will no longer be able to talk to the agent. Are you sure you would like to decline this mission?", None, {u'{[datetime]when}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'when'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235512, 'label': u'ObjectiveReportToAgent'}(u'Report to {[character]agentID.name}', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235513, 'label': u'BonusRewardsHeader'}(u'The following rewards will be awarded to you as a bonus if you complete the mission within {[timeinterval]timeRemaining.writtenForm, to=minute}', None, {u'{[timeinterval]timeRemaining.writtenForm, to=minute}': {'conditionalValues': [], 'variableType': 8, 'propertyName': 'writtenForm', 'args': 0, 'kwargs': {'to': 'minute'}, 'variableName': 'timeRemaining'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235514, 'label': u'BonusRewardsTitle'}(u'Bonus Rewards', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235515, 'label': u'DelayMissionMessage'}(u'Delaying your decision on a mission will end your conversation with the agent, but the mission offer will remain in your journal.  If you are not ready to accept this mission right now, you may defer it and come back to the agent later, until the offer expires.  Expired offers will automatically be removed from your journal.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235516, 'label': u'BonusTimePassed'}(u'<font color=#E3170D>Bonus no longer available.  The bonus time interval has passed.</font>', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235517, 'label': u'DelayMessageTitle'}(u'Delay Mission', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235518, 'label': u'AgentLocation'}(u'Agent Location', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235519, 'label': u'AcceptedGrantedItemDetail'}(u'The following item was granted to you when the mission was accepted', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235520, 'label': u'DungeonObjectiveFailed'}(u'<font color="#E0FF0000">Objective failed.</font>', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235521, 'label': u'MissionBriefingCorrupt'}(u'Error:  mission briefing corrupt.  This has been logged server side, and will undoubtedly be fixed as soon as possible.  Sorry for the inconvenience.', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235522, 'label': u'ObjectiveHeader'}(u'Objective', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235523, 'label': u'DungeonObjectiveCompleted'}(u'<font color="#E000FF00">Objective completed.</font>', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235524, 'label': u'CollateralHeader'}(u'Prior to accepting this mission, the following must be provided by you as collateral, to be returned to you upon successful completion of the mission:', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235525, 'label': u'DungeonObjectiveBody'}(u'Destroy these targets', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235526, 'label': u'CollateralTitle'}(u'Collateral', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235528, 'label': u'ObjectiveLocation'}(u'Location', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235537, 'label': u'GrantedItems'}(u'Granted Items', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235538, 'label': u'MissionExpirationTitle'}(u'Mission Expiration', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235539, 'label': u'OptionalObjectiveBody'}(u'Optionally, destroy these targets', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235540, 'label': u'DungeonObjectiveNormalRestrictions'}(u'This site contains normal {startHttpLink}ship restrictions{endHttpLink}.', None, {u'{startHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'startHttpLink'}, u'{endHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'endHttpLink'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235541, 'label': u'DungeonObjectiveSpecialRestrictions'}(u'This site contains special {startHttpLink}ship restrictions{endHttpLink}.', None, {u'{startHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'startHttpLink'}, u'{endHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'endHttpLink'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235542, 'label': u'FetchObjectiveBlurb'}(u'Acquire these goods:', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235543, 'label': u'FetchObjectiveItem'}(u'Item', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235544, 'label': u'OptionalObjectiveHeader'}(u'Optional Objective', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235545, 'label': u'FetchObjectiveHeader'}(u'Bring Item Objective', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235546, 'label': u'FetchObjectiveDropOffLocation'}(u'Drop-off Location', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235547, 'label': u'GrantedItemDetail'}(u'The following item will be granted to you when the mission is accepted', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235548, 'label': u'MissionBriefing'}(u'Mission briefing', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235549, 'label': u'MissionObjectives'}(u'{missionName} Objectives', None, {u'{missionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionName'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235550, 'label': u'MissionObjectivesComplete'}(u'{missionName} Objectives Complete', None, {u'{missionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionName'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235551, 'label': u'OverviewAndObjectivesBlurb'}(u'The following objectives must be completed to finish the mission:', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235552, 'label': u'NumResearchPoints'}(u'{[numeric]rpAmount} Research Points.', None, {u'{[numeric]rpAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rpAmount'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235553, 'label': u'NumLoyaltyPoints'}(u'{[numeric]lpAmount} Loyalty Points.', None, {u'{[numeric]lpAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'lpAmount'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235554, 'label': u'MissionReferral'}(u'Referral to {[character]agentID.name}', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235555, 'label': u'ImportantStandingsWarning'}(u'<b><i>This is an important mission, which will have significant impact on your faction standings.</i></b>', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235560, 'label': u'QuitImportantMissionMessage'}(u'If you quit this important mission you will lose a lot of standings with your agent, as well as his corp and faction. If you lose enough standings, you will no longer be able to talk to the agent. Are you sure you would like to quit this mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235561, 'label': u'QuitMissionTitle'}(u'Quit Mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235562, 'label': u'QuitMissionMessage'}(u'If you quit this mission you will lose standings with your agent, as well as his corp and faction.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to quit this mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235563, 'label': u'RewardsTitle'}(u'Rewards', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235564, 'label': u'TransportPickupLocation'}(u'Pickup Location', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235565, 'label': u'QuitMissionNoMoreMissions'}(u'{[character]agentID.name} has no other missions to offer right now. Are you sure you want to quit?', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235566, 'label': u'TransportDropOffLocation'}(u'Drop-off Location', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235567, 'label': u'TransportObjectiveHeader'}(u'Transport Objective', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235568, 'label': u'CantQuitRemotely'}(u'You cannot quit this mission remotely; go to {[character]agentID.nameWithPossessive} location and talk to {[character]agentID.gender -> "him", "her"}  in person.', None, {u'{[character]agentID.nameWithPossessive}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'nameWithPossessive', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}, u'{[character]agentID.gender -> "him", "her"}': {'conditionalValues': [u'him', u'her'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235569, 'label': u'RewardsHeader'}(u'The following rewards will be yours if you complete this mission:', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235570, 'label': u'QuitImportantMissionTitle'}(u'Quit Important Mission?', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235571, 'label': u'TransportBlurb'}(u'Transport these goods:', None, None)
 * {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235572, 'label': u'TransportCargo'}(u'Cargo', None, None)
 * {'FullPath': u'UI/Agents/Dialogue', 'messageID': 235573, 'label': u'ThisOfferExpiresAt'}(u'This offer expires at {[datetime]expireTime}', None, {u'{[datetime]expireTime}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'expireTime'}})
 * {'FullPath': u'UI/Agents/Dialogue', 'messageID': 235574, 'label': u'ThisMissionExpiresAt'}(u'This mission expires at {[datetime]expireTime}', None, {u'{[datetime]expireTime}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'expireTime'}})
 */