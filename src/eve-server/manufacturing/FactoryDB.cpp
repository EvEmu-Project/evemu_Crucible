
 /**
  * @name FactoryDB.cpp
  *   db query methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18   UD 5Nov20
  */

/*
 * # Manufacturing Logging:
 * MANUF__ERROR
 * MANUF__WARNING
 * MANUF__MESSAGE
 * MANUF__INFO
 * MANUF__DEBUG
 * MANUF__TRACE
 * MANUF__DUMP
 */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "character/Character.h"
#include "manufacturing/FactoryDB.h"

/** @todo  is there is a better way to do this??  */
bool FactoryDB::IsProducableBy(const uint32 assemblyLineID, const ItemType *pType) {
    Rsp_InstallJob into;
    return FactoryDB::GetMultipliers(assemblyLineID, pType, into);
}

void FactoryDB::GetSalvage(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (754, 966)"))
        codelog(DATABASE__ERROR, "Error in GetSalvage query: %s", res.error.c_str());
}

void FactoryDB::GetCompounds(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (282, 333, 423, 427, 530, 711, 712)"))
        codelog(DATABASE__ERROR, "Error in GetCompounds query: %s", res.error.c_str());
}

void FactoryDB::GetMinerals(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID = 18 AND published = 1"))   // minerals
        codelog(DATABASE__ERROR, "Error in GetMinerals query: %s", res.error.c_str());
}

void FactoryDB::GetComponents(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (280, 283, 313, 334, 428, 429, 526, 536, 873, 886, 913, 964)"))   // components
        codelog(DATABASE__ERROR, "Error in GetComponents query: %s", res.error.c_str());
}

void FactoryDB::GetCommodities(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (1042, 1034, 1040, 1041)"))   // PI Commodities
        codelog(DATABASE__ERROR, "Error in GetCommodities query: %s", res.error.c_str());
}

void FactoryDB::GetMiscCommodities(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID = 314"))
        codelog(DATABASE__ERROR, "Error in GetMinerals query: %s", res.error.c_str());
}

void FactoryDB::GetResources(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (1031, 1032, 1033, 1035)"))   // PI Resources
        codelog(DATABASE__ERROR, "Error in GetResources query: %s", res.error.c_str());
}

void FactoryDB::GetRAMMaterials(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, materialTypeID, quantity FROM invTypeMaterials"))
        codelog(DATABASE__ERROR, "Error in GetRAMMaterials query: %s", res.error.c_str());
}

void FactoryDB::GetRAMRequirements(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements"))
        codelog(DATABASE__ERROR, "Error in GetRAMRequirements query: %s", res.error.c_str());
}

void FactoryDB::SetJobEventID(const uint32 jobID, const uint32 eventID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE ramJobs SET eventID=%u WHERE jobID=%u", eventID, jobID);
}

bool FactoryDB::DeleteBlueprint(uint32 blueprintID) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM invBlueprints WHERE itemID=%u", blueprintID)) {
        _log(DATABASE__ERROR, "Failed to delete blueprint %u: %s.", blueprintID, err.c_str());
        return false;
    }
    return true;
}

PyRep* FactoryDB::GetMaterialCompositionOfItemType(const uint32 typeID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT requiredTypeID AS typeID, quantity"
        " FROM ramTypeRequirements"
        " WHERE typeID = (SELECT blueprintTypeID FROM invBlueprintTypes WHERE productTypeID = %u)"
        " AND activityID = 1 AND damagePerJob = 1",
        typeID))
    {
        _log(DATABASE__ERROR, "Could not retrieve material composition for type %u : %s", typeID, res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

void FactoryDB::GetOutpostMaterialCompositionOfItemType(const uint32 typeID, DBQueryResult& res) {
    if (!sDatabase.RunQuery(res,
        "SELECT materialTypeID AS typeID, quantity"
        " FROM invTypeMaterials"
        " WHERE typeID = %u",
        typeID))
    {
        _log(DATABASE__ERROR, "Could not retrieve outpost material composition for type %u : %s", typeID, res.error.c_str());
    }
}

bool FactoryDB::SaveBlueprintData(uint32 blueprintID, EvERam::bpData& data) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO invBlueprints"
        "  (itemID, copy, mLevel, pLevel, runs)"
        " VALUES"
        "  (%u, %u, %i, %i, %i)"
        "ON DUPLICATE KEY UPDATE "
        "mLevel=VALUES(mLevel), "
        "pLevel=VALUES(pLevel), "
        "runs=VALUES(runs) ",
        blueprintID, data.copy, data.mLevel, data.pLevel, data.runs))
    {
        codelog(DATABASE__ERROR, "Error in SaveBlueprint query: %s.", err.c_str());
        return false;
    }

    return true;
}

bool FactoryDB::GetBlueprintData(uint32 blueprintID, EvERam::bpData& into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  copy,"
        "  mLevel,"
        "  pLevel,"
        "  runs"
        " FROM invBlueprints"
        " WHERE itemID=%u",
        blueprintID))
    {
        codelog(DATABASE__ERROR, "Error in GetBlueprintData query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Blueprint %u not found.", blueprintID);
        return false;
    }

    into.copy = row.GetBool(0);
    into.mLevel = row.GetInt(1);
    into.pLevel = row.GetInt(2);
    into.runs = row.GetInt(3);

    return true;
}

void FactoryDB::GetBlueprintType(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  blueprintTypeID,"
        "  parentBlueprintTypeID,"
        "  productTypeID,"
        "  productionTime,"
        "  techLevel,"
        "  researchProductivityTime,"
        "  researchMaterialTime,"
        "  researchCopyTime,"
        "  researchTechTime,"
        "  productivityModifier,"
        "  materialModifier,"
        "  wasteFactor,"
        "  maxProductionLimit, "
        "  chanceOfRE,"
        "  g.categoryID"
        " FROM invBlueprintTypes AS bt"
        "  LEFT JOIN invTypes AS t ON t.typeID = bt.blueprintTypeID"
        "  LEFT JOIN invGroups AS g USING (groupID)"))
        //" WHERE t.published = 1" ))
    {
        codelog(DATABASE__ERROR, "Error in GetBlueprintType query: %s.", res.error.c_str());
    }
}

PyRep *FactoryDB::GetJobs2(const int32 ownerID, const bool completed)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " job.jobID,"
        " job.assemblyLineID,"
        " assemblyLine.containerID,"
        " job.installedItemID,"
        " installedItem.typeID AS installedItemTypeID,"
        " installedItem.ownerID AS installedItemOwnerID,"
        " blueprint.pLevel AS installedItemProductivityLevel,"
        " blueprint.mLevel AS installedItemMaterialLevel,"
        // quite ugly, but lets us use DBResultToRowset
        " IF(assemblyLine.activityID = 1, blueprintType.productTypeID, installedItem.typeID) AS outputTypeID,"
        " job.outputFlag,"
        " job.installerID,"
        " assemblyLine.activityID,"
        " job.runs,"
        " job.installTime,"
        " job.beginProductionTime,"
        " job.pauseProductionTime,"
        " job.endProductionTime,"
        " job.completedStatusID != 0 AS completed,"
        " job.licensedProductionRuns,"
        " station.solarSystemID AS installedInSolarSystemID,"
        " job.completedStatusID AS completedStatus,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID"
        " FROM ramJobs AS job"
        " LEFT JOIN entity AS installedItem ON job.installedItemID = installedItem.itemID"
        " LEFT JOIN ramAssemblyLines AS assemblyLine ON job.assemblyLineID = assemblyLine.assemblyLineID"
        " LEFT JOIN invBlueprints AS blueprint ON installedItem.itemID = blueprint.itemID"
        " LEFT JOIN invBlueprintTypes AS blueprintType ON installedItem.typeID = blueprintType.blueprintTypeID"
        " LEFT JOIN ramAssemblyLineStations AS station ON assemblyLine.containerID = station.stationID"
        " WHERE job.ownerID = %u"
        " AND job.completedStatusID %s 0"
        " GROUP BY job.jobID",
        ownerID, (completed ? "!=" : "=") ))
    {
        _log(DATABASE__ERROR, "Failed to query jobs for owner %u: %s", ownerID, res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyRep *FactoryDB::AssemblyLinesSelectPublic(const uint32 regionID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT DISTINCT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID,"
        " types.activityID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN crpNPCCorporations AS corp ON station.ownerID = corp.corporationID"
        " LEFT JOIN ramAssemblyLineTypes as types ON station.assemblyLineTypeID = types.assemblyLineTypeID"
        " WHERE station.ownerID = corp.corporationID"
        " AND station.regionID = %u",
        regionID))
    {
        _log(DATABASE__ERROR, "Failed to query public assembly lines for region %u: %s.", regionID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep *FactoryDB::AssemblyLinesSelectPersonal(const uint32 charID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT DISTINCT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID,"
        " line.activityID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE station.ownerID = %u"
        " AND (line.restrictionMask & %u) = %u",
        charID, (EvERam::RestrictionMask::ByCorp | EvERam::RestrictionMask::ByAlliance), (EvERam::RestrictionMask::ByCorp | EvERam::RestrictionMask::ByAlliance)))
    {
        _log(DATABASE__ERROR, "Failed to query personal assembly lines for char %u: %s.", charID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep *FactoryDB::AssemblyLinesSelectPrivate(const uint32 charID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT DISTINCT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID,"
        " line.activityID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE station.ownerID = %u",
        charID))
    {
        _log(DATABASE__ERROR, "Failed to query private assembly lines for char %u: %s.", charID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
PyRep *FactoryDB::AssemblyLinesSelectCorporation(const uint32 corpID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT DISTINCT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID,"
        " line.activityID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE station.ownerID = %u"
        " AND (line.restrictionMask & %u) = %u",
        corpID, EvERam::RestrictionMask::ByCorp, EvERam::RestrictionMask::ByCorp))
    {
        _log(DATABASE__ERROR, "Failed to query corporation assembly lines for corp %u: %s.", corpID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
PyRep *FactoryDB::AssemblyLinesSelectAlliance(const int32 allianceID) {
    DBQueryResult res;

    // This produces the same output but with less complex joins and
    // the quantity column is completely redundant.
    // SELECT
    //     job.containerId,
    //     station.stationTypeId AS containerTypeId,
    //     station.solarSystemId AS containerLocationId,
    //     job.typeId as assemblyLineTypeId,
    //     COUNT(job.containerId) as quantity,
    //     station.corporationId as ownerId
    //     FROM industrySlots AS job 
    //         JOIN evemu.staStations AS station ON station.stationId = job.containerId
    //         JOIN evemu.crpCorporation AS corp ON corp.corporationId = station.corporationId
    //     GROUP BY job.containerId, job.typeId;

    if (!sDatabase.RunQuery(res,
        "SELECT DISTINCT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID,"
        " line.activityID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN crpCorporation AS crp ON station.ownerID = crp.corporationID"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE crp.allianceID = %u"
        " AND (line.restrictionMask & %u) = %u",
        allianceID, EvERam::RestrictionMask::ByAlliance, EvERam::RestrictionMask::ByAlliance))
    {
        _log(DATABASE__ERROR, "Failed to query alliance assembly lines for alliance %u: %s.", allianceID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
PyRep *FactoryDB::AssemblyLinesGet(const uint32 containerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " assemblyLineID,"
        " assemblyLineTypeID,"
        " containerID,"
        " nextFreeTime,"
        " costInstall,"
        " costPerHour,"
        " restrictionMask,"
        " discountPerGoodStandingPoint,"
        " surchargePerBadStandingPoint,"
        " minimumStanding,"
        " minimumCharSecurity,"
        " minimumCorpSecurity,"
        " maximumCharSecurity,"
        " maximumCorpSecurity"
        " FROM ramAssemblyLines"
        " WHERE containerID = %u",
        containerID)) {
        _log(DATABASE__ERROR, "Failed to query assembly lines for container %u: %s.", containerID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
bool FactoryDB::GetAssemblyLineProperties(const uint32 assemblyLineID, Character* pChar, Rsp_InstallJob& into, bool isCorpJob/*false*/) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " alt.baseMaterialMultiplier,"
        " alt.baseTimeMultiplier,"
        " al.costInstall,"
        " alt.minCostPerHour,"
        " al.costPerHour,"
        " al.ownerID,"                          //5
        " al.discountPerGoodStandingPoint,"
        " al.surchargePerBadStandingPoint"
        " FROM ramAssemblyLines AS al"
        " LEFT JOIN ramAssemblyLineTypes AS alt USING (assemblyLineTypeID)"
        " WHERE al.assemblyLineID = %u",
        assemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to query properties for assembly line %u: %s.", assemblyLineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No properties found for assembly line %u.", assemblyLineID);
        return false;
    }

    into.materialMultiplier     = row.GetFloat(0);
    into.timeMultiplier         = row.GetFloat(1);
    into.installCost            = row.GetFloat(2);
    if (row.GetFloat(3) > row.GetFloat(4)) {            //min of base cost/hr vs minimum cost/hr
        into.usageCost          = row.GetFloat(3);
    } else {
        into.usageCost          = row.GetFloat(4);
    }

    // verify line standings modifiers (some have no modifiers)
    if ((row.GetFloat(6) == 0) and (row.GetFloat(7) == 0))
        return true;

    float standing(1), costModifier(1);
    uint32 factionID(sDataMgr.GetCorpFaction(row.GetInt(5)));
    if (isCorpJob) {
        // this is only for PC corps.  take higher of (npc faction to pc corp)/2 or npc corp to pc corp
        float cStanding(StandingDB::GetStanding(row.GetInt(5), pChar->corporationID()));
        float fStanding(StandingDB::GetStanding(factionID, pChar->corporationID()));
        fStanding /= 2;
        // this works for negative standings also
        if (cStanding > fStanding) {
            standing = cStanding;
        } else {
            standing = fStanding;
        }

        /** @todo  this shit will have to be verified for negative standings */
        // modify end result by 25% for char standings with station owner
        standing *= (1 - (0.025f * StandingDB::GetStanding(row.GetInt(5), pChar->itemID())));
    } else {
        // else take personal standings with station corp only
        standing = StandingDB::GetStanding(row.GetInt(5), pChar->itemID());
    }

    if (standing < 0) {
        costModifier += row.GetFloat(7) * -standing;
    } else {
        costModifier -= row.GetFloat(6) * standing;
    }

    // make sure costModifier isnt 0 (some lines have 0 as modifier)
    if (costModifier == 0)
        costModifier = 1;

    _log(MANUF__MESSAGE, "FactoryDB::GetALProps() - Cost Modifier %.2f, standing %.2f", costModifier, standing);

    // modify setup cost based on standings
    into.installCost *= costModifier;

    return true;
}

/** @todo  need to add check/query for POS assembly modules here */
bool FactoryDB::GetAssemblyLineRestrictions(const int32 assemblyLineID, EvERam::LineRestrictions &data) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " ownerID,"
        " minimumStanding,"
        " minimumCharSecurity,"
        " maximumCharSecurity,"
        " minimumCorpSecurity,"
        " maximumCorpSecurity,"
        " restrictionMask,"
        " activityID"
        " FROM ramAssemblyLines"
        " WHERE assemblyLineID = %i",
        assemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to query verify properties for assembly line %u: %s.", assemblyLineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No verify properties found for assembly line %u.", assemblyLineID);
        return false;
    }

    data.ownerID        = row.GetUInt(0);
    data.minStanding    = row.GetFloat(1);
    data.minCharSec     = row.GetFloat(2);
    data.maxCharSec     = row.GetFloat(3);
    data.minCorpSec     = row.GetFloat(4);
    data.maxCorpSec     = row.GetFloat(5);
    data.rMask          = row.GetUInt(6);
    data.activityID     = row.GetUInt(7);

    return true;
}

uint32 FactoryDB::InstallJob(const uint32 ownerID, const uint32 installerID, Call_InstallJob& args,
                             const int64 beginTime, const int64 endTime, const uint32 systemID)
{
    DBerror err;
    uint32 jobID(0);
    // insert job
    if (!sDatabase.RunQueryLID(err, jobID,
        "INSERT INTO ramJobs"
        " (ownerID, installerID, assemblyLineID, installedItemID, installTime, beginProductionTime, endProductionTime,"
        " runs, outputFlag, licensedProductionRuns, completedStatusID, installedInSolarSystemID)"
        " VALUES"
        " (%u, %u, %i, %i, %.0f, %lli, %lli,"
        " %i, %i, %i, 0, %i)",
        ownerID, installerID, args.AssemblyLineID, args.bpItemID, GetFileTimeNow(), beginTime, endTime,
        args.runs, args.outputFlag, args.copyRuns, systemID))
    {
        _log(DATABASE__ERROR, "Failed to insert new job to database: %s.", err.c_str());
        return 0;
    }

    // update nextFreeTime
    if (!sDatabase.RunQuery(err,
        "UPDATE ramAssemblyLines"
        " SET nextFreeTime = %lli"
        " WHERE assemblyLineID = %i",
        endTime, args.AssemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to update next free time for assembly line %u: %s.", args.AssemblyLineID, err.c_str());
        return 0;
    }

    return jobID;
}

uint32 FactoryDB::CountManufacturingJobs(const uint32 installerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " COUNT(job.jobID)"
        " FROM ramJobs AS job"
        " LEFT JOIN ramAssemblyLines AS line USING (assemblyLineID)"
        " WHERE job.installerID = %u"
        " AND job.completedStatusID = 0"
        " AND line.activityID = 1",
        installerID))
    {
        _log(DATABASE__ERROR, "Failed to count manufacturing jobs for installer %u.", installerID);
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No rows returned while counting manufacturing jobs for installer %u.", installerID);
        return 0;
    }

    return row.GetUInt(0);
}
uint32 FactoryDB::CountResearchJobs(const uint32 installerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " COUNT(job.jobID)"
        " FROM ramJobs AS job"
        " LEFT JOIN ramAssemblyLines AS line USING (assemblyLineID)"
        " WHERE job.installerID = %u"
        " AND job.completedStatusID = 0"
        " AND line.activityID != 1",    // is this accurate?
        installerID))
    {
        _log(DATABASE__ERROR, "Failed to count research jobs for installer %u.", installerID);
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

bool FactoryDB::GetJobProperties(const uint32 jobID, EvERam::JobProperties &data) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT job.installedItemID, job.ownerID, job.outputFlag, job.runs, job.licensedProductionRuns,"
        " job.endProductionTime, job.completedStatusID, line.activityID, job.eventID"
        " FROM ramJobs AS job"
        " LEFT JOIN ramAssemblyLines AS line USING (assemblyLineID)"
        " WHERE job.jobID = %u",
        jobID))
    {
        _log(DATABASE__ERROR, "Failed to query properties of job %u: %s.", jobID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No properties found for job %u.", jobID);
        return false;
    }

    data.itemID         = row.GetUInt(0);
    data.ownerID        = row.GetUInt(1);
    data.outputFlag     = (EVEItemFlags)row.GetUInt(2);
    data.jobRuns        = row.GetInt(3);
    data.licensedRuns   = row.GetInt(4);
    data.endTime        = row.GetInt64(5);
    data.status         = row.GetInt(6);
    data.activity       = row.GetUInt(7);
    data.eventID        = row.GetUInt(8);

    return true;
}

bool FactoryDB::CompleteJob(const uint32 jobID, const int8 completedStatus) {
    DBerror err;

    if (!sDatabase.RunQuery(err, "UPDATE ramJobs SET completedStatusID = %i WHERE jobID = %u", completedStatus, jobID)) {
        _log(DATABASE__ERROR, "Failed to complete job %u (status = %i): %s.", jobID, completedStatus, err.c_str());
        return false;
    }

    return true;
}

uint32 FactoryDB::GetTech2Blueprint(const uint32 blueprintTypeID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT blueprintTypeID FROM invBlueprintTypes WHERE parentBlueprintTypeID = %u", blueprintTypeID)) {
        _log(DATABASE__ERROR, "Unable to get T2 type for type ID %u: %s", blueprintTypeID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

int64 FactoryDB::GetNextFreeTime(const uint32 assemblyLineID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT nextFreeTime FROM ramAssemblyLines WHERE assemblyLineID = %u", assemblyLineID)) {
        _log(DATABASE__ERROR, "Failed to query next free time for assembly line %u: %s.", assemblyLineID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Assembly line %u not found.", assemblyLineID);
        return 0;
    }

    return (row.IsNull(0) ? 0 : row.GetInt64(0));
}

bool FactoryDB::GetMultipliers(const uint32 assemblyLineID, const ItemType* pType, Rsp_InstallJob& into) {
    DBQueryResult res;
    // check Category first
    if (!sDatabase.RunQuery(res,
        "SELECT alc.materialMultiplier, alc.timeMultiplier"
        " FROM ramAssemblyLineTypeDetailPerCategory AS alc"
        " LEFT JOIN ramAssemblyLines AS al USING (assemblyLineTypeID)"
        " WHERE al.assemblyLineID = %u AND alc.categoryID = %u",
        assemblyLineID, pType->categoryID()))
    {
        _log(DATABASE__ERROR, "Failed to check producability of cat %u by line %u: %s", pType->categoryID(), assemblyLineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        into.materialMultiplier *= row.GetDouble(0);
        into.timeMultiplier *= row.GetDouble(1);
    }

    // then Group  (all materialMultiplier = 1)
    if (!sDatabase.RunQuery(res,
        "SELECT alg.materialMultiplier, alg.timeMultiplier"
        " FROM ramAssemblyLineTypeDetailPerGroup AS alg"
        " LEFT JOIN ramAssemblyLines AS al USING (assemblyLineTypeID)"
        " WHERE al.assemblyLineID = %u"
        " AND alg.groupID = %u",
        assemblyLineID, pType->groupID()))
    {
        _log(DATABASE__ERROR, "Failed to check producability of group %u by line %u: %s", pType->groupID(), assemblyLineID, res.error.c_str());
        return false;
    }

    if (res.GetRow(row)) {
        into.materialMultiplier *= row.GetDouble(0);
        into.timeMultiplier *= row.GetDouble(1);
    }

    return true;
}

bool FactoryDB::IsRefinable(const uint16 typeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT typeID FROM ramTypeRequirements"
        " WHERE typeID=%u AND extra = 0 LIMIT 1",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to check ore type ID: %s.", res.error.c_str());
        return false;
    }

    return (res.ColumnCount() > 0);
}

bool FactoryDB::IsRecyclable(const uint16 typeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT typeID FROM ramTypeRequirements"
        " WHERE damagePerJob = 1 AND activityID = 6 AND typeID = %u"
        " AND extra = 1 LIMIT 1",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to check item type ID: %s.", res.error.c_str());
        return false;
    }

    if (res.ColumnCount() > 0)
        return true;

    if (!sDatabase.RunQuery(res,
        "SELECT typeID FROM ramTypeRequirements"
        " WHERE damagePerJob = 1 AND activityID = 1 AND productTypeID = %u"
        " AND extra = 1 LIMIT 1",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to check item type ID: %s.", res.error.c_str());
        return false;
    }

    return (res.ColumnCount() > 0);
}
