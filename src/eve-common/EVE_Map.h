
/*
 *  EVE_Map.h
 *   map-specific enumerators
 *
 */

#ifndef EVE_MAP_H
#define EVE_MAP_H


struct JumpData {
    uint8 ctype;
    uint32 fromreg;
    uint32 fromcon;
    uint32 fromsol;
    uint32 tosol;
    uint32 tocon;
    uint32 toreg;
};

namespace Map {
    namespace Jumptype {
        enum {
            Region          = 0,    // purple
            Constellation   = 1,    // red
            System          = 2     // blue
        };
    }
}


#endif  // EVE_MAP_H