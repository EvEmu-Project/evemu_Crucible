
/*
 *  EVE_Player.h
 *   Player data to avoid recompile on change
 *
 */

#ifndef EVE_PLAYER_H
#define EVE_PLAYER_H


//  -updated 18Dec16  -again 12Apr20
namespace Player {

    namespace Timer {
        enum {
            Default             = 1000,
            Board               = 600,
            Jump                = 1800,    // used to delay sending Destiny::State (client error fix)
            Undock              = 2000,    // used to delay sending Destiny::State (client error fix)
            Docking             = 1000,    //  to delay docking (as on live)
            Jumping             = 4000,    //  to delay jumping
            Moving              = 1000,
            Scanning            = 10000,   // used to delay scan results based on skills, items, and other shit
            Killed              = 800,    // used to reset ego after killed or otherwise changing ships
            Proc                = 1000,    // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
            Jetcan              = 600000,  // used to delay jetcan creation.  10min default
            Logout              = 10000,    // used to hold client object until WarpOut finishes
            Login               = 4000,    // delay before sending SetState when client logs in undocked
            Session             = 10000,   // used to prevent multiple session changes from occurring too fast
            DockInvul           = 3000,
            Fleet               = 1500,
            JumpInvul           = 15000,   // increased from 5s
            WarpOutInvul        = 5000,
            WarpInInvul         = 18000,   // increased from 10s
            UndockInvul         = 20000,
            RestoringInvul      = 60000,
            JumpCloak           = 30000,
            LoginCloak          = 20000,
            UnCloak             = 5000     // not implemented yet
        };
    }

    namespace State {
        enum {
            Idle        = 1,
            Jump        = 2,
            Dock        = 3,
            Undock      = 4,
            Killed      = 5,
            Logout      = 6,
            Board       = 7,
            Login       = 8,
            Uncloak     = 9
        };
    }
}


#endif  // EVE_PLAYER_H