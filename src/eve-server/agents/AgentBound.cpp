
 /**
  * @name AgentBound.cpp
  *   agent specific code
  *    removed from AgentMgrService.cpp
  *
  * @Author:        Allan
  * @date:      26 June 2018
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
#include "../../eve-common/EVE_Missions.h"
//#include "../../eve-common/EVE_Skills.h"
#include "../../eve-common/EVE_Standings.h"

#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "corporation/LPService.h"
#include "EVE_AgentHelpers.h"
#include "agents/AgentBound.h"
#include "agents/AgentMgrService.h"
#include "station/Station.h"
#include "services/ServiceManager.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "system/DestinyManager.h"


namespace {

// Both synthetic security missions have a literal briefing because their
// 6553x IDs are not localized client message IDs. The original Blockade
// briefing remains unchanged; this is only for the new elite mission.
std::string EliteSecurityMissionBriefing(const MissionOffer& offer)
{
    const uint32 kills = std::min<uint32>(offer.courierTypeID, offer.courierAmount);
    // Existing accepted 18-NPC sites retain their actual target and briefing.
    const bool legacyFleet = offer.stateID == Mission::State::Accepted
        && offer.courierAmount > Mission::kEliteSecurityRequiredKills;
    std::string briefing =
        "DED & OFFICER EXPEDITION\n\n"
        "HIGH-THREAT SECURITY ENCOUNTER\n"
        "An organized DED/overseer and pirate-officer force has been detected.\n"
        "Expect varied ship classes, including powerful officers and overseers.\n\n"
        "OBJECTIVES\n";
    if (legacyFleet) {
        briefing +=
            "- Defeat DED and deadspace ships (5)\n"
            "- Destroy faction commander ships (5)\n"
            "- Eliminate pirate officers (5)\n"
            "- Destroy DED overseers (3)\n\n";
    } else {
        briefing +=
            "- Defeat DED and deadspace ships (2)\n"
            "- Destroy faction commander ships (2)\n"
            "- Eliminate pirate officers (2)\n"
            "- Destroy DED overseers (2)\n\n";
    }
    briefing +=
        "WRECK LOOT\n"
        "Elite NPC wrecks carry an appropriate faction, officer or deadspace\n"
        "module from this server's item catalogue. Salvage remains normal.\n\n"
        "COMBAT PROGRESS\n"
        "Hostiles destroyed: ";
    briefing += std::to_string(kills);
    briefing += " / ";
    briefing += std::to_string(offer.courierAmount);
    briefing += "\n\nCOMPLETION\nReturn to the issuing Security Agent when the pocket is cleared."
                "\n\nREWARD\n50,000,000 ISK"
                "\nTIME LIMIT\n120 minutes from acceptance.";
    return briefing;
}

} // namespace

AgentBound::AgentBound(EVEServiceManager& mgr, AgentMgrService& parent, Agent *agt) :
    EVEBoundObject(mgr, parent),
    m_agent(agt)
{
    this->Add("DoAction", &AgentBound::DoAction);
    this->Add("GetAgentLocationWrap", &AgentBound::GetAgentLocationWrap);
    this->Add("GetInfoServiceDetails", &AgentBound::GetInfoServiceDetails);
    this->Add("GetMissionBriefingInfo", &AgentBound::GetMissionBriefingInfo);
    this->Add("GetMissionObjectiveInfo", &AgentBound::GetMissionObjectiveInfo);
    this->Add("GetMissionKeywords", &AgentBound::GetMissionKeywords);
    this->Add("GetMissionJournalInfo", &AgentBound::GetMissionJournalInfo);
    this->Add("GetDungeonShipRestrictions", &AgentBound::GetDungeonShipRestrictions);
    this->Add("RemoveOfferFromJournal", &AgentBound::RemoveOfferFromJournal);
    this->Add("GetOfferJournalInfo", &AgentBound::GetOfferJournalInfo);
    this->Add("GetEntryPoint", &AgentBound::GetEntryPoint);
    this->Add("GotoLocation", &AgentBound::GotoLocation);
    this->Add("WarpToLocation", &AgentBound::WarpToLocation);
    this->Add("GetMyJournalDetails", &AgentBound::GetMyJournalDetails);
}
PyResult AgentBound::GetAgentLocationWrap(PyCallArgs &call) {
    // this is detailed info on agent's location
    return m_agent->GetLocationWrap();
}

PyResult AgentBound::GetInfoServiceDetails(PyCallArgs& call) {
    // this is agents personal info... level, division, station, etc.
    return m_agent->GetInfoServiceDetails();
}


/**     ***********************************************************************
 * @note   these below are partially coded
 */

PyResult AgentBound::DoAction(PyCallArgs &call, std::optional <PyInt*> actionID) {
    // this is first call when initiating agent convo
    _log(AGENT__DUMP,  "AgentBound::Handle_DoAction() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    // this actually returns a complicated tuple depending on other variables involving this agent and char.
    /*
        agentSays, dialogue, extraInfo = self.__GetConversation(agentDialogueWindow, actionID)

    def __GetConversation(self, wnd, actionID):
       tmp = wnd.sr.agentMoniker.DoAction(actionID)
        ret, wnd.sr.oob = tmp
        agentSays, wnd.sr.dialogue = ret
        firstActionID = wnd.sr.dialogue[0][0]
        firstActionDialogue = wnd.sr.dialogue[0][1]
        wnd.sr.agentSays = self.ProcessMessage(agentSays, wnd.sr.agentID)
        return (wnd.sr.agentSays, wnd.sr.dialogue, wnd.sr.oob)
     *
     */

    Character* pchar = call.client->GetChar().get();
    float charStanding = StandingDB::GetStanding(m_agent->GetID(), pchar->itemID());
    float quality = EvEMath::Agent::EffectiveQuality(m_agent->GetQuality(), pchar->GetSkillLevel(EvESkill::Connections), charStanding);
    float bonus = EvEMath::Agent::GetStandingBonus(charStanding, m_agent->GetFactionID(), pchar->GetSkillLevel(EvESkill::Connections), pchar->GetSkillLevel(EvESkill::Diplomacy), pchar->GetSkillLevel(EvESkill::CriminalConnections));
    float standing = EvEMath::Agent::EffectiveStanding(charStanding, bonus);

    std::string response = "";
    bool missionQuit = false, missionCompleted = false, missionDeclined = false;

    PyTuple* agentSays = new PyTuple(2);
    // dialog is button info
    PyList* dialog = new PyList();

    using namespace Dialog::Button;

    // to set 'admin dialog options' (which i dont know wtf they are yet), i *think* you add a tuple of *something* that is NOT dict or int.
    if (false /*admin options*/) {
        PyTuple* adminButton = new PyTuple(2);
        adminButton->SetItem(0, new PyInt(Admin));
        adminButton->SetItem(1, new PyString("Admin Options"));
        dialog->AddItem(adminButton);
    }

    if (m_agent->CanUseAgent(call.client)) {
        switch (actionID.has_value() ? actionID.value()->value() : 0) {
            case 0: {
                //  if char has current mission with this agent, add this one.
                MissionOffer offer = MissionOffer();
                if (m_agent->HasMission(pchar->itemID(), offer)) {
                    PyTuple* button1 = new PyTuple(2);
                        button1->SetItem(0, new PyInt(ViewMission)); // this are buttonIDs which are unique and sequential to each agent, regardless of chars
                        button1->SetItem(1, new PyInt(ViewMission));
                    dialog->AddItem(button1);
                    if (call.client->IsMissionComplete(offer))  {
                        PyTuple* button2 = new PyTuple(2);
                            button2->SetItem(0, new PyInt(Complete));
                            button2->SetItem(1, new PyInt(Complete));
                        dialog->AddItem(button2);
                    }
                    if (offer.missionID == Mission::kEliteSecurityMissionID) {
                        agentSays->SetItem(0, new PyString(EliteSecurityMissionBriefing(offer)));
                        agentSays->SetItem(1, PyStatic.NewNone());
                    } else if (offer.missionID == Mission::kBlockadeSecurityTestMissionID) {
                        const uint32 killsDestroyed =
                            (offer.courierTypeID < offer.courierAmount)
                                ? offer.courierTypeID : offer.courierAmount;
                        std::string briefing =
                            "THE BLOCKADE\n"
                            "ANGEL CARTEL INCURSION\n\n"
                            "SITUATION\n"
                            "An Angel Cartel strike force is operating in this system. "
                            "Destroy every hostile ship and secure the bookmarked mission pocket.\n\n"
                            "MISSION OBJECTIVES\n"
                            "- 8 cruisers\n"
                            "- 4 battlecruisers\n\n"
                            "COMBAT PROGRESS\n"
                            "Kills confirmed: ";
                        briefing += std::to_string(killsDestroyed);
                        briefing += " / ";
                        briefing += std::to_string(offer.courierAmount);
                        briefing += "\n\nCOMPLETION\n"
                            "Return to this agent after the pocket is clear.\n\n"
                            "MISSION REWARD\n"
                            "50,000,000 ISK\n"
                            "TIME LIMIT\n"
                            "120 minutes from acceptance.";
                        agentSays->SetItem(0, new PyString(briefing));
                        agentSays->SetItem(1, PyStatic.NewNone());
                    } else {
                        agentSays->SetItem(0, new PyInt(offer.briefingID));
                        agentSays->SetItem(1, new PyInt(offer.characterID));
                    }
                } else {
                    // dialogue data.  if RequestMission is only option, client auto-responds with DoAction(RequestMission optionID)
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(RequestMission));
                        button2->SetItem(1, new PyInt(RequestMission));
                    dialog->AddItem(button2);
                // response as string for custom data.  response as pyint to use client data (using getlocale shit)
                    // default initial agent response based on agent location, level, bloodline, quality, and char/agent standings
                    //  this will be modeled after UO speech data, in tiers and levels.
                    // if RequestMission is only option, this is ignored.  see note under 'dialog data'
                    response = "Why the fuck am I looking at you again, ";
                    response += call.client->GetName();
                    response += "?";
                    agentSays->SetItem(0, new PyString(response));  //msgInfo  -- if tuple[0].string then return msgInfo
                    agentSays->SetItem(1, PyStatic.NewNone());      // ContentID  -- PyNone used when msgInfo is string (mostly for initial greetings)
                }

                // if agent does location, add this one...
                if (m_agent->IsLocator()) {
                    PyTuple* button3 = new PyTuple(2);
                        button3->SetItem(0, new PyInt(LocateCharacter));
                        button3->SetItem(1, new PyInt(LocateCharacter));
                    dialog->AddItem(button3);
                }

                // if agent does research, add this one...
                if (m_agent->IsResearch()) {
                    PyTuple* button4 = new PyTuple(2);
                        button4->SetItem(0, new PyInt(StartResearch));
                        button4->SetItem(1, new PyInt(StartResearch));
                    dialog->AddItem(button4);
                }
            } break;
            case RequestMission: {  //2
                MissionOffer offer = MissionOffer();
                if (!m_agent->MakeOffer(pchar->itemID(), offer)) {
                    agentSays->SetItem(0, new PyString("No suitable security mission is available from this agent right now. You may already have an offer from this agent, or its mission templates may be unavailable."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }
                m_agent->SendMissionUpdate(call.client, "offered");

                //  this one will get complicated and is based on agent/char interaction
                //   detail in /eve/client/script/ui/station/agents/agents.py

                /*  agentSays is a tuple of msgData and contentID
                *      msgData can be single integer of briefingID, a string literal, or a tuple as defined above.
                *   contentID is used for specific char's mission keywords.  we're not using it like there here....
                */

                if (offer.missionID == Mission::kEliteSecurityMissionID) {
                    agentSays->SetItem(0, new PyString(EliteSecurityMissionBriefing(offer)));
                    agentSays->SetItem(1, PyStatic.NewNone());
                } else if (offer.missionID == Mission::kBlockadeSecurityTestMissionID) {
                    agentSays->SetItem(0, new PyString(
                        "\n\nTHE BLOCKADE\n"
                        "ANGEL CARTEL INCURSION\n\n"
                        "SITUATION\n"
                        "Angel Cartel forces have established a hostile presence in this system. "
                        "Your task is to eliminate the entire strike force and secure the mission pocket.\n\n"
                        "MISSION OBJECTIVES\n"
                        "- Destroy 8 cruisers\n"
                        "- Destroy 4 battlecruisers\n\n"
                        "COMBAT PROGRESS\n"
                        "Kills confirmed: 0 / 12\n\n"
                        "COMPLETION\n"
                        "Return to this agent once all hostile ships have been destroyed.\n\n"
                        "MISSION REWARD\n"
                        "50,000,000 ISK\n"
                        "TIME LIMIT\n"
                        "120 minutes from acceptance."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                } else {
                    agentSays->SetItem(0, new PyInt(offer.briefingID));
                    agentSays->SetItem(1, new PyInt(offer.characterID));
                }

                // dialog can also contain mission data.
                //   set a dialog tuple[1] to dict and fill with MissionBriefingInfo
                PyTuple* button1 = new PyTuple(2);
                    button1->SetItem(0, new PyInt(Accept));
                    button1->SetItem(1, new PyInt(Accept));
                dialog->AddItem(button1);
                PyTuple* button2 = new PyTuple(2);
                    button2->SetItem(0, new PyInt(Decline));
                    button2->SetItem(1, new PyInt(Decline));
                dialog->AddItem(button2);
                PyTuple* button3 = new PyTuple(2);
                    button3->SetItem(0, new PyInt(Defer));
                    button3->SetItem(1, new PyInt(Defer));
                dialog->AddItem(button3);
            } break;
            case ViewMission: { //1
                MissionOffer offer = MissionOffer();
                m_agent->GetOffer(pchar->itemID(), offer);
                if (offer.missionID == Mission::kEliteSecurityMissionID) {
                    agentSays->SetItem(0, new PyString(EliteSecurityMissionBriefing(offer)));
                    agentSays->SetItem(1, PyStatic.NewNone());
                } else if (offer.missionID == Mission::kBlockadeSecurityTestMissionID) {
                    const uint32 killsDestroyed =
                        (offer.courierTypeID < offer.courierAmount)
                            ? offer.courierTypeID : offer.courierAmount;
                    std::string briefing =
                        "THE BLOCKADE\n"
                        "ANGEL CARTEL INCURSION\n\n"
                        "SITUATION\n"
                        "An Angel Cartel strike force is operating in this system. "
                        "Destroy every hostile ship and secure the bookmarked mission pocket.\n\n"
                        "MISSION OBJECTIVES\n"
                        "- 8 cruisers\n"
                        "- 4 battlecruisers\n\n"
                        "COMBAT PROGRESS\n"
                        "Kills confirmed: ";
                    briefing += std::to_string(killsDestroyed);
                    briefing += " / ";
                    briefing += std::to_string(offer.courierAmount);
                    briefing += "\n\nCOMPLETION\n"
                        "Return to this agent after the pocket is clear.\n\n"
                        "MISSION REWARD\n"
                        "50,000,000 ISK\n"
                        "TIME LIMIT\n"
                        "120 minutes from acceptance.";
                    agentSays->SetItem(0, new PyString(briefing));
                    agentSays->SetItem(1, PyStatic.NewNone());
                } else {
                    agentSays->SetItem(0, new PyInt(offer.briefingID));
                    agentSays->SetItem(1, new PyInt(offer.characterID));
                }
                if (offer.stateID < Mission::State::Accepted) {
                    PyTuple* button1 = new PyTuple(2);
                        button1->SetItem(0, new PyInt(Accept));
                        button1->SetItem(1, new PyInt(Accept));
                    dialog->AddItem(button1);
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(Decline));
                        button2->SetItem(1, new PyInt(Decline));
                    dialog->AddItem(button2);
                    PyTuple* button3 = new PyTuple(2);
                        button3->SetItem(0, new PyInt(Defer));
                        button3->SetItem(1, new PyInt(Defer));
                    dialog->AddItem(button3);
                } else if (offer.stateID == Mission::State::Accepted) {
                    PyTuple* button1 = new PyTuple(2);
                        button1->SetItem(0, new PyInt(Quit));
                        button1->SetItem(1, new PyInt(Quit));
                    dialog->AddItem(button1);
                    if (call.client->IsMissionComplete(offer))  {
                        PyTuple* button2 = new PyTuple(2);
                            button2->SetItem(0, new PyInt(Complete));
                            button2->SetItem(1, new PyInt(Complete));
                        dialog->AddItem(button2);
                    }
                }
            } break;
            case Accept:            //3
            case AcceptRemotely: {  //5
                MissionOffer offer = MissionOffer();
                if (!m_agent->HasMission(pchar->itemID(), offer)
                    || (offer.stateID != Mission::State::Offered
                        && offer.stateID != Mission::State::Allocated)
                    || (offer.expiryTime > 0 && offer.expiryTime <= GetFileTimeNow())) {
                    agentSays->SetItem(0, new PyString("This mission offer is no longer available."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }
                if (offer.typeID == Mission::Type::Encounter && !call.client->StartEncounterMission(offer)) {
                    call.client->SendErrorMsg("Unable to create the encounter site in this solar system. The mission was not accepted.");
                    agentSays->SetItem(0, new PyString("I cannot establish the encounter site right now. Please try again."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }

                offer.stateID = Mission::State::Accepted;
                offer.dateAccepted = GetFileTimeNow();
                // The synthetic Blockade test mission is Level 0, so the normal
                // level-based formula would give it no time to complete.
                // Give this combat test a full two hours after acceptance.
                const uint32 missionDurationMinutes =
                    (offer.missionID == Mission::kBlockadeSecurityTestMissionID)
                        ? Mission::kBlockadeSecurityTestDurationMinutes
                        : ((offer.missionID == Mission::kEliteSecurityMissionID)
                            ? Mission::kEliteSecurityDurationMinutes
                            : (30 * m_agent->GetLevel()));
                offer.expiryTime = GetFileTimeNow() + (missionDurationMinutes * EvE::Time::Minute);
                if (offer.typeID != Mission::Type::Encounter && offer.courierTypeID) {
                    sItemFactory.SetUsingClient(call.client);
                    ItemData data(offer.courierTypeID, pchar->itemID(), locTemp, flagNone, offer.courierAmount);
                    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
                    if (iRef.get() != nullptr)
                        iRef->Move(offer.originID, flagHangar, true);
                    sItemFactory.UnsetUsingClient();
                }
                m_agent->UpdateOffer(pchar->itemID(), offer);
                m_agent->SendMissionUpdate(call.client, "offer_accepted");
                agentSays->SetItem(0, new PyInt(m_agent->GetAcceptRsp(pchar->itemID())));
                agentSays->SetItem(1, new PyInt(pchar->itemID()));
            } break;
            case Complete:              //6
            case CompleteRemotely: {    //7
                MissionOffer offer = MissionOffer();
                if (!m_agent->HasMission(pchar->itemID(), offer)
                    || offer.stateID != Mission::State::Accepted
                    || (offer.expiryTime > 0 && offer.expiryTime <= GetFileTimeNow())) {
                    agentSays->SetItem(0, new PyString("There is no active mission available for completion."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }
                if (offer.typeID == Mission::Type::Encounter && !call.client->IsMissionComplete(offer)) {
                    call.client->SendErrorMsg("The encounter objectives have not been completed.");
                    agentSays->SetItem(0, new PyString("Destroy all of the required hostile ships before claiming this reward."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }

                offer.stateID = Mission::State::Completed;
                offer.dateCompleted = GetFileTimeNow();
                m_agent->UpdateOffer(pchar->itemID(), offer);
                m_agent->SendMissionUpdate(call.client, "completed");
                agentSays->SetItem(0, new PyInt(m_agent->GetCompleteRsp(pchar->itemID())));
                agentSays->SetItem(1, new PyInt(pchar->itemID()));

                if (offer.typeID != Mission::Type::Encounter && offer.courierTypeID)
                    call.client->RemoveMissionItem(offer.courierTypeID, offer.courierAmount);
                if (offer.rewardItemID) {
                    sItemFactory.SetUsingClient(call.client);
                    ItemData data(offer.rewardItemID, pchar->itemID(), locTemp, flagNone, offer.rewardItemQty);
                    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
                    if (iRef.get() != nullptr)
                        iRef->Move(m_agent->GetStationID(), flagHangar, true);
                    sItemFactory.UnsetUsingClient();
                }
                if (offer.rewardISK)
                    AccountService::TransferFunds(m_agent->GetID(), pchar->itemID(), offer.rewardISK,
                            "Mission Reward", Journal::EntryType::AgentMissionReward, m_agent->GetID());
                if (EvEAgent::BonusWithinDeadline(offer.dateAccepted, offer.bonusTime,
                                                    GetFileTimeNow(), offer.bonusISK))
                    AccountService::TransferFunds(m_agent->GetID(), pchar->itemID(), offer.bonusISK,
                            "Mission Bonus Reward", Journal::EntryType::AgentMissionTimeBonusReward, m_agent->GetID());
                if (offer.rewardLP)
                    LPService::AddLP(pchar->itemID(), m_agent->GetCorpID(), offer.rewardLP);

                if (offer.typeID == Mission::Type::Encounter)
                    call.client->RemoveEncounterMissionSite(offer);

                m_agent->UpdateStandings(call.client, Standings::MissionCompleted, offer.important);
                // Keep completed missions in history rather than the active index.
                sMissionDataMgr.ArchiveMissionOffer(pchar->itemID(), offer);
                m_agent->RemoveOffer(pchar->itemID());
                // Tell the Crucible client that this conversation completed
                // its mission (the existing response flag was always false).
                missionCompleted = true;
            } break;
            case Defer: {   //10
                // extend expiry time and close
                MissionOffer offer = MissionOffer();
                if (m_agent->HasMission(pchar->itemID(), offer)
                    && offer.stateID == Mission::State::Offered
                    && offer.expiryTime > GetFileTimeNow()) {
                    offer.stateID = Mission::State::Allocated; //Defered
                    offer.expiryTime += EvE::Time::Day;
                    m_agent->UpdateOffer(pchar->itemID(), offer);
                    m_agent->SendMissionUpdate(call.client, "prolong");
                    agentSays->SetItem(0, new PyString("I can give you 24 hours to think about it."));    //msgInfo  -- if tuple[0].string then return msgInfo
                    agentSays->SetItem(1, PyStatic.NewNone());    // ContentID  -- PyNone used when msgInfo is string to return without processing
                }
            } break;
            case Decline: { //9
                MissionOffer offer = MissionOffer();
                if (!m_agent->HasMission(pchar->itemID(), offer)
                    || (offer.stateID != Mission::State::Offered
                        && offer.stateID != Mission::State::Allocated)) {
                    agentSays->SetItem(0, new PyString("There is no mission offer to decline."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }
                missionDeclined = true;
                m_agent->DeleteOffer(pchar->itemID());
                m_agent->SendMissionUpdate(call.client, "offer_declined");
                agentSays->SetItem(0, new PyInt(m_agent->GetDeclineRsp(pchar->itemID())));
                agentSays->SetItem(1, new PyInt(pchar->itemID()));
                /** @todo  add lp, etc, etc  */
                m_agent->UpdateStandings(call.client, Standings::MissionDeclined);
            } break;
            case Quit: {    //11
                MissionOffer offer = MissionOffer();
                if (!m_agent->HasMission(pchar->itemID(), offer)
                    || offer.stateID != Mission::State::Accepted) {
                    agentSays->SetItem(0, new PyString("There is no active mission to quit."));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    break;
                }
                missionQuit = true;
                if (offer.typeID != Mission::Type::Encounter && offer.courierTypeID)
                    call.client->RemoveMissionItem(offer.courierTypeID, offer.courierAmount);
                if (offer.typeID == Mission::Type::Encounter)
                    call.client->RemoveEncounterMissionSite(offer);
                m_agent->DeleteOffer(pchar->itemID());
                m_agent->SendMissionUpdate(call.client, "quit");
                agentSays->SetItem(0, new PyInt(m_agent->GetDeclineRsp(pchar->itemID())));
                agentSays->SetItem(1, new PyInt(pchar->itemID()));
                m_agent->UpdateStandings(call.client, Standings::MissionFailure, offer.important);
            } break;
            case Continue: {    //8
                // not sure what to do here.
            } break;
            case StartResearch: {   //12
                // not sure what to do here.
            } break;
            case CancelResearch: {  //13
                // not sure what to do here.
            } break;
            case BuyDatacores: {    //14
                // not sure what to do here.
            } break;
            case LocateCharacter:   //15
            case LocateAccept: {    //16
                // not sure what to do here.
            } break;
            case LocateReject: {    //17
                // not sure what to do here.
            } break;
            case Yes: {             //18
                // not sure what to do here.
            } break;
            case No: {              //19
                // not sure what to do here.
            } break;
            case AcceptChoice:{     //4
                // i think this is for options
            } break;
            case Admin: {           //20
                // not sure what to do here.
            } break;
            default: {
                // error
                _log(AGENT__ERROR, "AgentBound::Handle_DoAction() - unhandled buttonID %u", actionID );
                call.client->SendErrorMsg("Internal Server Error. Ref: ServerError xxxxx.");
                PySafeDecRef(dialog);
                PySafeDecRef(agentSays);
                return nullptr;
            }
        }
    } else {
        agentSays->SetItem(0, new PyInt(m_agent->GetStandingsRsp(pchar->itemID())));
        agentSays->SetItem(1, PyStatic.NewNone() /*new PyInt(pchar->itemID())*/);
    }

    // extraInfo data....
    PyDict* xtraInfo = new PyDict();
        xtraInfo->SetItemString("loyaltyPoints",    new PyInt(LPService::GetLPBalanceForCorp(pchar->itemID(),m_agent->GetCorpID())));  // this is char current LP
        xtraInfo->SetItemString("missionCompleted", new PyBool(missionCompleted));
        xtraInfo->SetItemString("missionQuit",      new PyBool(missionQuit));
        xtraInfo->SetItemString("missionDeclined",  new PyBool(missionDeclined));

    if (agentSays->empty()) {
        agentSays->SetItem(0, PyStatic.NewNone());  // briefingID
        agentSays->SetItem(1, PyStatic.NewNone());  // ContentID
    }
    PyTuple* inner = new PyTuple(2);
        inner->SetItem(0, agentSays);
        inner->SetItem(1, dialog);
    PyTuple* outer = new PyTuple(2);
        outer->SetItem(0, inner);
        outer->SetItem(1, xtraInfo);

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_DoAction RSP:" );
        outer->Dump(AGENT__RSP_DUMP, "    ");
    }

    PySafeDecRef(agentSays);

    return outer;
}

PyResult AgentBound::GetMissionBriefingInfo(PyCallArgs &call) {
    // called from iniate agent convo... should be populated when mission available
    // will return PyNone if no mission avalible
    _log(AGENT__MESSAGE,  "AgentBound::Handle_GetMissionBriefingInfo()");

    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    switch (offer.stateID) {
        //case Mission::State::Allocated:
        case Mission::State::Accepted:
        case Mission::State::Failed:
        case Mission::State::Completed:
        case Mission::State::Rejected:
        case Mission::State::Defered:
        case Mission::State::Expired: {
            return PyStatic.NewNone();
        }
    }

    // these are found in the client data by MessageIDs ....  i.e.  {[location]objectiveDestinationID.name}
    // contentID is the key for the keywords data on live...not used here
    PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(offer.originID));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(offer.originSystemID));
        keywords->SetItemString("objectiveTypeID", new PyInt(offer.typeID == Mission::Type::Encounter ? 0 : offer.courierTypeID));
        keywords->SetItemString("objectiveQuantity", new PyInt(offer.courierAmount));
        keywords->SetItemString("objectiveDestinationID", new PyInt(offer.destinationID));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(offer.destinationSystemID));
        if (offer.rewardISK) {
            keywords->SetItemString("rewardTypeID", new PyInt(itemTypeCredits));
            keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardISK));
        } else {
            // wouldnt these be in 'extra' or ?
            keywords->SetItemString("rewardTypeID", new PyInt(offer.rewardItemID));
            keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardItemQty));
        }
        keywords->SetItemString("dungeonLocationID", new PyInt(offer.dungeonLocationID));
        keywords->SetItemString("dungeonSolarSystemID", new PyInt(offer.dungeonSolarSystemID));
    PyDict *briefingInfo = new PyDict();
        briefingInfo->SetItemString("ContentID", new PyInt(offer.characterID));
        briefingInfo->SetItemString("Mission Keywords", keywords);
        if (Mission::IsSyntheticSecurityEncounter(offer.missionID)) {
            // The synthetic mission has no localization message IDs. Returning
            // integer zero makes the client render the literal "message 0".
            briefingInfo->SetItemString("Mission Title ID", PyStatic.NewNone());
            briefingInfo->SetItemString("Mission Briefing ID", PyStatic.NewNone());
        } else {
            briefingInfo->SetItemString("Mission Title ID", new PyInt(offer.missionID));
            briefingInfo->SetItemString("Mission Briefing ID", new PyInt(offer.briefingID));
        }
        if (offer.missionID == Mission::kEliteSecurityMissionID) {
            briefingInfo->SetItemString("Mission Title",
                new PyString("DED & Officer Expedition"));
            briefingInfo->SetItemString("Mission Briefing",
                new PyString(EliteSecurityMissionBriefing(offer)));
        } else if (offer.missionID == Mission::kBlockadeSecurityTestMissionID) {
            const uint32 killsDestroyed =
                (offer.courierTypeID < offer.courierAmount)
                    ? offer.courierTypeID : offer.courierAmount;
            std::string missionBriefing =
                "ANGEL CARTEL INCURSION\n\n"
                "SITUATION\n"
                "An Angel Cartel strike force has established a hostile presence in this system. "
                "Eliminate the entire force and secure the bookmarked mission pocket.\n\n"
                "MISSION OBJECTIVES\n"
                "- 8 cruisers\n"
                "- 4 battlecruisers\n\n"
                "COMBAT PROGRESS\n"
                "Kills confirmed: ";
            missionBriefing += std::to_string(killsDestroyed);
            missionBriefing += " / ";
            missionBriefing += std::to_string(offer.courierAmount);
            missionBriefing += "\n\nCOMPLETION\n"
                "Return to your agent once all hostile ships have been destroyed.\n\n"
                "REWARD\n"
                "50,000,000 ISK\n\n"
                "TIME LIMIT\n"
                "120 minutes from acceptance.";
            briefingInfo->SetItemString("Mission Title", new PyString("The Blockade - Angel Cartel"));
            briefingInfo->SetItemString("Mission Briefing", new PyString(missionBriefing));
        }
        switch(offer.typeID) {
            case Mission::Type::Courier:
                briefingInfo->SetItemString("Mission Image", sMissionDataMgr.GetCourierRes()); break;
            case Mission::Type::Mining:
                briefingInfo->SetItemString("Mission Image", sMissionDataMgr.GetMiningRes()); break;
            case Mission::Type::Encounter:
                briefingInfo->SetItemString("Mission Image", sMissionDataMgr.GetKillRes()); break;
        }
        // decline time OR expiration time.  if not decline then expiration
        briefingInfo->SetItemString("Decline Time", PyStatic.NewNone());   // -1 is generic decline msg
        briefingInfo->SetItemString("Expiration Time", new PyLong(offer.expiryTime) );

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionBriefingInfo() RSP:" );
        briefingInfo->Dump(AGENT__RSP_DUMP, "    ");
    }

    return briefingInfo;
}

PyResult AgentBound::GetMissionKeywords(PyCallArgs &call, PyInt* contentID) {
    // thse are the variables embedded in the messageIDs
    //self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionKeywords() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    /*   none of this really matters as we're not using 'contentID' like live does
    Call_SingleArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 contentID = PyRep::IntegerValueU32(args.arg);
    if (contentID == 0)
        return PyStatic.NewNone();
    */
    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    PyDict* keywords = new PyDict();
    keywords->SetItemString("objectiveLocationID", new PyInt(offer.originID));
    keywords->SetItemString("objectiveLocationSystemID", new PyInt(offer.originSystemID));
    keywords->SetItemString("objectiveTypeID", new PyInt(offer.typeID == Mission::Type::Encounter ? 0 : offer.courierTypeID));
    keywords->SetItemString("objectiveQuantity", new PyInt(offer.courierAmount));
    keywords->SetItemString("objectiveDestinationID", new PyInt(offer.destinationID));
    keywords->SetItemString("objectiveDestinationSystemID", new PyInt(offer.destinationSystemID));
    if (offer.rewardISK) {
        keywords->SetItemString("rewardTypeID", new PyInt(itemTypeCredits));
        keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardISK));
    } else {
        // wouldnt these be in 'extra' or ?
        keywords->SetItemString("rewardTypeID", new PyInt(offer.rewardItemID));
        keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardItemQty));
    }
    keywords->SetItemString("dungeonLocationID", new PyInt(offer.dungeonLocationID));
    keywords->SetItemString("dungeonSolarSystemID", new PyInt(offer.dungeonSolarSystemID));

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionKeywords() RSP:" );
        keywords->Dump(AGENT__RSP_DUMP, "    ");
    }

    return keywords;
}

PyResult AgentBound::GetMissionObjectiveInfo(PyCallArgs &call, std::optional <PyInt*> characterID, std::optional <PyInt*> contentID)
{
    // sends charID, contentID (although there's another call without any parameters, hence the optionals used)
    // returns PyDict loaded with mission info  or PyNone
    //  returning mission info sets double-pane view, where PyNone sets single-pane view
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    // Both RPC variants must load the real player's mission first.
    // Previously only the zero-argument path loaded it; requests with
    // characterID/contentID silently returned a default/empty offer.
    const uint32 callerID = call.client->GetCharacterID();
    if (characterID.has_value() && characterID.value()->value() != callerID)
        return PyStatic.NewNone();
    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(callerID, offer))
        return PyStatic.NewNone();

    if (call.tuple->size() == 0) {
        switch (offer.stateID) {
            case Mission::State::Accepted:
            case Mission::State::Completed:
            case Mission::State::Rejected:
            case Mission::State::Defered:
            case Mission::State::Expired:
                return PyStatic.NewNone();
        }
    }
    return GetMissionObjectiveInfo(call.client, offer);
}

PyResult AgentBound::GetMyJournalDetails(PyCallArgs &call) {
    //parallelCalls.append((sm.GetService('agents').GetAgentMoniker(agentID).GetMyJournalDetails, ()))
    //missionState, importantMission, missionType, missionName, agentID, expirationTime, bookmarks, remoteOfferable, remoteCompletable = each
    // this is to update ONLY info with this agent....
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMyJournalDetails() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    PyTuple *tuple = new PyTuple(2);
    //missions:
    PyList* missions = new PyList();
    MissionOffer offer = MissionOffer();
    if (m_agent->HasMission(call.client->GetCharacterID(), offer)) {
        if (offer.stateID < Mission::State::Completed) {
            PyTuple* mData = new PyTuple(9);
                mData->SetItem(0, new PyInt(offer.stateID)); //missionState  .. these may be wrong also.
                mData->SetItem(1, new PyInt(offer.important?1:0)); //importantMission  -- integer boolean
                mData->SetItem(2, new PyString(sMissionDataMgr.GetTypeLabel(offer.typeID))); //missionTypeLabel
                mData->SetItem(3, new PyString(offer.name)); //missionName
                mData->SetItem(4, new PyInt(offer.agentID)); //agentID
                mData->SetItem(5, new PyLong(offer.expiryTime)); //expirationTime
                mData->SetItem(6, offer.bookmarks != nullptr ? offer.bookmarks->Clone() : new PyList()); //bookmarks -- if populated, this is PyList of PyDicts as defined below...
                mData->SetItem(7, new PyBool(offer.remoteOfferable)); //remoteOfferable
                mData->SetItem(8, new PyBool(offer.remoteCompletable)); //remoteCompletable
            missions->AddItem(mData);
        }
    }
    tuple->SetItem(0, missions);

    //research:
    PyList* research = new PyList();
    tuple->SetItem(1, research);

    if (is_log_enabled(AGENT__RSP_DUMP))
        tuple->Dump(AGENT__RSP_DUMP, "   ");
    return tuple;
}

PyResult AgentBound::GetMissionJournalInfo(PyCallArgs &call, std::optional <PyInt*> characterID, std::optional <PyInt*> contentID) {
    //called on rclick in journal to "read details"
    //ret = self.GetAgentMoniker(agentID).GetMissionJournalInfo(charID, contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionJournalInfo() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    PyDict* journalInfo = new PyDict();
    journalInfo->SetItemString("contentID", new PyInt(offer.characterID));
    if (Mission::IsSyntheticSecurityEncounter(offer.missionID)) {
        journalInfo->SetItemString("missionNameID", PyStatic.NewNone());
        journalInfo->SetItemString("briefingTextID", PyStatic.NewNone());
    } else {
        journalInfo->SetItemString("missionNameID", new PyInt(offer.missionID));
        journalInfo->SetItemString("briefingTextID", new PyInt(offer.briefingID));
    }
    journalInfo->SetItemString("missionState", new PyInt(offer.stateID));
    journalInfo->SetItemString("expirationTime", new PyLong(offer.expiryTime) );
    journalInfo->SetItemString("objectives", GetMissionObjectiveInfo(call.client, offer));
    switch(offer.typeID) {
        case Mission::Type::Courier:
            journalInfo->SetItemString("missionImage", sMissionDataMgr.GetCourierRes()); break;
        case Mission::Type::Mining:
            journalInfo->SetItemString("missionImage", sMissionDataMgr.GetMiningRes()); break;
        case Mission::Type::Encounter:
            journalInfo->SetItemString("missionImage", sMissionDataMgr.GetKillRes()); break;
    }

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionJournalInfo() RSP:" );
        journalInfo->Dump(AGENT__RSP_DUMP, "    ");
    }

    return journalInfo;
}

PyDict* AgentBound::GetMissionObjectiveInfo(Client* pClient, MissionOffer& offer)
{
    PyDict* objectiveData = new PyDict();
    objectiveData->SetItemString("missionTitleID",
        Mission::IsSyntheticSecurityEncounter(offer.missionID)
            ? PyStatic.NewNone() : new PyInt(offer.missionID));
    if (offer.missionID == Mission::kEliteSecurityMissionID) {
        objectiveData->SetItemString("missionTitle",
            new PyString("DED & Officer Expedition"));
        objectiveData->SetItemString("missionBriefing",
            new PyString(EliteSecurityMissionBriefing(offer)));
        objectiveData->SetItemString("killsDestroyed",
            new PyInt(std::min<uint32>(offer.courierTypeID, offer.courierAmount)));
        objectiveData->SetItemString("killsRequired",
            new PyInt(offer.courierAmount));
    } else if (offer.missionID == Mission::kBlockadeSecurityTestMissionID) {
        const uint32 killsDestroyed =
            (offer.courierTypeID < offer.courierAmount)
                ? offer.courierTypeID : offer.courierAmount;
        std::string missionBriefing =
            "ANGEL CARTEL INCURSION\n\n"
            "SITUATION\n"
            "An Angel Cartel strike force has established a hostile presence in this system. "
            "Eliminate the entire force and secure the bookmarked mission pocket.\n\n"
            "MISSION OBJECTIVES\n"
            "- 8 cruisers\n"
            "- 4 battlecruisers\n\n"
            "COMBAT PROGRESS\n"
            "Kills confirmed: ";
        missionBriefing += std::to_string(killsDestroyed);
        missionBriefing += " / ";
        missionBriefing += std::to_string(offer.courierAmount);
        missionBriefing += "\n\nCOMPLETION\n"
            "Return to your agent once all hostile ships have been destroyed.\n\n"
            "REWARD\n"
            "50,000,000 ISK\n\n"
            "TIME LIMIT\n"
            "120 minutes from acceptance.";
        objectiveData->SetItemString("missionTitle", new PyString("The Blockade - Angel Cartel"));
        objectiveData->SetItemString("missionBriefing", new PyString(missionBriefing));
        objectiveData->SetItemString("killsDestroyed", new PyInt(killsDestroyed));
        objectiveData->SetItemString("killsRequired", new PyInt(offer.courierAmount));
    }
    objectiveData->SetItemString("contentID", new PyInt(offer.characterID));
    objectiveData->SetItemString("importantStandings", new PyInt(offer.important));     // boolean integer
    // will need to test for this to set correctly.....
    if (pClient->IsMissionComplete(offer)) {     // Mission::Status:: data here 0=no, 1=yes, 2=cheat
        objectiveData->SetItemString("completionStatus", new PyInt(Mission::Status::Complete));
    } else {
        objectiveData->SetItemString("completionStatus", new PyInt(Mission::Status::Incomplete));
    }
    objectiveData->SetItemString("missionState", new PyInt(offer.stateID /*Mission::State::Offered*/));   // Mission::State:: data here for agentGift populating.  Accepted/failed to display gift items as accepted
    objectiveData->SetItemString("loyaltyPoints", new PyInt(offer.rewardLP));
    objectiveData->SetItemString("researchPoints", new PyInt(0));

    /*  this puts title/msg at bottom of right pane
    if (offer.stateID == Mission::State::Accepted)
        if (offer.typeID == Mission::Type::Courier) {
            PyTuple* missionExtra = new PyTuple(2);  // this is tuple(2)  headerID, bodyID    -- std locale msgIDs
                missionExtra->SetItem(0, new PyString("Reminder...."));   // this should be separate title from mission name
                missionExtra->SetItem(1, new PyString("Remember to get the %s from your hangar before you leave.", call.client->GetCourierItemRef(m_agent->GetID())->name()));   // this is additional info about mission, etc.
            objectiveData->SetItemString("missionExtra", missionExtra);
        } */

    PyList* locList = new PyList();    // tuple of list of locationIDs (pickup and dropoff)
        locList->AddItem(new PyInt(offer.originSystemID));
        locList->AddItem(new PyInt(offer.destinationSystemID));
    objectiveData->SetItemString("locations", locList);

    PyList* giftList = new PyList();    // this is list of tuple(3)  typeID, quantity, extra
    /*
    PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
        extra->SetItemString("specificItemID", PyStatic.NewNone());
        extra->SetItemString("blueprintInfo", PyStatic.NewNone());
    PyTuple* agentGift = new PyTuple(3);
        agentGift->SetItem(0, PyStatic.NewNone());
        agentGift->SetItem(1, PyStatic.NewNone());
        agentGift->SetItem(2, extra);
        giftList->AddItem(agentGift);
    */
    objectiveData->SetItemString("agentGift", giftList);

    PyList* normList = new PyList();    // this is list of tuple(3)  typeID, quantity, extra
    if (offer.rewardISK) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* normalRewards = new PyTuple(3);
            normalRewards->SetItem(0, new PyInt(itemTypeCredits));
            normalRewards->SetItem(1, new PyInt(offer.rewardISK));
            normalRewards->SetItem(2, extra);
        normList->AddItem(normalRewards);
    }
    if (offer.rewardItemID) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* normalRewards = new PyTuple(3);
            normalRewards->SetItem(0, new PyInt(offer.rewardItemID));
            normalRewards->SetItem(1, new PyInt(offer.rewardItemQty));
            normalRewards->SetItem(2, extra);
        normList->AddItem(normalRewards);
    }
    objectiveData->SetItemString("normalRewards", normList);

    PyList* collateralList = new PyList(); // this is list of tuple(3)  typeID, quantity, extra
    /*
    PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
        extra->SetItemString("specificItemID", PyStatic.NewNone());
        extra->SetItemString("blueprintInfo", PyStatic.NewNone());
    PyTuple* collateral = new PyTuple(3);
        collateral->SetItem(0, PyStatic.NewNone());
        collateral->SetItem(1, PyStatic.NewNone());
        collateral->SetItem(2, extra);
        */
    objectiveData->SetItemString("collateral", collateralList);

    PyList* bonusList = new PyList();   // this is list of tuple(4)  timeRemaining, typeID, quantity, extra
    if (offer.bonusTime > 0) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* bonusRewards = new PyTuple(4);
        if (offer.dateAccepted > 0) {
            bonusRewards->SetItem(0, new PyLong(EvEAgent::BonusTimeRemaining(
                offer.dateAccepted, offer.bonusTime, GetFileTimeNow())));
        } else {
            bonusRewards->SetItem(0, new PyLong(EvEAgent::BonusTimeRemaining(
                0, offer.bonusTime, GetFileTimeNow())));
        }
            bonusRewards->SetItem(1, new PyInt(itemTypeCredits));   // bonus is *usually* isk.  for now, we'll keep it as isk (easier)
            bonusRewards->SetItem(2, new PyInt(offer.bonusISK));
            bonusRewards->SetItem(3, extra);
        bonusList->AddItem(bonusRewards);
    }
    // bonusList can be multiple items, usualy only item or isk for time bonus
    if (false/*bonus2*/) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* bonusRewards2 = new PyTuple(4);
            bonusRewards2->SetItem(0, new PyLong(12000000000)); //20m
            bonusRewards2->SetItem(1, new PyInt(itemTypeTrit));
            bonusRewards2->SetItem(2, new PyInt(offer.rewardISK));
            bonusRewards2->SetItem(3, extra);
        bonusList->AddItem(bonusRewards2);
    }
    objectiveData->SetItemString("bonusRewards", bonusList);
    /*  for collateral and rewards, as follows...
    typeID, quantity, extra in objectiveData['normalRewards']
    typeID, quantity, extra in objectiveData['collateral']
    typeID, quantity, extra in objectiveData['agentGift']
    or
    timeRemaining, typeID, quantity, extra in objectiveData['bonusRewards']

        specificItemID = extra.get('specificItemID', 0)
        blueprintInfo = extra.get('blueprintInfo', None)
        */

    objectiveData->SetItemString("objectives", GetMissionObjectives(pClient, offer));
    PyList* dunList = new PyList();  // this is a list of dunData dicts
    /*
    PyDict* dunData = new PyDict();
        dunData->SetItemString("dungeonID", new PyInt(1000));
        dunData->SetItemString("completionStatus", new PyInt(Dungeon::Status::Started));
        dunData->SetItemString("optional", new PyInt());
        dunData->SetItemString("briefingMessage", new PyInt());
        dunData->SetItemString("objectiveCompleted", new PyBool(false));
        dunData->SetItemString("ownerID", new PyInt(m_agent->GetID()));
        dunData->SetItemString("shipRestrictions", new PyInt(0));   // 0=normal 1=special with link to *something else*
        dunData->SetItemString("location", m_agent->GetLocationWrap());
    */
    objectiveData->SetItemString("dungeons", dunList);
    /* dunData data....
     * dungeonID
     * completionStatus
     * optional
     * briefingMessage
     * objectiveCompleted
     * ownerID
     * location
        location['locationID']
        location['locationType']
        location['solarsystemID']
        location['coords']
        location['agentID']
        ?location['referringAgentID']
        ?location['shipTypeID']
     * shipRestrictions  0=normal 1=special with link to *something else*
     */

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionObjectiveInfo() RSP:" );
        objectiveData->Dump(AGENT__RSP_DUMP, "    ");
    }

    return objectiveData;
}

PyTuple* AgentBound::GetMissionObjectives(Client* pClient, MissionOffer& offer)
{
    // set mission objectiveData based on mission type.
    PyDict* dropoffLocation = new PyDict();
    if (offer.typeID == Mission::Type::Encounter && offer.destinationID != 0) {
        dropoffLocation->SetItemString("typeID", new PyInt(offer.destinationTypeID));
        dropoffLocation->SetItemString("locationID", new PyInt(offer.destinationID));
        dropoffLocation->SetItemString("solarsystemID", new PyInt(offer.destinationSystemID));
    } else if (sDataMgr.IsStation(offer.destinationID)) {
        dropoffLocation->SetItemString("typeID", new PyInt(offer.destinationTypeID) );
        dropoffLocation->SetItemString("locationID", new PyInt(offer.destinationID) );
        dropoffLocation->SetItemString("solarsystemID", new PyInt(offer.destinationSystemID) );
    } else {
        dropoffLocation->SetItemString("shipTypeID", new PyInt(offer.destinationTypeID) );
        dropoffLocation->SetItemString("agentID", new PyInt(offer.destinationOwnerID) );
        // get agent in space location and set here
        PyTuple* coords = new PyTuple(3);
            coords->SetItem(0, new PyFloat(0)); //x
            coords->SetItem(1, new PyFloat(0)); //y
            coords->SetItem(2, new PyFloat(0)); //z
        dropoffLocation->SetItemString("coords", coords);
        dropoffLocation->SetItemString("referringAgentID", new PyInt(offer.agentID) );
    }

    PyTuple* objectives = new PyTuple(1);
    switch (offer.typeID) {
        case Mission::Type::Trade:
        case Mission::Type::Courier: {
            PyDict* pickupLocation = new PyDict();
                pickupLocation->SetItemString("typeID", new PyInt(m_agent->GetLocTypeID()) );
                pickupLocation->SetItemString("locationID", new PyInt(offer.originID) );
                pickupLocation->SetItemString("solarsystemID", new PyInt(offer.originSystemID) );
            PyDict* cargo = new PyDict();
                cargo->SetItemString("hasCargo", new PyBool(pClient->ContainsTypeQty(offer.courierTypeID, offer.courierAmount)));
                cargo->SetItemString("typeID", new PyInt(offer.courierTypeID));
                cargo->SetItemString("quantity", new PyInt(offer.courierAmount));
                cargo->SetItemString("volume", new PyFloat(offer.courierItemVolume * offer.courierAmount));    // calculated shipment volume.  *this is direct to window*
            PyTuple* objData = new PyTuple(5);
                objData->SetItem(0, new PyInt(offer.originOwnerID));
                objData->SetItem(1, pickupLocation/*m_agent->GetLocationWrap()*/);
                objData->SetItem(2, new PyInt(offer.destinationOwnerID));
                objData->SetItem(3, dropoffLocation/*m_agent->GetLocationWrap()*/);
                objData->SetItem(4, cargo);
            PyTuple* objType = new PyTuple(2);   // this is list of tuple(2)    objType, objData
                objType->SetItem(0, new PyString("transport"));
                objType->SetItem(1, objData);
            objectives->SetItem(0, objType);
        } break;
        case Mission::Type::Encounter: {
            PyDict* progress = new PyDict();
                progress->SetItemString("currentKills", new PyInt(offer.courierTypeID));
                progress->SetItemString("requiredKills", new PyInt(offer.courierAmount));
                const bool objectiveComplete =
                    offer.courierAmount > 0 && offer.courierTypeID >= offer.courierAmount;
                progress->SetItemString("complete", new PyBool(objectiveComplete));
                progress->SetItemString("objectiveCompleted", new PyBool(objectiveComplete));
            PyTuple* objData = new PyTuple(3);
                objData->SetItem(0, new PyInt(offer.destinationOwnerID));
                objData->SetItem(1, dropoffLocation);
                objData->SetItem(2, progress);
            PyTuple* objType = new PyTuple(2);
                objType->SetItem(0, new PyString("kill"));
                objType->SetItem(1, objData);
            objectives->SetItem(0, objType);
        } break;
        case Mission::Type::Mining: {
            PyDict* cargo = new PyDict();
                cargo->SetItemString("hasCargo", new PyBool(pClient->ContainsTypeQty(offer.courierTypeID, offer.courierAmount)));
                cargo->SetItemString("typeID", new PyInt(offer.courierTypeID));
                cargo->SetItemString("quantity", new PyInt(offer.courierAmount));
                cargo->SetItemString("volume", new PyFloat(offer.courierItemVolume * offer.courierAmount));
            PyTuple* objData = new PyTuple(3);
                objData->SetItem(0, new PyInt(offer.destinationOwnerID));
                objData->SetItem(1, dropoffLocation);
                objData->SetItem(2, cargo);
            PyTuple* objType = new PyTuple(2);
                objType->SetItem(0, new PyString("fetch"));
                objType->SetItem(1, objData);
            objectives->SetItem(0, objType);
        } break;
        case Mission::Type::Anomic:
        case Mission::Type::EpicArc:
        case Mission::Type::Burner:
        case Mission::Type::Cosmos:
        case Mission::Type::Data:
        case Mission::Type::Research:
        case Mission::Type::Storyline:
        case Mission::Type::Tutorial: {
            objectives->SetItem(0, PyStatic.NewNone());
        } break;
    }

    // cleanup
    PySafeDecRef(dropoffLocation);

    return objectives;

    /*  objectives data...
    if objType == 'agent':      -- report to agent
        agentID, agentLocation = objData
        agentLocation['locationID']
        agentLocation['locationType']
        agentLocation['solarsystemID']
        if not in station
            agentLocation['coords']
            agentLocation['agentID']
            ?agentLocation['referringAgentID']
            ?agentLocation['shipTypeID']

    elif objType == 'transport':        -- courier and trade missions
        pickupOwnerID, pickupLocation, dropoffOwnerID, dropoffLocation, cargo = objData
        pickupLocation['locationID']
        pickupLocation['locationType']
        pickupLocation['solarsystemID']
        dropoffLocation['locationID']
        dropoffLocation['locationType']
        dropoffLocation['solarsystemID']
        if not in station
            dropoffLocation['coords']
            dropoffLocation['agentID']
            ?dropoffLocation['referringAgentID']
            ?dropoffLocation['shipTypeID']
        cargo['hasCargo']
        cargo['typeID']
        cargo['volume']
        cargo['quantity']

    elif objType == 'fetch':            -- encounter and mining missions
        dropoffOwnerID, dropoffLocation, cargo = objData
        dropoffLocation['locationID']
        dropoffLocation['locationType']
        dropoffLocation['solarsystemID']
        if not in station
            dropoffLocation['coords']
            dropoffLocation['agentID']
            ?dropoffLocation['referringAgentID']
            ?dropoffLocation['shipTypeID']
        cargo['hasCargo']
        cargo['typeID']
        cargo['volume']
        cargo['quantity']
        */
}


/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult AgentBound::GetDungeonShipRestrictions(PyCallArgs &call, PyInt* dungeonID) {
    //restrictions = self.GetAgentMoniker(agentID).GetDungeonShipRestrictions(dungeonID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetDungeonShipRestrictions() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::RemoveOfferFromJournal(PyCallArgs &call) {
    //called on rclick in journal to "remove offer"
    //self.GetAgentMoniker(agentID).RemoveOfferFromJournal()
    _log(AGENT__DUMP,  "AgentBound::Handle_RemoveOfferFromJournal() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::GetOfferJournalInfo(PyCallArgs &call) {
    //html = self.GetAgentMoniker(agentID).GetOfferJournalInfo()
    _log(AGENT__DUMP,  "AgentBound::Handle_GetOfferJournalInfo() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::GetEntryPoint(PyCallArgs &call) {
    //entryPoint = sm.StartService('agents').GetAgentMoniker(bookmark.agentID).GetEntryPoint()
    _log(AGENT__DUMP,  "AgentBound::Handle_GetEntryPoint() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::GotoLocation(PyCallArgs &call, PyString* locationType, PyInt* locationNumber, PyRep* referringAgentID) {
    //sm.StartService('agents').GetAgentMoniker(bookmark.agentID).GotoLocation(bookmark.locationType, bookmark.locationNumber, referringAgentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GotoLocation() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    // The client sends None for referringAgentID from the mission bookmark.
    // Resolve the active encounter target here: this is the RPC used by the
    // journal's "Warp to Location" context-menu action.
    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer)
        || offer.stateID != Mission::State::Accepted
        || offer.typeID != Mission::Type::Encounter
        || offer.destinationID == 0
        || offer.destinationSystemID != call.client->GetSystemID()) {
        call.client->SendErrorMsg("The mission site is not available in this solar system.");
        return PyStatic.NewNone();
    }

    ShipSE* pShip = call.client->GetShipSE();
    if (pShip == nullptr || pShip->DestinyMgr() == nullptr || pShip->SystemMgr() == nullptr) {
        call.client->SendErrorMsg("Your ship is not ready to warp.");
        return PyStatic.NewNone();
    }

    SystemEntity* pTarget = pShip->SystemMgr()->GetSE(offer.destinationID);
    if (pTarget == nullptr) {
        call.client->SendErrorMsg("The mission site beacon could not be found. Please contact an administrator.");
        return PyStatic.NewNone();
    }

    pShip->DestinyMgr()->WarpTo(pTarget->GetPosition(), 0, false, pTarget);
    return PyStatic.NewNone();
}

PyResult AgentBound::WarpToLocation(PyCallArgs &call, PyString* locationType, PyInt* locationNumber, PyFloat* warpRange, PyBool* fleet, PyInt* referringAgentID) {
    // Mission journal bookmarks call this agent-bound RPC rather than the
    // ordinary item warp command. Resolve the active encounter's destination
    // beacon and warp directly to its live system entity.
    _log(AGENT__DUMP, "AgentBound::Handle_WarpToLocation() - size=%lli", call.tuple->size());
    call.Dump(AGENT__DUMP);

    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer)
        || offer.stateID != Mission::State::Accepted
        || offer.typeID != Mission::Type::Encounter
        || offer.destinationID == 0
        || offer.destinationSystemID != call.client->GetSystemID()) {
        call.client->SendErrorMsg("The mission site is not available in this solar system.");
        return PyStatic.NewNone();
    }

    ShipSE* pShip = call.client->GetShipSE();
    if (pShip == nullptr || pShip->DestinyMgr() == nullptr || pShip->SystemMgr() == nullptr) {
        call.client->SendErrorMsg("Your ship is not ready to warp.");
        return PyStatic.NewNone();
    }

    SystemEntity* pTarget = pShip->SystemMgr()->GetSE(offer.destinationID);
    if (pTarget == nullptr) {
        call.client->SendErrorMsg("The mission site beacon could not be found. Please contact an administrator.");
        return PyStatic.NewNone();
    }

    const int32 distance = (warpRange != nullptr
        ? static_cast<int32>(warpRange->value()) : 0);
    pShip->DestinyMgr()->WarpTo(pTarget->GetPosition(), distance, false, pTarget);
    return PyStatic.NewNone();
}
