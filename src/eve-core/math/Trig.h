
/**
 * @name EVE_Trig.h
 *    math defines and methods for trig used in EvEmu
 *  using EvE namespace
 * @Author:         Allan
 * @date:   30 Aug 2015
 */

#include "math/gpoint.h"

// EvE uses the 3d left hand cartesian coordinate system, centered on star.
// +x is left, +y is up elevation, +z is "into", or up in 2d

namespace EvE {

    namespace Trig {

        const double E = 2.71828182845904523536028747135;
        const double Pi = 3.1415926535897932384626433832795;
        const double Pi2 = 6.28318530717958647692528676656;
        const double RadiansInDegrees = 0.0174532925199432957692369076849;   //  pi/180
        const double DegreesInRadians = 57.2957795130823208767981548141;   //  180/pi

        inline double Deg2Rad(double deg) { return (deg * RadiansInDegrees); }
        inline double Rad2Deg(double rad) { return (rad * DegreesInRadians); }



/*
 * azimuth is the counterclockwise angle in the x-y plane measured in radians from the positive x-axis.
 * elevation is the elevation angle in radians from the x-y plane.
 */

/*
The mapping from spherical coordinates to three-dimensional Cartesian coordinates is

x = r .* cos(elevation) .* cos(azimuth)
y = r .* cos(elevation) .* sin(azimuth)
z = r .* sin(elevation)
*/
        //GPoint Sph2Cart(float az, float ele, float radius) {        }

/*The mapping from three-dimensional Cartesian coordinates to spherical coordinates is

azimuth = atan2(y,x)
elevation = atan2(z,sqrt(x.^2 + y.^2))
r = sqrt(x.^2 + y.^2 + z.^2)

The notation for spherical coordinates is not standard.
For the cart2sph function, elevation is measured from the x-y plane.
Notice that if elevation = 0, the point is in the x-y plane.
If elevation = pi/2, then the point is on the positive z-axis.
*/

        //void Cart2Sph(GPoint pos, float& az, float& ele, float& radius) {        }


    }
}

/*
 * def RayToPlaneIntersection(P, d, Q, n):
 *    """
 *        The intersection point(S) on a plane where d shot from P would intersect
 *        the plane defined by Q and n.
 *
 *        If the P lies on the plane defined by n and Q, there are infinite number of
 *        intersection points so the function returns P.
 *
 *        d' = - Q.Dot(n)
 *        t = -(n.Dot(P) + d' )/n.Dot(d)
 *        S = P + t*d
 *    """
 *    denom = geo2.Vec3Dot(n, d)
 *    if abs(denom) < 1e-05:
 *        return P
 *    else:
 *        distance = -geo2.Vec3Dot(Q, n)
 *        t = -(geo2.Vec3Dot(n, P) + distance) / denom
 *        scaledRay = geo2.Scale(d, t)
 *        ret = geo2.Add(scaledRay, P)
 *        return geo2.Vector(*ret)
 */