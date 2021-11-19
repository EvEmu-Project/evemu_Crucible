
/**
 * @name StaticDataMgr.cpp
 *   memory object caching system for retrieving, managing and saving ingame data
 *
 * @Author:         Allan
 * @date:   1Jul15 / 1Aug16
 *
 * Original Idea  - 1 July 15
 * Code completion and implementation  - 1 August 2016
 *
 */


#include "../eve-common/EVE_Character.h"
#include "../eve-common/EVE_POS.h"

#include "StaticDataMgr.h"
#include "EVEServerConfig.h"
#include "database/EVEDBUtils.h"
#include "manufacturing/FactoryDB.h"
#include "map/MapDB.h"
#include "station/StationDB.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"

/*
 * DATA__ERROR          # specific "data not found but should be there" msgs
 * DATA__WARNING        # misc "data not found but nbd" msgs
 * DATA__MESSAGE        # misc data msgs (mt)
 * DATA__INFO           # data loading msgs (container and amount) (mt)
 */

StaticDataMgr::StaticDataMgr()
: m_keyMap(nullptr),
m_agents(nullptr),
m_operands(nullptr),
m_billTypes(nullptr),
m_entryTypes(nullptr),
m_factionInfo(nullptr),
m_npcDivisions(nullptr)
{
    m_ramReq.clear();
    m_moonGoo.clear();
    m_ramMatl.clear();
    m_regions.clear();
    m_attrTypeData.clear();
    m_minerals.clear();
    m_compounds.clear();
    m_bpMatlData.clear();
    m_systemData.clear();
    m_solSysData.clear();
    m_staticData.clear();
    m_salvageMap.clear();
    m_agentSystem.clear();
    m_corpFaction.clear();
    m_typeAttrMap.clear();
    m_LootGroupMap.clear();
    m_stationCount.clear();
    m_stationConst.clear();
    m_stationRegion.clear();
    m_stationSystem.clear();
    m_oreBySecClass.clear();
    m_LootGroupTypeMap.clear();
    m_WrecksToTypesMap.clear();
}

StaticDataMgr::~StaticDataMgr()
{
    //Clear();
}

void StaticDataMgr::Close()
{
    PySafeDecRef(m_keyMap);
    PySafeDecRef(m_agents);
    PySafeDecRef(m_operands);
    PySafeDecRef(m_billTypes);
    PySafeDecRef(m_entryTypes);
    PySafeDecRef(m_factionInfo);
    PySafeDecRef(m_npcDivisions);

    for (auto cur : m_bpMatlData)
        PySafeDecRef(cur.second);

    sLog.Warning("    StaticDataMgr", "Static Data Manager has been closed.");
}


int StaticDataMgr::Initialize()
{
    Populate();
    sLog.Blue("    StaticDataMgr", "Static Data Manager Initialized.");
    return 1;
}

void StaticDataMgr::Clear()
{
    m_ramReq.clear();
    m_moonGoo.clear();
    m_ramMatl.clear();
    m_regions.clear();
    m_attrTypeData.clear();
    m_minerals.clear();
    m_compounds.clear();
    m_bpMatlData.clear();
    m_systemData.clear();
    m_staticData.clear();
    m_salvageMap.clear();
    m_agentSystem.clear();
    m_corpFaction.clear();
    m_typeAttrMap.clear();
    m_LootGroupMap.clear();
    m_stationCount.clear();
    m_stationConst.clear();
    m_stationRegion.clear();
    m_stationSystem.clear();
    m_oreBySecClass.clear();
    m_LootGroupTypeMap.clear();
    m_WrecksToTypesMap.clear();

    PySafeDecRef(m_keyMap);
    PySafeDecRef(m_agents);
    PySafeDecRef(m_operands);
    PySafeDecRef(m_billTypes);
    PySafeDecRef(m_entryTypes);
    PySafeDecRef(m_factionInfo);
    PySafeDecRef(m_npcDivisions);

    for (auto cur : m_bpMatlData)
        PySafeDecRef(cur.second);
    m_bpMatlData.clear();
}

void StaticDataMgr::Populate()
{
    double beginTime(GetTimeMSeconds());
    double startTime(GetTimeMSeconds());

    m_keyMap = ManagerDB::GetKeyMap();
    if (m_keyMap == nullptr)
        sLog.Error("    StaticDataMgr", "m_keyMap is null");

    m_agents = ManagerDB::GetAgents();
    if (m_agents == nullptr)
        sLog.Error("    StaticDataMgr", "m_agents is null");

    m_operands = ManagerDB::GetOperands();
    if (m_operands == nullptr)
        sLog.Error("    StaticDataMgr", "m_operands is null");

    m_billTypes = ManagerDB::GetBillTypes();
    if (m_billTypes == nullptr)
        sLog.Error("    StaticDataMgr", "m_billTypes is null");

    m_entryTypes = ManagerDB::GetEntryTypes();
    if (m_entryTypes == nullptr)
        sLog.Error("    StaticDataMgr", "m_entryTypes is null");

    m_npcDivisions = ManagerDB::GetNPCDivisions();
    if (m_npcDivisions == nullptr)
        sLog.Error("    StaticDataMgr", "m_npcDivisions is null");

    sLog.Cyan("    StaticDataMgr", "Base Static data sets loaded in %.3fms.", (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    GetFactionInfoRsp rsp;
    ManagerDB::LoadCorpFactions(rsp.factionIDbyNPCCorpID);
    ManagerDB::LoadFactionStationCounts(rsp.factionStationCount);
    ManagerDB::LoadFactionSystemCounts(rsp.factionSolarSystemCount);
    ManagerDB::LoadFactionRegions(rsp.factionRegions);
    ManagerDB::LoadFactionConstellations(rsp.factionConstellations);
    ManagerDB::LoadFactionSolarSystems(rsp.factionSolarSystems);
    ManagerDB::LoadFactionRaces(rsp.factionRaces);
    rsp.npcCorpInfo = ManagerDB::LoadNPCCorpInfo();
    m_factionInfo = rsp.Encode();
    if (m_factionInfo == nullptr) {
        sLog.Error("    StaticDataMgr", "m_factionInfo is null");
    } else {
        sLog.Cyan("    StaticDataMgr", "Faction data sets loaded in %.3fms.", (GetTimeMSeconds() - startTime));
    }

    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    startTime = GetTimeMSeconds();
    ManagerDB::LoadNPCCorpFactionData(*res);
    while (res->GetRow(row)) {
        //SELECT corporationID, factionID FROM crpNPCCorporations
        m_corpFaction.emplace(row.GetUInt(0), row.GetUInt(1));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Corps in NPC Corp Faction map loaded in %.3fms.", m_corpFaction.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetCategoryData(*res);
    while (res->GetRow(row)) {
        //SELECT categoryID, categoryName, description, published FROM invCategories
        Inv::CatData data       = Inv::CatData();
            data.id             = row.GetUInt(0);
            data.name           = row.GetText(1);
            data.description    = row.GetText(2);
            data.published      = (sConfig.server.AllowNonPublished ? true : row.GetBool(3));
        m_catData.emplace(row.GetUInt(0), data);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Inventory Categories loaded in %.3fms.", m_catData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetGroupData(*res);
    while (res->GetRow(row)) {
        //SELECT groupID, categoryID, groupName, description, useBasePrice, allowManufacture, allowRecycler,
        //  anchored, anchorable, fittableNonSingleton, published FROM invGroups
        Inv::GrpData data               = Inv::GrpData();
            data.id                     = row.GetUInt(0);
            data.catID                  = row.GetUInt(1);
            data.name                   = row.GetText(2);
            data.description            = row.GetText(3);
            data.useBasePrice           = row.GetBool(4);
            data.allowManufacture       = row.GetBool(5);
            data.allowRecycler          = row.GetBool(6);
            data.anchored               = row.GetBool(7);
            data.anchorable             = row.GetBool(8);
            data.fittableNonSingleton   = row.GetBool(9);
            data.published              = (sConfig.server.AllowNonPublished ? true : row.GetBool(10));
        m_grpData.emplace(row.GetUInt(0), data);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Inventory Groups loaded in %.3fms.", m_grpData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetTypeData(*res);
    while (res->GetRow(row)) {
        Inv::TypeData data              = Inv::TypeData();
            data.id                     = row.GetUInt(0);
            data.groupID                = row.GetUInt(1);
            data.name                   = row.GetText(2);
            data.description            = row.GetText(3);
            data.radius                 = row.GetFloat(4);
            data.mass                   = row.GetFloat(5);
            data.volume                 = row.GetFloat(6);
            data.capacity               = row.GetFloat(7);
            data.portionSize            = row.GetUInt(8);
            data.race                   = row.GetUInt(9);
            data.basePrice              = row.GetDouble(10);
            data.published              = (sConfig.server.AllowNonPublished ? true : row.GetBool(11));
            data.marketGroupID          = (row.IsNull(11) ? 0 : row.GetUInt(12));
            data.chanceOfDuplicating    = row.GetFloat(13);
            data.metaLvl                = (row.IsNull(14) ? 0 : row.GetUInt(14));
            // these will take a bit of work, but will eliminate multiple db hits on inventory/menu loading ingame
            data.isRecyclable           = FactoryDB::IsRecyclable(data.id);   // +5s to startup
            data.isRefinable            = FactoryDB::IsRefinable(data.id);     // +3s to startup
        m_typeData.emplace(row.GetUInt(0), data);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Inventory Types loaded in %.3fms.", m_typeData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetAttributeTypes(*res);
    while (res->GetRow(row)) {
        //SELECT attributeID, attributeName, attributeCategory, displayName, categoryID FROM dgmAttribute
        AttrTypeData typeData               = AttrTypeData();
        typeData.attributeID            = row.GetInt(0);
        typeData.attributeName          = (row.IsNull(1) ? "*none*" : row.GetText(1));
        typeData.attributeCategory      = (row.IsNull(2) ? 0        : row.GetInt(2));
        typeData.displayName            = (row.IsNull(3) ? "*none*" : row.GetText(3));
        typeData.categoryID             = (row.IsNull(4) ? 0        : row.GetInt(4));
        m_attrTypeData.emplace(row.GetInt(0), typeData);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Attribute data sets loaded in %.3fms.", m_attrTypeData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetSystemData(*res);
    while (res->GetRow(row)) {
        //SELECT solarSystemID, solarSystemName, constellationID, regionID, securityClass, security FROM mapSolarSystems
        SystemData sysData        = SystemData();
        sysData.systemID          = row.GetInt(0);
        sysData.name              = row.GetText(1);
        sysData.constellationID   = row.GetInt(2);
        sysData.regionID          = row.GetInt(3);
        sysData.securityClass     = (row.IsNull(4) ? "0" : row.GetText(4));
        sysData.securityRating    = row.GetFloat(5);    // this gives system trueSec
        sysData.factionID         = (row.IsNull(6) ? 0 : row.GetUInt(6));
        m_systemData.emplace(row.GetInt(0), sysData);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Static System data sets loaded in %.3fms.", m_systemData.size(), (GetTimeMSeconds() - startTime));
/*
    startTime = GetTimeMSeconds();
    ManagerDB::GetSolarSystemData(*res);
    while (res->GetRow(row)) {
        //SELECT solarSystemID, solarSystemName, constellationID, regionID, securityClass, security FROM mapSolarSystems
        SolarSystemData sysData   = SolarSystemData();
        sysData.systemID          = row.GetInt(0);
        sysData.name              = row.GetText(1);
        sysData.constellationID   = row.GetInt(2);
        sysData.regionID          = row.GetInt(3);
        sysData.securityClass     = (row.IsNull(4) ? "0" : row.GetText(4));
        sysData.securityRating    = row.GetFloat(5);    // this gives system trueSec
        sysData.factionID         = (row.IsNull(6) ? 0 : row.GetUInt(6));
        m_solSysData.emplace(row.GetInt(0), sysData);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Static SolarSystem data sets loaded in %.3fms.", m_solSysData.size(), (GetTimeMSeconds() - startTime));
*/

    startTime = GetTimeMSeconds();
    ManagerDB::GetWHSystemClass(*res);
    while (res->GetRow(row)) {
        //SELECT locationID, wormholeClassID FROM mapLocationWormholeClasses
        m_whRegions.emplace(row.GetInt(0), row.GetInt(1));
    }
    sLog.Cyan("    StaticDataMgr", "%lu WH System Classes loaded in %.3fms.", m_whRegions.size(), (GetTimeMSeconds() - startTime));

    // Load wormhole destination classes into static memory object
    startTime = GetTimeMSeconds();
    int size = 0;
    for (int i = 1; i < 10; i++) {
        ManagerDB::GetWHClassDestinations(i, *res);
        DBResultRow row;
        m_whClassDestinations[i];
        while (res->GetRow(row)) {
            m_whClassDestinations[i].push_back(row.GetUInt(0));
        }
        size += m_whClassDestinations[i].size();
    }

    sLog.Cyan("    StaticDataMgr", "%lu WH Destination Classes loaded in %.3fms.",
              size, (GetTimeMSeconds() - startTime));

    // Load wormhole system classes into static memory object
    startTime = GetTimeMSeconds();
    size = 0;
    for (int i = 1; i < 10; i++) {
        ManagerDB::GetWHClassSystems(i, *res);
        DBResultRow row;
        m_whClassSystems[i];
        while (res->GetRow(row)) {
            m_whClassSystems[i].push_back(row.GetUInt(0));
        }
        size += m_whClassSystems[i].size();
    }

    sLog.Cyan("    StaticDataMgr", "%lu WH Class Systems loaded in %.3fms.",
              size, (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetStaticData(*res);
    while (res->GetRow(row)) {
        //SELECT itemID, regionID, constellationID, solarSystemID, typeID, radius, x, y, z FROM mapDenormalize
        StaticData data         = StaticData();
        data.itemID             = row.GetInt(0);
        data.regionID           = row.GetInt(1);
        data.constellationID    = row.GetInt(2);
        data.systemID           = row.GetInt(3);
        data.typeID             = row.GetInt(4);
        data.radius             = row.GetFloat(5);
        data.position           = GPoint(row.GetDouble(6),row.GetDouble(7),row.GetDouble(8));
        m_staticData.emplace(row.GetInt(0), data);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Static Entity data sets loaded in %.3fms.", m_staticData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    MapDB::GetStationCount(*res);
    while (res->GetRow(row)) {
        //SELECT map.solarSystemID, count(sta.stationID) FROM staStations sta
        m_stationCount.emplace(row.GetInt(0), row.GetInt(1));
    }
    StationDB::GetStationRegion(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, regionID FROM staStations
        m_stationRegion.emplace(row.GetInt(0), row.GetInt(1));
    }
    StationDB::GetStationConstellation(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, constellationID FROM staStations
        m_stationConst.emplace(row.GetInt(0), row.GetInt(1));
    }
    StationDB::GetStationSystem(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, solarSystemID FROM staStations
        m_stationSystem.emplace(row.GetInt(0), row.GetInt(1));
    }

    std::map<uint32, std::vector<uint32>>::iterator itr = m_stationList.begin();
    for (auto cur : m_stationSystem) {
        itr = m_stationList.find(cur.second);
        if (itr != m_stationList.end()) {
            itr->second.push_back(cur.first);
        } else {
            std::vector<uint32> sVec;
            sVec.push_back(cur.first);
            m_stationList.emplace(std::pair<uint32, std::vector<uint32>>(cur.second, sVec));
        }
    }
    sLog.Cyan("    StaticDataMgr", "%lu Static Station query sets loaded in %.3fms.", (m_stationConst.size() + m_stationRegion.size() + m_stationSystem.size() + m_stationList.size()), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetTypeAttributes(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, attributeID, valueInt, valueFloat FROM dgmTypeAttributes
        DmgTypeAttribute typeAttr = DmgTypeAttribute();
        typeAttr.attributeID = row.GetInt(1);
        if (row.IsNull(2)) {
            typeAttr.value = row.GetDouble(3);
        } else {
            typeAttr.value = row.GetInt(2); // highest value seen is 2,000,000,000 (struct HP)
        }

        m_typeAttrMap.emplace(row.GetInt(0), typeAttr);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Type Attribute Sets loaded in %.3fms", m_typeAttrMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetSkillList(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=skill]
        m_skills.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Skills loaded in %.3fms.", m_skills.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    FactoryDB::GetComponents(*res);     //766
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=composite or component]
        m_components.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetMinerals(*res);       //8
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=mineral]
        m_minerals.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetCompounds(*res);      //181
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=compound]
        m_compounds.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetSalvage(*res);        //53
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=salvage]
        m_salvage.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetResources(*res);      //15
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=pi resource]
        m_resources.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetCommodities(*res);    //66
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=pi commodity]
        m_commodities.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetMiscCommodities(*res);    //456
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=misc commodity]
        m_miscCommodities.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    FactoryDB::GetRAMMaterials(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
        EvERam::RamMaterials ramMatls = EvERam::RamMaterials();
        ramMatls.quantity       = row.GetInt(2);
        ramMatls.materialTypeID = row.GetInt(1);
        m_ramMatl.emplace(row.GetInt(0), ramMatls);
    }
    FactoryDB::GetRAMRequirements(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements
        EvERam::RamRequirements ramReq = EvERam::RamRequirements();
        ramReq.activityID       = row.GetInt(1);
        ramReq.requiredTypeID   = row.GetInt(2);
        ramReq.quantity         = row.GetInt(3);
        ramReq.damagePerJob     = row.GetFloat(4);
        ramReq.extra            = row.GetBool(5);
        m_ramReq.emplace(row.GetInt(0), ramReq);
    }
    sLog.Cyan("    StaticDataMgr", "%lu R.A.M. defs loaded in %.3fms.", (m_ramMatl.size() + m_ramReq.size()), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    FactoryDB::GetBlueprintType(*res);
    while (res->GetRow(row)) {
        //SELECT blueprintTypeID, parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime,
        //  researchTechTime, productivityModifier, materialModifier, wasteFactor, maxProductionLimit, chanceOfRE, catID FROM invBlueprintTypes
        EvERam::bpTypeData bpTypeData = EvERam::bpTypeData();
            bpTypeData.parentBlueprintTypeID    = row.GetInt(1);
            bpTypeData.productTypeID            = row.GetInt(2);
            bpTypeData.productionTime           = row.GetInt(3);
            bpTypeData.techLevel                = row.GetInt(4);
            bpTypeData.researchProductivityTime = row.GetInt(5);
            bpTypeData.researchMaterialTime     = row.GetInt(6);
            bpTypeData.researchCopyTime         = row.GetInt(7);
            bpTypeData.researchTechTime         = row.GetInt(8);
            bpTypeData.productivityModifier     = row.GetInt(9);
            bpTypeData.materialModifier         = row.GetInt(10);
            bpTypeData.wasteFactor              = row.GetInt(11);
            bpTypeData.maxProductionLimit       = row.GetInt(12);
            bpTypeData.chanceOfRE               = row.GetFloat(13);
            bpTypeData.catID                    = (row.IsNull(14) ? 0 : row.GetInt(14));
        m_bpTypeData.emplace(row.GetInt(0), bpTypeData);
        m_bpProductData.emplace(row.GetInt(2), bpTypeData);
    }
    for (auto cur : m_bpTypeData)
        m_bpMatlData[cur.first] = SetBPMatlType(cur.second.catID, cur.first, cur.second.productTypeID);
    sLog.Cyan("    StaticDataMgr", "%lu BP Type defs loaded in %.3fms.", m_bpTypeData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetMoonResouces(*res);
    while (res->GetRow(row)) {
        //SELECT typeID,volume FROM invTypes [where group=moongoo]
        m_moonGoo.emplace(row.GetInt(0), (uint8)(row.GetFloat(1) *10));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Moon Resources loaded in %.3fms.", m_moonGoo.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetOreBySSC(*res);
    while (res->GetRow(row)) {
        //SELECT systemSec, roidID, percent FROM roidDistribution
        OreTypeChance oreChance = OreTypeChance();
            oreChance.typeID  = row.GetInt(1);
            oreChance.chance  = row.GetFloat(2);
        m_oreBySecClass.emplace(row.GetText(0), oreChance);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Ore defs loaded in %.3fms.", m_oreBySecClass.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    //SELECT factionID, itemID FROM facSalvage
    ManagerDB::GetSalvageGroups(*res);
    while (res->GetRow(row))
        m_salvageMap.emplace(row.GetInt(0), row.GetInt(1));
    sLog.Cyan("    StaticDataMgr", "%lu salvage definitions loaded in %.3fms.", m_salvageMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.emplace(row.GetInt(0), row.GetInt(1));
    }

    ManagerDB::GetRegionRatFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0
        m_ratRegions.emplace(row.GetInt(0), row.GetInt(1));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Region Faction Data Sets loaded in %.3fms.", (m_regions.size() + m_ratRegions.size()), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetFactionGroups(*res);
    DBQueryResult* res2 = new DBQueryResult();
    DBResultRow row2;
    RatFactionGroups factionGroup;
    uint16 typeCount = 0;
    while (res->GetRow(row)) {
        //SELECT shipClass, groupID, factionID FROM npcClassGroup
        factionGroup.shipClass = row.GetInt(0);
        factionGroup.groupID = (uint16)row.GetInt(1);
        m_npcGroups.emplace(row.GetInt(2), factionGroup);

        rt_typeIDs rtt;
        ManagerDB::GetGroupTypeIDs((uint8)row.GetInt(0), (uint16)row.GetInt(1), row.GetInt(2), *res2);
        while (res2->GetRow(row2)) {
            //SELECT typeID FROM invTypes WHERE groupID = %u (plus specific checks) ORDER BY typeID
            rtt.push_back((uint16)row2.GetInt(0));
            ++typeCount;
        }
        rt_groups rtg;
        rtg.emplace((uint16)row.GetInt(1), rtt);
        m_npcTypes.emplace((uint8)row.GetInt(0), rtg);
    }

    ManagerDB::GetSpawnClasses(*res);
    while (res->GetRow(row)) {
        //SELECT type, sub, f, af, d, c, ac, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM npcSpawnClass
        RatSpawnClass spawnClass = RatSpawnClass();
        spawnClass.type = row.GetInt(0);
        spawnClass.sub = row.GetInt(1);
        spawnClass.f = row.GetInt(2);
        spawnClass.af = row.GetInt(3);
        spawnClass.d = row.GetInt(4);
        spawnClass.c = row.GetInt(5);
        spawnClass.ac = row.GetInt(6);
        spawnClass.bc = row.GetInt(7);
        spawnClass.bs = row.GetInt(8);
        spawnClass.h = row.GetInt(9);
        spawnClass.o = row.GetInt(10);
        spawnClass.cf = row.GetInt(11);
        spawnClass.cd = row.GetInt(12);
        spawnClass.cc = row.GetInt(13);
        spawnClass.cbc = row.GetInt(14);
        spawnClass.cbs = row.GetInt(15);
        m_npcClasses.emplace((uint8)row.GetInt(0), spawnClass);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Rat Groups, %u Rat Classes, and %u Rat Types for %u regions loaded in %.3fms.",\
              m_npcGroups.size(), m_npcClasses.size(), typeCount, m_ratRegions.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    SystemDB::GetWrecksToTypes(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, wreckTypeID FROM invTypesToWrecks
        m_WrecksToTypesMap[row.GetInt(0)] = row.GetInt(1);
    }
    sLog.Cyan("    StaticDataMgr", "%lu wreck objects loaded in %.3fms.", m_WrecksToTypesMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    SystemDB::GetLootGroups(*res);
    while (res->GetRow(row)) {
        //SELECT npcGroupID, itemGroupID, groupDropChance FROM lootGroup
        LootGroup lootGroup = LootGroup();
        lootGroup.lootGroupID = row.GetInt(1);
        lootGroup.dropChance = row.GetDouble(2);
        m_LootGroupMap.emplace(row.GetInt(0), lootGroup);
    }

    startTime = GetTimeMSeconds();
    SystemDB::GetLootGroupTypes(*res);
    while (res->GetRow(row)) {
        //SELECT itemGroupID, itemID, itemMetaLevel, minAmount, maxAmount FROM lootItemGroup
        LootGroupType GroupType = LootGroupType();
        GroupType.lootGroupID = row.GetInt(0);
        GroupType.typeID =  row.GetInt(1);
        GroupType.metaLevel = row.GetInt(2);
        GroupType.minQuantity = row.GetInt(3);
        GroupType.maxQuantity = row.GetInt(4);
        m_LootGroupTypeMap.emplace(row.GetInt(0), GroupType);
    }
    sLog.Cyan("    StaticDataMgr", "%lu loot groups and %u loot group types loaded in %.3fms.",
              m_LootGroupMap.size(), m_LootGroupTypeMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    uint32 locationID = 0;
    ManagerDB::GetAgentLocation(*res);
    while (res->GetRow(row)) {
        //SELECT agentID, locationID FROM agtAgents
        locationID = row.GetInt(1);
        if (IsStationID(locationID)) {
            locationID = GetStationSystem(locationID);
        }
        if (!IsSolarSystemID(locationID)) {
            _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
            continue;
        }

        m_agentSystem.emplace(row.GetInt(0), locationID);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Agent Data Sets loaded in %.3fms.", m_agentSystem.size(), (GetTimeMSeconds() - startTime));

    //cleanup
    SafeDelete(res);
    SafeDelete(res2);

    sLog.Cyan("    StaticDataMgr", "Static Data loaded in %.3fms.", (GetTimeMSeconds() - beginTime));
}

void StaticDataMgr::GetInfo()
{
    /* return info about loaded items? */
    /*
     * m_systemData;
     * m_staticData;
     *
     */
}

void StaticDataMgr::GetCategory(uint8 catID, Inv::CatData& into)
{
    std::map<uint16, Inv::CatData>::const_iterator itr = m_catData.find(catID);
    if (itr != m_catData.end())
        into = itr->second;
}

const char* StaticDataMgr::GetCategoryName(uint8 catID)
{
    std::map<uint16, Inv::CatData>::const_iterator itr = m_catData.find(catID);
    if (itr != m_catData.end())
        return itr->second.name.c_str();

    _log(DATA__ERROR, "GetCategoryName() - Category %u not found in map", catID);
    return "None";
}

void StaticDataMgr::GetGroup(uint16 grpID, Inv::GrpData& into)
{
    std::map<uint16, Inv::GrpData>::const_iterator itr = m_grpData.find(grpID);
    if (itr != m_grpData.end())
        into = itr->second;
}

const char* StaticDataMgr::GetGroupName(uint16 grpID)
{
    std::map<uint16, Inv::GrpData>::const_iterator itr = m_grpData.find(grpID);
    if (itr != m_grpData.end())
        return itr->second.name.c_str();

    _log(DATA__ERROR, "GetGroupName() - Group %u not found in map", grpID);
    return "None";
}

void StaticDataMgr::GetType(uint16 typeID, Inv::TypeData& into)
{
    std::map<uint16, Inv::TypeData>::const_iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        into = itr->second;
}

const char* StaticDataMgr::GetTypeName(uint16 typeID)
{
    std::map<uint16, Inv::TypeData>::const_iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.name.c_str();

    _log(DATA__ERROR, "GetGroupName() - Group %u not found in map", typeID);
    return "None";
}

void StaticDataMgr::GetTypes(std::map< uint16, Inv::TypeData >& into)
{
    into = m_typeData;
}

const char* StaticDataMgr::GetAttrName(uint16 attrID)
{
    std::map<uint16, AttrTypeData>::const_iterator itr = m_attrTypeData.find(attrID);
    if (itr != m_attrTypeData.end())
        return itr->second.attributeName.c_str();
        //return itr->second.displayName.c_str();

    _log(DATA__ERROR, "GetAttrName() - Attribute %u not found in map", attrID);
    return "None";
}

PyInt* StaticDataMgr::GetAgentSystemID(int32 agentID)
{
    std::map<uint32, uint32>::iterator itr = m_agentSystem.find(agentID);
    if (itr != m_agentSystem.end())
        return new PyInt(itr->second);

    _log(DATA__WARNING, "Failed to query system info for agent %u: Agent not found.", agentID);
    return new PyInt(0);
}

void StaticDataMgr::GetSalvage(uint32 factionID, std::vector<uint32> &itemList) {
    auto itr = m_salvageMap.equal_range(factionID);
    for (auto it = itr.first; it != itr.second; ++it)
        itemList.push_back(it->second);
}

bool StaticDataMgr::GetRoidDist(const char* secClass, std::unordered_multimap<float, uint16>& roids) {
    auto groupRange = m_oreBySecClass.equal_range(secClass);
    for (auto it = groupRange.first; it != groupRange.second; ++it) {
        _log(MINING__INFO, "GetRoidDist - adding %u with chance %.3f", it->second.typeID, it->second.chance);
        roids.insert(std::pair<float, uint32>(it->second.chance, it->second.typeID));
    }

    return !roids.empty();
}

void StaticDataMgr::GetDgmTypeAttrVec(uint16 typeID, std::vector< DmgTypeAttribute >& typeAttrVec)
{
    auto itr = m_typeAttrMap.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        typeAttrVec.push_back(it->second);
}

bool StaticDataMgr::IsSkillTypeID(uint16 typeID)
{
    return (m_skills.find(typeID) != m_skills.end());
}

bool StaticDataMgr::GetSkillName(uint16 skillID, std::string& name)
{
    std::map<uint16, std::string>::iterator itr = m_skills.find(skillID);
    if (itr != m_skills.end()) {
        name = itr->second;
        return true;
    }

    _log(DATA__MESSAGE, "Failed to query name for skill %u: Skill not found.", skillID);
    return false;
}

void StaticDataMgr::GetComponentData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_components) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetMineralData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_minerals) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetCompoundData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_compounds) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetSalvageData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_salvage) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetPIResourceData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_resources) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetPICommodityData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_commodities) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetMiscCommodityData(std::map< uint16, Market::matlData >& into)
{
    for (auto cur : m_miscCommodities) {
        Market::matlData data = Market::matlData();
        data.typeID = cur.first;
        data.name = cur.second;
        into[cur.first] = data;
    }
}

void StaticDataMgr::GetMoonResouces(std::map<uint16, uint8>& data)
{
    // make copy
    for (auto cur : m_moonGoo)
        data.emplace(cur.first, cur.second);
}

uint16 StaticDataMgr::GetRandRatType(uint8 sClass, uint16 groupID)
{
    if (groupID == 0)
        return 0;
    std::vector< uint16 > typeVec;
    auto classRange = m_npcTypes.equal_range(sClass);
    for (auto it = classRange.first; it != classRange.second; ++it) {
        for (auto itr : it->second)
            if (itr.first == groupID) {
                for (auto tItr : itr.second)
                    typeVec.push_back(tItr);
                break;
            }
    }
    if (typeVec.size())
        return typeVec.at(MakeRandomInt(0, typeVec.size()));

    _log(DATA__WARNING, "Failed to get random rat for sClass %u and groupID %u", sClass, groupID);
    return 0;
}

bool StaticDataMgr::GetNPCTypes(uint16 groupID, std::vector< uint16 >& typeVec)
{
    /*  this is now invalid.....
    auto groupRange = m_npcTypes.equal_range(groupID);
    for (auto it = groupRange.first; it != groupRange.second; ++it)
        typeVec.push_back(it->second);

    return !typeVec.empty();
    */
    return false;
}

bool StaticDataMgr::GetNPCGroups(uint32 factionID, std::map< uint8, uint16 >& groupMap)
{
    auto groupRange = m_npcGroups.equal_range(factionID);
    for (auto it = groupRange.first; it != groupRange.second; ++it)
        groupMap.emplace(it->second.shipClass, it->second.groupID);

    return !groupMap.empty();
}

bool StaticDataMgr::GetNPCClasses(uint8 sClass, std::vector< RatSpawnClass >& classMap)
{
    auto classRange = m_npcClasses.equal_range(sClass);
    for (auto it = classRange.first; it != classRange.second; ++it) {
        RatSpawnClass spawnClass = RatSpawnClass();
        spawnClass.type = it->second.type;
        spawnClass.sub = it->second.sub;
        spawnClass.f = it->second.f;
        spawnClass.af = it->second.af;
        spawnClass.d = it->second.d;
        spawnClass.c = it->second.c;
        spawnClass.ac = it->second.ac;
        spawnClass.bc = it->second.bc;
        spawnClass.bs = it->second.bs;
        spawnClass.h = it->second.h;
        spawnClass.o = it->second.o;
        spawnClass.cf = it->second.cf;
        spawnClass.cd = it->second.cd;
        spawnClass.cc = it->second.cc;
        spawnClass.cbc = it->second.cbc;
        spawnClass.cbs = it->second.cbs;
        classMap.push_back(spawnClass);
    }

    return !classMap.empty();
}

uint32 StaticDataMgr::GetWreckID(uint32 typeID)
{
    std::map<uint32, uint32>::const_iterator itr = m_WrecksToTypesMap.find(typeID);
    if (itr != m_WrecksToTypesMap.end())
        return itr->second;
    return 0;
}

void StaticDataMgr::GetLoot(uint32 groupID, std::vector<LootList>& lootList) {
    double profileStartTime(GetTimeUSeconds());

    float randChance(0.0f);
    uint8 metaLevel(0);
    std::vector<LootGroupType> lootGrpVec;
    lootGrpVec.clear();

    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    auto range = m_LootGroupMap.equal_range(groupID);
    for (auto it = range.first; it != range.second; ++it) {
        _log(LOOT__INFO, "checking lootGroup %u with chance of %.2f", it->second.lootGroupID, it->second.dropChance);
        // make lootMap of lootGroupID's
        if (MakeRandomFloat(0, 1) < it->second.dropChance) {
            randChance = MakeRandomFloat(0, 1);
            metaLevel = 0;
            if (randChance < 0.1) {
                metaLevel = 4;
            } else if (randChance < 0.25) {
                metaLevel = 3;
            } else if (randChance < 0.4) {
                metaLevel = 2;
            } else if (randChance < 0.6) {
                metaLevel = 1;
            }
            /*need to figure out how to get faction loot for faction wrecks
             *    elif meta_level == 7:
             *        drop_chance = 0.15   # Faction stuff = 15%
             *    elif meta_level == 8:
             *        drop_chance = 0.15   # Faction projectiles = 15%
             *    elif meta_level == 9:
             *        drop_chance = 0.15   # Faction SB's and Missile launchers
             */

            auto range2 = m_LootGroupTypeMap.equal_range(it->second.lootGroupID);
            for (auto it2 = range2.first; it2 != range2.second; ++it2)
                if (it2->second.metaLevel == metaLevel)
                    lootGrpVec.push_back(it2->second);

            if (!lootGrpVec.empty()) {
                LootList loot_list;
                uint16 i = MakeRandomInt(0, lootGrpVec.size());
                loot_list.itemID = lootGrpVec[i].typeID;
                loot_list.minDrop = lootGrpVec[i].minQuantity;
                loot_list.maxDrop = lootGrpVec[i].maxQuantity;
                lootList.push_back(loot_list);
                _log(LOOT__INFO, "adding %u to lootList", lootGrpVec[i].typeID);
                lootGrpVec.clear();
            }
        }
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::loot, GetTimeUSeconds() - profileStartTime);
}

void StaticDataMgr::GetBpTypeData(uint16 typeID, EvERam::bpTypeData& tData)
{
    std::map<uint16, EvERam::bpTypeData>::iterator itr = m_bpTypeData.find(typeID);
    if (itr != m_bpTypeData.end()) {
        tData = itr->second;
    } else {
        _log(DATA__MESSAGE, "Failed to query info for bpType %u: Type not found.", typeID);
    }
}

bool StaticDataMgr::GetBpDataForItem(uint16 typeID, EvERam::bpTypeData& tData)
{
    std::map<uint16, EvERam::bpTypeData>::iterator itr = m_bpProductData.find(typeID);
    if (itr != m_bpProductData.end()) {
        tData = itr->second;
        return true;
    }
    return false;
}

bool StaticDataMgr::IsRecyclable(uint16 typeID)
{
    std::map<uint16, Inv::TypeData>::iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.isRecyclable;
    return false;
}

bool StaticDataMgr::IsRefinable(uint16 typeID)
{
    std::map<uint16, Inv::TypeData>::iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.isRefinable;
    return false;
}

void StaticDataMgr::GetRamReturns(uint16 typeID, int8 activityID, std::vector< EvERam::RequiredItem >& ramReqs)
{
    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        if ((it->second.activityID == activityID) and (it->second.extra) and !(IsSkillTypeID(it->second.requiredTypeID))) {
            EvERam::RequiredItem data = EvERam::RequiredItem();
            data.typeID = it->second.requiredTypeID;
            data.quantity = it->second.quantity;
            data.damagePerJob = it->second.damagePerJob;
            data.isSkill = IsSkillTypeID(it->second.requiredTypeID);
            data.extra = it->second.extra;
            ramReqs.push_back(data);
        }
}

void StaticDataMgr::GetRamMaterials(uint16 typeID, std::vector< EvERam::RamMaterials >& ramMatls)
{
    auto itr = m_ramMatl.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        ramMatls.push_back(it->second);
}

void StaticDataMgr::GetRamRequirements(uint16 typeID, std::vector< EvERam::RamRequirements >& ramReqs)
{
    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        ramReqs.push_back(it->second);
}

void StaticDataMgr::GetRamRequiredItems(const uint32 typeID, const int8 activity, std::vector< EvERam::RequiredItem >& into)
{
    if (activity == EvERam::Activity::Manufacturing) {
        std::map<uint16, EvERam::bpTypeData>::iterator itr = m_bpTypeData.find(typeID);
        if (itr != m_bpTypeData.end()) {
            auto range = m_ramMatl.equal_range(itr->second.productTypeID);
            for (auto it = range.first; it != range.second; ++it) {
                EvERam::RequiredItem data = EvERam::RequiredItem();
                data.typeID = it->second.materialTypeID;
                data.quantity = it->second.quantity;
                into.push_back(data);
            }
        }
    }

    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.activityID == activity) {
            EvERam::RequiredItem data = EvERam::RequiredItem();
            data.typeID = it->second.requiredTypeID;
            data.quantity = it->second.quantity;
            data.damagePerJob = it->second.damagePerJob;
            data.isSkill = IsSkillTypeID(it->second.requiredTypeID);
            data.extra = it->second.extra;
            into.push_back(data);
        }
}

PyRep* StaticDataMgr::GetStationCount()
{
    PyList* list = new PyList();
    std::map<uint32, uint8>::iterator itr = m_stationCount.begin(), end = m_stationCount.end();
    while (itr != end) {
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(itr->first));
        tuple->SetItem(1, new PyInt(itr->second));
        list->AddItem(tuple);
        ++itr;
    }
    return list;
}

uint8 StaticDataMgr::GetStationCount(uint32 systemID)
{
    std::map<uint32, uint8>::iterator itr = m_stationCount.find(systemID);
    if (itr != m_stationCount.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query station count for system %u: System not found.", systemID);
    return 0;
}

bool StaticDataMgr::GetStationList(uint32 systemID, std::vector< uint32 >& data)
{
    std::map<uint32, std::vector<uint32>>::iterator itr = m_stationList.find(systemID);
    if (itr != m_stationList.end()) {
        data = itr->second;
        return true;
    }
    return false;
}

uint32 StaticDataMgr::GetStationRegion(uint32 stationID)
{
    std::map<uint32, uint32>::iterator itr = m_stationRegion.find(stationID);
    if (itr != m_stationRegion.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query region info for station %u: Station not found.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationConstellation(uint32 stationID)
{
    std::map<uint32, uint32>::iterator itr = m_stationConst.find(stationID);
    if (itr != m_stationConst.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query constellation info for station %u: Station not found.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationSystem(uint32 stationID)
{
    std::map<uint32, uint32>::iterator itr = m_stationSystem.find(stationID);
    if (itr != m_stationSystem.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query system info for station %u: Station not found.", stationID);
    return 0;
}

uint8 StaticDataMgr::GetWHSystemClass(uint32 systemID)
{
    std::map<uint32, uint8>::iterator itr = m_whRegions.find(systemID);
    if (itr != m_whRegions.end())
        return itr->second;

    SystemManager* pSysMgr(sEntityList.FindOrBootSystem(systemID));
    if (pSysMgr == nullptr)
        return 0;

    itr = m_whRegions.find(pSysMgr->GetRegionID());
    if (itr != m_whRegions.end())
        return itr->second;

    // dont have data for systemID nor regionID...throw error and ?something else?
    _log(DATA__MESSAGE, "Failed to query WH Class for systemID %u: System not found.", systemID);
    if (IsKSpaceID(systemID))
        return 0;
    if (IsWSpaceID(systemID))
        return 0;

    return 0;
}

bool StaticDataMgr::GetSystemData(uint32 locationID, SystemData& data)
{
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
    }
    if (!IsSolarSystem(locationID)) {
        _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return false;
    }

    std::map<uint32, SystemData>::iterator itr = m_systemData.find(locationID);
    if (itr != m_systemData.end()) {
        data = itr->second;
        return true;
    }

    _log(DATA__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
    return false;
}

const char* StaticDataMgr::GetSystemName(uint32 locationID)
{
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
    }
    if (!IsSolarSystem(locationID)) {
        _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return "Error";
    }

    std::map<uint32, SystemData>::iterator itr = m_systemData.find(locationID);
    if (itr != m_systemData.end())
        return itr->second.name.c_str();

    _log(DATA__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
    return "Invalid";
}

bool StaticDataMgr::GetSolarSystemData(uint32 sysID, SolarSystemData& into)
{
    if (!IsSolarSystem(sysID)) {
        _log(DATA__MESSAGE, "Failed to query info:  locationID %u is not system.", sysID);
        return false;
    }
    m_solSysData;
    return true;
}

bool StaticDataMgr::GetStaticInfo(uint32 itemID, StaticData& data)
{
    std::map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end()) {
        data = itr->second;
        return true;
    }

    _log(DATA__MESSAGE, "Failed to query info for static item %u: Item not found.", itemID);
    return false;
}

uint16 StaticDataMgr::GetStaticType(uint32 itemID)
{
    std::map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end())
        return itr->second.typeID;
    return 0;
}

uint32 StaticDataMgr::GetRegionFaction(uint32 regionID)
{
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query faction for region %u: region not found.", regionID);
    return 0;
}

uint32 StaticDataMgr::GetRegionRatFaction(uint32 regionID)
{
    std::map<uint32, uint32>::iterator itr = m_ratRegions.find(regionID);
    if (itr != m_ratRegions.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query rat faction for region %u: region not found.", regionID);
    return 0;

/*
def GetPirateFactionsOfRegion(self, regionID):
return {10000001: (500019,),
    10000002: (500010,),
    10000003: (500010,),
    10000005: (500011,),
    10000006: (500011,),
    10000007: (500011,),
    10000008: (500011,),
    10000009: (500011,),
    10000010: (500010,),
    10000011: (500011,),
    10000012: (500011,),
    10000014: (500019,),
    10000015: (500010,),
    10000016: (500010,),
    10000020: (500019,),
    10000022: (500019,),
    10000023: (500010,),
    10000025: (500011,),
    10000028: (500011,),
    10000029: (500010,),
    10000030: (500011,),
    10000031: (500011,),
    10000032: (500020,),
    10000033: (500010,),
    10000035: (500010,),
    10000036: (500019,),
    10000037: (500020,),
    10000038: (500012,),
    10000039: (500019,),
    10000041: (500020,),
    10000042: (500011,),
    10000043: (500019,),
    10000044: (500020,),
    10000045: (500010,),
    10000046: (500020,),
    10000047: (500019,),
    10000048: (500020,),
    10000049: (500012, 500019),
    10000050: (500012,),
    10000051: (500020,),
    10000052: (500012,),
    10000054: (500012,),
    10000055: (500010,),
    10000056: (500011,),
    10000057: (500020,),
    10000058: (500020,),
    10000059: (500019,),
    10000060: (500012,),
    10000061: (500011,),
    10000062: (500011,),
    10000063: (500012,),
    10000064: (500020,),
    10000065: (500012,),
    10000067: (500012,),
    10000068: (500020,)}
    */
}

bool StaticDataMgr::IsSolarSystem(uint32 systemID/*0*/)
{
    // if systemID has entry here, it is valid
    std::map<uint32, SystemData>::iterator itr = m_systemData.find(systemID);
    return (itr != m_systemData.end());
}

bool StaticDataMgr::IsStation(uint32 stationID/*0*/)
{
    // if stationID has entry here, it is valid
    std::map<uint32, uint32>::iterator itr = m_stationRegion.find(stationID);
    return (itr != m_stationRegion.end());
}

DBRowDescriptor* StaticDataMgr::CreateHeader() {
    // this is correct data for crucible.  dont alter
    PyList *keywords = new PyList();
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));
    DBRowDescriptor* header = new DBRowDescriptor(keywords);
        header->AddColumn("itemID",     DBTYPE_I8);     // int64
        header->AddColumn("typeID",     DBTYPE_I4);     // int32
        header->AddColumn("ownerID",    DBTYPE_I4);     // int32
        header->AddColumn("locationID", DBTYPE_I4);     // this should be I8 (according to packets)
        header->AddColumn("flagID",     DBTYPE_I2);     // int16
        header->AddColumn("quantity",   DBTYPE_I4);     // int32
        header->AddColumn("groupID",    DBTYPE_I4);     // int32
        header->AddColumn("categoryID", DBTYPE_I4);     // int32
        header->AddColumn("customInfo", DBTYPE_STR);
    return header;
}

PyDict* StaticDataMgr::GetBPMatlData(uint16 typeID)
{
    auto itr = m_bpMatlData.find(typeID);
    if (itr != m_bpMatlData.end()) {
        PyIncRef(itr->second);
        itr->second->Dump(MANUF__DEBUG, "    ");
        return itr->second;
    }
    return nullptr;
}

PyDict* StaticDataMgr::SetBPMatlType(int8 catID, uint16 typeID, uint16 prodID)
{
    // dunno how to do this part better...
    PyList* matlListManuf = new PyList();
    PyList* skillListManuf = new PyList();
    PyList* extraListManuf = new PyList();
    PyList* matlListTE = new PyList();
    PyList* skillListTE = new PyList();
    PyList* matlListME = new PyList();
    PyList* skillListME = new PyList();
    PyList* matlListCopy = new PyList();
    PyList* skillListCopy = new PyList();
    PyList* matlListDup = new PyList();
    PyList* skillListDup = new PyList();
    PyList* extraListDup = new PyList();
    PyList* matlListRE = new PyList();
    PyList* skillListRE = new PyList();
    PyList* matlListInvent = new PyList();
    PyList* skillListInvent = new PyList();

    DBRowDescriptor* header = new DBRowDescriptor();
        header->AddColumn("quantity",          DBTYPE_I4);
        header->AddColumn("requiredTypeID",    DBTYPE_I4);
        header->AddColumn("damagePerJob",      DBTYPE_R4);

    // NOTE: this is for BLUEPRINTS ONLY and is always populated (ancient relic error fix)
    if (catID == EVEDB::invCategories::Blueprint) {
        // ramMaterials is only for manufacturing the bp product
        std::vector<EvERam::RamMaterials> ramMatls;
        ramMatls.clear();
        GetRamMaterials(prodID, ramMatls);
        for (auto cur : ramMatls) {
            PyPackedRow* row = new PyPackedRow(header);
                row->SetField("quantity",        new PyInt(cur.quantity));
                row->SetField("requiredTypeID",  new PyInt(cur.materialTypeID));
                row->SetField("damagePerJob",    new PyFloat(1.0f));
            matlListManuf->AddItem(row);
        }
    }

    // booleans to only set items that are populated  NOTE: manuf is always populated for blueprints
    bool manuf(false), copy(false), invent(false), dup(false), me(false), re(false), te(false), tech(false);
    //  get R.A.M. skills and materials for both bp typeID and product typeID
    // the ramRequirements table holds ALL skill/item data for all aspects of RAM per BlueprintTypeID.
    std::vector<EvERam::RamRequirements> ramReqs;
    ramReqs.clear();
    GetRamRequirements(typeID, ramReqs);
    //GetRamRequirements(prodID, ramReqs);
    for (auto cur : ramReqs) {
        PyPackedRow* row = new PyPackedRow(header);
            row->SetField("quantity",        new PyInt(cur.quantity));
            row->SetField("requiredTypeID",  new PyInt(cur.requiredTypeID));
            row->SetField("damagePerJob",    new PyFloat(cur.damagePerJob));

        using namespace EvERam;
        switch(cur.activityID) {
            case Activity::Manufacturing: {         //1
                /** @todo  this needs work.  dunno how to remove 'extra' materials from this list */
                manuf = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListManuf->AddItem(row);
                } else if (cur.extra) {
                    extraListManuf->AddItem(row);
                } else {
                    matlListManuf->AddItem(row);
                }
            } break;
            case Activity::ResearchTech: {          //2
                // not used.  not defined in client.  no data for this activity
            } break;
            case Activity::ResearchTime: {          //3
                te = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListTE->AddItem(row);
                } else {
                    matlListTE->AddItem(row);
                }
            } break;
            case Activity::ResearchMaterial: {      //4
                me = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListME->AddItem(row);
                } else {
                    matlListME->AddItem(row);
                }
            } break;
            case Activity::Copying: {               //5
                copy = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListCopy->AddItem(row);
                } else {
                    matlListCopy->AddItem(row);
                }
            } break;
            case Activity::Duplicating: {           //6
                dup = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListDup->AddItem(row);
                } else if (cur.extra) {
                    extraListDup->AddItem(row);
                } else {
                    matlListDup->AddItem(row);
                }
            } break;
            case Activity::ReverseEngineering: {    //7
                re = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListRE->AddItem(row);
                } else {
                    matlListRE->AddItem(row);
                }
            } break;
            case Activity::Invention: {             //8
                invent = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListInvent->AddItem(row);
                } else {
                    matlListInvent->AddItem(row);
                }
            } break;
        }
    }

    // this is the response.  test for items populated above and create an ItemString in the dict for that item.
    // items not populated will not be shown in the BP info.
    DBQueryResult mtRes;
    PyRep* mtCRowSet = DBResultToCRowset(mtRes);
    PyDict* rsp = new PyDict();
    // activity '0' should stay empty
    //activityNone = 0
    //rsp->SetItem(0, new PyDict());

    if (manuf) {        //activityManufacturing = 1
        PyDict* Manufacturing = new PyDict();
            Manufacturing->SetItemString("skills", skillListManuf);
            Manufacturing->SetItemString("rawMaterials", matlListManuf);
            CRowSet *rowset = new CRowSet(&header);
            PyList::const_iterator itr = extraListManuf->begin();
            for (; itr != extraListManuf->end(); ++itr) {
                PyPackedRow* from = (*itr)->AsPackedRow();
                PyPackedRow* into = rowset->NewRow();
                into->SetField((uint32)0, from->GetField(0));
                into->SetField((uint32)1, from->GetField(1));
                into->SetField((uint32)2, from->GetField(2));
            }
        Manufacturing->SetItemString("extras", rowset);     // have to build a crowset for this
        rsp->SetItem(new PyInt(1), new PyObject("util.KeyVal", Manufacturing));
    }
    if (tech) {        //activityResearchingTechnology = 2
        // not used.  not defined in client.  no data for this activity
    }
    if (te) {        //activityResearchingTimeProductivity = 3
        PyDict* ResearchTime = new PyDict();
            ResearchTime->SetItemString("skills", skillListTE);
            ResearchTime->SetItemString("rawMaterials", matlListTE);
            PyIncRef(mtCRowSet);
        ResearchTime->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(3), new PyObject("util.KeyVal", ResearchTime));
    }
    if (me) {        //activityResearchingMaterialProductivity = 4
        PyDict* ResearchMaterial = new PyDict();
            ResearchMaterial->SetItemString("skills", skillListME);
            ResearchMaterial->SetItemString("rawMaterials", matlListME);
            PyIncRef(mtCRowSet);
        ResearchMaterial->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(4), new PyObject("util.KeyVal", ResearchMaterial));
    }
    if (copy) {        //activityCopying = 5
        PyDict* Copying = new PyDict();
            Copying->SetItemString("skills", skillListCopy);
            Copying->SetItemString("rawMaterials", matlListCopy);
            PyIncRef(mtCRowSet);
        Copying->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(5), new PyObject("util.KeyVal", Copying));
    }
    if (dup) {       //activityDuplicating = 6
        // no longer used...updated to "copying" after RMR
        PyDict* Duplicating = new PyDict();
            Duplicating->SetItemString("skills", skillListDup);
            Duplicating->SetItemString("rawMaterials", matlListDup);
            CRowSet *rowset = new CRowSet(&header);
            PyList::const_iterator itr = extraListDup->begin();
            for (; itr != extraListDup->end(); ++itr) {
                PyPackedRow* from = (*itr)->AsPackedRow();
                PyPackedRow* into = rowset->NewRow();
                into->SetField((uint32)0, from->GetField(0));
                into->SetField((uint32)1, from->GetField(1));
                into->SetField((uint32)2, from->GetField(2));
            }
        Duplicating->SetItemString("extras", rowset);    // have to build a crowset for this
        rsp->SetItem(new PyInt(6), new PyObject("util.KeyVal", Duplicating));
    }
    if (re) {        //activityReverseEngineering = 7
        PyDict* ReverseEngineering = new PyDict();
            ReverseEngineering->SetItemString("skills", skillListRE);
            ReverseEngineering->SetItemString("rawMaterials", matlListRE);
            PyIncRef(mtCRowSet);
        ReverseEngineering->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(7), new PyObject("util.KeyVal", ReverseEngineering));
    }
    if (invent) {     //activityInvention = 8
        PyDict* Invention = new PyDict();
            Invention->SetItemString("skills", skillListInvent);
            Invention->SetItemString("rawMaterials", matlListInvent);
            PyIncRef(mtCRowSet);
        Invention->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(8), new PyObject("util.KeyVal", Invention));
    }

    /**
     * For some reason, Ref decrement causes internal parts of data to change. In this particular instance, CRowSets with data
     * turns to Tuples, and client freaks out because it can no longer find named attributes.
     * Commenting it out until we figure out how to resolve it.
     */
    /*
    // cleanup
    PyDecRef(matlListManuf);
    PyDecRef(skillListManuf);
    PyDecRef(extraListManuf);
    PyDecRef(matlListTE);
    PyDecRef(skillListTE);
    PyDecRef(matlListME);
    PyDecRef(skillListME);
    PyDecRef(matlListCopy);
    PyDecRef(skillListCopy);
    PyDecRef(matlListDup);
    PyDecRef(skillListDup);
    PyDecRef(extraListDup);
    PyDecRef(matlListRE);
    PyDecRef(skillListRE);
    PyDecRef(matlListInvent);
    PyDecRef(skillListInvent);
    PyDecRef(mtCRowSet);
    */

    return rsp;
}

/** @todo  finish this.
 *      - only used by GetCurrentEntities().  custom call for alasiya eve
 */
std::string StaticDataMgr::GetOwnerName(int32 ownerID)
{
    if (ownerID == 1)
        return "System";

    return "Unknown - WIP";
}

uint8 StaticDataMgr::GetRegionQuarter(uint32 regionID)
{
    uint32 factionID = 0;
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        factionID = (*itr).second;

    // caldari=1, minmatar=2, amarr=3, gallente=4, none=5
    switch (factionID) {
        case factionCaldari:        //Caldari State
        case factionGuristas:       //Guristas Pirates
            return 1;
        case factionMinmatar:       //Minmatar Republic
        case factionAngel:          //Angel Cartel
            return 2;
        case factionAmarr:          //Amarr Empire
        case factionAmmatar:        //Ammatar Mandate
        case factionKhanid:         //Khanid Kingdom
        case factionBloodRaider:    //Blood Raider Covenant
        case factionSanshas:        //Sansha's Nation
            return 3;
        case factionGallente:       //Gallente Federation
        case factionSerpentis:      //Serpentis
            return 4;
        case factionJove:           //Jove Empire
        case factionCONCORD:        //CONCORD Assembly
        case factionSyndicate:      //The Syndicate
        case factionInterBus:       //The InterBus
        case factionORE:            //ORE
        case factionThukker:        //Thukker Tribe
        case factionSistersOfEVE:   //Servant Sisters of EVE
        case factionSociety:        //The Society of Conscious Thought
        case factionMordusLegion:   //Mordu's Legion Command
            return 5;
    }
    // default to 'none'
    return 5;
}

uint32 StaticDataMgr::GetFactionCorp(uint32 factionID)
{
    switch (factionID) {
        case factionAngel:          return corpArchangels;
        case factionSanshas:        return corpTruePower;
        case factionBloodRaider:    return corpBloodRaider;
        case factionGuristas:       return corpGuristas;
        case factionSerpentis:      return corpSerpentis;
        case factionRogueDrones:    return corpRogueDrones;

        case factionCONCORD:        return corpCONCORD;
        case factionInterBus:       return corpInterbus;
        case factionSociety:        return corpSocietyofCT;
        case factionMordusLegion:   return corpMordusLegion;

        case factionCaldari:        return corpCaldariNavy;
        case factionORE:            return corpORE;
        case factionAmarr:          return corpAmarrNavy;
        case factionGallente:       return corpGuristas;
        case factionJove:           return corpJovianDirectorate;
        case factionAmmatar:        return corpAmmatarConsulate;

        case factionKhanid:         return corpKhanidNavy;
        case factionThukker:        return corpThukkerMix;
        case factionSistersOfEVE:   return corpSoE;

        case factionNoFaction:
        default:                    return 0;

        //case factionSleepers:    return corpRogueDrones;
        //case factionMinmatar:        return corp;
        //case factionSyndicate:    return corps;
    }

    return 0;
}

std::string StaticDataMgr::GetCorpName(uint32 corpID)
{
    switch (corpID) {
        case corpArchangels:        return "Angel Cartel";
        case corpTrueCreations:     return "True Creations/Sansha";    // sansha's nation
        case corpTruePower:         return "True Power/Sansha";   // sansha's nation
        case corpBloodRaider:       return "Blood Raiders";
        case corpGuristas:          return "Guristas";
        case corpSerpentis:         return "Serpentis";
        case corpSerpentisInquest:  return "Serpentis Inquest";
        case corpRogueDrones:       return "Rogue Drones";
    }

    _log(DATA__ERROR, "Name not found for corp %u", corpID);
    return "Undefined - WIP";
}

uint32 StaticDataMgr::GetCorpFaction(uint32 corpID)
{
    std::map<uint32, uint32>::iterator itr = m_corpFaction.find(corpID);
    if (itr != m_corpFaction.end())
        return itr->second;

    if (IsNPCCorp(corpID))
        _log(DATA__ERROR, "Faction not found for NPC corp %s", GetCorpName(corpID).c_str());

    return 0;
}

std::string StaticDataMgr::GetFactionName(uint32 factionID)
{
    switch (factionID) {
        case factionAngel:          return "Angel Cartel";
        case factionSanshas:        return "Sansha Nation";
        case factionBloodRaider:    return "Blood Raiders";
        case factionGuristas:       return "Guristas Pirates";
        case factionSerpentis:      return "Serpentis";
        case factionRogueDrones:    return "Drone";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetRaceName(uint8 raceID)
{
    switch (raceID) {
        case Char::Race::Caldari:       return "Caldari";
        case Char::Race::Minmatar:      return "Minmatar";
        case Char::Race::Amarr:         return "Amarr";
        case Char::Race::Gallente:      return "Gallente";
        case Char::Race::Jove:          return "Jove";
        case Char::Race::Pirate:        return "Pirate";
        case Char::Race::Sleepers:      return "Sleeper";
        case Char::Race::ORE:           return "ORE";
    }
    // default to none
    return "Race Not Defined";
}

uint32 StaticDataMgr::GetRaceFaction(uint8 raceID)
{
    switch (raceID) {
        case Char::Race::Caldari:       return factionCaldari;
        case Char::Race::Minmatar:      return factionMinmatar;
        case Char::Race::Amarr:         return factionAmarr;
        case Char::Race::Gallente:      return factionGallente;
        case Char::Race::Jove:          return factionJove;
        case Char::Race::Pirate:        return factionNoFaction;
        case Char::Race::Sleepers:      return factionSleepers;
        case Char::Race::ORE:           return factionORE;
    }
    // default to none
    return factionNoFaction;
}

uint8 StaticDataMgr::GetFactionRace(uint32 factionID)
{
    switch (factionID) {
        case factionCaldari:        return Char::Race::Caldari;
        case factionMinmatar:       return Char::Race::Minmatar;
        case factionAmarr:          return Char::Race::Amarr;
        case factionGallente:       return Char::Race::Gallente;
        case factionJove:           return Char::Race::Jove;
        case factionNoFaction:      return Char::Race::Pirate;
        case factionSleepers:       return Char::Race::Sleepers;
        case factionORE:            return Char::Race::ORE;
        case factionAmmatar:        return Char::Race::Ammatar;
    }
    // default to Gallente
    return Char::Race::Gallente;
}

const char* StaticDataMgr::GetRigSizeName(uint8 size)
{
    switch (size) {
        case 0:      return "Undefined";
        case 1:      return "Small";
        case 2:      return "Medium";
        case 3:      return "Large";
        case 4:      return "Capitol";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetProcStateName(int8 state)
{
    using namespace EVEPOS;
    switch(state) {
        case ProcState::Invalid:            return "Invalid";
        case ProcState::Unanchoring:        return "Unanchoring";
        case ProcState::Anchoring:          return "Anchoring";
        case ProcState::Offlining:          return "Offlining";
        case ProcState::Onlining:           return "Onlining";
        case ProcState::Online:             return "Online";
        case ProcState::Operating:          return "Operating";
        case ProcState::Reinforcing:        return "Reinforcing";
        case ProcState::SheildReinforcing:  return "SheildReinforcing";
        case ProcState::ArmorReinforcing:   return "ArmorReinforcing";
        default:                            return "Bad State";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetFlagName(uint16 flag)
{
    return GetFlagName((EVEItemFlags)flag);
}

const char* StaticDataMgr::GetFlagName(EVEItemFlags flag)
{
    switch (flag) {
        case flagNone:                          return "AutoFit";
        case flagWallet:                        return "Wallet";
        //case flagFactory:                       return "Factory";
        case flagWardrobe:                      return "Wardrobe";
        case flagHangar:                        return "Hangar";
        case flagCargoHold:                     return "Cargo Hold";
        case flagBriefcase:                     return "Briefcase";
        case flagSkill:                         return "Skill";
        case flagReward:                        return "Reward";
        case flagConnected:                     return "Connected";
        case flagDisconnected:                  return "Disconnected";
        case flagLowSlot0:                      return "First Low Slot";
        case flagLowSlot1:                      return "Second Low Slot";
        case flagLowSlot2:                      return "Third Low Slot";
        case flagLowSlot3:                      return "Fourth Low Slot";
        case flagLowSlot4:                      return "Fifth Low Slot";
        case flagLowSlot5:                      return "Sixth Low Slot";
        case flagLowSlot6:                      return "Seventh Low Slot";
        case flagLowSlot7:                      return "Eighth Low Slot";
        case flagMidSlot0:                      return "First Mid Slot";
        case flagMidSlot1:                      return "Second Mid Slot";
        case flagMidSlot2:                      return "Third Mid Slot";
        case flagMidSlot3:                      return "Fourth Mid Slot";
        case flagMidSlot4:                      return "Fifth Mid Slot";
        case flagMidSlot5:                      return "Sixth Mid Slot";
        case flagMidSlot6:                      return "Seventh Mid Slot";
        case flagMidSlot7:                      return "Eighth Mid Slot";
        case flagHiSlot0:                       return "First Hi Slot";
        case flagHiSlot1:                       return "Second Hi Slot";
        case flagHiSlot2:                       return "Third Hi Slot";
        case flagHiSlot3:                       return "Fourth Hi Slot";
        case flagHiSlot4:                       return "Fifth Hi Slot";
        case flagHiSlot5:                       return "Sixth Hi Slot";
        case flagHiSlot6:                       return "Seventh Hi Slot";
        case flagHiSlot7:                       return "Eighth Hi Slot";
        case flagFixedSlot:                     return "Fixed Slot";
        case flagFactoryBlueprint:              return "Factory Blueprint";
        case flagFactoryMinerals:               return "Factory Minerals";
        case flagFactoryOutput:                 return "Factory Output";
        case flagFactoryActive:                 return "Factory Active";
        //case flagPromenadeSlot1:                return "PromenadeSlot1";
        case flagCapsule:                       return "Capsule";
        case flagPilot:                         return "Pilot";
        case flagPassenger:                     return "Passenger";
        case flagBoardingGate:                  return "Boarding Gate";
        case flagCrew:                          return "Crew";
        case flagSkillInTraining:               return "Skill In Training";
        case flagCorpMarket:                    return "Corp Market";
        case flagLocked:                        return "Locked";
        case flagUnlocked:                      return "Unlocked";
        case flagOffice:                        return "Office";
        case flagImpounded:                     return "Impounded";
        case flagProperty:                      return "Property";
        //case flagDelivery:                      return "Delivery";
        case flagBonus:                         return "Bonus";
        case flagDroneBay:                      return "Drone Bay";
        case flagBooster:                       return "Booster";
        case flagImplant:                       return "Implant";
        case flagShipHangar:                    return "Ship Hangar";
        case flagShipOffline:                   return "Ship Offline";
        case flagRigSlot0:                      return "First Rig Slot";
        case flagRigSlot1:                      return "Second Rig Slot";
        case flagRigSlot2:                      return "Third Rig Slot";
        case flagRigSlot3:                      return "Fourth Rig Slot";
        case flagRigSlot4:                      return "Fifth Rig Slot";
        case flagRigSlot5:                      return "Sixth Rig Slot";
        case flagRigSlot6:                      return "Seventh Rig Slot";
        case flagRigSlot7:                      return "Eighth Rig Slot";
        case flagFactoryOperation:              return "Factory Operation";
        case flagCorpHangar2:                   return "CorpHangar2";
        case flagCorpHangar3:                   return "CorpHangar3";
        case flagCorpHangar4:                   return "CorpHangar4";
        case flagCorpHangar5:                   return "CorpHangar5";
        case flagCorpHangar6:                   return "CorpHangar6";
        case flagCorpHangar7:                   return "CorpHangar7";
        case flagSecondaryStorage:              return "Secondary Storage";
        case flagCaptainsQuarters:              return "Captains Quarters";
        case flagWisPromenade:                  return "Promenade";
        //case flagWorldSpace:                  return "WorldSpace";
        case flagSubSystem0:                    return "First SubSystem";
        case flagSubSystem1:                    return "Second SubSystem";
        case flagSubSystem2:                    return "Third SubSystem";
        case flagSubSystem3:                    return "Fourth SubSystem";
        case flagSubSystem4:                    return "Fifth SubSystem";
        case flagSubSystem5:                    return "Sixth SubSystem";
        case flagSubSystem6:                    return "Seventh SubSystem";
        case flagSubSystem7:                    return "Eighth SubSystem";
        case flagFuelBay:                       return "Fuel Bay";
        case flagOreHold:                       return "Ore Hold";
        case flagGasHold:                       return "Gas Hold";
        case flagMineralHold:                   return "Mineral Hold";
        case flagSalvageHold:                   return "Salvage Hold";
        case flagShipHold:                      return "Ship Hold";
        case flagSmallShipHold:                 return "Small Ship Hold";
        case flagMediumShipHold:                return "Medium Ship Hold";
        case flagLargeShipHold:                 return "Large Ship Hold";
        case flagIndustrialShipHold:            return "Industrial Ship Hold";
        case flagAmmoHold:                      return "Ammunition Hold";
        case flagStructureActive:               return "Structure Active";
        case flagStructureInactive:             return "Structure Inactive";
        case flagJunkyardReprocessed:           return "Junkyard Reprocessed";
        case flagJunkyardTrashed:               return "Junkyard Trashed";
        case flagCommandCenterHold:             return "Command Center Hold";
        case flagPlanetaryCommoditiesHold:      return "Planetary Commodities Hold";
        case flagPlanetSurface:                 return "Planet Surface";
        case flagMaterialBay:                   return "Material Bay";
        case flagDustCharacterBackpack:         return "DustCharacterBackpack";
        case flagDustCharacterBattle:           return "DustCharacterBattle";
        case flagQuafeBay:                      return "Quafe Bay";
        case flagFleetHangar:                   return "Fleet Hangar";
        case flagResearchFacilitySlotFirst:     return "ResearchFacilitySlotFirst";
        case flagResearchFacilitySlotLast:      return "ResearchFacilitySlotLast";
        case flagMissile:                       return "Missile";
        case flagClone:                         return "Clone";
        case flagIllegal:                       return "Illegal";
    }
    return "Undefined";
}

uint32 StaticDataMgr::GetWreckFaction(uint32 typeID)
{
    // these will need to be separated and updated after detailed salvage table is completed
    switch(typeID) {
        case 26469:  //   Amarr Battlecruiser Wreck
        case 26470:  //   Amarr Battleship Wreck
        case 26472:  //   Amarr Carrier Wreck
        case 26473:  //   Amarr Cruiser Wreck
        case 26474:  //   Amarr Destroyer Wreck
        case 26475:  //   Amarr Dreadnought Wreck
        case 26476:  //  Amarr Elite Battlecruiser Wreck
        case 26477:  //   Amarr Elite Battleship Wreck
        case 26478:  //   Amarr Elite Cruiser Wreck
        case 26479:   //  Amarr Elite Destroyer Wreck
        case 26480:   //  Amarr Elite Frigate Wreck
        case 26481:   //  Amarr Elite Industrial Wreck
        case 26482:   //  Amarr Elite Mining Barge Wreck
        case 26483:   //  Amarr Freighter Wreck
        case 26484:   //  Amarr Frigate Wreck
        case 26485:  //  Amarr Industrial Wreck
        case 26486:   //  Amarr Mining Barge Wreck
        case 26487:   //  Amarr Supercarrier Wreck
        case 26488:   //  Amarr Rookie ship Wreck
        case 26489:   //  Amarr Shuttle Wreck
        case 26490:   //  Amarr Titan Wreck
        case 27050:   //  Amarr Large Wreck
        case 27051:   // Amarr Medium Wreck
        case 27052:   //  Amarr Small Wreck
        case 29033:   // Amarr Elite Freighter Wreck
        case 27927:  //  Mission Amarr Carrier Wreck
        case 30822: { //   Amarr Advanced Cruiser Wreck
            return factionAmarr;
        } break;

        case 26491:  //    Caldari Battlecruiser Wreck
        case 26492:  //    Caldari Battleship Wreck
        case 26494:  //    Caldari Carrier Wreck
        case 26495:  //   Caldari Cruiser Wreck
        case 26496:  //    Caldari Destroyer Wreck
        case 26497:  //    Caldari Dreadnought Wreck
        case 26498:  //    Caldari Elite Battlecruiser Wreck
        case 26499:  //   Caldari Elite Battleship Wreck
        case 26500:  //   Caldari Elite Cruiser Wreck
        case 26501:  //   Caldari Elite Destroyer Wreck
        case 26502:  //   Caldari Elite Frigate Wreck
        case 26503:  //   Caldari Elite Industrial Wreck
        case 26504:  //   Caldari Elite Mining Barge Wreck
        case 26505:  //    Caldari Freighter Wreck
        case 26506:  //    Caldari Frigate Wreck
        case 26507:  //    Caldari Industrial Wreck
        case 26508:  //    Caldari Mining Barge Wreck
        case 26509:  //    Caldari Supercarrier Wreck
        case 26510:  //   Caldari Rookie ship Wreck
        case 26511:  //    Caldari Shuttle Wreck
        case 26512:  //    Caldari Titan Wreck
        case 27926:  //    Mission Caldari Carrier Wreck
        case 30823:  //    Caldari Advanced Cruiser Wreck
        case 29034:  //   Caldari Elite Freighter Wreck
        case 27047:  //    Caldari Large Wreck
        case 27048:  //    Caldari Medium Wreck
        case 27049: {  //    Caldari Small Wreck
            return factionCaldari;
        } break;

        case 29035:  //    Gallente Elite Freighter Wreck
        case 30824:  //    Gallente Advanced Cruiser Wreck
        case 27929:  //    Mission Gallente Carrier Wreck
        case 27053:  //    Gallente Large Wreck
        case 27054:  //    Gallente Medium Wreck
        case 27055:  //    Gallente Small Wreck
        case 26513:  //    Gallente Battlecruiser Wreck
        case 26514:  //    Gallente Battleship Wreck
        case 26516:  //    Gallente Carrier Wreck
        case 26517:  //    Gallente Cruiser Wreck
        case 26518:  //    Gallente Destroyer Wreck
        case 26519:  //    Gallente Dreadnought Wreck
        case 26520:  //    Gallente Elite Battlecruiser Wreck
        case 26521:  //    Gallente Elite Battleship Wreck
        case 26522:  //    Gallente Elite Cruiser Wreck
        case 26523:  //    Gallente Elite Destroyer Wreck
        case 26524:  //    Gallente Elite Frigate Wreck
        case 26525:  //    Gallente Elite Industrial Wreck
        case 26526:  //    Gallente Elite Mining Barge Wreck
        case 26527:  //    Gallente Freighter Wreck
        case 26528:  //    Gallente Frigate Wreck
        case 26529:  //    Gallente Industrial Wreck
        case 26530:  //    Gallente Mining Barge Wreck
        case 26531:  //    Gallente Supercarrier Wreck
        case 26532:  //    Gallente Rookie ship Wreck
        case 26533:  //    Gallente Shuttle Wreck
        case 26534: { //   Gallente Titan Wreck
            return factionGallente;
        } break;

        case 26535:  //    Minmatar Battlecruiser Wreck
        case 26536:  //    Minmatar Battleship Wreck
        case 26538:  //    Minmatar Carrier Wreck
        case 26539:  //    Minmatar Cruiser Wreck
        case 26540:  //    Minmatar Destroyer Wreck
        case 26541:  //    Minmatar Dreadnought Wreck
        case 26542:  //    Minmatar Elite Battlecruiser Wreck
        case 26543:  //    Minmatar Elite Battleship Wreck
        case 26544:  //    Minmatar Elite Cruiser Wreck
        case 26545:  //    Minmatar Elite Destroyer Wreck
        case 26546:  //    Minmatar Elite Frigate Wreck
        case 26547:  //    Minmatar Elite Industrial Wreck
        case 26548:  //    Minmatar Elite Mining Barge Wreck
        case 29036:  //    Minmatar Elite Freighter Wreck
        case 26549:  //    Minmatar Freighter Wreck
        case 26550:  //    Minmatar Frigate Wreck
        case 26551:  //    Minmatar Industrial Wreck
        case 26552:  //    Minmatar Mining Barge Wreck
        case 26553:  //    Minmatar Supercarrier Wreck
        case 26554:  //    Minmatar Rookie ship Wreck
        case 26555:  //    Minmatar Shuttle Wreck
        case 26556:  //    Minmatar Titan Wreck
        case 27928:  //    Mission Minmatar Carrier Wreck
        case 30825:  //    Minmatar Advanced Cruiser Wreck
        case 27041:  //    Minmatar Large Wreck
        case 27042:  //    Minmatar Medium Wreck
        case 27043: { //   Minmatar Small Wreck
            return factionMinmatar;
        } break;

        case 26972:  //    Faction Drone Wreck   - faction police drones
        case 26939:  //   CONCORD Large Wreck
        case 26940:  //    CONCORD Medium Wreck
        case 26941: { //    CONCORD Small Wreck
            return factionCONCORD;
        } break;

        case 27044:  //    Khanid Large Wreck
        case 27045:  //    Khanid Medium Wreck
        case 27046: { //   Khanid Small Wreck
            return factionKhanid;
        } break;

        case 27056:  //    Thukker Large Wreck
        case 27057:  //    Thukker Medium Wreck
        case 27058: { //    Thukker Small Wreck
            return factionThukker;
        } break;

        case 27060:  //   Mordu Large Wreck
        case 27061:  //   Mordu Medium Wreck
        case 27062: { //    Mordu Small Wreck
            return factionMordusLegion;
        } break;

        case 28603:  //   Rorqual Wreck
        case 29639: { //    Orca Wreck
            return factionORE;
        } break;

        case 30457:  //    Sleeper Small Advanced Wreck
        case 30458:  //    Sleeper Medium Advanced Wreck
        case 30459:  //    Sleeper Large Wreck
        case 30484:  //    Sleeper Small Basic Wreck
        case 30485:  //    Sleeper Small Intermediate Wreck
        case 30492:  //   Sleeper Medium Basic Wreck
        case 30493:  //    Sleeper Medium Intermediate Wreck
        case 30494:  //   Sleeper Large Basic Wreck
        case 30495:  //   Sleeper Large Intermediate Wreck
        case 30496: { //    Sleeper Large Advanced Wreck
            return factionSleepers;
        } break;

        case 26561:  //   Angel Small Wreck
        case 26562:  //   Angel Medium Wreck
        case 26563:  //   Angel Large Wreck
        case 26564:  //   Angel Small Commander Wreck
        case 26699:  //   Angel Medium Commander Wreck
        case 26565:  //   Angel Large Commander Wreck
        case 26566: { //   Angel Officer Wreck
            return factionAngel;
        } break;

        case 26567:  //   Blood Small Wreck
        case 26568:  //   Blood Medium Wreck
        case 26569:  //   Blood Large Wreck
        case 26570:  //   Blood Small Commander Wreck
        case 26571:  //   Blood Medium Commander Wreck
        case 26700:  //   Blood Large Commander Wreck
        case 26572: { //   Blood Officer Wreck
            return factionBloodRaider;
        } break;

        case 26573:  //   Guristas Small Wreck
        case 26574:  //   Guristas Medium Wreck
        case 26575:  //   Guristas Large Wreck
        case 26576:  //   Guristas Small Commander Wreck
        case 26577:  //   Guristas Medium Commander Wreck
        case 26701:  //   Guristas Large Commander Wreck
        case 26578: { //   Guristas Officer Wreck
            return factionGuristas;
        } break;

        case 26579:  //   Sanshas Small Wreck
        case 26580:  //   Sanshas Medium Wreck
        case 26581:  //   Sanshas Large Wreck
        case 26582:  //   Sanshas Small Commander Wreck
        case 26583:  //   Sanshas Medium Commander Wreck
        case 26702:  //   Sanshas Large Commander Wreck
        case 26584:  //   Sanshas Officer Wreck
        case 3260: { //   Sanshas Supercarrier Wreck
            return factionSanshas;
        } break;

        case 26585:  //   Serpentis Small Wreck
        case 26586:  //   Serpentis Medium Wreck
        case 26587:  //   Serpentis Large Wreck
        case 26588:  //   Serpentis Small Commander Wreck
        case 26589:  //   Serpentis Medium Commander Wreck
        case 26703:  //   Serpentis Large Commander Wreck
        case 26590: { //   Serpentis Officer Wreck
            return factionSerpentis;
        } break;

        case 26591:  //   Rogue Small Wreck
        case 26592:  //   Rogue Medium Wreck
        case 26593:  //   Rogue Large Wreck
        case 26594:  //   Rogue Elite Small Wreck
        case 26595:  //   Rogue Elite Medium Wreck
        case 26596:  //   Rogue Officer Wreck
        case 28221:  //   Rogue Large Commander Wreck
        case 28222:  //   Rogue Medium Commander Wreck
        case 28223: { //   Rogue Small Commander Wreck
            return factionRogueDrones;
        } break;

        // generic wrecks
        case 26468:  //   Capsule Wreck
        case 26557:  //   Frigate Wreck
        case 26558:  //   Cruiser Wreck
        case 26559:  //   Battleship Wreck
        case 26918:  //   Overseer Frigate Wreck
        case 26919:  //   Overseer Cruiser Wreck
        case 26920:  //   Overseer Battleship Wreck
        case 27202:  //   Convoy Wreck
        case 27286:  //   Pirate Drone Wreck
        case 26560: { //   Pirate Wreck
            return factionUnknown;
        } break;
    }

    // safe default
    return factionUnknown;

    /*
     *    28255 :  //   Mission Faction Freighter Wreck
     *    29347:  //    Mission Faction Vessel Wreck
     *    29365:  //    Mission Faction Industrial Wreck
     */
}

// Add a new outpost to the staticDataMgr
void StaticDataMgr::AddOutpost(StationData &stData)
{
    // Update m_stationCount
    std::map<uint32, uint8>::iterator itr = m_stationCount.lower_bound(stData.systemID);
    if (itr != m_stationCount.end() && !(m_stationCount.key_comp()(stData.systemID, itr->first)))
    {
        itr->second = itr->second + 1;
    }
    else
    {
        m_stationCount.emplace(stData.systemID, 1);
    }

    // Update m_stationRegion
    if (m_stationRegion.find(stData.stationID) == m_stationRegion.end()) {
        m_stationRegion.emplace(stData.stationID, stData.regionID);
    }

    // Update m_stationConstellation
    if (m_stationConst.find(stData.stationID) == m_stationConst.end()) {
        m_stationConst.emplace(stData.stationID, stData.constellationID);
    }

    // Update m_stationSystem
    if (m_stationSystem.find(stData.stationID) == m_stationSystem.end()) {
        m_stationSystem.emplace(stData.stationID, stData.systemID);
    }
}