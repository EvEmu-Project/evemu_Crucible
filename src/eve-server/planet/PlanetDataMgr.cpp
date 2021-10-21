
 /**
  * @name PlanetDataMgr.cpp
  *   Specific Class for managing planet and pi data
  * @Author:         Allan
  * @date:   30 November 2016
  */


 /*
  * PLANET__ERROR
  * PLANET__WARNING
  * PLANET__MESSAGE
  * PLANET__DEBUG
  * PLANET__INFO
  * PLANET__TRACE
  * PLANET__DUMP
  * PLANET__RES_DUMP
  * PLANET__GC_DUMP
  * PLANET__PKT_TRACE
  * PLANET__DB_ERROR
  * PLANET__DB_WARNING
  */

#include "eve-server.h"

#include "inventory/ItemFactory.h"
#include "inventory/InventoryItem.h"
#include "planet/PlanetDataMgr.h"
#include "planet/Colony.h"

PlanetDataMgr::PlanetDataMgr()
{
}

int PlanetDataMgr::Initialize()
{
    _Populate();
    sLog.Blue("    PlanetDataMgr", "Planet Data Manager Initialized.");
    return 1;
}

void PlanetDataMgr::_Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetPlanetData(*res);
    while (res->GetRow(row)) {
        // SELECT planet.typeID, resource.typeID
        m_planetData.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("    PlanetDataMgr", "%u planet data groups in %u buckets loaded in %.3fms.",\
            m_planetData.size(), m_planetData.bucket_count(), (GetTimeMSeconds() - start));
}

void PlanetDataMgr::GetPlanetData(uint32 planetID, std::vector<uint16> &typeIDs)
{
    auto itr = m_planetData.equal_range(planetID);
    for (auto it = itr.first; it != itr.second; ++it)
        typeIDs.push_back(it->second);
}

const char* PlanetDataMgr::GetCommandName(int8 commandID)
{
    using namespace PI::Command;
    switch (commandID) {
        case Invalid:                       return "Invalid";
        case CreatePin:                     return "CreatePin";
        case RemovePin:                     return "RemovePin";
        case CreateLink:                    return "CreateLink";
        case RemoveLink:                    return "RemoveLink";
        case CreateRoute:                   return "CreateRoute";
        case SetLinkLevel:                  return "SetLinkLevel";
        case UpgradeCommandCenter:          return "UpgradeCommandCenter";
        case SetSchematic:                  return "SetSchematic";
        case RemoveRoute:                   return "RemoveRoute";
        case AddExtractorHead:              return "AddExtractorHead";
        case MoveExtractorHead:             return "MoveExtractorHead";
        case InstallProgram:                return "InstallProgram";
        case KillExtractorHead:             return "KillExtractorHead";
        case PrioritizeRoute:               return "PrioritizeRoute";
        default:                            return "UnknownCommandID";
    }
}


PIDataMgr::PIDataMgr()
{
}

int PIDataMgr::Initialize()
{
    _Populate();
    sLog.Blue("        PIDataMgr", "Planet Interaction Data Manager Initialized.");
    return 1;
}

// do we need anything from piPinMap?  - maps SchematicID to manuf facility's typeID
void PIDataMgr::_Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();

    m_db.GetSchematicData(*res);
    DBResultRow row;
    std::map<uint8, PI_Schematic>::iterator itr;
    while (res->GetRow(row)) {
        // SELECT `schematicID`, `typeID`, `quantity`, `isInput`
        itr = m_schematicData.find(row.GetInt(0));
        if (itr != m_schematicData.end()) {
            if (row.GetBool(3)) {
                itr->second.inputs[row.GetInt(1)] = row.GetInt(2);
            } else {
                itr->second.outputType = row.GetInt(1);
                itr->second.outputQty = row.GetInt(2);
            }
        } else {
            PI_Schematic data = PI_Schematic();
            if (row.GetBool(3)) {
                data.inputs[row.GetInt(1)] = row.GetInt(2);
            } else {
                data.outputType = row.GetInt(1);
                data.outputQty = row.GetInt(2);
            }
            m_schematicData[row.GetInt(0)] = data;
        }
    }

    m_db.GetSchematicTimes(*res);
    while (res->GetRow(row)) {
        // SELECT `schematicID`, `cycleTime`
        itr = m_schematicData.find(row.GetInt(0));
        if (itr != m_schematicData.end())
            itr->second.cycleTime = row.GetInt(1);
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("        PIDataMgr", "%u PI Schematic data groups loaded in %.3fms.", m_schematicData.size(), (GetTimeMSeconds() - start));
}

void PIDataMgr::GetSchematicData(uint8 schematicID, PI_Schematic& data)
{
    std::map<uint8, PI_Schematic>::iterator itr = m_schematicData.find(schematicID);
    if (itr != m_schematicData.end()) {
        data = itr->second;
        return;
    }
    _log(PLANET__ERROR, "PIDataMgr::GetSchematicData() - Data not found for schematic %u", schematicID);
}

/* these are for PI */
/*
 *    AttrHarvesterType = 709,
 *    AttrHarvesterQuality = 710,
 *    AttrLogisticalCapacity = 1631,
 *    AttrPlanetRestriction = 1632,
 *    AttrPowerLoadPerKm = 1633,
 *    AttrCPULoadPerKm = 1634,
 *    AttrCPULoadLevelModifier = 1635,
 *    AttrPowerLoadLevelModifier = 1636,
 *    AttrImportTax = 1638,
 *    AttrExportTax = 1639,
 *    AttrImportTaxMultiplier = 1640,
 *    AttrExportTaxMultiplier = 1641,
 *    AttrPinExtractionQuantity = 1642,
 *    AttrPinCycleTime = 1643,
 *    AttrExtractorDepletionRange = 1644,
 *    AttrExtractorDepletionRate = 1645,
 *    AttrCommandCenterHoldCapacity = 1646,
 *    AttrECUDecayFactor = 1683,            // this attr is empty
 *    AttrECUMaxVolume = 1684,
 *    AttrECUOverlapFactor = 1685,
 *    AttrECUNoiseFactor = 1687,            // this attr is empty
 *    AttrECUAreaOfInfluence = 1689,
 *    AttrECUExtractorHeadCPU = 1690,
 *    AttrECUExtractorHeadPower = 1691,
 *
 * decayFactor = 0.012
 * noiseFactor = 0.8
 * baseValue = 1998.0
 */

PyRep* PIDataMgr::GetProgramResultInfo(Colony* pColony, uint32 pinID, uint16 typeID, PyList* heads, float headRadius)
{
    //  ECU pinID, resource typeID, list of {headID, lat, long}, radius of head (small number...rad maybe?)
    // qtyToDistribute, cycleTime, numCycles = self.remoteHandler.GetProgramResultInfo(pinID, typeID, pin.heads, headRadius)

    /** @todo get head interference and calculate decayFactor, noiseFactor and overlapFactor for heads.  */

    /*
     *    SEC = 10000000L
     *    MIN = SEC * 60L
     *    HOUR = MIN * 60L
     * RADIUS_DRILLAREAMAX = 0.05
     * RADIUS_DRILLAREAMIN = 0.01
     * RADIUS_DRILLAREADIFF = RADIUS_DRILLAREAMAX - RADIUS_DRILLAREAMIN
     *
     * def GetProgramLengthFromHeadRadius(headRadius):
     *    return ((headRadius - RADIUS_DRILLAREAMIN) / RADIUS_DRILLAREADIFF) * 335 + 1   << length in hours between 1 and 336  (336h = 14d)
     * def GetCycleTimeFromProgramLength(programLength):
     *    return 0.25 * 2 ^ max(0, math.floor(math.log(programLength / 25.0, 2)) + 1)
     *
     *        programLength = planetCommon.GetProgramLengthFromHeadRadius(headRadius)
     *        cycleTime = planetCommon.GetCycleTimeFromProgramLength(programLength)
     *        numCycles = int(programLength / cycleTime)
     *        cycleTime = int(cycleTime * HOUR)
     */
    InventoryItemRef iRef = sItemFactory.GetItem(pinID);
    float cycleTime = iRef->GetAttribute(AttrPinCycleTime).get_int()/*300*/, length = 0;
    uint16 numCycles = 0;
    double one = ((headRadius - 0.01f) /0.04);
    length = one * 335 + 1;  //293
    double two = log2(length /25);  //3.584962501
    cycleTime = EvE::max(floor(two) + 1);    //4
    cycleTime = 0.25 * (pow(2, cycleTime));  // this is (float) in hours (0.25, 0.5, etc)
    numCycles = (uint16)(length / cycleTime);   //73
    int64 iCycleTime = cycleTime * EvE::Time::Hour;

    uint32 qtyPerCycle = GetProgramOutput(iRef, iCycleTime);
    //qtyPerCycle *= heads->size();

    _log(PLANET__TRACE, "PlanetMgr::GetProgramResultInfo() - cycleTime:%.2f, iCycleTime:%li, length:%.2f, numCycles:%u, qtyPerCycle:%u, heads: %u, headRadius:%.4f", \
                cycleTime, iCycleTime, length, numCycles, qtyPerCycle, heads->size(), headRadius);

    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyInt(qtyPerCycle));    //qtyToDistribute  (2843)
        res->SetItem(1, new PyLong(iCycleTime));    //cycleTime - in usec  (9000000000)
        res->SetItem(2, new PyInt(numCycles));      //numCycles   (12)

    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "    ");

    pColony->SetProgramResults(pinID, typeID, numCycles, headRadius, cycleTime, qtyPerCycle);

    /*  based on values noted in GetProgramOutput()
    13:17:37 [PlanetTrace] PlanetMgr::GetProgramResultInfo() -  cycleTime:0.25, iCycleTime:1251639296, length:8.28, numCycles:33, qtyPerCycle:1380 (one:0.02172, two:-1.59504) headRadius:0.0109
    13:17:37 [PlanetResDump]      Tuple: 3 elements
    13:17:37 [PlanetResDump]       [ 0]    Integer: 1380
    13:17:37 [PlanetResDump]       [ 1]       Long: 1251639296
    13:17:37 [PlanetResDump]       [ 2]    Integer: 33
    */
    PySafeDecRef(heads);
    return res;
}

/*              heads.append((headID, latitude, longitude))
                headID, phi, theta = head
                if surfacePoint:        <<--  this is mousePoint
                    theta = 2.0 * math.pi - surfacePoint.theta
                    phi = surfacePoint.phi
                else:
                    theta = 2.0 * math.pi - theta
                    phi = phi
                return max(0.0, builder.GetValueAt(self.sh, theta, phi))
                */
// ecu program methods from client

uint32 PIDataMgr::GetProgramOutput(InventoryItemRef iRef, int64 cycleTime, int64 startTime, int64 currentTime)
{
    // this is in client to display the Extractor window program results.

    if (startTime == 0)
        startTime = GetFileTimeNow() - 2 * EvE::Time::Second;
    if (currentTime == 0)
        currentTime = GetFileTimeNow();

    int cycleNum = EvE::max((currentTime - startTime + EvE::Time::Second) / cycleTime - 1, 1);
    float barWidth = cycleTime / EvE::Time::Second / 900.0; //0.13888
    float t = (cycleNum + 0.5f) * barWidth; // 0.20833
    // qtyPerCycle default is 1000.  it is reset to calculated after this returns
    /*
    uint32 qtyPerCycle = iRef->GetDefaultAttribute(AttrPinExtractionQuantity).get_uint32();
    float decayValue = qtyPerCycle / (1 + t * iRef->GetAttribute(AttrECUDecayFactor).get_float());     // 1000
    float f1 = 1.0f / 12;   // 0.08333
    float f2 = 1.0f / 5;    // 0.2
    float f3 = 1.0f / 2;    // 0.5
    float phaseShift = pow(qtyPerCycle, 0.7);   // 125.89254
    float sinA = cos(phaseShift + t * f1);      // 0.96985
    float sinB = cos(phaseShift / 2 + t * f2);  // 0.98784
    float sinC = cos(t * f3);                   // 0.99457
    float sinStuff = (sinA + sinB + sinC) / 3;  // 0.98408
    sinStuff = EvE::max(sinStuff);
    float barHeight = decayValue * (1 + iRef->GetAttribute(AttrECUNoiseFactor).get_float() * sinStuff);     //0.8
    */
    return uint32(barWidth * 1000);     // 0.13888 * 1000          16
}

uint32 PIDataMgr::GetProgramOutputPrediction(InventoryItemRef iRef, int64 cycleTime, uint32 numCycles/*0*/)
{
    uint32 val = 0;
    if (numCycles > 120)    // hardcoded in client
        numCycles = 120;
    for (int i=1; i <= numCycles; ++i)
        val += GetProgramOutput(iRef, cycleTime, i * cycleTime);
    return val;
}

uint32 PIDataMgr::GetMaxOutput(InventoryItemRef iRef, uint32 qtyPerCycle/*0*/, int64 cycleTime/*0*/)
{
    if (qtyPerCycle == 0)
        qtyPerCycle = iRef->GetAttribute(AttrPinExtractionQuantity).get_uint32();
    if (cycleTime == 0)
        cycleTime = iRef->GetAttribute(AttrPinCycleTime).get_int() * EvE::Time::Second; // base time is 300s
    float scalar = iRef->GetAttribute(AttrECUNoiseFactor).get_float() +1;
    return (scalar * qtyPerCycle) * cycleTime / EvE::Time::Second / 900.0;
}

uint16 PIDataMgr::GetHeadType(uint16 ecuTypeID, uint16 programType)
{
    switch (ecuTypeID) {
        case 2848: {  //Barren ECU
            switch (programType) {
                case 2268: return 2409; //Barren Aqueous Liquid Extractor
                case 2267: return 2430; //Barren Base Metals Extractor
                case 2270: return 2435; //Barren Noble Metals Extractor
                case 2073: return 2449; //Barren Microorganisms Extractor
                case 2288: return 2459; //Barren Carbon Compounds Extractor
            }
        } break;
        case 3060: {  //Gas ECU
            switch (programType) {
                case 2268: return 2416; //Gas Aqueous Liquid Extractor
                case 2309: return 2424; //Gas Ionic Solutions Extractor
                case 2310: return 2426; //Gas Noble Gas Extractor
                case 2311: return 2427; //Gas Reactive Gas Extractor
                case 2267: return 2433; //Gas Base Metals Extractor
            }
        } break;
        case 3061: {  //Ice ECU
            switch (programType) {
                case 2268: return 2415; //Ice Aqueous Liquid Extractor
                case 2310: return 2423; //Ice Noble Gas Extractor
                case 2073: return 2432; //Ice Microorganisms Extractor
                case 2286: return 2438; //Ice Planktic Colonies Extractor
                case 2272: return 2441; //Ice Heavy Metals Extractor
            }
        } break;
        case 3062: {  //Lava ECU
            switch (programType) {
                case 2308: return 2418; //Lava Suspended Plasma Extractor
                case 2267: return 2428; //Lava Base Metals Extractor
                case 2272: return 2439; //Lava Heavy Metals Extractor
                case 2306: return 2442; //Lava Non-CS Crystals Extractor
                case 2307: return 2448; //Lava Felsic Magma Extractor
            }
        } break;
        case 3063: {  //Oceanic ECU
            switch (programType) {
                case 2268: return 2414; //Oceanic Aqueous Liquid Extractor
                case 2287: return 2458; //Oceanic Complex Organisms Extractor
                case 2286: return 2452; //Oceanic Planktic Colonies Extractor
                case 2288: return 2461; //Oceanic Carbon Compounds Extractor
                case 2073: return 2451; //Oceanic Microorganisms Extractor
            }
        } break;
        case 3064: {  //Plasma ECU
            switch (programType) {
                case 2308: return 2417; //Plasma Suspended Plasma Extractor
                case 2267: return 2429; //Plasma Base Metals Extractor
                case 2270: return 2434; //Plasma Noble Metals Extractor
                case 2272: return 2440; //Plasma Heavy Metals Extractor
                case 2306: return 2443; //Plasma Non-CS Crystals Extractor
            }
        } break;
        case 3067: {  //Storm ECU
            switch (programType) {
                case 2268: return 2413; //Storm Aqueous Liquid Extractor
                case 2308: return 2419; //Storm Suspended Plasma Extractor
                case 2309: return 2422; //Storm Ionic Solutions Extractor
                case 2310: return 2425; //Storm Noble Gas Extractor
                case 2267: return 2431; //Storm Base Metals Extractor
            }
        } break;
        case 3068: {  //Temperate ECU
            switch (programType) {
                case 2268: return 2412; //Temperate Aqueous Liquid Extractor
                case 2073: return 2450; //Temperate Microorganisms Extractor
                case 2287: return 2453; //Temperate Complex Organisms Extractor
                case 2288: return 2460; //Temperate Carbon Compounds Extractor
                case 2305: return 2462; //Temperate Autotrophs Extractor
            }
        } break;
    }
    _log(PLANET__ERROR, "PIDataMgr::GetHeadType() - Extractor typeID not found using ECU typeID: %u and Resource typeID: %u", ecuTypeID, programType);
    return 2412; //Temperate Aqueous Liquid Extractor  <<< as good a default as any...
}

uint8 PIDataMgr::GetProductLevel(uint16 typeID)
{
    switch (typeID) {
    // P0 - Raw Materials
        case  2267: //Base Metals
        case  2270: //Noble Metals
        case  2272: //Heavy Metals
        case  2306: //Non-CS Crystals
        case  2307: //Felsic Magma
        case  2268: //Aqueous Liquids
        case  2308: //Suspended Plasma
        case  2309: //Ionic Solutions
        case  2310: //Noble Gas
        case  2311: //Reactive Gas
        case  2073: //Microorganisms
        case  2286: //Planktic Colonies
        case  2287: //Complex Organisms
        case  2288: //Carbon Compounds
        case  2305: //Autotrophs
            return 0;

    // P1 - Basic Commodities
        case  2389: //Plasmoids
        case  2390: //Electrolytes
        case  2392: //Oxidizing Compound
        case  2393: //Bacteria
        case  2395: //Proteins
        case  2396: //Biofuels
        case  2397: //Industrial Fibers
        case  2398: //Reactive Metals
        case  2399: //Precious Metals
        case  2400: //Toxic Metals
        case  2401: //Chiral Structures
        case  3779: //Biomass
        case  9828: //Silicon
        case  3683: //Oxygen
        case  3645: //Water
            return 1;

    // P2 - Refined Commodities
        case    44: //Enriched Uranium
        case  2312: //Supertensile Plastics
        case  2317: //Oxides
        case  2319: //Test Cultures
        case  2321: //Polyaramids
        case  2327: //Microfiber Shielding
        case  2328: //Water-Cooled CPU
        case  2329: //Biocells
        case  2463: //Nanites
        case  3689: //Mechanical Parts
        case  3691: //Synthetic Oil
        case  3693: //Fertilizer
        case  3695: //Polytextiles
        case  3697: //Silicate Glass
        case  3725: //Livestock
        case  3775: //Viral Agent
        case  3828: //Construction Blocks
        case  9830: //Rocket Fuel
        case  9832: //Coolant
        case  9836: //Consumer Electronics
        case  9838: //Superconductors
        case  9840: //Transmitter
        case  9842: //Miniature Electronics
        case 15317: //Genetically Enhanced Livestock
            return 2;

    // P3 - Specialized Commodities
        case  2344: //Condensates
        case  2345: //Camera Drones
        case  2346: //Synthetic Synapses
        case  2348: //Gel-Matrix Biopaste
        case  2349: //Supercomputers
        case  2351: //Smartfab Units
        case  2352: //Nuclear Reactors
        case  2354: //Neocoms
        case  2358: //Biotech Research Reports
        case  2360: //Industrial Explosives
        case  2361: //Hermetic Membranes
        case  2366: //Hazmat Detection Systems
        case  2367: //Cryoprotectant Solution
        case  9834: //Guidance Systems
        case  9846: //Planetary Vehicles
        case  9848: //Robotics
        case 12836: //Transcranial Microcontrollers
        case 17136: //Ukomi Superconductors
        case 17392: //Data Chips
        case 17898: //High-Tech Transmitters
        case 28974: //Vaccines
            return 3;

    // P4 - Advanced Commodities
        case  2867: //Broadcast Node
        case  2868: //Integrity Response Drones
        case  2869: //Nano-Factory
        case  2870: //Organic Mortar Applicators
        case  2871: //Recursive Computing Module
        case  2872: //Self-Harmonizing Power Core
        case  2875: //Sterile Conduits
        case  2876: //Wetware Mainframe
            return 4;
    }
    _log(PLANET__ERROR, "PIDataMgr::GetProductLevel() - Commodity product level not found for typeID: %u", typeID);
    EvE::traceStack();
    return 0;
}

const char* PIDataMgr::GetProductName(uint16 typeID)
{
    switch (typeID) {
        // P0 - Raw Materials
        case  2267:        return "Base Metals";
        case  2270:        return "Noble Metals";
        case  2272:        return "Heavy Metals";
        case  2306:        return "Non-CS Crystals";
        case  2307:        return "Felsic Magma";
        case  2268:        return "Aqueous Liquids";
        case  2308:        return "Suspended Plasma";
        case  2309:        return "Ionic Solutions";
        case  2310:        return "Noble Gas";
        case  2311:        return "Reactive Gas";
        case  2073:        return "Microorganisms";
        case  2286:        return "Planktic Colonies";
        case  2287:        return "Complex Organisms";
        case  2288:        return "Carbon Compounds";
        case  2305:        return "Autotrophs";
            // P1 - Basic Commodities
        case  2389:        return "Plasmoids";
        case  2390:        return "Electrolytes";
        case  2392:        return "Oxidizing Compound";
        case  2393:        return "Bacteria";
        case  2395:        return "Proteins";
        case  2396:        return "Biofuels";
        case  2397:        return "Industrial Fibers";
        case  2398:        return "Reactive Metals";
        case  2399:        return "Precious Metals";
        case  2400:        return "Toxic Metals";
        case  2401:        return "Chiral Structures";
        case  3779:        return "Biomass";
        case  9828:        return "Silicon";
        case  3683:        return "Oxygen";
        case  3645:        return "Water";
            // P2 - Refined Commodities
        case    44:        return "Enriched Uranium";
        case  2312:        return "Supertensile Plastics";
        case  2317:        return "Oxides";
        case  2319:        return "Test Cultures";
        case  2321:        return "Polyaramids";
        case  2327:        return "Microfiber Shielding";
        case  2328:        return "Water-Cooled CPU";
        case  2329:        return "Biocells";
        case  2463:        return "Nanites";
        case  3689:        return "Mechanical Parts";
        case  3691:        return "Synthetic Oil";
        case  3693:        return "Fertilizer";
        case  3695:        return "Polytextiles";
        case  3697:        return "Silicate Glass";
        case  3725:        return "Livestock";
        case  3775:        return "Viral Agent";
        case  3828:        return "Construction Blocks";
        case  9830:        return "Rocket Fuel";
        case  9832:        return "Coolant";
        case  9836:        return "Consumer Electronics";
        case  9838:        return "Superconductors";
        case  9840:        return "Transmitter";
        case  9842:        return "Miniature Electronics";
        case 15317:        return "Genetically Enhanced Livestock";
            // P3 - Specialized Commodities
        case  2344:        return "Condensates";
        case  2345:        return "Camera Drones";
        case  2346:        return "Synthetic Synapses";
        case  2348:        return "Gel-Matrix Biopaste";
        case  2349:        return "Supercomputers";
        case  2351:        return "Smartfab Units";
        case  2352:        return "Nuclear Reactors";
        case  2354:        return "Neocoms";
        case  2358:        return "Biotech Research Reports";
        case  2360:        return "Industrial Explosives";
        case  2361:        return "Hermetic Membranes";
        case  2366:        return "Hazmat Detection Systems";
        case  2367:        return "Cryoprotectant Solution";
        case  9834:        return "Guidance Systems";
        case  9846:        return "Planetary Vehicles";
        case  9848:        return "Robotics";
        case 12836:        return "Transcranial Microcontrollers";
        case 17136:        return "Ukomi Superconductors";
        case 17392:        return "Data Chips";
        case 17898:        return "High-Tech Transmitters";
        case 28974:        return "Vaccines";
            // P4 - Advanced Commodities
        case  2867:        return "Broadcast Node";
        case  2868:        return "Integrity Response Drones";
        case  2869:        return "Nano-Factory";
        case  2870:        return "Organic Mortar Applicators";
        case  2871:        return "Recursive Computing Module";
        case  2872:        return "Self-Harmonizing Power Core";
        case  2875:        return "Sterile Conduits";
        case  2876:        return "Wetware Mainframe";
    }
    _log(PLANET__ERROR, "PIDataMgr::GetProductName() - Commodity product not found for typeID: %u", typeID);
    return "NULL";
}

