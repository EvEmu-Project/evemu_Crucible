
 /**
  * @name MissionDataMgr.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */

#include "../EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "agents/Agent.h"
#include "agents/AgentMgrService.h"
#include "database/EVEDBUtils.h"
#include "missions/MissionDataMgr.h"
#include "inventory/ItemFactory.h"

MissionDataMgr::MissionDataMgr()
{
    m_procCount = 0;
    m_names.clear();
    m_offers.clear();
    m_mining.clear();
    m_courier.clear();
    m_xoffers.clear();
    m_missions.clear();
}

MissionDataMgr::~MissionDataMgr()
{
    PyDecRef(KillPNG);
    PyDecRef(MiningPNG);
    PyDecRef(CourierPNG);
}

void MissionDataMgr::Clear()
{
    m_names.clear();
    m_offers.clear();
    m_mining.clear();
    m_courier.clear();
    m_xoffers.clear();
    m_missions.clear();
}

int MissionDataMgr::Initialize()
{
    Populate();
    sLog.Blue("   MissionDataMgr", "Mission Data Manager Initialized.");
    return 1;
}

void MissionDataMgr::GetInfo()
{
    // not used yet
}

// called every minute from EntityList::Process()
void MissionDataMgr::Process()
{
    // process open offers every 5m
    if (++m_procCount > 5) {
        m_procCount = 0;

        Agent* pAgent(nullptr);
        Client* pClient(nullptr);
        std::multimap<uint32, MissionOffer>::iterator itr = m_offers.begin();
        while (itr != m_offers.end()) {
            if (itr->second.expiryTime < GetFileTimeNow()) {
                pAgent = sEntityList.GetAgent(itr->second.agentID);
                pClient = sEntityList.FindClientByCharID(itr->first);
                // notify client if they are online.  eventaully we'll send mail also
                if (itr->second.stateID == Mission::State::Accepted) {
                    pAgent->SendMissionUpdate(pClient, "failed");
                    itr->second.stateID = Mission::State::Failed;
                    if (itr->second.courierTypeID) {
                        // remove item from player's possession
                        if (pClient != nullptr) {
                            pClient->RemoveMissionItem(itr->second.courierTypeID, itr->second.courierAmount);
                        } else {
                            MissionDB::RemoveMissionItem(itr->first, itr->second.courierTypeID, itr->second.courierAmount);
                        }
                    }
                } else if (itr->second.stateID == Mission::State::Offered) {
                    pAgent->SendMissionUpdate(pClient, "offer_expired");
                    itr->second.stateID = Mission::State::Expired;
                }
                std::multimap<uint32, MissionOffer>::iterator itr2 = m_aoffers.find(itr->second.agentID);
                if (itr2 != m_aoffers.end())
                    m_aoffers.erase(itr2);
                m_xoffers.emplace(itr->first, itr->second);
                pAgent->RemoveOffer(itr->first);
                MissionDB::UpdateMissionOffer(itr->second);
                itr = m_offers.erase(itr);
                pAgent = nullptr;
                pClient = nullptr;
            } else {
                ++itr;
            }
        }
    }
}


void MissionDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    double begin = GetTimeMSeconds();

    CourierPNG = new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>");
    MiningPNG = new PyString("<img src='res:/UI/netres/mission_content/miningmission.png' align=center hspace=4 vspace=4>");
    KillPNG = new PyString("<img src='res:/UI/netres/mission_content/killmission.png' align=center hspace=4 vspace=4>");
    /*  not sure if these are used/needed....
    PNG = new PyString("<img src='res:/UI/netres/mission_content/agent_interaction.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/agent_talkto.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_amarr.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_caldari.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_gallente.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_minmatar.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_npe.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/blood_stained.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/angels_and_artifacts.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/smash_and_grab.png' align=center hspace=4 vspace=4>");
    */

    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    MissionDB::LoadCourierData(*res);
    while (res->GetRow(row)) {
        //SELECT id, briefingID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime, sysRange, raceID FROM qstCourier
        CourierData data = CourierData();
        data.missionID     = row.GetInt(0);
        data.briefingID    = row.GetInt(1);
        data.name          = row.GetText(2);
        data.level         = row.GetInt(3);
        data.typeID        = row.GetInt(4);
        data.important     = row.GetBool(5);
        data.storyline     = row.GetBool(6);
        data.itemTypeID    = row.GetInt(7);
        data.itemQty       = row.GetInt(8);
        data.itemVolume    = row.GetFloat(9);
        data.rewardISK     = row.GetInt(10);
        data.rewardItemID  = row.GetInt(11);
        data.rewardItemQty = row.GetInt(12);
        data.bonusISK      = row.GetInt(13);
        data.bonusTime     = row.GetInt(14);
        data.range         = row.GetInt(15);
        data.raceID        = row.GetInt(16);
        if (data.important) {
            m_courierImp.emplace(row.GetInt(3), data);
        } else {
            m_courier.emplace(row.GetInt(3), data);
        }
    }
    sLog.Cyan("   MissionDataMgr", "%lu(%lu) Courier Mission Data Sets loaded in %.3fms.", m_courier.size(), m_courierImp.size(),(GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMiningData(*res);
    while (res->GetRow(row)) {
        //SELECT id, briefingID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime, sysRange, raceID FROM qstMining
        CourierData data = CourierData();
        data.missionID     = row.GetInt(0);
        data.briefingID    = row.GetInt(1);
        data.name          = row.GetText(2);
        data.level         = row.GetInt(3);
        data.typeID        = row.GetInt(4);
        data.important     = row.GetBool(5);
        data.storyline     = row.GetBool(6);
        data.itemTypeID    = row.GetInt(7);
        data.itemQty       = row.GetInt(8);
        data.itemVolume    = row.GetFloat(9);
        data.rewardISK     = row.GetInt(10);
        data.rewardItemID  = row.GetInt(11);
        data.rewardItemQty = row.GetInt(12);
        data.bonusISK      = row.GetInt(13);
        data.bonusTime     = row.GetInt(14);
        data.range         = row.GetInt(15);
        data.raceID        = row.GetInt(16);
        if (data.important) {
            m_miningImp.emplace(row.GetInt(3), data);
        } else {
            m_mining.emplace(row.GetInt(3), data);
        }
    }
    sLog.Cyan("   MissionDataMgr", "%lu(%lu) Mining Mission Data Sets loaded in %.3fms.", m_mining.size(), m_miningImp.size(), (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Encounter Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Storyline Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Tutorial Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Research Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Anomic Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Data Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Trade Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Burner Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Cosmos Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Arc Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMissionData(*res);
    while (res->GetRow(row)) {
        //SELECT id, briefingID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID,
        // rewardISK, rewardItemID, rewardISKQty, rewardItemQty, bonusISK, bonusTime FROM agtMissions
        MissionData data = MissionData();
        data.missionID = row.GetInt(0);
        data.briefingID = row.GetInt(1);
        data.name = row.GetText(2);
        data.level = row.GetInt(3);
        data.typeID = row.GetInt(4);
        data.important = row.GetBool(5);
        data.constellationID = row.GetInt(8);
        data.corporationID = row.GetInt(9);
        data.dungeonID = row.GetInt(10);
        if (data.important) {
            m_missionsImp.emplace(row.GetInt(3), data);
        } else {
            m_missions.emplace(row.GetInt(3), data);
        }
    }
    sLog.Cyan("   MissionDataMgr", "%lu(%lu) Unsorted Mission Data Sets loaded in %.3fms.", m_missions.size(), m_missionsImp.size(), (GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadOpenOffers(*res);
    while (res->GetRow(row)) {
        //SELECT acceptFee, agentID, characterID, courierAmount, courierTypeID, courierItemVolume, dateAccepted, dateIssued, destinationID, destinationTypeID, destinationOwnerID, destinationSystemID,
        // expiryTime, important, storyline, missionID, briefingID, name, offerID, originID, originOwnerID, originSystemID, remoteCompletable, remoteOfferable,
        // rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID FROM agtOffers
        MissionOffer offer = MissionOffer();
        offer.acceptFee = row.GetInt(0);
        offer.agentID = row.GetInt(1);
        offer.characterID = row.GetInt(2);
        offer.courierAmount = row.GetInt(3);
        offer.courierTypeID = row.GetInt(4);
        offer.courierItemVolume = row.GetFloat(5);
        offer.dateAccepted = row.GetInt64(6);
        offer.dateIssued = row.GetInt64(7);
        offer.destinationID = row.GetInt(8);
        offer.destinationTypeID = row.GetInt(9);
        offer.destinationOwnerID = row.GetInt(10);
        offer.destinationSystemID = row.GetInt(11);
        offer.expiryTime = row.GetInt64(12);
        offer.important = row.GetInt(13);
        offer.storyline = row.GetInt(14);
        offer.missionID = row.GetInt(15);
        offer.briefingID = row.GetInt(16);
        offer.name = row.GetText(17);
        offer.offerID = row.GetInt(18);
        offer.originID = row.GetInt(19);
        offer.originOwnerID = row.GetInt(20);
        offer.originSystemID = row.GetInt(21);
        offer.remoteCompletable = row.GetInt(22);
        offer.remoteOfferable = row.GetInt(23);
        offer.rewardISK = row.GetInt(24);
        offer.rewardItemID = row.GetInt(25);
        offer.rewardItemQty = row.GetInt(26);
        offer.rewardLP = row.GetInt(27);
        offer.bonusISK = row.GetInt(28);
        offer.bonusTime = row.GetInt(29);
        offer.stateID = row.GetInt(30);
        offer.typeID = row.GetInt(31);
        offer.dungeonLocationID = row.GetInt(32);
        offer.dungeonSolarSystemID = row.GetInt(33);
        offer.dateCompleted = 0;
        // will need to determine how to store/retrieve bookmarks as a list of dicts here
        offer.bookmarks = new PyList();
        m_offers.emplace(row.GetInt(2), offer);
        m_aoffers.emplace(row.GetInt(1), offer);    // do we really want dupe data here?  yes.  need offer by char and by agent
    }
    sLog.Cyan("   MissionDataMgr", "%lu Open Mission Offers loaded in %.3fms.", m_offers.size(), (GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    // config switch to allow loading/displaying of expired/completed mission offers
    if (sConfig.server.LoadOldMissions)
        MissionDB::LoadClosedOffers(*res);
    while (res->GetRow(row)) {
        //SELECT agentID, characterID, courierAmount, courierTypeID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline, missionID, name,
        // offerID, originID, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID FROM agtOffers
        MissionOffer offer = MissionOffer();
        offer.agentID = row.GetInt(0);
        offer.characterID = row.GetInt(1);
        offer.courierAmount = row.GetInt(2);
        offer.courierTypeID = row.GetInt(3);
        offer.dateAccepted = row.GetInt64(4);
        offer.dateCompleted = row.GetInt64(5);
        offer.dateIssued = row.GetInt64(6);
        offer.destinationID = row.GetInt(7);
        offer.expiryTime = row.GetInt64(8);
        offer.important = row.GetInt(9);
        offer.storyline = row.GetInt(10);
        offer.missionID = row.GetInt(11);
        offer.name = row.GetText(12);
        offer.offerID = row.GetInt(13);
        offer.originID = row.GetInt(14);
        offer.rewardISK = row.GetInt(15);
        offer.rewardItemID = row.GetInt(16);
        offer.rewardItemQty = row.GetInt(17);
        offer.rewardLP = row.GetInt(18);
        offer.stateID = row.GetInt(19);
        offer.typeID = row.GetInt(20);
        offer.briefingID = 0;
        offer.acceptFee = 0;
        offer.bonusISK = 0;
        offer.bonusTime = 0;
        offer.remoteCompletable = 0;
        offer.remoteOfferable = 0;
        offer.originOwnerID = 0;
        offer.originSystemID = 0;
        offer.destinationTypeID = 0;
        offer.destinationOwnerID = 0;
        offer.destinationSystemID = 0;
        offer.dungeonLocationID = 0;
        offer.dungeonSolarSystemID = 0;
        offer.bookmarks = new PyList(); //invalid offers will not have bms
        m_xoffers.emplace(row.GetInt(2), offer);
    }
    sLog.Cyan("   MissionDataMgr", "%lu Closed Mission Offers loaded in %.3fms.", m_xoffers.size(), (GetTimeMSeconds() - start));

    // cleanup
    SafeDelete(res);
/*
    m_names.emplace("Arisite Envy",   45000 );
    m_names.emplace("Asteroid Catastrophe",    1080 );
    m_names.emplace("Better World", 6000    );
    m_names.emplace("Beware They Live",    9000   );
    m_names.emplace("Bountiful Bandine",   2000         );
    m_names.emplace("Burnt Traces",    1080           );
    m_names.emplace("Cheap Chills",    20000  );
    m_names.emplace("Claimjumpers",    1800       );
    m_names.emplace("Data Mining", 299   );
    m_names.emplace("Down and Dirty",  2250       );
    m_names.emplace("Drone Distribution",  4000   );
    m_names.emplace("Feeding the Giant",   44800 );
    m_names.emplace("Gas Injections",  4250  );
    m_names.emplace("Geodite and Gemology",    44800 );
    m_names.emplace("Ice Installation",    20000  );
    m_names.emplace("Like Drones to a Cloud",  4250 );
    m_names.emplace("Mercium Belt",    6000   );
    m_names.emplace("Mercium Experiments", 1080     );
    m_names.emplace("Mother Lode", 44800  );
    m_names.emplace("Not Gneiss at All",   45000 );
    m_names.emplace("Pile of Pithix",  9000   );
    m_names.emplace("Persistent Pests",    4000   );
    m_names.emplace("Starting Simple", 2000     );
    m_names.emplace("Stay Frosty", 10000   );
    m_names.emplace("Understanding Augmene",   2625  );
    m_names.emplace("Unknown Events",  6000 );
    */
    sLog.Cyan("   MissionDataMgr", "Mission Data loaded in %.3fms.", (GetTimeMSeconds() - begin));
}

void MissionDataMgr::AddMissionOffer(uint32 charID, MissionOffer& data)
{
    m_offers.emplace(charID, data);
    m_aoffers.emplace(data.agentID, data);
}

void MissionDataMgr::RemoveMissionOffer(uint32 charID, MissionOffer& data)
{
    auto itr = m_offers.equal_range(charID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.agentID == data.agentID) {
            m_offers.erase(it);
            break;
        }

    itr = m_aoffers.equal_range(data.agentID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.characterID == charID) {
            m_aoffers.erase(it);
            break;
        }
}

void MissionDataMgr::LoadAgentOffers(const uint32 agentID, std::map< uint32, MissionOffer >& data)
{
    auto itr = m_aoffers.equal_range(agentID);
    for (auto it = itr.first; it != itr.second; ++it)
        data[it->second.characterID] = (it->second);
}

void MissionDataMgr::LoadMissionOffers(uint32 charID, std::vector<MissionOffer>& data)
{
    auto itr = m_offers.equal_range(charID);
    for (auto it = itr.first; it != itr.second; ++it)
        data.push_back(it->second);

    // config switch to allow loading/displaying of expired/completed mission offers
    // not completely working yet.....AgentMgrService::Handle_GetMyJournalDetails() will need work to implement this.
    if (sConfig.server.LoadOldMissions) {
        auto itr = m_xoffers.equal_range(charID);
        for (auto it = itr.first; it != itr.second; ++it)
            data.push_back(it->second);
    }
}

void MissionDataMgr::CreateMissionOffer(uint8 typeID, uint8 level, uint8 raceID, bool important, MissionOffer& data)
{
    // variable mission data based on agent, init to 0 here.
    data.stateID                = Mission::State::Allocated;
    data.dateIssued             = GetFileTimeNow();
    data.remoteOfferable        = false;
    data.remoteCompletable      = false;
    data.range                  = 0;
    data.offerID                = 0;
    data.agentID                = 0;
    data.rewardLP               = 0;
    data.originID               = 0;
    data.originOwnerID          = 0;
    data.originSystemID         = 0;
    data.acceptFee              = 0;
    data.expiryTime             = 0;
    data.characterID            = 0;
    data.dateAccepted           = 0;
    data.dateCompleted          = 0;
    data.destinationID          = 0;
    data.destinationTypeID      = 0;
    data.destinationOwnerID     = 0;
    data.destinationSystemID    = 0;
    data.dungeonLocationID      = 0;
    data.dungeonSolarSystemID   = 0;
    data.bookmarks              = new PyList();

    /** @todo  this will need to be adjusted for raceID eventually */
    switch (typeID) {
        case Mission::Type::Courier: {
            CourierData cData = CourierData();
            std::vector<CourierData> cVec;
            if (important) {
                auto itr = m_courierImp.equal_range(level);
                for (auto it = itr.first; it != itr.second; ++it)
                    cVec.push_back(it->second);
            } else {
                auto itr = m_courier.equal_range(level);
                for (auto it = itr.first; it != itr.second; ++it)
                    cVec.push_back(it->second);
            }
            cData = cVec[MakeRandomInt(0, (cVec.size() -1))];
            // verify mission race acceptable
            if ((cData.raceID) and ((cData.raceID & raceID) != raceID)) {
                for (auto cur :cVec) {
                    if ((cur.raceID & raceID) == raceID)
                        cData = cur;
                }
            }

            data.name               = cData.name;
            data.typeID             = cData.typeID;
            data.bonusISK           = cData.bonusISK;
            data.rewardISK          = cData.rewardISK;
            data.bonusTime          = cData.bonusTime;
            data.important          = cData.important;
            data.storyline          = cData.storyline;
            data.missionID          = cData.missionID;
            data.briefingID         = cData.briefingID;
            data.rewardItemID       = cData.rewardItemID;
            data.rewardItemQty      = cData.rewardItemQty;
            data.courierTypeID      = cData.itemTypeID;
            data.courierAmount      = cData.itemQty;
            data.courierItemVolume  = cData.itemVolume;
            data.range              = cData.range;
        } break;
        case Mission::Type::Mining: {
            CourierData cData = CourierData();
            std::vector<CourierData> cVec;
            if (important) {
                auto itr = m_miningImp.equal_range(level);
                for (auto it = itr.first; it != itr.second; ++it)
                    cVec.push_back(it->second);
            } else {
                auto itr = m_mining.equal_range(level);
                for (auto it = itr.first; it != itr.second; ++it)
                    cVec.push_back(it->second);
            }
            cData = cVec[MakeRandomInt(0, (cVec.size() -1))];

            data.name               = cData.name;
            data.typeID             = cData.typeID;
            data.bonusISK           = cData.bonusISK;
            data.rewardISK          = cData.rewardISK;
            data.bonusTime          = cData.bonusTime;
            data.important          = cData.important;
            data.storyline          = cData.storyline;
            data.missionID          = cData.missionID;
            data.briefingID         = cData.briefingID;
            data.rewardItemID       = cData.rewardItemID;
            data.rewardItemQty      = cData.rewardItemQty;
            data.courierTypeID      = cData.itemTypeID;
            data.courierAmount      = cData.itemQty;
            data.courierItemVolume  = cData.itemVolume;
            data.range              = cData.range;
        } break;
        case Mission::Type::Tutorial: {
        } break;
        case Mission::Type::Encounter: {
        } break;
        case Mission::Type::Trade: {
        } break;
        case Mission::Type::Research: {
        } break;
        case Mission::Type::Data: {
        } break;
        case Mission::Type::Storyline: {
        } break;
        case Mission::Type::Cosmos: {
        } break;
        case Mission::Type::Arc: {
        } break;
        case Mission::Type::Anomic: {
        } break;
    }

    _log(AGENT__DEBUG, "Created %s level %u %s offer - '%s'", (important?"an important":"a"), level, GetTypeName(data.typeID).c_str(), data.name.c_str());
}


std::string MissionDataMgr::GetTypeName(uint8 typeID)
{
    using namespace Mission::Type;
    switch (typeID) {
        case Tutorial:          return "Tutorial";
        case Encounter:         return "Encounter";
        case Courier:           return "Courier";
        case Trade:             return "Trade";
        case Mining:            return "Mining";
        case Research:          return "Research";
        case Data:              return "Data";
        case Storyline:         return "Storyline";
        case Cosmos:            return "Cosmos";
        case Arc:               return "Arc";
        case Anomic:            return "Anomic";
        case Burner:            return "Burner";
        default:                return "Invalid";
    }
}

std::string MissionDataMgr::GetTypeLabel(uint8 typeID)
{
    using namespace Mission::Type;
    switch (typeID) {
        case Tutorial:          return "UI/Agents/MissionTypes/Tutorial";
        case Encounter:         return "UI/Agents/MissionTypes/Encounter";
        case Courier:           return "UI/Agents/MissionTypes/Courier";
        case Trade:             return "UI/Agents/MissionTypes/Trade";
        case Mining:            return "UI/Agents/MissionTypes/Mining";
        case Research:          return "UI/Agents/MissionTypes/Research";
        case Data:              return "UI/Agents/MissionTypes/Data";
        case Storyline:         return "UI/Agents/MissionTypes/Storyline";
        case Cosmos:            return "UI/Agents/MissionTypes/Cosmos";
        case Arc:               return "UI/Agents/MissionTypes/EpicArc";
        case Anomic:            return "UI/Agents/MissionTypes/Anomic";
        case Burner:            return "UI/Agents/MissionTypes/Burner";
        default:                return "Invalid";
    }
}

void MissionDataMgr::UpdateMissionData(uint32 charID, MissionOffer& data)
{
    auto itr = m_offers.equal_range(charID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.agentID == data.agentID) {
            it->second = data;
            break;
        }

    itr = m_aoffers.equal_range(data.agentID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.characterID == charID) {
            it->second = data;
            break;
        }
}
