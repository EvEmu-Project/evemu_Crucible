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
#ifndef GATYPES_H
#define GATYPES_H

#include "GaPreReqs.h"
#include "GaMath.h"

namespace Ga
{
	class GaException {
		public:
			GaException(){}
			GaException(const char *msg) { m_msg = msg; }
			GaException(const std::string &msg) { m_msg = msg; }
			const std::string &Description() { return m_msg; }
		private:
			std::string m_msg;
	};

	class GaExport GaVec3
	{
	public:
		static GaVec3		ZERO;
		static GaVec3		ONE;
		static GaVec3		UNIT_X;
		static GaVec3		UNIT_Y;
		static GaVec3		UNIT_Z;
		
		GaExpInl GaVec3():x(0.0f),y(0.0f),z(0.0f){}
		GaExpInl GaVec3(GaFloat v):x(v),y(v),z(v){}
		GaExpInl GaVec3(const GaFloat *v):x(v[0]),y(v[1]),z(v[2]){}
		GaExpInl GaVec3(GaFloat X,GaFloat Y,GaFloat Z):x(X),y(Y),z(Z){}
		GaExpInl GaVec3(const GaVec3& oth):x(oth.x),y(oth.y),z(oth.z){}
		
        GaExpInl GaVec3 operator-(const GaVec3 &oth) const
        {
			return GaVec3(x - oth.x,y - oth.y,z - oth.z);
		}

		GaExpInl GaVec3 operator*(GaFloat s) const
		{
			return GaVec3(x * s,y * s,z * s);
		}

		GaExpInl GaVec3 operator/(GaFloat s) const
		{
			return GaVec3(x / s,y / s,z / s);
		}

		GaExpInl GaVec3 operator*(const GaVec3 &s) const
		{
			return GaVec3(x * s.x,y * s.y,z * s.z);
		}

		GaExpInl GaVec3& operator=(const GaVec3& oth)
        {
            x = oth.x;
            y = oth.y;
            z = oth.z;            

			return *this;
		}

		GaExpInl bool operator!=(const GaVec3& oth) const
        {
            return (x != oth.x) || (y != oth.y) || (z != oth.z);            
		}

		GaExpInl bool operator==(const GaVec3& oth) const
        {
            return (x == oth.x) && (y == oth.y) && (z == oth.z);            
		}

		GaExpInl GaVec3 operator+(const GaVec3 &oth) const
		{
			return GaVec3(x + oth.x,y + oth.y,z + oth.z);
		}

		GaExpInl GaVec3& operator+=(const GaVec3 &oth)
		{
			x += oth.x;
			y += oth.y;
			z += oth.z;

			return *this;
		}
 
		GaExpInl GaVec3& operator-=(const GaVec3 &oth)
		{
			x -= oth.x;
			y -= oth.y;
			z -= oth.z;

			return *this;
		}
 
		GaExpInl GaVec3& operator*=(GaFloat s)
		{
			x *= s;
			y *= s;
			z *= s;

			return *this;
		}
 
		GaExpInl GaVec3& operator/=(GaFloat s)
		{
			x /= s;
			y /= s;
			z /= s;

			return *this;
		}
 
        GaExpInl GaVec3& operator*=(const GaVec3& vec)
		{
			x *= vec.x;
			y *= vec.y;
			z *= vec.z;
 
            return *this;
        }

		GaExpInl GaVec3 crossProduct(const GaVec3 &oth) const
		{
			return GaVec3
			(
				(y * oth.z) - (oth.y * z),
				(z * oth.x) - (oth.z * x),		
				(x * oth.y) - (oth.x * y)
			);
		}

		GaExpInl GaFloat dotProduct(const GaVec3 &oth) const
		{
			return x * oth.x + y * oth.y + z * oth.z;
		}

		GaExpInl GaFloat lengthSquared() const{return (x * x) + (y * y) + (z * z);}
		GaExpInl GaFloat length() const {return Math::squareRoot(lengthSquared());}

		GaExpInl GaFloat normalize()
		{
			GaFloat len = length();
			if(len > Math::GaEpsilon)
			{
				GaFloat inv = 1.0 / len;

				x *= inv;
				y *= inv;
				z *= inv;
			}
			return(len);	//just in case they want it
		}

		//optimized checks for a very common case
		bool isZero() const {
			return(x == 0.0f && y == 0.0f && z == 0.0f);
		}
		bool isNotZero() const {
			return(x != 0.0f || y != 0.0f || z != 0.0f);
		}

        GaExpInl GaFloat operator[](GaUint i)
        {
			assert(i < 3 && "GaVec3: subscript out of range");
			return m[i];
		}

		GaQuat rotationTo(const GaVec3 &oth) const;

		GaVec3 &operator=(const Parameter &oth);
		GaVec3(const Parameter &oth);

		union
		{
			struct
			{
				GaFloat x,y,z;        
			};
			GaFloat m[3];
		};
	};
	
	//commutative.
	GaExpInl GaVec3 operator*(const GaFloat c, const GaVec3 &v1) {
		return(v1 * c);
	}
	GaExpInl GaVec3 operator/(const GaFloat c, const GaVec3 &v1) {
		return(v1 / c);
	}
	
	class GaExport GaQuat
	{
	public:
		static GaQuat		IDENTITY;
		static GaQuat		ZERO;
		
		GaExpInl GaQuat():w(0),v(0,0,0){}
		GaExpInl GaQuat(GaFloat W,const GaVec3 &V):w(W),v(V){}
		GaExpInl GaQuat(GaFloat W,GaFloat X,GaFloat Y,GaFloat Z):w(W),v(X,Y,Z){}
		
		GaQuat(const GaRadian &a,const GaVec3 &axis);
		GaQuat(const GaMat3x3 &rot);

		GaExpInl GaVec3 operator*(const GaVec3 &vec) const
		{
			GaVec3 uv = v.crossProduct(vec);
			GaVec3 uuv = v.crossProduct(uv); 
	
			uv *= (2.0f * w); 
			uuv *= 2.0f; 
		
			return GaVec3(vec.x + uv.x + uuv.x,vec.y + uv.y + uuv.y,vec.z + uv.z + uuv.z);
		}

		GaExpInl GaQuat operator*(const GaQuat &oth) const
		{
			return GaQuat
			(
				w * oth.w   - v.x * oth.v.x - v.y * oth.v.y - v.z * oth.v.z,
				w * oth.v.x + v.x * oth.w   + v.y * oth.v.z - v.z * oth.v.y,
				w * oth.v.y + v.y * oth.w   + v.z * oth.v.x - v.x * oth.v.z,
				w * oth.v.z + v.z * oth.w   + v.x * oth.v.y - v.y * oth.v.x
			);
		}

		GaExpInl GaQuat& operator=(const GaQuat& oth)
        {
			w = oth.w;
            v = oth.v;           

			return *this;
		}

		GaQuat &operator=(const Parameter &oth);
		GaQuat(const Parameter &oth);

		GaQuat inverse() const;

		GaFloat w;
		GaVec3 v;
	};

	class GaExport GaMat3x3
	{
	public:
		GaExpInl GaMat3x3(){}

		GaExpInl GaMat3x3(const GaFloat *v)
		{
			m[0][0] = v[0]; m[0][1] = v[1]; m[0][2] = v[2]; 
			m[1][0] = v[3]; m[1][1] = v[4]; m[1][2] = v[5]; 
			m[2][0] = v[6]; m[2][1] = v[7]; m[2][2] = v[8]; 
		}

		GaExpInl GaMat3x3(GaFloat m00,GaFloat m01,GaFloat m02,
			            GaFloat m10,GaFloat m11,GaFloat m12,
				        GaFloat m20,GaFloat m21,GaFloat m22)
		{
			m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
			m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
			m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
		}
		
		GaExpInl GaMat3x3(const GaQuat &q)
		{
			GaFloat fTx  = 2.0*q.v.x;
			GaFloat fTy  = 2.0*q.v.y;
			GaFloat fTz  = 2.0*q.v.z;
			GaFloat fTwx = fTx*q.w;
			GaFloat fTwy = fTy*q.w;
			GaFloat fTwz = fTz*q.w;
			GaFloat fTxx = fTx*q.v.x;
			GaFloat fTxy = fTy*q.v.x;
			GaFloat fTxz = fTz*q.v.x;
			GaFloat fTyy = fTy*q.v.y;
			GaFloat fTyz = fTz*q.v.y;
			GaFloat fTzz = fTz*q.v.z;

			m[0][0] = 1.0-(fTyy+fTzz);
			m[0][1] = fTxy-fTwz;
			m[0][2] = fTxz+fTwy;
			m[1][0] = fTxy+fTwz;
			m[1][1] = 1.0-(fTxx+fTzz);
			m[1][2] = fTyz-fTwx;
			m[2][0] = fTxz-fTwy;
			m[2][1] = fTyz+fTwx;
			m[2][2] = 1.0-(fTxx+fTyy);
		}

        GaExpInl GaFloat *operator[](GaUint i)
        {
			assert(i < 3 && "GaMat3x3: subscript out of range");
			return m[i];
		}
 
        const GaExpInl GaFloat *const operator[](GaUint i) const
        {
			assert(i < 3 && "GaMat3x3: subscript out of range");
			return m[i];
		}
 
		GaFloat m[3][3];
	};

	class GaExport GaMat4x4
	{
	public:
		GaExpInl GaMat4x4(){}

		GaExpInl GaMat4x4(const GaFloat *v)
		{
			m[0][0] = v[ 0]; m[0][1] = v[ 1]; m[0][2] = v[ 2]; m[0][3] = v[ 3];
			m[1][0] = v[ 4]; m[1][1] = v[ 5]; m[1][2] = v[ 6]; m[1][3] = v[ 7];
			m[2][0] = v[ 8]; m[2][1] = v[ 9]; m[2][2] = v[10]; m[2][3] = v[11];
			m[3][0] = v[12]; m[3][1] = v[13]; m[3][2] = v[14]; m[3][3] = v[15];
		}

		GaExpInl GaMat4x4(GaFloat m00,GaFloat m01,GaFloat m02,GaFloat m03,
			            GaFloat m10,GaFloat m11,GaFloat m12,GaFloat m13,
				        GaFloat m20,GaFloat m21,GaFloat m22,GaFloat m23,
				        GaFloat m30,GaFloat m31,GaFloat m32,GaFloat m33)
		{
			m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
			m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
			m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
			m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
		}

		GaExpInl GaMat4x4(const GaVec3 &p,const GaQuat &q)
		{
			GaMat3x3 rot(q);

			m[0][0] = rot[0][0]; m[0][1] = rot[0][1]; m[0][2] = rot[0][2]; m[0][3] = p.x;
 			m[1][0] = rot[1][0]; m[1][1] = rot[1][1]; m[1][2] = rot[1][2]; m[1][3] = p.y;		
			m[2][0] = rot[2][0]; m[2][1] = rot[2][1]; m[2][2] = rot[2][2]; m[2][3] = p.z;
			m[3][0] = 0.0;       m[3][1] = 0.0;       m[3][2] = 0.0;       m[3][3] = 1.0;
		}

		GaExpInl GaMat4x4 operator*(const GaMat4x4 &oth) const
		{
			GaMat4x4 o;

			o.m[0][0] = m[0][0] * oth.m[0][0] + m[0][1] * oth.m[1][0] + m[0][2] * oth.m[2][0] + m[0][3] * oth.m[3][0];
			o.m[0][1] = m[0][0] * oth.m[0][1] + m[0][1] * oth.m[1][1] + m[0][2] * oth.m[2][1] + m[0][3] * oth.m[3][1];
			o.m[0][2] = m[0][0] * oth.m[0][2] + m[0][1] * oth.m[1][2] + m[0][2] * oth.m[2][2] + m[0][3] * oth.m[3][2];
			o.m[0][3] = m[0][0] * oth.m[0][3] + m[0][1] * oth.m[1][3] + m[0][2] * oth.m[2][3] + m[0][3] * oth.m[3][3];

			o.m[1][0] = m[1][0] * oth.m[0][0] + m[1][1] * oth.m[1][0] + m[1][2] * oth.m[2][0] + m[1][3] * oth.m[3][0];
			o.m[1][1] = m[1][0] * oth.m[0][1] + m[1][1] * oth.m[1][1] + m[1][2] * oth.m[2][1] + m[1][3] * oth.m[3][1];
			o.m[1][2] = m[1][0] * oth.m[0][2] + m[1][1] * oth.m[1][2] + m[1][2] * oth.m[2][2] + m[1][3] * oth.m[3][2];
			o.m[1][3] = m[1][0] * oth.m[0][3] + m[1][1] * oth.m[1][3] + m[1][2] * oth.m[2][3] + m[1][3] * oth.m[3][3];

			o.m[2][0] = m[2][0] * oth.m[0][0] + m[2][1] * oth.m[1][0] + m[2][2] * oth.m[2][0] + m[2][3] * oth.m[3][0];
			o.m[2][1] = m[2][0] * oth.m[0][1] + m[2][1] * oth.m[1][1] + m[2][2] * oth.m[2][1] + m[2][3] * oth.m[3][1];
			o.m[2][2] = m[2][0] * oth.m[0][2] + m[2][1] * oth.m[1][2] + m[2][2] * oth.m[2][2] + m[2][3] * oth.m[3][2];
			o.m[2][3] = m[2][0] * oth.m[0][3] + m[2][1] * oth.m[1][3] + m[2][2] * oth.m[2][3] + m[2][3] * oth.m[3][3];

			o.m[3][0] = m[3][0] * oth.m[0][0] + m[3][1] * oth.m[1][0] + m[3][2] * oth.m[2][0] + m[3][3] * oth.m[3][0];
			o.m[3][1] = m[3][0] * oth.m[0][1] + m[3][1] * oth.m[1][1] + m[3][2] * oth.m[2][1] + m[3][3] * oth.m[3][1];
			o.m[3][2] = m[3][0] * oth.m[0][2] + m[3][1] * oth.m[1][2] + m[3][2] * oth.m[2][2] + m[3][3] * oth.m[3][2];
			o.m[3][3] = m[3][0] * oth.m[0][3] + m[3][1] * oth.m[1][3] + m[3][2] * oth.m[2][3] + m[3][3] * oth.m[3][3];

			return o;
		}

        GaExpInl GaFloat *operator[](GaUint i)
        {
			assert(i < 4 && "GaMax4x4: subscript out of range");
			return m[i];
		}
 
		GaExpInl const GaFloat *const operator[](GaUint i) const
		{
			assert(i < 4 && "GaMax4x4: subscript out of range");
			return m[i];
		}

		GaExpInl GaVec3 operator*(const GaVec3 &v) const
		{
			GaFloat w = 1.0f / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );
			
			GaVec3 r(
				( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * w,
				( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * w,
				( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * w );

			return r;
		}

		GaExpInl void toFloatMatrix(float *o)
		{
			o[0] = m[0][0]; o[4] = m[0][1];  o[8] = m[0][2]; o[12] = m[0][3];
			o[1] = m[1][0]; o[5] = m[1][1];  o[9] = m[1][2]; o[13] = m[1][3];
			o[2] = m[2][0]; o[6] = m[2][1]; o[10] = m[2][2]; o[14] = m[2][3];
			o[3] = m[3][0]; o[7] = m[3][1]; o[11] = m[3][2]; o[15] = m[3][3];
		}

		GaExpInl GaMat4x4 operator*(GaFloat s)
		{
			return GaMat4x4
			(
				s*m[0][0], s*m[0][1], s*m[0][2], s*m[0][3],
                s*m[1][0], s*m[1][1], s*m[1][2], s*m[1][3],
                s*m[2][0], s*m[2][1], s*m[2][2], s*m[2][3],
                s*m[3][0], s*m[3][1], s*m[3][2], s*m[3][3]
			);
		}

		GaMat4x4 inverse() const;
		GaMat4x4 adjoint() const;
		GaFloat determinant() const;

		GaFloat m[4][4];
	};
	
	class GaExport GaDegree
	{
	public:
		GaExpInl GaDegree(){d = 0;}
		GaExpInl GaDegree(GaFloat a){d = a;}
		GaDegree(const GaRadian &a);

		GaFloat d;
	};

	class GaExport GaRadian
	{
	public:
		GaExpInl GaRadian(){r = 0;}
		GaExpInl GaRadian(GaFloat a){r = a;}
		GaExpInl GaRadian(const GaDegree &a){r = a.d * Math::GaRadianToDegree;}

		GaFloat r;
	};
}

#endif
