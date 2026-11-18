#include "eve-test.h"
#include "EVE_ScanningMath.h"

#include <cmath>
#include <cstdlib>
#include <limits>

int utils_ScanDirectionTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    constexpr double pi = 3.1415926535897932384626433832795;
    const double nan = std::numeric_limits<double>::quiet_NaN();

    // Unnormalized camera vectors must produce the same result as unit axes.
    const bool passed =
        EVEmuScan::WithinConeRadians(1, 0, 0, 100, 0, 0, pi/6)
        && EVEmuScan::WithinConeRadians(25, 0, 0, 300, 0, 0, pi/6)
        && !EVEmuScan::WithinConeRadians(25, 0, 0, 0, 100, 0, pi/6)
        && !EVEmuScan::WithinConeRadians(1, 0, 0, -100, 0, 0, pi/6)
        && EVEmuScan::WithinConeRadians(1, 0, 0, 0, 100, 0, pi)
        && !EVEmuScan::WithinConeRadians(1, 0, 0, 0, 100, 0, pi/6)
        && EVEmuScan::WithinConeRadians(0, 0, 0, -5, 0, 0, 2*pi)
        && !EVEmuScan::WithinConeRadians(0, 0, 0, -5, 0, 0, pi/6)
        && !EVEmuScan::WithinConeRadians(1, 0, 0, 5, 0, 0, nan)
        && !EVEmuScan::WithinConeRadians(1, 0, 0, nan, 0, 0, pi)
        && !EVEmuScan::WithinConeRadians(1, 0, 0, 0, 0, 0, pi)
        && !EVEmuScan::WithinConeRadians(1, 0, 0, 5, 0, 0, 0);

    if (!passed) {
        ::printf("Directional scan cone regression test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Directional scan cone regression test passed.\n");
    return EXIT_SUCCESS;
}
