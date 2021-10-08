
 /**
  * @name AllianceData.h
  *     Alliance Data containers for EVEmu
  *
  * @Author:        James
  * @date:          7 April 2021
  */

#ifndef EVEMU_SRC_ALLY_DATA_H_
#define EVEMU_SRC_ALLY_DATA_H_


#include "../eve-server.h"

    /*
    "corpRole"      - corporation management-type roles (manager, officer, trader)  NOT access-type roles
    "rolesAtAll"    - access roles everywhere.  is joined with other access roles
    "rolesAtBase"   - access roles at base defined for this char. overrides hq if hq and base are same location for char
    "rolesAtHQ"     - access roles at corp HQ.
    "rolesAtOther"  - access roles for non-station containers with corp hangars
    */


namespace Alliance {

    /* POD structure for alliance app data */
    struct ApplicationInfo {
        bool valid;
        uint32 appID;
        uint32 allyID;
        uint32 corpID;
        uint32 state;
        uint32 deleted;
        int64 appTime;
        std::string appText;
    };

}

#endif  // EVEMU_SRC_CORP_DATA_H_