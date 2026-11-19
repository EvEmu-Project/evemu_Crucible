#ifndef EVEMU_MINING_FLIGHT_H
#define EVEMU_MINING_FLIGHT_H

#include "math/gpoint.h"

#include <algorithm>
#include <cmath>

namespace EvEMining {

inline double MovementMass(double typeMass) {
    // The Crucible mining drone rows have zero mass. Use the mass of a
    // light scout drone for server agility and client movement prediction.
    return std::isfinite(typeMass) && typeMass > 0.0 ? typeMass : 3000.0;
}

struct FlightStep {
    GPoint position;
    bool inOrbit;
};

// Move at most one tick's travel toward a stationary or moving target.
inline GPoint Approach(const GPoint& position, const GPoint& center, double stopRadius, double speed) {
    const GVector inward(position, center);
    const double distance = inward.length();
    if (!std::isfinite(distance) || !std::isfinite(stopRadius) || stopRadius < 0.0
        || !std::isfinite(speed) || speed <= 0.0 || distance <= stopRadius)
        return position;

    return position + inward * (std::min(speed, distance - stopRadius) / distance);
}

inline FlightStep Orbit(const GPoint& position, const GPoint& center, double radius, double speed) {
    GVector radial(center, position);
    const double distance = radial.length();
    if (!std::isfinite(distance) || !std::isfinite(radius) || radius <= 0.0
        || !std::isfinite(speed) || speed <= 0.0)
        return {position, false};

    if (distance > 0.0)
        radial /= distance;
    else
        radial = GVector(1.0, 0.0, 0.0);

    const double correction = radius - distance;
    if (std::abs(correction) > 0.01) {
        const double step = std::clamp(correction, -speed, speed);
        return {center + radial * (distance + step), step == correction};
    }

    // Follow a tangent on the local sphere. The perpendicular fallback
    // handles a drone that approaches from directly above or below.
    const GVector axis = std::abs(radial.y) < 0.99
        ? GVector(0.0, 1.0, 0.0) : GVector(1.0, 0.0, 0.0);
    GVector tangent = axis.crossProduct(radial);
    tangent /= tangent.length();
    const double angle = std::min(0.5, speed / radius);
    return {center + (radial * std::cos(angle) + tangent * std::sin(angle)) * radius, true};
}

} // namespace EvEMining

#endif
