#include "EVE_MiningFlight.h"
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

    // Use absolute system coordinates, a large asteroid, and a moving ship.
    // Every published mining drone must finish 60 orbit ticks and make it
    // back into unloading range without a step exceeding its speed.
    const GPoint rock(149597870700.0, -294000000000.0, 200000000000.0);
    const double droneRadius = 5.0;
    const double rockRadius = 16000.0;
    const double orbitRadius = rockRadius + droneRadius + 200.0;
    const double shipRadius = 500.0;
    const double returnRadius = shipRadius + droneRadius + 500.0;
    if (EvEMining::MovementMass(0.0) != 3000.0 || EvEMining::MovementMass(4500.0) != 4500.0)
        return 1;

    for (const double speed : {250.0, 300.0, 400.0, 500.0}) {
        GPoint ship = rock + GVector(15000.0, 15000.0, 0.0);
        GPoint drone = ship;
        int miningTicks = 0;
        for (int tick = 0; tick < 180 && miningTicks < 60; ++tick) {
            const auto flight = EvEMining::Orbit(drone, rock, orbitRadius, speed);
            const double radius = flight.position.distance(rock);
            if (!std::isfinite(radius) || drone.distance(flight.position) > speed + 0.001
                || (miningTicks > 0 && !flight.inOrbit)
                || (flight.inOrbit && std::abs(radius - orbitRadius) > 0.001)) {
                std::fprintf(stderr, "Mining flight left its orbit or exceeded its speed\n");
                return 1;
            }
            drone = flight.position;
            if (flight.inOrbit)
                ++miningTicks;
        }
        if (miningTicks != 60)
            return 1;

        for (int tick = 0; tick < 200 && drone.distance(ship) > returnRadius + 500.0; ++tick) {
            ship += GVector(50.0, -20.0, 15.0);
            const GPoint next = EvEMining::Approach(drone, ship, returnRadius, speed);
            if (drone.distance(next) > speed + 0.001 || next.distance(ship) < returnRadius - 0.001)
                return 1;
            drone = next;
        }
        if (drone.distance(ship) > returnRadius + 500.0) {
            std::fprintf(stderr, "Mining drone did not reach the moving ship\n");
            return 1;
        }
    }

    const auto fromCenter = EvEMining::Orbit(rock, rock, orbitRadius, 500.0);
    if (fromCenter.inOrbit || fromCenter.position.distance(rock) != 500.0)
        return 1;
    return 0;
}
