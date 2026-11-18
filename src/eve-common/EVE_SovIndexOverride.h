#ifndef EVEMU_SOV_INDEX_OVERRIDE_H
#define EVEMU_SOV_INDEX_OVERRIDE_H

#include <cstdint>

namespace EVESovIndex {

// DEV / ADMIN testing overrides are NOT the real claim age and must never
// be applied after the ownership claim (TCU itemID) has changed.
struct Override {
    std::uint32_t claimStructureID;
    std::uint16_t militaryPoints;
    std::uint16_t industrialPoints;
    std::uint16_t strategicDays;
};

inline bool IsActive(std::uint32_t currentClaimID, const Override& test)
{
    return currentClaimID != 0 && currentClaimID == test.claimStructureID;
}

inline std::uint32_t EffectivePoints(std::uint32_t livePoints,
                                     const Override* test,
                                     std::uint32_t claimID, bool military)
{
    return test != nullptr && IsActive(claimID, *test)
        ? (military ? test->militaryPoints : test->industrialPoints)
        : livePoints;
}

inline std::uint32_t EffectiveStrategicDays(std::uint32_t actualDays,
                                            const Override* test,
                                            std::uint32_t claimID)
{
    return test != nullptr && IsActive(claimID, *test)
        && test->strategicDays > actualDays ? test->strategicDays : actualDays;
}

} // namespace EVESovIndex
#endif // EVEMU_SOV_INDEX_OVERRIDE_H
