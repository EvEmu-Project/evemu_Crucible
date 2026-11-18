#include "eve-test.h"
#include "EVE_SovIndexOverride.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int utils_SovIndexOverrideTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const EVESovIndex::Override test{
        140000001,
        EVESovIndex::LevelFiveMilitaryPoints,
        EVESovIndex::LevelFiveIndustrialPoints,
        EVESovIndex::LevelFiveStrategicDays};
    const bool passed =
        // Artificial activity points must fit signed 32-bit client rows,
        // while exceeding the historical Level 5 mining amount (24m m3).
        EVESovIndex::LevelFiveIndustrialPoints >= 24000000U
        && EVESovIndex::LevelFiveMilitaryPoints > EVESovIndex::LevelFiveIndustrialPoints
        && EVESovIndex::LevelFiveMilitaryPoints <= 2147483647U
        && EVESovIndex::LevelFiveIndustrialPoints <= 2147483647U
        // Same TCU must receive the override.
        && EVESovIndex::IsActive(140000001, test)
        && EVESovIndex::EffectivePoints(0, &test, 140000001, true)
            == EVESovIndex::LevelFiveMilitaryPoints
        && EVESovIndex::EffectivePoints(0, &test, 140000001, false)
            == EVESovIndex::LevelFiveIndustrialPoints
        && EVESovIndex::EffectiveStrategicDays(0, &test, 140000001) == 100
        && EVESovIndex::EffectiveStrategicDays(7, &test, 140000001) == 100
        // Preserve a longer *real* claim age.
        && EVESovIndex::EffectiveStrategicDays(140, &test, 140000001) == 140
        // Claim transfers must not inherit old level-five testing data.
        && !EVESovIndex::IsActive(140000002, test)
        && EVESovIndex::EffectivePoints(1, &test, 140000002, true) == 1
        && EVESovIndex::EffectivePoints(2, &test, 140000002, false) == 2
        && EVESovIndex::EffectiveStrategicDays(0, &test, 140000002) == 0
        && EVESovIndex::EffectivePoints(2, &test, 0, true) == 2
        && EVESovIndex::EffectiveStrategicDays(9, nullptr, 140000001) == 9;
    if (!passed) {
        ::printf("Sovereignty index Level 5 test override regression failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Sovereignty index Level 5 test override regression passed.\n");
    return EXIT_SUCCESS;
}
