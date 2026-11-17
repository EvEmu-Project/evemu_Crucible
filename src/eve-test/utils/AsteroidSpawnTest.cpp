#include "eve-test.h"
#include "EVE_AsteroidSpawnHelpers.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <unordered_map>

int utils_AsteroidSpawnTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    using Dist = std::unordered_multimap<float, uint16_t>;
    const Dist fractions{{0.25f, 34}, {0.75f, 35}};
    const Dist percentages{{25.0f, 34}, {75.0f, 35}};
    const Dist rounded{{0.10f, 34}, {0.20f, 35}};  // sum < 1
    const Dist validWithInvalid{
        {0.0f, 37}, {-1.0f, 38},
        {std::numeric_limits<float>::quiet_NaN(), 39}, {1.0f, 34}
    };

    auto countFirstType = [](const Dist& distribution) {
        int count = 0;
        for (int i = 0; i < 1000; ++i)
            if (EvEAsteroidSpawns::SelectWeightedType(i / 1000.0, distribution) == 34)
                ++count;
        return count;
    };

    const int fractionCount = countFirstType(fractions);
    const int percentageCount = countFirstType(percentages);
    const int roundedCount = countFirstType(rounded);
    const Dist empty;
    const bool passed =
        fractionCount >= 249 && fractionCount <= 251
        && percentageCount >= 249 && percentageCount <= 251
        && roundedCount >= 332 && roundedCount <= 335
        && EvEAsteroidSpawns::SelectWeightedType(1.0, fractions) != 0
        && EvEAsteroidSpawns::SelectWeightedType(0.0, validWithInvalid) == 34
        && EvEAsteroidSpawns::SelectWeightedType(0.5, empty) == 0
        && EvEAsteroidSpawns::SelectWeightedType(0.5,
            Dist{{0.0f, 34}, {-0.3f, 35}}) == 0
        && EvEAsteroidSpawns::SelectWeightedType(
            std::numeric_limits<double>::quiet_NaN(), fractions) == 0;

    if (!passed) {
        std::printf("Asteroid type-weight tests failed: fraction=%d percent=%d rounded=%d\n",
                    fractionCount, percentageCount, roundedCount);
        return EXIT_FAILURE;
    }
    std::printf("Asteroid type-weight tests passed.\n");
    return EXIT_SUCCESS;
}
