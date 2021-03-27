
 /**
  * @name FactoryDB.h
  *   db query methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18
  */


#ifndef EVE_MANUF_FACTORYDB_H
#define EVE_MANUF_FACTORYDB_H

#include "ServiceDB.h"
#include "../eve-common/EVE_RAM.h"
#include "packets/Manufacturing.h"
#include "inventory/InventoryItem.h"

class Character;

class FactoryDB
: public ServiceDB
{
public:
    // client calls
    static PyRep* GetJobs2(const int32 ownerID, const bool completed);
    static PyRep* AssemblyLinesSelectPublic(const uint32 regionID);
    static PyRep* AssemblyLinesSelectPersonal(const uint32 charID);
    static PyRep* AssemblyLinesSelectPrivate(const uint32 charID);
    static PyRep* AssemblyLinesSelectCorporation(const uint32 corporationID);
    static PyRep* AssemblyLinesSelectAlliance(const int32 allianceID);
    static PyRep* AssemblyLinesGet(const uint32 containerID);
    static PyRep* GetMaterialCompositionOfItemType(const uint32 typeID);

    // for static data mgr
    static bool IsRefinable(const uint16 typeID);
    static bool IsRecyclable(const uint16 typeID);
    static void GetMinerals(DBQueryResult& res);
    static void GetRAMMaterials(DBQueryResult& res);
    static void GetBlueprintType(DBQueryResult& res);
    static void GetRAMRequirements(DBQueryResult& res);

    // InstallJob stuff
    static bool GetAssemblyLineProperties(const uint32 assemblyLineID, Character *pChar, Rsp_InstallJob &into, bool isCorpJob=false);
    static bool GetAssemblyLineRestrictions(const int32 assemblyLineID, EvERam::LineRestrictions& data);
    static uint32 InstallJob(const uint32 ownerID, const uint32 installerID, Call_InstallJob &args, const int64 beginTime, const int64 endTime);

    // CompleteJob stuff
    static bool GetJobProperties(const uint32 jobID, EvERam::JobProperties& data);
    static bool CompleteJob(const uint32 jobID, const int8 completedStatus);

    // misc queries
    static bool DeleteBlueprint(uint32 blueprintID);
    static bool GetBlueprint(uint32 blueprintID, EvERam::bpData& into);
    static bool SaveBlueprintData(uint32 blueprintID, EvERam::bpData& data);
    static bool IsProducableBy(const uint32 assemblyLineID, const ItemType *pType);
    static bool GetMultipliers(const uint32 assemblyLineID, const ItemType *pType, Rsp_InstallJob &into);

    static uint32 CountManufacturingJobs(const uint32 installerID);
    static uint32 CountResearchJobs(const uint32 installerID);
    static uint32 GetTech2Blueprint(const uint32 blueprintTypeID);

    static int64 GetNextFreeTime(const uint32 assemblyLineID);

    // for calendar events
    static void SetJobEventID(const uint32 jobID, const uint32 eventID);

};

#endif  // EVE_MANUF_FACTORYDB_H

