 /**
  * @name AnomalyMgr.h
  *     Anomaly management system for EVEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */

#ifndef EVEMU_SYSTEM_ANOMALYMGR_H_
#define EVEMU_SYSTEM_ANOMALYMGR_H_

/*  this class is in charge of creating/destroying and maintaining
 * anomaly types in it's system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */


#include "system/cosmicMgrs/ManagerDB.h"

class DungeonMgr;
class BeltMgr;
class EVEServiceManager;
class SpawnMgr;
class SystemManager;

class AnomalyMgr
{
  public:
    AnomalyMgr(SystemManager* mgr, EVEServiceManager& svc);
    ~AnomalyMgr();

    bool Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr);
    void Close();
    void Process();

    void CreateAnomaly(int8 typeID);
    void SaveAnomaly(CosmicSignature& sig);
    void LoadAnomalies();

    //  assign sigID and add to anom list to allow showing on scanner
    void AddSignal(SystemEntity* pSE, uint32 id = 0);
    void RemoveSignal(uint32 itemID);
    // list for ship scanner
    void GetAnomalyList(std::vector< CosmicSignature >& sig);
    // list for probe
    void GetSignatureList(std::vector< CosmicSignature >& sig);

    uint32 GetAnomalyID(std::string& sigID);
    GPoint GetAnomalyPos(std::string& sigID);

    const char* GetScanGroupName(uint8 groupID=0);

    void RegisterExitWH(CosmicSignature &sig);

protected:
    ManagerDB m_mdb;
    ServiceDB m_sdb;

    uint8 GetDungeonType();

private:
    /* we do not own any of these (our sysmgr does) */
    BeltMgr* m_beltMgr;
    DungeonMgr* m_dungMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    EVEServiceManager& m_services;

    Timer m_spawnTimer;
    Timer m_procTimer;

    bool m_initalized;
    bool m_firstSpawn;

    // internal data counters   hard-capped at 256/128
    uint8 m_maxSigs;    // max total for this system
    uint8 m_Sigs; // total probe-needed items, hard-capped at 256
    // these should be fine soft-capped at 128
    int8 m_WH;    // < 0 means "not allowed"
    int8 m_Grav; // < 0 means "not allowed"
    int8 m_Mag; // < 0 means "not allowed"
    int8 m_Ladar; // < 0 means "not allowed"
    int8 m_Radar; // < 0 means "not allowed"
    // simple combat sites, no probe needed
    int8 m_Unrated;  // < 0 means "not allowed"
    // DED sites
    int8 m_Complex;  // < 0 means "not allowed"
    // system total, including pos, wrecks, ships.  65535 *should* be large enough
    uint16 m_Anoms; // this counts signals added thru sysmgr also

    std::vector<uint8> m_typeList; // List of pregenerated signature types for this system

    std::map<uint32, CosmicSignature> m_sigByItemID;            // signatures in system - need probes to scan down
    std::map<uint32, CosmicSignature> m_anomByItemID;           // anomalies in system - no probes needed
    std::map<std::string, CosmicSignature> m_sigBySigID;        // map for all signatures in system
};

#endif  // EVEMU_SYSTEM_ANOMALYMGR_H_
