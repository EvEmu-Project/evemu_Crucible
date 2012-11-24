/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Aim, Captnoord, Zhur, Bloody.Rabbit
*/

#ifndef __UTILS__FAST_INT_H__INCL__
#define __UTILS__FAST_INT_H__INCL__

// fast int abs
inline int32 int32abs( const int32 value )
{
    return (value ^ (value >> 31)) - (value >> 31);
}

// fast int abs and recast to unsigned
inline uint32 int32abs2uint32( const int32 value )
{
    return (uint32)(value ^ (value >> 31)) - (value >> 31);
}

/// Fastest Method of float2int32
inline int32 float2int32( const float value )
{
#ifdef HAVE___ASM
    int32 i;
    __asm {
        fld value
        frndint
        fistp i
    }
    return i;
#else /* !HAVE___ASM */
    union { int32 asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

    return n.asInt[0];
#endif /* !HAVE___ASM */
}

/// Fastest Method of double2int32
inline int32 double2int32( const double value )
{
#ifdef HAVE___ASM
    int32 i;
    __asm {
        fld value
        frndint
        fistp i
    }
    return i;
#else /* !HAVE___ASM */
  union { int32 asInt[2]; double asDouble; } n;
  n.asDouble = value + 6755399441055744.0;

  return n.asInt[0];
#endif /* !HAVE___ASM */
}

#endif /* !__UTILS__FAST_INT_H__INCL__ */
