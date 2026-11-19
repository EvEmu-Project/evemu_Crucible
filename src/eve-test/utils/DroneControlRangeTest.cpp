#include "eve-test.h"
#include "EVE_DroneControlRange.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

int utils_DroneControlRangeTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const double base = EvEDrone::ControlRange(0, 0);
    const double scoutV = EvEDrone::ControlRange(5, 0);
    const double bothV = EvEDrone::ControlRange(5, 5);
    const double withBonus = EvEDrone::ControlRange(5, 5, 0, 0, 20000);
    const double fitted = EvEDrone::ControlRange(5, 5, 105000, 0, 20000);
    const double charValue = EvEDrone::ControlRange(1, 0, 0, 75000);
    const double invalid = EvEDrone::ControlRange(-5, 99,
        std::numeric_limits<double>::quiet_NaN(), -100.0,
        std::numeric_limits<double>::infinity());

    if (base != 20000.0 || scoutV != 45000.0 || bothV != 60000.0
        || withBonus != 80000.0 || fitted != 105000.0
        || charValue != 75000.0 || invalid != 35000.0) {
        ::printf("Drone control range failed: %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n",
                 base, scoutV, bothV, withBonus, fitted, charValue, invalid);
        return EXIT_FAILURE;
    }
    ::printf("Drone control range tests passed.\n");
    return EXIT_SUCCESS;
}
