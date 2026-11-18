#ifndef EVE_INDUSTRY_FORMULAS_H
#define EVE_INDUSTRY_FORMULAS_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

// Shared by job quotes, availability checks, and actual material deductions.
// Round the TOTAL quantity once, avoiding differing stack consumption for
// multi-run research/manufacturing quotations.
namespace EVEmuIndustry {

inline std::uint32_t RequiredMaterialQuantity(std::uint32_t base, double materialMultiplier,
                                             double characterMultiplier, std::uint32_t runs, bool extra = false) {
    if (runs == 0 || !std::isfinite(materialMultiplier) || !std::isfinite(characterMultiplier)
        || materialMultiplier < 0.0 || characterMultiplier < 1.0)
        return 0;
    // Extra RAM requirements are fixed per run (e.g. datacores). ME and
    // character Production Efficiency must not discount them.
    const double perRun = extra ? static_cast<double>(base)
        : std::max(0.0, static_cast<double>(base) *
            (materialMultiplier + characterMultiplier - 1.0));
    const double total = std::round(perRun * runs);
    if (!std::isfinite(total) || total >= std::numeric_limits<std::uint32_t>::max())
        return std::numeric_limits<std::uint32_t>::max();
    return static_cast<std::uint32_t>(std::max(0.0, total));
}

inline bool ValidCopyRuns(std::int32_t copies, std::int32_t runsPerCopy, std::uint32_t maxRuns) {
    return copies > 0 && copies <= std::numeric_limits<std::int16_t>::max()
        && runsPerCopy > 0
        && runsPerCopy <= std::numeric_limits<std::int16_t>::max()
        && maxRuns > 0
        && static_cast<std::uint32_t>(runsPerCopy) <= maxRuns;
}

} // namespace EVEmuIndustry
#endif // EVE_INDUSTRY_FORMULAS_H
