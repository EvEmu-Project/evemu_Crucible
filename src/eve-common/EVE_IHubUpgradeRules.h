#ifndef EVEMU_IHUB_UPGRADE_RULES_H
#define EVEMU_IHUB_UPGRADE_RULES_H

#include <cstdint>

namespace EVESovIHub {

// Crucible strategic infrastructure upgrades are defined by invTypes.
// 2009 is Supercapital Construction Facilities; previously rejected by
// the default branch of Inventory::ValidateIHubUpgrade.
constexpr std::uint32_t CynosuralNavigation = 2008U;
constexpr std::uint32_t CynosuralSuppression = 2001U;
constexpr std::uint32_t SupercapitalConstructionFacilities = 2009U;
constexpr std::uint32_t AdvancedLogisticsNetwork = 32422U;

// Existing server semantics for the older navigation/suppression upgrades
// are preserved; Supercapital construction requires Strategic Level 5.
// Return zero for unsupported types rather than silently installing them.
inline std::uint32_t RequiredStrategicDays(std::uint32_t typeID)
{
    switch (typeID) {
        case CynosuralNavigation: return 2U;
        case CynosuralSuppression:
        case AdvancedLogisticsNetwork: return 3U;
        case SupercapitalConstructionFacilities: return 100U;
        default: return 0U;
    }
}

inline bool MeetsStrategicRequirement(std::uint32_t typeID,
                                     std::uint32_t effectiveDays)
{
    const std::uint32_t required = RequiredStrategicDays(typeID);
    return required != 0U && effectiveDays >= required;
}

} // namespace EVESovIHub
#endif
