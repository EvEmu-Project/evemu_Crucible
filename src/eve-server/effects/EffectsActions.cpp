/**
* @name EffectsActions.cpp
*   This file is for decoding and processing the effect's action data
*   Copyright 2017  EVEmu Team
*
* @Author:    Allan
* @date:      27 March 2017
*
*/

/** @note  not currently used.  may not be used.  */

#include "Client.h"
#include "effects/EffectsActions.h"
#include "system/SystemEntity.h"

/*
 * # Effects Logging:
 * EFFECTS=0
 * EFFECTS__ERROR=1
 * EFFECTS__WARNING=0
 * EFFECTS__MESSAGE=0
 * EFFECTS__DEBUG=0
 * EFFECTS__TRACE=0
 */


void FxAction::DoAction(uint8 action, SystemEntity* pSE)
{
    using namespace FX;
    if (action == Action::dgmActInvalid)
        ; //make error and return

    switch (action) {
        case dgmATTACK: { // 13,
        } break;
        case dgmCHEATTELEDOCK: { // 15,
        } break;
        case dgmCHEATTELEGATE: { // 16,
        } break;
        case dgmDECLOAKWAVE: { // 19,
        } break;
        case dgmECMBURST: { //    30,
        } break;
        case dgmEMPWAVE: { //     32,
        } break;
        case dgmLAUNCH: { //      44,
        } break;
        case dgmLAUNCHDEFENDERMISSILE: { // 45,
        } break;
        case dgmLAUNCHDRONE: { // 46,
        } break;
        case dgmLAUNCHFOFMISSILE: { // 47,
        } break;
        case dgmPOWERBOOST: { //  53,   //effectID 48  - Consumes power booster charges to increase the available power in the capacitor.
        } break;
        case dgmTARGETHOSTILES: { // 70,
        } break;
        case dgmTARGETSILENTLY: { // 71,
        } break;
        case dgmTOOLTARGETSKILLS: { // 72,
        } break;
        case dgmVERIFYTARGETGROUP: { // 74,
        } break;
        /* unique/special to EVEmu */
        case dgmSPEEDBOOST: { //  75    //effectID 14  - prop mod to call destiny speed updates
            if ((pSE != nullptr) and (pSE->DestinyMgr())) {
                pSE->DestinyMgr()->SpeedBoost();
            } else {
                _log(EFFECTS__ERROR, "FxAction::DoAction(): dgmSPEEDBOOST called with invalid pSE or DestinyMgr");
            }
        } break;
        /* these below are special as they are called AFTER the module's cycle */
        case dgmCARGOSCAN: { //   14,
        } break;
        case dgmMINE: { //        50,
        } break;
        case dgmSHIPSCAN: { //    66,
        } break;
        case dgmSURVEYSCAN: { //  69,
        } break;
    }

}
