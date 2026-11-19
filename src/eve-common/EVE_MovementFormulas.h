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

// A gate jump is permitted within 2,500m of the combined ship/gate
// collision surfaces. Target 2,400m so rounding never requires a second
// approach tick just to cross the exact 2,500m boundary.
constexpr double GateWarpLandingSurfaceRange = 2400.0;
constexpr double GateJumpSurfaceRange = 2500.0;
constexpr double DockSurfaceRange = 2500.0;

// Movement can clamp a malformed separation for its own calculations.
// A dock or jump range check must fail closed on invalid geometry.
inline double InteractionSurfaceGap(double centerDistance, double shipRadius, double targetRadius) {
    if (!std::isfinite(centerDistance) || !std::isfinite(shipRadius) || !std::isfinite(targetRadius)
        || centerDistance < 0.0 || shipRadius < 0.0 || targetRadius < 0.0)
        return std::numeric_limits<double>::infinity();
    return SurfaceSeparation(centerDistance, shipRadius, targetRadius);
}

// At the configured exit speed the ship has reached the end of the warp
// path. Remove the exponential tail so there is no final position jump.
inline double WarpDecelerationRemaining(double initialDistance, double decay,
                                        double peakSpeed, double exitSpeed) {
    if (initialDistance <= 0.0 || !(peakSpeed > exitSpeed))
        return 0.0;
    const double exitFraction = exitSpeed / peakSpeed;
    return initialDistance * std::clamp((decay - exitFraction) / (1.0 - exitFraction), 0.0, 1.0);
}

// Find the first intersection with the approach sphere along the ship's
// actual heading. If this tick misses, preserve the full movement step.
inline double ApproachTravelStep(double centerDistance, double remaining,
                                 double headingCosine, double speed) {
    if (remaining <= 0.0)
        return 0.0;
    if (remaining > speed || headingCosine <= 0.0)
        return speed;
    const double cosine = std::min(1.0, headingCosine);
    const double stopRadius = centerDistance - remaining;
    const double perpendicularSquared = centerDistance * centerDistance * (1.0 - cosine * cosine);
    const double discriminant = stopRadius * stopRadius - perpendicularSquared;
    if (discriminant < 0.0)
        return speed;
    const double intersection = remaining * (centerDistance + stopRadius)
        / (centerDistance * cosine + std::sqrt(discriminant));
    return std::min(speed, intersection);
}

inline float DegreesPerMovementTick(float agility) {
    // The legacy equation reaches zero for freighters and can be negative
    // for a capital with increased mass. Keep those hulls able to turn.
    return std::max(1.0f, (60.0f - agility) / 10.0f);
}

// WarpTo/CmdWarpTo use a destination center and a *center-based* stop range,
// whereas the client-requested minRange and gate activation distance refer to
// the gap between the ship's and target's surfaces. Add BOTH radii exactly
// once. An integral stop range is rounded up to avoid hull overlap.
inline std::int32_t WarpCenterStopRange(double requestedSurfaceRange,
                                        double shipRadius, double targetRadius,
                                        double minimumSurfaceRange = 0.0) {
    if (!std::isfinite(requestedSurfaceRange) || !std::isfinite(shipRadius)
        || !std::isfinite(targetRadius) || !std::isfinite(minimumSurfaceRange))
        return std::numeric_limits<std::int32_t>::max();
    const double surface = std::max({0.0, requestedSurfaceRange, minimumSurfaceRange});
    const double center = surface + std::max(0.0, shipRadius) + std::max(0.0, targetRadius);
    const double maxRange = static_cast<double>(std::numeric_limits<std::int32_t>::max());
    return static_cast<std::int32_t>(std::ceil(std::min(center, maxRange)));
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

// Clamp the legacy orbit equation to both hulls' actual surface clearance.
// Return zero only if no valid orbit fits within the current bubble.
inline double ResolvedOrbitRadius(double minimumRadius, double predictedRadius,
                                  double maximumRadius) {
    if (!std::isfinite(minimumRadius) || minimumRadius <= 0.0
        || !std::isfinite(maximumRadius) || maximumRadius <= 0.0)
        return 0.0;
    const double clearance = std::ceil(minimumRadius);
    if (clearance > maximumRadius)
        return 0.0;
    if (!std::isfinite(predictedRadius) || predictedRadius < clearance
        || predictedRadius > maximumRadius)
        return clearance;
    return std::max(clearance, std::floor(predictedRadius));
}

// Some archived NPCs have no entityCruiseSpeed. Use actual maxVelocity,
// not an invented universal speed; truly stationary NPCs remain stationary.
inline double NpcCruiseSpeed(double cruiseSpeed, double maxVelocity) {
    if (std::isfinite(cruiseSpeed) && cruiseSpeed > 0.0)
        return cruiseSpeed;
    if (std::isfinite(maxVelocity) && maxVelocity > 0.0)
        return maxVelocity;
    return 0.0;
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
