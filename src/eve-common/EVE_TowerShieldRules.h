#ifndef EVEMU_TOWER_SHIELD_RULES_H
#define EVEMU_TOWER_SHIELD_RULES_H

#include <cmath>
#include <cstdint>

namespace EVEPOS {
namespace ShieldRules {

// Visuals depend on tower online state, never on a configured password.
inline bool ShouldDisplay(int state, int onlineState) {
    return state >= onlineState;
}

// The normal EVE type attribute is preferred. Some older dumps lack it,
// leaving FieldSE with radius=0, invisible to the client. Use the canonical
// small/medium/large fallback only in that case.
inline double EffectiveRadius(int size, double configuredRadius) {
    if (std::isfinite(configuredRadius) && configuredRadius > 0.0)
        return configuredRadius;
    switch (size) {
        case 3: return 30000.0;
        case 2: return 22500.0;
        default: return 15000.0;
    }
}

} // namespace ShieldRules
} // namespace EVEPOS

#endif
