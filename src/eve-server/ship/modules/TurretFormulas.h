
 /**
  * @name TurretFormulas.h
  *   forumlas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */

#ifndef _EVE_SHIP_MOD_FORMULAS_H_
#define _EVE_SHIP_MOD_FORMULAS_H_

#include "ship/Ship.h"

class NPC;
class DroneSE;
class TurretModule;
class TurretFormulas {
public:
    //  returns damage modifier from hit, based on calculations made about source, item, and target.
    //    return 0 is missed
    float GetToHit(ShipItemRef shipRef, TurretModule* pMod, SystemEntity* pTarget);
    float GetNPCToHit(NPC* pNPC, SystemEntity* pTarget);
    float GetDroneToHit(DroneSE* pDrone, SystemEntity* pTarget);
    float GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget);
};


#endif  //_EVE_SHIP_MOD_FORMULAS_H_