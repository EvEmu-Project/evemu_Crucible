/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://github.com/evemuproject/evemu_server
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Cometo
 *    Updates:  Allan
 */

#ifndef __COLONY_H_INCL__
#define __COLONY_H_INCL__

#include "PyCallable.h"

#include "planet/PlanetDB.h"

class PlanetSE;
class SystemEntity;
class PlanetMgr;

class Colony {
public:
    Colony(PyServiceMgr* mgr, Client* pClient, SystemEntity* pSE);
    ~Colony();

    void Init();
    void Load();
    void LoadPlants();    // for loading current data to pins
    void Save();
    void Update(bool updateTimes=false);
    void Shutdown();
    void UpdatePlantPins(uint32 pinID=0);  // for saving current data from runtime plantPin to saved colonyPin
    void AbandonColony();

    void Process();
    void ProcessECUs(bool& save);
    void ProcessPlants(bool& save);

    void RemovePin(uint32 pinID);
    void RemoveLink(uint32 src, uint32 dest);
    void RemoveRoute(uint16 routeID);

    void UpgradeLink(uint32 src, uint32 dest, uint8 level);
    void UpgradeCommandCenter(uint32 pinID, int8 level);

    void CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude);
    void CreateLink(uint32 src, uint32 dest, uint16 level);
    void CreateRoute(uint16 routeID, uint32 typeID, uint32 qty, PyList* path);
    void CreateCommandPin(uint32 itemID, uint32 typeID, double latitude, double longitude);

    void AddExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude);
    void MoveExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude);
    void KillExtractorHead(uint32 ecuID, uint16 headID);

    void InstallProgram(uint32 ecuID, uint16 typeID, float headRadius, PlanetMgr* pPMgr);
    void SetSchematic(uint32 pinID, uint8 schematicID=0);
    void SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, float headRadius, float cycleTime, uint32 qtyPerCycle);

    PyRep* LaunchCommodities(uint32 pinID, std::map< uint16, uint32 >& items);
    void PlanetXfer(uint32 spaceportID, std::map< uint32, uint16 > importItems, std::map< uint32, uint16 > exportItems, double taxRate);

    void PrioritizeRoute(uint16 routeID, uint8 priority);

    uint32 GetOwner();

    PyRep* GetColony();
    PyTuple* GetPins();
    PyTuple* GetLinks();
    PyTuple* GetRoutes();
    PyDict* TransferCommodities(uint32 srcID, uint32 destID, std::map< uint16, uint32 > items);

    bool HasColony()                                    { return (ccPin->ccPinID ? true : false); }

    int8 GetLevel()                                     { return ccPin->level; }
    int64 GetSimTime()                                  { return m_procTime; }

private:
    PyServiceMgr* m_svcMgr;
    PlanetSE* m_pSE;
    PI_CCPin* ccPin;
    Client* m_client;

    Timer m_colonyTimer;

    PlanetDB m_db;

    bool m_active;
    bool m_loaded;
    bool m_newHead;
    bool m_toUpdate;

    uint8 m_pLevel;
    uint16 m_pg;
    uint16 m_cpu;
    uint32 m_colonyID;

    int64 m_procTime;

    std::vector<uint32> tempECUs;
    std::map<uint8, uint32> tempPinIDs;
    // pLevel, pinID
    std::multimap<uint8, uint32> m_plantMap;        // map plant's P level to pinID.  to be used during Update()
    // srcPinID, routeData
    std::multimap<uint32, PI_Route> m_srcRoutes;     // map sourcePin to routeData
    // destPinID, routeData
    std::multimap<uint32, PI_Route> m_destRoutes;     // map destPin to routeData
};

/*
 *
 *  PlanetaryImportTax = 96,     // * Planet ID
 *  PlanetaryExportTax = 97,     // * Planet ID
 *  PlanetaryConstruction = 98,
 *  AttrImportTax = 1638,
 *  AttrExportTax = 1639,
 *  AttrImportTaxMultiplier = 1640,
 *  AttrExportTaxMultiplier = 1641,
 *  AttrnpcCustomsOfficeTaxRate = 1780,
 *  AttrdefaultCustomsOfficeTaxRate = 1781,
 *
 Command Center Properties
 Level    Capy    CPU        PG           Upgrade Cost
 0       500 m3  1,675 tf    6,000 MW       0  ISK
 1       500 m3  7,057 tf    9,000 MW     580k ISK
 2       500 m3  12,136 tf   12,000 MW    930k ISK
 3       500 m3  17,215 tf   15,000 MW    1.2m ISK
 4       500 m3  21,315 tf   17,000 MW    1.5m ISK
 5       500 m3  25,415 tf   19,000 MW    2.1m ISK

 Structure Properties
 Name                        CPU         Power       Cost
 Extractor Control Unit      400 tf      2600 MW      45m ISK
 Extractor Head              110 tf      550 MW        0  ISK
 Basic Industry Facility     200 tf      800 MW       75m ISK
 Advanced Industry Facility  500 tf      700 MW      250m ISK
 High-Tech Industry Facility 1100 tf     400 MW      525m ISK
 Storage Facility            500 tf      700 MW      250m ISK
 Space Port                  3600 tf     700 MW      900m ISK


 *
 * Link Requirements by Distance
 Distance   CPU         Power
 2.5 km      16 tf       11 MW
 10 km       18 tf       12 MW
 20 km       20 tf       14 MW
 50 km       26 tf       18 MW
 100 km      36 tf       26 MW
 200 km      56 tf       41 MW
 500 km      116 tf      86 MW
 1000 km     215 tf      160 MW
 2000 km     416 tf      311 MW
 5000 km     1016 tf     761 MW
 40000 km    8016 tf     6001 MW


 Link Upgrade Costs

 Data on relative costs of upgrading the link capacity (uses a link that is 500km as a base):
 Level   Capacity    CPU         Power
 0       250 m3      116 tf      86 MW
 1       500 m3      280 tf      183 MW
 2       1000 m3     481 tf      291 MW
 3       2000 m3     713 tf      407 MW
 4       4000 m3     968 tf      528 MW
 5       8000 m3     1245 tf     655 MW
 6       16 km3      1542 tf     786 MW
 7       32 km3      1855 tf     921 MW
 8       64 km3      2185 tf     1059 MW
 9       128 km3     2530 tf     1200 MW
 10      256 km3     2889 tf     1344 MW

 */
#endif

/* {'messageKey': 'RouteFailedValidationCannotRouteCommodities', 'dataID': 17875533, 'suppressable': False, 'bodyID': 256544, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3293}
 * {'messageKey': 'RouteFailedValidationExpeditedDestinationCannotAccept', 'dataID': 17876139, 'suppressable': False, 'bodyID': 256765, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3376}
 * {'messageKey': 'RouteFailedValidationExpeditedSourceLacksCommodity', 'dataID': 17876179, 'suppressable': False, 'bodyID': 256779, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3322}
 * {'messageKey': 'RouteFailedValidationExpeditedSourceLacksCommodityQty', 'dataID': 17876176, 'suppressable': False, 'bodyID': 256778, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3323}
 * {'messageKey': 'RouteFailedValidationExpeditedSourceNotReady', 'dataID': 17875596, 'suppressable': False, 'bodyID': 256567, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3325}
 * {'messageKey': 'RouteFailedValidationExpeditedSourceNotStorage', 'dataID': 17875590, 'suppressable': False, 'bodyID': 256565, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3321}
 * {'messageKey': 'RouteFailedValidationLinkDoesNotExist', 'dataID': 17875530, 'suppressable': False, 'bodyID': 256543, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3292}
 * {'messageKey': 'RouteFailedValidationNoBandwidthAvailable', 'dataID': 17875593, 'suppressable': False, 'bodyID': 256566, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3324}
 * {'messageKey': 'RouteFailedValidationPinDoesNotExist', 'dataID': 17875524, 'suppressable': False, 'bodyID': 256541, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3290}
 * {'messageKey': 'RouteFailedValidationPinNotYours', 'dataID': 17875527, 'suppressable': False, 'bodyID': 256542, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3291}
 * {'messageKey': 'RouteRemoveNotFound', 'dataID': 17875539, 'suppressable': False, 'bodyID': 256546, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3295}
 * {'messageKey': 'RouteRemoveRouteNotYours', 'dataID': 17875542, 'suppressable': False, 'bodyID': 256547, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3296}
 * {'messageKey': 'RunProcessCycle', 'dataID': 17881150, 'suppressable': False, 'bodyID': 258668, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258667, 'messageID': 1969}
 */
