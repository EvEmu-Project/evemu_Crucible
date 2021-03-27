/*
    ------------------------------------------------------------------------------------
    This file is for decoding the module states for EVEmu
    Copyright 2017  Alasiya-EVEmu Team
    ------------------------------------------------------------------------------------
    Author:     Allan
    Based on original idea and code from Aknor Jaden and Luck
*/

#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H


namespace Module {
    namespace State {
        enum {
            // internal module states
            // to be used in conjunction with effectState (when to apply effect)
            /* 'Online' is used for:
             * ACTIVE modules fitted and online, but not activated (using the PASSIVE effects only where applicable)
             * PASSSIVE modules fitted and online
             * RIG modules fitted (always online when fit)
             */
            Unfitted         = 0,
            Offline          = 1,    // module fitted, but NOT put online yet - NOT used for rigs
            Online           = 2,    // module online  - rigs are either online or unfitted.
            Deactivating     = 3,    // module transitioning from Activated to Online (deactivate module - waiting for timer to run out)
            Activated        = 4,    // used only for activated ACTIVE modules (Overloaded mode is calculated separately)

            // internal charge states
            Unloaded         = 5,
            Loaded           = 6,
            Loading          = 7,
            Reloading        = 8
        };
    }

    namespace Bank {
        enum {
            Undefined        = 0,
            Low              = 1,
            Mid              = 2,
            High             = 3,
            Rig              = 4,
            Subsystem        = 5
        };
    }

    namespace Size {
        enum {
            Undefined        = 0,
            Small            = 1,
            Medium           = 2,
            Large            = 3,
            Capitol          = 4
        };
    }
}

#endif
