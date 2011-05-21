/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

#include "EVEServerPCH.h"

PyRep *RamProxyDB::GetJobs2(const uint32 ownerID, const bool completed, const uint64 fromDate, const uint64 toDate) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
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
		" WHERE job.ownerID = %u"
		" AND job.completedStatusID %s 0"
		" AND job.installTime >= " I64u
		" AND job.endProductionTime <= " I64u
		" GROUP BY job.jobID",
		ownerID, (completed ? "!=" : "="), fromDate, toDate))
	{
		_log(DATABASE__ERROR, "Failed to query jobs for owner %u: %s", ownerID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *RamProxyDB::AssemblyLinesSelectPublic(const uint32 regionID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
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
		" AND station.regionID = %u",
		regionID))
	{
		_log(DATABASE__ERROR, "Failed to query public assembly lines for region %u: %s.", regionID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}
PyRep *RamProxyDB::AssemblyLinesSelectPersonal(const uint32 charID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" station.stationID AS containerID,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID,"
		" station.assemblyLineTypeID,"
		" station.quantity,"
		" station.ownerID"
		" FROM ramAssemblyLineStations AS station"
		" LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
		" WHERE station.ownerID = %u"
		" AND (line.restrictionMask & 12) = 0", // (restrictionMask & (ramRestrictByCorp | ramRestrictByAlliance)) = 0
		charID))
	{
		_log(DATABASE__ERROR, "Failed to query personal assembly lines for char %u: %s.", charID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}
PyRep *RamProxyDB::AssemblyLinesSelectCorporation(const uint32 corporationID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" station.stationID AS containerID,"
		" station.stationTypeID AS containerTypeID,"
		" station.solarSystemID AS containerLocationID,"
		" station.assemblyLineTypeID,"
		" station.quantity,"
		" station.ownerID"
		" FROM ramAssemblyLineStations AS station"
		" LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
		" WHERE station.ownerID = %u"
		" AND (line.restrictionMask & 4) = 4", // (restrictionMask & ramRestrictByCorp) = ramRestrictByCorp
		corporationID))
	{
		_log(DATABASE__ERROR, "Failed to query corporation assembly lines for corp %u: %s.", corporationID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}
PyRep *RamProxyDB::AssemblyLinesSelectAlliance(const uint32 allianceID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
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
		" WHERE crp.allianceID = %u"
		" AND (line.restrictionMask & 8) = 8", // (restrictionMask & ramRestrictByAlliance) = ramRestrictByAlliance
		allianceID))
	{
		_log(DATABASE__ERROR, "Failed to query alliance assembly lines for alliance %u: %s.", allianceID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}
PyRep *RamProxyDB::AssemblyLinesGet(const uint32 containerID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
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
		containerID))
	{
		_log(DATABASE__ERROR, "Failed to query assembly lines for container %u: %s.", containerID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

bool RamProxyDB::GetAssemblyLineProperties(const uint32 assemblyLineID, double &baseMaterialMultiplier, double &baseTimeMultiplier, double &costInstall, double &costPerHour) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" assemblyLineType.baseMaterialMultiplier,"
		" assemblyLineType.baseTimeMultiplier,"
		" assemblyLine.costInstall,"
		" assemblyLine.costPerHour"
		" FROM ramAssemblyLines AS assemblyLine"
		" LEFT JOIN ramAssemblyLineTypes AS assemblyLineType ON assemblyLine.assemblyLineTypeID = assemblyLineType.assemblyLineTypeID"
		" WHERE assemblyLine.assemblyLineID = %u",
		assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to query properties for assembly line %u: %s.", assemblyLineID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No properties found for assembly line %u.", assemblyLineID);
		return false;
	}

	baseMaterialMultiplier = row.GetDouble(0);
	baseTimeMultiplier = row.GetDouble(1);
	costInstall = row.GetDouble(2);
	costPerHour = row.GetDouble(3);

	return true;
}

bool RamProxyDB::GetAssemblyLineVerifyProperties(const uint32 assemblyLineID, uint32 &ownerID, double &minCharSecurity, double &maxCharSecurity, EVERamRestrictionMask &restrictionMask, EVERamActivity &activity) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" ownerID,"
		" minimumCharSecurity,"
		" maximumCharSecurity,"
		" restrictionMask,"
		" activityID"
		" FROM ramAssemblyLines"
		" WHERE assemblyLineID = %u",
		assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to query verify properties for assembly line %u: %s.", assemblyLineID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No verify properties found for assembly line %u.", assemblyLineID);
		return false;
	}

	ownerID = row.GetUInt(0);
	minCharSecurity = row.GetDouble(1);
	maxCharSecurity = row.GetDouble(2);
	restrictionMask = (EVERamRestrictionMask)row.GetUInt(3);
	activity = (EVERamActivity)row.GetUInt(4);

	return true;
}

bool RamProxyDB::InstallJob(const uint32 ownerID, const  uint32 installerID, const uint32 assemblyLineID, const uint32 installedItemID, const uint64 beginProductionTime, const uint64 endProductionTime, const char *description, const uint32 runs, const EVEItemFlags outputFlag, const uint32 installedInSolarSystem, const int32 licensedProductionRuns) {
	DBerror err;

	// insert job
	if(!sDatabase.RunQuery(err,
		"INSERT INTO ramJobs"
		" (ownerID, installerID, assemblyLineID, installedItemID, installTime, beginProductionTime, endProductionTime, description, runs, outputFlag,"
		" completedStatusID, installedInSolarSystemID, licensedProductionRuns)"
		" VALUES"
		" (%u, %u, %u, %u, " I64u ", " I64u ", " I64u ", '%s', %u, %d, 0, %u, %li)",
		ownerID, installerID, assemblyLineID, installedItemID, Win32TimeNow(), beginProductionTime, endProductionTime, description,
		runs, (int)outputFlag, installedInSolarSystem, licensedProductionRuns))
	{
		_log(DATABASE__ERROR, "Failed to insert new job to database: %s.", err.c_str());
		return false;
	}

	// update nextFreeTime
	if(!sDatabase.RunQuery(err,
		"UPDATE ramAssemblyLines"
		" SET nextFreeTime = " I64u
		" WHERE assemblyLineID = %u",
		endProductionTime, assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to update next free time for assembly line %u: %s.", assemblyLineID, err.c_str());
		return false;
	}

	return true;
}

bool RamProxyDB::IsProducableBy(const uint32 assemblyLineID, const uint32 groupID) {
	double tmp;
	return(_GetMultipliers(assemblyLineID, groupID, tmp, tmp));
}

bool RamProxyDB::MultiplyMultipliers(const uint32 assemblyLineID, const uint32 productGroupID, double &materialMultiplier, double &timeMultiplier) {
	double tmpMat, tmpTime;
	if(!_GetMultipliers(assemblyLineID, productGroupID, tmpMat, tmpTime))
		return false;

	materialMultiplier *= tmpMat;
	timeMultiplier *= tmpTime;

	return true;
}

uint32 RamProxyDB::CountManufacturingJobs(const uint32 installerID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" COUNT(job.jobID)"
		" FROM ramJobs AS job"
		" LEFT JOIN ramAssemblyLines AS line ON job.assemblyLineID = line.assemblyLineID"
		" WHERE job.installerID = %u"
		" AND job.completedStatusID = 0"
		" AND line.activityID = 1",
		installerID))
	{
		_log(DATABASE__ERROR, "Failed to count manufacturing jobs for installer %u.", installerID);
		return 0;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No rows returned while counting manufacturing jobs for installer %u.", installerID);
		return 0;
	}

	return(row.GetUInt(0));
}
uint32 RamProxyDB::CountResearchJobs(const uint32 installerID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" COUNT(job.jobID)"
		" FROM ramJobs AS job"
		" LEFT JOIN ramAssemblyLines AS line ON job.assemblyLineID = line.assemblyLineID"
		" WHERE job.installerID = %u"
		" AND job.completedStatusID = 0"
		" AND line.activityID != 1",
		installerID))
	{
		_log(DATABASE__ERROR, "Failed to count research jobs for installer %u.", installerID);
		return 0;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No rows returned while counting research jobs for installer %u.", installerID);
		return 0;
	}

	return(row.GetUInt(0));
}

bool RamProxyDB::GetRequiredItems(const uint32 typeID, const EVERamActivity activity, std::vector<RequiredItem> &into) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" material.requiredTypeID,"
		" material.quantity,"
		" material.damagePerJob,"
		" IF(materialGroup.categoryID = 16, 1, 0) AS isSkill"
		" FROM typeActivityMaterials AS material"
		" LEFT JOIN invTypes AS materialType ON material.requiredTypeID = materialType.typeID"
		" LEFT JOIN invGroups AS materialGroup ON materialType.groupID = materialGroup.groupID"
		" WHERE material.typeID = %u"
		" AND material.activityID = %d"
		//this is needed as db is quite crappy ...
		" AND material.quantity > 0",
		typeID, (const int)activity))
	{
		_log(DATABASE__ERROR, "Failed to query data to build BillOfMaterials: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	while(res.GetRow(row))
		into.push_back(RequiredItem(row.GetUInt(0), row.GetUInt(1), row.GetFloat(2), row.GetInt(3) ? true : false));

	return true;
}

bool RamProxyDB::GetJobProperties(const uint32 jobID, uint32 &installedItemID, uint32 &ownerID, EVEItemFlags &outputFlag, uint32 &runs, uint32 &licensedProductionRuns, EVERamActivity &activity) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT job.installedItemID, job.ownerID, job.outputFlag, job.runs, job.licensedProductionRuns, assemblyLine.activityID"
		" FROM ramJobs AS job"
		" LEFT JOIN ramAssemblyLines AS assemblyLine ON job.assemblyLineID = assemblyLine.assemblyLineID"
		" WHERE job.jobID = %u",
		jobID))
	{
		_log(DATABASE__ERROR, "Failed to query properties of job %u: %s.", jobID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No properties found for job %u.", jobID);
		return false;
	}

	installedItemID = row.GetUInt(0);
	ownerID = row.GetUInt(1);
	outputFlag = (EVEItemFlags)row.GetUInt(2);
	runs = row.GetUInt(3);
	licensedProductionRuns = row.GetUInt(4);
	activity = (EVERamActivity)row.GetUInt(5);

	return true;
}

bool RamProxyDB::GetJobVerifyProperties(const uint32 jobID, uint32 &ownerID, uint64 &endProductionTime, EVERamRestrictionMask &restrictionMask, EVERamCompletedStatus &status) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT job.ownerID, job.endProductionTime, job.completedStatusID, line.restrictionMask"
				" FROM ramJobs AS job"
				" LEFT JOIN ramAssemblyLines AS line ON line.assemblyLineID = job.assemblyLineID"
				" WHERE job.jobID = %u",
				jobID))
	{
		_log(DATABASE__ERROR, "Unable to query completion properties for job %u: %s", jobID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No completion properties found for job %u.", jobID);
		return false;
	}

	ownerID = row.GetUInt(0);
	endProductionTime = row.GetUInt64(1);
	status = (EVERamCompletedStatus)row.GetUInt(2);
	restrictionMask = (EVERamRestrictionMask)row.GetUInt(3);

	return true;
}

bool RamProxyDB::CompleteJob(const uint32 jobID, const EVERamCompletedStatus completedStatus) {
	DBerror err;

	if(!sDatabase.RunQuery(err,
		"UPDATE ramJobs"
		" SET completedStatusID = %u"
		" WHERE jobID = %u",
		(uint32)completedStatus, jobID))
	{
		_log(DATABASE__ERROR, "Failed to complete job %u (completed status = %u): %s.", jobID, (uint32)completedStatus, err.c_str());
		return false;
	}

	return true;
}

std::string RamProxyDB::GetStationName(const uint32 stationID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT stationName"
		" FROM staStations"
		" WHERE stationID = %u",
		stationID))
	{
		_log(DATABASE__ERROR, "Failed to query station name of station %u: %s.", stationID, res.error.c_str());
		return("");
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Station %u not found.", stationID);
		return("");
	}

	return(row.GetText(0));
}

uint32 RamProxyDB::GetTech2Blueprint(const uint32 blueprintTypeID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT blueprintTypeID"
				" FROM invBlueprintTypes"
				" WHERE parentBlueprintTypeID = %u",
				blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Unable to get T2 type for type ID %u: %s", blueprintTypeID, res.error.c_str());
		return 0;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		// no error because it's normal
		return 0;
	}

	return(row.GetUInt(0));
}

uint64 RamProxyDB::GetNextFreeTime(const uint32 assemblyLineID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" nextFreeTime"
		" FROM ramAssemblyLines"
		" WHERE assemblyLineID = %u",
		assemblyLineID))
	{
		_log(DATABASE__ERROR, "Failed to query next free time for assembly line %u: %s.", assemblyLineID, res.error.c_str());
		return 0;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Assembly line %u not found.", assemblyLineID);
		return 0;
	} else
		return(row.GetUInt64(0));
}

uint32 RamProxyDB::GetRegionOfContainer(const uint32 containerID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT regionID"
				" FROM ramAssemblyLineStations"
				" WHERE stationID = %u",
				containerID))
	{
		_log(DATABASE__ERROR, "Unable to query region for container %u: %s", containerID, res.error.c_str());
		return 0;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No region found for container %u.", containerID);
		return 0;
	}

	return(row.GetUInt(0));

}

bool RamProxyDB::_GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier) {
	DBQueryResult res;

	// check table ramAssemblyLineTypeDetailPerGroup first
	if(!sDatabase.RunQuery(res,
				"SELECT materialMultiplier, timeMultiplier"
				" FROM ramAssemblyLineTypeDetailPerGroup"
				" JOIN ramAssemblyLines USING (assemblyLineTypeID)"
				" WHERE assemblyLineID = %u"
				" AND groupID = %u",
				assemblyLineID, groupID))
	{
		_log(DATABASE__ERROR, "Failed to check producability of group %u by line %u: %s", groupID, assemblyLineID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(res.GetRow(row)) {
		materialMultiplier = row.GetDouble(0);
		timeMultiplier = row.GetDouble(1);
		return true;
	}

	// then ramAssemblyLineTypeDetailPerCategory
	if(!sDatabase.RunQuery(res,
				"SELECT materialMultiplier, timeMultiplier"
				" FROM ramAssemblyLineTypeDetailPerCategory"
				" JOIN ramAssemblyLines USING (assemblyLineTypeID)"
				" JOIN invGroups USING (categoryID)"
				" WHERE assemblyLineID = %u"
				" AND groupID = %u",
				assemblyLineID, groupID))
	{
		_log(DATABASE__ERROR, "Failed to check producability of group %u by line %u: %s", groupID, assemblyLineID, res.error.c_str());
		return false;
	}

	if(res.GetRow(row)) {
		materialMultiplier = row.GetDouble(0);
		timeMultiplier = row.GetDouble(1);
		return true;
	} else {
		return false;
	}
}

