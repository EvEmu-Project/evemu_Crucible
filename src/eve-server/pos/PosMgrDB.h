
/**
 * @name PosMgrDB.h
 *   DataBase methods for all POS items
 *
 * @Author:         Allan
 * @date:   8 December 17
 */


#ifndef EVEMU_POS_POSMGR_H_
#define EVEMU_POS_POSMGR_H_

#include "ServiceDB.h"
#include "../../eve-common/EVE_POS.h"

class ReactorData;
class StructureSE;
class PosMgrDB
: public ServiceDB
{
public:
    PyRep *GetSiloCapacityForType(uint16 typeID);
    PyRep* GetCorpControlTowers(uint32 corpID);

    static void GetLinkableJumpArrays(uint32 corpID, DBQueryResult& res);
    static void GetCorpJumpArrays(uint32 corpID, DBQueryResult& res);
    static void GetAllianceJumpArrays(uint32 allyID, DBQueryResult& res);

    void GetControlTowerFuelRequirements(DBQueryResult& res);

    // pos data methods
    void DeleteData(uint32 itemID);

    bool GetBaseData(EVEPOS::StructureData& data);
    void SaveBaseData(EVEPOS::StructureData& data);
    void UpdateBaseData(EVEPOS::StructureData& data);

    bool GetTowerData(EVEPOS::TowerData& tData, EVEPOS::StructureData& sData);
    void SaveTowerData(EVEPOS::TowerData& tData, EVEPOS::StructureData& sData);

    bool GetBridgeData(EVEPOS::JumpBridgeData& data);
    void SaveBridgeData(EVEPOS::JumpBridgeData& data);
    void UpdateBridgeData(EVEPOS::JumpBridgeData& data);

    bool GetReactorData(ReactorData* pData, EVEPOS::StructureData& sData);
    void SaveReactorData(ReactorData* pData, EVEPOS::StructureData& sData);
    void UpdateReactorData(ReactorData* pData, EVEPOS::StructureData& sData);

    bool GetCustomsData(EVEPOS::CustomsData& cData, EVEPOS::OrbitalData& oData);
    void SaveCustomsData(EVEPOS::CustomsData& cData, EVEPOS::OrbitalData& oData);
    void UpdateCustomsData(EVEPOS::CustomsData& cData, EVEPOS::OrbitalData& oData);

    void UpdateAccess(int32 itemID, EVEPOS::TowerData& data);
    void UpdateNotify(int32 itemID, EVEPOS::TowerData& data);
    void UpdateSentry(int32 itemID, EVEPOS::TowerData& data);
    void UpdatePassword(int32 itemID, EVEPOS::TowerData& data);
    void UpdateTimeStamp(int32 itemID, EVEPOS::StructureData& data);
    void UpdatePermission(int32 itemID, EVEPOS::TowerData& data);
    void UpdateUsageFlags(int32 itemID, EVEPOS::StructureData& data);
    void UpdateDeployFlags(int32 itemID, EVEPOS::TowerData& data);
    void UpdateHarmonicAndPassword(int32 itemID, EVEPOS::TowerData& data);

};

#endif  // EVEMU_POS_POSMGR_H_

/*environment/spaceObject/station.py*/
