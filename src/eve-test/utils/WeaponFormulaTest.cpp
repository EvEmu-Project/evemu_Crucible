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

    // Legacy turret signature resolution, tracking, falloff and shared roll:
    const double stationary = CalculateTurretHitChance(0.0, 0.1, 125, 125, 10000, 10000, 5000);
    const double atFalloff = CalculateTurretHitChance(0.0, 0.1, 125, 125, 15000, 10000, 5000);
    const double angular = CalculateTurretHitChance(0.1, 0.1, 125, 125, 10000, 10000, 5000);
    const double smallSig = CalculateTurretHitChance(0.1, 0.1, 125, 62.5, 10000, 10000, 5000);
    const double noFalloff = CalculateTurretHitChance(0.0, 0.1, 125, 125, 11000, 10000, 0);
    const float normalHit = CalculateTurretHitQuality(0.5, 0.2);
    const float miss = CalculateTurretHitQuality(0.1, 0.5);
    const float wreck = CalculateTurretHitQuality(0.5, 0.005);
    const float noImpossibleWreck = CalculateTurretHitQuality(0.001, 0.005);
    const float noResistance = NormalizeDamageResonance(0.0, false);
    const float immune = NormalizeDamageResonance(0.0, true);
    const float halfDamage = NormalizeDamageResonance(0.5, true);

    const bool passed =
        std::fabs(fullDamage - 1.0) < 0.000001 &&
        std::fabs(signatureLimited - 0.5) < 0.000001 &&
        std::fabs(velocityLimited - 0.1) < 0.000001 &&
        std::fabs(cappedDamage - 1.0) < 0.000001 &&
        std::fabs(invalidRadius) < 0.000001 &&
        std::fabs(stationary - 1.0) < 0.000001 &&
        std::fabs(atFalloff - 0.5) < 0.000001 &&
        std::fabs(angular - 0.5) < 0.000001 &&
        std::fabs(smallSig - 0.0625) < 0.000001 &&
        noFalloff == 0.0 && std::fabs(normalHit - 0.69) < 0.000001 &&
        miss == 0.0f && wreck == 3.0f && noImpossibleWreck == 0.0f &&
        noResistance == 1.0f && immune == 0.0f && halfDamage == 0.5f;

    if (!passed) {
        ::printf("Weapon formula test failed: full=%.6f signature=%.6f velocity=%.6f capped=%.6f invalid=%.6f\n",
                 fullDamage, signatureLimited, velocityLimited, cappedDamage, invalidRadius);
        return EXIT_FAILURE;
    }

    ::printf("Weapon formula tests passed.\n");
    return EXIT_SUCCESS;
}
