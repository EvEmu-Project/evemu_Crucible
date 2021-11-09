
 /**
  * @name BeltMgr.cpp
  *     Asteroid Belt Spawn management system for EVEmu
  *
  * @Author:        Allan
  * @date:          15 April 2016
  *
  */

 /** @note  roid save/load uses InventoryItem::Load(), which saves/reads from entity table, which we DONT want for roids.
  * for now, save and load are disabled, roids are temp items, and use generic InventoryItem class.
  * this will have to be revisited and corrected to properly implement persistent roids/belts
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "math/Trig.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"


BeltMgr::BeltMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_respawnTimer(0),
m_system(mgr),
m_services(svc),
m_initialized(false)
{
    assert(m_system != nullptr);

    m_belts.clear();
    m_active.clear();
    m_spawned.clear();
}

void BeltMgr::Init()
{
    if (!sConfig.cosmic.BeltEnabled) {
        _log(COSMIC_MGR__INIT, "BeltMgr Disabled.  Not Initializing Belt Manager for %s(%u)", m_system->GetName(), m_system->GetID());
        return;
    }

    m_belts.clear();
    m_active.clear();
    m_spawned.clear();

    m_respawnTimer.Start(sConfig.cosmic.BeltRespawn *60 *60 *1000);  // hours->ms

    m_initialized = true;
    _log(COSMIC_MGR__INIT, "BeltMgr Initialized for %s(%u)", m_system->GetName(), m_system->GetID());
}

void BeltMgr::RegisterBelt(InventoryItemRef itemRef)
{
    uint32 beltID = itemRef->itemID();
    m_belts.insert(std::pair<uint32, InventoryItemRef>(beltID, itemRef));
    m_active.insert(std::pair<uint32, bool>(beltID, false));
    m_spawned.insert(std::pair<uint32, bool>(beltID, false));
}

//not used
void BeltMgr::ClearBelt(uint16 bubbleID)
{
    //ClearAll();
}

void BeltMgr::ClearAll() {
    Save();
    for (auto cur : m_asteroids) {
        m_system->RemoveEntity(cur.second);
        //cur.second->Delete();
        SafeDelete(cur.second);
    }
    m_asteroids.clear();
    m_belts.clear();
}

void BeltMgr::CheckSpawn(uint16 bubbleID)
{
    if (IsSpawned(bubbleID))
        return;
    /*  if there are already roids created for this belt, they will be loaded in Load()
     * if Load() has roids for this belt, this belt will have true already set, and checked in SpawnBelt()
     */
    if (!Load(bubbleID)) {
        std::unordered_multimap<float, uint16> roidTypes;
        roidTypes.clear();
        SpawnBelt(bubbleID, roidTypes);
    }
}

bool BeltMgr::IsSpawned(uint16 bubbleID)
{
    uint32 beltID = sBubbleMgr.GetBeltID(bubbleID);
    std::map<uint32, bool>::iterator itr = m_spawned.find(beltID);
    if (itr != m_spawned.end())
        return itr->second;
    return false;
}

bool BeltMgr::IsActive(uint16 bubbleID)
{
    uint32 beltID = sBubbleMgr.GetBeltID(bubbleID);
    std::map<uint32, bool>::iterator itr = m_active.find(beltID);
    if (itr != m_active.end())
        return itr->second;
    return false;
}

void BeltMgr::SetActive(uint16 bubbleID, bool active/*true*/)
{
    uint32 beltID = sBubbleMgr.GetBeltID(bubbleID);
    std::map<uint32, bool>::iterator itr = m_active.find(beltID);
    if (itr != m_active.end()) {
        itr->second = active;
    } else {
        m_active.insert(std::pair<uint32, bool>(beltID, active));
    }
}

void BeltMgr::Process() {
    if (!m_initialized)
        return;

    if (m_respawnTimer.Check()) {
        for (auto cur : m_spawned)
            if (!cur.second) {
                std::unordered_multimap<float, uint16> roidTypes;
                roidTypes.clear();
                SpawnBelt(cur.first, roidTypes);
            }
    }
}

bool BeltMgr::Load(uint16 bubbleID) {
    std::vector<AsteroidData> entities;
    entities.clear();
    uint32 beltID = sBubbleMgr.GetBeltID(bubbleID);
    if (beltID == 0)
        return false;
    if (!ManagerDB::LoadSystemRoids(m_system->GetID(), beltID, entities))
        return false;

    for (auto entity : entities) {
        AsteroidItemRef itemRef = sItemFactory.GetAsteroidRef(entity.itemID);
        if (itemRef.get() == nullptr) {
            _log(COSMIC_MGR__WARNING, "BeltMgr::Load() -  Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID);
            continue;
        }
        // set attribs using loaded values from asteroid table.
        itemRef->SetAttribute(AttrRadius,    itemRef->type().radius() * entity.radius); // Radius
        itemRef->SetAttribute(AttrQuantity,  entity.quantity);                          // Quantity
        itemRef->SetAttribute(AttrVolume,    itemRef->type().volume());                 // Volume
        itemRef->SetAttribute(AttrMass,      itemRef->type().mass() * entity.quantity); // Mass

        AsteroidSE* pASE = new AsteroidSE(itemRef, *(m_system->GetServiceMgr()), m_system );
        if (pASE == nullptr) {
            _log(COSMIC_MGR__WARNING, "BeltMgr::Load() -  Unable to spawn itemID %u - %s (type %u).", entity.itemID, entity.itemName.c_str(), entity.typeID);
            continue;
        }
        _log(COSMIC_MGR__TRACE, "BeltMgr::Load() - Loaded asteroid %u, type %u for %s(%u)", entity.itemID, entity.typeID, m_system->GetName(), m_system->GetID() );
        m_system->AddEntity(pASE);
        m_asteroids.emplace(std::pair<uint32, AsteroidSE*>(beltID, pASE));
        pASE->SetMgr(this, beltID);
    }
    std::map<uint32, bool>::iterator itr = m_spawned.find(beltID);
    if (itr == m_spawned.end()) {
        m_spawned.insert(std::pair<uint32, bool>(beltID, true));
    } else {
        itr->second = true;
    }

    itr = m_active.find(beltID);
    if (itr == m_active.end()) {
        m_active.insert(std::pair<uint32, bool>(beltID, true));
    } else {
        itr->second = true;
    }

    return true;
}

void BeltMgr::Save() {
    if (m_asteroids.empty()) {
        _log(COSMIC_MGR__TRACE, "BeltMgr::Save - m_asteroids is empty for %s(%u).  nothing to save.", m_system->GetName(), m_system->GetID());
        return;
    }

    double start = GetTimeUSeconds();
    std::vector<AsteroidData> roids;
    roids.clear();
    uint16 save(0), skip(0);
    for (auto cur : m_asteroids) {
        // we are not saving anomaly belts (yet.  small belts in anomalies will become their own grav sites (wip))
        if (IsTempItem(cur.first)) {
            ++skip;
            continue;
        }
        AsteroidData entry = AsteroidData();
        entry.itemID = cur.second->GetID();
        entry.itemName = cur.second->GetName();
        entry.typeID = cur.second->GetSelf()->typeID();
        entry.systemID = m_system->GetID();
        entry.beltID = cur.first;
        entry.radius = cur.second->GetRadius();
        entry.quantity = ((25000 * log(entry.radius)) - 112404.8);   // quantity in m^3
        entry.position = cur.second->GetPosition();
        roids.push_back(entry);
        ++save;
    }

    ManagerDB::SaveSystemRoids(m_system->GetID(), roids);
    _log(COSMIC_MGR__TRACE, "BeltMgr::Save - Saving %u Asteroids for %s(%u) took %.3fus.  Skipped %u temp anomaly asteroids.", \
            save, m_system->GetName(), m_system->GetID(), (GetTimeUSeconds() - start), skip);
}

void BeltMgr::GetList(uint32 beltID, std::vector< AsteroidSE* >& list)
{
    auto range = m_asteroids.equal_range(beltID);
    for (auto itr = range.first; itr != range.second; ++itr)
        list.push_back(itr->second);
}

    /*
struct CosmicSignature {
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    GPoint position;
};
*/

bool BeltMgr::Create(CosmicSignature& sig, std::unordered_multimap<float, uint16>& roidTypes)
{
    // register this as a belt.
    SystemEntity* pSE = m_system->GetSE(sig.sigItemID);
    if (pSE == nullptr)
        return false;
    pSE->SysBubble()->SetBelt(pSE->GetSelf());
    SpawnBelt(pSE->SysBubble()->GetID(), roidTypes, 0, true);
    return true;
}

void BeltMgr::SpawnBelt(uint16 bubbleID, std::unordered_multimap<float, uint16>& roidTypes, int type/*0*/, bool anomaly/*false*/)
{
    if (IsSpawned(bubbleID))
        return;

    uint32 beltID = sBubbleMgr.GetBeltID(bubbleID);
    if ((!IsCelestialID(beltID)) and (!anomaly))
        return;

    SystemEntity* pSE = m_system->GetSE(beltID);
    if (pSE == nullptr)
        return;

    bool ice = (pSE->GetTypeID() == 17774);

    float secRating = m_system->GetSystemSecurityRating();
    float secValue = m_system->GetSecValue();
    std::unordered_multimap<float, uint16> roidDist;
    if (ice) {
        // caldari=1, minmatar=2, amarr=3, gallente=4, none=5
        GetIceDist(sDataMgr.GetRegionQuarter(m_system->GetRegionID()), secRating, roidDist);
    } else if (anomaly) {
        roidDist = roidTypes;
    } else {
        sDataMgr.GetRoidDist(m_system->GetSystemSecurityClass(), roidDist);
    }

    int8 pcs = 5;
    double radius = 8000;
    radius *= sConfig.cosmic.roidRadiusMultiplier;
    double roidradius = 0, theta = 0, randRadius = 0, elevation = 0;
    if (anomaly) {
        pcs += roidDist.size();
        radius += (radius * secValue);
        radius += (pcs * 1000 / 4);
        elevation = (radius / 4);
    } else if (ice) {  //880 total systems with ice. 293 in hisec
        //  ice needs to be 30k to 75k, with radius of 40k to 100k
        radius *= 3; //24k base
        if (secRating > 0.7) {
            pcs = 1;
        } else if (secRating > 0.4) {
            pcs = 2;
        } else if (secRating > 0.0) {
            pcs = 3;
        } else if (secRating > -0.4) {
            pcs = 4;
        } else {
            pcs = 6;
        }
    } else {
        pcs += MakeRandomInt(5, 30);
        radius += (radius * secValue);
        radius += (pcs * 1000 / 4);
        elevation = (radius / 4);
    }

    double degreeSeparation = (180 / pcs);
    ++pcs;
    GPoint center(pSE->SysBubble()->GetCenter());
    GPoint mposition(NULL_ORIGIN);
    for (uint8 i = 1; i < pcs; ++i) {
        if (ice) {
            if (secRating > -0.2) {
                roidradius = MakeRandomInt(20, 40) * 1000; // (40k,70k)  72-102k radius
            } else {
                roidradius = MakeRandomInt(40, 70) * 1000; // (40k,80k)  82-112k radius
            }
            radius += roidradius;
            elevation = (radius + (roidradius / 2) / 2);
        } else {
            roidradius = MakeRandomInt(3000, 8000) * secValue;
        }
        if (anomaly) {
            // random for anomaly...use the "special" placements at end of this file....eventually.
            theta = MakeRandomFloat(0, (EvE::Trig::Pi*2));
            mposition.x = (radius + roidradius / 5) * cos(theta);
            mposition.z = (radius + roidradius / 5) * sin(theta);
        } else if (type == 0) {
            randRadius = MakeRandomFloat(-radius / 4, radius / 2);
            // half-circle type
            theta = EvE::Trig::Deg2Rad(degreeSeparation * i);
            mposition.x = (randRadius + roidradius + radius) * cos(theta);
            mposition.z = (randRadius + roidradius + radius) * sin(theta);
        }
        mposition.y = MakeRandomFloat(-elevation, elevation);
        SpawnAsteroid(beltID, GetAsteroidType(MakeRandomFloat(), roidDist), roidradius, (center + mposition), ice);
    }

    std::map<uint32, bool>::iterator itr = m_spawned.find(beltID);
    if (itr == m_spawned.end()) {
        m_spawned.insert(std::pair<uint32, bool>(beltID, true));
    } else {
        itr->second = true;
    }

    itr = m_active.find(beltID);
    if (itr == m_active.end()) {
        m_active.insert(std::pair<uint32, bool>(beltID, false));
    } else {
        itr->second = false;
    }

    _log(COSMIC_MGR__TRACE, "BeltMgr::SpawnBelt - Belt spawned with %u roids of %s in %s %u for %s(%u)", \
            pcs, (ice?"ice":"ore"), (anomaly?"anomalyID":"beltID"), beltID, m_system->GetName(), m_system->GetID() );
}

uint32 BeltMgr::GetAsteroidType(double p, const std::unordered_multimap<float, uint16>& roids) {
    std::unordered_multimap<float, uint16>::const_iterator cur = roids.begin();
    float chance = 0.0;
    for(; cur != roids.end(); ++cur ) {
        chance += cur->first;
        _log(COSMIC_MGR__DEBUG, "BeltMgr::GetAsteroidType - checking %u with chance %.3f(%.3f)", cur->second, chance, p);
        if (chance > p )
            return cur->second;
    }

    return 0;
}

void BeltMgr::SpawnAsteroid(uint32 beltID, uint32 typeID, double radius, const GPoint& position, bool ice/*false*/) {
    if (typeID == 0) {
        _log(COSMIC_MGR__WARNING, "BeltMgr::SpawnAsteroid - typeID is 0");
        return;
    }
    if (IsNaN(radius)) {
        _log(COSMIC_MGR__WARNING, "BeltMgr::SpawnAsteroid - radius is NaN");
        return;
    }
    if (radius <= 0) {
        _log(COSMIC_MGR__WARNING, "BeltMgr::SpawnAsteroid - radius < 0");
        return;
    }

    double quantity = 0;
    if (ice) {
        quantity = radius * 2;
    } else {
        radius *= sConfig.cosmic.roidRadiusMultiplier;
        //Amount of Ore = (25000*ln(Radius))-112404.8   V = 25000Ln(r) - 112407
        quantity = ((25000 * log(radius)) - 112404.8);
    }

    AsteroidData adata = AsteroidData();
        adata.beltID = beltID;
        adata.systemID = m_system->GetID();
        adata.typeID = typeID;
        adata.quantity = quantity;
        adata.radius = radius;
        adata.position = position;
    ItemData idata(typeID, ownerSystem, m_system->GetID(), flagNone, "", position);
    InventoryItemRef iRef(nullptr);
    if (IsTempItem(beltID)) {
        iRef = AsteroidItem::SpawnTemp(idata, adata);   // create temp item for anomaly belt
    } else {
        iRef = sItemFactory.SpawnAsteroid(idata, adata);
    }
    if (iRef.get() == nullptr)
        return;

    AsteroidSE* pASE = new AsteroidSE( iRef, *(m_system->GetServiceMgr()), m_system );
    if (pASE == nullptr)
        return;

    m_asteroids.emplace(std::pair<uint32, AsteroidSE*>(beltID, pASE));
    pASE->SetMgr(this, beltID);
    m_system->AddEntity(pASE, false);   // we're not adding roids to signal list
}

void BeltMgr::RemoveAsteroid(uint32 beltID, AsteroidSE* pASE)
{
    ManagerDB::RemoveAsteroid(pASE->GetID());
    // this doesnt work right.  not sure why yet.
    auto range = m_asteroids.equal_range(beltID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        if (itr->second == pASE) {
            m_asteroids.erase(itr);
            return;
        }
    }
/*
    if (m_asteroids.count(beltID)) {
        std::map<uint32, bool>::iterator itr =  m_spawned.find(beltID);
        if (itr != m_spawned.end())
            itr->second = false;
        else
            m_spawned.insert(std::pair<uint32, bool>(beltID, false));
    }
*/
}

void BeltMgr::GetIceDist(uint8 quarter, float secStatus, std::unordered_multimap<float, uint16>& roidDist)
{
    // get map region for ice distro
    switch (quarter) {
        case 1: {       // caldari
            if (secStatus < 0.0) {
                roidDist.insert(std::pair<float,uint32>(0.2, 16265));    // White Glaze - caldari
                roidDist.insert(std::pair<float,uint32>(0.16, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.16, 16267));   // Dark Glitter - all but gallente < 0.1
                roidDist.insert(std::pair<float,uint32>(0.16, 17976));   // Pristine White Glaze - caldari < 0.0
                roidDist.insert(std::pair<float,uint32>(0.16, 16268));   // Gelidus - all < 0.0
                roidDist.insert(std::pair<float,uint32>(0.16, 16269));   // Krystallos - all < 0.0
            } else if (secStatus < 0.1) {
                roidDist.insert(std::pair<float,uint32>(0.70, 16265));   // White Glaze - caldari
                roidDist.insert(std::pair<float,uint32>(0.20, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.10, 16267));   // Dark Glitter - all but gallente < 0.1
            } else if (secStatus < 0.4) {
                roidDist.insert(std::pair<float,uint32>(0.75, 16265));   // White Glaze - caldari
                roidDist.insert(std::pair<float,uint32>(0.25, 16266));   // Glare Crust - all < 0.4
            } else {
                roidDist.insert(std::pair<float,uint32>(1.0, 16265));    // White Glaze - caldari
            }
        } break;
        case 2: {       // minmatar
            if (secStatus < 0.0) {
                roidDist.insert(std::pair<float,uint32>(0.2, 16263));    // Glacial Mass - minmatar
                roidDist.insert(std::pair<float,uint32>(0.16, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.16, 16267));   // Dark Glitter - all but gallente < 0.1
                roidDist.insert(std::pair<float,uint32>(0.16, 17977));   // Smooth Glacial Mass - minmatar < 0.0
                roidDist.insert(std::pair<float,uint32>(0.16, 16268));   // Gelidus - all < 0.0
                roidDist.insert(std::pair<float,uint32>(0.16, 16269));   // Krystallos - all < 0.0
            } else if (secStatus < 0.1) {
                roidDist.insert(std::pair<float,uint32>(0.70, 16263));   // Glacial Mass - minmatar
                roidDist.insert(std::pair<float,uint32>(0.20, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.10, 16267));   // Dark Glitter - all but gallente < 0.1
            } else if (secStatus < 0.4) {
                roidDist.insert(std::pair<float,uint32>(0.75, 16263));   // Glacial Mass - minmatar
                roidDist.insert(std::pair<float,uint32>(0.25, 16266));   // Glare Crust - all < 0.4
            } else {
                roidDist.insert(std::pair<float,uint32>(1.0, 16263));    // Glacial Mass - minmatar
            }
        } break;
        case 3: {       //amarr
            if (secStatus < 0.0) {
                roidDist.insert(std::pair<float,uint32>(0.2, 16262));    // Clear Icicle - amarr
                roidDist.insert(std::pair<float,uint32>(0.16, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.16, 16267));   // Dark Glitter - all but gallente < 0.1
                roidDist.insert(std::pair<float,uint32>(0.16, 17978));   // Enriched Clear Icicle - amarr < 0.0
                roidDist.insert(std::pair<float,uint32>(0.16, 16268));   // Gelidus - all < 0.0
                roidDist.insert(std::pair<float,uint32>(0.16, 16269));   // Krystallos - all < 0.0
            } else if (secStatus < 0.1) {
                roidDist.insert(std::pair<float,uint32>(0.70, 16262));   // Clear Icicle - amarr
                roidDist.insert(std::pair<float,uint32>(0.20, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.10, 16267));   // Dark Glitter - all but gallente < 0.1
            } else if (secStatus < 0.4) {
                roidDist.insert(std::pair<float,uint32>(0.75, 16262));   // Clear Icicle - amarr
                roidDist.insert(std::pair<float,uint32>(0.25, 16266));   // Glare Crust - all < 0.4
            } else {
                roidDist.insert(std::pair<float,uint32>(1.0, 16262));    // Clear Icicle - amarr
            }
        } break;
        case 4: {       //gallente
            if (secStatus < 0.0) {
                roidDist.insert(std::pair<float,uint32>(0.3, 16264));    // Blue Ice - gallente
                roidDist.insert(std::pair<float,uint32>(0.17, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.17, 17975));   // Thick Blue Ice - gallente < 0.0
                roidDist.insert(std::pair<float,uint32>(0.17, 16268));   // Gelidus - all < 0.0
                roidDist.insert(std::pair<float,uint32>(0.17, 16269));   // Krystallos - all < 0.0
            } else if (secStatus < 0.4) {
                roidDist.insert(std::pair<float,uint32>(0.75, 16264));   // Blue Ice - gallente
                roidDist.insert(std::pair<float,uint32>(0.25, 16266));   // Glare Crust - all < 0.4
            } else {
                roidDist.insert(std::pair<float,uint32>(1.0, 16264));    // Blue Ice - gallente
            }
        } break;
        case 5: {       // none?  no region quarter?
            if (secStatus < 0.0) {
                roidDist.insert(std::pair<float,uint32>(0.4, 16266));    // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.2, 16267));    // Dark Glitter - all but gallente < 0.1
                roidDist.insert(std::pair<float,uint32>(0.2, 16268));    // Gelidus - all < 0.0
                roidDist.insert(std::pair<float,uint32>(0.2, 16269));    // Krystallos - all < 0.0
            } else if (secStatus < 0.1) {
                roidDist.insert(std::pair<float,uint32>(0.75, 16266));   // Glare Crust - all < 0.4
                roidDist.insert(std::pair<float,uint32>(0.25, 16267));   // Dark Glitter - all but gallente < 0.1
            } else if (secStatus < 0.4) {
                roidDist.insert(std::pair<float,uint32>(1.0, 16266));    // Glare Crust - all < 0.4
            }
        } break;
    }
}

/*          this gives random single point on sphere with radius of 'r'
 *
        double theta = MakeRandomFloat(0.0, (2*M_PI) );
        double phi = MakeRandomFloat(0.0, (2*M_PI) );
        x += r * sin(theta) * cos(phi);
        y += r * sin(theta) * sin(phi);
        z += r * cos(theta);

*/
/*          straight line from center
 *
        theta += angleSeperation;
        double theta = MakeRandomFloat(0.0, (theta) );
        double phi = MakeRandomFloat(0.0, (2*M_PI) );
        mposition.x += radius * sin(theta) * cos(phi);
        mposition.y += radius * sin(theta) * sin(phi);
        mposition.z += radius * cos(theta);
    */

/*      up-an-over arc from center
 *
        theta += angleSeperation;
        double theta = MakeRandomFloat(0.0, (theta) );
        double phi = MakeRandomFloat(0.0, (2*M_PI) );
        mposition.x += radius * sin(theta) * cos(phi);
        mposition.y += radius * sin(theta) * sin(phi);
        mposition.z += radius * cos(theta);
*/
/*      this makes triple helix design      -allan 7May15
 *
    int8 pcs = 0, rand = MakeRandomInt(-10, 20 );
    if (customCount > 15 )
        pcs = customCount + rand;
    else
        pcs = 30 + rand;

    float radius = 15000;
    const double security = 1.1 - m_db.GetSecurity(sys->GetID());
    radius += (radius * security) *2;

    GPoint mposition = NULL_ORIGIN;
    const GPoint position(who->SysBubble()->GetCenter() );
    double roidradius = 0, thefloat = 0;
    uint32 theta = 0, angleSeperation = floor(180 /pcs);

    for (uint32 i = 0; i < pcs; ++i) {
        roidradius = 3000;

        if (makeIceBelt) {
            thefloat = MakeRandomFloat(1.0, 5.0 );
            roidradius += thefloat * 1000;
            roidradius *= 5;
        } else {
            thefloat = MakeRandomFloat(1.0, 7.0 );
            roidradius += thefloat * 1000;
        }
        roidradius *= security;

        theta += angleSeperation;
        mposition.x = cos(theta) *radius + (roidradius *2) + MakeRandomFloat(-1000, 5000 );
        mposition.z = sin(theta) *radius + (roidradius *2) + MakeRandomFloat(-3000, 5000 );
        mposition.y += MakeRandomFloat(-2000, 3000 ) + roidradius;

        SpawnAsteroid(sys, GetAsteroidType(MakeRandomFloat(), roidDist ), roidradius, position + mposition );
        sLog.Warning("Command_spawnbelt", "roidradius: %.2f, mpos: %.2f, %.2f, %.2f, angleSeperation: %u, theta: %u",
                        roidradius, mposition.x, mposition.y, mposition.z, angleSeperation, theta );
    }

*/
/*
    for (uint32 i = 0; i < pcs; ++i) {
        roidradius = MakeRandomFloat(1.0, 7.0 ) *1000 *security +secRad;
        theta = MakeRandomFloat(0.0, M_PI );
        phi = MakeRandomFloat(0.0, M_PI );
        mposition.x = radius * sin(theta) * cos(phi);
        mposition.z = radius * sin(theta) * sin(phi);
        mposition.y = MakeRandomInt(-security, security) *100;
*/

    /*
     *  triple helix
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        theta += angleSeperation;
        mposition.x = cos(theta) *radius + (roidradius *2) + MakeRandomFloat(-1000, 5000 );
        mposition.z = sin(theta) *radius + (roidradius *2) + MakeRandomFloat(-3000, 5000 );
        mposition.y += MakeRandomFloat(-2000, 3000 ) + roidradius;

        TODO:  make neocom note "there is odd vortex here..", then make ships drift!  (and update position)
     */
    /*
     *  flat circle @ 50k
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        theta += angleSeperation;
        mposition.x = radius * cos(theta);
        mposition.z = radius * sin(theta);
     */
    /*
     *   ufo style
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        phi += angleSeperation;
        if (phi > 90)
            theta = phi - 180;
        else
            theta = phi;

        mposition.x = radius * sin(theta) + (roidradius /3);
        mposition.z = radius * cos(phi) + (roidradius /3);
        mposition.y = MakeRandomInt(-secRad, secRad);
    */
    /*
     * diagonal
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        phi += angleSeperation;
        if (phi > 90)
            theta = phi - 180;
        else
            theta = phi;

        mposition.x = radius * sin(theta);// + (roidradius /3);
        mposition.z = radius * cos(phi);// + (roidradius /3);
        mposition.y = MakeRandomInt(-secRad, secRad);
    */

    /* idea...
     * huge rock in center, with smaller rocks in 'orbit' at various angles
     */