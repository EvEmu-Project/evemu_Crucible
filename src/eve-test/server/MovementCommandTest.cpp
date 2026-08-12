#include "eve-test.h"

#include "destiny/MovementCommand.h"

#include <cmath>
#include <cstdio>
#include <limits>

int server_MovementCommandTest(int, char **) {
  if (DestinyMovement::IsValidSpeedFraction(
          std::numeric_limits<double>::quiet_NaN()) or
      DestinyMovement::IsValidSpeedFraction(
          std::numeric_limits<double>::infinity()) or
      DestinyMovement::IsValidSpeedFraction(-0.1) or
      DestinyMovement::IsValidSpeedFraction(1.1) or
      !DestinyMovement::IsValidSpeedFraction(0.0) or
      !DestinyMovement::IsValidSpeedFraction(
          DestinyMovement::MIN_SPEED_FRACTION) or
      !DestinyMovement::IsValidSpeedFraction(
          DestinyMovement::MAX_SPEED_FRACTION)) {
    std::fprintf(stderr, "Speed fraction validation failed\n");
    return EXIT_FAILURE;
  }

  const GVector normalized =
      DestinyMovement::NormalizeDirection(GPoint(0.0, 3.0, 4.0));
  if (std::abs(normalized.x) > 1.0e-12 or
      std::abs(normalized.y - 0.6) > 1.0e-12 or
      std::abs(normalized.z - 0.8) > 1.0e-12) {
    std::fprintf(stderr, "Direction normalization failed\n");
    return EXIT_FAILURE;
  }

  if (DestinyMovement::IsValidDirection(GPoint()) or
      DestinyMovement::IsValidDirection(
          GPoint(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0)) or
      DestinyMovement::IsValidDirection(
          GPoint(std::numeric_limits<double>::infinity(), 0.0, 0.0)) or
      DestinyMovement::IsValidDirection(
          GPoint(std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max(), 0.0)) or
      DestinyMovement::IsValidDirection(
          GPoint(DestinyMovement::MIN_DIRECTION_LENGTH, 0.0, 0.0))) {
    std::fprintf(stderr, "Invalid direction accepted\n");
    return EXIT_FAILURE;
  }

  if (!DestinyMovement::IsValidDirection(GPoint(
          std::nextafter(DestinyMovement::MIN_DIRECTION_LENGTH, 1.0),
          0.0, 0.0))) {
    std::fprintf(stderr, "Direction threshold rejected\n");
    return EXIT_FAILURE;
  }

  if (!DestinyMovement::IsSameDirection(normalized, GVector(0.0, 6.0, 8.0)) or
      DestinyMovement::IsSameDirection(normalized, GVector(0.0, -3.0, 4.0))) {
    std::fprintf(stderr, "Direction equivalence failed\n");
    return EXIT_FAILURE;
  }

  const double boundary = DestinyMovement::SAME_DIRECTION_DOT;
  const double aboveBoundary = boundary + 1.0e-6;
  const double belowBoundary = boundary - 1.0e-6;
  const GVector above(
      aboveBoundary, std::sqrt(1.0 - aboveBoundary * aboveBoundary), 0.0);
  const GVector below(
      belowBoundary, std::sqrt(1.0 - belowBoundary * belowBoundary), 0.0);
  if (!DestinyMovement::IsSameDirection(GVector(1.0, 0.0, 0.0),
                                        above) or
      DestinyMovement::IsSameDirection(
          GVector(1.0, 0.0, 0.0),
          below)) {
    std::fprintf(stderr, "Direction boundary failed\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
