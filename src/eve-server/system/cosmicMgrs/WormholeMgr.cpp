
 /**
  * @name WormholeMgr.cpp
  *     WH management system for EVEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  * Updates:        James
  * Date:           13 November 2021
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryItem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
#include "map/MapData.h"

/*  this class will need to keep track of all WH in universe, what systems they connect to, and how long they last.
 *
 *  it will need access to the system manager, a list of systems, and a way to boot a new system when needed.
 *
 *  this class will also need access to the wspace system manager (which may be same sysmgr)
 * with a way to access/track/boot as needed.
 *
 *  when one WH collapses, it will be in charge of creating new WH in the place of the old
 * one, making approprate connections, and tracking any other changes in the WH itself
 *
 *  this class is also in charge of all dynamic WH data in the db
 *
 */

WormholeMgr::WormholeMgr()
:  m_updateTimer(0),
    m_services(nullptr),
    m_initalized(false)
{
}

WormholeMgr::~WormholeMgr()
{
    /* nothing to do here */
}

void WormholeMgr::Initialize(PyServiceMgr* svc) {
    double start = GetTimeMSeconds();
    m_services = svc;

    m_updateTimer.Start(300000);    // arbitrary 5m default

    m_initalized = true;

    /* load current data, start timers, process current data, and create new items, if needed */

    sLog.Blue(" Wormhole Manager", "Wormhole Manager Initialized.");

}

// this is called on a minute timer from EntityList
void WormholeMgr::Process() {
    if (!m_initalized)
        return;
    if (m_updateTimer.Check(false)) {
        /* do something useful here */
    }
}

void WormholeMgr::Create(CosmicSignature& sig, uint32 exitSystemID/*=0*/, uint32 exitSourceItemID/*=0*/)
{
    // this really isnt needed.  may need later
    if (sig.dungeonType != Dungeon::Type::Wormhole)
        return;

    /*
     * Band        1/5     1/10    1/15    1/20    1/25    1/40    1/45    1/60    1/80
     * Percentage  20.0%   10.0%   6.67%   5.0%    4.0%    2.5%    2.22%   1.67%   1.25%
     *    Base sigStrength
     * k-space
     * 1/10 = exit(k162), hi>hi(a641), lo>c3(x702), lo>lo(x944)
     * 1/15 = hi>c1(z971)
     * 1/20 = hi>c2(r943),lo>c1(r943)
     * 1/25 = hi>c3(x702),hi>lo(r051),null>c3(x702),null>lo(n944)
     * 1/40 = hi>c4(o128),lo>c4(o128),lo>hi(b449),hi>null(v283),null>c1(z971),null>hi(b449)
     * 1/45 = null>c2(r943),null>c4(o128)
     * 1/60 = hi>c5(m555),lo>c5(n432),lo>null(s199),null>c5(n432)
     * 1/80 = hi>c6(b041)
     *
     * w-space
     * 1/10 = c1*hi(n110),c1>c1(h121),c2*hi(b274),c2>c1(z647),c3*lo(u210),c3>c3(n968),c4*c3(c247),c5*c5(h296),c5>null(z142),c6*c5(v911),c6>null(z142)
     * 1/15 = c1>c2(c125),c2>c2(d382),c3>c43(t405),c4*c4(x877),c5*c6(v753),c6*c6(w237)
     * 1/20 = c1*lo(j244),c1>c3(o883),c2*lo(a239),c2>c3(o477),c3*hi(d845),c3>c1(v301),c4*c1(p060),c5>lo(c140),c6*c3(l477),c6>lo(c140)
     * 1/25 = c1>c4(m609),c2>c4(y683),c3>c2(i182),c4*c2(n766),c6*c4(z457)
     * 1/40 = c1*null(z060),c1>c5(l614),c2*null(e545),c2>c5(n062),c3*null(k346),c3>c5(n770),c4*c5(h900),c5*c1(y790),c5>hi(d792),c6*c1(q317),c6>hi(d792)
     * 1/45 = c2>c6(r474),c3>c6(a982)
     * 1/60 = c5*c3(m267),c5*c4(e175),
     * 1/80 = c1>c6(s804),c4*c6(u574),c5*c2(d364),c6*c2(g024)
     *
     * note: * instead of > means static
     * wh to class 1,2,3,4 have 16h lifetime
     * others have 24h lifetime
     */
    sig.sigStrength = 0.1;

    //Destination for non-exit wormholes
    uint32 destSystem = 0; 
    CelestialObjectRef iRef;

    // For exit wormholes (k162)
    if (exitSystemID != 0) {
        GPoint pos(sig.position);
        ItemData wData(30831, sig.ownerID, sig.systemID, flagNone, sig.sigName.c_str(), pos);
        iRef = sItemFactory.SpawnWormhole(wData);
        if (iRef.get() == nullptr)
            return;

        // Set the destination system attribute for the wormhole
        // In this case, exitSystemID is the origin wormhole
        iRef->SetAttribute(AttrWormholeTargetSystem1, exitSystemID);
    // For all other kinds of wormholes
    } else {
        // decide which type of wormhole to create here
        const ItemType* whType = GetRandomWormholeType(sig.systemID);
        if (whType == nullptr)
            _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::Create() - Create Failure, SystemID not in Database %u", sig.systemID);
            return;
        destSystem = GetRandomDestination(whType);
        // create wormhole here
        sig.sigName = whType->name();
        sig.sigTypeID = whType->id();

        GPoint pos(sig.position);
        ItemData wData(whType->id(), sig.ownerID, sig.systemID, flagNone, sig.sigName.c_str(), pos);

        iRef = sItemFactory.SpawnWormhole(wData);
        if (iRef.get() == nullptr)
            return;

        // Set the destination system attribute for the wormhole
        iRef->SetAttribute(AttrWormholeTargetSystem1, destSystem);
    }

    // verify system is loaded
    SystemManager* pSysMgr = sEntityList.FindOrBootSystem(sig.systemID);
    if (pSysMgr == nullptr) {
        _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::Create() - Boot failure for system %u", sig.systemID);
        return;
    }

    // set itemID to return to anomaly mgr after creation succeeds
    sig.sigItemID = iRef->itemID();
    if (exitSystemID != 0) {
        // Set exit wormhole's attributes so it can find its' entrance
        iRef->SetAttribute(AttrWormholeTargetSystem1, exitSourceItemID);
        // Set source wormhole's attributes so it can find its' exit
        CelestialObjectRef sRef;
        sRef = sItemFactory.GetCelestialRef(exitSourceItemID);
        sRef->SetAttribute(AttrWormholeTargetSystem2, sig.sigItemID);
        sRef->SaveItem();
        // Set exit wormhole's attributes based upon source wormhole's attributes
        iRef->SetAttribute(AttrWormholeMassRegeneration, sRef->GetAttribute(AttrWormholeMassRegeneration).get_int());
        iRef->SetAttribute(AttrWormholeTargetSystemClass, sDataMgr.GetWHSystemClass(exitSystemID));
        iRef->SetAttribute(AttrWormholeMaxStableTime, sRef->GetAttribute(AttrWormholeMaxStableTime).get_int());
        iRef->SetAttribute(AttrWormholeMaxStableMass, sRef->GetAttribute(AttrWormholeMaxStableMass).get_int());
        iRef->SetAttribute(AttrWormholeMaxJumpMass, sRef->GetAttribute(AttrWormholeMaxJumpMass).get_int());
    }
    iRef->SaveItem();

    // Reload entity from factory
    WormholeSE* wSE = new WormholeSE(iRef, *(pSysMgr->GetServiceMgr()), pSysMgr);
    if (wSE == nullptr) {
        _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::Create() - SE Create failure for %s(%u)", iRef->name(), iRef->itemID());
        return;
    }

    // add wormhole to system (signal added to AnomalyMgr on successful return)
    pSysMgr->AddEntity(wSE, false);
    sig.bubbleID = wSE->SysBubble()->GetID();
    // add wormhole to vector
    m_wormholes.push_back(iRef->itemID());

    // Call CreateExit() to create an exit wormhole (only if Create() was not called for an exit already)
    if (exitSystemID == 0) {
        if (sEntityList.IsSystemLoaded(destSystem)) {
            SystemManager* pToSys = sEntityList.FindOrBootSystem(destSystem);
            if (pSysMgr == nullptr) {
                _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::Create() - Boot failure for system %u", destSystem);
                return;
            }
            CreateExit(pSysMgr, pToSys, sig.sigItemID);
        } else {
            CreateExit(pSysMgr, destSystem, sig.sigItemID);
        }
    }

    _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::Create() - Created %s in %s(%u) with %.3f%% sigStrength.", \
            iRef->name(), pSysMgr->GetName(), sig.systemID, sig.sigStrength *100);

}

// Create exit for loaded systems
void WormholeMgr::CreateExit(SystemManager* pFromSys, SystemManager* pToSys, uint32 sourceItemID)
{
    // compile data for exit
    CosmicSignature sig = CosmicSignature();
    
    sig.sigID = sEntityList.GetAnomalyID();
    sig.systemID = pToSys->GetID();
    sig.dungeonType = Dungeon::Type::Wormhole;

    sig.sigItemID = 0;
    sig.sigName = "Wormhole K162 ";
    //default to 1/80
    sig.sigStrength = 0.0125;
    sig.sigTypeID = 30831;
    sig.sigGroupID = EVEDB::invGroups::Wormhole;
    sig.scanGroupID = Scanning::Group::Signature;
    sig.scanAttributeID = AttrScanAllStrength;
    sig.ownerID = 1;

    sig.position = sMapData.GetAnomalyPoint(pToSys);

    // send data to Create() for processing
    Create(sig, pFromSys->GetID());

    // Register this exit wormhole with the destination's AnomalyMgr
    pToSys->GetAnomMgr()->RegisterExitWH(sig);

    _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::CreateExit() - Creating Exit(loaded) from %s(%u) to %s(%u)", \
                pFromSys->GetName(), pFromSys->GetID(), pToSys->GetName(), pToSys->GetID());
}

// Create exit for unloaded systems
void WormholeMgr::CreateExit(SystemManager* pFromSys, uint32 exitSystemID, uint32 sourceItemID)
{
    // compile data for exit
    CosmicSignature sig = CosmicSignature();
    
    sig.sigID = sEntityList.GetAnomalyID();
    sig.systemID = exitSystemID;
    sig.dungeonType = Dungeon::Type::Wormhole;

    sig.sigName = "Wormhole K162 ";
    //default to 1/80
    sig.sigStrength = 0.0125;
    sig.sigTypeID = 30831;
    sig.sigGroupID = EVEDB::invGroups::Wormhole;
    sig.scanGroupID = Scanning::Group::Signature;
    sig.scanAttributeID = AttrScanAllStrength;
    sig.ownerID = 1;
    sig.position = sMapData.GetAnomalyPoint(exitSystemID);

    CelestialObjectRef iRef;
    CelestialObjectRef sRef;

    GPoint pos(sig.position);
    ItemData wData(30831, sig.ownerID, sig.systemID, flagNone, sig.sigName.c_str(), pos);
    iRef = sItemFactory.SpawnWormhole(wData);
    if (iRef.get() == nullptr)
        return;
    iRef->SetAttribute(AttrWormholeTargetSystem1, pFromSys->GetID());

    // Set exit wormhole's attributes so it can find its' entrance
    iRef->SetAttribute(AttrWormholeTargetSystem2, sourceItemID);
    // Set exit wormhole's attributes based upon source wormhole's attributes
    sRef = sItemFactory.GetCelestialRef( sourceItemID );
    iRef->SetAttribute(AttrWormholeMassRegeneration, sRef->GetAttribute(AttrWormholeMassRegeneration).get_int());
    iRef->SetAttribute(AttrWormholeTargetSystemClass, sDataMgr.GetWHSystemClass(exitSystemID));
    iRef->SetAttribute(AttrWormholeMaxStableTime, sRef->GetAttribute(AttrWormholeMaxStableTime).get_int());
    iRef->SetAttribute(AttrWormholeMaxStableMass, sRef->GetAttribute(AttrWormholeMaxStableMass).get_int());
    iRef->SetAttribute(AttrWormholeMaxJumpMass, sRef->GetAttribute(AttrWormholeMaxJumpMass).get_int());
    sig.sigItemID = iRef->itemID();
    iRef->SaveItem();

    // Set source wormhole's attributes so it can find its' exit
    sRef->SetAttribute(AttrWormholeTargetSystem2, sig.sigItemID);
    sRef->SaveItem();

    // Save the exit wormhole signature to the database
    m_mdb->SaveAnomaly(sig);

    _log(WORMHOLE_MGR__DEBUG, "WormholeMgr::CreateExit() - Creating Exit(unloaded) from %s(%u) to system(%u)", \
                pFromSys->GetName(), pFromSys->GetID(), exitSystemID);
}

// Pick a random type of wormhole to create based upon the class of the system in question
const ItemType* WormholeMgr::GetRandomWormholeType(uint32 systemID) {
    std::vector<uint32> destTypes = sDataMgr.GetWHDestinationTypes(sDataMgr.GetWHSystemClass(systemID));
    if(destTypes.size()<1) {
        return nullptr
    }
    uint32 typeID = destTypes[MakeRandomInt(0,destTypes.size()-1)];
    return sItemFactory.GetType(typeID);
}

// Pick a random destination of wormhole based upon its typeID
uint32 WormholeMgr::GetRandomDestination(const ItemType* whType) {
    uint8 targetClass = whType->GetAttribute(AttrWormholeTargetSystemClass).get_uint32();
    std::vector<uint32> destSystems = sDataMgr.GetWHClassSystems(targetClass);
    return destSystems[MakeRandomInt(0,destSystems.size()-1)];
}

//SELECT `locationID`, `wormholeClassID` FROM `mapLocationWormholeClasses`

/* attributeID  attributeName   attributeCategory   attributeIdx    description     categoryID
1381    wormholeTargetSystemClass   4   0   Target System Class for wormholes   7
1382    wormholeMaxStableTime   5   0   The maximum amount of time a wormhole will stay open    7
1383    wormholeMaxStableMass   5   0   The maximum amount of mass a wormhole can transit before collapsing     7
1384    wormholeMassRegeneration    5   0   The amount of mass a wormhole regenerates per cycle     7
1385    wormholeMaxJumpMass     5   0   The maximum amount of mass that can transit a wormhole in one go    7
    */

/* WH groupID: 988
 *      70 items in db
 *
 * graphicIDs - 3715 (lt blue, red center)
 *              2017
 *              2013
 *              2010
 *              2009 (dark, single point center with light to rbottom)
 *              2008
 *              2007
 */

/* typeID   typeName    graphicID
30463   Test wormhole   2907
30579   Wormhole Z971   3715
30583   Wormhole R943   3715
30584   Wormhole X702   3715
30642   Wormhole O128   3715
30643   Wormhole N432   3715
30644   Wormhole M555   3715
30645   Wormhole B041   3715
30646   Wormhole U319   3715
30647   Wormhole B449   3715
30648   Wormhole N944   3715
30649   Wormhole S199   3715
30657   Wormhole A641   3715
30658   Wormhole R051   3715
30659   Wormhole V283   3715
30660   Wormhole H121   3715
30661   Wormhole C125   3715
30662   Wormhole O883   3715
30663   Wormhole M609   3715
30664   Wormhole L614   3715
30665   Wormhole S804   3715
30666   Wormhole N110   3715
30667   Wormhole J244   3715
30668   Wormhole Z060   3715
30671   Wormhole Z647   3715
30672   Wormhole D382   3715
30673   Wormhole O477   3715
30674   Wormhole Y683   3715
30675   Wormhole N062   3715
30676   Wormhole R474   3715
30677   Wormhole B274   3715
30678   Wormhole A239   3715
30679   Wormhole E545   3715
30680   Wormhole V301   3715
30681   Wormhole I182   3715
30682   Wormhole N968   3715
30683   Wormhole T405   3715
30684   Wormhole N770   3715
30685   Wormhole A982   3715
30686   Wormhole S047   3715
30687   Wormhole U210   3715
30688   Wormhole K346   3715
30689   Wormhole P060   3715
30690   Wormhole N766   3715
30691   Wormhole C247   3715
30692   Wormhole X877   3715
30693   Wormhole H900   3715
30694   Wormhole U574   3715
30695   Wormhole D845   3715
30696   Wormhole N290   3715
30697   Wormhole K329   3715
30698   Wormhole Y790   3715
30699   Wormhole D364   3715
30700   Wormhole M267   3715
30701   Wormhole E175   3715
30702   Wormhole H296   3715
30703   Wormhole V753   3715
30704   Wormhole D792   3715
30705   Wormhole C140   3715
30706   Wormhole Z142   3715
30707   Wormhole Q317   3715
30708   Wormhole G024   3715
30709   Wormhole L477   3715
30710   Wormhole Z457   3715
30711   Wormhole V911   3715
30712   Wormhole W237   3715
30713   Wormhole B520   3715
30714   Wormhole C391   3715
30715   Wormhole C248   3715
30831   Wormhole K162   3715
*/