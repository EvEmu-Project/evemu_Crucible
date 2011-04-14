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

#ifndef __RAM_PROXY_DB__H__
#define __RAM_PROXY_DB__H__

#include "inventory/InventoryItem.h"
#include "ServiceDB.h"

//from table 'ramActivities'
enum EVERamActivity {
	ramActivityManufacturing = 1,
	ramActivityResearchingTechnology = 2,
	ramActivityResearchingTimeProductivity = 3,
	ramActivityResearchingMaterialProductivity = 4,
	ramActivityCopying = 5,
	ramActivityDuplicating = 6,
	ramActivityReverseEngineering = 7,
	ramActivityInvention = 8
};

//from table 'ramCompletedStatuses'
enum EVERamCompletedStatus {
	ramCompletedStatusInProgress = 0,
	ramCompletedStatusDelivered = 1,
	ramCompletedStatusAbort = 2,
	ramCompletedStatusGMAbort = 3,
	ramCompletedStatusUnanchor = 4,
	ramCompletedStatusDestruction = 5
};

//restrictionMask from table 'ramAssemblyLines'
enum EVERamRestrictionMask {
	ramRestrictNone = 0,
	ramRestrictBySecurity = 1,
	ramRestrictByStanding = 2,
	ramRestrictByCorp = 4,
	ramRestrictByAlliance = 8
};

struct RequiredItem {
	RequiredItem(uint32 _typeID, uint32 _quantity, double _damagePerJob, bool _isSkill)
		: typeID(_typeID), quantity(_quantity), damagePerJob(_damagePerJob), isSkill(_isSkill) {}

	uint32 typeID;
	uint32 quantity;
	double damagePerJob;
	bool isSkill;
};

class RamProxyDB : public ServiceDB
{
public:
	PyRep *GetJobs2(const uint32 ownerID, const bool completed, const uint64 fromDate, const uint64 toDate);
	PyRep *AssemblyLinesSelectPublic(const uint32 regionID);
	PyRep *AssemblyLinesSelectPersonal(const uint32 charID);
	PyRep *AssemblyLinesSelectCorporation(const uint32 corporationID);
	PyRep *AssemblyLinesSelectAlliance(const uint32 allianceID);
	PyRep *AssemblyLinesGet(const uint32 containerID);

	// InstallJob stuff
	bool GetAssemblyLineProperties(const uint32 assemblyLineID, double &baseMaterialMultiplier, double &baseTimeMultiplier, double &costInstall, double &costPerHour);
	bool GetAssemblyLineVerifyProperties(const uint32 assemblyLineID, uint32 &ownerID, double &minCharSecurity, double &maxCharSecurity, EVERamRestrictionMask &restrictionMask, EVERamActivity &activity);
	bool InstallJob(const uint32 ownerID, const uint32 installerID, const uint32 assemblyLineID, const uint32 installedItemID, const uint64 beginProductionTime, const uint64 endProductionTime, const char *description, const uint32 runs, const EVEItemFlags outputFlag, const uint32 installedInSolarSystem, const int32 licensedProductionRuns);

	bool IsProducableBy(const uint32 assemblyLineID, const uint32 groupID);
	bool MultiplyMultipliers(const uint32 assemblyLineID, const uint32 productGroupID, double &materialMultiplier, double &timeMultiplier);
	uint32 CountManufacturingJobs(const uint32 installerID);
	uint32 CountResearchJobs(const uint32 installerID);

	bool GetRequiredItems(const uint32 typeID, const EVERamActivity activity, std::vector<RequiredItem> &into);

	// CompleteJob stuff
	bool GetJobProperties(const uint32 jobID, uint32 &installedItemID, uint32 &ownerID, EVEItemFlags &outputFlag, uint32 &runs, uint32 &licensedProductionRuns, EVERamActivity &activity);
	bool GetJobVerifyProperties(const uint32 jobID, uint32 &ownerID, uint64 &endProductionTime, EVERamRestrictionMask &restrictionMask, EVERamCompletedStatus &status);
	bool CompleteJob(const uint32 jobID, const EVERamCompletedStatus completedStatus);

	// other
	std::string GetStationName(const uint32 stationID);
	uint32 GetRegionOfContainer(const uint32 containerID);
	uint32 GetTech2Blueprint(const uint32 blueprintTypeID);
	uint64 GetNextFreeTime(const uint32 assemblyLineID);

protected:
	bool _GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier);
};

#endif

