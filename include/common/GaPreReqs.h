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
#define GANGSTA_DOUBLE

#ifndef GAPREREQS_H
#define GAPREREQS_H

#if defined ( WIN32 )
#   define WIN32_LEAN_AND_MEAN
#   ifndef NOMINMAX
#      define NOMINMAX
#   endif
#   include <windows.h>
#else
#   define USE_LTDL
#endif

#include <assert.h>

#define _USE_MATH_DEFINES 
#include <float.h>
#include <math.h>
#include <limits>

#include <string>
#include <sstream>
#include <vector>
#include <exception>

namespace Ga
{
    #if defined ( WIN32 )
	#   pragma warning (disable : 4251)
	#   pragma warning (disable : 4244)
	#   pragma warning (disable : 4305)
    #   define GaExport
    #else
    #   define GaExport
    #endif

    #if defined ( __GCCWIN32__ )
    #   define GaExpInl
    #else
    #   define GaExpInl inline
    #endif
    
	typedef unsigned int			GaUint;
	typedef unsigned char			GaByte;
	#ifdef GANGSTA_DOUBLE
	typedef double					GaFloat;
	static const GaFloat GaInfinity = DBL_MAX;
	#else
	typedef float					GaFloat;
	static const GaFloat GaInfinity = FLT_MAX;
	#endif

	typedef std::string				GaString;
	typedef std::stringstream		GaStringStream;
	typedef std::vector<GaString>	GaStringList;
	
	class GaVec3;
	class GaQuat;
	class GaMat3x3;
	class GaMat4x4;
	class GaDegree;
	class GaRadian;
	
	class Parameter;
	template<class _Ty> class GaPtr;

	class Manager;
	class DynamicObject;
	class DynamicModule;
	class DebugVisual;
	class CallbackInterface;
	class PhysicsDriver;
	class SubWorld;
	class World;
	class Body;
	class Shape;
	class Joint;
	class Ray;
	class Material;
	class Collision;
}

#endif
