
 /**
  * @name FleetService.cpp
  *     Fleet Service code for EVEmu
  *     This singleton object is used to access and manipulate all dynamic fleet data
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

//work in progress

#include "eve-server.h"
#include "chat/LSCService.h"
#include "fleet/FleetService.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

/*
FLEET__ERROR
FLEET__WARNING
FLEET__MESSAGE
FLEET__DEBUG
FLEET__INFO
FLEET__TRACE
FLEET__DUMP
FLEET__BIND_DUMP
*/

//  Manager class functions and methods...

FleetService::FleetService()
: m_services(nullptr),
m_initalized(false)
{
}

void FleetService::Initialize(PyServiceMgr* svc)
{
    m_services = svc;

    m_joinReq.clear();
    m_inviteData.clear();
    m_fleetWings.clear();
    m_wingSquads.clear();
    m_wingDataMap.clear();
    m_fleetMembers.clear();
    m_fleetDataMap.clear();
    m_squadDataMap.clear();
    m_fleetAdvertMap.clear();

    //  these will have to be incremented individually, then stored according to fleet
    m_fleetID = FLEET_ID;  //950000000
    m_wingID = WING_ID;   //960000000
    m_squadID = SQUAD_ID; //970000000

    m_initalized = true;
    sLog.Blue("     FleetService", "Fleet Service Initialized.");
}


uint32 FleetService::CreateFleet(Client *pClient)
{
    if (pClient == nullptr)
        return 0;

    Character* pChar = pClient->GetChar().get();
    if (pChar == nullptr)
        return 0;

    FleetData fData = FleetData();
        fData.isFreeMove = true;
        fData.isLootLogging = false;
        fData.isRegistered = false;
        fData.isVoiceEnabled = sConfig.chat.EnableVoiceChat;   //false
        fData.creator = pClient;
        fData.leader = pClient;
        fData.booster = pClient;
        fData.name = pChar->itemName();
        fData.name += "'s Fleet";
        fData.motd = "default motd";
        fData.isMutedByLeader.clear();
        fData.isExcludedFromMuting.clear();
        fData.squads = 1;
        fData.dateCreated = GetFileTimeNow();
    m_fleetDataMap.emplace(m_fleetID, fData);

    WingData wData = WingData();
        wData.fleetID = m_fleetID;
        wData.name = "Wing 1";
        wData.booster = nullptr;
        wData.leader = nullptr;
    m_wingDataMap.emplace(m_wingID, wData);
    m_fleetWings.emplace(m_fleetID, m_wingID);

    BoostData bData = BoostData();
        bData.armored = pChar->GetSkillLevel(EvESkill::ArmoredWarfare);
        bData.info = pChar->GetSkillLevel(EvESkill::InformationWarfare);
        bData.mining = pChar->GetSkillLevel(EvESkill::MiningForeman);
        bData.siege = pChar->GetSkillLevel(EvESkill::SiegeWarfare);
        bData.skirmish = pChar->GetSkillLevel(EvESkill::SkirmishWarfare);
    SquadData sData = SquadData();
        sData.boost = bData;
        sData.name = "Squad 1";
        sData.booster = nullptr;
        sData.leader = nullptr;
        sData.wingID = m_wingID;
        sData.fleetID = m_fleetID;
        sData.members.clear();
    m_squadDataMap.emplace(m_squadID, sData);
    m_wingSquads.emplace(m_wingID, m_squadID);

    // set char fleet data and send to client
    CharFleetData fleet = CharFleetData();
        fleet.fleetID = m_fleetID;        //this is also lsc channel #
        fleet.wingID = -1;
        fleet.squadID = -1;
        fleet.job = Fleet::Job::Creator;
        fleet.role = Fleet::Role::FleetLeader;
        fleet.booster = Fleet::Booster::Fleet;
        fleet.joinTime = GetFileTimeNow();
    pChar->SetFleetData(fleet);

    // update the fleet member map with new member for this fleet.
    m_fleetMembers.emplace(m_fleetID, pClient);

    _log(FLEET__INFO, "FleetService::CreateFleet() - fleetID: %u, wingID: %u, squadID: %u, leaderID: %u", m_fleetID, m_wingID, m_squadID, pChar->itemID());

    sLog.Cyan("CreateFleet", "bData -- leader: %i, armored: %i, info: %i, siege: %i, skirmish: %i, mining: %i", \
            sData.boost.leader, sData.boost.armored, sData.boost.info, sData.boost.siege, sData.boost.skirmish, sData.boost.mining);

    if (sConfig.chat.EnableFleetChat)
        m_services->lsc_service->CreateSystemChannel(m_fleetID);
    if (sConfig.chat.EnableWingChat)
        m_services->lsc_service->CreateSystemChannel(m_wingID);
    if (sConfig.chat.EnableSquadChat)
        m_services->lsc_service->CreateSystemChannel(m_squadID);

    // increment counters after channels are created (to avoid wrong channel creation)
    ++m_fleetID;
    ++m_wingID;
    ++m_squadID;

    return fleet.fleetID;
}

PyRep* FleetService::CreateWing(uint32 fleetID)
{
    int8 count = m_fleetWings.count(fleetID);
    // do we need an error here?
    if (count > 4)
        return nullptr;
    WingData wData = WingData();
        wData.fleetID = fleetID;
        wData.name = "Wing ";
        wData.name += std::to_string(count + 1);
        wData.booster = nullptr;
        wData.leader = nullptr;
    m_wingDataMap.emplace(m_wingID, wData);
    m_fleetWings.emplace(fleetID, m_wingID);

    _log(FLEET__INFO, "FleetService::CreateWing() - fleetID: %u, wingID: %u", fleetID, m_wingID);

    PyTuple* tuple1 = new PyTuple(1);
        tuple1->SetItem(0, new PyInt(m_wingID));
    SendFleetUpdate(fleetID, "OnFleetWingAdded", tuple1);

    if (sConfig.chat.EnableWingChat)
        m_services->lsc_service->CreateSystemChannel(m_wingID);

    std::list<int32> wing, squad;
    wing.clear();
    squad.clear();
    wing.emplace(wing.end(), m_wingID);
    UpdateBoost(fleetID, true, wing, squad);

    PyInt* res = new PyInt(m_wingID);
    ++m_wingID;
    return res;
}

void FleetService::CreateSquad(uint32 fleetID, uint32 wingID)
{
    // do we need an error here?
    if (m_wingSquads.count(wingID) == 5)
        return;
    IncFleetSquads(fleetID, wingID);
    FleetData data = FleetData();
    GetFleetData(fleetID, data);
    BoostData bData = BoostData();
    SquadData sData = SquadData();
        sData.boost = bData;
        sData.name = "Squad ";
        sData.name += std::to_string(data.squads);
        sData.wingID = wingID;
        sData.fleetID = fleetID;
        sData.booster = nullptr;
        sData.leader = nullptr;
        sData.members.clear();
    m_squadDataMap.emplace(m_squadID, sData);
    m_wingSquads.emplace(wingID, m_squadID);

    _log(FLEET__INFO, "FleetService::CreateSquad() - fleetID: %u, wingID: %u, squadID: %u", fleetID, wingID, m_squadID);

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(wingID));
        tuple->SetItem(1, new PyInt(m_squadID));
    SendFleetUpdate(fleetID, "OnFleetSquadAdded", tuple);

    if (sConfig.chat.EnableSquadChat)
        m_services->lsc_service->CreateSystemChannel(m_squadID);

    std::list<int32> wing, squad;
    wing.clear();
    wing.emplace(wing.end(), wingID);
    squad.clear();
    squad.emplace(squad.end(), m_squadID);
    UpdateBoost(fleetID, true, wing, squad);

    ++m_squadID;
}

void FleetService::CreateFleetAdvert(uint32 fleetID, FleetAdvert data)
{
    /** @todo   update inviteScope to use names */
    std::map<uint32, FleetAdvert>::iterator itr = m_fleetAdvertMap.find(fleetID);
    if (itr != m_fleetAdvertMap.end()) {
        m_fleetAdvertMap.erase(fleetID);
        _log(FLEET__INFO, "CreateFleetAdvert()  Updating Advert for FleetID: %u, Scope: %u", fleetID, data.inviteScope);
    } else {
        _log(FLEET__INFO, "CreateFleetAdvert()  Creating Advert for FleetID: %u, Scope: %u", fleetID, data.inviteScope);
    }

    m_fleetAdvertMap.emplace(fleetID, data);
    std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
    if (fItr != m_fleetDataMap.end())
        UpdateOptions(fleetID, fItr->second.isFreeMove, /*isRegistered*/true, fItr->second.isVoiceEnabled);

    // update chat channel(s) with new description?
}

bool FleetService::AddMember(Client* pClient, uint32 fleetID, int32 wingID, int32 squadID, int8 job, int8 role, int8 booster)
{
    Character* pChar = pClient->GetChar().get();
    if (pChar == nullptr)
        return false;

    _log(FLEET__TRACE, "AddMember - CharID: %u, FleetID: %u, WingID: %i, Squad: %i, Job: %s, Role: %s, Booster: %s", \
    pChar->itemID(), fleetID, wingID, squadID, GetJobName(job).c_str(), GetRoleName(role).c_str(), GetBoosterName(booster).c_str());

    // add new member to fleet data
    m_fleetMembers.emplace(fleetID, pClient);

    // set char fleet data and send to client
    CharFleetData fData = CharFleetData();
        fData.fleetID = fleetID;
        fData.wingID = wingID;
        fData.squadID = squadID;
        fData.job = job; //Fleet::Job::None;
        fData.role = role;
        fData.booster = booster;
        fData.joinTime = GetFileTimeNow();
    pChar->SetFleetData(fData);

    PyDict* dict = new PyDict();
        dict->SetItemString("targetTags", new PyDict());
    PyTuple* obj = new PyTuple(1);
        obj->SetItem(0, new PyObject("util.KeyVal", dict));
    pClient->SendNotification("OnFleetStateChange", "charid", obj, true);

    bool fleet(false);
    if (role == Fleet::Role::FleetLeader) {
        std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
        if (fItr == m_fleetDataMap.end()) {
            _log(FLEET__ERROR, "Fleet Data for fleetID: %u not foune.", fleetID);
            return false;
        }
        if (fItr->second.booster == nullptr) {
            fItr->second.booster = pClient;
            fleet = true;
        }
        if (fItr->second.leader == nullptr) {
            fItr->second.leader = pClient;
            fleet = true;
        } else {
            _log(FLEET__ERROR, "FC is already filled.");
            return false;
        }
    }

    PyTuple* count = new PyTuple(1);
        count->SetItem(0, new PyInt((255 - m_fleetMembers.count(fleetID))));  // this is slots left from 255 (256 - leader)
    pClient->SendNotification("OnFleetActive", "clientID", count, true);

    std::list<int32> wing, squad;
    wing.clear();
    squad.clear();

    if (IsWingID(wingID)) {
        std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(wingID);
        if (itr == m_wingDataMap.end()) {
            _log(FLEET__ERROR, "Wing Data for wingID: %u not foune.", wingID);
            return false;
        }
        if (role == Fleet::Role::WingLeader) {
            if (itr->second.booster == nullptr) {
                itr->second.booster = pClient;
                wing.emplace(wing.end(), wingID);
            }
            if (itr->second.leader == nullptr) {
                itr->second.leader = pClient;
            } else {
                _log(FLEET__ERROR, "WC is already filled.");
                return false;
            }
        }
        PyTuple* count = new PyTuple(2);
            count->SetItem(0, new PyInt(wingID));
            count->SetItem(1, new PyInt(IsWingActive(wingID) ? 0 : 1));
        pClient->SendNotification("OnWingActive", "clientID", count, true);

        if (!IsSquadID(squadID))
            wing.emplace(wing.end(), wingID);
    }

    if (IsSquadID(squadID)) {
        std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
        if (itr == m_squadDataMap.end()) {
            _log(FLEET__ERROR, "Squad Data for squadID: %u not foune.", squadID);
            return false;
        }
        if (role == Fleet::Role::SquadLeader) {
            if (itr->second.booster == nullptr) {
                itr->second.booster = pClient;
                squad.emplace(squad.end(), squadID);
            }
            if (itr->second.leader == nullptr) {
                itr->second.leader = pClient;
            } else {
                _log(FLEET__ERROR, "SC is already filled.");
                return false;
            }
        }
        itr->second.members.emplace(pChar->itemID(), pClient);
        PyTuple* count = new PyTuple(2);
            count->SetItem(0, new PyInt(squadID));
            count->SetItem(1, PyStatic.NewOne());
        pClient->SendNotification("OnSquadActive", "clientID", count, true);

        squad.emplace(squad.end(), squadID);
    }

    // update all members with new member data
    JoinFleetRSP join;
        join.charID = pClient->GetCharacterID();
        join.clientID = pClient->GetClientID();
        join.job = pChar->fleetJob();
        join.role = pChar->fleetRole();
        join.shipTypeID = pClient->GetShip()->typeID();
        join.skillFleetCommand = pChar->GetSkillLevel(EvESkill::FleetCommand);
        join.skillLeadership = pChar->GetSkillLevel(EvESkill::Leadership);
        join.skillWingCommand = pChar->GetSkillLevel(EvESkill::WingCommand);
        join.squadID = pChar->squadID();
        join.roleBooster = pChar->fleetBooster();
        join.solarSystemID = pClient->GetSystemID();
        join.wingID = pChar->wingID();
        join.timestamp = pChar->fleetJoinTime();
    PyTuple* res = new PyTuple(1);
        res->SetItem(0, join.Encode());
    SendFleetUpdate(fleetID, "OnFleetJoin", res);

    UpdateBoost(fleetID, fleet, wing, squad);
    return true;
}

bool FleetService::UpdateMember(uint32 charID, uint32 fleetID, int32 newWingID, int32 newSquadID, int8 newJob, int8 newRole, int8 newBooster)
{
    bool fleet(false);
    std::list<int32> wing, squad;
    wing.clear();
    squad.clear();
    int8 oldRole(0), oldJob(0), oldBooster(0);
    int32 oldWingID(0), oldSquadID(0);
    // verify member data
    Client* pClient = sEntityList.FindClientByCharID(charID);
    if (pClient == nullptr)
        return false;
    Character* pChar = pClient->GetChar().get();
    if (pChar == nullptr)
        return false;

    oldJob = pChar->fleetJob();
    oldRole = pChar->fleetRole();
    oldWingID = pChar->wingID();
    oldSquadID = pChar->squadID();
    oldBooster = pChar->fleetBooster();

    // update fleet data
    if (oldBooster != newBooster) {
        if (oldBooster == Fleet::Booster::Fleet) {
            std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
            if (fItr == m_fleetDataMap.end())
                return false;
            fItr->second.booster = nullptr;
            fleet = true;
        }

        if (newBooster == Fleet::Booster::Fleet) {
            std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
            if (fItr == m_fleetDataMap.end())
                return false;
            fItr->second.booster = pClient;
            fleet = true;
        }
    }

    if (oldRole != newRole) {
        if (oldRole == Fleet::Role::FleetLeader) {
            std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
            if (fItr == m_fleetDataMap.end())
                return false;
            fItr->second.leader = nullptr;
        }

        if (newRole == Fleet::Role::FleetLeader) {
            std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
            if (fItr == m_fleetDataMap.end())
                return false;
            fItr->second.leader = pClient;
            if (fItr->second.booster == nullptr) {
                fItr->second.booster = pClient;
                newBooster = Fleet::Booster::Fleet;
                fleet = true;
            }
        }
    }
    // update wing data
    if (IsWingID(oldWingID)) {
        std::map<uint32, WingData>::iterator wItr = m_wingDataMap.find(oldWingID);
        if (wItr == m_wingDataMap.end())
            return false;
        if ((oldRole != newRole) and (oldRole == Fleet::Role::WingLeader)) {
            wItr->second.leader = nullptr;
        }
        if ((oldBooster != newBooster) and (oldBooster == Fleet::Booster::Wing)) {
            wItr->second.booster = nullptr;
            wing.emplace(wing.end(), oldWingID);
        }
    }

    if (newWingID == 0) {
        newWingID = oldWingID;
    } else if (IsWingID(newWingID)) {
        std::map<uint32, WingData>::iterator wItr = m_wingDataMap.find(newWingID);
        if (wItr == m_wingDataMap.end())
            return false;
        if ((oldBooster != newBooster) and (newBooster == Fleet::Booster::Wing)) {
            wItr->second.booster = pClient;
            wing.emplace(wing.end(), newWingID);
        }
        if ((oldRole != newRole) and (newRole == Fleet::Role::WingLeader)) {
            wItr->second.leader = pClient;
            if (wItr->second.booster == nullptr) {
                wItr->second.booster = pClient;
                newBooster = Fleet::Booster::Wing;
                wing.emplace(wing.end(), newWingID);
            }
        }
    }

    // update squad data
    if (IsSquadID(oldSquadID)) {
        std::map<uint32, SquadData>::iterator sItr = m_squadDataMap.find(oldSquadID);
        if (sItr == m_squadDataMap.end())
            return false;
        if ((oldRole != newRole) and (oldRole == Fleet::Role::SquadLeader))
            sItr->second.leader = nullptr;
        if ((oldBooster != newBooster) and (oldBooster == Fleet::Booster::Squad)) {
            sItr->second.booster = nullptr;
            squad.emplace(squad.end(), oldSquadID);
        }
        if (oldSquadID != newSquadID) {
            sItr->second.members.erase(charID);
            // need a fast way to iterate thru wing data for active status...
            SendActiveStatus(fleetID, oldWingID, oldSquadID);
        }
    }

    if (newSquadID == 0) {
        newSquadID = oldSquadID;
    } else if (IsSquadID(newSquadID)) {
        std::map<uint32, SquadData>::iterator sItr = m_squadDataMap.find(newSquadID);
        if (sItr == m_squadDataMap.end())
            return false;
        if ((oldBooster != newBooster) and (newBooster == Fleet::Booster::Squad)) {
            sItr->second.booster = pClient;
            squad.emplace(squad.end(), newSquadID);
        }
        if ((oldRole != newRole) and (newRole == Fleet::Role::SquadLeader)) {
            sItr->second.leader = pClient;
            if (sItr->second.booster == nullptr) {
                sItr->second.booster = pClient;
                newBooster = Fleet::Booster::Squad;
                squad.emplace(squad.end(), newSquadID);
            }
        }
        if (newSquadID != oldSquadID) {
            sItr->second.members.emplace(charID, pClient);
            SendActiveStatus(fleetID, newWingID, newSquadID);
        }
    }

    _log(FLEET__TRACE, "UpdateMember - CharID: %u, FleetID: %u, oldWingID: %i, newWingID: %i, oldSquad: %i, newSquadID: %i, oldJob: %s, newJob: %s, oldRole: %s, newRole: %s, oldBooster: %s, newBooster: %s", \
            charID, fleetID, oldWingID, newWingID, oldSquadID, newSquadID, \
            GetJobName(oldJob).c_str(), GetJobName(newJob).c_str(), GetRoleName(oldRole).c_str(), GetRoleName(newRole).c_str(), \
            GetBoosterName(oldBooster).c_str(), GetBoosterName(newBooster).c_str());

    // update char data
    CharFleetData fData = CharFleetData();
        fData.fleetID = fleetID;
        fData.wingID = newWingID;
        fData.squadID = newSquadID;
        fData.job = newJob;
        fData.role = newRole;
        fData.booster = newBooster;
        fData.joinTime = pChar->fleetJoinTime();
    pChar->SetFleetData(fData);

    // update fleet members with new data
    MemberChangedRSP res;
        res.charID = charID;
        res.fleetID = fleetID;
        res.oldWingID = oldWingID;
        res.oldSquadID = oldSquadID;
        res.oldRole = oldRole;
        res.oldJob = oldJob;
        res.oldBooster = oldBooster;
        res.newWingID = newWingID;
        res.newSquadID = newSquadID;
        res.newRole = newRole;
        res.newJob = newJob;
        res.newBooster = newBooster;
        res.isOnlyMember = (m_fleetMembers.count(fleetID) > 1 ? false : true);
    SendFleetUpdate(fleetID, "OnFleetMemberChanged", res.Encode());

    UpdateBoost(fleetID, fleet, wing, squad);

    // send "OnFleetMove" (empty tuple) if/when voice is enabled for this fleet

    return true;
}

void FleetService::UpdateBoost(uint32 fleetID, bool fleet, std::list<int32>& wing, std::list<int32>& squad)
{
    double start = GetTimeUSeconds();
    /*  this needs to check all boosters to update squad boost levels
     *   we will also check current levels to see if members need to be updated
     */
    std::vector< uint32 > wingIDs, squadIDs;
    std::map<ShipSE*, BoostData> memberUpdateMap;

    bool fBoost(false);
    int8 armored(0), info(0), leader(0), mining(0), siege(0), skirmish(0);

    BoostData fData = BoostData();
    BoostData bData = BoostData();

    std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
    if (fItr != m_fleetDataMap.end()) {
        // set base boost data from FB
        if ((fItr->second.leader != nullptr) and (fItr->second.leader->IsInSpace()))
            if (m_fleetWings.count(fleetID) <= fItr->second.leader->GetChar()->GetSkillLevel(EvESkill::FleetCommand)) {
                if ((fItr->second.booster != nullptr) and (fItr->second.booster->IsInSpace()))
                    if (fItr->second.leader->GetSystemID() == fItr->second.booster->GetSystemID()) {
                        Character* pChar = fItr->second.booster->GetChar().get();
                        if (pChar != nullptr) {
                            if (pChar->HasSkillTrainedToLevel(EvESkill::ArmoredWarfare, 1))
                                fData.armored   = pChar->GetSkillLevel(EvESkill::ArmoredWarfare);
                            if (pChar->HasSkillTrainedToLevel(EvESkill::InformationWarfare, 1))
                                fData.info      = pChar->GetSkillLevel(EvESkill::InformationWarfare);
                            if (pChar->HasSkillTrainedToLevel(EvESkill::SiegeWarfare, 1))
                                fData.siege     = pChar->GetSkillLevel(EvESkill::SiegeWarfare);
                            if (pChar->HasSkillTrainedToLevel(EvESkill::SkirmishWarfare, 1))
                                fData.skirmish  = pChar->GetSkillLevel(EvESkill::SkirmishWarfare);
                            if (pChar->HasSkillTrainedToLevel(EvESkill::MiningForeman, 1))
                                fData.mining    = pChar->GetSkillLevel(EvESkill::MiningForeman);
                            if (fData.armored or fData.info or fData.leader or fData.mining or fData.siege or fData.skirmish)
                                fBoost = true;
                        }
                    }
                // this is for FC only.  will always get own skill, and here they get their fleet boost, also
                fData.leader = fItr->second.leader->GetChar()->GetSkillLevel(EvESkill::Leadership);
                if (fItr->second.leader->IsInSpace())
                    fItr->second.leader->GetShipSE()->ApplyBoost(fData);
            }
    }

    _log( FLEET__TRACE, "UpdateBoost - FB: %s, leader: %i, armored: %i, info: %i, siege: %i, skirmish: %i, mining: %i", \
            (fBoost ? "true" : "false"), fData.leader, fData.armored, fData.info, fData.siege, fData.skirmish, fData.mining);

    // check squad boost updates....sb overwriting higher boosts
    if (fleet) {
        // update all fleet members due to fleet booster update
        wingIDs.clear();
        GetWingIDs(fleetID, wingIDs);
        for (auto wingID : wingIDs) {
            if (!IsWingID(wingID))        // if WingID is invalid, remove it from map!!
                continue;
            bData = BoostData();
            if (fBoost) {
                bData.armored   = fData.armored;
                bData.info      = fData.info;
                bData.mining    = fData.mining;
                bData.siege     = fData.siege;
                bData.skirmish  = fData.skirmish;
            }
            SetWingBoostData(wingID, bData);

            squadIDs.clear();
            GetSquadIDs(wingID, squadIDs);
            for (auto squadID : squadIDs) {
                if (!IsSquadID(squadID))
                    continue;
                bool sboost(false);
                SetSquadBoostData(squadID, bData, sboost);
                SquadData sData = SquadData();
                GetSquadData(squadID, sData);
                for (auto cur : sData.members)  // SC is a member
                    if ((sboost) and (cur.second->GetSystemID() == sData.booster->GetSystemID()))
                        memberUpdateMap.emplace(cur.second->GetShipSE(), sData.boost);
            }
        }
    } else if (!wing.empty()) {
        wing.sort();
        wing.unique();
        for (auto wingID : wing) {
            if (!IsWingID(wingID))
                continue;
            bData = BoostData();
            if (fBoost) {
                bData.armored   = fData.armored;
                bData.info      = fData.info;
                bData.mining    = fData.mining;
                bData.siege     = fData.siege;
                bData.skirmish  = fData.skirmish;
            }
            SetWingBoostData(wingID, bData);

            squadIDs.clear();
            GetSquadIDs(wingID, squadIDs);
            for (auto squadID : squadIDs) {
                if (!IsSquadID(squadID))
                    continue;
                bool sboost(false);
                SetSquadBoostData(squadID, bData, sboost);
                SquadData sData = SquadData();
                GetSquadData(squadID, sData);
                for (auto cur : sData.members)
                    if ((sboost) and (cur.second->GetSystemID() == sData.booster->GetSystemID()))
                        memberUpdateMap.emplace(cur.second->GetShipSE(), sData.boost);
            }
        }
    } else if (!squad.empty()) {
        squad.sort();
        squad.unique();
        for (auto squadID : squad) {
            if (!IsSquadID(squadID))       // if squadID is invalid, remove it from map!!
                continue;
            bData = BoostData();
            if (fBoost) {
                bData.armored   = fData.armored;
                bData.info      = fData.info;
                bData.mining    = fData.mining;
                bData.siege     = fData.siege;
                bData.skirmish  = fData.skirmish;
            }
            SquadData sData = SquadData();
            GetSquadData(squadID, sData);
            SetWingBoostData(sData.wingID, bData);
            bool sboost(false);
            SetSquadBoostData(squadID, bData, sboost);
            for (auto cur : sData.members)
                if (sboost and (cur.second->GetSystemID() == sData.booster->GetSystemID()))
                    memberUpdateMap.emplace(cur.second->GetShipSE(), sData.boost);
        }
    }

    // update boost effects on these members' ships using updated boost levels
    // this is for fleet boost only, as modules will apply/remove their effects using the FxSystem
    for (auto cur : memberUpdateMap)
        cur.first->ApplyBoost(cur.second);

    _log( FLEET__TRACE, "FleetService::UpdateBoost() - Updated %u members of fleetID: %u in %.2fus.  fleet: %s, wing: %s, squad: %s", \
            memberUpdateMap.size(), fleetID, GetTimeUSeconds() - start, (fleet ? "true" : "false"), (wing.empty() ? "false" : "true"), (squad.empty() ? "false" : "true"));
}

void FleetService::SetWingBoostData(uint32 wingID, BoostData& bData)
{
    bool boost(false);
    int8 leader(0), armored(0), info(0), mining(0), siege(0), skirmish(0);
    std::map<uint32, WingData>::iterator wItr = m_wingDataMap.find(wingID);
    if (wItr == m_wingDataMap.end())
        return;
    if ((wItr->second.leader != nullptr) and (wItr->second.leader->IsInSpace()))
        if (m_wingSquads.count(wingID) <= wItr->second.leader->GetChar()->GetSkillLevel(EvESkill::WingCommand)) {
            if ((wItr->second.booster != nullptr) and (wItr->second.booster->IsInSpace()))
                if (wItr->second.leader->GetSystemID() == wItr->second.booster->GetSystemID()) {
                    Character* pChar = wItr->second.booster->GetChar().get();
                    if (pChar != nullptr) {
                        leader = wItr->second.leader->GetChar()->GetSkillLevel(EvESkill::Leadership);    // this applies ONLY to self
                        if (pChar->HasSkillTrainedToLevel(EvESkill::ArmoredWarfare, 1))
                            armored     = pChar->GetSkillLevel(EvESkill::ArmoredWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::InformationWarfare, 1))
                            info        = pChar->GetSkillLevel(EvESkill::InformationWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::SiegeWarfare, 1))
                            siege       = pChar->GetSkillLevel(EvESkill::SiegeWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::SkirmishWarfare, 1))
                            skirmish    = pChar->GetSkillLevel(EvESkill::SkirmishWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::MiningForeman, 1))
                            mining      = pChar->GetSkillLevel(EvESkill::MiningForeman);
                    }
                    boost = true;
                    wItr->second.boost.armored  = ((armored < bData.armored)   ? bData.armored   : armored);
                    wItr->second.boost.info     = ((info < bData.info)         ? bData.info      : info);
                    wItr->second.boost.mining   = ((mining < bData.mining)     ? bData.mining    : mining);
                    wItr->second.boost.siege    = ((siege < bData.siege)       ? bData.siege     : siege);
                    wItr->second.boost.skirmish = ((skirmish < bData.skirmish) ? bData.skirmish  : skirmish);
                } else {
                    wItr->second.boost = BoostData();
                }
            // this is for WC only.  will always get own skill, and here they get their wing boost, also
            wItr->second.boost.leader   = leader;
            wItr->second.leader->GetShipSE()->ApplyBoost(wItr->second.boost);
        }

    if (!boost)
        wItr->second.boost = BoostData();

    _log( FLEET__TRACE, "BoostData - WB: %s, wingID: %u - leader: %i, armored: %i, info: %i, siege: %i, skirmish: %i, mining: %i", \
            (boost ? "true" : "false"), wingID, wItr->second.boost.leader, wItr->second.boost.armored, wItr->second.boost.info, \
            wItr->second.boost.siege, wItr->second.boost.skirmish, wItr->second.boost.mining);
}

void FleetService::SetSquadBoostData(uint32 squadID, BoostData bData, bool& sboost)
{
    int8 leader(0), armored(0), info(0), mining(0), siege(0), skirmish(0);
    std::map<uint32, SquadData>::iterator sItr = m_squadDataMap.find(squadID);
    if (sItr == m_squadDataMap.end())
        return;
    if ((sItr->second.leader != nullptr) and (sItr->second.leader->IsInSpace()))
        if (sItr->second.members.size() <= (sItr->second.leader->GetChar()->GetSkillLevel(EvESkill::Leadership) * 2)) {
            if ((sItr->second.booster != nullptr) and (sItr->second.booster->IsInSpace()))
                if (sItr->second.leader->GetSystemID() == sItr->second.booster->GetSystemID()) {
                    Character* pChar = sItr->second.booster->GetChar().get();
                    if (pChar != nullptr) {
                        leader = sItr->second.leader->GetChar()->GetSkillLevel(EvESkill::Leadership);    // this applies ONLY to self
                        if (pChar->HasSkillTrainedToLevel(EvESkill::ArmoredWarfare, 1))
                            armored     = pChar->GetSkillLevel(EvESkill::ArmoredWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::InformationWarfare, 1))
                            info        = pChar->GetSkillLevel(EvESkill::InformationWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::SiegeWarfare, 1))
                            siege       = pChar->GetSkillLevel(EvESkill::SiegeWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::SkirmishWarfare, 1))
                            skirmish    = pChar->GetSkillLevel(EvESkill::SkirmishWarfare);
                        if (pChar->HasSkillTrainedToLevel(EvESkill::MiningForeman, 1))
                            mining      = pChar->GetSkillLevel(EvESkill::MiningForeman);
                    }
                    sItr->second.boost.armored  = ((armored < bData.armored)   ? bData.armored   : armored);
                    sItr->second.boost.info     = ((info < bData.info)         ? bData.info      : info);
                    sItr->second.boost.mining   = ((mining < bData.mining)     ? bData.mining    : mining);
                    sItr->second.boost.siege    = ((siege < bData.siege)       ? bData.siege     : siege);
                    sItr->second.boost.skirmish = ((skirmish < bData.skirmish) ? bData.skirmish  : skirmish);
                } else {
                    sItr->second.boost = BoostData();
                }
            // squad will always get this if SC is skilled
            sboost = true;
            sItr->second.boost.leader = leader;
        }

    if (!sboost)
        sItr->second.boost = BoostData();

    _log( FLEET__TRACE, "BoostData - SB: %s, squadID: %u - leader: %i, armored: %i, info: %i, siege: %i, skirmish: %i, mining: %i", \
            (sboost ? "true" : "false"), squadID, sItr->second.boost.leader, sItr->second.boost.armored, sItr->second.boost.info, \
            sItr->second.boost.siege, sItr->second.boost.skirmish, sItr->second.boost.mining);
}

void FleetService::UpdateOptions(uint32 fleetID, bool isFreeMove, bool isRegistered, bool isVoiceEnabled)
{
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr == m_fleetDataMap.end())
        return;
    // if nothing changed, do nothing
    if ((itr->second.isFreeMove == isFreeMove) and (itr->second.isRegistered == isRegistered) and (itr->second.isVoiceEnabled == isVoiceEnabled))
        return;

    PyDict* was = new PyDict();
        was->SetItemString("isFreeMove",        new PyBool(itr->second.isFreeMove));
        was->SetItemString("isRegistered",      new PyBool(itr->second.isRegistered));
        was->SetItemString("isVoiceEnabled",    new PyBool(itr->second.isVoiceEnabled));
    PyDict* is = new PyDict();
        is->SetItemString("isFreeMove",         new PyBool(isFreeMove));
        is->SetItemString("isRegistered",       new PyBool(isRegistered));
        is->SetItemString("isVoiceEnabled",     new PyBool(isVoiceEnabled));
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyObject("util.KeyVal", was));
        tuple->SetItem(1, new PyObject("util.KeyVal", is));
    SendFleetUpdate(fleetID, "OnFleetOptionsChanged", tuple);

    _log(FLEET__TRACE, "FleetService::UpdateOptions() - fleetID: %u FreeMove: %s, Registered: %s, Voice: %s", \
            fleetID, isFreeMove ? "true" : "false", isRegistered ? "true" : "false", isVoiceEnabled ? "true" : "false");
    itr->second.isFreeMove = isFreeMove;
    itr->second.isRegistered = isRegistered;
    itr->second.isVoiceEnabled = isVoiceEnabled;
}

PyRep* FleetService::GetMOTD(uint32 fleetID)
{
    PyTuple* tuple = new PyTuple(1);
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr != m_fleetDataMap.end()) {
        tuple->SetItem(0, new PyString(itr->second.motd));
    } else {
        tuple->SetItem(0, PyStatic.NewNone());
    }
    return tuple;
}

void FleetService::SetMOTD(uint32 fleetID, std::string motd)
{
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr != m_fleetDataMap.end())
        itr->second.motd = motd;

    PyTuple* tuple = new PyTuple(1);
        tuple->SetItem(0, new PyString(motd));
    SendFleetUpdate(fleetID, "OnFleetMotdChanged", tuple);

    /** @todo  update motd in fleet chat window (which is not coded yet in LSC system) */
}

void FleetService::RenameWing(uint32 wingID, std::string name)
{
    std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(wingID);
    if (itr == m_wingDataMap.end())
        return;

    itr->second.name = name;

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(wingID));
        tuple->SetItem(1, new PyString(name));
    SendFleetUpdate(itr->second.fleetID, "OnFleetWingNameChanged", tuple);

    _log(FLEET__TRACE, "FleetService::RenameWing() %u to %s", wingID, name.c_str());
}

void FleetService::RenameSquad(uint32 squadID, std::string name)
{
    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
    if (itr == m_squadDataMap.end())
        return;

    itr->second.name = name;

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(squadID));
        tuple->SetItem(1, new PyString(name));
    SendFleetUpdate(itr->second.fleetID, "OnFleetSquadNameChanged", tuple);

    _log(FLEET__TRACE, "FleetService::RenameSquad() %u to %s", squadID, name.c_str());
}

bool FleetService::IsWingActive(int32 wingID)
{
    auto range = m_wingSquads.equal_range(wingID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        std::map<uint32, SquadData>::iterator sItr = m_squadDataMap.find(itr->second);
        if (sItr != m_squadDataMap.end())
            if (sItr->second.members.size() > 0)
                return true;
    }
    return false;
}

void FleetService::GetSquadIDs(uint32 wingID, std::vector< uint32 >& squadIDs)
{
    auto range = m_wingSquads.equal_range(wingID);
    for (auto itr = range.first; itr != range.second; ++itr)
        squadIDs.push_back(itr->second);
}

void FleetService::GetWingIDs(uint32 fleetID, std::vector< uint32 >& wingIDs)
{
    auto range = m_fleetWings.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        wingIDs.push_back(itr->second);
}

void FleetService::GetFleetData(uint32 fleetID, FleetData& data)
{
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr != m_fleetDataMap.end())
        data = itr->second;
}

void FleetService::GetWingData(uint32 wingID, WingData& data)
{
    std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(wingID);
    if (itr != m_wingDataMap.end())
        data = itr->second;
}

void FleetService::GetSquadData(uint32 squadID, SquadData& data)
{
    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
    if (itr != m_squadDataMap.end())
        data = itr->second;
}

void FleetService::GetMemeberVec(uint32 fleetID, std::vector< Client* >& data)
{
    auto range = m_fleetMembers.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        data.push_back(itr->second);
}

PyRep* FleetService::GetFleetAdvert(uint32 fleetID)
{
    Client* pClient(nullptr);
    std::map<uint32, FleetAdvert>::iterator itr = m_fleetAdvertMap.find(fleetID);
    if (itr == m_fleetAdvertMap.end())
        return nullptr;

    pClient = itr->second.leader;
    if (pClient == nullptr)
        return nullptr;
    AvalibleFleetsRSP fleetRSP;
        fleetRSP.fleetID = itr->first;
        fleetRSP.local_minSecurity = itr->second.local_minSecurity;
        fleetRSP.description = itr->second.description;
        fleetRSP.public_minStanding = itr->second.public_minStanding;
        fleetRSP.fleetName = itr->second.fleetName;
        fleetRSP.advertTime = itr->second.advertTime;
        fleetRSP.dateCreated = itr->second.dateCreated;
        fleetRSP.joinNeedsApproval = itr->second.joinNeedsApproval;

    PyTuple* localTuple = new PyTuple(1);
    PyList* localList = new PyList();
    for (auto cur1 : itr->second.local_allowedEntities) // corpID, allianceID, militiaID - if applicable
        localList->AddItemInt(cur1);
    localTuple->SetItem(0, localList);
    PyToken* token = new PyToken("__builtin__.set");
    PyTuple* localTuple2 = new PyTuple(2);
        localTuple2->SetItem(0, token);
        localTuple2->SetItem(1, localTuple);
        fleetRSP.local_allowedEntities = new PyObjectEx(false, localTuple2);

    PyTuple* publicTuple = new PyTuple(1);
    PyList* publicList = new PyList();
    for (auto cur2 : itr->second.public_allowedEntities)    // searches creator's addy book and addes charIDs based on standings
        publicList->AddItemInt(cur2);
        publicTuple->SetItem(0, publicList);
    PyTuple* publicTuple2 = new PyTuple(2);
        PyIncRef(token);
        publicTuple2->SetItem(0, token);
        publicTuple2->SetItem(1, publicTuple);
        fleetRSP.public_allowedEntities = new PyObjectEx(false, publicTuple2);

        fleetRSP.local_minStanding = itr->second.local_minStanding;
        fleetRSP.numMembers = m_fleetMembers.count(itr->first);
        fleetRSP.hideInfo = itr->second.hideInfo;
        fleetRSP.public_minSecurity = itr->second.public_minSecurity;
        fleetRSP.inviteScope = itr->second.inviteScope;
        fleetRSP.solarSystemID = itr->second.solarSystemID;
        fleetRSP.charID = pClient->GetCharacterID();
        fleetRSP.corpID = pClient->GetCorporationID();
        fleetRSP.warFactionID = pClient->GetWarFactionID();
        fleetRSP.securityStatus = pClient->GetSecurityRating();
        fleetRSP.allianceID = pClient->GetAllianceID();

    fleetRSP.Dump(FLEET__DEBUG);
    return fleetRSP.Encode();
}

void FleetService::RemoveFleetAdvert(uint32 fleetID)
{
    _log(FLEET__TRACE, "RemoveFleetAdvert for FleetID: %u", fleetID);
    m_fleetAdvertMap.erase(fleetID);
    // set options to remove registration
    std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
    if (fItr != m_fleetDataMap.end())
        UpdateOptions(fleetID, fItr->second.isFreeMove, /*isRegistered*/false, fItr->second.isVoiceEnabled);
}

void FleetService::DeleteFleet(uint32 fleetID)
{
    std::vector<uint32> wings, squads;
    GetWingIDs(fleetID, wings);
    for (auto wing : wings) {
        m_wingDataMap.erase(wing);
        squads.clear();
        GetSquadIDs(wing, squads);
        for (auto squad : squads)
            m_squadDataMap.erase(squad);
    }
    m_fleetDataMap.erase(fleetID);
    RemoveFleetAdvert(fleetID);
}

void FleetService::DeleteWing(uint32 wingID)
{
    std::map<uint32, WingData>::iterator wItr = m_wingDataMap.find(wingID);
    if (wItr == m_wingDataMap.end())
        return;

    DecFleetSquads(wItr->second.fleetID, wingID);

    std::vector<uint32> squads;
    GetSquadIDs(wingID, squads);
    std::map<uint32, SquadData>::iterator sItr;
    for (auto cur : squads) {
        sItr = m_squadDataMap.find(cur);
        if (sItr == m_squadDataMap.end())
            continue;
        m_squadDataMap.erase(sItr);
    }

    PyTuple* tuple = new PyTuple(1);
        tuple->SetItem(0, new PyInt(wingID));
    SendFleetUpdate(wItr->second.fleetID, "OnFleetWingDeleted", tuple);

    m_wingDataMap.erase(wItr);
}

void FleetService::DeleteSquad(uint32 squadID)
{
    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
    if (itr == m_squadDataMap.end())
        return;

    DecFleetSquads(itr->second.fleetID, itr->second.wingID);

    PyTuple* tuple = new PyTuple(1);
        tuple->SetItem(0, new PyInt(squadID));
    SendFleetUpdate(itr->second.fleetID, "OnFleetSquadDeleted", tuple);

    m_squadDataMap.erase(itr);
}

void FleetService::IncFleetSquads(uint32 fleetID, uint32 wingID)
{
    std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
    if (fItr == m_fleetDataMap.end())
        return;
    ++(fItr->second.squads);
}

void FleetService::DecFleetSquads(uint32 fleetID, uint32 wingID)
{
    std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
    if (fItr == m_fleetDataMap.end())
        return;
    --(fItr->second.squads);
}

Client* FleetService::GetFleetLeader(uint32 fleetID)
{
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr == m_fleetDataMap.end())
        return nullptr;
    return itr->second.leader;
}

uint32 FleetService::GetFleetLeaderID(uint32 fleetID)
{
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr == m_fleetDataMap.end())
        return 0;
    return itr->second.leader->GetCharacterID();
}

Client* FleetService::GetWingLeader(uint32 wingID)
{
    std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(wingID);
    if (itr == m_wingDataMap.end())
        return nullptr;
    return itr->second.leader;
}

uint32 FleetService::GetWingLeaderID(uint32 wingID)
{
    std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(wingID);
    if (itr == m_wingDataMap.end())
        return 0;
    return itr->second.leader->GetCharacterID();
}

Client* FleetService::GetSquadLeader(uint32 squadID)
{
    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
    if (itr == m_squadDataMap.end())
        return nullptr;
    return itr->second.leader;
}

uint32 FleetService::GetSquadLeaderID(uint32 squadID)
{
    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
    if (itr == m_squadDataMap.end())
        return 0;
    return itr->second.leader->GetCharacterID();
}

std::string FleetService::GetFleetName(uint32 fleetID)
{
    std::map<uint32, FleetData>::iterator itr = m_fleetDataMap.find(fleetID);
    if (itr == m_fleetDataMap.end())
        return "Fleet";
    return itr->second.name;
}

std::string FleetService::GetWingName(uint32 wingID)
{
    std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(wingID);
    if (itr == m_wingDataMap.end())
        return "Wing";
    return itr->second.name;
}

std::string FleetService::GetSquadName(uint32 squadID)
{
    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
    if (itr == m_squadDataMap.end())
        return "Squad";
    return itr->second.name;
}

std::string FleetService::GetFleetDescription(uint32 fleetID)
{
    std::map<uint32, FleetAdvert>::iterator itr = m_fleetAdvertMap.find(fleetID);
    if (itr == m_fleetAdvertMap.end())
        return "No Info";
    return itr->second.description;
}

void FleetService::GetRandUnitIDs(uint32 fleetID, int32& wingID, int32& squadID)
{
    std::vector<uint32> wings, squads;
    auto range = m_fleetWings.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        wings.push_back(itr->second);

    wingID = wings.at(MakeRandomInt(0, wings.size()));

    range = m_wingSquads.equal_range(wingID);
    for (auto itr = range.first; itr != range.second; ++itr)
        squads.push_back(itr->second);

    // make sure there is room in this squad for another member
    squadID = squads.at(MakeRandomInt(0, squads.size()));
}

void FleetService::LeaveFleet(Client* pClient)
{
    Character* pChar = pClient->GetChar().get();
    if (pChar == nullptr)
        return;

    PyTuple* tuple = new PyTuple(1);
    tuple->SetItem(0, new PyInt(pChar->itemID()));
    SendFleetUpdate(pChar->fleetID(), "OnFleetLeave", tuple);

    RemoveMember(pClient);

    CharFleetData fleet = CharFleetData();
    //call updates on fleet session data
    pChar->SetFleetData(fleet);
}

void FleetService::RemoveMember(Client* pClient)
{
    Character* pChar = pClient->GetChar().get();
    if (pChar == nullptr)
        return;

    uint32 fleetID = pChar->fleetID();

    // update fleet data
    if (fleetID) {
        std::map<uint32, FleetData>::iterator fItr = m_fleetDataMap.find(fleetID);
        if (fItr != m_fleetDataMap.end()) {
            if (pChar->fleetBooster() == Fleet::Booster::Fleet)
                fItr->second.booster = nullptr;
            if (pChar->fleetBooster() == Fleet::Role::FleetLeader)
                fItr->second.leader = nullptr;
        }
    }

    // update wing data
    if (pChar->wingID()) {
        std::map<uint32, WingData>::iterator itr = m_wingDataMap.find(pChar->wingID());
        if (itr != m_wingDataMap.end()) {
            if (pChar->fleetBooster() == Fleet::Booster::Wing)
                itr->second.booster = nullptr;
            if (pChar->fleetBooster() == Fleet::Role::WingLeader)
                itr->second.leader = nullptr;
        }
    }

    // update squad data
    if (pChar->squadID()) {
        std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(pChar->squadID());
        if (itr != m_squadDataMap.end()) {
            itr->second.members.erase(pChar->itemID());
            if (pChar->fleetBooster() == Fleet::Booster::Squad)
                itr->second.booster = nullptr;
            if (pChar->fleetBooster() == Fleet::Role::SquadLeader)
                itr->second.leader = nullptr;
        }
    }

    auto range = m_fleetMembers.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        if (itr->second == pClient) {
            m_fleetMembers.erase(itr);
            break;
        }
}

PyRep* FleetService::GetWings(uint32 fleetID)
{
    std::vector< uint32 > wingIDs, squadIDs;
    wingIDs.clear();
    GetWingIDs(fleetID, wingIDs);
    PyDict* dict = new PyDict();
    for (auto wingID : wingIDs) {
        squadIDs.clear();
        WingData wData = WingData();
        GetWingData(wingID, wData);
        WingRSP wing;
            wing.name = wData.name;
            wing.wingID = wingID;
        GetSquadIDs(wingID, squadIDs);
        PyDict* dict2 = new PyDict();
        for (auto squadID : squadIDs) {
            SquadData sData = SquadData();
            GetSquadData(squadID, sData);
            SquadRSP squad;
                squad.name = sData.name;
                squad.squadID = squadID;
            dict2->SetItem(new PyInt(squadID), squad.Encode());
        }
        wing.squads = dict2;
        dict->SetItem(new PyInt(wingID), wing.Encode());
    }

    dict->Dump(FLEET__DEBUG, "    ");
    return dict;
}

PyRep* FleetService::GetAvailableFleets() {
    Client* pClient(nullptr);

    PyDict* fleetDict = new PyDict();
    for (auto cur : m_fleetAdvertMap) {
        pClient = cur.second.leader;
        if (pClient == nullptr)
            continue;
        AvalibleFleetsRSP fleetRSP;
            fleetRSP.fleetID = cur.first;
            fleetRSP.local_minSecurity = cur.second.local_minSecurity;
            fleetRSP.description = cur.second.description;
            fleetRSP.public_minStanding = cur.second.public_minStanding;
            fleetRSP.fleetName = cur.second.fleetName;
            fleetRSP.advertTime = cur.second.advertTime;
            fleetRSP.dateCreated = cur.second.dateCreated;
            fleetRSP.joinNeedsApproval = cur.second.joinNeedsApproval;

        PyTuple* localTuple = new PyTuple(1);
        PyList* localList = new PyList();
        for (auto cur1 : cur.second.local_allowedEntities)
            localList->AddItemInt(cur1);
        localTuple->SetItem(0, localList);
        PyToken* token = new PyToken("__builtin__.set");
        PyTuple* localTuple2 = new PyTuple(2);
        localTuple2->SetItem(0, token);
        localTuple2->SetItem(1, localTuple);
            fleetRSP.local_allowedEntities = new PyObjectEx(false, localTuple2);

        PyTuple* publicTuple = new PyTuple(1);
        PyList* publicList = new PyList();
        for (auto cur2 : cur.second.public_allowedEntities)
            publicList->AddItemInt(cur2);
        publicTuple->SetItem(0, publicList);
        PyTuple* publicTuple2 = new PyTuple(2);
        PyIncRef(token);
        publicTuple2->SetItem(0, token);
        publicTuple2->SetItem(1, publicTuple);
            fleetRSP.public_allowedEntities = new PyObjectEx(false, publicTuple2);

            fleetRSP.local_minStanding = cur.second.local_minStanding;
            fleetRSP.numMembers = m_fleetMembers.count(cur.first);
            fleetRSP.hideInfo = cur.second.hideInfo;
            fleetRSP.public_minSecurity = cur.second.public_minSecurity;
            fleetRSP.inviteScope = cur.second.inviteScope;
            fleetRSP.solarSystemID = cur.second.solarSystemID;
            fleetRSP.charID = pClient->GetCharacterID();
            fleetRSP.corpID = pClient->GetCorporationID();
            fleetRSP.warFactionID = pClient->GetWarFactionID();
            fleetRSP.securityStatus = pClient->GetSecurityRating();
            fleetRSP.allianceID = pClient->GetAllianceID();
        fleetDict->SetItem(new PyLong(cur.first), fleetRSP.Encode() );
    }

    fleetDict->Dump(FLEET__DEBUG, "    ");
    return fleetDict;
}

void FleetService::FleetBroadcast(Client* pFrom, uint32 itemID, int8 scope, int8 group, std::string msg)
{
    uint32 fleetID = pFrom->GetChar()->fleetID();
    if (!IsFleetID(fleetID)) {
        _log(FLEET__WARNING, "%s called FleetBroadcast with invalid fleetID %u.", pFrom->GetName(), fleetID);
        return;
    }
    if (group == Fleet::BCast::Group::None) {
        _log(FLEET__WARNING, "%s called FleetBroadcast with group = None for fleet %u.", pFrom->GetName(), fleetID);
        return;
    }

    int32 wingID(pFrom->GetChar()->wingID());
    if ((wingID > 0) and !IsWingID(wingID)) {
        _log(FLEET__WARNING, "%s called FleetBroadcast with invalid wing %i for fleet %u.", pFrom->GetName(), wingID, fleetID);
        return;
    }
    int32 squadID(pFrom->GetChar()->squadID());
    if ((squadID > 0) and !IsSquadID(squadID)) {
        _log(FLEET__WARNING, "%s called FleetBroadcast with invalid squad %i for fleet %u.", pFrom->GetName(), squadID, fleetID);
        return;
    }

    uint16 scopeID(0);
    std::vector<Client*> members;
    switch (scope) {
        case Fleet::BCast::Scope::System: {
            scopeID = pFrom->GetSystemID();
        } break;
        case Fleet::BCast::Scope::Bubble: {
            scopeID = pFrom->GetShipSE()->SysBubble()->GetID();
        } break;
    }
    switch (group) {
        case Fleet::BCast::Group::All: {
            auto range = m_fleetMembers.equal_range(fleetID);
            for (auto fItr = range.first; fItr != range.second; ++fItr) {
                if (scope == Fleet::BCast::Scope::Universe) {
                    members.push_back(fItr->second);
                } else if (scope == Fleet::BCast::Scope::System) {
                    if (fItr->second->GetSystemID() == scopeID)
                        members.push_back(fItr->second);
                } else if (scope == Fleet::BCast::Scope::Bubble) {
                    if (fItr->second->GetShipSE()->SysBubble()->GetID() == scopeID)
                        members.push_back(fItr->second);
                }
            }
        } break;
        // these 2 need to check fleet hierarchy for proper member list
        case Fleet::BCast::Group::Down: {
            if (wingID == -1) {
                auto range = m_fleetMembers.equal_range(fleetID);
                for (auto fItr = range.first; fItr != range.second; ++fItr) {
                    if (scope == Fleet::BCast::Scope::Universe) {
                        members.push_back(fItr->second);
                    } else if (scope == Fleet::BCast::Scope::System) {
                        if (fItr->second->GetSystemID() == scopeID)
                            members.push_back(fItr->second);
                    } else if (scope == Fleet::BCast::Scope::Bubble) {
                        if (fItr->second->GetShipSE()->SysBubble()->GetID() == scopeID)
                            members.push_back(fItr->second);
                    }
                }
            } else {
                if (squadID == -1) {
                    std::vector<uint32> squads;
                    GetSquadIDs(wingID, squads);
                    std::map<uint32, SquadData>::iterator itr;
                    for (auto cur : squads) {
                        itr = m_squadDataMap.find(cur);
                        if (itr == m_squadDataMap.end())
                            continue;
                        for (auto member : itr->second.members)
                            if (scope == Fleet::BCast::Scope::Universe) {
                                members.push_back(member.second);
                            } else if (scope == Fleet::BCast::Scope::System) {
                                if (member.second->GetSystemID() == scopeID)
                                    members.push_back(member.second);
                            } else if (scope == Fleet::BCast::Scope::Bubble) {
                                if (member.second->GetShipSE()->SysBubble()->GetID() == scopeID)
                                    members.push_back(member.second);
                            }
                    }
                } else {
                    std::map<uint32, SquadData>::iterator itr = m_squadDataMap.find(squadID);
                    if (itr == m_squadDataMap.end())
                        break;
                    for (auto member : itr->second.members)
                        if (scope == Fleet::BCast::Scope::Universe) {
                            members.push_back(member.second);
                        } else if (scope == Fleet::BCast::Scope::System) {
                            if (member.second->GetSystemID() == scopeID)
                                members.push_back(member.second);
                        } else if (scope == Fleet::BCast::Scope::Bubble) {
                            if (member.second->GetShipSE()->SysBubble()->GetID() == scopeID)
                                members.push_back(member.second);
                        }
                }
            }
        } break;
        case Fleet::BCast::Group::Up: {
            if (wingID == -1) {
                // pFrom is FC.  nobody above to msg.  make error here
                _log(FLEET__WARNING, "FC %s called FleetBroadcast with group == Up for fleet %u.", pFrom->GetName(), fleetID);
                pFrom->SendErrorMsg("You cannot broadcast to Superiors as FC.");
            } else {
                if (squadID == -1) {
                    members.push_back(GetFleetLeader(fleetID));
                    members.push_back(GetWingLeader(wingID));
                } else {
                    members.push_back(GetFleetLeader(fleetID));
                    members.push_back(GetWingLeader(wingID));
                    members.push_back(GetSquadLeader(squadID));
                }
            }
        } break;
    }

    // Broadcast(name, group, charID, solarSystemID = None, itemID = None, broadcastName(BroadcastEvent/label) = None):
    //OnFleetBroadcast(name, group, charID, solarSystemID, itemID):
    //   ('HealCapacitor', 3, 95895066, 30003500, 1019274373727L, None)))

    PyTuple* payload = new PyTuple(5);
        payload->SetItem(0, new PyString(msg));
        payload->SetItem(1, new PyInt(group));
        payload->SetItem(2, new PyInt(pFrom->GetCharacterID()));
        payload->SetItem(3, new PyInt(pFrom->GetSystemID()));
        payload->SetItem(4, new PyInt(itemID));
        // if BCastName(label) then add next item properly
        payload->SetItem(5, PyStatic.NewNone());

    uint8 count(0);
    for (auto cur : members) {
        if (cur == nullptr)
            continue;
        PySafeIncRef(payload);
        cur->SendNotification("OnFleetBroadcast", "clientID", payload, true);
        ++count;
    }

    if (is_log_enabled(FLEET__BCAST_DUMP)) {
        std::ostringstream grp;
        switch (group) {
            case Fleet::BCast::Group::All: {
                grp << "All " << std::to_string(count);
            } break;
            case Fleet::BCast::Group::Down:
            case Fleet::BCast::Group::Up: {
                grp << std::to_string(count);
                grp << " " << GetBCastGroupName(group).c_str();
            } break;
        }

        _log(FLEET__BCAST_DUMP, "%s FleetBroadcast '%s' to %s members of fleet %u.", GetBCastScopeName(scope).c_str(), msg.c_str(), grp.str().c_str() , fleetID);
        payload->Dump(FLEET__BCAST_DUMP, "   ");
    }
}

void FleetService::SendFleetUpdate(uint32 fleetID, const char* notifyType, PyTuple* payload)
{
    if (is_log_enabled(FLEET__UPDATE_DUMP)) {
        _log(FLEET__UPDATE_DUMP, "SendFleetUpdate '%s' to members of fleet %u.", notifyType, fleetID);
        payload->Dump(FLEET__UPDATE_DUMP, "   ");
    }

    std::vector<Client*> members;
    auto range = m_fleetMembers.equal_range(fleetID);
    for (auto fItr = range.first; fItr != range.second; ++fItr)
        members.push_back(fItr->second);

    for (auto cur : members) {
        if (cur == nullptr)
            continue;
        PySafeIncRef(payload);
        cur->SendNotification(notifyType, "*fleetid", payload, true);    // this sends "*fleetid" update to all fleet members and is sequenced
    }
}

void FleetService::GetFleetMembersOnGrid(Client* pClient, std::vector< uint32 >& data)
{
    std::vector<Client*> members;
    uint16 scopeID = pClient->GetShipSE()->SysBubble()->GetID();
    auto range = m_fleetMembers.equal_range(pClient->GetFleetID());
    for (auto fItr = range.first; fItr != range.second; ++fItr)
        members.push_back(fItr->second);

    for (auto cur : members) {
        if (cur == nullptr)
            continue;
        if (cur->GetShipSE()->SysBubble()->GetID() == scopeID)
            data.push_back(cur->GetCharacterID());
    }
}

void FleetService::GetFleetMembersInSystem(Client* pClient, std::vector< uint32 >& data)
{
    std::vector<Client*> members;
    uint16 scopeID = pClient->GetShipSE()->SystemMgr()->GetID();
    auto range = m_fleetMembers.equal_range(pClient->GetFleetID());
    for (auto fItr = range.first; fItr != range.second; ++fItr)
        members.push_back(fItr->second);

    for (auto cur : members) {
        if (cur == nullptr)
            continue;
        if (cur->GetShipSE()->SystemMgr()->GetID() == scopeID)
            data.push_back(cur->GetCharacterID());
    }
}

void FleetService::GetFleetClientsInSystem(Client* pClient, std::vector< Client* >& data)
{
    std::vector<Client*> members;
    uint32 scopeID = pClient->GetShipSE()->SystemMgr()->GetID();
    auto range = m_fleetMembers.equal_range(pClient->GetFleetID());
    for (auto fItr = range.first; fItr != range.second; ++fItr)
        members.push_back(fItr->second);

    for (auto cur : members) {
        if (cur == nullptr)
            continue;
        if (cur->GetShipSE()->SystemMgr()->GetID() == scopeID)
            data.push_back(cur);
    }
}

std::vector<Client *> FleetService::GetFleetClients(uint32 fleetID) {
    std::vector<Client*> members;
    auto range = m_fleetMembers.equal_range(fleetID);
    for (auto fItr = range.first; fItr != range.second; ++fItr)
        members.push_back(fItr->second);

    return members;
}

void FleetService::SendActiveStatus(uint32 fleetID, int32 wingID, int32 squadID)
{
    PyTuple* count = new PyTuple(1);
    count->SetItem(0, new PyInt((255 - m_fleetMembers.count(fleetID))));  // this is slots left from 255 (256 - leader)
    SendFleetUpdate(fleetID, "OnFleetActive", count);

    if (wingID > 0) {
        WingData wData = WingData();
        GetWingData(wingID, wData);
        PyTuple* count = new PyTuple(2);
            count->SetItem(0, new PyInt(wingID));
            count->SetItem(1, new PyInt(IsWingActive(wingID) ? 1 : 0));
        SendFleetUpdate(fleetID, "OnWingActive", count);
    }

    if (squadID > 0) {
        SquadData sData = SquadData();
        GetSquadData(squadID, sData);
        PyTuple* count = new PyTuple(2);
            count->SetItem(0, new PyInt(squadID));
            count->SetItem(1, new PyInt(sData.members.size()? 1 : 0));
        SendFleetUpdate(fleetID, "OnSquadActive", count);
    }
}

bool FleetService::GetInviteData(uint32 charID, InviteData& data)
{
    std::map<uint32, InviteData>::iterator itr = m_inviteData.find(charID);
    if (itr != m_inviteData.end()) {
        data = itr->second;
        return true;
    }
    return false;
}

bool FleetService::SaveInviteData(uint32 charID, InviteData& data)
{
    std::map<uint32, InviteData>::iterator itr = m_inviteData.find(charID);
    if (itr != m_inviteData.end())
        return false;

    m_inviteData.emplace(charID, data);    // map of invites by invitedCharID
    return true;
}

void FleetService::RemoveInviteData(uint32 charID)
{
    m_inviteData.erase(charID); // remove invite data
}

bool FleetService::AddJoinRequest(uint32 fleetID, Client* pClient)
{
    // ensure only one request per client
    auto range = m_joinReq.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        if (itr->second == pClient)
            return false;
    m_joinReq.emplace(fleetID, pClient);
    return true;
}

void FleetService::GetJoinRequests(uint32 fleetID, std::vector< Client* >& data)
{
    auto range = m_joinReq.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        data.push_back(itr->second);
}

void FleetService::RemoveJoinRequest(uint32 fleetID, Client* pClient)
{
    auto range = m_joinReq.equal_range(fleetID);
    for (auto itr = range.first; itr != range.second; ++itr)
        if (itr->second == pClient) {
            m_joinReq.erase(itr);
            return;
        }
}

std::string FleetService::GetJobName(int8 job)
{
    switch (job) {
        case 0:     return "None";
        case 1:     return "Scout";
        case 2:     return "Creator";
        default:    return "Invalid Job";
    }
}

std::string FleetService::GetRoleName(int8 role)
{
    switch (role) {
        case 1:     return "FleetLeader";
        case 2:     return "WingLeader";
        case 3:     return "SquadLeader";
        case 4:     return "Member";
        default:    return "Invalid Role";
    }
}

std::string FleetService::GetBoosterName(int8 booster)
{
    switch (booster) {
        case 0:     return "No";
        case 1:     return "Fleet";
        case 2:     return "Wing";
        case 3:     return "Squad";
        default:    return "Invalid Booster";
    }
}

std::string FleetService::GetBCastScopeName(int8 scope)
{
    switch (scope) {
        case 0:     return "Universe";
        case 1:     return "System";
        case 2:     return "Bubble";
        default:    return "Invalid Scope";
    }
}

std::string FleetService::GetBCastGroupName(int8 group)
{
    switch (group) {
        case 0:     return "No";
        case 1:     return "Subordinate";
        case 2:     return "Superior";
        case 3:     return "All";
        default:    return "Invalid Group";
    }
}

std::string FleetService::GetBoosterData(uint32 fleetID, uint16& length)
{
    if (!IsFleetID(fleetID) or (fleetID > m_fleetID))
        return "Invalid Data";

    /** @todo  add system checks in here */
    Character* pChar(nullptr);
    std::ostringstream str;
    str.clear();

    bool fboost(false);
    FleetData fData = FleetData();
    GetFleetData(fleetID, fData);
    str << "<color=teal>" << fData.name << "  Created By: " << fData.creator->GetChar()->itemName();
    str << "  Members: " << std::to_string(m_fleetMembers.count(fleetID)) << "</color><br>"; //54
    length += 54;
    length += fData.name.size();
    length += fData.creator->GetChar()->itemName().size();

    if ((fData.leader != nullptr) and (fData.leader->IsInSpace()) and (pChar = fData.leader->GetChar().get()) != nullptr) {
        if (m_fleetWings.count(fleetID) > pChar->GetSkillLevel(EvESkill::FleetCommand)) {
            str << "<color=red>";
            fboost = false;
        } else {
            str << "<color=green>";
            fboost = true;
        }
        length += 13;
        str << "Fleet Cmdr: " << pChar->itemName(); //12
        length += pChar->itemName().size();

        str << "    " << std::to_string(pChar->GetSkillLevel(EvESkill::FleetCommand)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::WingCommand)) << "/";
        str << std::to_string(pChar->GetSkillLevel(EvESkill::Leadership)) << "</color><br>";//15
        length += 30;
    } else {
        str << "<color=red>No Fleet Cmdr</color><br>";
        length += 37;
    }

    if ((fData.booster != nullptr) and (fData.booster->IsInSpace()) and (pChar = fData.booster->GetChar().get()) != nullptr) {
        if (fData.leader->GetSystemID() == fData.booster->GetSystemID()) {
            if (pChar->HasSkillTrainedToLevel(EvESkill::ArmoredWarfare, 1) or pChar->HasSkillTrainedToLevel(EvESkill::InformationWarfare, 1)
                or pChar->HasSkillTrainedToLevel(EvESkill::SiegeWarfare, 1) or pChar->HasSkillTrainedToLevel(EvESkill::SkirmishWarfare, 1)
                or pChar->HasSkillTrainedToLevel(EvESkill::MiningForeman, 1)) {
                if (fboost) {
                    str << "<color=green>";
                } else {
                    str << "<color=yellow>";
                }
                } else {
                    if (fboost) {
                        str << "<color=yellow>";
                    } else {
                        str << "<color=red>";
                    }
                    fboost = false;
                }
                length += 14;
        } else {
            str << "<color=red> (Not In System)";
            fboost = false;
            length += 30;
        }
        str << "Fleet Booster: " << pChar->itemName();//15
        length += pChar->itemName().size();
        str << "    " << std::to_string(pChar->GetSkillLevel(EvESkill::ArmoredWarfare)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::InformationWarfare)) << "/";
        str << std::to_string(pChar->GetSkillLevel(EvESkill::SiegeWarfare)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::SkirmishWarfare)) << "/";
        str << std::to_string(pChar->GetSkillLevel(EvESkill::MiningForeman)) << "</color><br>";//30
        length += 50;
    } else {
        str << "<color=red>No Fleet Booster</color><br>";
        length += 40;
        fboost = false;
    }

    std::vector< uint32 > wingIDs, squadIDs;
    wingIDs.clear();
    GetWingIDs(fleetID, wingIDs);
    for (auto wingID : wingIDs) {
        if (!IsWingID(wingID))
            continue;
        bool wboost(false);
        WingData wData = WingData();
        GetWingData(wingID, wData);
        if ((wData.leader != nullptr) and (wData.leader->IsInSpace()) and (pChar = wData.leader->GetChar().get()) != nullptr) {
            if (m_wingSquads.count(wingID) > pChar->GetSkillLevel(EvESkill::WingCommand)) {
                str << "<color=red>";
            } else {
                if (fboost) {
                    str << "<color=green>";
                } else {
                    str << "<color=yellow>";
                }
                wboost = true;
            }
            length += 13;
            str << "  " << wData.name.c_str() << " Cmdr: " << pChar->itemName(); //10
            length += wData.name.size();
            length += pChar->itemName().size();
            str << "    " << std::to_string(pChar->GetSkillLevel(EvESkill::FleetCommand)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::WingCommand)) << "/";
            str << std::to_string(pChar->GetSkillLevel(EvESkill::Leadership)) << "</color><br>";//15
            length += 30;
        } else {
            str << "  <color=red>" << wData.name.c_str() << " No Cmdr</color><br>";//33
            length += wData.name.size();
            length += 35;
        }
        if ((wData.booster != nullptr) and (wData.booster->IsInSpace()) and (pChar = wData.booster->GetChar().get()) != nullptr) {
            if (wData.leader->GetSystemID() == wData.booster->GetSystemID()) {
                if (pChar->HasSkillTrainedToLevel(EvESkill::ArmoredWarfare, 1) or pChar->HasSkillTrainedToLevel(EvESkill::InformationWarfare, 1)
                    or pChar->HasSkillTrainedToLevel(EvESkill::SiegeWarfare, 1) or pChar->HasSkillTrainedToLevel(EvESkill::SkirmishWarfare, 1)
                    or pChar->HasSkillTrainedToLevel(EvESkill::MiningForeman, 1)) {
                    if (wboost) {
                        str << "<color=green>";
                    } else {
                        str << "<color=yellow>";
                    }
                    } else {
                        if (wboost) {
                            str << "<color=yellow>";
                        } else {
                            str << "<color=red>";
                        }
                        wboost = false;
                    }
                    length += 14;
            } else {
                str << "<color=red> (Not In System)";
                wboost = false;
                length += 30;
            }
            str << "  " << "Booster: " << pChar->itemName();//11
            length += pChar->itemName().size();
            str << "    " << std::to_string(pChar->GetSkillLevel(EvESkill::ArmoredWarfare)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::InformationWarfare));
            str << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::SiegeWarfare)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::SkirmishWarfare)) << "/";
            str << std::to_string(pChar->GetSkillLevel(EvESkill::MiningForeman)) << "</color><br>";//32
            length += 38;
        } else {
            str << "  <color=red>" << wData.name.c_str() << " No Booster</color><br>";//43
            length += wData.name.size();
            length += 45;
            wboost = false;
        }

        squadIDs.clear();
        GetSquadIDs(wingID, squadIDs);
        for (auto squadID : squadIDs) {
            if (!IsSquadID(squadID))
                continue;
            bool sboost(false);
            SquadData sData = SquadData();
            GetSquadData(squadID, sData);
            if ((sData.leader != nullptr) and (sData.leader->IsInSpace()) and (pChar = sData.leader->GetChar().get()) != nullptr) {
                if (sData.members.size() > (pChar->GetSkillLevel(EvESkill::Leadership) * 2)) {
                    str << "<color=red>";
                } else {
                    if (wboost) {
                        str << "<color=green>";
                    } else {
                        str << "<color=yellow>";
                    }
                    sboost = true;
                }
                length += 13;
                str << "    " << sData.name.c_str() <<  " Cmdr: " << pChar->itemName(); //11
                length += sData.name.size();
                length += pChar->itemName().size();
                str << "    " << std::to_string(pChar->GetSkillLevel(EvESkill::FleetCommand)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::WingCommand)) << "/";
                str << std::to_string(pChar->GetSkillLevel(EvESkill::Leadership)) << "</color><br>";//21
                length += 22;
            } else {
                str << "    <color=red>" << sData.name.c_str() << " No Cmdr</color><br>";
                length += sData.name.size();
                length += 37;
            }
            if ((sData.booster != nullptr) and (sData.booster->IsInSpace()) and (pChar = sData.booster->GetChar().get()) != nullptr) {
                if (sData.leader->GetSystemID() == sData.booster->GetSystemID()) {
                    if (pChar->HasSkillTrainedToLevel(EvESkill::ArmoredWarfare, 1) or pChar->HasSkillTrainedToLevel(EvESkill::InformationWarfare, 1)
                        or pChar->HasSkillTrainedToLevel(EvESkill::SiegeWarfare, 1) or pChar->HasSkillTrainedToLevel(EvESkill::SkirmishWarfare, 1)
                        or pChar->HasSkillTrainedToLevel(EvESkill::MiningForeman, 1)) {
                        if (sboost) {
                            str << "<color=green>";
                        } else {
                            str << "<color=yellow>";
                        }
                        } else {
                            if (sboost) {
                                str << "<color=yellow>";
                            } else {
                                str << "<color=red>";
                            }
                            sboost = false;
                        }
                        length += 14;
                } else {
                    str << "<color=red> (Not In System)";
                    sboost = false;
                    length += 30;
                }
                str << "    Booster: " << pChar->itemName();//13
                length += pChar->itemName().size();
                str << "    " << std::to_string(pChar->GetSkillLevel(EvESkill::ArmoredWarfare)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::InformationWarfare));
                str << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::SiegeWarfare)) << "/" << std::to_string(pChar->GetSkillLevel(EvESkill::SkirmishWarfare)) << "/";
                str << std::to_string(pChar->GetSkillLevel(EvESkill::MiningForeman)) << "</color><br>";//25
                length += 40;
            } else {
                str << "    <color=red>" << sData.name.c_str() << " No Booster</color><br>";//45
                length += sData.name.size();
                length += 43;
                sboost = false;
            }
            if (sboost) {
                str << "    <color=green>";
            } else {
                str << "    <color=red>";
            }
            str << "Members: " << std::to_string(sData.members.size()) << "  Effective: ";
            length += 40;
            std::string bdata;
            if (sboost) {   // this is the only way i could get these working right...dunno why
                bdata = std::to_string(sData.boost.leader);
                bdata += "/";
                bdata += std::to_string(sData.boost.armored);
                bdata += "/";
                bdata += std::to_string(sData.boost.info);
                bdata += "/";
                bdata += std::to_string(sData.boost.siege);
                bdata += "/";
                bdata += std::to_string(sData.boost.skirmish);
                bdata += "/";
                bdata += std::to_string(sData.boost.mining);
                length += 11;
            } else {
                bdata = "0/0/0/0/0/0  (";
                bdata += std::to_string(sData.boost.leader);
                bdata += "/";
                bdata += std::to_string(sData.boost.armored);
                bdata += "/";
                bdata += std::to_string(sData.boost.info);
                bdata += "/";
                bdata += std::to_string(sData.boost.siege);
                bdata += "/";
                bdata += std::to_string(sData.boost.skirmish);
                bdata += "/";
                bdata += std::to_string(sData.boost.mining);
                bdata += ")";
                length += 26;
            }
            str << bdata.c_str();
            str << "</color><br>";//12
            length += 12;
        }
        if (squadIDs.empty()) {
            str << "    <color=fuchsia>No Squads</color><br><br>";
            length += 45;
        }
    }
    if (wingIDs.empty()) {
        str << "  <color=fuchsia>No Wings</color><br><br>";
        length += 45;
    }

    return str.str();
}
