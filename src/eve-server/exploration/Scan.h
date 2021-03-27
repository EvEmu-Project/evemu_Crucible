 /**
  * @name Scan.h
  *   Scanning methods for EVEmu
  *
  * @Author:        Allan
  * @date:          7Dec15 (working)
  *
  */


#ifndef EVEMU_SCANING_SCAN_H_
#define EVEMU_SCANING_SCAN_H_

#include "packets/Scan.h"
#include "system/cosmicMgrs/ManagerDB.h"

struct SignalData {
    float distance;
    float certainty;
    float deviation;
    CosmicSignature sig;
    PyRep* probes;
    PyRep* probePos;
};


class PyRep;
class Client;
class ProbeSE;
class SystemManager;

class Scan {
public:
    Scan(Client* pClient);
    //Scan(Scan& scan);
    ~Scan()                                             { /* do nothing here */ }

    // called by client timer, which was set for probeStateChange
    void ProcessScan(bool useProbe=false);

    uint8 GetProbeCount()                               { return (uint8)m_probeMap.size(); }

    PyRep* ConeScan(Call_ConeScan args);
    void RequestScans(PyDict* dict);
    void ShipScanResult();
    void ProbeScanResult();

    void SystemScanStarted(uint16 duration);

    // probe methods
    void AddProbe(ProbeSE* pProbe);
    void RemoveProbe(ProbeSE* pProbe);

    bool GetProbeDataForSig(SignalData& data);
    // this will determine signal strength, deviation, and other aspects based on signal type and probe data
    void GetSignalData(SignalData& data, std::vector<ProbeSE*>& probeVec);

protected:
    void ScanStart();
    void SurveyScan();

private:
    ManagerDB* m_db;
    Client* m_client;
    SystemManager* m_system;

    void CalcProbeAngles(GPoint& sigPos, std::vector<ProbeSE*>& probeVec, std::map<float, std::pair<ProbeSE*, ProbeSE*>>& angleMap);

    bool m_probeScan;

    std::map<uint32, ProbeSE*> m_probeMap;        // probeID/ProbeSE*
    std::map<uint32, ProbeSE*> m_activeProbeMap;  // probeID/ProbeSE*
};

#endif // EVEMU_SCANING_SCAN_H_

/*{'messageKey': 'ScnAlreadyActive', 'dataID': 17879691, 'suppressable': False, 'bodyID': 258103, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2050}
 * {'messageKey': 'ScnAlreadyInProgress', 'dataID': 17880021, 'suppressable': False, 'bodyID': 258232, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2234}
 * {'messageKey': 'ScnGrpsNotEnoughSkill', 'dataID': 17879688, 'suppressable': False, 'bodyID': 258102, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2049}
 * {'messageKey': 'ScnMixedProbes', 'dataID': 17880018, 'suppressable': False, 'bodyID': 258231, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2233}
 * {'messageKey': 'ScnNoLauncher', 'dataID': 17881289, 'suppressable': False, 'bodyID': 258722, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258721, 'messageID': 1506}
 * {'messageKey': 'ScnNoMaterials', 'dataID': 17881579, 'suppressable': False, 'bodyID': 258824, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258823, 'messageID': 1507}
 * {'messageKey': 'ScnNoProbes', 'dataID': 17881292, 'suppressable': False, 'bodyID': 258723, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1508}
 * {'messageKey': 'ScnNoResults', 'dataID': 17881295, 'suppressable': False, 'bodyID': 258724, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1509}
 * {'messageKey': 'ScnProbeForShips', 'dataID': 17880384, 'suppressable': False, 'bodyID': 258371, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2231}
 * {'messageKey': 'ScnProbeProximity', 'dataID': 17879709, 'suppressable': False, 'bodyID': 258110, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2072}
 * {'messageKey': 'ScnProbeRecoverToPod', 'dataID': 17877357, 'suppressable': False, 'bodyID': 257220, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257219, 'messageID': 2862}
 * {'messageKey': 'ScnProbeUnusable', 'dataID': 17880172, 'suppressable': False, 'bodyID': 258291, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258290, 'messageID': 2195}
 * {'messageKey': 'ScnShipIsCapsule', 'dataID': 17878793, 'suppressable': False, 'bodyID': 257760, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257759, 'messageID': 2315}
 * {'messageKey': 'ScnShipScannerDeadspaceOnly', 'dataID': 17878747, 'suppressable': False, 'bodyID': 257742, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257741, 'messageID': 2286}
 * {'messageKey': 'ScnShipScannerTooManyProbes', 'dataID': 17878742, 'suppressable': False, 'bodyID': 257740, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257739, 'messageID': 2285}
 */

