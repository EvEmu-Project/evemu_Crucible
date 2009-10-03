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

GaVec3 GaVec3::ZERO = GaVec3(0.0);
GaVec3 GaVec3::ONE = GaVec3(1.0);
GaVec3 GaVec3::UNIT_X = GaVec3(1,0,0);
GaVec3 GaVec3::UNIT_Y = GaVec3(0,1,0);
GaVec3 GaVec3::UNIT_Z = GaVec3(0,0,1);

GaQuat GaQuat::IDENTITY = GaQuat(1,0,0,0);
GaQuat GaQuat::ZERO = GaQuat(0,0,0,0);

GaQuat GaVec3::rotationTo(const GaVec3 &oth) const
{
	GaQuat q;
    GaVec3 v0 = *this;
	GaVec3 v1 = oth;

	v0.normalize();
    v1.normalize();

	GaVec3 c = v0.crossProduct(v1);
	GaFloat d = v0.dotProduct(v1);

	if (d >= 1.0f)
	{
		q = GaQuat::IDENTITY;
	}
	else
	{
		GaFloat s = Math::squareRoot( (1.0f+d)*2.0f );
		assert (s != 0.0f && "Divide by zero!");
		GaFloat invs = 1.0f / s;

		q.v.x = c.x * invs;
		q.v.y = c.y * invs;
		q.v.z = c.z * invs;
		q.w = s * 0.5f;
	}
	return q;
}

GaQuat::GaQuat(const GaRadian &a,const GaVec3 &axis)
{
	GaFloat sin_a = Math::sine(a.r * 0.5);
	GaFloat cos_a = Math::cosine(a.r * 0.5);

	w = cos_a;

	v.x = axis.x * sin_a;
	v.y = axis.y * sin_a;
	v.z = axis.z * sin_a;
}

GaFloat matrix_minor(const GaMat4x4 &m,GaUint r0,GaUint r1,GaUint r2,GaUint c0,GaUint c1,GaUint c2)
{
	return m[r0][c0] * (m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2]) -
           m[r0][c1] * (m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2]) +
           m[r0][c2] * (m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1]);
}

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
	d = a.r * Math::GaDegreeToRadian;
}

