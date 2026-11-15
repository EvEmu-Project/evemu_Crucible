/*
    EVEmu weapon damage helpers.
    Shared by server combat code and focused unit tests.
*/
#ifndef EVE_WEAPON_FORMULAS_H
#define EVE_WEAPON_FORMULAS_H

#include <algorithm>
#include <cmath>

inline double CalculateMissileDamageFactor(double signatureRadius,
                                           double explosionRadius,
                                           double explosionVelocity,
                                           double targetVelocity,
                                           double damageReductionFactor,
                                           double damageReductionSensitivity)
{
    if (!std::isfinite(signatureRadius) || !std::isfinite(explosionRadius)
        || !std::isfinite(explosionVelocity) || !std::isfinite(targetVelocity)
        || signatureRadius < 0.0 || explosionRadius <= 0.0)
        return 0.0;

    const double signatureFactor = signatureRadius / explosionRadius;
    if (signatureFactor <= 0.0)
        return 0.0;

    double velocityFactor = 1.0;
    if (explosionVelocity >= 0.0 && targetVelocity >= 0.0
        && damageReductionFactor > 0.0 && damageReductionSensitivity > 0.0
        && std::fabs(damageReductionSensitivity - 1.0) > 0.000001) {
        const double velocityTerm =
            (explosionVelocity / std::max(targetVelocity, 1.0)) * signatureFactor;
        if (velocityTerm > 0.0 && std::isfinite(velocityTerm)) {
            velocityFactor = std::pow(
                velocityTerm,
                std::log(damageReductionFactor) / std::log(damageReductionSensitivity));
        } else {
            velocityFactor = 0.0;
        }
    }

    if (!std::isfinite(velocityFactor))
        return 0.0;

    return std::max(0.0, std::min(1.0, std::min(signatureFactor, velocityFactor)));
}


// Crucible/legacy turret formula (signature resolution is a separate
// attribute in this client era; do not substitute the modern 40,000 constant).
inline double CalculateTurretHitChance(double angularVelocity, double trackingSpeed,
                                       double signatureResolution, double targetSignature,
                                       double distance, double optimal, double falloff)
{
    if (!std::isfinite(angularVelocity) || !std::isfinite(trackingSpeed)
        || !std::isfinite(signatureResolution) || !std::isfinite(targetSignature)
        || !std::isfinite(distance) || !std::isfinite(optimal) || !std::isfinite(falloff)
        || angularVelocity < 0.0 || trackingSpeed <= 0.0
        || signatureResolution <= 0.0 || targetSignature <= 0.0
        || distance <= 0.0 || optimal < 0.0 || falloff < 0.0)
        return 0.0;

    const double trackingTerm = (angularVelocity / trackingSpeed)
        * (signatureResolution / targetSignature);
    const double overOptimal = std::max(0.0, distance - optimal);
    if (overOptimal > 0.0 && falloff == 0.0)
        return 0.0;
    const double rangeTerm = (overOptimal > 0.0) ? (overOptimal / falloff) : 0.0;
    const double chance = std::pow(0.5, trackingTerm * trackingTerm + rangeTerm * rangeTerm);
    return std::isfinite(chance) ? std::max(0.0, std::min(1.0, chance)) : 0.0;
}

// The SAME roll determines both hit/miss and quality. Even a wrecking hit
// must first pass the hit-chance threshold. No guaranteed "minimum" hits.
inline float CalculateTurretHitQuality(double chance, double roll, double wreckChance = 0.01)
{
    if (!std::isfinite(chance) || !std::isfinite(roll)
        || chance <= 0.0 || roll < 0.0 || roll >= chance || roll > 1.0)
        return 0.0f;
    const double wreck = (std::isfinite(wreckChance))
        ? std::max(0.0, std::min(1.0, wreckChance)) : 0.01;
    return (roll < wreck) ? 3.0f : static_cast<float>(roll + 0.49);
}

// 0.0 is legitimate full immunity; 1.0 is no resistance. A missing
// attribute means "unknown" and should not silently give 99% resistance.
inline float NormalizeDamageResonance(double value, bool isPresent)
{
    if (!isPresent || !std::isfinite(value))
        return 1.0f;
    return static_cast<float>(std::max(0.0, std::min(1.0, value)));
}

 // MaxVelocity is metres/second; ExplosionDelay and flight timers are ms.
inline double CalculateMissileMaximumRange(double velocity, double lifetimeMs)
{
    if (!std::isfinite(velocity) || !std::isfinite(lifetimeMs)
        || velocity <= 0.0 || lifetimeMs <= 0.0)
        return 0.0;
    const double range = velocity * lifetimeMs / 1000.0;
    return std::isfinite(range) ? range : 0.0;
}

// A defender subtracts its warhead damage from incoming missile HP.
// Do not treat every interception as a guaranteed one-hit kill.
inline double CalculateRemainingMissileHP(double currentHP, double incomingDamage)
{
    if (!std::isfinite(currentHP) || currentHP <= 0.0)
        return 0.0;
    if (!std::isfinite(incomingDamage) || incomingDamage <= 0.0)
        return currentHP;
    return std::max(0.0, currentHP - incomingDamage);
}

#endif
