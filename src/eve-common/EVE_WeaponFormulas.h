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

#endif
