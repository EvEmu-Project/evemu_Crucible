
 /**
  * @name PlanetDataMgr.h
  *   Specific Class for managing planet and pi data
  * @Author:         Allan
  * @date:   30 November 2016
  */


#ifndef EVEMU_PLANET_PLANETDATAMGR_H_
#define EVEMU_PLANET_PLANETDATAMGR_H_

#include <unordered_map>
#include "planet/PlanetDB.h"


// this class is a singleton object to have a common place for all (cached) planet data
class PlanetDataMgr
: public Singleton< PlanetDataMgr >
{
public:
    PlanetDataMgr();
    ~PlanetDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void GetPlanetData(uint32 planetID, std::vector<uint16> &typeIDs);

    const char* GetCommandName(int8 commandID);

protected:
    void _Populate();

private:
    PlanetDB m_db;

    std::unordered_multimap<uint32, uint32> m_planetData;
};

#define sPlanetDataMgr \
( PlanetDataMgr::get() )


class Colony;

// this class is a singleton object to have a common place for all (cached) PI schematic and program data
class PIDataMgr
: public Singleton< PIDataMgr >
{
public:
    PIDataMgr();
    ~PIDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    PyRep* GetProgramResultInfo(Colony* pColony, uint32 pinID, uint16 typeID, PyList* heads, float headRadius);

    void GetSchematicData(uint8 schematicID, PI_Schematic& data);

    uint8 GetProductLevel(uint16 typeID);
    const char* GetProductName(uint16 typeID);
    uint16 GetHeadType(uint16 ecuTypeID, uint16 programType);

    uint32 GetMaxOutput(InventoryItemRef iRef, uint32 qtyPerCycle = 0, int64 cycleTime = 0);
    uint32 GetProgramOutput(InventoryItemRef iRef, int64 cycleTime, int64 startTime=0, int64 currentTime=0);
    uint32 GetProgramOutputPrediction(InventoryItemRef iRef, int64 cycleTime, uint32 numCycles = 0);

protected:
    void _Populate();

private:
    PlanetDB m_db;

    std::map<uint8, PI_Schematic> m_schematicData;
};

#define sPIDataMgr \
( PIDataMgr::get() )


#endif  // EVEMU_PLANET_PLANETDATAMGR_H_