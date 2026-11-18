#ifndef EVEMU_MOVEMENT_FORMULAS_H
#define EVEMU_MOVEMENT_FORMULAS_H

#include <algorithm>
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

// Destiny advances once per tick. Tractor targets stop at the requested
// surface clearance even when a full-speed step would travel farther.
inline double TractorTravelStep(double separation, double stopRange, double speed) {
    if (!std::isfinite(separation) || !std::isfinite(stopRange)
        || !std::isfinite(speed) || speed <= 0.0)
        return 0.0;
    return std::min(speed, std::max(0.0, separation - stopRange));
}

// Warp acceleration reports total distance traveled from entry, not the
// increment to deduct from the previous tick.
inline double WarpRemainingAfterAcceleration(double total, double traveled) {
    return std::max(0.0, total - traveled);
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

// The Crucible SDE stores the fitted warp top speed in attribute 600.
// Raw hull examples: battleships 3 AU/s, frigates 6 AU/s, interceptors 13.5 AU/s.
// Do not multiply the live fitted value by the 3 AU/s fallback again.
inline double CrucibleWarpSpeedAU(double fittedSpeed, double typeSpeed) {
    if (std::isfinite(fittedSpeed) && fittedSpeed > 0.0)
        return fittedSpeed;
    if (std::isfinite(typeSpeed) && typeSpeed > 0.0)
        return typeSpeed;
    return 3.0;
}

} // namespace EVEmuMovement

#endif // EVEMU_MOVEMENT_FORMULAS_H
