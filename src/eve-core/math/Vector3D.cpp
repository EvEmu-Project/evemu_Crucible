/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     eve-moo
 */

#include "Vector3D.h"

double Vector3D::magnitude() const
{
    double m;

    //calculate the length of the Vector3D
    m = (x * x) + (y * y) + (z * z);
    m = sqrt(m);

    return m;
}

void Vector3D::normalize()
{
    double m;

    //calculate the length of the Vector3D
    m = (x * x) + (y * y) + (z * z);
    m = sqrt(m);

    // if the length is zero then the Vector3D is zero so return
    if (m == 0)
    {
        return;
    }

    //Scale the Vector3D to a unit length
    x /= m;
    y /= m;
    z /= m;
}

double Vector3D::dotProduct(const Vector3D &v) const
{
    return ((x * v.x) + (y * v.y) + (z * v.z));
}

Vector3D Vector3D::crossProduct(const Vector3D &v) const
{
    double nx = y * v.z - z * v.y;
    double ny = z * v.x - x * v.z;
    double nz = x * v.y - y * v.x;
    return Vector3D(nx, ny, nz);
}

Vector3D Vector3D::reflection(Vector3D norm) const
{
    Vector3D refl;
    refl = norm * (-2 * dotProduct(norm));
    refl = (*this) - refl;
    return refl;
}

Vector3D Vector3D::refraction(Vector3D norm, double fact) const
{
    Vector3D refr;
    double n_r = this->dotProduct(norm);
    double k = 1 - (n_r * n_r);
    k = 1 - (fact * fact) * k;
    if (k < 0)
    {
        return refr;
    }
    refr = (*this) * fact;
    refr -= norm * (fact * n_r + sqrt(k));
    return refr;
}

void Vector3D::set(double nx, double ny, double nz)
{
    x = nx;
    y = ny;
    z = nz;
}

void Vector3D::set(const Vector3D &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

Vector3D& Vector3D::operator=(const Vector3D &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector3D Vector3D::operator+(const Vector3D &v) const
{
    return Vector3D(*this) += v;
}

Vector3D& Vector3D::operator+=(const Vector3D &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector3D Vector3D::operator-(const Vector3D &v) const
{
    return Vector3D(*this) -= v;
}

Vector3D& Vector3D::operator-=(const Vector3D &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector3D Vector3D::operator*(const Vector3D &v) const
{
    return Vector3D(*this) *= v;
}

Vector3D& Vector3D::operator*=(const Vector3D &v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

Vector3D Vector3D::operator/(const Vector3D &v) const
{
    return Vector3D(*this) /= v;
}

Vector3D& Vector3D::operator/=(const Vector3D &v)
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

Vector3D Vector3D::operator*(const double &k) const
{
    return Vector3D(*this) *= k;
}

Vector3D& Vector3D::operator*=(const double &k)
{
    x *= k;
    y *= k;
    z *= k;
    return *this;
}

Vector3D Vector3D::operator/(const double &k) const
{
    return Vector3D(*this) /= k;
}

Vector3D& Vector3D::operator/=(const double &k)
{
    x /= k;
    y /= k;
    z /= k;
    return *this;
}
