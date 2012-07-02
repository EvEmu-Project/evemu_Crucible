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
    Author:     Bloody.Rabbit
*/

#include "eve-core.h"

#include "utils/str2conv.h"

template<>
bool str2< bool >( const char* str )
{
    if( !::strcasecmp( str, "true" ) )
        return true;
    else if( !::strcasecmp( str, "false" ) )
        return false;
    else if( !::strcasecmp( str, "yes" ) )
        return true;
    else if( !::strcasecmp( str, "no" ) )
        return false;
    else if( !::strcasecmp( str, "y" ) )
        return true;
    else if( !::strcasecmp( str, "n" ) )
        return false;
    else if( !::strcasecmp( str, "on" ) )
        return true;
    else if( !::strcasecmp( str, "off" ) )
        return false;
    else if( !::strcasecmp( str, "enable" ) )
        return true;
    else if( !::strcasecmp( str, "disable" ) )
        return false;
    else if( !::strcasecmp( str, "enabled" ) )
        return true;
    else if( !::strcasecmp( str, "disabled" ) )
        return false;
    else if( str2< int >( str ) )
        return true;
    else
        return false;
}

#define STR2( type, fmt )                       \
    template<>                                  \
    type str2< type >( const char* str )        \
    {                                           \
        type v = 0;                             \
        ::sscanf( str, "%" fmt, &v );           \
        return v;                               \
    }

STR2( int8,  SCNd8 )
STR2( int16, SCNd16 )
STR2( int32, SCNd32 )
STR2( int64, SCNd64 )

STR2( uint8,  SCNu8 )
STR2( uint16, SCNu16 )
STR2( uint32, SCNu32 )
STR2( uint64, SCNu64 )

STR2( float, "f" )
STR2( double, "lf" )
STR2( long double, "Lf" )

#undef STR2
