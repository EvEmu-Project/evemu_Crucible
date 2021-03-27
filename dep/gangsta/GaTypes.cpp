/*
    This file is part of the Gangsta Wrapper physics SDK abstraction library suite.
    Copyright (C) 2005 Ed Jones (Green Eyed Monster)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	See also the files README.txt and LICENSE.txt in the root directory of this module.
*/
#include "GaTypes.h"

using namespace Ga;
GaQuat GaQuat::IDENTITY = GaQuat(1,0,0,0);
GaQuat GaQuat::ZERO = GaQuat(0,0,0,0);

GaQuat::GaQuat(const GaRadian &a,const GaVec3 &axis)
{
	w = Math::cosine(a.r * 0.5);

    GaFloat sin_a = Math::sine(a.r * 0.5);
	v.x = axis.x * sin_a;
	v.y = axis.y * sin_a;
	v.z = axis.z * sin_a;

    v.x *= 57.2957795131;   //  180/pi (rad->deg)
    v.y *= 57.2957795131;   //  180/pi (rad->deg)
    v.z *= 57.2957795131;   //  180/pi (rad->deg)

    if (v.x < 0)
        v.x += 360;
    if (v.y < 0)
        v.y += 360;
    if (v.z < 0)
        v.z += 360;
}

GaFloat matrix_minor(const GaMat4x4 &m,GaUint r0,GaUint r1,GaUint r2,GaUint c0,GaUint c1,GaUint c2)
{
	return m[r0][c0] * (m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2]) -
           m[r0][c1] * (m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2]) +
           m[r0][c2] * (m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1]);
}

GaFloat GaVec3::angle(float ax, float ay, float bx, float by)
{
    GaFloat dx = bx - ax;
    GaFloat dy = by - ay;
    GaFloat result = ((atan2(dx , dy)) * 180 /Math::GaPi);
    result -= 90;
    if (result < 0)
        result += 360;

    return result;
}

GaVec3 GaVec3::rotationTo(const GaVec3 &pos) const
{
    // yaw, pitch, roll = dunRotation

    GaQuat q(GaQuat::ZERO);
    GaVec3 axis(*this);
    axis.normalize();
    /*
    GaVec3 toVec((pos.x - x), (pos.y - y), (pos.z - z)); // (to, from)
    toVec.normalize();
    GaFloat dot = toVec.dotProduct(axis);
    GaFloat radians = Math::arcCosine(dot);
    if (isnan(radians))
        return q;

    q.w = Math::cosine(radians * 0.5);
    if ((q.w > 1) or (q.w < 0))
        return GaQuat::ZERO;
*/
    q.v.x = Math::arcTangent(pos.z - z, pos.x - x);  // rad from dir to target on z axis
    q.v.y = axis.x;
    q.v.z = 0;

    /*
    GaFloat sin_a = Math::sine(radians * 0.5);
    q.v.x = axis.x * sin_a;
    q.v.y = axis.y * sin_a;
    q.v.z = axis.z * sin_a;
    */
    //q.v.x *= Math::GaDegreesInRadian; //57.2957795131;   //  180/pi (rad->deg)
    //q.v.y *= Math::GaDegreesInRadian; //57.2957795131;   //  180/pi (rad->deg)
    q.v.z *= Math::GaDegreesInRadian; //57.2957795131;   //  180/pi (rad->deg)

    /*
    if (q.v.x < 0)
        q.v.x += 360;
    if (q.v.y < 0)
        q.v.y += 360;
    if (q.v.z < 0)
        q.v.z += 360;
    */

    return GaVec3(q.v.x, q.v.y, q.v.z);
}
/*
GaVec3 GaVec3::rotByQuat(GaVec3 v, GaQuat q)
{
    // Extract the vector part of the quaternion
    GaVec3 u(q.v.x, q.v.y, q.v.z);
    // Extract the scalar part of the quaternion
    double s = q.w;
    // Do the math
    return u* (2.0 * (u* v)) + v* (s*s - (u* u)) + (u^ v)* (2.0 * s);
}

GaVec3 GaVec3::angleRot(GaVec3 from, GaVec3 to, double angle)
{
    GaVec3 axis((to.x-from.x), (to.y-from.y), (to.z-from.z));
    axis.normalize();
    angle = angle / 2.0;

    GaQuat q_temp(0.0, 0.0, 0.0, 1.0 );
    q_temp.v.x = axis.x * sin(angle);
    q_temp.v.y = axis.y * sin(angle);
    q_temp.v.z = axis.z * sin(angle);
    q_temp.w = cos(angle);
*/
    /*
     *    q.v.x *= Math::GaDegreesInRadian; //57.2957795131;   //  180/pi (rad->deg)
     *    q.v.y *= Math::GaDegreesInRadian; //57.2957795131;   //  180/pi (rad->deg)
     *    q.v.z *= Math::GaDegreesInRadian; //57.2957795131;   //  180/pi (rad->deg)
     *
     *    if (q.v.x < 0)
     *        q.v.x += 360;
     *    if (q.v.y < 0)
     *        q.v.y += 360;
     *    if (q.v.z < 0)
     *        q.v.z += 360;
     */
    /*
    return rotByQuat(from, q_temp);
}
*/
GaVec3 GaVec3::slerp(GaVec3 v0, GaVec3 v1, double t)
{
    // Only unit quaternions are valid rotations.
    // Normalize to avoid undefined behavior.
    v0.normalize();
    v1.normalize();

    // Compute the cosine of the angle between the two vectors.
    double dot = v0.dotProduct(v1);

    // If the dot product is negative, slerp won't take
    // the shorter path. Note that v1 and -v1 are equivalent when
    // the negation is applied to all four components. Fix by
    // reversing one quaternion.
    if (dot < 0.0f) {
        v1 = (v1 * -1);
        dot = -dot;
    }

    if (dot > 0.9995) {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.

        GaVec3 result = v0 + t*(v1 - v0);
        result.normalize();
        return result;
    }

    // Since dot is in range [0, 0.9995], acos is safe
    double theta_0 = acos(dot);        // theta_0 = angle between input vectors
    double theta = theta_0*t;          // theta = angle between v0 and result
    double sin_theta = sin(theta);     // compute this value only once
    double sin_theta_0 = sin(theta_0); // compute this value only once

    double s0 = cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
    double s1 = sin_theta / sin_theta_0;

    return (s0 * v0) + (s1 * v1);
}

/*
public static Vector3 RotateX(Vector3 v1, double rad)
{
    double x = v1.X;
    double y = (v1.Y * Math.Cos(rad)) - (v1.Z * Math.Sin(rad));
    double z = (v1.Y * Math.Sin(rad)) + (v1.Z * Math.Cos(rad));
    return new Vector3(x, y, z);
}

public Vector3 RotateX(double rad)
{
    return RotateX(this, rad);
}

public static Vector3 Pitch(Vector3 v1, double rad)
{
    return RotateX(v1, rad);
}

public Vector3 Pitch(double rad)
{
    return Pitch(this, rad);
}
public static Vector3 RotateY(Vector3 v1, double rad)
{
    double x = (v1.Z * Math.Sin(rad)) + (v1.X * Math.Cos(rad));
    double y = v1.Y;
    double z = (v1.Z * Math.Cos(rad)) - (v1.X * Math.Sin(rad));
    return new Vector3(x, y, z);
}

public Vector3 RotateY(double rad)
{
    return RotateY(this, rad);
}

public static Vector3 Yaw(Vector3 v1, double rad)
{
    return RotateY(v1, rad);
}

public Vector3 Yaw(double rad)
{
    return Yaw(this, rad);
}
public static Vector3 RotateZ(Vector3 v1, double rad)
{
    double x = (v1.X * Math.Cos(rad)) - (v1.Y * Math.Sin(rad));
    double y = (v1.X * Math.Sin(rad)) + (v1.Y * Math.Cos(rad));
    double z = v1.Z;
    return new Vector3(x, y, z);
}

public Vector3 RotateZ(double rad)
{
    return RotateZ(this, rad);
}

public static Vector3 Roll(Vector3 v1, double rad)
{
    return RotateZ(v1, rad);
}

public Vector3 Roll(double rad)
{
    return Roll(this, rad);
}
*/

GaMat4x4 GaMat4x4::adjoint() const
{
	return GaMat4x4
	(
		matrix_minor(*this, 1, 2, 3, 1, 2, 3),
        -matrix_minor(*this, 0, 2, 3, 1, 2, 3),
        matrix_minor(*this, 0, 1, 3, 1, 2, 3),
        -matrix_minor(*this, 0, 1, 2, 1, 2, 3),

        -matrix_minor(*this, 1, 2, 3, 0, 2, 3),
        matrix_minor(*this, 0, 2, 3, 0, 2, 3),
        -matrix_minor(*this, 0, 1, 3, 0, 2, 3),
        matrix_minor(*this, 0, 1, 2, 0, 2, 3),

        matrix_minor(*this, 1, 2, 3, 0, 1, 3),
        -matrix_minor(*this, 0, 2, 3, 0, 1, 3),
        matrix_minor(*this, 0, 1, 3, 0, 1, 3),
        -matrix_minor(*this, 0, 1, 2, 0, 1, 3),

        -matrix_minor(*this, 1, 2, 3, 0, 1, 2),
        matrix_minor(*this, 0, 2, 3, 0, 1, 2),
        -matrix_minor(*this, 0, 1, 3, 0, 1, 2),
        matrix_minor(*this, 0, 1, 2, 0, 1, 2)
	);
}

GaFloat GaMat4x4::determinant() const
{
    return m[0][0] * matrix_minor(*this, 1, 2, 3, 1, 2, 3) -
           m[0][1] * matrix_minor(*this, 1, 2, 3, 0, 2, 3) +
           m[0][2] * matrix_minor(*this, 1, 2, 3, 0, 1, 3) -
	       m[0][3] * matrix_minor(*this, 1, 2, 3, 0, 1, 2);
}

GaMat4x4 GaMat4x4::inverse() const
{
	return adjoint() * (1.0f / determinant());
}

GaQuat::GaQuat(const GaMat3x3 &rot)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	GaFloat fTrace = rot[0][0]+rot[1][1]+rot[2][2];
	GaFloat fRoot;

	if ( fTrace > 0.0f )
	{
		// |w| > 1/2, may as well choose w > 1/2
		fRoot = Math::squareRoot(fTrace + 1.0f);  // 2w
		w = 0.5f*fRoot;
		fRoot = 0.5f/fRoot;  // 1/(4w)
		v.x = (rot[2][1]-rot[1][2])*fRoot;
		v.y = (rot[0][2]-rot[2][0])*fRoot;
		v.z = (rot[1][0]-rot[0][1])*fRoot;
	}
	else
	{
		// |w| <= 1/2
		static GaUint s_iNext[3] = { 1, 2, 0 };
		GaUint i = 0;
		if ( rot[1][1] > rot[0][0] )
			i = 1;
		if ( rot[2][2] > rot[i][i] )
			i = 2;
		GaUint j = s_iNext[i];
		GaUint k = s_iNext[j];

		fRoot = Math::squareRoot(rot[i][i]-rot[j][j]-rot[k][k] + 1.0f);
		GaFloat* apkQuat[3] = { &v.x, &v.y, &v.z };
		*apkQuat[i] = 0.5f*fRoot;
		fRoot = 0.5f/fRoot;
		w = (rot[k][j]-rot[j][k])*fRoot;
		*apkQuat[j] = (rot[j][i]+rot[i][j])*fRoot;
		*apkQuat[k] = (rot[k][i]+rot[i][k])*fRoot;
	}
}

GaQuat GaQuat::inverse() const
{
	GaFloat nrm = w*w + v.x*v.x + v.y*v.y + v.z*v.z;
    if(nrm > 0.0)
    {
		GaFloat inv = 1.0f/nrm;
        return GaQuat(w*inv,-v.x*inv,-v.y*inv,-v.z*inv);
    }
    else
    {
		return GaQuat(0,0,0,0);
    }
}

GaDegree::GaDegree(const GaRadian &a)
{
    d = a.r * Math::GaRadianInDegree;
}

