#ifndef DESTINY_MOVEMENT_COMMAND_H
#define DESTINY_MOVEMENT_COMMAND_H

#include "math/gpoint.h"

#include <cmath>

namespace DestinyMovement {

constexpr double DIRECTION_TARGET_DISTANCE = 1.0e16;
constexpr double MIN_DIRECTION_LENGTH = 1.0e-9;
constexpr double SAME_DIRECTION_DOT = 0.9999;
constexpr double MIN_SPEED_FRACTION = 0.1;
constexpr double MAX_SPEED_FRACTION = 1.0;

inline bool IsValidSpeedFraction(double fraction) {
  return std::isfinite(fraction) && fraction >= 0.0 &&
         fraction <= MAX_SPEED_FRACTION &&
         (fraction == 0.0 || fraction >= MIN_SPEED_FRACTION);
}

inline bool IsValidDirection(const GPoint &direction) {
  if (direction.isNaN() or direction.isInf())
    return false;

  const double length = std::hypot(
      std::hypot(direction.x, direction.y), direction.z);
  return std::isfinite(length) && length > MIN_DIRECTION_LENGTH;
}

inline GVector NormalizeDirection(const GPoint &direction) {
  GVector normalized(direction);
  if (IsValidDirection(direction))
    normalized.normalize();
  else
    normalized = GVector();

  if (normalized.isNaN() or normalized.isInf() || normalized.isZero())
    normalized = GVector();
  return normalized;
}

inline bool IsSameDirection(const GVector &current, const GVector &requested) {
  if (!IsValidDirection(GPoint(current)) or
      !IsValidDirection(GPoint(requested)))
    return false;

  GVector currentUnit(current);
  GVector requestedUnit(requested);
  currentUnit.normalize();
  requestedUnit.normalize();
  return currentUnit.dotProduct(requestedUnit) >= SAME_DIRECTION_DOT;
}

} // namespace DestinyMovement

#endif
