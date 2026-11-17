#include "eve-test.h"
#include "EVE_WeaponFormulas.h"

#include <cmath>
#include <cstdlib>
#include <limits>

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
    const double atDoubleFalloff = CalculateTurretHitChance(0.0, 0.1, 125, 125, 20000, 10000, 5000);
    const double underOptimal = CalculateTurretHitChance(0.0, 0.1, 125, 125, 9999, 10000, 5000);
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

    const double fiveSecondRange = CalculateMissileMaximumRange(1500.0, 5000.0);
    const double invalidRange = CalculateMissileMaximumRange(
        std::numeric_limits<double>::infinity(), 1000.0);

    const double defenderFirstHit = CalculateRemainingMissileHP(200.0, 75.0);
    const double defenderSecondHit = CalculateRemainingMissileHP(defenderFirstHit, 150.0);
    const double invalidDefenderHit = CalculateRemainingMissileHP(
        200.0, std::numeric_limits<double>::quiet_NaN());

    const double missileStep = CalculateMissileTravelStep(5000.0, 1500.0);
    const double cappedStep = CalculateMissileTravelStep(500.0, 1500.0);
    const double invalidStep = CalculateMissileTravelStep(500.0, -1.0);
    const bool impact = IsMissileWithinImpactRange(150.0, 25.0, 125.0);
    const bool noRemoteHit = IsMissileWithinImpactRange(150.01, 25.0, 125.0);
    const int repeatAfterOneCycle = WeaponCyclesRemainingAfterShot(2);
    const int repeatAfterTwoCycles = WeaponCyclesRemainingAfterShot(repeatAfterOneCycle);
    const int continuousRepeat = WeaponCyclesRemainingAfterShot(999);

    const bool passed =
        std::fabs(fullDamage - 1.0) < 0.000001 &&
        std::fabs(signatureLimited - 0.5) < 0.000001 &&
        std::fabs(velocityLimited - 0.1) < 0.000001 &&
        std::fabs(cappedDamage - 1.0) < 0.000001 &&
        std::fabs(invalidRadius) < 0.000001 &&
        std::fabs(stationary - 1.0) < 0.000001 &&
        std::fabs(atFalloff - 0.5) < 0.000001 &&
        std::fabs(atDoubleFalloff - 0.0625) < 0.000001 &&
        std::fabs(underOptimal - 1.0) < 0.000001 &&
        std::fabs(angular - 0.5) < 0.000001 &&
        std::fabs(smallSig - 0.0625) < 0.000001 &&
        noFalloff == 0.0 && std::fabs(normalHit - 0.69) < 0.000001 &&
        miss == 0.0f && wreck == 3.0f && noImpossibleWreck == 0.0f &&
        noResistance == 1.0f && immune == 0.0f && halfDamage == 0.5f &&
        fiveSecondRange == 7500.0 && invalidRange == 0.0 &&
        defenderFirstHit == 125.0 && defenderSecondHit == 0.0 &&
        invalidDefenderHit == 200.0 &&
        missileStep == 1500.0 && cappedStep == 500.0 && invalidStep == 0.0 &&
        impact && !noRemoteHit &&
        repeatAfterOneCycle == 1 && repeatAfterTwoCycles == 0 &&
        continuousRepeat == 999;

    if (!passed) {
        ::printf("Weapon formula test failed: full=%.6f signature=%.6f velocity=%.6f capped=%.6f invalid=%.6f\n",
                 fullDamage, signatureLimited, velocityLimited, cappedDamage, invalidRadius);
        return EXIT_FAILURE;
    }

    ::printf("Weapon formula tests passed.\n");
    return EXIT_SUCCESS;
}
