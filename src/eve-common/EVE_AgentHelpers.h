#ifndef EVEMU_AGENT_HELPERS_H
#define EVEMU_AGENT_HELPERS_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <string>

namespace EvEAgent {

// EVE FILETIME uses 10,000,000 ticks/second. Mission bonusTime is minutes,
// and its deadline is counted from acceptance, not the initial offer date.
inline int64_t BonusTimeRemaining(double acceptedAt, uint16_t bonusMinutes, double now) {
    const int64_t duration = static_cast<int64_t>(bonusMinutes) * 60 * 10000000;
    if (duration <= 0 || !std::isfinite(acceptedAt) || !std::isfinite(now))
        return 0;
    if (acceptedAt <= 0.0)
        return duration;
    const double remaining = static_cast<double>(duration) - (now - acceptedAt);
    if (!std::isfinite(remaining) || remaining <= 0.0)
        return 0;
    return static_cast<int64_t>(std::min(remaining, static_cast<double>(duration)));
}

inline bool BonusWithinDeadline(double acceptedAt, uint16_t bonusMinutes,
                                double completedAt, uint32_t rewardISK) {
    return rewardISK > 0 && bonusMinutes > 0 && acceptedAt > 0.0
        && std::isfinite(acceptedAt) && std::isfinite(completedAt)
        && completedAt >= acceptedAt
        && BonusTimeRemaining(acceptedAt, bonusMinutes, completedAt) > 0;
}

inline uint32_t SupportedPortraitSize(uint32_t size) {
    // The current image CDN supports powers-of-two sizes; Crucible also asks
    // for 40px and 50px agent portraits.
    for (uint32_t supported : {32u, 64u, 128u, 256u, 512u, 1024u})
        if (size <= supported)
            return supported;
    return 1024;
}

inline std::string ModernImageUrl(const std::string& category,
                                  uint32_t id, uint32_t size) {
    if (id == 0 || size == 0)
        return std::string();
    const char* prefix = nullptr;
    const char* variation = nullptr;
    if (category == "Character") {
        prefix = "characters"; variation = "portrait";
    } else if (category == "Corporation") {
        prefix = "corporations"; variation = "logo";
    } else if (category == "Alliance") {
        prefix = "alliances"; variation = "logo";
    } else if (category == "InventoryType") {
        prefix = "types"; variation = "icon";
    } else if (category == "Render") {
        prefix = "types"; variation = "render";
    } else {
        return std::string();
    }
    std::ostringstream url;
    url << "https://images.evetech.net/" << prefix << "/" << id
        << "/" << variation << "?size=" << SupportedPortraitSize(size);
    return url.str();
}

} // namespace EvEAgent
#endif
