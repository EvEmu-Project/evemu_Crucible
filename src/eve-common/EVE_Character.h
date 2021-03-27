
/*
 *  EVE_Character.h
 *   Misc Character data
 *
 */

#ifndef EVE_CHARACTER_H
#define EVE_CHARACTER_H

namespace Char {

    namespace Race {
        enum {
            Caldari     = 1,
            Minmatar    = 2,
            Amarr       = 4,
            Ammatar     = 6,        // not an actual race, but combo of Minmatar and Amarr
            Gallente    = 8,
            Jove        = 16,
            Pirate      = 32,
            Sleepers    = 64,
            ORE         = 128
        };
    }

    namespace Type {
        enum {
            Amarr       = 1373,
            NiKunni     = 1374,
            Civire      = 1375,
            Deteis      = 1376,
            Gallente    = 1377,
            Intaki      = 1378,
            Sebiestor   = 1379,
            Brutor      = 1380,
            Static      = 1381,
            Modifier    = 1382,
            Achura      = 1383,
            JinMei      = 1384,
            Khanid      = 1385,
            Vherokior   = 1386

        };
    }

    namespace PDState {
        enum {
            //paperdoll state.  unused, but may be used later
            NoRecustomization           = 0,
            Resculpting                 = 1,
            NoExistingCustomization     = 2,
            FullRecustomizing           = 3,
            ForceRecustomize            = 4
        };
    }

    namespace Rookie {
        namespace Ship {
            enum {
                Amarr           = 596,
                Caldari         = 601,
                Gallente        = 606,
                Minmatar        = 588
            };
        }

        namespace Weapon {
            enum {
                Amarr           = 3634,
                Caldari         = 3638,
                Gallente        = 3640,
                Minmatar        = 3636
            };
        }
    }
}


#endif  // EVE_CHARACTER_H