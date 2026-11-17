#ifndef EVEMU_MOVEMENT_FORMULAS_H
#define EVEMU_MOVEMENT_FORMULAS_H

#include <cmath>

namespace EVEmuMovement {

// Separation of two spherical surfaces; never wrap negative/overlapping
// distances into an unsigned integer or pass a negative clearance to AI.
inline double SurfaceSeparation(double centerDistance, double radiusA, double radiusB) {
    if (!std::isfinite(centerDistance) || centerDistance < 0.0
        || !std::isfinite(radiusA) || !std::isfinite(radiusB))
        return 0.0;
    const double separation = centerDistance -
        ((radiusA > 0.0 ? radiusA : 0.0) + (radiusB > 0.0 ? radiusB : 0.0));
    return (separation > 0.0) ? separation : 0.0;
}

// Requested orbit distance is from the target surface, not its center.
// Keep a small clearance for discrete one-second movement calculations.
inline double MinimumOrbitRadius(double requestedRange, double selfRadius, double targetRadius) {
    if (!std::isfinite(requestedRange) || requestedRange < 0.0
        || !std::isfinite(selfRadius) || !std::isfinite(targetRadius))
        return 0.0;
    return requestedRange +
        (selfRadius > 0.0 ? selfRadius : 0.0) +
        (targetRadius > 0.0 ? targetRadius : 0.0) + 25.0;
}

// In Crucible, dogma attribute 600 is already the ship's warp speed in AU/s.
// Prefer the fitted value (which can include bonuses), then the type's
// historical base value. Use the dogma default only for missing/invalid data.
inline double ResolveCrucibleWarpSpeedAU(double fitted, double typeDefault) {
    if (std::isfinite(fitted) && fitted > 0.1)
        return fitted;
    if (std::isfinite(typeDefault) && typeDefault > 0.1)
        return typeDefault;
    return 3.0;
}

} // namespace EVEmuMovement

#endif // EVEMU_MOVEMENT_FORMULAS_H
