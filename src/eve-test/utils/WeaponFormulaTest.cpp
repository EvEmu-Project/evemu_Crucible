#include "eve-test.h"
#include "EVE_WeaponFormulas.h"

#include <cmath>
#include <cstdlib>

int utils_WeaponFormulaTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const double fullDamage = CalculateMissileDamageFactor(100.0, 100.0, 100.0, 100.0, 0.5, 0.5);
    const double signatureLimited = CalculateMissileDamageFactor(50.0, 100.0, 100.0, 100.0, 0.5, 0.5);
    const double velocityLimited = CalculateMissileDamageFactor(100.0, 100.0, 100.0, 1000.0, 0.5, 0.5);
    const double cappedDamage = CalculateMissileDamageFactor(500.0, 100.0, 100.0, 1.0, 0.5, 0.5);
    const double invalidRadius = CalculateMissileDamageFactor(100.0, 0.0, 100.0, 100.0, 0.5, 0.5);

    const bool passed =
        std::fabs(fullDamage - 1.0) < 0.000001 &&
        std::fabs(signatureLimited - 0.5) < 0.000001 &&
        std::fabs(velocityLimited - 0.1) < 0.000001 &&
        std::fabs(cappedDamage - 1.0) < 0.000001 &&
        std::fabs(invalidRadius) < 0.000001;

    if (!passed) {
        ::printf("Weapon formula test failed: full=%.6f signature=%.6f velocity=%.6f capped=%.6f invalid=%.6f\n",
                 fullDamage, signatureLimited, velocityLimited, cappedDamage, invalidRadius);
        return EXIT_FAILURE;
    }

    ::printf("Weapon formula tests passed.\n");
    return EXIT_SUCCESS;
}
