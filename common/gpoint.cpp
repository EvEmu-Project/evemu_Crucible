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


#include "common.h"
#include "gpoint.h"
#include <math.h>

#if 0

GPoint::GPoint() {
}

GPoint::GPoint(double x, double y, double z) {
	pt[0] = x;
	pt[1] = y;
	pt[2] = z;
}

//dot of x,y,z
double GPoint::dot3(const GPoint &them) const {
	return((pt[0] * them.pt[0]) + (pt[1] * them.pt[1]) +
			 (pt[2] * them.pt[2]));
}

//cross product
GPoint GPoint::cross(const GPoint &them) const {
	return(GPoint(pt[1] * them.pt[2] - pt[2] * them.pt[1],
					 pt[2] * them.pt[0] - pt[0] * them.pt[2],
					 pt[0] * them.pt[1] - pt[1] * them.pt[0]));
}

const GPoint &GPoint::operator+=(const GPoint &v) {
	pt[0] += v.pt[0];
	pt[1] += v.pt[1];
	pt[2] += v.pt[2];
	return(*this);
}

const GPoint &GPoint::operator-=(const GPoint &v) {
	pt[0] -= v.pt[0];
	pt[1] -= v.pt[1];
	pt[2] -= v.pt[2];
	return(*this);
}

const GPoint &GPoint::operator*=(const double c) {
	pt[0] *= c;
	pt[1] *= c;
	pt[2] *= c;
	return(*this);
}

const GPoint &GPoint::operator/=(const double c) {
	pt[0] /= c;
	pt[1] /= c;
	pt[2] /= c;
	return(*this);
}

GVector::GVector() {
}

GVector::GVector(const GPoint &them) : GPoint(them) {
}

GVector::GVector(const GPoint &from, const GPoint &to)
: GPoint(
	to.x()-from.x(),
	to.y()-from.y(),
	to.z()-from.z()) {
}

GVector::GVector(double x, double y, double z) : GPoint(x, y, z) {
}

double GVector::length() const {
	double len2 = (pt[0] * pt[0]) + (pt[1] * pt[1]) + (pt[2] * pt[2]);
	if(len2 == 0)
		return(0);
	return(sqrt(len2));
}

double GVector::length2() const {
	return((pt[0] * pt[0]) + (pt[1] * pt[1]) + (pt[2] * pt[2]));
}

void GVector::normalize() {
	double len = length();	//stupid square roots take forever
	if(len == 0) {
		pt[0] = 0;
		pt[1] = 0;
		pt[2] = 0;
	} else {
		pt[0] /= len;
		pt[1] /= len;
		pt[2] /= len;
	}
}

double GVector::normalize_getlen() {
	double len = length();	//stupid square roots take forever
	if(len == 0) {
		pt[0] = 0;
		pt[1] = 0;
		pt[2] = 0;
	} else {
		pt[0] /= len;
		pt[1] /= len;
		pt[2] /= len;
	}
	return(len);
}

GVector4::GVector4(const GPoint &them) : GVector(them) {
	W = 1.0f;
}

GVector4::GVector4(double x, double y, double z, double w) : GVector(x, y, z) {
	W = w;
}

//dot product of x,y,z,w
double GVector4::dot4(const GVector4 &them) const {
	return((pt[0] * them.pt[0]) + (pt[1] * them.pt[1]) +
			 (pt[2] * them.pt[2]) + (W * them.W));
}

//dot product of x,y,z+w
double GVector4::dot4(const GPoint &them) const {
	return((pt[0] * them.pt[0]) + (pt[1] * them.pt[1]) +
			 (pt[2] * them.pt[2]) + W);
}

GPoint operator+(const GPoint &v1, const GPoint &v2) {
	return(GPoint(v1.pt[0] + v2.pt[0], v1.pt[1] + v2.pt[1], v1.pt[2] + v2.pt[2]));
}

GPoint operator-(const GPoint &v1, const GPoint &v2) {
	return(GPoint(v1.pt[0] - v2.pt[0], v1.pt[1] - v2.pt[1], v1.pt[2] - v2.pt[2]));
}

GPoint operator*(const GPoint &v1, const double c) {
	return(GPoint(v1.pt[0] * c, v1.pt[1] * c, v1.pt[2] * c));
}

GPoint operator/(const GPoint &v1, const double c) {
	return(GPoint(v1.pt[0] / c, v1.pt[1] / c, v1.pt[2] / c));
}

GPoint operator*(const double c, const GPoint &v1) {
	return(GPoint(v1.pt[0] * c, v1.pt[1] * c, v1.pt[2] * c));
}

GPoint operator/(const double c, const GPoint &v1) {
	return(GPoint(v1.pt[0] / c, v1.pt[1] / c, v1.pt[2] / c));
}

G2Point::G2Point() {
}

G2Point::G2Point(double x, double y) {
	pt[0] = x;
	pt[1] = y;
}

G2iPoint::G2iPoint() {
}

G2iPoint::G2iPoint(int nx, int ny) {
	x = nx;
	y = ny;
}



#endif


