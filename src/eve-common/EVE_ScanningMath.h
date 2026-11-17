#ifndef EVEMU_SCANNING_MATH_H
#define EVEMU_SCANNING_MATH_H

#include <cmath>

namespace EVEmuScan {

// The Crucible ConeScan client sends the FULL cone angle in radians and a
// direction vector that is not guaranteed to have unit length.
// Compare normalized vectors rather than applying acos() to a raw dot product.
inline bool WithinConeRadians(double axisX, double axisY, double axisZ,
                              double targetX, double targetY, double targetZ,
                              double fullAngle)
{
    constexpr double kPi = 3.1415926535897932384626433832795;
    if (!std::isfinite(fullAngle) || fullAngle <= 0.0
        || !std::isfinite(axisX) || !std::isfinite(axisY) || !std::isfinite(axisZ)
        || !std::isfinite(targetX) || !std::isfinite(targetY) || !std::isfinite(targetZ))
        return false;

    // The 360-degree setting must include targets behind the ship even if
    // the client supplies a zero-length camera direction.
    if (fullAngle >= 2.0 * kPi - 1e-6)
        return true;

    const double axisLength = std::sqrt(axisX*axisX + axisY*axisY + axisZ*axisZ);
    const double targetLength = std::sqrt(targetX*targetX + targetY*targetY + targetZ*targetZ);
    if (!std::isfinite(axisLength) || axisLength <= 0.0
        || !std::isfinite(targetLength) || targetLength <= 0.0)
        return false;

    const double rawCosine = (axisX*targetX + axisY*targetY + axisZ*targetZ)
        / (axisLength * targetLength);
    if (!std::isfinite(rawCosine))
        return false;
    const double cosine = rawCosine > 1.0 ? 1.0
                        : rawCosine < -1.0 ? -1.0 : rawCosine;
    return cosine >= std::cos(fullAngle / 2.0) - 1e-12;
}

} // namespace EVEmuScan

#endif // EVEMU_SCANNING_MATH_H
