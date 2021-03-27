
 /**
  * @name TurretFormulas.cpp
  *   formulas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */

 /* default crit chances - can change in server config
  *      NPC  - 1.5%
  *   Player  - 2%
  *   Sentry  - 2%
  *    Drone  - 3%
  *  Concord  - 5%
  */

#include "character/Character.h" // this sets compat includes for this file
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "npc/Sentry.h"
#include "ship/modules/TurretFormulas.h"
#include "ship/modules/TurretModule.h"


float TurretFormulas::GetToHit(ShipItemRef shipRef, TurretModule* pMod, SystemEntity* pTarget)
{
    if (pTarget == nullptr)
        return 0;
    uint32 falloff = pMod->GetAttribute(AttrFalloff).get_uint32();
    uint32 range = pMod->GetAttribute(AttrMaxRange).get_uint32();
    float distance = shipRef->position().distance(pTarget->DestinyMgr()->GetPosition());
    _log(DAMAGE__TRACE, "Turret::GetToHit - distance:%.2f, range:%.2f, falloff:%u", distance, range, falloff);

    // calculate transversal from other data
    /* i have had problems finding exact data for transversal velocity
     * ideas/data taken from https://wiki.eveuniversity.org/Velocity
     * The transversal velocity is computed by subtracting the two velocity vectors from one another, and then finding the length of the vector.
     * angular velocity = transversal velocity / distance
     */
    GVector vector = pTarget->GetVelocity() - shipRef->GetPilot()->GetShipSE()->GetVelocity();
    float transversalV = vector.length();
    float angularVel = transversalV / distance;
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    float sigRes = pMod->GetAttribute(AttrOptimalSigRadius).get_float();
    float trackSpeed = pMod->GetAttribute(AttrTrackingSpeed).get_float();
    _log(DAMAGE__TRACE, "Turret::GetToHit - transversalV:%.3f, angularV:%.3f, tracking:%.3f, targetSig:%.1f, sigRes:%.1f", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);
    //  calculations for chance to hit  --UD 29May17
    /*ChanceToHit = 0.5 ^ ((((Transversal speed/(Range to target * Turret Tracking))*(Turret Signature Resolution / Target Signature Radius))^2)
     *                  + ((max(0, Range To Target - Turret Optimal Range))/Turret Falloff)^2)
     *
     *     a =  Transversal speed/(Range to target * Turret Tracking)
     *     b =  Turret Signature Resolution / Target Signature Radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     * tohit =  0.5 ^ (c + e)
     */
    float a = (angularVel / trackSpeed);
    float b = (sigRes / targSig);
    float modifier = 0.0f;
    if ((a < 1) and (b > 1)) {
        /* in cases where weapon can track target, but sigRes > targSig, the weapon would not hit on live but *should* hit with reduced damage
         * modify formula to remove Signature variable from equation, test toHit against tracking,
         * then use Signature variables to determine amount of damage reduction (i.e. large gun vs. small ship)
         */
        b = 1;
        modifier = (targSig / sigRes);
    }
    float c = pow((a * b), 2);
    float d = EvE::max(distance - range);
    float e = pow((d / falloff), 2);
    float x = pow(0.5, c);
    float y = pow(0.5, e);
    float ChanceToHit = x * y;
    _log(DAMAGE__TRACE, "Turret::GetToHit - (%.3f * %.3f)^2 = c:%.5f : (%.3f / %u)^2 = e:%.5f", a, b, c, d, falloff, e);
    float rNum = MakeRandomFloat();
    _log(DAMAGE__TRACE, "Turret::GetToHit - %f * %f = %.5f  - Rand:%.3f  - %s", \
            x, y, ChanceToHit, rNum, ((rNum <= sConfig.rates.PlayerCritChance) ? "Crit" : (rNum < ChanceToHit ? "Hit" : "Miss")));
    if (rNum <= sConfig.rates.PlayerCritChance)
        return 3.0f;
    if (rNum < ChanceToHit) {
        if (modifier)
            return modifier;
        return (rNum + 0.49);
    }
    return 0;
}

float TurretFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    if (pTarget == nullptr)
        return 0;
    uint16 sigRes = pNPC->GetAIMgr()->GetSigRes();
    uint16 range = pNPC->GetAIMgr()->GetOptimalRange();
    uint32 falloff = pNPC->GetAIMgr()->GetFalloff();
    float distance = pNPC->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    float trackSpeed = pNPC->GetAIMgr()->GetTrackingSpeed();
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - distance:%.2f, range:%.u, falloff:%u", distance, range, falloff);

    GVector vector = pTarget->GetVelocity() - pNPC->GetVelocity();
    float transversalV = vector.length();
    float angularVel = transversalV / distance;
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - transversalV:%.3f, angularVel:%.3f tracking:%.3f, targetSig:%.1f, sigRes:%u", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);

    float a = (angularVel / trackSpeed);
    float b = (sigRes / targSig);
    float modifier = 0.0f;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    float c = pow((a * b), 2);
    float d = EvE::max(distance - range);
    float e = pow((d / falloff), 2);
    float x = pow(0.5, c);
    float y = pow(0.5, e);
    float ChanceToHit = x * y;
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - (%.3f * %.3f)^2 = c:%.5f : (%.3f / %u)^2 = e:%.5f", a, b, c, d, falloff, e);
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - %f * %f = %.5f", x, y, ChanceToHit);
    float rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - ChanceToHit:%f, Rand:%.3f - %s", ChanceToHit, rNum, ((rNum <= sConfig.rates.NpcCritChance) ? "Crit" : (rNum < ChanceToHit ? "Hit" : "Miss")));
    if (rNum <= sConfig.rates.NpcCritChance)
        return 3.0f;
    if (rNum < ChanceToHit) {
        if (modifier)
            return modifier;
        return (rNum + 0.49);
    }
    return 0;
}

float TurretFormulas::GetDroneToHit(DroneSE* pDrone, SystemEntity* pTarget)
{
    if (pTarget == nullptr)
        return 0;
    float falloff = pDrone->GetSelf()->GetAttribute(AttrFalloff).get_float();
    float distance = pDrone->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    GVector vector = pTarget->GetVelocity() - pDrone->GetVelocity();
    float transversalV = vector.length();
    float a = (transversalV / (distance * pDrone->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float()));
    float b = (pDrone->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    float c = pow((a * b), 2);
    float d = EvE::max(distance - pDrone->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float());
    float e = pow((d / falloff), 2);
    float ChanceToHit = pow(0.5, c + e);
    float rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= sConfig.rates.DroneCritChance)
        return 3.0f;
    if (rNum < ChanceToHit)
        return (rNum + 0.49);
    // drones will have a minimum damage instead of zero
    return 0.1;
}

float TurretFormulas::GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget)
{
    if (pTarget == nullptr)
        return 0;
    float sigRes = pSentry->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float();
    float falloff = pSentry->GetSelf()->GetAttribute(AttrFalloff).get_float();
    float distance = pSentry->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    float a = (pTarget->GetVelocity().length() / (distance * pSentry->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float()));
    float b = (sigRes / targSig);
    float modifier = 0.0f;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    float c = pow((a * b), 2);
    float d = EvE::max(distance - pSentry->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float());
    float e = pow((d / falloff), 2);
    float ChanceToHit = pow(0.5, c + e);
    float rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= sConfig.rates.SentryCritChance)
        return 3.0f;
    if (rNum < ChanceToHit) {
        if (modifier)
            return modifier;
        return (rNum + 0.49);
    }
    return 0;
}
