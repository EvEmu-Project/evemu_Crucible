/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef G_POINT_H
#define G_POINT_H

#include "common.h"

#include "GaTypes.h"

//typedef Ga::GaVec3 GPoint;
class GPoint : public Ga::GaVec3 {
public:
	 GPoint():Ga::GaVec3(){}
	 GPoint(Ga::GaFloat v):Ga::GaVec3(v){}
	 GPoint(const Ga::GaFloat *v):Ga::GaVec3(v){}
	 GPoint(Ga::GaFloat X,Ga::GaFloat Y,Ga::GaFloat Z):Ga::GaVec3(X, Y, Z){}
	 GPoint(const GPoint& oth):Ga::GaVec3(oth){}
	 GPoint(const Ga::GaVec3& oth):Ga::GaVec3(oth){}
};
		
class GVector : public Ga::GaVec3 {
public:
	GVector():Ga::GaVec3(){}
	GVector(Ga::GaFloat v):Ga::GaVec3(v){}
	GVector(const Ga::GaFloat *v):Ga::GaVec3(v){}
	GVector(Ga::GaFloat X,Ga::GaFloat Y,Ga::GaFloat Z):Ga::GaVec3(X, Y, Z){}
	GVector(const GPoint& oth):Ga::GaVec3(oth){}
	GVector(const Ga::GaVec3& oth):Ga::GaVec3(oth){}
	GVector(const GPoint& from, const GPoint& to)
		: Ga::GaVec3( (to.x-from.x), (to.y-from.y), (to.z-from.z) ) {}
};

#if 0
//TODO: inline most of this crap.

class GPoint {
public:
	GPoint();
	GPoint(double x, double y, double z);
	
	inline void operator()(double nx, double ny, double nz) { pt[0] = nx; pt[1] = ny; pt[2] = nz; }

	inline const double x() const { return(pt[0]); }
	inline const double y() const { return(pt[1]); }
	inline const double z() const { return(pt[2]); }
	
	GPoint cross(const GPoint &them) const;
	double dot3(const GPoint &them) const;
	
	const GPoint &operator+=(const GPoint &v2);
	const GPoint &operator-=(const GPoint &v2);
	const GPoint &operator*=(const double c);
	const GPoint &operator/=(const double c);

	double pt[3];

};

GPoint operator+(const GPoint &v1, const GPoint &v2);
GPoint operator-(const GPoint &v1, const GPoint &v2);
GPoint operator*(const GPoint &v1, const double c);
GPoint operator/(const GPoint &v1, const double c);
GPoint operator*(const double c, const GPoint &v1);
GPoint operator/(const double c, const GPoint &v1);

class GVector : public GPoint {
public:
	GVector();
	GVector(const GPoint &them);
	GVector(const GPoint &from, const GPoint &to);
	GVector(double x, double y, double z);
	
	void normalize();
	double normalize_getlen();	//normalize and return the calculated length while your at it
	double length() const;
	double length2() const;	//length squared
};

class GVector4 : public GVector {
public:
	GVector4();
	GVector4(const GPoint &them);
	GVector4(double x, double y, double z, double w = 1.0f);
	
	inline void operator()(double nx, double ny, double nz, double nw) { pt[0] = nx; pt[1] = ny; pt[2] = nz; W = nw; }
	double dot4(const GVector4 &them) const;
	double dot4(const GPoint &them) const;
	
	inline const double w() const { return(W); }
	
	double W;
};

class G2Point {
public:
	G2Point();
	G2Point(double x, double y);
	
	inline void operator()(double nx, double ny) { pt[0] = nx; pt[1] = ny; }

	inline const double x() const { return(pt[0]); }
	inline const double y() const { return(pt[1]); }
	
	double pt[2];
};

class G2iPoint {
public:
	G2iPoint();
	G2iPoint(int nx, int ny);
	
	inline void operator()(int nx, int ny) { x = nx; y = ny; }
	
	int x;
	int y;
};

inline bool operator==(const G2iPoint &pt1, const G2iPoint &pt2) {
	return(pt1.x == pt2.x && pt1.y == pt2.y);
}

inline bool operator!=(const G2iPoint &pt1, const G2iPoint &pt2) {
	return(pt1.x != pt2.x || pt1.y != pt2.y);
}

#endif

#endif
