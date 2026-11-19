
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

#include "EVE_WeaponFormulas.h"
#include "character/Character.h" // this sets compat includes for this file
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "npc/Sentry.h"
#include "ship/modules/TurretFormulas.h"
#include "ship/modules/TurretModule.h"


float TurretFormulas::GetToHit(ShipItemRef shipRef, TurretModule* pMod, SystemEntity* pTarget)
{
    // Turret targets and their owning ship can be removed during a session change.
    // Validate every object used below before dereferencing it.
    if (shipRef.get() == nullptr || pMod == nullptr || pTarget == nullptr)
        return 0;

    Client* pilot = shipRef->GetPilot();
    ShipSE* sourceSE = (pilot != nullptr) ? pilot->GetShipSE() : nullptr;
    DestinyManager* targetDestiny = pTarget->DestinyMgr();
    InventoryItemRef targetSelf = pTarget->GetSelf();
    if (sourceSE == nullptr || targetDestiny == nullptr || targetSelf.get() == nullptr) {
        _log(DAMAGE__ERROR, "TurretFormulas::GetToHit() - invalid source or target state; treating shot as a miss.");
        return 0;
    }

    // Use effective fitted Crucible optimal/falloff at full precision and
    // live source position. Falloff is NOT a hard cutoff at optimal range.
    const double falloff = pMod->GetAttribute(AttrFalloff).get_double();
    const double range = pMod->GetAttribute(AttrMaxRange).get_double();
    const double distance = sourceSE->GetPosition().distance(targetDestiny->GetPosition());
    if (!std::isfinite(distance) || distance <= 0.001)
        return 0;
    _log(DAMAGE__TRACE, "Turret::GetToHit - distance:%.2f, optimal:%.2f, falloff:%.2f",
         distance, range, falloff);

    // Turret tracking uses angular velocity, not total relative speed.
    // Remove the radial component by taking the cross product of relative
    // velocity with the normalized line-of-sight vector. Motion directly
    // toward/away from the shooter must not reduce tracking chance.
    GVector lineOfSight(sourceSE->GetPosition(), targetDestiny->GetPosition());
    lineOfSight.normalize();
    GVector relativeVelocity = pTarget->GetVelocity() - sourceSE->GetVelocity();
    float transversalV = relativeVelocity.crossProduct(lineOfSight).length();
    float angularVel = transversalV / distance;
    float targSig = targetSelf->GetAttribute(AttrSignatureRadius).get_float();
    if (targSig < 0.01f)
        targSig = targetSelf->GetAttribute(AttrRadius).get_uint32() / 10.0f;
    float sigRes = pMod->GetAttribute(AttrOptimalSigRadius).get_float();
    float trackSpeed = pMod->GetAttribute(AttrTrackingSpeed).get_float();
    if (targSig <= 0.0f || sigRes <= 0.0f || trackSpeed <= 0.0f)
        return 0;
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
    const double chance = CalculateTurretHitChance(
        angularVel, trackSpeed, sigRes, targSig, distance, range, falloff);
    const float result = CalculateTurretHitQuality(
        chance, MakeRandomFloat(0.0f, 1.0f), sConfig.rates.PlayerCritChance);
    _log(DAMAGE__TRACE, "Turret: chance %.5f, quality %.3f", chance, result);
    return result;
}

float TurretFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    if (pNPC == nullptr || pNPC->GetAIMgr() == nullptr
        || pNPC->DestinyMgr() == nullptr || pTarget == nullptr
        || pTarget->DestinyMgr() == nullptr || pTarget->GetSelf().get() == nullptr)
        return 0;

    uint16 sigRes = pNPC->GetAIMgr()->GetSigRes();
    uint32 range = pNPC->GetAIMgr()->GetOptimalRange();
    uint32 falloff = pNPC->GetAIMgr()->GetFalloff();

    float distance = pNPC->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    if (distance <= 0.001f)
        return 0;
    float trackSpeed = pNPC->GetAIMgr()->GetTrackingSpeed();
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    if (targSig < 0.01f)
        targSig = pTarget->GetSelf()->GetAttribute(AttrRadius).get_uint32() / 10.0f;
    if (targSig <= 0.0f || sigRes == 0 || trackSpeed <= 0.0f)
        return 0;
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - distance:%.2f, range:%u, falloff:%u", distance, range, falloff);

    GVector lineOfSight(pNPC->DestinyMgr()->GetPosition(), pTarget->DestinyMgr()->GetPosition());
    lineOfSight.normalize();
    GVector relativeVelocity = pTarget->GetVelocity() - pNPC->GetVelocity();
    float transversalV = relativeVelocity.crossProduct(lineOfSight).length();
    float angularVel = transversalV / distance;
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - transversalV:%.3f, angularVel:%.3f tracking:%.3f, targetSig:%.1f, sigRes:%u", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);

    const double chance = CalculateTurretHitChance(
        angularVel, trackSpeed, sigRes, targSig, distance, range, falloff);
    return CalculateTurretHitQuality(
        chance, MakeRandomFloat(0.0f, 1.0f), sConfig.rates.NpcCritChance);
}

float TurretFormulas::GetDroneToHit(DroneSE* pDrone, SystemEntity* pTarget)
{
    if (pDrone == nullptr || pTarget == nullptr
        || pDrone->GetSelf().get() == nullptr || pTarget->GetSelf().get() == nullptr
        || pDrone->DestinyMgr() == nullptr || pTarget->DestinyMgr() == nullptr)
        return 0.0f;

    InventoryItemRef drone = pDrone->GetSelf();
    InventoryItemRef target = pTarget->GetSelf();
    const float distance = static_cast<float>(
        pDrone->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition()));
    if (distance <= 0.001f)
        return 0.0f;

    float tracking = drone->GetAttribute(AttrTrackingSpeed).get_float();
    if (tracking <= 0.0f)
        tracking = 1.0f;

    float targetSignature = target->GetAttribute(AttrSignatureRadius).get_float();
    if (targetSignature <= 0.0f)
        targetSignature = target->GetAttribute(AttrRadius).get_float();
    if (targetSignature <= 0.0f)
        targetSignature = 1.0f;

    float signatureResolution = drone->GetAttribute(AttrOptimalSigRadius).get_float();
    if (signatureResolution <= 0.0f)
        signatureResolution = targetSignature;

    // Turret tracking is based on transversal velocity, not total relative
    // velocity. Radial motion toward/away from the drone must not penalize hit chance.
    GVector lineOfSight(pDrone->GetPosition(), pTarget->GetPosition());
    lineOfSight.normalize();
    const GVector relativeVelocity = pTarget->GetVelocity() - pDrone->GetVelocity();
    const float transversalVelocity = relativeVelocity.crossProduct(lineOfSight).length();
    const float angularVelocity = transversalVelocity / distance;
    float optimalRange = drone->GetAttribute(AttrEntityAttackRange).get_float();
    if (optimalRange <= 0.0f)
        optimalRange = drone->GetAttribute(AttrMaxRange).get_float();
    float falloff = drone->GetAttribute(AttrFalloff).get_float();
    if (falloff <= 0.0f)
        falloff = (optimalRange > 0.0f) ? optimalRange : 1.0f;

    const double chance = CalculateTurretHitChance(
        angularVelocity, tracking, signatureResolution, targetSignature,
        distance, optimalRange, falloff);
    return CalculateTurretHitQuality(
        chance, MakeRandomFloat(0.0f, 1.0f), sConfig.rates.DroneCritChance);
}

float TurretFormulas::GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget)
{
    if (pSentry == nullptr || pTarget == nullptr
        || pSentry->GetSelf().get() == nullptr || pTarget->GetSelf().get() == nullptr
        || pTarget->DestinyMgr() == nullptr)
        return 0.0f;

    InventoryItemRef sentry = pSentry->GetSelf();
    InventoryItemRef target = pTarget->GetSelf();
    const float distance = static_cast<float>(
        pSentry->GetPosition().distance(pTarget->DestinyMgr()->GetPosition()));
    if (distance <= 0.001f)
        return 0.0f;

    const float signatureResolution = sentry->GetAttribute(AttrOptimalSigRadius).get_float();
    const float tracking = sentry->GetAttribute(AttrTrackingSpeed).get_float();
    float targetSignature = target->GetAttribute(AttrSignatureRadius).get_float();
    if (targetSignature < 0.01f)
        targetSignature = target->GetAttribute(AttrRadius).get_float() / 10.0f;
    if (signatureResolution <= 0.0f || tracking <= 0.0f || targetSignature <= 0.0f)
        return 0.0f;

    // Use transversal velocity (angular velocity), not total target speed.
    // Radial movement toward or away from the sentry must not penalize tracking.
    GVector lineOfSight(pSentry->GetPosition(), pTarget->GetPosition());
    lineOfSight.normalize();
    const GVector relativeVelocity = pTarget->GetVelocity() - pSentry->GetVelocity();
    const float transversalVelocity = relativeVelocity.crossProduct(lineOfSight).length();
    const float angularVelocity = transversalVelocity / distance;
    const float optimalRange = sentry->GetAttribute(AttrEntityAttackRange).get_float();
    const float falloff = sentry->GetAttribute(AttrFalloff).get_float();
    const double chance = CalculateTurretHitChance(
        angularVelocity, tracking, signatureResolution, targetSignature,
        distance, optimalRange, falloff);
    return CalculateTurretHitQuality(
        chance, MakeRandomFloat(0.0f, 1.0f), sConfig.rates.SentryCritChance);
}
