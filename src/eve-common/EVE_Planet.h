
/*  EVE_Planet.h
 *    enumerators and other defines for PI system
 *
 *  Base PI system code by Comet0
 * Updates and rewrites by Allan
 */


#ifndef EVE_PLANET_H
#define EVE_PLANET_H
#include <map>

#include "eve-common.h"

namespace PI {
    namespace Command {
        enum  {
            Invalid                 = 0,
            CreatePin               = 1,
            RemovePin               = 2,
            CreateLink              = 3,
            RemoveLink              = 4,
            SetLinkLevel            = 5,
            CreateRoute             = 6,
            RemoveRoute             = 7,
            SetSchematic            = 8,
            UpgradeCommandCenter    = 9,
            AddExtractorHead        = 10,
            KillExtractorHead       = 11,
            MoveExtractorHead       = 12,
            InstallProgram          = 13,
            PrioritizeRoute         = 14
        };
    }

    namespace Pin {
        namespace State {
            enum {
                Edit        = -2,
                Disabled    = -1,
                Idle        =  0,
                Active      =  1
            };
        }

        enum {
            Level0  = 0,
            Level1  = 1,
            Level2  = 2,
            Level3  = 3,
            Level4  = 4,
            Level5  = 5,
            // the following are only used by Planetary_Links
            Level6  = 6,
            Level7  = 7,
            Level8  = 8,
            Level9  = 9,
            Level10 = 10
        };
    }

    namespace Route {
        enum {
            PriorityLow  = -1,
            PriorityNorm =  0,
            PriorityHi   =  1
        };
    }

    //piLaunchOrbitDecayTime = DAY * 5
    namespace Cargo {
        enum {
            InOrbit     = 0,
            Deployed    = 1,
            Claimed     = 2,
            Deleted     = 3
        };
    }

}

namespace Launch {
    struct Data {
        uint8 status;
        uint32 launchID;
        uint32 itemID;
        uint32 solarSystemID;
        uint32 planetID;
        int64 launchTime;
        double x;
        double y;
        double z;
    };
}

/** @todo these need their own namespace */
/* POD structure entries for PI data */
struct PlanetResourceData {
    uint16 type_1;
    uint16 type_2;
    uint16 type_3;
    uint16 type_4;
    uint16 type_5;
    float dist_1;
    float dist_2;
    float dist_3;
    float dist_4;
    float dist_5;
    std::string buffer_1;
    std::string buffer_2;
    std::string buffer_3;
    std::string buffer_4;
    std::string buffer_5;
};

struct PI_Link {
    int8 state;
    uint16 level;
    uint16 typeID;
    uint32 endpoint1;
    uint32 endpoint2;
};

struct PI_Route {
    int8 state;
    int8 priority;
    uint16 commodityTypeID;
    uint16 commodityQuantity;
    uint32 srcPinID;
    uint32 destPinID;
    std::list<uint32> path;
};

struct PI_Heads {
    uint16 typeID;
    uint32 ecuPinID;
    double latitude;
    double longitude;
};

struct PI_Schematic {
    uint16 outputQty;
    uint16 outputType;
    uint16 cycleTime;                   // in seconds

    // typeID, qty
    std::map<uint16, uint16> inputs;
};

struct PI_Plant {
    // specifically for processing plants. this is not saved in db as a group, but is in pinData
    bool hasReceivedInputs :1;          // plant has received material from upstream process.  this enables check to verify type/qty for processing
    bool receivedInputsLastCycle :1;    // plant has received all required mat'l to make a production run.

    int8 state;
    uint8 pLevel;                       // production level of this plant
    uint8 schematicID;
    uint16 qtyPerCycle;
    int64 cycleTime;                    // in filetime
    //int64 expiryTime;                   // in filetime
    int64 installTime;                  // in filetime
    int64 lastRunTime;                  // in filetime

    PI_Schematic data;
};

struct PI_Pin {
    bool update :1;                     // specifically for updating contents in db. this is a runtime value.

    bool isECU :1;                      // common for all pins
    bool isBase :1;                     // common for all pins
    bool isStorage :1;                  // common for all pins
    bool isProcess :1;                  // common for all pins
    bool isConsumer :1;                 // common for all pins
    bool isLaunchable :1;               // common for all pins
    bool isCommandCenter :1;            // common for all pins

    // these two are checked in client for the pin.CanActivate() method.  it will return true if either are true.  that is the only reference i can find.
    bool hasReceivedInputs :1;          // Process Only
    bool receivedInputsLastCycle :1;    // Process Only

    int8 state;                         // common for all pins
    uint16 level;                       // common for all pins
    uint16 typeID;                      // common for all pins
    uint16 schematicID;                 // used in ecu as extractor head typeID
    uint16 programType;                 // used in extractors as extracted resource typeID
    uint32 qtyPerCycle;                 // Process and ECU
    uint32 ownerID;                     // common for all pins
    int64 lastRunTime;                  // common for all pins - copy of launchTime for Spaceports
    int64 cycleTime;                    // Process and ECU      // saved as filetime
    int64 expiryTime;                   // ECU Only             // saved as filetime
    int64 installTime;                  // Process and ECU processing time, Pin Creation Time for others      // saved as filetime
    int64 lastLaunchTime;               // Command Center and Spaceports  // saved as filetime

    float latitude;                     // planetary location common for all pins
    float longitude;                    // planetary location common for all pins

    float capacity;                     // pin volume cap in m3.  - this is not implemented yet
    float quantity;                     // volume of current contents in m3.  - this is not implemented yet
    float headRadius;                   // ECU Only

    std::map<uint16, PI_Heads> heads;   // ECU Only
    std::map<uint16, uint32> contents;  // Storage    <typeID, qty>
};


class PI_CCPin {
public:

    PI_CCPin()                                          { Init(); }
    ~PI_CCPin()                                         { /* do nothing here */ }

    void Clear()
    {
        pins.clear();
        links.clear();
        plants.clear();
        routes.clear();
    }
    void Init()
    {
        Clear();
        level = 0;
        ccPinID = 0;
        qtyPerCycle = 0;
    }

    uint8 GetLevel()                                    { return level; }
    uint32 GetPinID()                                   { return ccPinID; }

    uint8 level;
    uint32 ccPinID;
    uint32 qtyPerCycle; 

    // pinID, pinData
    std::map<uint32, PI_Pin> pins;
    // linkID, linkData
    std::map<uint32, PI_Link> links;
    // routeID, routeData
    std::map<uint16, PI_Route> routes;
    // plantPinID, plantData   - this dynamic data is not saved
    std::map<uint32, PI_Plant> plants;
};

/*  these are internal client state events
enum PlanetEvents {
    EVENT_NORMAL = 0,
    EVENT_BUILDPIN = 1,
    EVENT_CREATELINKSTART = 2,
    EVENT_CREATELINKEND = 3,
    EVENT_CREATEROUTE = 4,
    EVENT_SURVEY = 5,
    SUBEVENT_NORMAL = 6,
    SUBEVENT_MOVEEXTRACTIONHEAD = 7
}; */

                    /*
            Extractors = 1026,
            Command_Centers = 1027,
            Processors = 1028,
            Storage_Facilities = 1029,
            Spaceports = 1030,
            Planetary_Resources = 1031,
            Planet_Solid = 1032,
            Planet_Liquid_Gas = 1033,
            Refined_Commodities = 1034,
            Planet_Organic = 1035,
            Planetary_Links = 1036,
            Specialized_Commodities = 1040,
            Advanced_Commodities = 1041,
            Basic_Commodities = 1042,
            */
/*
     COMMAND_CREATEPIN: 'CreatePin',
     COMMAND_REMOVEPIN: 'RemovePin',
     COMMAND_CREATELINK: 'CreateLink',
     COMMAND_REMOVELINK: 'RemoveLink',
     COMMAND_SETLINKLEVEL: 'SetLinkLevel',
     COMMAND_CREATEROUTE: 'CreateRoute',
     COMMAND_REMOVEROUTE: 'RemoveRoute',
     COMMAND_SETSCHEMATIC: 'SetSchematic',
     COMMAND_UPGRADECOMMANDCENTER: 'UpgradeCommandCenter',
     COMMAND_ADDEXTRACTORHEAD: 'AddExtractorHead',
     COMMAND_KILLEXTRACTORHEAD: 'KillExtractorHead',
     COMMAND_MOVEEXTRACTORHEAD: 'MoveExtractorHead',
     COMMAND_INSTALLPROGRAM: 'InstallProgram'}
*/
/*
   __identifiers__ = {COMMAND_CREATEPIN: ['pinID'],
     COMMAND_REMOVEPIN: ['pinID'],
     COMMAND_CREATELINK: ['endpoint1', 'endpoint2'],
     COMMAND_REMOVELINK: ['endpoint1', 'endpoint2'],
     COMMAND_SETLINKLEVEL: ['endpoint1', 'endpoint2', 'level'],
     COMMAND_CREATEROUTE: ['routeID'],
     COMMAND_REMOVEROUTE: ['routeID'],
     COMMAND_SETSCHEMATIC: ['pinID'],
     COMMAND_UPGRADECOMMANDCENTER: ['pinID', 'level'],
     COMMAND_ADDEXTRACTORHEAD: ['pinID', 'headID'],
     COMMAND_KILLEXTRACTORHEAD: ['pinID', 'headID'],
     COMMAND_MOVEEXTRACTORHEAD: ['pinID', 'headID'],
     COMMAND_INSTALLPROGRAM: ['pinID']}

   __arguments__ = {COMMAND_CREATEPIN: ['typeID', 'latitude', 'longitude'],
     COMMAND_REMOVEPIN: [],
     COMMAND_CREATELINK: ['level'],
     COMMAND_REMOVELINK: [],
     COMMAND_SETLINKLEVEL: [],
     COMMAND_CREATEROUTE: ['path', 'typeID', 'quantity'],
     COMMAND_REMOVEROUTE: [],
     COMMAND_SETSCHEMATIC: ['schematicID'],
     COMMAND_UPGRADECOMMANDCENTER: [],
     COMMAND_ADDEXTRACTORHEAD: ['latitude', 'longitude'],
     COMMAND_KILLEXTRACTORHEAD: [],
     COMMAND_MOVEEXTRACTORHEAD: ['latitude', 'longitude'],
     COMMAND_INSTALLPROGRAM: ['typeID', 'headRadius']}
*/
/*
            info.proximity = const.planetResourceProximityPlanet
            for i, scanRange in enumerate(const.planetResourceScanningRanges):
                if scanRange >= dist:
                    info.proximity = i
        minBand, maxBand = const.planetResourceProximityLimits[info.proximity]
        info.newBand = min(maxBand, minBand + info.planetology + info.advancedPlanetology * 2)
        requiredSkill = 5 - info.proximity

 * planetResourceScanDistance = 1000000000
 * planetResourceProximityDistant = 0
 * planetResourceProximityRegion = 1
 * planetResourceProximityConstellation = 2
 * planetResourceProximitySystem = 3
 * planetResourceProximityPlanet = 4
 * planetResourceProximityLimits = [(2, 6),
 * (4, 10),
 * (6, 15),
 * (10, 20),
 * (15, 30)]
 * planetResourceScanningRanges = [9.0,
 * 7.0,
 * 5.0,
 * 3.0,
 * 1.0]
 * planetResourceUpdateTime = 1 * HOUR
 * planetResourceMaxValue = 1.21
 */

#endif  // EVE_PLANET_H
