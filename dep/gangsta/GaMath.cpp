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
#include "GaMath.h"

using namespace Ga;

const GaFloat Math::GaPi = 3.1415926535897932384626433832795;
const GaFloat Math::GaDegreeToRadian = 180.0 / GaPi;
const GaFloat Math::GaRadianToDegree = GaPi / 180.0;
const GaFloat Math::GaEpsilon = FLT_EPSILON;
const GaFloat Math::GaMinFloat = FLT_MIN;
const GaFloat Math::GaMaxFloat = FLT_MAX;
