
/**
 * @name AnomalyMgr.cpp
 *     Anomaly management system for EVEmu
 *
 * @Author:        Allan
 * @date:          12 December 2015 (original idea)
 * @update:        3 August 2017 (begin implementation)
 *
 */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "map/MapData.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"

#include "../../../eve-common/EVE_Scanning.h"

/*  this class will keep track of all Anomalies for its system
 *.   the scan system will query this class for current anomaly data
 *
 *  it will need access to its system manager (thru m_system), the wh mgr (thru sWHMgr), dungeon mgr (thru m_dungeon),
 *       belt mgr (thru m_beltMgr), and spawn mgr (thru m_spawnMgr)
 *. this will use spawn mgr to spawn npcs, using npc class groups following roid rat layout
 *
 *  when one anomaly despawns, this class is in charge of calling cleanup and creating another as needed.
 *
 *  this class is also in charge of all dynamic anomaly data in the db
 *    pos items, wrecks and abandoned ships will have to process thru here also,
 *    as they get sigIDs and are listed on scan results, but cannot be totally scanned down
 */

/*
 * COSMIC_MGR__ERROR
 * COSMIC_MGR__WARNING
 * COSMIC_MGR__MESSAGE
 * COSMIC_MGR__DEBUG
 * COSMIC_MGR__TRACE
 */
AnomalyMgr::AnomalyMgr(SystemManager* mgr, PyServiceMgr& svc)
:m_services(svc),
m_system(mgr),
m_beltMgr(nullptr),
m_dungMgr(nullptr),
m_spawnMgr(nullptr),
m_spawnTimer(0),
m_procTimer(0),
m_WH(0),
m_Sigs(0),
m_Anoms(0),
m_Grav(0),
m_Mag(0),
m_Ladar(0),
m_Radar(0),
m_Unrated(0),
m_Complex(0),
m_maxSigs(0),
m_initalized(false)
{
    m_sigBySigID.clear();
    m_sigByItemID.clear();
}

AnomalyMgr::~AnomalyMgr()
{
    // this shouldnt be needed...SysMgr should handle all objects in their systems
    /*
    InventoryItemRef iRef(nullptr);
    for (auto sig : m_sigByItemID) {
        iRef = sItemFactory.GetItemRef(sig.first);
        if (iRef.get() == nullptr)
            continue;
        iRef->Delete();
    }
    */
}

bool AnomalyMgr::Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr) {
    m_beltMgr = beltMgr;
    m_dungMgr = dungMgr;
    m_spawnMgr = spawnMgr;

    if (m_beltMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. beltMgr == nullptr.  Not Initializing Anomaly Manager for %s(%u)", m_system->GetName(), m_system->GetID());
        return m_initalized;
    }

    if (m_dungMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. dungMgr == nullptr.  Not Initializing Anomaly Manager for %s(%u)", m_system->GetName(), m_system->GetID());
        return m_initalized;
    }

    if (m_spawnMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. spawnMgr == nullptr.  Not Initializing Anomaly Manager for %s(%u)", m_system->GetName(), m_system->GetID());
        return m_initalized;
    }

    if (!sConfig.cosmic.AnomalyEnabled) {
        _log(COSMIC_MGR__INIT, "Anomaly System Disabled.  Not Initializing Anomaly Manager for %s(%u)", m_system->GetName(), m_system->GetID());
        return true;
    }
    if (!sConfig.cosmic.DungeonEnabled){
        _log(COSMIC_MGR__INIT, "Dungeon System Disabled.  Not Initializing Anomaly Manager for %s(%u)", m_system->GetName(), m_system->GetID());
        return true;
    }

    // Load saved anomalies from db upon system init
    LoadAnomalies();

    // set internal check data
    // range is 0.1 for 1.0 system to 2.0 for -0.9 system
    float security = m_system->GetSecValue();
    if (sConfig.debug.IsTestServer) {
        m_maxSigs = 2;
        m_procTimer.Start(10000);  // 10s
    } else {
             if (security == 2.0)  { m_maxSigs = 25; }
        else if (security > 1.499) { m_maxSigs = 20; }
        else if (security > 0.999) { m_maxSigs = 15; }
        else if (security > 0.749) { m_maxSigs = 12; }
        else if (security > 0.449) { m_maxSigs = 8; }
        else if (security > 0.249) { m_maxSigs = 5; }
        else                       { m_maxSigs = 3; }

        m_procTimer.Start(120000);  // 2m
    }

    // these use config option to (en/dis)able individual types
    m_Grav = sConfig.exploring.Gravametric;
    m_Mag = sConfig.exploring.Magnetometric;
    m_Ladar = sConfig.exploring.Ladar;
    m_Radar = sConfig.exploring.Radar;
    m_Unrated = sConfig.exploring.Unrated;
    m_Complex = sConfig.exploring.Complex;

    /* load current data?, start timers, process current data, and create new items, if needed */
    /** @todo all anomalies are currently temp items.  if/when we start saving them, create new table and itemIDs*/

    _log(COSMIC_MGR__INIT, "AnomalyMgr Initialized for %s(%u) with %u Max Signals for security class %0.2f.  Test Server %s", \
                m_system->GetName(), m_system->GetID(), m_maxSigs, security, sConfig.debug.IsTestServer?"enabled":"disabled");

    return (m_initalized = true);
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;
    if (m_procTimer.Check(/*!sConfig.debug.IsTestServer*/)) {
        if (m_Sigs < m_maxSigs)
            CreateAnomaly();
        if (m_Anoms < (m_maxSigs /2))
            CreateAnomaly(Dungeon::Type::Anomaly);
        
    }

    //TODO: Implement checking for expired anomalies
    /*if (m_spawnTimer.Check(false)) {
        // Check for expired anomalies and delete them

    }*/
}

void AnomalyMgr::Close()
{
    _log(COSMIC_MGR__MESSAGE, "Closing AnomalyMgr for %s(%u).", m_system->GetName(), m_system->GetID());
}

void AnomalyMgr::LoadAnomalies() {
    // check for existing data and load accordingly.
    // this will only hit on system load
    DBQueryResult res;
    m_mdb.GetAnomaliesBySystem(m_system->GetID(), res);
    DBResultRow row;
    while (res.GetRow(row)) {
        //sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,wormholeID,x,y,z
        CosmicSignature sig = CosmicSignature();
        sig.sigID = row.GetText(0);
        sig.sigItemID = row.GetInt(1);
        sig.dungeonType = row.GetInt(2);
        sig.sigName = row.GetText(3);
        sig.systemID = row.GetInt(4);
        sig.sigTypeID = row.GetInt(5);
        sig.sigGroupID = row.GetInt(6);
        sig.scanGroupID = row.GetInt(7);
        sig.sigStrength = 10.0f;
        sig.scanAttributeID = row.GetInt(8);
        sig.position.x = row.GetDouble(9);
        sig.position.y = row.GetDouble(10);
        sig.position.z = row.GetDouble(11);

        // Register loaded signatures
        m_dungMgr->MakeDungeon(sig);
        m_sigBySigID.emplace(sig.sigID, sig);
        m_sigByItemID.emplace(sig.sigItemID, sig);
        ++m_Sigs;

        _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::LoadAnomalies() - Created Signal %s(%u) for %s in %s(%u), bubbleID %u with %.3f%% sigStrength.", \
        sDunDataMgr.GetDungeonType(sig.dungeonType), sig.dungeonType, \
        sig.sigName.c_str(), m_system->GetName(), sig.systemID, sig.bubbleID, sig.sigStrength *100);
    }

}

void AnomalyMgr::SaveAnomaly(CosmicSignature& sig)
{
    // Save anomalies based upon their type to the database
    if (sig.dungeonType == Dungeon::Type::Wormhole) {
        m_mdb.SaveAnomaly(sig);
    }
}

void AnomalyMgr::GetSignatureList(std::vector<CosmicSignature>& sig)
{
    // sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    // retrieval method for scan queries
    for (auto cur : m_sigByItemID)
        sig.push_back(cur.second);
}

void AnomalyMgr::GetAnomalyList(std::vector<CosmicSignature>& sig) {
    // sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    // retrieval method for scan queries
    for (auto cur : m_anomByItemID)
        sig.push_back(cur.second);
}

void AnomalyMgr::CreateAnomaly(int8 typeID/*0*/)
{
    // compile data for new system anomaly.
    CosmicSignature sig = CosmicSignature();
        sig.systemID = m_system->GetID();
        sig.sigID = sEntityList.GetAnomalyID();
        // *Mgr will determine name, itemID and sigStrength.
        sig.sigItemID = 0;
        sig.sigName = "Test Name Here";
        //default to 1/80
        sig.sigStrength = 0.0125;
        // default to rogue drones
        sig.ownerID = factionRogueDrones;
    if (sConfig.debug.AnomalyFaction) {
        sig.ownerID = sConfig.debug.AnomalyFaction;
    } else if (MakeRandomFloat() > 0.1) { // 10% chance to be rogue drones
        sig.ownerID = sDataMgr.GetRegionRatFaction(m_system->GetRegionID());
    }

    // there are some regions that dont have rat factions....what do we do in that case??
    //if (sig.ownerID == 0)
    //    return;     // make error here?

    if (typeID == 0) {
        sig.dungeonType = GetDungeonType();
    } else {  // proc calling anomaly or mission/escalation being setup.
        sig.dungeonType = typeID;
    }

    if (sig.dungeonType == 0) {
        _log(COSMIC_MGR__ERROR, "Dungeon Type returned 0 for %s in %s(%u)", \
                    sig.sigName.c_str(), m_system->GetName(), m_system->GetID());
        return;
    }

    sig.position = sMapData.GetAnomalyPoint(m_system);

    // some sites will use sys sov for ships.  use this for them....
    // sig.ownerID = sDataMgr.GetRegionFaction(m_system->GetRegionID());
    switch(sig.dungeonType) {
        case Dungeon::Type::Gravimetric: { // 2
            sig.sigTypeID = EVEDB::invTypes::CosmicSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanGravimetricStrength;
        } break;
        case Dungeon::Type::Magnetometric: { // 3,
            sig.sigTypeID = EVEDB::invTypes::DeadspaceSignature;// need probes and exploring skills
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanMagnetometricStrength;
        } break;
        case Dungeon::Type::Radar: {       // 4,
            sig.sigTypeID = EVEDB::invTypes::DeadspaceSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanRadarStrength;
        } break;
        case Dungeon::Type::Ladar: {       // 5,
            sig.sigTypeID = EVEDB::invTypes::DeadspaceSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanLadarStrength;
        } break;
        case Dungeon::Type::Wormhole: {    // 6
            // enable WH to be warped to...they are deco only at this time.
            //  once working, these will be by probe only, and removed from anomaly list
            sig.sigTypeID = EVEDB::invTypes::CosmicSignature;
            sig.sigGroupID = EVEDB::invGroups::Wormhole;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanAllStrength;  // Unknown
            // hand off to WHMgr for creation and exit after return
            sWHMgr.Create(sig);
            // creation failure will set itemID to 0
            if (sig.sigItemID) {
                m_sigBySigID.emplace(sig.sigID, sig);
                m_sigByItemID.emplace(sig.sigItemID, sig);
            }
            // Save wormhole to the database for later loading
            SaveAnomaly(sig);
            return;
        } break;
        case Dungeon::Type::Anomaly: {      // 7   simple combat sites
            sig.sigTypeID = EVEDB::invTypes::CosmicAnomaly;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.scanGroupID = Scanning::Group::Anomaly;
            sig.scanAttributeID = AttrScanAllStrength;
            sig.sigStrength = 1.0f;
        } break;
        case Dungeon::Type::Mission: {      // 1
            sig.sigTypeID = EVEDB::invTypes::CosmicSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanAllStrength;  // Unknown
            // we're not counting mission shit in sig count
        }
        // these will use default for now.  revisit later when system matures more and i better understand how to implement them.
        case Dungeon::Type::Escalation:   // 9
        case Dungeon::Type::Unrated:       // 8
        case Dungeon::Type::Rated: { // 10
            sig.sigTypeID = EVEDB::invTypes::DeadspaceSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.scanAttributeID = AttrScanAllStrength;  // Unknown
        } break;
        // error or denied
        case 0:
        default:
            return;
    }

    // create and register here
    // all anomalies will be created/populated by dungMgr, except WH (handed off to WHMgr above)
    /** @todo  check distances based on system size */
    if (!m_dungMgr->MakeDungeon(sig)) // pass by ref here, so other vars can be set.
        return;
    // add new sig to sysSigMaps
    m_sigBySigID.emplace(sig.sigID, sig);
    if (sig.scanGroupID == Scanning::Group::Anomaly) {
        m_anomByItemID.emplace(sig.sigItemID, sig);
        ++m_Anoms;
    } else {
        m_sigByItemID.emplace(sig.sigItemID, sig);
        ++m_Sigs;
    }

    // Save anomaly to the database for later loading
    SaveAnomaly(sig);

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::Create() - Created Signal %s(%u) for %s in %s(%u), bubbleID %u with %.3f%% sigStrength.", \
            sDunDataMgr.GetDungeonType(sig.dungeonType), sig.dungeonType, \
            sig.sigName.c_str(), m_system->GetName(), sig.systemID, sig.bubbleID, sig.sigStrength *100);
}

// Register an exit wormhole created by WhMgr's 'CreateExit()' function
void AnomalyMgr::RegisterExitWH(CosmicSignature &sig)
{
    m_sigBySigID.emplace(sig.sigID, sig);
    m_sigByItemID.emplace(sig.sigItemID, sig);
    ++m_Sigs;

    // Save anomaly to the database for later loading
    SaveAnomaly(sig);

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::RegisterExitWH() - Created Signal %s(%u) for %s in %s(%u), bubbleID %u with %.3f%% sigStrength.", \
            sDunDataMgr.GetDungeonType(sig.dungeonType), sig.dungeonType, \
            sig.sigName.c_str(), m_system->GetName(), sig.systemID, sig.bubbleID, sig.sigStrength *100);
}

uint8 AnomalyMgr::GetDungeonType()
{
    uint8 typeID = MakeRandomInt(2,10); // skip typeMission
    switch(typeID) {
        case Dungeon::Type::Escalation:  // 9
        case Dungeon::Type::Mission: {   // 1
            // cannot create this type here.  try again.
            return GetDungeonType();
        } break;
        case Dungeon::Type::Gravimetric: {   // 2
            if (m_Grav < 0)
                return GetDungeonType();

            ++m_Grav;
        } break;
        case Dungeon::Type::Magnetometric: {   // 3
            if (m_Mag < 0)
                return GetDungeonType();

            ++m_Mag;
        } break;
        case Dungeon::Type::Radar: {   // 4
            if (m_Radar < 0)
                return GetDungeonType();

            ++m_Radar;
        } break;
        case Dungeon::Type::Ladar: {   // 5
            if (m_Ladar < 0)
                return GetDungeonType();

            ++m_Ladar;
        } break;
        case Dungeon::Type::Wormhole: {   // 6
            // cap at 1 per system, except k162...which ISNT created in this system (it's an exit, from WMS)
            //  this will need updating and be controlled by WMS.  it will have full control of WH amounts/locations
            if (m_WH != 0)
                return GetDungeonType();

            ++m_WH;
        } break;
        case Dungeon::Type::Anomaly: {   // 7. this is noob dungeon, no probe required
            if (m_Anoms > (m_maxSigs /2))
                return GetDungeonType();
        } break;
        case Dungeon::Type::Unrated: {   // 8
            if ((m_Unrated < 0) or (m_Unrated > 2)) // cap at 3
                return GetDungeonType();

            ++m_Unrated;
        } break;
        case Dungeon::Type::Rated: {  // 10
            if ((m_Complex < 0) or (m_Complex > 1)) // cap at 2
                return GetDungeonType();

            ++m_Complex;
        } break;
    }

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::GetDungeonType() - Returning %s(%u)", sDunDataMgr.GetDungeonType(typeID), typeID);
    return typeID;
}

uint32 AnomalyMgr::GetAnomalyID(std::string& sigID)
{   // <std::string, CosmicSignature>
    std::map<std::string, CosmicSignature>::iterator itr = m_sigBySigID.find(sigID);
    if (itr != m_sigBySigID.end())
        return itr->second.sigItemID;
    return 0;
}

GPoint AnomalyMgr::GetAnomalyPos(std::string& sigID)
{
    GPoint pos(NULL_ORIGIN);
    std::map<std::string, CosmicSignature>::iterator itr = m_sigBySigID.find(sigID);
    if (itr != m_sigBySigID.end())
        pos = itr->second.position;

    return pos;
}


void AnomalyMgr::AddSignal(SystemEntity* pSE, uint32 id/*0*/)
{
    if (!m_initalized)
        return;

    //  shouldnt need this while `check(IsTestServer)` is disabled
    //if (!m_procTimer.Enabled())
    //    m_procTimer.Start(120000);  // 2m

    /* ALL SEs sent here by SysMgr except Globals and NPCs
     * we will determine here what is added and it's base sigStrength.
     * this is sill WIP.
     *   see Scan.xmlp and EVE_Scanning.h for client data
     */
    InventoryItemRef iRef = pSE->GetSelf();
    if (iRef.get() == nullptr)
        return; // we'll get over it.
    CosmicSignature sig = CosmicSignature();
        sig.dungeonType = Dungeon::Type::Anomaly;
        sig.ownerID = iRef->ownerID();
        if (id) {
            sig.sigID = id;
        } else {
            sig.sigID = sEntityList.GetAnomalyID();
        }
        sig.sigItemID = iRef->itemID();
        sig.sigName = iRef->itemName();
        sig.systemID = m_system->GetID();
        sig.position = iRef->position();
        // this will be wrong for capital ships and larger (maybe bs also)
        sig.sigStrength = iRef->GetAttribute(AttrSignatureRadius).get_float() /1000;
        // get bubbleID, which is only used for .siglist command
        sig.bubbleID = pSE->SysBubble()->GetID();

/* Signal Strength Base Data
 * Band        1/5     1/10    1/15    1/20    1/25    1/40    1/45    1/60    1/80
 * Percentage  20.0%   10.0%   6.67%   5.0%    4.0%    2.5%    2.22%   1.67%   1.25%
 */

    // if scanGroupID is anom or sig, use scanAttributeID to determine site type (in client code)
    //    scanGroupID must be one of the 5 groups coded in client (sig, anom, ship, drone, structure)
    //    scanGroupID of sig and anom are cached on client side (min time is 5m)
    switch (iRef->categoryID()) {
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::StructureUpgrade:
        case EVEDB::invCategories::SovereigntyStructure: {
            sig.sigTypeID = iRef->typeID();
            sig.sigGroupID = iRef->groupID();
            sig.scanGroupID = Scanning::Group::Structure;
            sig.scanAttributeID = AttrScanStrengthStructures;
            //sig.sigStrength = 1.0;
        } break;
        case EVEDB::invCategories::Ship: {
            //511     shipScanResistance
            sig.sigTypeID = iRef->typeID();
            sig.sigGroupID = iRef->groupID();
            sig.scanGroupID = Scanning::Group::Ship;
            sig.scanAttributeID = AttrScanStrengthShips;
            //sig.sigStrength = 1.0;
        } break;
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Charge:          // probes, missiles (at time of scan), and ??
        case EVEDB::invCategories::Deployable: {    // mobile warp disruptor
            sig.sigTypeID = iRef->typeID();
            sig.sigGroupID = iRef->groupID();
            sig.scanGroupID = Scanning::Group::DroneOrProbe;
            sig.scanAttributeID = AttrScanStrengthDronesProbes;
            sig.sigStrength = 0.0667;
        } break;
        case EVEDB::invCategories::Asteroid:{   // this wont hit (addSignal=false)
            return;     // we're not adding roids to list
        };
        case EVEDB::invCategories::Entity:
        case EVEDB::invCategories::Celestial:       //wrecks
        default:  {
            sig.sigTypeID = EVEDB::invTypes::CosmicAnomaly;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.scanGroupID = Scanning::Group::Anomaly;
            sig.scanAttributeID = AttrScanAllStrength;  // Unknown
            //sig.sigStrength = 1.0;      // this will need to be adjusted for entity/celestial types
        } break;
    }

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::AddSignal() - adding %s to anomaly list as %s(%u) with %.3f%% sigStrength.", \
                iRef->name(), GetScanGroupName(sig.scanGroupID), sig.scanGroupID, sig.sigStrength *100);

    // add new sig to our maps, but these are not added to anom/sig counts
    m_sigBySigID.emplace(sig.sigID, sig);
    if (sig.scanGroupID == Scanning::Group::Anomaly) {
        m_anomByItemID.emplace(sig.sigItemID, sig);
    } else {
        m_sigByItemID.emplace(sig.sigItemID, sig);
    }
}

void AnomalyMgr::RemoveSignal(uint32 itemID)
{
    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::RemoveSignal() - removing %u from anomaly list.", itemID);
    // remove sig from our map
    std::map<uint32, CosmicSignature>::iterator itr = m_sigByItemID.find(itemID);
    if (itr != m_sigByItemID.end()) {
        auto itr2 = m_sigBySigID.find(itr->second.sigID);
        if (itr2 != m_sigBySigID.end())
            m_sigBySigID.erase(itr2);
        m_sigByItemID.erase(itr);
    } else {  // not Signature, check in Anomaly map
        itr = m_anomByItemID.find(itemID);
        if (itr != m_anomByItemID.end()) {
            auto itr2 = m_sigBySigID.find(itr->second.sigID);
            if (itr2 != m_sigBySigID.end())
                m_sigBySigID.erase(itr2);
            m_anomByItemID.erase(itr);
        }
    }
}

const char* AnomalyMgr::GetScanGroupName(uint8 groupID/*0*/) {
    using namespace Scanning::Group;
    switch(groupID) {
        case Scrap:             return "Scrap";
        case Signature:         return "Signature";
        case Structure:         return "Structure";
        case DroneOrProbe:      return "Drone/Probe";
        case Celestial:         return "Celestial";
        case Anomaly:           return "Anomaly";
        case Ship:              return "Ship";
        case 0:                 return "Zero";
        default:                return "Invalid";
    }
    return "None";
}
