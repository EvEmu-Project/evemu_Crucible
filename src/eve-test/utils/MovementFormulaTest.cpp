#include "eve-test.h"
#include "EVE_MovementFormulas.h"

#include <cmath>
#include <cstdlib>
#include <limits>

int utils_MovementFormulaTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const double normal = EVEmuMovement::SurfaceSeparation(500.0, 50.0, 100.0);
    const double touching = EVEmuMovement::SurfaceSeparation(150.0, 50.0, 100.0);
    const double overlapping = EVEmuMovement::SurfaceSeparation(80.0, 50.0, 100.0);
    const double invalid = EVEmuMovement::SurfaceSeparation(
        std::numeric_limits<double>::quiet_NaN(), 50.0, 100.0);
    const double orbit = EVEmuMovement::MinimumOrbitRadius(500.0, 50.0, 1000.0);
    const double zeroOrbit = EVEmuMovement::MinimumOrbitRadius(0.0, 50.0, 1000.0);
    const double invalidOrbit = EVEmuMovement::MinimumOrbitRadius(
        std::numeric_limits<double>::infinity(), 50.0, 1000.0);

    const bool passed =
        normal == 350.0 && touching == 0.0 && overlapping == 0.0 &&
        invalid == 0.0 && orbit == 1575.0 && zeroOrbit == 1075.0 &&
        invalidOrbit == 0.0;

    if (!passed) {
        ::printf("Movement formula failure: normal=%f touching=%f overlap=%f invalid=%f "
                 "orbit=%f zeroOrbit=%f invalidOrbit=%f\n",
                 normal, touching, overlapping, invalid, orbit, zeroOrbit, invalidOrbit);
        return EXIT_FAILURE;
    }
    ::printf("Movement formula tests passed.\n");
    return EXIT_SUCCESS;
}
