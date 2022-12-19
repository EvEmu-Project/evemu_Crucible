/**
 * @name fxData.h
 *   This file is fxData container used with EffectsProcessor (FxProc), I had to split this to avoid circular dependencies on Windows
 *   Copyright 2022  EVEmu Team
 *
 * @Author:    Pursche
 * @date:      15 December 2022
 *
 */

#ifndef _EVE_FX_DATA_H__
#define _EVE_FX_DATA_H__

#include "../eve-server.h"

struct fxData {
    int8 math;          // math used on data
    int8 fxSrc;        // effect source location
    int8 targLoc;       // effect target location
    uint8 action;        // effect *DOES* something (module action aside from modification)
    uint16 targAttr;
    uint16 srcAttr;
    uint16 grpID;       // used to define items in env grouped by item groupID
    uint16 typeID;      // used to define items in env grouped by skill requirement
    InventoryItemRef srcRef;   // source item ref, if required
};
#endif // _EVE_FX_DATA_H__