#include "eve-test.h"
#include "EVE_MovementFormulas.h"
#include "destiny/DestinyStructs.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <limits>

int utils_MovementFormulaTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const double normal = EVEmuMovement::SurfaceSeparation(500.0, 50.0, 100.0);
    const double touching = EVEmuMovement::SurfaceSeparation(150.0, 50.0, 100.0);
    const double wreckInRange = EVEmuMovement::SurfaceSeparation(2550.0, 25.0, 50.0);
    const double wreckOutOfRange = EVEmuMovement::SurfaceSeparation(2600.0, 25.0, 50.0);
    const double overlapping = EVEmuMovement::SurfaceSeparation(80.0, 50.0, 100.0);
    const double invalid = EVEmuMovement::SurfaceSeparation(
        std::numeric_limits<double>::quiet_NaN(), 50.0, 100.0);
    const double orbit = EVEmuMovement::MinimumOrbitRadius(500.0, 50.0, 1000.0);
    const double zeroOrbit = EVEmuMovement::MinimumOrbitRadius(0.0, 50.0, 1000.0);
    const double invalidOrbit = EVEmuMovement::MinimumOrbitRadius(
        std::numeric_limits<double>::infinity(), 50.0, 1000.0);
    const double miningOrbit = EVEmuMovement::MinimumOrbitRadius(200.0, 5.0, 16000.0);
    // The legacy high-order equation can predict a center radius inside
    // the required hull clearance at normal 10-20km NPC orbit distances.
    const double fixedGuristasOrbit = EVEmuMovement::ResolvedOrbitRadius(
        10150.0, 10143.0, 1000000.0);
    const double fixedLargeOrbit = EVEmuMovement::ResolvedOrbitRadius(
        20150.0, 19942.0, 1000000.0);
    const double validOrbit = EVEmuMovement::ResolvedOrbitRadius(
        1050.2, 1180.9, 1000000.0);
    const double overflowOrbit = EVEmuMovement::ResolvedOrbitRadius(
        10150.0, std::numeric_limits<double>::infinity(), 1000000.0);
    const double impossibleOrbit = EVEmuMovement::ResolvedOrbitRadius(
        1100000.0, 1150000.0, 1000000.0);
    const double zeroNpcCruise = EVEmuMovement::NpcCruiseSpeed(0.0, 190.0);
    const double validNpcCruise = EVEmuMovement::NpcCruiseSpeed(145.0, 190.0);
    const double stationaryNpc = EVEmuMovement::NpcCruiseSpeed(0.0, 0.0);
    const double invalidNpc = EVEmuMovement::NpcCruiseSpeed(
        std::numeric_limits<double>::quiet_NaN(), 140.0);
    static_assert(sizeof(Destiny::ORBIT_Struct) == sizeof(Destiny::FOLLOW_Struct));
    static_assert(offsetof(Destiny::ORBIT_Struct, targetID) == 1);
    static_assert(offsetof(Destiny::ORBIT_Struct, followRange) == 9);
    const Destiny::ORBIT_Struct orbitPacket{0xFF, 4500000000LL, 200.0f};
    Destiny::FOLLOW_Struct decodedOrbit{};
    std::memcpy(&decodedOrbit, &orbitPacket, sizeof(decodedOrbit));

    // The server stores absolute AU/s in dogma 600.
    const double battleshipWarp = EVEmuMovement::CrucibleWarpSpeedAU(3.0, 3.0);
    const double frigateWarp = EVEmuMovement::CrucibleWarpSpeedAU(6.0, 6.0);
    const double interceptorWarp = EVEmuMovement::CrucibleWarpSpeedAU(13.5, 13.5);
    const double logisticsWarp = EVEmuMovement::CrucibleWarpSpeedAU(3.75, 3.75);
    const double fittedWarpBonus = EVEmuMovement::CrucibleWarpSpeedAU(7.2, 6.0);
    const double fallbackType = EVEmuMovement::CrucibleWarpSpeedAU(0.0, 6.0);
    const double fallbackMissing = EVEmuMovement::CrucibleWarpSpeedAU(0.0, 0.0);
    const double freighterWarp = EVEmuMovement::CrucibleWarpSpeedAU(0.75, 0.75);
    const double rorqualWarp = EVEmuMovement::CrucibleWarpSpeedAU(0.0, 1.20);
    const double orcaWarp = EVEmuMovement::CrucibleWarpSpeedAU(2.70, 2.70);
    const double fittedOrca = EVEmuMovement::CrucibleWarpSpeedAU(3.24, 2.70);
    const double shortPull = EVEmuMovement::TractorTravelStep(600.0, 500.0, 600.0);
    const double fullPull = EVEmuMovement::TractorTravelStep(2000.0, 500.0, 600.0);
    const double noPull = EVEmuMovement::TractorTravelStep(499.0, 500.0, 600.0);
    const double invalidPull = EVEmuMovement::TractorTravelStep(2000.0, 500.0,
        std::numeric_limits<double>::infinity());
    const double warpTickOne = EVEmuMovement::WarpRemainingAfterAcceleration(10000, 10);
    const double warpTickTwo = EVEmuMovement::WarpRemainingAfterAcceleration(10000, 200);
    const double warpEnd = EVEmuMovement::WarpRemainingAfterAcceleration(10000, 20000);
    // Gate landing is measured between collision surfaces, not from the
    // stargate center. Test both a small hull and a much larger ship.
    const auto gateFrigateStop = EVEmuMovement::WarpCenterStopRange(
        0.0, 35.0, 10000.0, EVEmuMovement::GateWarpLandingSurfaceRange);
    const auto gateCapitalStop = EVEmuMovement::WarpCenterStopRange(
        0.0, 2500.0, 10000.0, EVEmuMovement::GateWarpLandingSurfaceRange);
    // Include a station larger than the generic 90km warp-in cutoff and
    // enough hull sizes to cover small ships, the Orca and large capitals.
    for (const double hullRadius : {35.0, 500.0, 2000.0, 12000.25}) {
        for (const double targetRadius : {10000.0, 125000.25}) {
            const auto stop = EVEmuMovement::WarpCenterStopRange(
                0.0, hullRadius, targetRadius, EVEmuMovement::GateWarpLandingSurfaceRange);
            const double gap = EVEmuMovement::InteractionSurfaceGap(stop, hullRadius, targetRadius);
            if (gap < 2400.0 || gap >= 2401.0
                || gap > EVEmuMovement::GateJumpSurfaceRange || gap > EVEmuMovement::DockSurfaceRange) {
                ::printf("Warp landing clearance failure: hull=%f target=%f gap=%f\n",
                         hullRadius, targetRadius, gap);
                return EXIT_FAILURE;
            }
        }
    }
    const auto requestedTenKm = EVEmuMovement::WarpCenterStopRange(
        10000.0, 150.0, 10000.0, EVEmuMovement::GateWarpLandingSurfaceRange);
    const auto requestedHundredKm = EVEmuMovement::WarpCenterStopRange(
        100000.0, 150.0, 10000.0, EVEmuMovement::GateWarpLandingSurfaceRange);
    const auto fractionalStop = EVEmuMovement::WarpCenterStopRange(
        0.0, 10.25, 20.25, EVEmuMovement::GateWarpLandingSurfaceRange);
    const auto invalidStop = EVEmuMovement::WarpCenterStopRange(
        0.0, std::numeric_limits<double>::quiet_NaN(), 10000.0,
        EVEmuMovement::GateWarpLandingSurfaceRange);
    // Simulate the final 1-second approach to a 2.4km surface standoff.
    // High-speed gate and station approaches must never travel past it.
    const double gateFinalTick = EVEmuMovement::TractorTravelStep(
        2750.0, EVEmuMovement::GateWarpLandingSurfaceRange, 500.0);
    const double gateFullTick = EVEmuMovement::TractorTravelStep(
        5000.0, EVEmuMovement::GateWarpLandingSurfaceRange, 500.0);
    const double gateInRange = EVEmuMovement::TractorTravelStep(
        2300.0, EVEmuMovement::GateWarpLandingSurfaceRange, 500.0);
    const double gateInvalidSpeed = EVEmuMovement::TractorTravelStep(
        2750.0, EVEmuMovement::GateWarpLandingSurfaceRange,
        std::numeric_limits<double>::quiet_NaN());
    const double axisApproach = EVEmuMovement::ApproachTravelStep(13000.0, 350.0, 1.0, 500.0);
    const double angledApproach = EVEmuMovement::ApproachTravelStep(13000.0, 350.0, 0.8, 500.0);
    const double missedApproach = EVEmuMovement::ApproachTravelStep(13000.0, 350.0, 0.1, 500.0);
    const double angledEndDistance = std::sqrt(13000.0 * 13000.0
        + angledApproach * angledApproach - 2.0 * 13000.0 * 0.8 * angledApproach);
    const double invalidInteraction = EVEmuMovement::InteractionSurfaceGap(
        std::numeric_limits<double>::quiet_NaN(), 50.0, 100.0);
    const double shortDecelStart = EVEmuMovement::WarpDecelerationRemaining(
        120000.0, 1.0, 60000.0, 100.0);
    const double shortDecelFinalTick = EVEmuMovement::WarpDecelerationRemaining(
        120000.0, std::exp(-7.0), 60000.0, 100.0);
    const double shortDecelAtCutoff = EVEmuMovement::WarpDecelerationRemaining(
        120000.0, 100.0 / 60000.0, 60000.0, 100.0);
    const double shortDecelJustAboveCutoff = EVEmuMovement::WarpDecelerationRemaining(
        120000.0, 100.01 / 60000.0, 60000.0, 100.0);
    const double longDecelFinalTick = EVEmuMovement::WarpDecelerationRemaining(
        1.32e9, std::exp(-22.0), 4.49e11, 100.0);
    const double longDecelComplete = EVEmuMovement::WarpDecelerationRemaining(
        1.32e9, std::exp(-23.0), 4.49e11, 100.0);
    const bool passed =
        gateFinalTick == 350.0 && gateFullTick == 500.0
        && gateInRange == 0.0 && gateInvalidSpeed == 0.0
        && axisApproach == 350.0 && missedApproach == 500.0
        && angledApproach > 350.0 && angledApproach < 500.0
        && std::abs(angledEndDistance - 12650.0) < 1e-8
        && EVEmuMovement::ApproachTravelStep(13000.0, 350.0, -1.0, 500.0) == 500.0
        && std::isinf(invalidInteraction)
        && std::isinf(EVEmuMovement::InteractionSurfaceGap(400.0, -1.0, 50.0))
        && shortDecelStart == 120000.0 && shortDecelFinalTick == 0.0 && shortDecelAtCutoff == 0.0
        && shortDecelJustAboveCutoff > 0.0 && shortDecelJustAboveCutoff < 0.03
        && longDecelFinalTick > 0.0 && longDecelFinalTick < 0.1 && longDecelComplete == 0.0
        && EVEmuMovement::DegreesPerMovementTick(5.0f) == 5.5f
        && EVEmuMovement::DegreesPerMovementTick(40.0f) == 2.0f
        && EVEmuMovement::DegreesPerMovementTick(60.0f) == 1.0f
        && EVEmuMovement::DegreesPerMovementTick(75.0f) == 1.0f
        && gateFrigateStop == 12435 && gateCapitalStop == 14900
        && EVEmuMovement::SurfaceSeparation(gateFrigateStop, 35.0, 10000.0) == 2400.0
        && EVEmuMovement::SurfaceSeparation(gateCapitalStop, 2500.0, 10000.0) == 2400.0
        && EVEmuMovement::GateWarpLandingSurfaceRange < EVEmuMovement::GateJumpSurfaceRange
        && requestedTenKm == 20150
        && requestedHundredKm == 110150
        && EVEmuMovement::SurfaceSeparation(requestedTenKm, 150.0, 10000.0) == 10000.0
        && fractionalStop == 2431
        && invalidStop == std::numeric_limits<std::int32_t>::max()
        && EVEmuMovement::WarpCenterStopRange(0.0, 0.0, 0.0, 0.0) == 0
        && shortPull == 100 && fullPull == 600 && noPull == 0 && invalidPull == 0 &&
        warpTickOne == 9990 && warpTickTwo == 9800 && warpEnd == 0 &&
        normal == 350.0 && touching == 0.0 &&
        wreckInRange == 2475.0 && wreckOutOfRange == 2525.0 &&
        overlapping == 0.0 &&
        invalid == 0.0 && orbit == 1575.0 && zeroOrbit == 1075.0 &&
        invalidOrbit == 0.0 && miningOrbit == 16230.0 &&
        fixedGuristasOrbit == 10150.0 && fixedLargeOrbit == 20150.0 &&
        validOrbit == 1180.0 && overflowOrbit == 10150.0 &&
        impossibleOrbit == 0.0 && zeroNpcCruise == 190.0 &&
        validNpcCruise == 145.0 && stationaryNpc == 0.0 &&
        invalidNpc == 140.0 &&
        decodedOrbit.followID == 4500000000LL && decodedOrbit.followRange == 200.0f &&
        battleshipWarp == 3.0 && frigateWarp == 6.0 &&
        interceptorWarp == 13.5 && logisticsWarp == 3.75 &&
        fittedWarpBonus == 7.2 && fallbackType == 6.0 && fallbackMissing == 3.0 &&
        freighterWarp == 0.75 && rorqualWarp == 1.20 && orcaWarp == 2.70 && fittedOrca == 3.24 &&
        EVEmuMovement::WarpSpeedTenths(freighterWarp) == 8 &&
        EVEmuMovement::WarpSpeedTenths(rorqualWarp) == 12 &&
        EVEmuMovement::WarpSpeedTenths(orcaWarp) == 27 &&
        EVEmuMovement::WarpSpeedTenths(fittedOrca) == 32 &&
        EVEmuMovement::WarpSpeedTenths(1.50) == 15;

    if (!passed) {
        ::printf("Movement formula failure: normal=%f touching=%f overlap=%f invalid=%f "
                 "orbit=%f zeroOrbit=%f invalidOrbit=%f\n",
                 normal, touching, overlapping, invalid, orbit, zeroOrbit, invalidOrbit);
        return EXIT_FAILURE;
    }
    ::printf("Movement formula tests passed.\n");
    return EXIT_SUCCESS;
}
