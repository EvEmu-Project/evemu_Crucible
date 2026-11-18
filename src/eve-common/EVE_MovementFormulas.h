#ifndef EVEMU_MOVEMENT_FORMULAS_H
#define EVEMU_MOVEMENT_FORMULAS_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

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

// The server seed stores attribute 600 in AU/s. The archived Crucible
// export stores a multiplier of the 3 AU/s client baseline.
inline double CrucibleWarpSpeedAU(double fittedSpeed, double typeSpeed) {
    if (std::isfinite(fittedSpeed) && fittedSpeed > 0.0)
        return fittedSpeed;
    if (std::isfinite(typeSpeed) && typeSpeed > 0.0)
        return typeSpeed;
    return 3.0;
}

// Destiny packets carry whole tenths. Keep their rounding at the wire boundary.
inline std::int32_t WarpSpeedTenths(double speedAU) {
    return static_cast<std::int32_t>(std::clamp(std::round(speedAU * 10.0), 1.0,
        static_cast<double>(std::numeric_limits<std::int32_t>::max())));
}

} // namespace EVEmuMovement

#endif // EVEMU_MOVEMENT_FORMULAS_H
