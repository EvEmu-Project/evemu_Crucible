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

// Known strategic types require specific sovereign days; Military and
// Industrial groups use effective development-index raw points.
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

constexpr std::uint32_t StrategicGroup = 1016U;
constexpr std::uint32_t IndustrialGroup = 1020U;
constexpr std::uint32_t MilitaryGroup = 1021U;

enum class UpgradeKind { Unsupported, Strategic, Industrial, Military };
inline UpgradeKind KindForGroup(std::uint32_t groupID)
{
    switch (groupID) {
        case StrategicGroup: return UpgradeKind::Strategic;
        case IndustrialGroup: return UpgradeKind::Industrial;
        case MilitaryGroup: return UpgradeKind::Military;
        default: return UpgradeKind::Unsupported;
    }
}

inline bool IsSupportedUpgrade(std::uint32_t typeID, std::uint32_t groupID)
{
    const UpgradeKind kind = KindForGroup(groupID);
    return kind == UpgradeKind::Military || kind == UpgradeKind::Industrial
        || (kind == UpgradeKind::Strategic
            && RequiredStrategicDays(typeID) != 0U);
}

// Full CCP-like military and industrial activity-point generation and
// decay are not yet implemented in this fork. This deliberately accepts
// an upgrade only with DEV-simulated Level5 raw points; it must never
// mistake the existing placeholder 5 raw points for displayed Level 5.
inline bool MeetsActivityRequirement(std::uint32_t effectiveRawPoints,
                                     std::uint32_t requiredLevel,
                                     std::uint32_t testLevelFivePoints)
{
    return requiredLevel >= 1U && requiredLevel <= 5U
        && effectiveRawPoints >= testLevelFivePoints;
}

} // namespace EVESovIHub
#endif
