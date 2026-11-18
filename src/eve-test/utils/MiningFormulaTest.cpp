#include "EVE_MiningFormulas.h"

#include <cstdio>

int utils_MiningFormulaTest(int, char**)
{
    for (const double base : {10.0, 15.0, 25.0, 30.0}) {
        if (EvEMining::DroneYield(base, 0, 0) != base
            || EvEMining::DroneYield(base, 5, 5) != base * 2.5
            || EvEMining::DroneYield(base, -1, 6) != base * 2.0) {
            std::fprintf(stderr, "Unexpected mining drone yield for base volume %.2f\n", base);
            return 1;
        }
    }
    if (EvEMining::OreUnits(15.0, 0.1f, 500.0) != 150
        || EvEMining::OreUnits(25.0, 1.2f, 500.0) != 20
        || EvEMining::OreUnits(25.0, 1.2f, 4.5) != 4
        || EvEMining::OreUnits(2.4f, 1.2f, 10.0) != 2
        || EvEMining::OreUnits(2.399, 1.2f, 10.0) != 1
        || EvEMining::OreUnits(0.9, 1.0, 5.0) != 0
        || EvEMining::OreUnits(10.0, 1.0, 0.9) != 0
        || EvEMining::OreUnits(10.0, 0.0, 5.0) != 0
        || EvEMining::OreUnits(10.0, 1.0, std::numeric_limits<double>::quiet_NaN()) != 0) {
        std::fprintf(stderr, "Whole-unit mining, depletion, or hold-capacity calculation failed\n");
        return 1;
    }
    return 0;
}
