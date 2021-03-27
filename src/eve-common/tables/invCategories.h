#ifndef EVEDB_invCategories_H__
#define EVEDB_invCategories_H__

namespace EVEDB {
    namespace invCategories {
        enum {
            _System = 0,
            Owner = 1,          // char, corp, faction, alliance items.
            Celestial = 2,
            Station = 3,
            Material = 4,
            Accessories = 5,    // clone, voucher, outpost improvement/upgrade, plex
            Ship = 6,
            Module = 7,
            Charge = 8,
            Blueprint = 9,
            Trading = 10,       // not sure how to use this one....may look into later
            Entity = 11,        // includes control bunkers and player drones
            Bonus = 14,         // trait bonus/handicap.  not sure how/if to use these.
            Skill = 16,
            Commodity = 17,
            Drone = 18,
            Implant = 20,
            Deployable = 22,
            Structure = 23,
            Reaction = 24,
            Asteroid = 25,
            WorldSpace = 26,    // not sure about this cat.  gonna go with "not used"
            Abstract = 29,      // not sure how to use this one....may look into later
            Apparel = 30,       // i dont think these are in market, but they do have a mktGrpID.  not sure.
            Subsystem = 32,
            AncientRelics = 34,
            Decryptors = 35,
            StructureUpgrade = 39,
            SovereigntyStructure = 40,
            PlanetaryInteraction = 41,
            PlanetaryResources = 42,
            PlanetaryCommodities = 43,
            Orbitals = 46,
            // WIS items.  not sure if these are used.  they are listed as 'published'
            Placeables = 49,    // furniture, etc
            Effects = 53,       // lighting-type effects.  non-creatable item type
            Lights = 54,        // light sources.  non-creatable item type
            Cells = 59          // not used
        };
    }
}

#endif

/*
 * extra shit....
 * cat 11:
52   Trading     0   94  Trading
53  Trade Session   0   95  Trade Session

 * cat 29:
 *  typeID  typeName    marketGroupID   groupID     groupName
2733    Schematic   0   937     Decorations
29495   Rank    0   937     Decorations
29496   Medal   0   937     Decorations
29497   Ribbon  0   937     Decorations
29530   Certificate     0   937     Decorations
3859    genericAudioEmitter     0   1109    Audio
32682   Ship Perception Point   0   1121    Perception Points

*/
