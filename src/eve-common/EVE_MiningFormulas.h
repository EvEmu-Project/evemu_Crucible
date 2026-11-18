#ifndef EVEMU_MINING_FORMULAS_H
#define EVEMU_MINING_FORMULAS_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace EvEMining {

inline double DroneYield(double baseVolume, int operationLevel, int interfacingLevel) {
    return baseVolume * (1.0 + 0.05 * std::clamp(operationLevel, 0, 5))
        * (1.0 + 0.20 * std::clamp(interfacingLevel, 0, 5));
}

inline int32_t OreUnits(double volume, double unitVolume, double remainingUnits) {
    if (!std::isfinite(volume) || !std::isfinite(unitVolume) || !std::isfinite(remainingUnits)
        || volume <= 0.0 || unitVolume <= 0.0 || remainingUnits <= 0.0)
        return 0;
    double byVolume = std::floor(volume / unitVolume);
    // The inventory stores volumes as floats. Accept an exact fit when the
    // stored unit volume (for example 0.1 m³) rounds at the stack boundary.
    if (static_cast<float>((byVolume + 1.0) * unitVolume) <= static_cast<float>(volume))
        ++byVolume;
    const double units = std::min({byVolume, remainingUnits,
                                  static_cast<double>(std::numeric_limits<int32_t>::max())});
    return static_cast<int32_t>(std::floor(units));
}

} // namespace EvEMining

#endif
