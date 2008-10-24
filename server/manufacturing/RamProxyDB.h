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

#ifndef __RAM_PROXY_DB__H__
#define __RAM_PROXY_DB__H__

#include "../ServiceDB.h"
#include "PyRep.h"
#include "../inventory/InventoryItem.h"

//from table 'ramActivities'
enum EVERamActivity {
	ramActivityNone = 0,
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

class RamProxyDB : public ServiceDB {
private:
	bool _GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier);

public:
	RamProxyDB(DBcore *db);
	virtual ~RamProxyDB();

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

	bool GetAdditionalBlueprintProperties(const uint32 blueprintID, uint32 &materialLevel, double &wasteFactor, uint32 &productivityLevel, uint32 &productivityModifier);
	bool GetRequiredItems(const uint32 typeID, const EVERamActivity activity, std::vector<RequiredItem> &into);

	// CompleteJob stuff
	bool GetJobProperties(const uint32 jobID, uint32 &installedItemID, uint32 &ownerID, EVEItemFlags &outputFlag, uint32 &runs, uint32 &licensedProductionRuns, EVERamActivity &activity);
	bool GetJobVerifyProperties(const uint32 jobID, uint32 &ownerID, uint64 &endProductionTime, EVERamRestrictionMask &restrictionMask, EVERamCompletedStatus &status);
	bool CompleteJob(const uint32 jobID, const EVERamCompletedStatus completedStatus);

	// other
	std::string GetTypeName(const uint32 typeID);
	uint32 GetGroup(const uint32 typeID);
	EVERace GetRace(const uint32 typeID);
	uint32 GetPortionSize(const uint32 typeID);

	std::string GetStationName(const uint32 stationID);
	uint32 GetRegionOfContainer(const uint32 containerID);

	uint32 GetBlueprintProductionTime(const uint32 blueprintTypeID, const EVERamActivity activity);
	uint32 GetMaxProductionLimit(const uint32 blueprintTypeID);
	uint32 GetTech2Blueprint(const uint32 blueprintTypeID);

	uint64 GetNextFreeTime(const uint32 assemblyLineID);
};

#endif

