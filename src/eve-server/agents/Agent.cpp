
 /**
  * @name Agent.cpp
  *   agent specific code
  *    original agent code by zhur, this was completely rewritten based on new data.
  *
  * @Author:        Allan
  * @date:      19 June 2018
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

#include "../eve-server.h"
#include "../StaticDataMgr.h"
#include "../station/StationDataMgr.h"
#include "../map/MapData.h"

#include "agents/Agent.h"
#include "agents/AgentDB.h"
#include "../Client.h"
#include "../fleet/FleetService.h"
#include "../system/SystemManager.h"
#include "../standing/StandingMgr.h"


Agent::Agent(uint32 id)
: m_agentID(id)
{
    m_important = false;
    m_buttonID = 1;

    m_actions.clear();
    m_offers.clear();

    _log(AGENT__TRACE, "Agent created for AgentID %u", id);
}


bool Agent::Load() {
    AgentDB::LoadAgentData(m_agentID, m_agentData);
    sMissionDataMgr.LoadAgentOffers(m_agentID, m_offers);

    if (m_agentData.research)
        AgentDB::LoadAgentSkills(m_agentID, m_skills);

    _log(AGENT__TRACE, "Data Loaded for Agent %u - bl: %u, level: %u, locationID: %u, systemID: %u", \
                m_agentID, m_agentData.bloodlineID, m_agentData.level, m_agentData.locationID, m_agentData.solarSystemID);
    return true;
}

void Agent::MakeOffer(uint32 charID, MissionOffer& offer)
{
    // this will be based on agent type eventually
    uint8 misionType = Mission::Type::Courier;

    sMissionDataMgr.CreateMissionOffer(misionType, m_agentData.level, m_agentData.raceID, m_important, offer);

    /*  static mission data from db
    offer.name               = cData.name;
    offer.typeID             = cData.typeID;
    offer.bonusISK           = cData.bonusISK;
    offer.rewardISK          = cData.rewardISK;
    offer.bonusTime          = cData.bonusTime;
    offer.important          = cData.important;
    offer.storyline          = cData.storyline;
    offer.missionID          = cData.missionID;
    offer.briefingID         = cData.briefingID;
    offer.rewardItemID       = cData.rewardItemID;
    offer.rewardItemQty      = cData.rewardItemQty;
    offer.courierTypeID      = cData.itemTypeID;
    offer.courierAmount      = cData.itemQty;

    offer.stateID            = Mission::State::Allocated;
    offer.dateIssued         = GetFileTimeNow();
    */

    offer.characterID        = charID;

    // variable mission data based on agent
    offer.agentID            = m_agentID;
    offer.originID           = m_agentData.stationID;
    offer.originOwnerID      = m_agentData.corporationID;
    offer.originSystemID     = m_agentData.solarSystemID;
    offer.expiryTime         = GetFileTimeNow() + EvE::Time::Day;

    // make function to determine destination based on mission type, agent level, agent location, and some other shit.
    //offer.destinationID      = 0;
    //offer.destinationOwnerID      = 0;
    //offer.destinationSystemID = 0;
    //offer.destinationTypeID = 0;
    //offer.dungeonLocationID      = 0;
    //offer.dungeonSolarSystemID   = 0;
    sMapData.GetMissionDestination(this, misionType, offer);
    if (offer.destinationID == 0) {
        // make error here and reset
        sEntityList.FindClientByCharID(charID)->SendErrorMsg("Internal Server Error. Ref: ServerError 07208.");
        //return;
    }

    // not sure how this is checked/set
    offer.remoteOfferable    = false;
    offer.remoteCompletable  = false;
    // same with these
    offer.rewardLP           = 0;
    offer.acceptFee          = 0;

    // create and save bookmarks for this offer.... not sure how yet.
    offer.bookmarks          = new PyList();

    //offer.dateAccepted       = 0;     //not set here
    //offer.dateCompleted      = 0;     //not set here

    offer.stateID            = Mission::State::Offered;
    //offer.offerID            = 0;     //created when saving offer in db

    // update mission data as needed....
    if (offer.typeID == Mission::Type::Courier) {
        if ((offer.briefingID == 145156) or (offer.briefingID == 14515)) {
            // the destination and origin need to be reversed on this mission.
            offer.originID            = offer.destinationID;
            offer.originOwnerID       = offer.destinationOwnerID;
            offer.originSystemID      = offer.destinationSystemID;

            offer.destinationID       = m_agentData.stationID;
            offer.destinationOwnerID  = m_agentData.corporationID;
            offer.destinationSystemID = m_agentData.solarSystemID;
        }

    }
    MissionDB::CreateOfferID(offer);

    // keep local copy and also add to mission data mgr
    m_offers.emplace(charID, offer);
    sMissionDataMgr.AddMissionOffer(charID, offer);
}

bool Agent::HasMission(uint32 charID)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end())
        return true;
    return false;
}

bool Agent::HasMission(uint32 charID, MissionOffer& offer)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        offer = itr->second;
        return true;
    }
    return false;
}

void Agent::GetOffer(uint32 charID, MissionOffer& offer)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        offer = itr->second;
    } else {
        _log(AGENT__WARNING, "Agent::GetOffer() - offer not found for characterID %u", charID);
    }
}

/*{'FullPath': u'UI/Messages', 'messageID': 259310, 'label': u'MisMissionExpiredBody'}(u'This mission has already expired.', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259311, 'label': u'MisNoItemsTitle'}(u'No items available', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259312, 'label': u'MisNoItemsBody'}(u"There's nothing available at this station.", None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259313, 'label': u'MisNoShipsInCourierTitle'}(u'Cannot create courier mission', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259314, 'label': u'MisNoShipsInCourierBody'}(u'Only packaged ships can be couriered.', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259315, 'label': u'MisNoSuchMissionTitle'}(u'Mission not found', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259316, 'label': u'MisNoSuchMissionBody'}(u'This mission is no longer available.', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259317, 'label': u'MisNotAvailableBody'}(u'"This mission is no longer available"', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259318, 'label': u'MisPackageNotHereTitle'}(u"Can't deliver package", None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259319, 'label': u'MisPackageNotHereBody'}(u'The package is not on your hangar floor.', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259320, 'label': u'MisPilotOnBoardBody'}(u"Can't add ships with people in it in a courier mission.", None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259321, 'label': u'MisRewardLessAcceptTitle'}(u'Too unfair', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259322, 'label': u'MisRewardLessAcceptBody'}(u'Please make the reward higher than the accept fee.', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259323, 'label': u'MissMissionAcceptedHelpTitle'}(u'Mission Accepted', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259325, 'label': u'MissionItemAlreadyThereTitle'}(u'Create the Mission', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259326, 'label': u'MissionItemAlreadyThereBody'}(u'The item is already here. Are you sure you want to continue?', None, None)
 {'FullPath': u'UI/Messages', 'messageID': 259327, 'label': u'MissionNotIssuedBody'}(u'This mission cannot be accepted.', None, None)
 */
void Agent::UpdateOffer(uint32 charID, MissionOffer& offer)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        itr->second = offer;
        MissionDB::UpdateMissionOffer(itr->second);
        sMissionDataMgr.UpdateMissionData(charID, itr->second);
    } else {
        _log(AGENT__WARNING, "Agent::UpdateOffer() - offer not found for character %u", charID);
    }
}

void Agent::RemoveOffer(uint32 charID)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        m_offers.erase(itr);
    } else {
        _log(AGENT__WARNING, "Agent::RemoveOffer() - offer not found for character %u", charID);
    }
}

void Agent::DeleteOffer(uint32 charID)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        itr->second.stateID = Mission::State::Rejected;
        MissionDB::UpdateMissionOffer(itr->second);
        sMissionDataMgr.RemoveMissionOffer(charID, itr->second);
        m_offers.erase(itr);
    } else {
        _log(AGENT__WARNING, "Agent::DeleteOffer() - offer not found for character %u", charID);
    }
}


PyDict* Agent::GetLocationWrap() {
    PyDict *res = new PyDict();
        res->SetItemString("typeID", new PyInt(m_agentData.locationTypeID) );
        res->SetItemString("locationID", new PyInt(m_agentData.locationID) );
        res->SetItemString("solarsystemID", new PyInt(m_agentData.solarSystemID) );
    return res;

    /* other location data types to put in dict for agents in space
     * locationType
     * coords
     * referringAgentID
     * shipTypeID
     *
     */

    /*
     * def LocationWrapper(location, locationType = None):
     *    if locationType is None and 'locationType' in location:
     *        locationType = location['locationType']
     *    pseudoSecurityRating = cfg.solarsystems.Get(location['solarsystemID']).pseudoSecurity
     *    if pseudoSecurityRating <= 0:
     *        securityKey = '0.0'
     *    else:
     *        securityKey = str(round(pseudoSecurityRating, 1))
     *    secColor = SECURITY_COLORS[securityKey]
     *    secColorAsHtml = '#%02X%02X%02X' % (secColor[0], secColor[1], secColor[2])
     *    secWarning = '<font color=#E3170D>'
     *    secClass = util.SecurityClassFromLevel(pseudoSecurityRating)
     *    standingSvc = sm.GetService('standing')
     *    if secClass <= const.securityClassLowSec:
     *        secWarning += localization.GetByLabel('UI/Agents/LowSecWarning')
     *    elif standingSvc.GetMySecurityRating() <= -5:
     *        secWarning += localization.GetByLabel('UI/Agents/HighSecWarning')
     *    secWarning += '</font>'
     *    if 'coords' in location:
     *        x, y, z = location['coords']
     *        refAgentString = str(location['agentID'])
     *        if 'referringAgentID' in location:
     *            refAgentString += ',' + str(location['referringAgentID'])
     *        infoLinkData = ['showinfo',
     *         location['typeID'],
     *         location['locationID'],
     *         x,
     *         y,
     *         z,
     *         refAgentString,
     *         0,
     *         locationType]
     *    else:
     *        infoLinkData = ['showinfo', location['typeID'], location['locationID']]
     *    spacePigShipType = location.get('shipTypeID', None)
     *    if spacePigShipType is not None:
     *        locationName = localization.GetByLabel('UI/Agents/Items/ItemLocation', typeID=spacePigShipType, locationID=location['locationID'])
     *    else:
     *        locationName = cfg.evelocations.Get(location['locationID']).locationName
     *    return localization.GetByLabel('UI/Agents/LocationWrapper', startFontTag='<font color=%s>' % secColorAsHtml, endFontTag='</font>', securityRating=pseudoSecurityRating, locationName=locationName, linkdata=infoLinkData, securityWarning=secWarning)
     */
}

PyObject* Agent::GetInfoServiceDetails()
{
    // can this be static data created when agent is loaded?  avoid creating this everytime it's called.
    PyDict* res = new PyDict();
        res->SetItemString("stationID", new PyInt(m_agentData.stationID) );
        res->SetItemString("level", new PyInt(m_agentData.level) );

    // 'services' is a tuple of dicts containing data for [research], [locate], and [mission] services this agent offers

    /*  for research agents....
     *  skillTypeID, skillLevel in data.skills:
     * researchData = data.researchData
     * researchData['rpMultiplier']
     * researchData['skillTypeID']
     * researchData['points']  -- current points
     * researchData['pointsPerDay']
     * skillTypeID, blueprintTypeID in data.researchSummary:  -- for predictablePatentNames
     */

    /**  @todo  finish this..... */
    PyDict* research = new PyDict();
    if (m_agentData.research) {
        PyTuple* skill1 = new PyTuple(2);
            skill1->SetItem(0, new PyInt(11452)); // Mechanical Engineering
            skill1->SetItem(1, new PyInt(4));
        PyTuple* skill2 = new PyTuple(2);
            skill2->SetItem(0, new PyInt(11453));  //Electronic Engineering
            skill2->SetItem(1, new PyInt(3));
        PyList* skillList = new PyList();
            skillList->AddItem(skill1);
            skillList->AddItem(skill2);
        PyDict* researchData = new PyDict();
            researchData->SetItemString("rpMultiplier", new PyInt(2));
            researchData->SetItemString("skillTypeID", new PyInt(11452));   // this is player research field with this agent.  not sure how to make "none" yet
            researchData->SetItemString("points", new PyInt(150));
            researchData->SetItemString("pointsPerDay", new PyInt(30));
        PyTuple* patent1 = new PyTuple(2);
            patent1->SetItem(0, new PyInt(11452));
        PyList* patentlist1 = new PyList();
            patentlist1->AddItem(new PyInt(692));
            patent1->SetItem(1, patentlist1);
        PyTuple* patent2 = new PyTuple(2);
            patent2->SetItem(0, new PyInt(11453));
        PyList* patentlist2 = new PyList();
            patentlist2->AddItem(new PyInt(1196));
            patent2->SetItem(1, patentlist2);
        PyList* patentList = new PyList();
            patentList->AddItem(patent1);
            patentList->AddItem(patent2);

        research->SetItemString("agentServiceType", new PyString("research"));
        research->SetItemString("skills", skillList);
        research->SetItemString("researchSummary", patentList);
        research->SetItemString("researchData", researchData);
    } else {
        research->SetItemString("agentServiceType", PyStatic.NewNone());
    }

    /* for location agents....
     *
 level  Standings  Time to Run                             Cooldown    Cost                Range
    1     Any      Instant/1 minute                        5 minutes   1k/5k               Constellation
    2     1.0      Instant/1 minute/8 minutes              5 minutes   5k/10k/25k          Region
    3     3.0      Instant/30 seconds/4 minutes/8 minutes  15 minutes  10k/25k/50k/100k    Unlimited
    4     5.0      Instant/20 seconds/2 minutes/4 minutes  30 minutes  25k/50k/100k/250k   Unlimited

     * data.frequency
     *delayRange, delay, cost in data.delays:       -- (tuple) range (system, const, region, other region), responseTime (in sec), cost
     *   data.callbackID  -- bool for agent locator services being used (locator unavailable)
     *      OR
     *   data.lastUsed  -- blue time?

     Once done the locator agent will send you a Notification with the location of the target when you started the search which will include the system, constellation and region as well as the station the player might be docked at. If the target is in space, no station will be listed. E.g. "The sleazebag is currently in the Bukah system, Nohshayess constellation of the Khanid region."
     If the target is logged off, the locator agent will tell you the last known position. If the target is in Anoikis (wormhole space), the locator agent will tell you "I'm sorry, but I just can't help you with that one. I'm pretty sure O'b Haru Sen is well out of my zone of influence." - even if the agent can locate anyone in known space.

     (235843, `{[character]charID.gender -> "He", "She"} is in the {systemName} system of the {constellationName} constellation.`)
     (235844, `{[character]charID.gender -> "He", "She"} is in the {systemName} system, {constellationName} constellation of the {regionName} region.`)
     (235845, `{[character]charID.gender -> "He", "She"} is in your solar system.`)
     (235846, `{[character]charID.gender -> "He", "She"} is at your station.`)
     (235847, `{[character]charID.gender -> "He", "She"} is at {stationName} station in your solar system.`)
     (235848, `{[character]charID.gender -> "He", "She"} is at {stationName} station in the {systemName} system.`)
     (235849, `{[character]charID.gender -> "He", "She"} is at{stationName} station in the {systemName} system of the {constellationName} constellation.`)
     (235850, `{[character]charID.gender -> "He", "She"} is at {stationName} station in the {systemName} system, {constellationName} constellation of {regionName} region.`)
     (235851, `{[character]charID.gender -> "He", "She"} is in the {systemName} system.`)
     */
    PyDict* locate = new PyDict();
    if (m_agentData.locator) {
        PyTuple* sameSystem = new PyTuple(3);
            sameSystem->SetItem(0, new PyInt(0));
            sameSystem->SetItem(1, new PyInt(10));
            sameSystem->SetItem(2, new PyInt(20000));
        PyTuple* sameConst = new PyTuple(3);
            sameConst->SetItem(0, PyStatic.NewOne());
            sameConst->SetItem(1, new PyInt(30));
            sameConst->SetItem(2, new PyInt(200000));
        PyTuple* sameRegion = new PyTuple(3);
            sameRegion->SetItem(0, new PyInt(2));
            sameRegion->SetItem(1, new PyInt(60));
            sameRegion->SetItem(2, new PyInt(2000000));
        PyTuple* otherRegion = new PyTuple(3);
            otherRegion->SetItem(0, new PyInt(3));
            otherRegion->SetItem(1, new PyInt(120));
            otherRegion->SetItem(2, new PyInt(20000000));
        PyTuple* delays = new PyTuple(4);
            delays->SetItem(0, sameSystem);
            delays->SetItem(1, sameConst);
            delays->SetItem(2, sameRegion);
            delays->SetItem(3, otherRegion);

        locate->SetItemString("agentServiceType", new PyString("locate"));
        locate->SetItemString("frequency", new PyInt(1200));  // if this is PyNone (or 0?) agent location isnt avalible (client parsed msg)
        locate->SetItemString("delays", delays);
        locate->SetItemString("callbackID", new PyInt(2));
        locate->SetItemString("lastUsed", new PyInt(0));
    } else {
        locate->SetItemString("agentServiceType", PyStatic.NewNone());
    }

    // for mission agents....
    PyDict* mission = new PyDict();
        mission->SetItemString("agentServiceType", new PyString("mission"));
        // will need to check standings vs agent level to determine this boolean
        mission->SetItemString("available", new PyBool(true));

    PyTuple* services = new PyTuple(3);
        services->SetItem(0, new PyObject("util.KeyVal", research));
        services->SetItem(1, new PyObject("util.KeyVal", locate));
        services->SetItem(2, new PyObject("util.KeyVal", mission));
    res->SetItemString("services", services);

    // standings info for this agent.
    /** @todo  finish this.... */
    std::string msg = "Your personal standings must be ";
    msg += GetMinReqStanding(m_agentData.level);
    msg += " or higher toward this agent, its faction, or its corporation in order to use this agent's services.";
    res->SetItemString("incompatible", new PyString(msg));

    /* can also use locale labelIDs for this using a tuple to define minStandings, minEffective, corpMinStandings, mainEffective, effectiveMinStandings in other msgIDs
     * this will take char, corp, faction, agent, and some other shit into account to determine msg and data sent using the tuple system
    //  note:  this is kinda hacked right now.
    PyDict* dict = new PyDict();
        dict->SetItemString("minStandings", new PyFloat(GetMinReqStanding(m_agentData.level)));
       // dict->SetItemString("mainEffective", new PyFloat(GetMinReqStanding(m_agentData.level +10)));
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(235465));
        tuple->SetItem(1, dict);
    res->SetItemString("incompatible", tuple);
    */

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "Agent::GetInfoServiceDetails() Dump:" );
        res->Dump(AGENT__RSP_DUMP, "    ");
    }

    return new PyObject("util.KeyVal", res);
}

std::string Agent::GetMinReqStanding(uint8 level)
{
    switch(level) {
        // these are agentCorp -> char
        case 1:     return "-2.0";
        case 2:     return "1.0";
        case 3:     return "3.0";
        case 4:     return "5.0";
        case 5:     return "7.0";
        //these are agent/corp/faction -> char
        case 11:     return "-1.0";
        case 12:     return "1.5";
        case 13:     return "3.5";
        case 14:     return "5.5";
        case 15:     return "7.5";
    };

    return "-0.5";
}


uint32 Agent::GetAcceptRsp(uint32 charID)
{
    //(130431, 'Thank you.  Be careful out there, those logs must <b>not</b> fall into the wrong hands.')

    switch (MakeRandomInt(1, 13)) {
        case 1:   return 236738; // `Very well then, get going.`)
        case 2:   return 236739; // `Ok, hurry up will you.`)
        case 3:   return 236740; // `Good good, now get out there and give me some results.`)
        case 4:   return 236741; // `Thank you, and good luck.`)
        case 5:   return 236742; // `Thanks, I really appreciate it.`)
        case 6:   return 236743; // `Have fun!`)
        case 7:   return 236744; // `Now be careful out there, you hear me?`)
        case 8:   return 236745; // `Stay alive, friend.`)
        case 9:   return 236721; // `You have a long and prosperous future within {[npcOrganization]agentFactionID.name}, {[character]player.name}.`)
        case 10:  return 236746; // `I knew I could count on you.`)
        case 11:  return 236747; // `Great.  I know I can trust you with this, mate.`)
        case 12:  return 236749; // `Wonderful.  I expect a quick result with you on the job.`)
        case 13:  return 135877; // `I knew we could count on you, {[character]player.name}. Please hurry. ')
    }
    return 236745; // `Stay alive, friend.`)
}

uint32 Agent::GetDeclineRsp(uint32 charID)
{
    switch (MakeRandomInt(1, 14)) {
        case 1:   return 236693; // `It's your loss.`)
        case 2:   return 236695; // `Son, I am disappoint.`)
        case 3:   return 236793; // `Too bad, I'll try to find someone else then for that job.`)
        case 4:   return 236794; // `Bah, that mission wasn't that bad. Oh well, wait a bit and I'll come up with something else.`)
        case 5:   return 236796; // `Well, don't expect me to come up with something as good later on.`)
        case 6:   return 236797; // `Your wayward ways displease me.`)
        case 7:   return 236801; // `It's your loss.`)
        case 8:   return 236802; // `Your wayward ways displease me, young one.`)
        case 9:   return 236843; // `See if I offer this to you again... seriously try it.`)
        case 10:  return 130895; // 'Fine.  Be that way.  Asshole.')
        case 11:  return 133432; // 'Too bad, {[character]player.name}. You could have made some serious points with the powers-that-be on this one. ')
        case 12:  return 135858; // 'I'm sorry to hear that, {[character]player.name}. I'll find someone else, I suppose.')
        case 13:  return 135895; // 'Lollygagger')
        case 14:  return 136322; // 'I see. Easy money's not good enough, eh? Huh. ')
    }
    //(137499, 'Fine, I’ll get someone more capable. A word of advice: don’t repeat this display of cowardice and uselessness. We don’t tolerate such things in the Republic.  ')
    return 137499;
}

uint32 Agent::GetCompleteRsp(uint32 charID)
{
    switch (MakeRandomInt(1, 20)) {
        case 1:   return 236750; // `Fabulous.  I couldn't have asked for a better man for the job.`)
        case 2:   return 236751; // `Not bad.  Get back to me later for another assignment will you?`)
        case 3:   return 236752; // `Nice work.  I'm starting to like your style.`)
        case 4:   return 236753; // `Thanks, your services to {[npcOrganization]agentCorpID.name} is duly appreciated.`)
        case 5:   return 236754; // `Thank you.  Your accomplishment has been noted and saved into our database.`)
        case 6:   return 236755; // `Excellent job!`)
        case 7:   return 236756; // `Thanks, I really appreciate your help.`)
        case 8:   return 236757; // `I won't forget this.`)
        case 9:   return 236758; // `I'm grateful for your assistance.`)
        case 10:  return 236759; // `You have my gratitude.`)
        case 11:  return 236760; // `Thank you very much, I really appreciate it.`)
        case 12:  return 236761; // `Well done!  Take this reward and my gratitude as well.`)
        case 13:  return 236762; // `It's a pleasure doing business with you.`)
        case 14:  return 236763; // `Excellent work!  Care for another assignment?`)
        case 15:  return 236764; // `I look forward to your next visit.`)
        case 16:  return 236765; // `Again you finish the job right on time.  Keep this up and I'll probably get a promotion.`)
        case 17:  return 236766; // `I thank you from the bottom of my heart.`)
        case 18:  return 236767; // `If only I had more excellent pilots like you, {[character]player.name} ...`)
        case 19:  return 236768; // `Your talents as a pilot never cease to amaze me.  Keep up the good work!`)
        case 20:  return 139398; // 'I knew you were the right pilot for the job.')
    }
    return 236758; // `I'm grateful for your assistance.`)
}

//(235999, `You have failed the mission I gave you. I am disappointed in you. I was hoping for a little more competence.`)

uint32 Agent::GetQuitRsp(uint32 charID)
{
    //(236846, `Quitters never win.`)
    return 236846;
}

uint32 Agent::GetStandingsRsp(uint32 charID)
{
    switch (MakeRandomInt(1, 6)) {
        case 1:   return 236682; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.`)
        case 2:   return 236691; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.`)
        //case 1:   return 236708; // `Trial account users cannot access agents of level 3 or higher. Either use your map settings to find a level 1 or level 2 agent, or consider purchasing an EVE subscription if you wish to access higher-level content.`)
        case 3:   return 236785; // `What planet were you born on?  Check your standings next time or I'll drill a hole in you.`)
        case 4:   return 236787; // `Have you even bothered to check your standings.`)
        case 5:   return 236775; // `Hello, {[character]player.name}.  I was given the assignment of handing out a valuable item to loyal supporters of {[npcOrganization]agentFactionID.name}.  If you know of someone who has proven him or herself time and time again for {[npcOrganization]agentFactionID.nameWithArticle} then point that person to me and perhaps we can come to an 'arrangement' ...<br><br>You on the other hand do not meet my requirements or have already received my offer.`)
        case 6:   return 236853; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.`)
    }
    return 236853; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.`)
}


void Agent::UpdateStandings(Client* pClient, uint8 eventID, bool important/*false*/)
{
    /*
        [PyAddress Broadcast: broadcastType='charid', idType='OnStandingsModified']
    Payload:
        [NotificationStream Tuple01]
            [PyTuple 1 items]
                [PyList 2 items]
                    [PyList 5 items]
                        [PyInt 3013251]                 fromID
                        [PyInt 91576239]                toID
                        [PyFloat 0,00768598434114595]   change
                        [PyFloat -1]                    minAbs
                        [PyFloat 1]                     maxAbs
                        */
    Character* pChar = pClient->GetChar().get();
    uint32 charID = pChar->itemID();

    float charStanding = StandingDB::GetStanding(m_agentID, charID);
    float bonus = EvEMath::Agent::GetStandingBonus(charStanding, m_agentData.factionID, pChar->GetSkillLevel(EvESkill::Connections), pChar->GetSkillLevel(EvESkill::Diplomacy), pChar->GetSkillLevel(EvESkill::CriminalConnections));
    float standing = EvEMath::Agent::EffectiveStanding(charStanding, bonus);
    float quality = EvEMath::Agent::EffectiveQuality(m_agentData.quality, pChar->GetSkillLevel(EvESkill::Negotiation), standing);
    float newStanding = EvEMath::Agent::Efficiency(m_agentData.level, quality);    // 0.018 to 0.38
    SystemManager* pSysMgr = sEntityList.FindOrBootSystem(m_agentData.solarSystemID);
    if (pSysMgr != nullptr)
        newStanding *= pSysMgr->GetSecValue(); // 0.0018 to .76

    //newStanding = EvEMath::Agent::AgentStandingIncrease(standing, (newStanding /10));     -- this isnt used.
    newStanding = EvEMath::Agent::MissionStandingIncrease(newStanding, pChar->GetSkillLevel(EvESkill::Social));
    newStanding /= 8;

    newStanding *= sConfig.standings.BaseMissionMultiplier;

    if (important)
        newStanding *= sConfig.standings.ImportantMissionBonus;

    std::string msg = "Status Change for Mission ";
    switch (eventID) {
        case Standings::MissionFailedRollback: {
            msg += "failed ";
            newStanding *= sConfig.standings.MissionFailedRollback;
        } break;
        case Standings::MissionOfferExpired: {
            msg += "expired ";
            newStanding *= sConfig.standings.MissionOfferExpired;
        } break;
        case Standings::MissionBonus: {
            msg += "completion bonus ";
            newStanding *= sConfig.standings.MissionBonus;
        } break;
        case Standings::MissionCompleted: {
            msg += "completion ";
            newStanding *= sConfig.standings.MissionCompleted;
        } break;
        case Standings::MissionDeclined: {
            msg += "decline ";
            // test for mission declined in last 4 hours
            //newStanding *= sConfig.standings.MissionDeclined;
        } break;
        case Standings::MissionFailure: {
            msg += "failure ";
            newStanding *= sConfig.standings.MissionFailure;
        } break;
    }

    msg += "from ";
    msg += m_agentData.name;

    if (pClient->InFleet() and (newStanding > 0)) {
        float fleetStanding = newStanding * sConfig.standings.FleetMissionMultiplier;
        newStanding -= (fleetStanding /2);  // live does half,  but mission acceptor will get more here.
        // shared mission standings are from agent to character only.
        std::vector<Client*> clientVec;
        sFltSvc.GetFleetClientsInSystem(pClient, clientVec);
        for (auto cur : clientVec) {
            sStandingMgr.UpdateStandings(m_agentID, cur->GetCharacterID(), eventID, fleetStanding, msg);
            PyTuple* agent = new PyTuple(5);
                agent->SetItem(0, new PyInt(m_agentID));
                agent->SetItem(1, new PyInt(cur->GetCharacterID()));
                agent->SetItem(2, new PyFloat(fleetStanding));
                agent->SetItem(3, new PyInt(-1));
                agent->SetItem(4, PyStatic.NewOne());
            PyList* list = new PyList();
                list->AddItem(agent);
            PyTuple* payload = new PyTuple(1);
                payload->SetItem(0, list);
            cur->SendNotification("OnStandingsModified", "charid", payload, false);
            // fleet will share corp standings on some missions.  fix later.
        }
    }

    sStandingMgr.UpdateStandings(m_agentID, charID, eventID, newStanding, msg);
    sStandingMgr.UpdateStandings(m_agentData.corporationID, charID, eventID, newStanding * sConfig.standings.ACorp2CharMissionMultiplier, msg);
    sStandingMgr.UpdateStandings(m_agentData.factionID, charID, eventID, newStanding * sConfig.standings.AFaction2CharMissionMultiplier, msg);

    if (IsPlayerCorp(pClient->GetCorporationID())) {
        sStandingMgr.UpdateStandings(m_agentID, pClient->GetCorporationID(), eventID, newStanding * sConfig.standings.Agent2PCorpMissionMultiplier, msg);
        sStandingMgr.UpdateStandings(m_agentData.corporationID, pClient->GetCorporationID(), eventID, newStanding * sConfig.standings.ACorp2PCorpMissionMultiplier, msg);
        sStandingMgr.UpdateStandings(m_agentData.factionID, pClient->GetCorporationID(), eventID, newStanding * sConfig.standings.AFaction2PCorpMissionMultiplier, msg);
    }

    PyTuple* agent = new PyTuple(5);
        agent->SetItem(0, new PyInt(m_agentID));
        agent->SetItem(1, new PyInt(charID));
        agent->SetItem(2, new PyFloat(newStanding));
        agent->SetItem(3, new PyInt(-1));
        agent->SetItem(4, PyStatic.NewOne());
    PyTuple* corp = new PyTuple(5);
        corp->SetItem(0, new PyInt(m_agentData.corporationID));
        corp->SetItem(1, new PyInt(charID));
        corp->SetItem(2, new PyFloat(newStanding /4));
        corp->SetItem(3, new PyInt(-1));
        corp->SetItem(4, PyStatic.NewOne());
    PyTuple* faction = new PyTuple(5);
        faction->SetItem(0, new PyInt(m_agentData.factionID));
        faction->SetItem(1, new PyInt(charID));
        faction->SetItem(2, new PyFloat(newStanding /8));
        faction->SetItem(3, new PyInt(-1));
        faction->SetItem(4, PyStatic.NewOne());
    PyList* list = new PyList();
        list->AddItem(agent);
        list->AddItem(corp);
        list->AddItem(faction);
    PyTuple* payload = new PyTuple(1);
        payload->SetItem(0, list);

    if (is_log_enabled(STANDING__RSPDUMP)) {
        _log(STANDING__RSPDUMP, "Agent::UpdateStandings RSP:" );
        payload->Dump(STANDING__RSPDUMP, "    ");
    }

    pClient->SendNotification("OnStandingsModified", "charid", payload, false);    // i *think* this is unsequenced
}

void Agent::SendMissionUpdate(Client* pClient, std::string action)
{
    if (pClient == nullptr)
        return;
    //OnAgentMissionChange(action, agentID, tutorialID)
    /*
    agentMissionOffered = 'offered'
    agentMissionOfferAccepted = 'offer_accepted'
    agentMissionOfferDeclined = 'offer_declined'
    agentMissionOfferExpired = 'offer_expired'
    agentMissionOfferRemoved = 'offer_removed'
    agentMissionAccepted = 'accepted'
    agentMissionDeclined = 'declined'
    agentMissionCompleted = 'completed'
    agentTalkToMissionCompleted = 'talk_to_completed'
    agentMissionQuit = 'quit'
    agentMissionResearchUpdatePPD = 'research_update_ppd'
    agentMissionResearchStarted = 'research_started'
    agentMissionProlonged = 'prolong'
    agentMissionReset = 'reset'
    agentMissionModified = 'modified'       - force agent convo
    agentMissionFailed = 'failed'
    */

    PyTuple* payload = new PyTuple(3);
        payload->SetItem(0, new PyString(action));
        payload->SetItem(1, new PyInt(m_agentID));
        payload->SetItem(2, PyStatic.NewNone());    // NOTE if we ever get tutorials working, this will need to be fixed.
    pClient->SendNotification("OnAgentMissionChange", "charid", payload, false);    // i *think* this is unsequenced
}
    //specific to the calling action
    //OnInteractWith(agentID)       (force agent convo)

    /*
     * Level 1 = always available
     * Level 2 = +1.00 standing
     * Level 3 = +3.00 standing
     * Level 4 = +5.00 standing
     * Level 5 = +7.00 standing
     * (235462, 'Your personal standings must be -1.9 or higher toward this agent, its faction, or its corporation in order to use this agent's services.')
     * (235463, 'Your personal standings must be -1.9 or higher toward this agent, its faction, and its corporation in order to use this agent's services. Additionally, you need a minimum effective standing to this agent's corp of at least {[numeric]corpMinStandings, decimalPlaces=1} , as well as personal standing of at least {[numeric]effectiveMinStandings, decimalPlaces=1} to this agent's faction, corp, or to the agent in order to use this agent's services.')
     * (235464, 'Your personal standings must be -1.9 or higher toward this agent, its faction, and its corporation in order to use this agent's services. Additionally, you need a minimum effective standing of at least {[numeric]minStandings, decimalPlaces=1} to this agent's faction, corp, or to the agent in order to use this agent's services.')
     * (235465, 'Your effective personal standings must be {[numeric]minStandings, decimalPlaces=1} or higher toward this agent, its faction, or its corporation in order to use this agent's services')
     * (235466*, 'Your effective personal standings must be {[numeric]minEffective, decimalPlaces=1} or higher toward this agent's corporation in order to use this agent, as well as an effective personal standing of {[numeric]mainEffective, decimalPlaces=1} or higher toward this agent, its faction, or its corporation in order to use this agent's services.')
     * (235467, 'This agent can only be used through a direct referral.')
     */

bool Agent::CanUseAgent(Client* pClient)
{
    /** @todo this needs work!!  */
    if (m_agentData.typeID == Agents::Type::Aura)
        return true;
    if (m_agentData.level == 1)
        if (m_agentData.typeID != Agents::Type::Research)
            return true;

    Character* pChar = pClient->GetChar().get();
    uint32 charID = pChar->itemID();
    uint8 sConn = pChar->GetSkillLevel(EvESkill::Connections);
    uint8 sDiplo = pChar->GetSkillLevel(EvESkill::Diplomacy);
    uint8 sCrim = pChar->GetSkillLevel(EvESkill::CriminalConnections);
    float charStanding = StandingDB::GetStanding(m_agentID, charID);
    float bonus = EvEMath::Agent::GetStandingBonus(charStanding, m_agentData.factionID, sConn, sDiplo, sCrim);
    float standing = EvEMath::Agent::EffectiveStanding(charStanding, bonus);

    float facChr = StandingDB::GetStanding(m_agentData.factionID, charID);
    float corpChr = StandingDB::GetStanding(m_agentData.corporationID, charID);
    float charChr = StandingDB::GetStanding(m_agentID, charID);
    float facBonus = EvEMath::Agent::GetStandingBonus(facChr, m_agentData.factionID, sConn, sDiplo, sCrim);
    float corpBonus = EvEMath::Agent::GetStandingBonus(corpChr, m_agentData.factionID, sConn, sDiplo, sCrim);
    float charBonus = EvEMath::Agent::GetStandingBonus(charChr, m_agentData.factionID, sConn, sDiplo, sCrim);

    if (facBonus > 0.0f)
        facChr = (1.0 - (1.0 - facChr / 10.0) * (1.0 - facBonus / 10.0)) * 10.0;
    if (corpBonus > 0.0f)
        corpChr = (1.0 - (1.0 - corpChr / 10.0) * (1.0 - corpBonus / 10.0)) * 10.0;
    if (charBonus > 0.0f)
        charChr = (1.0 - (1.0 - charChr / 10.0) * (1.0 - charBonus / 10.0)) * 10.0;

    float m = (m_agentData.level - 1) * 2.0f - 1.0f;

    _log(AGENT__DEBUG, "%s(%u) CanUseAgent() - charSkills(con:%u,dip:%u,cri:%u), stand(%f, %f, %f)",\
                m_agentData.name.c_str(), m_agentID, sConn, sDiplo, sCrim, charStanding, bonus, standing);
    _log(AGENT__DEBUG, "%s(%u) CanUseAgent() - standings(fac:%.3f,crp:%.3f,chr:%.3f), bonus(%f, %f, %f) - m=%f", \
                m_agentData.name.c_str(), m_agentID, facChr, corpChr, charChr, facBonus, corpBonus, charBonus, m);

    if ((EvE::max(facChr, corpChr, charChr) >= m ) and (EvE::min(facChr, corpChr, charChr) > -2.0f)) {
        if ((m_agentData.typeID == Agents::Type::Research) and (corpChr < m - 2.0f))
            return false;
        return true;
    }

    return false;
}



/*  mission errata....
 *
 * courier missions:
 * L1 missions will keep you within the agent's constellation, up to 450 m3
 * L2/L3 will possibly send you to a neighboring constellation,4-6km3
 * L4 missions will always send you to a neighboring constellation. 8km3
 * If a Distribution mission has an item as a reward instead of ISK, then the item will appear in your personal hangar at the agent's station
 *
 * mining missions:
 * Mining missions require you to mine an asteroid or set of asteroids in a mission space, usually until the asteroids are depleted, and bring the ore back to the agent's station.
 * There is a risk of combat in mining missions, though the hostiles that show up tend to be much weaker than hostiles found in security missions.
 * It is advisable to have some offensive capability (like a set of combat drones) or have a strong enough tank that you can ignore any hostiles that show up and start shooting at you.
 * The mission may require you to mine more ore than can fit in your cargohold; this is typical of mining missions.
 * L1 missions will require mining up to 2km3 of ore,
 * L2 up to 6km3 of ore,
 * L3 up to 9km3 of ore or 10km3 of ice,
 * L4 up to 45km3 of ore, 20km3 of ice or 5km3 gas.
 *
 * encounter missions:
 * Level 1 is where most new players start. Most, if not all, level 1 missions can be done in a basic frigate, Only the most basic piloting skills are required.
 * Level 2 mining missions can be done in a cruiser or in a destroyer piloted by a more skilled pilot. These missions generally expect that you are continually improving your piloting skills and learning how to fit out new ships.
 * Level 3 missions require a battlecruiser. These missions go faster if you have trained for better ships and at least some Tech 2 fittings.
 * Level 4 missions require a Battleship with full T2 tank fitted. These missions can be time-consuming, but they offer large rewards.
 * Level 5 missions are designed for groups of players or capital ship and are exclusively located in Low Security space.
 */


/* agent errata...
 *
 * char mission history and current offers in agtOffers
 * notify:OnIncomingAgentMessage(agentID, message)
 * OnIncomingTransmission(self, transmission, isAgentMission = 0, *args)
 *  transmission.header, transmission.text, transmission.icon
 *        if transmission.header.startswith('[d]'):
 *            transmission.header = transmission.header[3:]
 *            self.delayedTransmission = transmission
 *
 *            (237604, `You can only do this on MASTER or LOCAL server`)
 *            (235975, `I am in need of your services, {[character]notification_receiverID.name, linkify}, for a very special mission.`)
 *            (235976, `I found {[character]charID.name, linkify} for you`)
 *
 *            The forumla for LP reward is:
 *            LP reward = (1.6288 - System security) × Base LP
 *
 *
 ****  see also shit in common/utils/evemath.*  *******
 * Agent_Effective_Quality = Agent_Quality + (5 * Negotiation_Skill_Level) + Round_Down(AgentPersonalStanding)
 *
 * Research_Points_Per_Day = Multiplier * ((1 + (Agent_Effective_Quality / 100)) * ((Core_Skill + Agent_Skill) ^ 2))
 *
 * RP/Day = ((Agent Level + Your Skill)^2 * (1 + (20 + 5 * Negotiation Skill + Agent Effective Standing) / 100)) * Multiplier
 * ((Agent Skill + Your Skill)^2 * (1 + Effective Quality / 100)) * Area Bonus
 */
