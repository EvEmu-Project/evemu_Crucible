/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

RamProxyDB::RamProxyDB(DBcore *db)
: ServiceDB(db)
{
}

RamProxyDB::~RamProxyDB() {
}

PyRep *RamProxyDB::GetJobs2(const uint32 ownerID, const bool completed, const uint64 fromDate, const uint64 toDate) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" job.jobID,"
		" job.assemblyLineID,"
		" assemblyLine.containerID,"
		" job.installedItemID,"
		" installedItem.typeID AS installedItemTypeID,"
		" installedItem.ownerID AS installedItemOwnerID,"
		" blueprint.productivityLevel AS installedItemProductivityLevel,"
		" blueprint.materialLevel AS installedItemMaterialLevel,"
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
		" job.installedInSolarSystemID,"
		" job.completedStatusID AS completedStatus,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID"
		" FROM ramJobs AS job"
		" LEFT JOIN entity AS installedItem ON job.installedItemID = installedItem.itemID"
		" LEFT JOIN ramAssemblyLines AS assemblyLine ON job.assemblyLineID = assemblyLine.assemblyLineID"
		" LEFT JOIN invBlueprints AS blueprint ON installedItem.itemID = blueprint.blueprintID"
		" LEFT JOIN invBlueprintTypes AS blueprintType ON installedItem.typeID = blueprintType.blueprintTypeID"
		" LEFT JOIN ramAssemblyLineStations AS station ON assemblyLine.containerID = station.stationID"
		" WHERE job.ownerID = %lu"
		" AND job.completedStatusID %s 0"
		" AND job.installTime >= " I64u
		" AND job.endProductionTime <= " I64u
		" GROUP BY job.jobID",
		ownerID, (completed ? "!=" : "="), fromDate, toDate))
	{
		_log(DATABASE__ERROR, "Failed to query jobs for owner %lu: %s", ownerID, res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *RamProxyDB::AssemblyLinesSelectPublic(const uint32 regionID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" station.stationID AS containerID,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID,"
		" station.assemblyLineTypeID,"
		" station.quantity,"
		" station.ownerID"
		" FROM ramAssemblyLineStations AS station"
		" LEFT JOIN crpNPCCorporations AS corp ON station.ownerID = corp.corporationID"
		" WHERE station.ownerID = corp.corporationID"
		" AND station.regionID = %lu",
		regionID))
	{
		_log(DATABASE__ERROR, "Failed to query public assembly lines for region %lu: %s.", regionID, res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}
PyRep *RamProxyDB::AssemblyLinesSelectPersonal(const uint32 charID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" station.stationID AS containerID,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID,"
		" station.assemblyLineTypeID,"
		" station.quantity,"
		" station.ownerID"
		" FROM ramAssemblyLineStations AS station"
		" LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
		" WHERE station.ownerID = %lu"
		" AND (line.restrictionMask & 12) = 0", // (restrictionMask & (ramRestrictByCorp | ramRestrictByAlliance)) = 0
		charID))
	{
		_log(DATABASE__ERROR, "Failed to query personal assembly lines for char %lu: %s.", charID, res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}
PyRep *RamProxyDB::AssemblyLinesSelectCorporation(const uint32 corporationID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" station.stationID AS containerID,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID,"
		" station.assemblyLineTypeID,"
		" station.quantity,"
		" station.ownerID"
		" FROM ramAssemblyLineStations AS station"
		" LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
		" WHERE station.ownerID = %lu"
		" AND (line.restrictionMask & 4) = 4", // (restrictionMask & ramRestrictByCorp) = ramRestrictByCorp
		corporationID))
	{
		_log(DATABASE__ERROR, "Failed to query corporation assembly lines for corp %lu: %s.", corporationID, res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}
PyRep *RamProxyDB::AssemblyLinesSelectAlliance(const uint32 allianceID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" station.stationID AS containerID,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID,"
		" station.assemblyLineTypeID,"
		" station.quantity,"
		" station.ownerID"
		" FROM ramAssemblyLineStations AS station"
		" LEFT JOIN corporation AS crp ON station.ownerID = crp.corporationID"
		" LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
		" WHERE crp.allianceID = %lu"
		" AND (line.restrictionMask & 8) = 8", // (restrictionMask & ramRestrictByAlliance) = ramRestrictByAlliance
		allianceID))
	{
		_log(DATABASE__ERROR, "Failed to query alliance assembly lines for alliance %lu: %s.", allianceID, res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}
PyRep *RamProxyDB::AssemblyLinesGet(const uint32 containerID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
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
		" WHERE containerID = %lu",
		containerID))
	{
		_log(DATABASE__ERROR, "Failed to query assembly lines for container %lu: %s.", containerID, res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

bool RamProxyDB::GetAssemblyLineProperties(const uint32 assemblyLineID, double &baseMaterialMultiplier, double &baseTimeMultiplier, double &costInstall, double &costPerHour) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" assemblyLineType.baseMaterialMultiplier,"
		" assemblyLineType.baseTimeMultiplier,"
		" assemblyLine.costInstall,"
		" assemblyLine.costPerHour"
		" FROM ramAssemblyLines AS assemblyLine"
		" LEFT JOIN ramAssemblyLineTypes AS assemblyLineType ON assemblyLine.assemblyLineTypeID = assemblyLineType.assemblyLineTypeID"
		" WHERE assemblyLine.assemblyLineID = %lu",
		assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to query properties for assembly line %lu: %s.", assemblyLineID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No properties found for assembly line %lu.", assemblyLineID);
		return(false);
	}

	baseMaterialMultiplier = row.GetDouble(0);
	baseTimeMultiplier = row.GetDouble(1);
	costInstall = row.GetDouble(2);
	costPerHour = row.GetDouble(3);

	return(true);
}

bool RamProxyDB::GetAssemblyLineVerifyProperties(const uint32 assemblyLineID, uint32 &ownerID, double &minCharSecurity, double &maxCharSecurity, EVERamRestrictionMask &restrictionMask, EVERamActivity &activity) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" ownerID,"
		" minimumCharSecurity,"
		" maximumCharSecurity,"
		" restrictionMask,"
		" activityID"
		" FROM ramAssemblyLines"
		" WHERE assemblyLineID = %lu",
		assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to query verify properties for assembly line %lu: %s.", assemblyLineID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No verify properties found for assembly line %lu.", assemblyLineID);
		return(false);
	}

	ownerID = row.GetUInt(0);
	minCharSecurity = row.GetDouble(1);
	maxCharSecurity = row.GetDouble(2);
	restrictionMask = (EVERamRestrictionMask)row.GetUInt(3);
	activity = (EVERamActivity)row.GetUInt(4);

	return(true);
}

bool RamProxyDB::InstallJob(const uint32 ownerID, const  uint32 installerID, const uint32 assemblyLineID, const uint32 installedItemID, const uint64 beginProductionTime, const uint64 endProductionTime, const char *description, const uint32 runs, const EVEItemFlags outputFlag, const uint32 installedInSolarSystem, const int32 licensedProductionRuns) {
	DBerror err;

	// insert job
	if(!m_db->RunQuery(err,
		"INSERT INTO ramJobs"
		" (ownerID, installerID, assemblyLineID, installedItemID, installTime, beginProductionTime, endProductionTime, description, runs, outputFlag,"
		" completedStatusID, installedInSolarSystemID, licensedProductionRuns)"
		" VALUES"
		" (%lu, %lu, %lu, %lu, " I64u ", " I64u ", " I64u ", '%s', %lu, %d, 0, %lu, %li)",
		ownerID, installerID, assemblyLineID, installedItemID, Win32TimeNow(), beginProductionTime, endProductionTime, description,
		runs, (int)outputFlag, installedInSolarSystem, licensedProductionRuns))
	{
		_log(DATABASE__ERROR, "Failed to insert new job to database: %s.", err.c_str());
		return(false);
	}

	// update nextFreeTime
	if(!m_db->RunQuery(err,
		"UPDATE ramAssemblyLines"
		" SET nextFreeTime = " I64u
		" WHERE assemblyLineID = %lu",
		endProductionTime, assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to update next free time for assembly line %lu: %s.", assemblyLineID, err.c_str());
		return(false);
	}

	return(true);
}

bool RamProxyDB::IsProducableBy(const uint32 assemblyLineID, const uint32 groupID) {
	double tmp;
	return(_GetMultipliers(assemblyLineID, groupID, tmp, tmp));
}

bool RamProxyDB::MultiplyMultipliers(const uint32 assemblyLineID, const uint32 productGroupID, double &materialMultiplier, double &timeMultiplier) {
	double tmpMat, tmpTime;
	if(!_GetMultipliers(assemblyLineID, productGroupID, tmpMat, tmpTime))
		return(false);

	materialMultiplier *= tmpMat;
	timeMultiplier *= tmpTime;

	return(true);
}

uint32 RamProxyDB::CountManufacturingJobs(const uint32 installerID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" COUNT(job.jobID)"
		" FROM ramJobs AS job"
		" LEFT JOIN ramAssemblyLines AS line ON job.assemblyLineID = line.assemblyLineID"
		" WHERE job.installerID = %lu"
		" AND job.completedStatusID = 0"
		" AND line.activityID = 1",
		installerID))
	{
		_log(DATABASE__ERROR, "Failed to count manufacturing jobs for installer %lu.", installerID);
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No rows returned while counting manufacturing jobs for installer %lu.", installerID);
		return(NULL);
	}

	return(row.GetUInt(0));
}
uint32 RamProxyDB::CountResearchJobs(const uint32 installerID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" COUNT(job.jobID)"
		" FROM ramJobs AS job"
		" LEFT JOIN ramAssemblyLines AS line ON job.assemblyLineID = line.assemblyLineID"
		" WHERE job.installerID = %lu"
		" AND job.completedStatusID = 0"
		" AND line.activityID != 1",
		installerID))
	{
		_log(DATABASE__ERROR, "Failed to count research jobs for installer %lu.", installerID);
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No rows returned while counting research jobs for installer %lu.", installerID);
		return(NULL);
	}

	return(row.GetUInt(0));
}
bool RamProxyDB::GetAdditionalBlueprintProperties(const uint32 blueprintID, uint32 &materialLevel, double &wasteFactor, uint32 &productivityLevel, uint32 &productivityModifier) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" blueprint.materialLevel,"
		" blueprintType.wasteFactor / 100,"
		" blueprint.productivityLevel,"
		" blueprintType.productivityModifier"
		" FROM invBlueprints AS blueprint"
		" LEFT JOIN entity ON blueprint.blueprintID = entity.itemID"
		" LEFT JOIN invBlueprintTypes AS blueprintType ON blueprintType.blueprintTypeID = entity.typeID"
		" WHERE blueprint.blueprintID = %lu",
		blueprintID))
	{
		_log(DATABASE__ERROR, "Failed to query additional properties for blueprint type %lu: %s.", blueprintID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No additional properties found for blueprint type %lu.", blueprintID);
		return(false);
	}

	materialLevel = row.GetUInt(0);
	wasteFactor = row.GetDouble(1);
	productivityLevel = row.GetUInt(2);
	productivityModifier = row.GetUInt(3);

	return(true);
}

bool RamProxyDB::GetRequiredItems(const uint32 typeID, const EVERamActivity activity, std::vector<RequiredItem> &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" material.requiredTypeID,"
		" material.quantity,"
		" material.damagePerJob,"
		" IF(materialGroup.categoryID = 16, 1, 0) AS isSkill"
		" FROM typeActivityMaterials AS material"
		" LEFT JOIN invTypes AS materialType ON material.requiredTypeID = materialType.typeID"
		" LEFT JOIN invGroups AS materialGroup ON materialType.groupID = materialGroup.groupID"
		" WHERE material.typeID = %lu"
		" AND material.activityID = %d"
		//this is needed as db is quite crappy ...
		" AND material.quantity > 0",
		typeID, (const int)activity))
	{
		_log(DATABASE__ERROR, "Failed to query data to build BillOfMaterials: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	while(res.GetRow(row))
		into.push_back(RequiredItem(row.GetUInt(0), row.GetUInt(1), row.GetFloat(2), row.GetInt(3) ? true : false));

	return(true);
}

bool RamProxyDB::GetJobProperties(const uint32 jobID, uint32 &installedItemID, uint32 &ownerID, EVEItemFlags &outputFlag, uint32 &runs, uint32 &licensedProductionRuns, EVERamActivity &activity) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT job.installedItemID, job.ownerID, job.outputFlag, job.runs, job.licensedProductionRuns, assemblyLine.activityID"
		" FROM ramJobs AS job"
		" LEFT JOIN ramAssemblyLines AS assemblyLine ON job.assemblyLineID = assemblyLine.assemblyLineID"
		" WHERE job.jobID = %lu",
		jobID))
	{
		_log(DATABASE__ERROR, "Failed to query properties of job %lu: %s.", jobID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No properties found for job %lu.", jobID);
		return(false);
	}

	installedItemID = row.GetUInt(0);
	ownerID = row.GetUInt(1);
	outputFlag = (EVEItemFlags)row.GetUInt(2);
	runs = row.GetUInt(3);
	licensedProductionRuns = row.GetUInt(4);
	activity = (EVERamActivity)row.GetUInt(5);

	return(true);
}

bool RamProxyDB::GetJobVerifyProperties(const uint32 jobID, uint32 &ownerID, uint64 &endProductionTime, EVERamRestrictionMask &restrictionMask, EVERamCompletedStatus &status) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT job.ownerID, job.endProductionTime, job.completedStatusID, line.restrictionMask"
				" FROM ramJobs AS job"
				" LEFT JOIN ramAssemblyLines AS line ON line.assemblyLineID = job.assemblyLineID"
				" WHERE job.jobID = %lu",
				jobID))
	{
		_log(DATABASE__ERROR, "Unable to query completion properties for job %lu: %s", jobID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No completion properties found for job %lu.", jobID);
		return(false);
	}

	ownerID = row.GetUInt(0);
	endProductionTime = row.GetUInt64(1);
	status = (EVERamCompletedStatus)row.GetUInt(2);
	restrictionMask = (EVERamRestrictionMask)row.GetUInt(3);

	return(true);
}

bool RamProxyDB::CompleteJob(const uint32 jobID, const EVERamCompletedStatus completedStatus) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"UPDATE ramJobs"
		" SET completedStatusID = %lu"
		" WHERE jobID = %lu",
		(uint32)completedStatus, jobID))
	{
		_log(DATABASE__ERROR, "Failed to complete job %lu (completed status = %lu): %s.", jobID, (uint32)completedStatus, err.c_str());
		return(false);
	}

	return(true);
}

std::string RamProxyDB::GetTypeName(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT typeName"
		" FROM invTypes"
		" WHERE typeID = %lu",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to query type name of type %lu: %s.", typeID, res.error.c_str());
		return("");
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No type name found for type %lu.", typeID);
		return("");
	}

	return(row.GetText(0));
}

std::string RamProxyDB::GetStationName(const uint32 stationID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT stationName"
		" FROM staStations"
		" WHERE stationID = %lu",
		stationID))
	{
		_log(DATABASE__ERROR, "Failed to query station name of station %lu: %s.", stationID, res.error.c_str());
		return("");
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Station %lu not found.", stationID);
		return("");
	}

	return(row.GetText(0));
}

uint32 RamProxyDB::GetMaxProductionLimit(const uint32 blueprintTypeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT maxProductionLimit"
		" FROM invBlueprintTypes"
		" WHERE blueprintTypeID = %lu",
		blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Failed to query max production limit of type %lu: %s.", blueprintTypeID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No max production limit found for type %lu.", blueprintTypeID);
		return(NULL);
	}

	return(row.GetUInt(0));
}

uint32 RamProxyDB::GetGroup(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT groupID"
				" FROM invTypes"
				" WHERE typeID = %lu",
				typeID))
	{
		_log(DATABASE__ERROR, "Unable to get group for type %lu: %s", typeID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Type %lu not found.", typeID);
		return(NULL);
	}

	return(row.GetUInt(0));
}

EVERace RamProxyDB::GetRace(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT raceID"
				" FROM invTypes"
				" WHERE typeID = %lu",
				typeID))
	{
		_log(DATABASE__ERROR, "Unable to get race for type %lu: %s", typeID, res.error.c_str());
		return((EVERace)NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Type %lu not found.", typeID);
		return((EVERace)NULL);
	}

	return(row.IsNull(0) ? (EVERace)NULL : (EVERace)row.GetUInt(0));
}
uint32 RamProxyDB::GetTech2Blueprint(const uint32 blueprintTypeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT blueprintTypeID"
				" FROM invBlueprintTypes"
				" WHERE parentBlueprintTypeID = %lu",
				blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Unable to get T2 type for type ID %lu: %s", blueprintTypeID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		// no error because it's normal
		return(NULL);
	}

	return(row.GetUInt(0));
}

uint32 RamProxyDB::GetPortionSize(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT portionSize"
				" FROM invTypes"
				" WHERE typeID=%lu",
				typeID
				))
	{
		_log(DATABASE__ERROR, "Unable to get portion size for type ID %lu: %s", typeID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No portion size found for type ID %lu.", typeID);
		return(NULL);
	}

	return(row.GetUInt(0));
}

uint64 RamProxyDB::GetNextFreeTime(const uint32 assemblyLineID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" nextFreeTime"
		" FROM ramAssemblyLines"
		" WHERE assemblyLineID = %lu",
		assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to query next free time for assembly line %lu: %s.", assemblyLineID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Assembly line %lu not found.", assemblyLineID);
		return(NULL);
	} else
		return(row.GetUInt64(0));
}

uint32 RamProxyDB::GetRegionOfContainer(const uint32 containerID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT regionID"
				" FROM ramAssemblyLineStations"
				" WHERE stationID = %lu",
				containerID))
	{
		_log(DATABASE__ERROR, "Unable to query region for container %lu: %s", containerID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No region found for container %lu.", containerID);
		return(NULL);
	}

	return(row.GetUInt(0));

}

uint32 RamProxyDB::GetBlueprintProductionTime(const uint32 blueprintTypeID, const EVERamActivity activity) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" blueprintType.productionTime,"
		" blueprintType.researchProductivityTime,"
		" blueprintType.researchMaterialTime,"
		" blueprintType.researchCopyTime,"
		" blueprintType.researchTechTime"
		" FROM invBlueprintTypes AS blueprintType"
		" WHERE blueprintType.blueprintTypeID = %lu",
		blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Failed to query production times for blueprint type %lu: %s.", blueprintTypeID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No production times found for blueprint type %lu.", blueprintTypeID);
		return(NULL);
	}

	switch(activity) {
		case ramActivityManufacturing:						return(row.GetUInt(0));
		case ramActivityResearchingTimeProductivity:		return(row.GetUInt(1));
		case ramActivityResearchingMaterialProductivity:	return(row.GetUInt(2));
		case ramActivityCopying:							return(row.GetUInt(3));
		case ramActivityInvention:							return(row.GetUInt(4));
		default:											return(NULL);
	}
}

bool RamProxyDB::_GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier) {
	DBQueryResult res;

	// check table ramAssemblyLineTypeDetailPerGroup first
	if(!m_db->RunQuery(res,
				"SELECT materialMultiplier, timeMultiplier"
				" FROM ramAssemblyLineTypeDetailPerGroup"
				" JOIN ramAssemblyLines USING (assemblyLineTypeID)"
				" WHERE assemblyLineID = %lu"
				" AND groupID = %lu",
				assemblyLineID, groupID))
	{
		_log(DATABASE__ERROR, "Failed to check producability of group %lu by line %lu: %s", groupID, assemblyLineID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(res.GetRow(row)) {
		materialMultiplier = row.GetDouble(0);
		timeMultiplier = row.GetDouble(1);
		return(true);
	}

	// then ramAssemblyLineTypeDetailPerCategory
	if(!m_db->RunQuery(res,
				"SELECT materialMultiplier, timeMultiplier"
				" FROM ramAssemblyLineTypeDetailPerCategory"
				" JOIN ramAssemblyLines USING (assemblyLineTypeID)"
				" JOIN invGroups USING (categoryID)"
				" WHERE assemblyLineID = %lu"
				" AND groupID = %lu",
				assemblyLineID, groupID))
	{
		_log(DATABASE__ERROR, "Failed to check producability of group %lu by line %lu: %s", groupID, assemblyLineID, res.error.c_str());
		return(false);
	}

	if(res.GetRow(row)) {
		materialMultiplier = row.GetDouble(0);
		timeMultiplier = row.GetDouble(1);
		return(true);
	} else {
		_log(DATABASE__ERROR, "Failed to find multipliers for group %lu for assembly line %lu. Group is probably not producable by this line.", groupID, assemblyLineID);
		return(false);
	}
}

