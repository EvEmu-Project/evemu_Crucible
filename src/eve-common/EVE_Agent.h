
/*
 *  EVE_Agent.h
 *   agent-specific data
 *
 */

#ifndef EVE_AGENT_H
#define EVE_AGENT_H

struct AgentData {
    bool gender;
    bool locator;
    bool research;
    int8 quality;
    uint8 level;
    uint8 raceID;
    uint8 divisionID;
    uint8 bloodlineID;
    uint16 typeID;
    uint16 locationTypeID;
    uint32 corporationID;
    uint32 solarSystemID;
    uint32 stationID;
    uint32 locationID;
    uint32 friendCorp;
    uint32 enemyCorp;
    uint32 factionID;
    std::string name;
};

struct AgentOffers {

};

struct AgentActions {

};

namespace Agents {
    namespace Type {
        enum {
            //  -allan 20Dec14
            None                = 1,
            Basic               = 2,
            Tutorial            = 3,
            Research            = 4,        //249
            GenericStoryLine    = 6,
            StoryLine           = 7,
            Event               = 8,
            FacWar              = 9,
            EpicArc             = 10,
            Aura                = 11
        };
    }

    namespace Range {
        enum {
            SameSystem = 1,
            SameOrNeighboringSystemSameConstellation = 2,
            SameOrNeighboringSystem = 3,
            NeighboringSystemSameConstellation = 4,
            NeighboringSystem = 5,
            SameConstellation = 6,
            SameOrNeighboringConstellationSameRegion = 7,
            SameOrNeighboringConstellation = 8,
            NeighboringConstellationSameRegion = 9,
            NeighboringConstellation = 10,
            NearestEnemyCombatZone = 11,
            NearestCareerHub = 12
        };
    }

    namespace IskMult {
        enum {
            Level1 = 1,
            Level2 = 2,
            Level3 = 4,
            Level4 = 8,
            Level5 = 16,
            RandomLow = 11000,
            RandomHigh = 16500
        };
    }

    namespace LpMult {
        enum {
            Level1 = 20,
            Level2 = 60,
            Level3 = 180,
            Level4 = 540,
            Level5 = 4860
        };
    }

    namespace Career {
        enum {
            Industry    = 1,
            Business    = 2,
            Military    = 3,
            Exploration = 4,
            AdvMilitary = 5
        };
    }

/*  see Corp::Division for this data...
    namespace Division {
        enum {                       //  Kill   Courier Trade   Mining
            Accounting          = 1, //    0%   88%     12%      0%
            Administration      = 2, //   47%   47%      6%      0%
            Advisory            = 3, //   14%   58%     14%     14%
            Archives            = 4, //    0%   92%      8%      0%
            Astrosurveying      = 5, //   13%   25%     13%     50%
            Command             = 6, //   88%    6%      6%      0%
            Distribution        = 7, //    5%   85%      5%      5%
            Financial           = 8, //   12%   70%     18%      0%
            Intelligence        = 9, //   74%   21%      5%      0%
            InternalSecurity    = 10, //  98%    2%      0%      0%
            Legal               = 11, //  67%   27%      6%      0%
            Manufacturing       = 12, //   0%   48%      4%     48%
            Marketing           = 13, //  17%   77%      6%      0%
            Mining              = 14, //   0%   10%      5%     85%
            Personnel           = 15, //  28%   66%      6%      0%
            Production          = 16, //   0%   52%     13%     35%
            PublicRelations     = 17, //  28%   66%      6%      0%
            RnD                 = 18, //   0%   50%     50%      0%
            Security            = 19, //  94%    6%      0%      0%
            Storage             = 20, //   6%   71%      6%     17%
            Surveillance        = 21, //  84%   11%      5%      0%

            // 724 new agents with one of these new divisions compared to RMR
            //281 l1
            //101 l2
            //112 l3
            //189 l4
            //41 l5     (all dID 24)
            DistributionNew     = 22,
            MiningNew           = 23,
            SecurityNew         = 24
        };
    }*/
}

namespace Dialog {
    namespace Button {
        enum {
            ViewMission         = 1,
            RequestMission      = 2,
            Accept              = 3,
            AcceptChoice        = 4,
            AcceptRemotely      = 5,
            Complete            = 6,
            CompleteRemotely    = 7,
            Continue            = 8,
            Decline             = 9,
            Defer               = 10,
            Quit                = 11,
            StartResearch       = 12,
            CancelResearch      = 13,
            BuyDatacores        = 14,
            LocateCharacter     = 15,
            LocateAccept        = 16,
            LocateReject        = 17,
            Yes                 = 18,
            No                  = 19,
            Admin               = 20    // added for 'admin' menu
        };
    }

}

/*
 * dataIDs sent by "OnIncomingAgentMessage" notification  (?)
 {'FullPath': u'UI/Agents/Bookmarks', 'messageID': 235205, 'label': u'AgentBase'}(u'Agent Home Base - {*[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235210, 'label': u'AgentBase'}(u'Objective & Agent Base -{[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235211, 'label': u'DropOff'}(u'Objective (Drop Off) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235212, 'label': u'DropOffAgentBase'}(u'Objective (Drop Off) & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235213, 'label': u'Sequence'}(u'Objective #{[numeric]index} - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235214, 'label': u'SequenceAgentBase'}(u'Objective #{[numeric]index} & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235215, 'label': u'SequenceDropOff'}(u'Objective #{[numeric]index} (Drop Off) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235216, 'label': u'SequenceDropOffAgentBase'}(u'Objective #{[numeric]index} (Drop Off) & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235217, 'label': u'SequencePickup'}(u'Objective #{[numeric]index} (Pick Up) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235218, 'label': u'SequencePickupAgentBase'}(u'Objective #{[numeric]index} (Pick Up) & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235219, 'label': u'Pickup'}(u'Objective (Pick Up) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235220, 'label': u'PickupAgentBase'}(u'Objective (Pick Up) & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Objective', 'messageID': 235221, 'label': u'Objective'}(u'Objective - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Reward', 'messageID': 235222, 'label': u'Reward'}(u'Reward - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Reward', 'messageID': 235223, 'label': u'AgentBase'}(u'Reward & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Reward', 'messageID': 235224, 'label': u'Sequence'}(u'Reward #{[numeric]index} - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Reward', 'messageID': 235225, 'label': u'SequenceAgentBase'}(u'Reward #{[numeric]index} & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235226, 'label': u'Encounter'}(u'Encounter - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235227, 'label': u'AgentBase'}(u'Encounter & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235228, 'label': u'Deadspace'}(u'Encounter (Deadspace) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235229, 'label': u'DeadspaceAgentBase'}(u'Encounter (Deadspace) & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235230, 'label': u'SequenceDeadspace'}(u'Encounter #{[numeric]index} (Deadspace) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235231, 'label': u'SequenceDeadspaceAgentBase'}(u'Encounter #{[numeric]index} (Deadspace) & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235232, 'label': u'Sequence'}(u'Encounter #{[numeric]index} - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Encounter', 'messageID': 235233, 'label': u'SequenceAgentBase'}(u'Encounter #{[numeric]index} & Agent Base - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[numeric]index}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'index'}})
 {'FullPath': u'UI/Agents/Bookmarks/Mining', 'messageID': 235234, 'label': u'Mining'}(u'Mining - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/Bookmarks/Mining', 'messageID': 235235, 'label': u'Deadspace'}(u'Mining (Deadspace) - {[location]location.name}', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}})
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235236, 'label': u'StorylineEncounter'}(u'Storyline - Encounter', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235237, 'label': u'StorylineMining'}(u'Storyline - Mining', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235238, 'label': u'Storyline'}(u'Storyline', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235239, 'label': u'EpicArcTalkToAgent'}(u'Epic Arc - Talk to Agent', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235240, 'label': u'ResearchTrade'}(u'Research - Trade', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235241, 'label': u'StorylineCourier'}(u'Storyline - Courier', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235242, 'label': u'ResearchCourier'}(u'Research - Courier', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235243, 'label': u'EpicArcTrade'}(u'Epic Arc - Trade', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235244, 'label': u'Research'}(u'Research', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235245, 'label': u'StorylineTrade'}(u'Storyline - Trade', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235246, 'label': u'EpicArcEncounter'}(u'Epic Arc - Encounter', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235247, 'label': u'Courier'}(u'Courier', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235248, 'label': u'Trade'}(u'Trade', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235249, 'label': u'EpicArc'}(u'Epic Arc', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235250, 'label': u'EpicArcAgentInteraction'}(u'Epic Arc - Agent Interaction', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235251, 'label': u'Encounter'}(u'Encounter', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235252, 'label': u'Mining'}(u'Mining', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235253, 'label': u'EpicArcCourier'}(u'Epic Arc - Courier', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235254, 'label': u'EpicArcMining'}(u'Epic Arc - Mining', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235255, 'label': u'ResearchEncounter'}(u'Research - Encounter', None, None)
 {'FullPath': u'UI/Agents/MissionTypes', 'messageID': 235256, 'label': u'StorylineAgentInteraction'}(u'Storyline - Agent Interaction', None, None)
 {'FullPath': u'UI/Agents/EpicArcs', 'messageID': 235257, 'label': u'QuitArcRestartLater'}(u'You quit this arc on {[datetime]quitTime, date=long, time=short}. You may restart it at {[datetime]restartTime, date=long, time=short}.', None, {u'{[datetime]restartTime, date=long, time=short}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {'format': u'%B %d, %Y %H:%M'}, 'variableName': 'restartTime'}, u'{[datetime]quitTime, date=long, time=short}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {'format': u'%B %d, %Y %H:%M'}, 'variableName': 'quitTime'}})
 {'FullPath': u'UI/Agents/EpicArcs', 'messageID': 235258, 'label': u'QuitArcRestartNow'}(u'You quit this arc on {[datetime]quitTime, date=long, time=short}. You may restart it by returning to the first agent in the arc.', None, {u'{[datetime]quitTime, date=long, time=short}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {'format': u'%B %d, %Y %H:%M'}, 'variableName': 'quitTime'}})
 {'FullPath': u'UI/Agents/Incompatible', 'messageID': 235462, 'label': u'CantUseMinStandings'}(u"Your personal standings must be -1.9 or higher toward this agent, its faction, or its corporation in order to use this agent's services.", None, None)
 {'FullPath': u'UI/Agents/Incompatible', 'messageID': 235463, 'label': u'ResearchAgent'}(u"Your personal standings must be -1.9 or higher toward this agent, its faction, and its corporation in order to use this agent's services. Additionally, you need a minimum effective standing to this agent's corp of at least {[numeric]corpMinStandings, decimalPlaces=1} , as well as personal standing of at least {[numeric]effectiveMinStandings, decimalPlaces=1} to this agent's faction, corp, or to the agent in order to use this agent's services.", None, {u'{[numeric]effectiveMinStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'effectiveMinStandings'}, u'{[numeric]corpMinStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'corpMinStandings'}})
 {'FullPath': u'UI/Agents/Incompatible', 'messageID': 235464, 'label': u'ResearchStandings'}(u"Your personal standings must be -1.9 or higher toward this agent, its faction, and its corporation in order to use this agent's services. Additionally, you need a minimum effective standing of at least {[numeric]minStandings, decimalPlaces=1} to this agent's faction, corp, or to the agent in order to use this agent's services.", None, {u'{[numeric]minStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'minStandings'}})
 {'FullPath': u'UI/Agents/Incompatible', 'messageID': 235465, 'label': u'CantUseRequiredStandings'}(u"Your effective personal standings must be {[numeric]minStandings, decimalPlaces=1} or higher toward this agent, its faction, or its corporation in order to use this agent's services", None, {u'{[numeric]minStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'minStandings'}})
 {'FullPath': u'UI/Agents/Incompatible', 'messageID': 235466, 'label': u'CantUseResearchStandings'}(u"Your effective personal standings must be {[numeric]minEffective, decimalPlaces=1} or higher toward this agent's corporation in order to use this agent, as well as an effective personal standing of {[numeric]mainEffective, decimalPlaces=1} or higher toward this agent, its faction, or its corporation in order to use this agent's services.", None, {u'{[numeric]mainEffective, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'mainEffective'}, u'{[numeric]minEffective, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'minEffective'}})
 {'FullPath': u'UI/Agents/Incompatible', 'messageID': 235467, 'label': u'NeedsReferral'}(u'This agent can only be used through a direct referral.', None, None)
 {'FullPath': u'UI/Agents/Dialogue', 'messageID': 235470, 'label': u'BrokenMessage'}(u"Wow!  I just don't know what to say to that...  I feel like an unhandled exception occurred on the server while I was putting together my next line.  I'm so confused, I don't know what's gotten into me, but for some reason I know this exception has been logged server-side, along with a stack trace, so the proper authorities will undoubtedly take care of this as soon as possible...   By the Gate, what AM I saying???  I need a doctor, quick!  Help!!!", None, None)
 {'FullPath': u'UI/Agents/Dialogue', 'messageID': 235471, 'label': u'CantUseOnTrial'}(u'Trial account users cannot access agents of level 3 or higher. Either use your map settings to find a level 1 or level 2 agent, or consider purchasing an EVE subscription if you wish to access higher-level content.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235484, 'label': u'DeclineImportantMessageTitle'}(u'Decline Important Mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235485, 'label': u'CargoCapacityWarningTitle'}(u'Cargo Capacity Warning', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235486, 'label': u'CargoCapacityWarningMessage'}(u'This mission involves objectives requiring a total capacity of {[numeric]requiredSpace, decimalPlaces=2} cargo units, but your active ship only has space for {[numeric]availableSpace, decimalPlaces=2} more cargo units in its cargo hold.  Accept anyway?', None, {u'{[numeric]requiredSpace, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'requiredSpace'}, u'{[numeric]availableSpace, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'availableSpace'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235487, 'label': u'MissingMissionObjectives'}(u'One or more mission objectives have not been completed.  Please check your mission journal for further information.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235488, 'label': u'DeclineImportantMessageBody'}(u'If you decline this important mission will lose a lot of standings with this agent.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to decline this important mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235489, 'label': u'CargoCapacityWarningAccept'}(u'To accept this mission, your ship would have to have space for {[numeric]neededCapacity, decimalPlaces=2} more cargo units in its cargo hold.', None, {u'{[numeric]neededCapacity, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'neededCapacity'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235490, 'label': u'MissingMissionObjectiveItem'}(u'One or more mission objectives have not been completed. For example, you must deliver {[item]objectiveTypeID.quantityName, quantity=objectiveQuantity} to complete this mission.  Please check your mission journal for further information.', None, {u'{[item]objectiveTypeID.quantityName, quantity=objectiveQuantity}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'quantityName', 'args': 0, 'kwargs': {'quantity': 'objectiveQuantity'}, 'variableName': 'objectiveTypeID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235491, 'label': u'DeclineMessageGeneric'}(u'Declining a mission from a particular agent more than once every 4 hours will result in a loss of standing with that agent.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235492, 'label': u'MissingMissionObjectivePlayerLocation'}(u'You have to be at the drop off location to deliver the items in person', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235493, 'label': u'MissingMissionObjectiveNonItem'}(u'One or more mission objectives have not been completed.  The item(s) must be located in your cargo hold or in your personal hangar (if the objective was within a station).  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  If a specific item was requested as opposed to any item of the specified type, please be sure that the correct specific item is indeed being provided.  Otherwise, please make sure that the item is not assembled, packaged or damaged.  Please check your mission journal for further information.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235494, 'label': u'AcceptFailureMessageTitle'}(u'Cannot Accept Mission', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235495, 'label': u'CompletionError'}(u'Ahem... there seems to have been a problem giving out your rewards.  Well, at least if you see this, all the other stuff should still work (standings, LP, next mission, storyline counter, etc)...', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235496, 'label': u'DeclineMessageTimeLeft'}(u'Declining a mission from this agent within the next {timeRemaining} will result in a loss of standing with this agent.', None, {u'{timeRemaining}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'timeRemaining'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235497, 'label': u'CompleteShareWithFleetMessage'}(u'You are currently in a fleet. Do you want to share the rewards of this mission with the other members of the group?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235498, 'label': u'DeclineMessage'}(u'If you decline a mission before {[datetime]when} you will lose standings with this agent, as well as his corp and faction.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to decline this mission?', None, {u'{[datetime]when}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'when'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235499, 'label': u'CompleteRewardBadCapacity'}(u'You do not have enough available cargo space to accept my generous reward.  The reward requires {[numeric]requiredSpace} cargo units but you ony have {[numeric]availableSpace} available.  Please either free up some cargo space or talk to me while docked in a station so I can transfer the reward to you there.', None, {u'{[numeric]requiredSpace}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSpace'}, u'{[numeric]availableSpace}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'availableSpace'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235500, 'label': u'CompletionFailure'}(u'Cannot Complete Mission', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235501, 'label': u'AcceptFailureMissingCollateral'}(u'You must provide the following as collateral prior to accepting this mission: {[item]typeID.quantityName, quantity=amount}', None, {u'{[item]typeID.quantityName, quantity=amount}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'quantityName', 'args': 0, 'kwargs': {'quantity': 'amount'}, 'variableName': 'typeID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235502, 'label': u'DeclineMissionTitle'}(u'Decline Mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235503, 'label': u'ShareWithFleetYes'}(u'Yes, split the mission rewards between all members (up to 10) equally.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235504, 'label': u'MissingMissionObjectiveItemLocation'}(u'One or more mission objectives have not been completed. For example, you must deliver {[item]objectiveTypeID.quantityName, quantity=objectiveQuantity} to {[location]location.name} to complete this mission.  The item(s) must be located in your cargo hold or in your personal hangar.  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  Please check your mission journal for further information.', None, {u'{[location]location.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'location'}, u'{[item]objectiveTypeID.quantityName, quantity=objectiveQuantity}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'quantityName', 'args': 0, 'kwargs': {'quantity': 'objectiveQuantity'}, 'variableName': 'objectiveTypeID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235505, 'label': u'ShareWithFleetNo'}(u'No, I want to claim the reward for myself.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235506, 'label': u'DeclineNoMoreMissions'}(u'{[character]agentID.name} has no other missions to offer right now. Are you sure you want to decline?', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235507, 'label': u'DeclineNextMissionSuffix'}(u'{declineMessageText}\n\nHowever, believe it or not, I have another assignment prepared for you already.', None, {u'{declineMessageText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'declineMessageText'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235508, 'label': u'CantAcceptRemotely'}(u'This mission cannot be accepted remotely; go to {[character]agentID.nameWithPossessive} location and ask {[character]agentID.gender -> "him", "her"} in person.', None, {u'{[character]agentID.nameWithPossessive}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'nameWithPossessive', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}, u'{[character]agentID.gender -> "him", "her"}': {'conditionalValues': [u'him', u'her'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235509, 'label': u'CompletionErrorNewMission'}(u"Ahem... there seems to have been a problem giving out your rewards.  Well, at least if you see this, all the other stuff should still work (standings, LP, next mission, storyline counter, etc)...  I've got another mission ready for you already however...", None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235510, 'label': u'DeclineFactionPenaltyWarningOnly'}(u"If you decline a mission before {[datetime]when} you will lose standings with this agent's faction. If you lose enough standings, you will no longer be able to talk to the agent. Are you sure you would like to decline this mission?", None, {u'{[datetime]when}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'when'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235512, 'label': u'ObjectiveReportToAgent'}(u'Report to {[character]agentID.name}', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235513, 'label': u'BonusRewardsHeader'}(u'The following rewards will be awarded to you as a bonus if you complete the mission within {[timeinterval]timeRemaining.writtenForm, to=minute}', None, {u'{[timeinterval]timeRemaining.writtenForm, to=minute}': {'conditionalValues': [], 'variableType': 8, 'propertyName': 'writtenForm', 'args': 0, 'kwargs': {'to': 'minute'}, 'variableName': 'timeRemaining'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235514, 'label': u'BonusRewardsTitle'}(u'Bonus Rewards', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235515, 'label': u'DelayMissionMessage'}(u'Delaying your decision on a mission will end your conversation with the agent, but the mission offer will remain in your journal.  If you are not ready to accept this mission right now, you may defer it and come back to the agent later, until the offer expires.  Expired offers will automatically be removed from your journal.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235516, 'label': u'BonusTimePassed'}(u'<font color=#E3170D>Bonus no longer available.  The bonus time interval has passed.</font>', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235517, 'label': u'DelayMessageTitle'}(u'Delay Mission', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235518, 'label': u'AgentLocation'}(u'Agent Location', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235519, 'label': u'AcceptedGrantedItemDetail'}(u'The following item was granted to you when the mission was accepted', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235520, 'label': u'DungeonObjectiveFailed'}(u'<font color="#E0FF0000">Objective failed.</font>', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235521, 'label': u'MissionBriefingCorrupt'}(u'Error:  mission briefing corrupt.  This has been logged server side, and will undoubtedly be fixed as soon as possible.  Sorry for the inconvenience.', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235522, 'label': u'ObjectiveHeader'}(u'Objective', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235523, 'label': u'DungeonObjectiveCompleted'}(u'<font color="#E000FF00">Objective completed.</font>', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235524, 'label': u'CollateralHeader'}(u'Prior to accepting this mission, the following must be provided by you as collateral, to be returned to you upon successful completion of the mission:', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235525, 'label': u'DungeonObjectiveBody'}(u'Destroy these targets', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235526, 'label': u'CollateralTitle'}(u'Collateral', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235528, 'label': u'ObjectiveLocation'}(u'Location', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235537, 'label': u'GrantedItems'}(u'Granted Items', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235538, 'label': u'MissionExpirationTitle'}(u'Mission Expiration', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235539, 'label': u'OptionalObjectiveBody'}(u'Optionally, destroy these targets', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235540, 'label': u'DungeonObjectiveNormalRestrictions'}(u'This site contains normal {startHttpLink}ship restrictions{endHttpLink}.', None, {u'{startHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'startHttpLink'}, u'{endHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'endHttpLink'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235541, 'label': u'DungeonObjectiveSpecialRestrictions'}(u'This site contains special {startHttpLink}ship restrictions{endHttpLink}.', None, {u'{startHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'startHttpLink'}, u'{endHttpLink}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'endHttpLink'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235542, 'label': u'FetchObjectiveBlurb'}(u'Acquire these goods:', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235543, 'label': u'FetchObjectiveItem'}(u'Item', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235544, 'label': u'OptionalObjectiveHeader'}(u'Optional Objective', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235545, 'label': u'FetchObjectiveHeader'}(u'Bring Item Objective', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235546, 'label': u'FetchObjectiveDropOffLocation'}(u'Drop-off Location', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235547, 'label': u'GrantedItemDetail'}(u'The following item will be granted to you when the mission is accepted', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235548, 'label': u'MissionBriefing'}(u'Mission briefing', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235549, 'label': u'MissionObjectives'}(u'{missionName} Objectives', None, {u'{missionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionName'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235550, 'label': u'MissionObjectivesComplete'}(u'{missionName} Objectives Complete', None, {u'{missionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionName'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235551, 'label': u'OverviewAndObjectivesBlurb'}(u'The following objectives must be completed to finish the mission:', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235552, 'label': u'NumResearchPoints'}(u'{[numeric]rpAmount} Research Points.', None, {u'{[numeric]rpAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rpAmount'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235553, 'label': u'NumLoyaltyPoints'}(u'{[numeric]lpAmount} Loyalty Points.', None, {u'{[numeric]lpAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'lpAmount'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235554, 'label': u'MissionReferral'}(u'Referral to {[character]agentID.name}', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235555, 'label': u'ImportantStandingsWarning'}(u'<b><i>This is an important mission, which will have significant impact on your faction standings.</i></b>', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235560, 'label': u'QuitImportantMissionMessage'}(u'If you quit this important mission you will lose a lot of standings with your agent, as well as his corp and faction. If you lose enough standings, you will no longer be able to talk to the agent. Are you sure you would like to quit this mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235561, 'label': u'QuitMissionTitle'}(u'Quit Mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235562, 'label': u'QuitMissionMessage'}(u'If you quit this mission you will lose standings with your agent, as well as his corp and faction.  If you lose enough standings, you will no longer be able to talk to the agent.  Are you sure you would like to quit this mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235563, 'label': u'RewardsTitle'}(u'Rewards', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235564, 'label': u'TransportPickupLocation'}(u'Pickup Location', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235565, 'label': u'QuitMissionNoMoreMissions'}(u'{[character]agentID.name} has no other missions to offer right now. Are you sure you want to quit?', None, {u'{[character]agentID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235566, 'label': u'TransportDropOffLocation'}(u'Drop-off Location', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235567, 'label': u'TransportObjectiveHeader'}(u'Transport Objective', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235568, 'label': u'CantQuitRemotely'}(u'You cannot quit this mission remotely; go to {[character]agentID.nameWithPossessive} location and talk to {[character]agentID.gender -> "him", "her"}  in person.', None, {u'{[character]agentID.nameWithPossessive}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'nameWithPossessive', 'args': 0, 'kwargs': {}, 'variableName': 'agentID'}, u'{[character]agentID.gender -> "him", "her"}': {'conditionalValues': [u'him', u'her'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235569, 'label': u'RewardsHeader'}(u'The following rewards will be yours if you complete this mission:', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235570, 'label': u'QuitImportantMissionTitle'}(u'Quit Important Mission?', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235571, 'label': u'TransportBlurb'}(u'Transport these goods:', None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 235572, 'label': u'TransportCargo'}(u'Cargo', None, None)
 {'FullPath': u'UI/Agents/Dialogue', 'messageID': 235573, 'label': u'ThisOfferExpiresAt'}(u'This offer expires at {[datetime]expireTime}', None, {u'{[datetime]expireTime}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'expireTime'}})
 {'FullPath': u'UI/Agents/Dialogue', 'messageID': 235574, 'label': u'ThisMissionExpiresAt'}(u'This mission expires at {[datetime]expireTime}', None, {u'{[datetime]expireTime}': {'conditionalValues': [], 'variableType': 6, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'expireTime'}})
 {'FullPath': u'UI/Agents', 'messageID': 235584, 'label': u'BookmarkObsolete'}(u'No mission in progress, this bookmark is obsolete.', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235585, 'label': u'ApproachLocationNotAccepted'}(u"You haven't accepted the mission yet, so there's no point in going there.", None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235586, 'label': u'TooCloseToWarp'}(u'Too close to warp', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235587, 'label': u'TooFarToApproach'}(u'Too far away to approach', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235588, 'label': u'BookmarkWrongSystem'}(u'That bookmark is located in {[location]locSystem.name} at ({[numeric]x}, {[numeric]y}, {[numeric]z}), not in {[location]charSystem.name}.', None, {u'{[numeric]z}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'z'}, u'{[location]charSystem.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'charSystem'}, u'{[numeric]x}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'x'}, u'{[numeric]y}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'y'}, u'{[location]locSystem.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'locSystem'}})
 {'FullPath': u'UI/Agents/EpicArcs', 'messageID': 235786, 'label': u'QuitMissionTerminateArc'}(u'This mission is part of an Epic Arc.  If you quit or decline now, the arc will end here.  You may restart it in three months, but your progress up to this point will be lost. Are you sure you want to quit?', None, None)
 {'FullPath': u'UI/Agents/EpicArcs', 'messageID': 235787, 'label': u'QuitMissionGoToEarlierMission'}(u'Quitting will reset the status of this mission, and may cause you to return to an earlier point, erasing all record of the accomplishments you have made since then. Are you sure you want to quit?', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235791, 'label': u'Availability'}(u'Availability', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235792, 'label': u'NotAvailableInProgress'}(u'Not until the prior operation has completed', None, None): u'SecurityDescription_10'}(u'They were trustworthy and like a deity to all of us.', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235826, 'label': u'SameConstellation'}(u'Same Constellation', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235827, 'label': u'AvailableAgain'}(u'Available again', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235828, 'label': u'EveryInterval'}(u'Every {[timeinterval]interval.writtenForm}', None, {u'{[timeinterval]interval.writtenForm}': {'conditionalValues': [], 'variableType': 8, 'propertyName': 'writtenForm', 'args': 0, 'kwargs': {}, 'variableName': 'interval'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235829, 'label': u'DifferentRegion'}(u'Different Region', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235830, 'label': u'MaxFrequency'}(u'Max Frequency', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235831, 'label': u'LocationServices'}(u'Location Services', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235832, 'label': u'ResultsInstantaneous'}(u'instantaneous', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235833, 'label': u'SameRegion'}(u'Same Region', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235834, 'label': u'SameSolarSystem'}(u'Same Solar System', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235838, 'label': u'LocatedEmailIntro1'}(u"I've found your sleazebag.", None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235839, 'label': u'LocatedEmailIntro2'}(u"I've found your scumsucker.", None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235840, 'label': u'LocatedEmailHeader'}(u'I found {[character]charID.name, linkinfo=linkdata} for you.', None, {u'{[character]charID.name, linkinfo=linkdata}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {'linkinfo': 'linkdata'}, 'variableName': 'charID'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235843, 'label': u'InOtherConstellation'}(u'{[character]charID.gender -> "He", "She"} is in the {systemName} system of the {constellationName} constellation.', None, {u'{constellationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'constellationName'}, u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{systemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'systemName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235844, 'label': u'InOtherRegion'}(u'{[character]charID.gender -> "He", "She"} is in the {systemName} system, {constellationName} constellation of the {regionName} region.', None, {u'{constellationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'constellationName'}, u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{regionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'regionName'}, u'{systemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'systemName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235845, 'label': u'InYourSystem'}(u'{[character]charID.gender -> "He", "She"} is in your solar system.', None, {u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235846, 'label': u'InYourStation'}(u'{[character]charID.gender -> "He", "She"} is at your station.', None, {u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235847, 'label': u'InYourSystemInStation'}(u'{[character]charID.gender -> "He", "She"} is at {stationName} station in your solar system.', None, {u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{stationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'stationName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235848, 'label': u'InOtherSystemInStation'}(u'{[character]charID.gender -> "He", "She"} is at {stationName} station in the {systemName} system.', None, {u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{stationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'stationName'}, u'{systemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'systemName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235849, 'label': u'InOtherConstellationInStation'}(u'{[character]charID.gender -> "He", "She"} is at{stationName} station in the {systemName} system of the {constellationName} constellation.', None, {u'{constellationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'constellationName'}, u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{stationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'stationName'}, u'{systemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'systemName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235850, 'label': u'InOtherRegionInStation'}(u'{[character]charID.gender -> "He", "She"} is at {stationName} station in the {systemName} system, {constellationName} constellation of {regionName} region.', None, {u'{constellationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'constellationName'}, u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{regionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'regionName'}, u'{stationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'stationName'}, u'{systemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'systemName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235851, 'label': u'InOtherSystem'}(u'{[character]charID.gender -> "He", "She"} is in the {systemName} system.', None, {u'{[character]charID.gender -> "He", "She"}': {'conditionalValues': [u'He', u'She'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'charID'}, u'{systemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'systemName'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 235855, 'label': u'LocatedEmailGoodbyeText'}(u'With regards,<br>\n{[character]agentID.name, linkinfo=linkdata}', None, {u'{[character]agentID.name, linkinfo=linkdata}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {'linkinfo': 'linkdata'}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents', 'messageID': 235873, 'label': u'MissionAvailabilityNone'}(u'N/A.  This agent does not have any missions', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235874, 'label': u'MissionAvailabilityStandard'}(u'This agent provides missions to any pilot that meets standing requirements', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235875, 'label': u'MissionAvailability'}(u'Mission Availability', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 235876, 'label': u'MissionServices'}(u'Mission Services', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 235917, 'label': u'ResearchFieldBonusRPMultiplier'}(u'{[numeric]rpMultiplier} x RP/day', None, {u'{[numeric]rpMultiplier}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rpMultiplier'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 235918, 'label': u'ResearchFieldBonus'}(u'Research Field Bonus', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 235919, 'label': u'YourResearch'}(u'Your Research', None, None) 'label': u'bodyResearchMissionAvailable'}(u"Our research has been fruitful, but I've encountered a snag and our research has been halted.  Please contact me as soon as possible.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236025, 'label': u'NotEnoughRPForDatacores'}(u"Sorry mate you don't have enough research points to buy any datacores. <br>Each {[item]datacoreTypeID.name} costs {[numeric]rpAmount} research points.", None, {u'{[item]datacoreTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'datacoreTypeID'}, u'{[numeric]rpAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rpAmount'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236026, 'label': u'DatacorePrice'}(u'{[item]datacoreTypeID.name}: {[numeric]rpAmount} RP', None, {u'{[item]datacoreTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'datacoreTypeID'}, u'{[numeric]rpAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rpAmount'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236027, 'label': u'ResearchField'}(u'Research Field', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236028, 'label': u'ResearchRate'}(u'Research Rate', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236029, 'label': u'ResearchServices'}(u'Research Services', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236030, 'label': u'CurrentStatusRP'}(u'{[numeric]rpAmount, decimalPlaces=2} RP', None, {u'{[numeric]rpAmount, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'rpAmount'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236032, 'label': u'CurrentStatus'}(u'Current Status', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236033, 'label': u'SkillListing'}(u'{[item]skillID.name} level {[numeric]skillLevel}', None, {u'{[item]skillID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'skillID'}, u'{[numeric]skillLevel}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'skillLevel'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236036, 'label': u'PredictablePatents'}(u'Predictable Patents', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 66646, 'label': u'FactionalWarfare'}(u'Factional Warfare', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236038, 'label': u'RelevantSkills'}(u'Relevant Skills', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236039, 'label': u'SelectResearchTypeTitle'}(u'Select Research Type', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236040, 'label': u'ResearchSummary'}(u'{[item]skillTypeID.name} Research Summary', None, {u'{[item]skillTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'skillTypeID'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236041, 'label': u'NoPredictablePatents'}(u'Nothing easily predictable.', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236050, 'label': u'DeclineSingleMissionTitle'}(u'Decline Storyline Mission?', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236051, 'label': u'DeclineSequenceConfirmMessage'}(u'This mission is part of a storyline sequence.  Declining this particular mission will prevent you from getting offered the rest of the storyline.  So, are you really sure you would like to decline this mission?', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236052, 'label': u'DeclineSequenceConfirmTitle'}(u'Decline Storyline Sequence?', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236055, 'label': u'QuitMissionNoMoreMissions'}(u'This is a storyline mission. This agent will not give you another mission if you quit, and you will no longer be able to work with {[character]agentID.gender -> "him", "her"}. So, are you really sure you would like to quit this mission?', None, {u'{[character]agentID.gender -> "him", "her"}': {'conditionalValues': [u'him', u'her'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236056, 'label': u'QuitSequenceConfirmMessage'}(u'This mission is part of a storyline sequence. Quitting this particular mission will prevent you from getting offered the rest of the storyline. So, are you really sure you would like to quit this mission?', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236057, 'label': u'QuitSequenceConfirmTitle'}(u'Quit Storyline Sequence?', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236058, 'label': u'QuitSingleMissionTitle'}(u'Quit Storyline Mission?', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236059, 'label': u'RemoveMissionConfirmMessage'}(u'You are about to remove a storyline mission from your journal. This agent will not immediately give you another mission if you decline, and you will not be able to work with {[character]agentID.gender -> "him", "her"} until you complete additional standard missions for other agents. So, are you really sure you would like to remove this mission from your journal?', None, {u'{[character]agentID.gender -> "him", "her"}': {'conditionalValues': [u'him', u'her'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236060, 'label': u'RemoveMissionConfirmTitle'}(u'Remove Storyline Mission?', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 236076, 'label': u'NumItemsAndProperties'}(u'{itemAndQuantity} ({propertyList})', None, {u'{itemAndQuantity}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'itemAndQuantity'}, u'{propertyList}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'propertyList'}})
 {'FullPath': u'UI/Agents/Items', 'messageID': 236077, 'label': u'SpecificItems'}(u'specific item', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 236078, 'label': u'BlueprintInfoCopy'}(u'copy', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 236080, 'label': u'BlueprintInfoSingleRun'}(u'single run', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 236081, 'label': u'BlueprintInfoOriginal'}(u'original', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 236082, 'label': u'BlueprintInfoMaterialLevel'}(u'material level: {[numeric]materialLevel}', None, {u'{[numeric]materialLevel}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'materialLevel'}})
 {'FullPath': u'UI/Agents/Items', 'messageID': 236083, 'label': u'ProductivityLevel'}(u'productivity level: {[numeric]productivityLevel}', None, {u'{[numeric]productivityLevel}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'productivityLevel'}})
 {'FullPath': u'UI/Agents/Items', 'messageID': 236084, 'label': u'ItemLocation'}(u'{[item]typeID.name} in {[location]locationID.name}.', None, {u'{[location]locationID.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'locationID'}, u'{[item]typeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'typeID'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236532, 'label': u'LocateWho'}(u'Locate Who?', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236533, 'label': u'EnterCharacterName'}(u'Enter character name', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236031, 'label': u'ResearchRateRPDay'}(u'{[numeric]rpAmount, decimalPlaces=2} RP/day', None, {u'{[numeric]rpAmount, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'rpAmount'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236037, 'label': u'SelectResearchTypeMessage'}(u'What type of research are you interested in?', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236587, 'label': u'CantLocateSelf'}(u'You need a shrink, not an agent.', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236589, 'label': u'CantLocateAgent'}(u"If you can't figure that one out for yourself, you'd better get a new pair of eyes.", None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236614, 'label': u'CharacterDoesntExist'}(u'"{characterSearch}" doesn\'t exist.', None, {u'{characterSearch}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'characterSearch'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236615, 'label': u'TooLowStandings'}(u'I\u2019m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236616, 'label': u'WillFindCharacterWithDelay'}(u"Always a pleasure doing business with you. It'll take me some time to find {[character]locateCharID.name}. I'll be in touch.", None, {u'{[character]locateCharID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'locateCharID'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236617, 'label': u'TooPoorToLocate'}(u"It'd cost you {[numeric]iskAmount} ISK, kid, which is more than you're worth.", None, {u'{[numeric]iskAmount}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'iskAmount'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236618, 'label': u'LocateCharacterPrice'}(u"I have a pretty good hunch where {[character]locateCharID.name} might be. It'll cost you {[numeric]iskAmount.isk} however.", None, {u'{[character]locateCharID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'locateCharID'}, u'{[numeric]iskAmount.isk}': {'conditionalValues': [], 'variableType': 9, 'propertyName': 'isk', 'args': 256, 'kwargs': {}, 'variableName': 'iskAmount'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236620, 'label': u'BusyCantLocate'}(u"I'm busy. Could you come back later?", None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236621, 'label': u'FoundCharacterImmediately'}(u"Always a pleasure doing business with you. I had a hunch you'd be looking for that bum. {[character]locateCharID.name} was just spotted.\n<br><br>\n{locationString}", None, {u'{[character]locateCharID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'locateCharID'}, u'{locationString}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'locationString'}})
 {'FullPath': u'UI/Agents/Locator', 'messageID': 236622, 'label': u'CharacterOutOfRange'}(u"I'm sorry, but I just can't help you with that one. I'm pretty sure {[character]locateCharID.name} is well out of my zone of influence.", None, {u'{[character]locateCharID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'locateCharID'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236681, 'label': u'MissionNotSameFaction'}(u'I will only give out missions to those who have joined the war!', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236682, 'label': u'OfferTooLowStandings'}(u'I\u2019m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236686, 'label': u'RootFactionClosed'}(u"We're closed to the public.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236687, 'label': u'MissionCantQuitInSpace'}(u"I'm sorry, what did you say? There's a bit of interference on the channel at the moment. Perhaps you could call back using a station line?", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236688, 'label': u'MissionDetails'}(u'Sure.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236689, 'label': u'RootCantUseStorylineAgent'}(u"I'm sorry, but I have no business with you at the moment.  Come back once you have a referral.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236690, 'label': u'OfferAccepted'}(u'A wise decision.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236691, 'label': u'MissionTooLowStandings'}(u'I\u2019m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236692, 'label': u'RootRaceClosed'}(u'Get out of my sight, mortal.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236693, 'label': u'OfferDeclined'}(u"It's your loss.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236695, 'label': u'MissionFailed'}(u'Son, I am disappoint.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236696, 'label': u'MissionStorylineTooLowStandings'}(u'Sorry, but I only work with people I trust.  Come back once you have a referral.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236697, 'label': u'RootNonAgent'}(u"I'm busy.  Go away.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236699, 'label': u'RootLevelClosed'}(u"I'm busy, go away.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236701, 'label': u'MissionAcceptFailure'}(u'Oh my! My computer just crashed horribly when I was entering your mission acceptance, and I lost the file on your mission. I knew I should have installed a backup file system... Anyhow, the cause of this has been logged on the server along with a nice stack trace explaining what went wrong, so the proper authorities will undoubtedly repair this mission as soon as possible. Perhaps you would like another mission in the meantime?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236702, 'label': u'MissionCanOnlyAskInPresence'}(u'Sounds good. Please drop by, so we can formalize the mission contract.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236703, 'label': u'MissionNoStorylineMissionNoneAvailable'}(u'Sorry, I have no jobs available for you.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236706, 'label': u'DonationsDonated'}(u'Thanks dude, I owe you one.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236707, 'label': u'MissionCantCompleteInSpace'}(u"I'm sorry, what did you say? There's a bit of interference on the channel at the moment. Perhaps you could call back using a station line?", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236708, 'label': u'CantUseOnTrial'}(u'Trial account users cannot access agents of level 3 or higher. Either use your map settings to find a level 1 or level 2 agent, or consider purchasing an EVE subscription if you wish to access higher-level content.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236710, 'label': u'DonationsTooSoon'}(u'Your generosity astounds me, but I could never forgive myself for taking more of your money so soon.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236712, 'label': u'MissionCanOnlyAcceptInPresence'}(u'Great! Please drop by, so we can formalize the mission contract.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236713, 'label': u'MissionNoStorylineMissionNoneConfigged'}(u'Sorry, I have no jobs for the general public.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236715, 'label': u'OfferAcceptMissingStuffAtStation'}(u"You don't have the items I required at {[location]agentStationID.name}.  The item(s) must be located in your cargo hold or in your personal hangar.  If you have multiple objectives of the same item type in the same location, please use either the hangar or your cargo hold, but not both.  If a specific item was requested as opposed to any item of the specified type, please be sure that the correct specific item is indeed being provided.  Otherwise, please make sure that the item is not assembled, packaged or damaged.  Check the journal for more detail", None, {u'{[location]agentStationID.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentStationID'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236716, 'label': u'OfferAcceptMissingStuffInSpace'}(u"You don't have the items I required in {[location]agentSolarSystemID.name}.  The item(s) must be located in your cargo hold.  If a specific item was requested as opposed to any item of the specified type, please be sure that the correct specific item is indeed being provided.  Otherwise, please make sure that the item is not assembled, packaged or damaged.  Check the journal for more detail", None, {u'{[location]agentSolarSystemID.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentSolarSystemID'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236717, 'label': u'RemoteRootInSpace'}(u'I am currently off in {[location]agentSolarSystemID.name}.  If you want to talk, please meet me in person.', None, {u'{[location]agentSolarSystemID.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentSolarSystemID'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236718, 'label': u'MissionConfirmArcRestart'}(u'Hello {[character]player.name}. Good to see you again. I would like to remind you that you have previously completed this epic arc and for you to embark on it once again I would have to erase all memory of the arc from your mind. Are you sure you would like to start this journey once again and lose all history of this arc?', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236720, 'label': u'Medium1'}(u"So you think you're tough? If that's the case, I might have some use for you.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236721, 'label': u'High1'}(u'You have a long and prosperous future within {[npcOrganization]agentFactionID.name}, {[character]player.name}.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}, u'{[npcOrganization]agentFactionID.name}': {'conditionalValues': [], 'variableType': 1, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentFactionID'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236722, 'label': u'Medium2'}(u'Nice to see you, {[character]player.name}.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236723, 'label': u'High4'}(u'I am exhilarated at seeing you, {[character]player.name}. I hope I can be of assistance.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236724, 'label': u'High3'}(u'Nice to see you {[character]player.name}!', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236725, 'label': u'Low1'}(u'Why hello there. What do you want?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236726, 'label': u'High2'}(u"{[character]player.name}! You're back! Please have a seat and make yourself feel comfortable. If there is anything you need, just ask.", None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236727, 'label': u'Low3'}(u"I hope you're not here to ask me for a favor. I hate beggars.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236728, 'label': u'Low2'}(u'What do you want? Spit it out, stooge.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236729, 'label': u'GenericGreetings'}(u'Greetings, {[character]player.name}.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236730, 'label': u'RootMissionInProgressMedium1'}(u'Nice to see you, {[character]player.name}, but you know you still have an unfinished mission from me, right?', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236731, 'label': u'RootMissionInProgressMedium2'}(u"Always a pleasure, of course. But the last mission I gave you isn't going to complete itself.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236732, 'label': u'RootMissionInProgressMedium3'}(u'Hey, {[character]player.name}. I really like your style... but I do need you to finish up your current mission for me.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236733, 'label': u'RootMissionInProgressHigh1'}(u"{[character]player.name}! You're back! If there is anything you need, just ask. But you are going to finish your current mission, right?", None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236734, 'label': u'RootMissionInProgressHigh2'}(u"Great to see you, {[character]player.name}, as always. I have to assume there's a good reason that you've come back before finishing your current mission??", None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236735, 'label': u'RootMissionInProgressLow1'}(u"You do know you haven't finished your current mission for me, right?", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236736, 'label': u'RootMissionInProgressLow3'}(u'{[character]player.name}, you need to finish your current mission for me.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236737, 'label': u'RootMissionInProgressLow2'}(u"You need to finish your current mission. Don't talk to me until then.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236738, 'label': u'MissionAcceptedLow1'}(u'Very well then, get going.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236739, 'label': u'MissionAcceptedLow2'}(u'Ok, hurry up will you.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236740, 'label': u'MissionAcceptedLow3'}(u'Good good, now get out there and give me some results.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236741, 'label': u'MissionAcceptedLowMed1'}(u'Thank you, and good luck.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236742, 'label': u'MissionAcceptedLowMed2'}(u'Thanks, I really appreciate it.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236743, 'label': u'MissionAcceptedLowMed3'}(u'Have fun!', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236744, 'label': u'MissionAcceptedLowMed4'}(u'Now be careful out there, you hear me?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236745, 'label': u'MissionAcceptedMedHigh1'}(u'Stay alive, friend.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236746, 'label': u'MissionAcceptedMedHigh2'}(u'I knew I could count on you.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236747, 'label': u'MissionAcceptedMedHigh3'}(u'Great.  I know I can trust you with this, mate.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236748, 'label': u'MissionAcceptedHigh1'}(u'Stay alive, friend.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236749, 'label': u'MissionAcceptedHigh2'}(u'Wonderful.  I expect a quick result with you on the job.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236750, 'label': u'MissionAcceptedHigh3'}(u"Fabulous.  I couldn't have asked for a better man for the job.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236751, 'label': u'MissionCompletedLow1'}(u'Not bad.  Get back to me later for another assignment will you?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236752, 'label': u'MissionCompletedLow2'}(u"Nice work.  I'm starting to like your style.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236753, 'label': u'MissionCompletedLow3'}(u'Thanks, your services to {[npcOrganization]agentCorpID.name} is duly appreciated.', None, {u'{[npcOrganization]agentCorpID.name}': {'conditionalValues': [], 'variableType': 1, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentCorpID'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236754, 'label': u'MissionCompletedLowMed1'}(u'Thank you.  Your accomplishment has been noted and saved into our database.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236756, 'label': u'MissionCompletedLowMed3'}(u'Thanks, I really appreciate your help.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236757, 'label': u'MissionCompletedLowMed4'}(u"I won't forget this.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236758, 'label': u'MissionCompletedLowMed5'}(u"I'm grateful for your assistance.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236759, 'label': u'MissionCompletedLowMed6'}(u'You have my gratitude.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236760, 'label': u'MissionCompletedMedHigh1'}(u'Thank you very much, I really appreciate it.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236761, 'label': u'MissionCompletedMedHigh2'}(u'Well done!  Take this reward and my gratitude as well.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236762, 'label': u'MissionCompletedMedHigh3'}(u"It's a pleasure doing business with you.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236763, 'label': u'MissionCompletedMedHigh4'}(u'Excellent work!  Care for another assignment?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236764, 'label': u'MissionCompletedMedHigh5'}(u'I look forward to your next visit.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236765, 'label': u'MissionCompletedMedHigh6'}(u"Again you finish the job right on time.  Keep this up and I'll probably get a promotion.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236766, 'label': u'MissionCompletedHigh1'}(u'I thank you from the bottom of my heart.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236767, 'label': u'MissionCompletedHigh2'}(u'If only I had more excellent pilots like you, {[character]player.name} ...', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236768, 'label': u'MissionCompletedHigh3'}(u'Your talents as a pilot never cease to amaze me.  Keep up the good work!', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236769, 'label': u'MissionCompletedHigh4'}(u'I am in your debt {[character]player.name}.  If you ever need anything, just look me up.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236770, 'label': u'StorylineNoneConfiggedGeneral'}(u'Sorry, I have no jobs for the general public.', None, None)
 {'FullPath': u'UI/Agents/Storyline', 'messageID': 236774, 'label': u'StorylineNoneConfiggedFaction'}(u'{agentFactionName} does not provide work to the general public.', None, {u'{agentFactionName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'agentFactionName'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236775, 'label': u'ImprovingRelationsOverride'}(u"Hello, {[character]player.name}.  I was given the assignment of handing out a valuable item to loyal supporters of {[npcOrganization]agentFactionID.name}.  If you know of someone who has proven him or herself time and time again for {[npcOrganization]agentFactionID.nameWithArticle} then point that person to me and perhaps we can come to an 'arrangement' ...<br><br>You on the other hand do not meet my requirements or have already received my offer.", None, {u'{[npcOrganization]agentFactionID.nameWithArticle}': {'conditionalValues': [], 'variableType': 1, 'propertyName': 'nameWithArticle', 'args': 0, 'kwargs': {}, 'variableName': 'agentFactionID'}, u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}, u'{[npcOrganization]agentFactionID.name}': {'conditionalValues': [], 'variableType': 1, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentFactionID'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236776, 'label': u'BoosterManufacturingOverride'}(u"You haven't heard? Gigantic harvestable gas clouds have been discovered here in {[location]agentConstellationID.name}! All you need is a Gas Cloud Harvester module and the persistence to find one of these clouds and you'll make a fortune! And if you have a Starbase or Outpost nearby, you might even be able to create valuable boosters out of the chemicals. Well, as long as you have a blueprint.<br><br>I've managed to bribe the local pirate captains so that they'll leave me alone, but I'm worried about going deeper into {[location]agentConstellationID.name}. It's pretty dangerous out there.", None, {u'{[location]agentConstellationID.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentConstellationID'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236777, 'label': u'ResearchHigh2'}(u"I take it you are back for some more research?  If so then I'll see if I can assist you.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236778, 'label': u'ResearchLow1'}(u'Why hello there.  What do you want?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236779, 'label': u'ResearchLow5'}(u"If you're here to find an excuse to show off those big, shiny weapons of yours, then I suggest you go find some nincompoop working for the {[npcOrganization]agentFactionID.name} military and leave me alone.  If not, then I might lend you an ear, depending on your worth to me.", None, {u'{[npcOrganization]agentFactionID.name}': {'conditionalValues': [], 'variableType': 1, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentFactionID'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236780, 'label': u'ResearchLow4'}(u'You just interrupted me from my studies.  This better be good ...', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236781, 'label': u'ResearchLow3'}(u'I take it you are interested in scientific studies.  Perhaps I can be of assistance.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236782, 'label': u'ResearchHigh1'}(u'Ah, how lucky I am to see you again!  Then again, luck is an expression for the simple minded, as all occurances can easily be explained with mathematical calculations. Oh what am I rambling about, please have a seat and tell me what is on your mind.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236783, 'label': u'ResearchHigh3'}(u'Welcome, fellow colleague.  I guess you came to discuss my new theory on quantum mechanics, or are you here on other business?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236784, 'label': u'ResearchHigh4'}(u"It's always a pleasure meeting you, {[character]player.name}.  I suppose you are here for some research, or some work perhaps?", None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236785, 'label': u'RootStandingsTooLow4'}(u"What planet were you born on?  Check your standings next time or I'll drill a hole in you.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236786, 'label': u'RootStandingsTooLow1'}(u'Begone, scum.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236787, 'label': u'RootStandingsTooLow3'}(u'Have you even bothered to check your standings.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236788, 'label': u'RootStandingsTooLow2'}(u"You've got a lot of nerve, showing your face around here.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236789, 'label': u'MissionCompletedNextMission'}(u'{missionCompletionText}\n<br><br>\nBy the way, I have another mission prepared for you already...', None, {u'{missionCompletionText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionCompletionText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236790, 'label': u'DonationsTooLowStandings1'}(u"I don't know you well enough to accept your money. Try seeking out an agent further down the corporate ladder until your standings have improved.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236792, 'label': u'DonationsTooLowStandings3'}(u'Nobody sent you? I only work with people I trust. Talk to one of my subordinates until your standings have improved.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236793, 'label': u'MissionDeclined1'}(u"Too bad, I'll try to find someone else then for that job.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236794, 'label': u'MissionDeclined2'}(u"Bah, that mission wasn't that bad. Oh well, wait a bit and I'll come up with something else.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236795, 'label': u'MissionDeclined3'}(u"It's your loss.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236796, 'label': u'MissionDeclined4'}(u"Well, don't expect me to come up with something as good later on.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236797, 'label': u'MissionDeclined5'}(u'Your wayward ways displease me.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236798, 'label': u'NoMission1'}(u'Sorry, kid, nothing at the moment. Could you come back later?', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236799, 'label': u'NoMission2'}(u"I'm sorry, but what I've got is promised to another pilot already. Could you come back later?", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236800, 'label': u'MissionQuitted1'}(u"I had a hunch you wouldn't pull through.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236801, 'label': u'MissionQuitted2'}(u"It's your loss.", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236802, 'label': u'MissionQuitted3'}(u'Your wayward ways displease me, young one.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236803, 'label': u'OfferStillOpen'}(u'Yes my offer is still open.\n<br><br>\n{missionOfferText}', None, {u'{missionOfferText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionOfferText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236804, 'label': u'MissionOffer1'}(u'Yes, I have something for you.<br><br>{missionBriefingText}', None, {u'{missionBriefingText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionBriefingText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236805, 'label': u'MissionOffer2'}(u"Sure I've got something.<br><br>{missionBriefingText}", None, {u'{missionBriefingText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionBriefingText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236806, 'label': u'MissionOffer3'}(u'I have just the thing you want.<br><br>{missionBriefingText}', None, {u'{missionBriefingText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionBriefingText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236807, 'label': u'MissionOffer4'}(u"Something just came up that's right up your alley.<br><br>{missionBriefingText}", None, {u'{missionBriefingText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionBriefingText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236808, 'label': u'MissionOffer5'}(u"For you, my friend, there's always something.<br><br>{missionBriefingText}", None, {u'{missionBriefingText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionBriefingText'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236809, 'label': u'MissionOffer6'}(u'Here is a mission suited for someone of your caliber.<br><br>{missionBriefingText}', None, {u'{missionBriefingText}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'missionBriefingText'}})
 {'FullPath': u'UI/Agents/DefaultMessages/RootAgentSays', 'messageID': 236810, 'label': u'ResearchLow2'}(u'Come to do a little research eh?  I\'m a busy {[character]agentID.gender -> "man", "girl"}, but I guess I can spare a few moments of my time.', None, {u'{[character]agentID.gender -> "man", "girl"}': {'conditionalValues': [u'man', u'girl'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236811, 'label': u'MissionCompletedResearch1'}(u"Good job!  I don't know how we'd be able to complete this research project without you!", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236812, 'label': u'MissionCompletedResearch2'}(u'Nice work.  Our research will definitely benefit from this.', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 236079, 'label': u'BlueprintInfoMultirun'}(u'{[numeric]runsRemaining} runs', None, {u'{[numeric]runsRemaining}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'runsRemaining'}})
 {'FullPath': u'UI/Agents/Research', 'messageID': 236828, 'label': u'DatacoresNoQuota'}(u"I don't have any more datacores to sell for now. You should try again tomorrow.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236831, 'label': u'BoughtDatacores'}(u'I hope those datacores will come in handy for you', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236853, 'label': u'TooLowStandings'}(u'I\u2019m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236874, 'label': u'ResearchStarted'}(u"With you and me working together, I'm sure there will be marvelous breakthroughs.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236875, 'label': u'TooLowRpm'}(u"You don't have sufficient skill as a Research Project Manager to handle any more research projects.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236876, 'label': u'CharacterSkillsNotSufficient'}(u"You don't have the skills to match mine.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236877, 'label': u'ResearchCancelled'}(u"I'm sorry you feel that way, but alas, our research hadn't gotten anywhere yet so no harm done.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236924, 'label': u'SureToCancelResearchAndMission'}(u'Are you sure you want to cancel your current research project?  All your points will be lost and you will quit the current mission you have with this agent.', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236925, 'label': u'SureToCancelResearch'}(u'Are you sure you want to cancel your current research project?  All your points will be lost.', None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236926, 'label': u'CancelResearchQuestion'}(u'Cancel Research?', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 239097, 'label': u'ItemSpawnInsufficientCargoSpaceTitle'}(u'Insufficient Cargo Space', None, None)
 {'FullPath': u'UI/Agents/Items', 'messageID': 239098, 'label': u'ItemSpawnInsufficientCargoSpaceBody'}(u'This item is too large for your cargo hold.  It has been placed into a jet container.', None, None)
 {'FullPath': u'UI/Station/Lobby', 'messageID': 239099, 'label': u'AgentsOfInterest'}(u'Agents of Interest', None, N
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236719, 'label': u'RemoteRootInStation'}(u'My offices are located at {[location]agentStationID.name}.  If you want to talk, please meet me in person.', None, {u'{[location]agentStationID.name}': {'conditionalValues': [], 'variableType': 3, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentStationID'}})
 {'FullPath': u'Agents/Messages/15018 - Intelligence Mining', 'messageID': 142532, 'label': u'null'}(u'I knew that we were taking a gamble by using a capsuleer. Get out of here, and go back to your safety net. Good luck with those bubble camps, though.', None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236755, 'label': u'MissionCompletedLowMed2'}(u'Excellent job!', None, None)
 {'FullPath': u'Agents/Messages/15028 - New Opportunities', 'messageID': 142581, 'label': u'null'}(u"I met with Abdiel Verat, one of the Angel Cartel's capsuleer liaisons.  She had a job for me: scout a nearby wormhole and report the presence of any hostiles. ", None, None)
 {'FullPath': u'UI/Corporations/Common', 'messageID': 60525, 'label': u'CorporateDivisionSecond'}(u'2nd Division', None, None)is job exceeds the limits. Please choose a lower batch quantity.<br>\r\n<br>\r\nProduction time: {productionTime}<br>\r\nLimit: {limit}', None, {u'{productionTime}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'productionTime'}, u'{limit}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}})
 {'FullPath': u'Agents/Messages/15058 - New Opportunities', 'messageID': 142706, 'label': u'null'}(u"Abdiel Verat, Angel Cartel. <br><br>\nI look out for talented people such as you who are interested in work. And times have certainly made our work interesting.<br><br>\nSince you\u2019re an egger, I\u2019m guessing you know your way around wormholes, yes? While they've been a great aid for our scientists, they\u2019ve also brought some problems to our doorstop, some of which is not good for business. <br><br>\nOur explorers have discovered a wormhole in a nearby system, but we cannot spare the manpower to lock it down. If you help us out by scouting the area, then I may have other more lucrative tasks to follow.\n", None, None)
 {'FullPath': u'UI/Agents/DefaultMessages', 'messageID': 236791, 'label': u'DonationsTooLowStandings2'}(u"Your money isn't good enough. Try seeking out an agent further down the corporate ladder until your standings have improved.", None, None)
 {'FullPath': u'UI/Agents/Research', 'messageID': 236827, 'label': u'DatacoreInvalidInput'}(u'What was that? I did not get your last transmission', None, None)
 {'FullPath': u'UI/Agents/Locator', 'messageID': 263917, 'label': u'WhoDoYouWantToFind'}(u'Who do you want to find?', None, None)None, None)
 {'FullPath': u'UI/Agents/StandardMission', 'messageID': 265087, 'label': u'CargoDescriptionWithSize'}(u'{cargoDescription} ({[numeric]size, decimalPlaces=1} m&sup3;)', None, {u'{cargoDescription}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'cargoDescription'}, u'{[numeric]size, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'size'}})
 {'FullPath': u'UI/Agents', 'messageID': 251135, 'label': u'StandardMissionCargoCapWarning'}(u'To accept this mission, your ship would have to have space for {[numeric]cargoUnits, decimalPlaces=2} more cargo units in its cargo hold.', None, {u'{[numeric]cargoUnits, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'cargoUnits'}})
 {'FullPath': u'UI/Agents', 'messageID': 251137, 'label': u'CannotAcceptMission'}(u'Cannot Accept Mission', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 251138, 'label': u'CargoCapacityWarning'}(u'Cargo Capacity Warning', None, None)
 {'FullPath': u'UI/Agents', 'messageID': 251146, 'label': u'StandardMissionAcceptCargoCapWarning'}(u'This mission involves objectives requiring a total capacity of {[numeric]requiredUnits, decimalPlaces=2} cargo units, but your active ship only has space for {[numeric]availableUnits, decimalPlaces=2} more cargo units in its cargo hold.  Accept anyway?', None, {u'{[numeric]requiredUnits, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'requiredUnits'}, u'{[numeric]availableUnits, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'availableUnits'}})
 */

/*
 * typedef enum {
 * 3018681,
 * 3018821,
 * 3018822,
 * 3018823,
 * 3018824,
 * 3018680,
 * 3018817,
 * 3018818,
 * 3018819,
 * 3018820,
 * 3018682,
 * 3018809,
 * 3018810,
 * 3018811,
 * 3018812,
 * 3018678,
 * 3018837,
 * 3018838,
 * 3018839,
 * 3018840,
 * 3018679,
 * 3018841,
 * 3018842,
 * 3018843,
 * 3018844,
 * 3018677,
 * 3018845,
 * 3018846,
 * 3018847,
 * 3018848,
 * 3018676,
 * 3018825,
 * 3018826,
 * 3018827,
 * 3018828,
 * 3018675,
 * 3018805,
 * 3018806,
 * 3018807,
 * 3018808,
 * 3018672,
 * 3018801,
 * 3018802,
 * 3018803,
 * 3018804,
 * 3018684,
 * 3018829,
 * 3018830,
 * 3018831,
 * 3018832,
 * 3018685,
 * 3018813,
 * 3018814,
 * 3018815,
 * 3018816,
 * 3018683,
 * 3018833,
 * 3018834,
 * 3018835,
 * 3018836]
 * }rookieAgentList;
 */
/*
 * auraAgentIDs = [
 * 3019499,
 * 3019493,
 * 3019495,
 * 3019490,
 * 3019497,
 * 3019496,
 * 3019486,
 * 3019498,
 * 3019492,
 * 3019500,
 * 3019489,
 * 3019494]
 */


#endif  // EVE_AGENT_H

