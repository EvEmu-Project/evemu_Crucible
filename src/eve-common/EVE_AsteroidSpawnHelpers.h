/*
 * EVEmu Crucible: deterministic, dependency-free asteroid weight selection.
 * Probability weights come from roidDistribution (systemSec, roidID, percent).
 * SQL data may store fractions, percentages or rounded weights; do not assume
 * they sum exactly to 1.0 and never select a zero or invalid typeID.
 */
#ifndef EVE_ASTEROID_SPAWN_HELPERS_H
#define EVE_ASTEROID_SPAWN_HELPERS_H

#include <cmath>
#include <cstdint>

namespace EvEAsteroidSpawns {

template<typename Distribution>
inline uint32_t SelectWeightedType(double roll, const Distribution& roids)
{
    if (!std::isfinite(roll))
        return 0;
    double total = 0.0;
    uint32_t lastTypeID = 0;
    for (const auto& entry : roids) {
        if (entry.second && std::isfinite(entry.first) && entry.first > 0.0f) {
            total += entry.first;
            lastTypeID = entry.second;
        }
    }
    if (!(total > 0.0) || !std::isfinite(total))
        return 0;

    if (roll < 0.0)
        roll = 0.0;
    else if (roll > 1.0)
        roll = 1.0;

    const double threshold = roll * total;
    double accumulated = 0.0;
    for (const auto& entry : roids) {
        if (!entry.second || !std::isfinite(entry.first) || entry.first <= 0.0f)
            continue;
        accumulated += entry.first;
        if (threshold < accumulated)
            return entry.second;
    }
    // A random roll of exactly 1.0 can occur (rand() == RAND_MAX).
    return lastTypeID;
}

} // namespace EvEAsteroidSpawns

#endif // EVE_ASTEROID_SPAWN_HELPERS_H
