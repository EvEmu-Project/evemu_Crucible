#ifndef EVEMU_DRONE_CONTROL_RANGE_H
#define EVEMU_DRONE_CONTROL_RANGE_H

#include <algorithm>
#include <cmath>

namespace EvEDrone {

// Crucible: base 20km; Scout Drone Operation +5km per level;
// Electronic Warfare Drone Interfacing +3km per level.
// Existing stronger computed attributes take precedence to avoid double-count.
inline double ControlRange(int scoutLevel, int ewarLevel,
                           double shipRange = 0.0, double characterRange = 0.0,
                           double shipBonus = 0.0)
{
    const int scout = std::max(0, std::min(5, scoutLevel));
    const int ewar = std::max(0, std::min(5, ewarLevel));
    const double skillRange = 20000.0 + 5000.0 * scout + 3000.0 * ewar;
    const double bonus = (std::isfinite(shipBonus) && shipBonus > 0.0) ? shipBonus : 0.0;
    const double fitted = (std::isfinite(shipRange) && shipRange > 0.0) ? shipRange : 0.0;
    const double character = (std::isfinite(characterRange) && characterRange > 0.0) ? characterRange : 0.0;
    return std::max(skillRange + bonus, std::max(fitted, character));
}

} // namespace EvEDrone

#endif
