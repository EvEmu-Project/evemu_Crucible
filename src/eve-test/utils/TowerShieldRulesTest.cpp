#include "eve-test.h"
#include "EVE_TowerShieldRules.h"
#include <cstdio>
#include <cstdlib>
#include <limits>

int utils_TowerShieldRulesTest(int argc, char* argv[])
{
    (void)argc; (void)argv;
    const bool passed =
        EVEPOS::ShieldRules::ShouldDisplay(4, 4)
        && EVEPOS::ShieldRules::ShouldDisplay(5, 4)
        && !EVEPOS::ShieldRules::ShouldDisplay(1, 4)
        && EVEPOS::ShieldRules::EffectiveRadius(3, 42000.0) == 42000.0
        && EVEPOS::ShieldRules::EffectiveRadius(3, 0.0) == 30000.0
        && EVEPOS::ShieldRules::EffectiveRadius(2, 0.0) == 22500.0
        && EVEPOS::ShieldRules::EffectiveRadius(1, 0.0) == 15000.0
        && EVEPOS::ShieldRules::EffectiveRadius(3, -1.0) == 30000.0
        && EVEPOS::ShieldRules::EffectiveRadius(3,
            std::numeric_limits<double>::quiet_NaN()) == 30000.0;
    if (!passed) {
        ::printf("POS force-field visual test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("POS force-field visual test passed.\n");
    return EXIT_SUCCESS;
}
