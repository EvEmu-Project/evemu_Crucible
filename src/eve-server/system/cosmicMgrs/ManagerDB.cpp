
 /**
  * @name ManagerDB.cpp
  *   cosmic manager database methods
  * @Author:         Allan
  * @date:   17 April 2016
  */


#include "eve-server.h"

#include "system/Asteroid.h"
#include "system/cosmicMgrs/ManagerDB.h"


void ManagerDB::GetCategoryData(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT categoryID, categoryName, description, published FROM invCategories"))
        codelog(DATABASE__ERROR, "Error in GetCategoryData query: %s.", res.error.c_str());

    _log(DATABASE__RESULTS, "GetCategoryData returned %lu items", res.GetRowCount());
}

void ManagerDB::GetGroupData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  groupID,"
        "  categoryID,"
        "  groupName,"
        "  description,"
        "  useBasePrice,"
        "  allowManufacture,"
        "  allowRecycler,"
        "  anchored,"
        "  anchorable,"
        "  fittableNonSingleton,"
        "  published "
        " FROM invGroups "))
        codelog(DATABASE__ERROR, "Error in GetGroupData query: %s.", res.error.c_str());

    _log(DATABASE__RESULTS, "GetGroupData returned %lu items", res.GetRowCount());
}

void ManagerDB::GetTypeData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  t.typeID,"
        "  t.groupID,"
        "  t.typeName,"
        "  t.description,"
        "  t.radius,"
        "  t.mass,"
        "  t.volume,"
        "  t.capacity,"
        "  t.portionSize,"
        "  t.raceID,"
        "  t.basePrice,"
        "  t.published,"
        "  t.marketGroupID,"
        "  t.chanceOfDuplicating,"
        "  m.metaGroupID"
        " FROM invTypes AS t"
        " LEFT JOIN invMetaTypes AS m USING (typeID)"))
        codelog(DATABASE__ERROR, "Error in GetTypeData query: %s.", res.error.c_str());

    _log(DATABASE__RESULTS, "GetTypeData returned %lu items", res.GetRowCount());
}

void ManagerDB::GetSkillList(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (SELECT groupID FROM invGroups WHERE categoryID = 16)"))
        codelog(DATABASE__ERROR, "Error in GetSkillList query: %s", res.error.c_str());

    _log(DATABASE__RESULTS, "GetSkillList returned %lu items", res.GetRowCount());
}

void ManagerDB::GetAttributeTypes(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT attributeID, attributeName, attributeCategory, displayName, categoryID FROM dgmAttributeTypes"))
        codelog(DATABASE__ERROR, "Error in GetAttributeTypes query: %s", res.error.c_str());

    _log(DATABASE__RESULTS, "GetAttributeTypes returned %lu items", res.GetRowCount());
}

void ManagerDB::GetTypeAttributes(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, attributeID, valueInt, valueFloat FROM dgmTypeAttributes"))
        codelog(DATABASE__ERROR, "Error in GetTypeAttributes query: %s", res.error.c_str());

    _log(DATABASE__RESULTS, "GetTypeAttributes returned %lu items", res.GetRowCount());
}

void ManagerDB::LoadNPCCorpFactionData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT corporationID, factionID FROM crpNPCCorporations" ))
        codelog(DATABASE__ERROR, "Error in LoadCorpFactionData query: %s", res.error.c_str());

    _log(DATABASE__RESULTS, "LoadCorpFactionData returned %lu items", res.GetRowCount());
}

PyObject *ManagerDB::GetEntryTypes() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT entryTypeID, entryTypeName, entryTypeNameID FROM jnlEntryTypeIDs"))    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetEntryTypes returned %lu items", res.GetRowCount());

    return DBResultToRowset(res);
}

PyObject *ManagerDB::GetKeyMap() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT keyID, keyType, keyName, keyNameID FROM actKeyTypes")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetKeyMap returned %lu items", res.GetRowCount());

    return DBResultToRowset(res);
}

PyObject* ManagerDB::GetBillTypes() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT billTypeID, billTypeName, description FROM billTypes")) {
        codelog(DATABASE__ERROR, "Failed to query bill types: %s.", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetBillTypes returned %lu items", res.GetRowCount());

    return DBResultToRowset(res);
}

PyObjectEx* ManagerDB::GetAgents() {
    // NOTE:  havent found data for agents in space yet....still looking.
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    agt.agentID,"
        "    agt.agentTypeID,"
        "    agt.divisionID,"
        "    agt.level,"
        "    agt.quality,"
        "    agt.corporationID,"
        "    chr.stationID,"
        "    chr.gender,"
        "    bl.bloodlineID"
        " FROM agtAgents AS agt"
        " LEFT JOIN chrNPCCharacters AS chr ON chr.characterID = agt.agentID"
        " LEFT JOIN bloodlineTypes AS bl ON bl.typeID = chr.typeID"
    ))
    {
        codelog(DATABASE__ERROR, "Error in GetAgents query: %s", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetAgents returned %lu items", res.GetRowCount());
    return DBResultToCRowset(res);
}

PyObjectEx *ManagerDB::GetOperands() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT operandID, operandKey, description, format, arg1categoryID, arg2categoryID, resultCategoryID, pythonFormat FROM dgmOperands" )) {
        codelog(DATABASE__ERROR, "Error in GetOperands query: %s.", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetOperands returned %lu items", res.GetRowCount());

    return DBResultToCIndexedRowset(res, "operandID");
}

void ManagerDB::LoadCorpFactions(std::map<uint32, uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corporationID,factionID FROM crpNPCCorporations" )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultToUIntUIntDict(res, into);
}

void ManagerDB::LoadFactionStationCounts(std::map<uint32, uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT factionID, COUNT(DISTINCT staStations.stationID) FROM crpNPCCorporations"
        " LEFT JOIN staStations USING (corporationID)"
        " GROUP BY factionID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultToUIntUIntDict(res, into);
}

void ManagerDB::LoadFactionSystemCounts(std::map<uint32, uint32> &into) {
    DBQueryResult res;
    //this is not quite right, but its good enough.
    if (!sDatabase.RunQuery(res, "SELECT factionID, COUNT(solarSystemID) FROM mapSolarSystems GROUP BY factionID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultToUIntUIntDict(res, into);
}

void ManagerDB::LoadFactionRegions(std::map<int32, PyRep *> &into) {
    DBQueryResult res;
    //this is not quite right, but its good enough.
    if (!sDatabase.RunQuery(res, "SELECT factionID,regionID FROM mapRegions WHERE factionID IS NOT NULL ORDER BY factionID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }
    DBResultToIntIntlistDict(res, into);
}

void ManagerDB::LoadFactionConstellations(std::map<int32, PyRep *> &into) {
    DBQueryResult res;
    //this is not quite right, but its good enough.
    if (!sDatabase.RunQuery(res, "SELECT factionID,constellationID FROM mapConstellations WHERE factionID IS NOT NULL ORDER BY factionID" ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }
    DBResultToIntIntlistDict(res, into);
}

void ManagerDB::LoadFactionSolarSystems(std::map<int32, PyRep *> &into) {
    DBQueryResult res;
    //this is not quite right, but its good enough.
    if (!sDatabase.RunQuery(res, "SELECT factionID,solarSystemID FROM mapSolarSystems WHERE factionID IS NOT NULL ORDER BY factionID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }
    DBResultToIntIntlistDict(res, into);
}

void ManagerDB::LoadFactionRaces(std::map<int32, PyRep *> &into) {
    DBQueryResult res;
    //this is not quite right, but its good enough.
    if (!sDatabase.RunQuery(res, "SELECT factionID,raceID FROM facRaces WHERE factionID IS NOT NULL ORDER BY factionID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }
    DBResultToIntIntlistDict(res, into);
}

PyDict* ManagerDB::LoadNPCCorpInfo() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   crp.corporationName,"
        "   ncrp.corporationID,"
        "   ncrp.size, ncrp.extent, ncrp.solarSystemID,"
        "   ncrp.factionID, ncrp.sizeFactor, ncrp.stationCount, ncrp.stationSystemCount,"
        "   crp.stationID, crp.ceoID, chr.characterName AS ceoName"
        " FROM crpNPCCorporations AS ncrp"
        "  LEFT JOIN crpCorporation AS crp ON ncrp.corporationID = crp.corporationID"
        "  LEFT JOIN chrNPCCharacters AS chr ON ceoID=chr.characterID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToIntRowDict(res, 1);
}

PyObject* ManagerDB::GetNPCDivisions() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        " divisionID, divisionName, description, leaderType, divisionNameID, leaderTypeID"
        " FROM crpNPCDivisions" )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

void ManagerDB::GetSalvageGroups(DBQueryResult& res) {
    //facSalvage (factionID,itemID,itemName)
    if (!sDatabase.RunQuery(res, "SELECT factionID, itemID FROM facSalvage")) {
        codelog(DATABASE__ERROR, "Error in GetSalvageGroups query: %s", res.error.c_str());
        return;
    }
}

void ManagerDB::GetOreBySSC(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT systemSec, roidID, percent FROM roidDistribution"))
        codelog(DATABASE__ERROR, "Error in GetRoidDist query: %s", res.error.c_str());
}

void ManagerDB::GetSystemData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT mss.solarSystemID, mss.solarSystemName, mss.constellationID, mss.regionID, mss.securityClass, md.security, mss.factionID"
        " FROM mapSolarSystems AS mss"
        " LEFT JOIN mapDenormalize AS md ON (md.itemID = mss.solarSystemID)"
    ))
        codelog(DATABASE__ERROR, "Error in GetSystemData query: %s", res.error.c_str());
}

void ManagerDB::GetStaticData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT itemID, regionID, constellationID, solarSystemID, typeID, radius, x, y, z FROM mapDenormalize WHERE solarSystemID IS NOT NULL"))
        codelog(DATABASE__ERROR, "Error in GetStaticInfo query: %s", res.error.c_str());
}

void ManagerDB::GetAgentLocation(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT agentID, locationID FROM agtAgents"))
        codelog(DATABASE__ERROR, "Error in GetAgentLocation query: %s", res.error.c_str());
}

void ManagerDB::GetMoonResouces(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID,volume FROM invTypes WHERE groupID = %u", EVEDB::invGroups::Moon_Materials))
        codelog(DATABASE__ERROR, "Error in GetMoonResouces query: %s", res.error.c_str());
}

void ManagerDB::SaveAnomaly(CosmicSignature& sig)
{// sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO sysSignatures"
        " (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)"
        " VALUES ('%s', %u, %u, '%s', %u, %u, %u, %u, %u, %f, %f, %f)", \
            sig.sigID.c_str(), sig.sigItemID, sig.dungeonType, sig.sigName.c_str(), sig.systemID, sig.sigTypeID, sig.sigGroupID, \
            sig.scanGroupID, sig.scanAttributeID, sig.position.x, sig.position.y, sig.position.z )) {
        _log(DATABASE__ERROR, "SaveAnomaly - unable to save dungeon");
        }
}

void ManagerDB::GetAnomalyList(DBQueryResult& res)
{// sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    if (!sDatabase.RunQuery(res,
        "SELECT sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z"
        " FROM sysSignatures"
        " ORDER BY systemid")) {
        _log(DATABASE__ERROR, "Error in GetAnomalyList query: %s", res.error.c_str());
        }
}

void ManagerDB::GetAnomaliesBySystem(uint32 systemID, DBQueryResult& res)
{// sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    if (!sDatabase.RunQuery(res,
        "SELECT sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z"
        " FROM sysSignatures"
        " WHERE systemID = %u", systemID)) {
        _log(DATABASE__ERROR, "Error in GetAnomalyList query: %s", res.error.c_str());
        }
}

GPoint ManagerDB::GetAnomalyPos(const std::string& string)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT x,y,z FROM sysSignatures WHERE sigID = '%s'", string.c_str()))
        _log(DATABASE__ERROR, "Error in GetAnomalyPos query: %s", res.error.c_str());

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "GetAnomalyPos query returned no items");
        return NULL_ORIGIN;
    }

    GPoint pos(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    return pos;
}

// these next two arent used yet.  dont remember what they were for.  persistant shit for restart?
void ManagerDB::GetSystemAnomalies(uint32 systemID, DBQueryResult& res)
{// sysSignatures (sigTypeID,scanGroupID,sigGroupID,scanAttributeID,sigName,sigID,x,y,z)
    if (!sDatabase.RunQuery(res,
        "SELECT sigTypeID, scanGroupID, sigGroupID, scanAttributeID, sigName, sigID, x, y, z"
        " FROM sysSignatures"
        " WHERE systemID = %u", systemID)) {
        _log(DATABASE__ERROR, "Error in GetSystemAnomalies query: %s", res.error.c_str());
        }
}

void ManagerDB::GetSystemAnomalies(uint32 systemID, std::vector< CosmicSignature >& sigs)
{// sysSignatures (sigID,sigItemID,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    // not used yet.
}

void ManagerDB::GetRegionFaction(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT regionID, factionID FROM mapRegions WHERE factionID != 0"))
        _log(DATABASE__ERROR, "Error in GetRegionFactionInfo query: %s", res.error.c_str());
}

void ManagerDB::GetRegionRatFaction(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0"))
        _log(DATABASE__ERROR, "Error in GetRegionFactionInfo query: %s", res.error.c_str());
}

void ManagerDB::GetFactionGroups(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT shipClass, groupID, factionID FROM npcClassGroup"))
        _log(DATABASE__ERROR, "Error in GetFactionGroups query: %s", res.error.c_str());
}

void ManagerDB::GetSpawnClasses(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT type, sub, f, af, d, c, ac, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM npcSpawnClass"))
        _log(DATABASE__ERROR, "Error in GetSpawnClasses query: %s", res.error.c_str());
}

// this is loaded in static data, so performance isnt a priority
void ManagerDB::GetGroupTypeIDs(uint8 shipClass, uint16 groupID, uint32 factionID, DBQueryResult& res) {
    // this now gets advanced frigates and cruisers based on faction (best i could think of)
    std::string name = "";

    // there are unused types in the data for these...just sayin
    switch (factionID) {
        case factionAngel: {  //  Cyber
            if ((shipClass == 2) or (shipClass == 5) or (shipClass == 16) or (shipClass == 19)) { // 2 = adv frig, 5 = adv cruiser, 16 = adv frig, 19 = adv cruiser
                name = "AND typeName LIKE '%Arch%'";
                name = "AND typeName NOT LIKE '%Cyber%'";
            } else if ((shipClass == 1) or (shipClass == 4) or (shipClass == 15) or (shipClass == 18)) { // 1 = frig, 4 = cruiser, 15 = frig, 18 = cruiser
                name = "AND typeName NOT LIKE '%Arch%'";
                name = "AND typeName NOT LIKE '%Cyber%'";
            }
        } break;
        case factionBloodRaider: {
            if ((shipClass == 2) or (shipClass == 5) or (shipClass == 16) or (shipClass == 19)) {
                name = "AND typeName LIKE '%Elder%'";
            } else if ((shipClass == 1) or (shipClass == 4) or (shipClass == 15) or (shipClass == 18)) {
                name = "AND typeName NOT LIKE '%Elder%'";
            }
        } break;
        case factionGuristas: {   //Outlaw
            if ((shipClass == 2) or (shipClass == 5) or (shipClass == 16) or (shipClass == 19)) {
                name = "AND typeName LIKE '%Dire%'";
                name = "AND typeName NOT LIKE '%Outlaw%'";
            } else if ((shipClass == 1) or (shipClass == 4) or (shipClass == 15) or (shipClass == 18)) {
                name = "AND typeName NOT LIKE '%Dire%'";
                name = "AND typeName NOT LIKE '%Outlaw%'";
            }
        } break;
        case factionSanshas: {
            if ((shipClass == 2) or (shipClass == 5) or (shipClass == 16) or (shipClass == 19)) {
                name = "AND typeName LIKE '%Loyal%'";
            } else if ((shipClass == 1) or (shipClass == 4) or (shipClass == 15) or (shipClass == 18)) {
                name = "AND typeName NOT LIKE '%Loyal%'";
            }
        } break;
        case factionSerpentis: {   //Crook
            if ((shipClass == 2) or (shipClass == 5) or (shipClass == 16) or (shipClass == 19)) {
                name = "AND typeName LIKE '%Guardian%'";
                name = "AND typeName NOT LIKE '%Crook%'";
            } else if ((shipClass == 1) or (shipClass == 4) or (shipClass == 15) or (shipClass == 18)) {
                name = "AND typeName NOT LIKE '%Guardian%'";
                name = "AND typeName NOT LIKE '%Crook%'";
            }
        } break;
        case factionRogueDrones: {
            if ((shipClass == 2) or (shipClass == 5) or (shipClass == 16) or (shipClass == 19)) {
                name = "AND typeName LIKE '%Strain%'";
            } else if ((shipClass == 1) or (shipClass == 4) or (shipClass == 15) or (shipClass == 18)) {
                name = "AND typeName NOT LIKE '%Strain%'";
            }
        } break;
    }

    if (!sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID = %u %s ORDER BY typeID", groupID, name.c_str()))
        _log(DATABASE__ERROR, "Error in GetGroupTypeIDs query: %s", res.error.c_str());
}

// called during startup/shutdown by ItemFactory
void ManagerDB::DeleteSpawnedRats()
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE '%%beltrat%%'");
}

void ManagerDB::CreateRoidItemID(ItemData& idata, AsteroidData& adata)
{
    DBerror err;
    sDatabase.RunQueryLID(err, adata.itemID,
        "INSERT INTO sysAsteroids (itemName,typeID,systemID,beltID,quantity,radius,x, y, z)"
        " VALUES ('%s', %u, %u, %u, %f, %f, %f, %f, %f)",
                          adata.itemName.c_str(), adata.typeID, adata.systemID, adata.beltID, adata.quantity, adata.radius,
                          adata.position.x, adata.position.y, adata.position.z );
}

bool ManagerDB::GetAsteroidData(uint32 asteroidID, AsteroidData& dbData)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT itemName, typeID, systemID, beltID, quantity, radius, x, y, z"
        " FROM sysAsteroids"
        " WHERE itemID = %u", asteroidID)) {
        _log(DATABASE__ERROR, "Error in GetAsteroidData query: %s", res.error.c_str());
            return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        dbData.itemID = asteroidID;
        dbData.itemName = row.GetText(0);
        dbData.typeID = row.GetInt(1);
        dbData.systemID = row.GetInt(2);
        dbData.beltID = row.GetInt(3);
        dbData.quantity = row.GetDouble(4);
        dbData.radius = row.GetDouble(5);
        GPoint pos(row.GetDouble(6), row.GetDouble(7), row.GetDouble(8));
        dbData.position = pos;
        return true;
    } else {
        dbData = AsteroidData();
    }

    return false;
}

bool ManagerDB::LoadSystemRoids(uint32 systemID, uint32& beltID, std::vector< AsteroidData >& into)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT itemID, itemName, typeID, systemID, beltID, quantity, radius, x, y, z"
        " FROM sysAsteroids"
        " WHERE systemID = %u"
        "  AND beltID = %u", systemID, beltID)) {
        _log(DATABASE__ERROR, "Error in LoadSystemRoids query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "LoadSystemRoids returned %lu items", res.GetRowCount());
    DBResultRow row;
    while(res.GetRow(row)) {
        AsteroidData entry = AsteroidData();
        entry.itemID = row.GetInt(0);
        entry.itemName = row.GetText(1);
        entry.typeID = row.GetInt(2);
        entry.systemID = row.GetInt(3);
        entry.beltID = row.GetInt(4);
        entry.quantity = row.GetDouble(5);
        entry.radius = row.GetDouble(6);
        GPoint pos(row.GetDouble(7), row.GetDouble(8), row.GetDouble(9));
        entry.position = pos;
        into.push_back(entry);
    }

    return !into.empty();
}

void ManagerDB::RemoveAsteroid(uint32 asteroidID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM sysAsteroids WHERE itemID = %u", asteroidID);
}

void ManagerDB::ClearAsteroids()
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM sysAsteroids WHERE 1");
    sDatabase.RunQuery(err, "ALTER TABLE sysAsteroids auto_increment = 450000000");
}

void ManagerDB::SaveRoid(AsteroidData& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE sysAsteroids"
        " SET quantity = %f,"
        " radius = %f"
        " WHERE itemID = %u",
        data.quantity, data.radius, data.itemID))
    {
        _log(DATABASE__ERROR, "SaveSystemRoids - unable to save roids - %s", err.c_str());
    }
}

void ManagerDB::SaveSystemRoids(uint32 systemID, std::vector< AsteroidData >& roids)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO sysAsteroids";
    Inserts << " (itemID,itemName,typeID,systemID,beltID,quantity,radius,x, y, z)";
    bool first = true;
    for (auto cur : roids) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        // itemID and attributeID keys.
        Inserts << "(" << cur.itemID << ", '" << cur.itemName << "', " << cur.typeID << ", " << systemID << ", " << cur.beltID << ", ";
        Inserts << cur.quantity << ", " << cur.radius << ", " << std::to_string(cur.position.x) << ", " << std::to_string(cur.position.y);
        Inserts << ", " << std::to_string(cur.position.z) << ")";
    }
    // did we get at least 1 insert?
    if (!first) {
        // finish creating the command.
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "radius=VALUES(radius)";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveSystemRoids - unable to save roids - %s", err.c_str());
    }
}

void ManagerDB::GetWHSystemClass(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT locationID, wormholeClassID FROM mapLocationWormholeClasses"))
        _log(DATABASE__ERROR, "Error in GetWHSystemClass query: %s", res.error.c_str());
}

void ManagerDB::GetDunEntryData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunEntryID, dunEntryName, xpos, ypos, zpos FROM dunEntryData"))
        _log(DATABASE__ERROR, "Error in GetDunRoomData query: %s", res.error.c_str());
}

void ManagerDB::GetDunGroupData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT d.dunGroupID, d.itemTypeID, d.itemGroupID, t.typeName, t.groupID, g.categoryID, t.radius, d.xpos, d.ypos, d.zpos"
        " FROM dunGroupData AS d"
        "  LEFT JOIN invTypes AS t ON d.itemTypeID = t.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID" ))
    {
        _log(DATABASE__ERROR, "Error in GetDunGroupData query: %s", res.error.c_str());
    }
}

void ManagerDB::GetDunRoomData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomID, dunGroupID, xpos, ypos, zpos FROM dunRoomData"))
        _log(DATABASE__ERROR, "Error in GetDunRoomData query: %s", res.error.c_str());
}

void ManagerDB::GetDunRoomInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomID, dunRoomType, dunRoomCategory, dunRoomSpawnID, dunRoomSpawnType FROM dunRoomInfo"))
        _log(DATABASE__ERROR, "Error in GetDunRoomInfo query: %s", res.error.c_str());
}

void ManagerDB::GetDunSpawnInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomSpawnID, dunRoomSpawnType, xpos, ypos, zpos FROM dunRoomSpawnInfo"))
        _log(DATABASE__ERROR, "Error in GetDunSpawnInfo query: %s", res.error.c_str());
}

void ManagerDB::GetDunTemplates(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunTemplateID, dunTemplateName, dunEntryID, dunSpawnID, dunRoomID, dunTemplateDescription, dunTemplateFactionID FROM dunTemplates"))
        _log(DATABASE__ERROR, "Error in GetDunTemplates query: %s", res.error.c_str());
}

/*bool ManagerDB::GetSavedDungeons(uint32 systemID, std::vector< Dungeon::ActiveData >& into)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT systemID, state, dunTemplateID, dunExpiryTime, xpos, ypos, zpos"
        " FROM dunActive"   //Active Dungeons
        " WHERE systemID = %u", systemID))
    {
        _log(DATABASE__ERROR, "Error in GetSavedDungeons query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetSavedDungeons returned %lu items", res.GetRowCount());
    DBResultRow row;
    while(res.GetRow(row)) {
        Dungeon::ActiveData entry = Dungeon::ActiveData();
            entry.systemID = row.GetInt(0);
            entry.state = row.GetInt(1);
            entry.dunItemID = 0;
            entry.dunTemplateID = row.GetInt(2);
            entry.dunExpiryTime = row.GetInt64(3);
            entry.x = row.GetInt(4);
            entry.y = row.GetInt(5);
            entry.z = row.GetInt(6);
        into.push_back(entry);
    }

    return !into.empty();
}*/

/*void ManagerDB::SaveActiveDungeon(Dungeon::ActiveData& dun)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO dunActive" //Active Dungeons
        " (systemID, dungeonID, state, dunTemplateID, dunExpiryTime, xpos, ypos, zpos)"
        " VALUES "
        "(%u, %u, %u, %u, %lli, %f, %f, %f)",
        dun.systemID, dun.dunItemID, dun.state, dun.dunTemplateID, dun.dunExpiryTime, dun.x, dun.y, dun.z ))
    {
        _log(DATABASE__ERROR, "SaveActiveDungeon - unable to save dungeon: %s", err.c_str());
    }
}*/

void ManagerDB::ClearDungeons()
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM dunActive WHERE 1");
    sDatabase.RunQuery(err, "DELETE FROM sysSignatures WHERE dungeonType != 6 AND 1");
    // anomaly items are all temp, except roids, so we may not need this...
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM entity WHERE customInfo LIKE 'Dungeon%%')");
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE 'Dungeon%%'");
}

void ManagerDB::ClearDungeons(uint32 systemID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM dunActive WHERE systemID = %u", systemID);
    sDatabase.RunQuery(err, "DELETE FROM sysSignatures WHERE dungeonType != 6 AND systemID = %u", systemID);
    // anomaly items are all temp, except roids, so we may not need this...
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM entity WHERE locationID = %u AND customInfo LIKE 'Dungeon%%')", systemID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE locationID = %u AND customInfo LIKE 'Dungeon%%'", systemID);
}

/*
 * SELECT timeStamp, timeSpan, pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket FROM srvStatisticData
 * SELECT month, pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket FROM srvStatisticHistory
 */
void ManagerDB::GetStatisticData(DBQueryResult& res, int64 starttime)
{
    if (!sDatabase.RunQuery(res,
        "SELECT pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket, probesLaunched, sitesScanned"
        " FROM srvStatisticData"
        " WHERE timeStamp > %lli", starttime))
        codelog(DATABASE__ERROR, "Error in GetStatisticData query: %s", res.error.c_str());
}

void ManagerDB::SaveStatisticData(StatisticData& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO srvStatisticData"
        " (timeStamp, timeSpan, pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket, probesLaunched, sitesScanned)"
        " VALUES "
        "(%f, %u, %u, %u, %u, %u, %f, %f, %f, %f, %u, %u)", GetFileTimeNow(),
        data.span, data.pcShots, data.pcMissiles, data.ramJobs, data.shipsSalvaged, data.pcBounties, data.npcBounties, data.oreMined, data.iskMarket, data.probesLaunched, data.sitesScanned ))
    {
        _log(DATABASE__ERROR, "SaveStatisticData - unable to save data: %s", err.c_str());
    }
}

void ManagerDB::UpdateStatisticHistory(StatisticData& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE srvStatisticHistory"
        " SET pcShots=%u, pcMissiles=%u, ramJobs=%u, shipsSalvaged=%u, pcBounties=%f, npcBounties=%f, oreMined=%f, iskMarket=%f, probesLaunched=%u, sitesScanned=%u"
        " WHERE idx = 1",
        data.pcShots, data.pcMissiles, data.ramJobs, data.shipsSalvaged, data.pcBounties, data.npcBounties,
        data.oreMined, data.iskMarket, data.probesLaunched, data.sitesScanned ))
    {
        _log(DATABASE__ERROR, "srvStatisticHistory - unable to save data: %s", err.c_str());
    }
}

void ManagerDB::GetWHClassDestinations(uint32 systemClass, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
    "SELECT typeID "
    "FROM mapWormholeDestinationClasses "
    "WHERE class = %u", systemClass))
    {
        codelog(DATABASE__ERROR, "Error in GetWHDestinationClass query: %s", res.error.c_str());
    }
}

void ManagerDB::GetWHClassSystems(uint32 systemClass, DBQueryResult& res)
{
    // For WH / nullsec systems, use this query
    if (systemClass <= 6 or systemClass == 9) {
        if (!sDatabase.RunQuery(res,
        "SELECT solarSystemID,wormholeClassID FROM mapSolarSystems mss "
        "INNER JOIN mapLocationWormholeClasses mlwc ON mss.regionID = mlwc.locationID "
        "AND wormholeClassID = %u", systemClass))
        {
            codelog(DATABASE__ERROR, "Error in GetWHClassSystems(wh/ns) query: %s", res.error.c_str());
        }
    }
    // For lowsec systems, use solarSystemID as mapped from mlwc table
    else if (systemClass == 8) {
        if (!sDatabase.RunQuery(res,
        "SELECT solarSystemID,wormholeClassID FROM mapSolarSystems mss "
        "INNER JOIN mapLocationWormholeClasses mlwc ON mss.solarSystemID = mlwc.locationID "
        "AND wormholeClassID = %u", systemClass))
        {
            codelog(DATABASE__ERROR, "Error in GetWHClassSystems(ls) query: %s", res.error.c_str());
        }
    }
    // For highsec systems, use all systems in the highsec regions excepting those with low security status
    else if (systemClass == 7) {
        if (!sDatabase.RunQuery(res,
        "SELECT solarSystemID,wormholeClassID FROM mapSolarSystems mss "
        "INNER JOIN mapLocationWormholeClasses mlwc ON mss.regionID = mlwc.locationID "
        "WHERE solarSystemID NOT IN (SELECT locationID FROM mapLocationWormholeClasses mlwc2 WHERE mlwc2.wormholeClassID = 8) "
        "AND wormholeClassID = %u", systemClass))
        {
            codelog(DATABASE__ERROR, "Error in GetWHClassSystems(hs) query: %s", res.error.c_str());
        }
    }
}