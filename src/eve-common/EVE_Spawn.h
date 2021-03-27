
 /**
  * @name EVE_Spawn.h
  *     NPC Spawn system data for EVEmu
  *
  * @Author:    Allan
  * @date:      25Jan18
  *
  */


#ifndef EVE_SPAWN_H
#define EVE_SPAWN_H


namespace Spawn {

    // these class names correspond to the "type" of spawn data found in npcSpawnClass table
    namespace Class {
        enum {
            // belt and grav site spawns - grav rat class is system rat class +1
            None         = 0,
            Easy         = 1,
            Fair         = 2,
            Average      = 3,
            Medium       = 4,
            Hard         = 5,
            Crazy        = 6,
            Insane       = 7,
            Hell         = 8,
            Extra        = 9,    // placeholder - not used yet
            Hauler       = 10,
            Commander    = 11,
            Officer      = 12,
            
            BeltSpawn    = 19,   // test spot for non-belt/gate

            // W.I.P.
            // anomaly faction spawns...these have waves
            Hideaway     = 20,
            Burrow       = 21,
            Refuge       = 22,
            Den          = 23,
            Yard         = 24,
            RallyPoint   = 25,
            Port         = 26,
            Hub          = 27,
            Haven        = 28,
            Sanctum      = 29,
            // anomaly drone spawns...these have waves
            Cluster      = 30,
            Collection   = 31,
            Assembly     = 32,
            Gathering    = 33,
            Surveillance = 34,
            Menagerie    = 35,
            Herd         = 36,
            Squad        = 37,
            Patrol       = 38,
            Horde        = 39,
            // unrated faction spawns..these have waves and pockets
            Hideout      = 40,
            Lookout      = 41,
            Watch        = 42,
            Vigil        = 43,
            Outpost      = 44,
            Annex        = 45,
            Base         = 46,
            Fortress     = 47,
            Complex      = 48,
            StagingPoint = 49,
            // unrated drone spawns..these have waves and pockets
            HauntedYard  = 50,
            DesolateSite = 51,
            ChemicalYard = 52,
            TrialYard    = 53,
            DirtySite    = 54,
            Ruins        = 55,
            Independence = 56,
            Radiance     = 57,
            Hierarchy    = 58,
            // mag site spawns  -rats dont spawn in these sites on live.
            //   they will here.
            // salvage site (mag subcategory)
            Crumbling = 60,
            Decayed = 61,
            Ruined = 62,
            // relic site (mag subcategory)
            Looted = 65,
            Ransacked = 66,
            Pristine = 67,

            // radar site spawns    - 5 groups in each, from easy to deadly (Easy, Average, Medium, Hard, Insane)
            Shard = 70,
            Tower = 71,
            Mainframe = 72,
            Center = 73,
            Server  = 74,

            // ladar site spawns
            // lots of weird names for these....not sure how im gonna do them yet.

            // ded rated spawns  5 factions, 9 sites each, waves, pockets and gates.  drones have 3 sites on live.  (5-6 here)
            // these will start at ??

            // escalation spawns  -this will be a fair amount of data.
            // these will start at 1c

            // Mission spawns   -this will be a LOT of data...650+ missions (that i know of at this time)
            // these will start at 1k

        };
    }

    // this is currently unused, but have plans for future expansion.
    namespace Group {
        enum {
            None        = 0,
            Roaming     = 1,    //roid/gate/rats
            Static      = 2,    //ded sites are 'static'
            Anomaly     = 3,    //basic combat site
            Combat      = 4,    // non-anomaly combat site
            Deadspace   = 5,    //all non-static, non-combat sites
            Mission     = 6,
            Incursion   = 7,
            Sleeper     = 8,
            Escalation  = 9
        };

    }
}


#endif  // EVE_SPAWN_H