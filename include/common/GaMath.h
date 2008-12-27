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
#ifndef GAMATH_H
#define GAMATH_H

#include "GaPreReqs.h"

namespace Ga
{
	class Math
	{
	public:
		static const GaExport GaFloat GaPi;
		static const GaExport GaFloat GaDegreeToRadian;
		static const GaExport GaFloat GaRadianToDegree;
		static const GaExport GaFloat GaEpsilon;
		static const GaExport GaFloat GaMinFloat;
		static const GaExport GaFloat GaMaxFloat;

#ifdef GANGSTA_DOUBLE
		static inline GaFloat squareRoot(GaFloat f){return sqrt(f);}
		static inline GaFloat arcCosine(const GaFloat &r){return acos(r);}
		static inline GaFloat cosine(const GaFloat &r){return cos(r);}
		static inline GaFloat sine(const GaFloat &r){return sin(r);}
		static inline GaFloat absolute(GaFloat f){return fabs(f);}
#else
		static inline GaFloat squareRoot(GaFloat f){return sqrtf(f);}
		static inline GaFloat arcCosine(const GaFloat &r){return acosf(r);}
		static inline GaFloat cosine(const GaFloat &r){return cosf(r);}
		static inline GaFloat sine(const GaFloat &r){return sinf(r);}
		static inline GaFloat absolute(GaFloat f){return fabs(f);}
#endif
	};
}

#endif
