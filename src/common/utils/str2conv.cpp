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

#include "CommonPCH.h"

#include "utils/str2conv.h"

template<>
bool str2< bool >( const char* str )
{
    if( !strcasecmp( str, "true" ) )
	    return true;
    else if( !strcasecmp( str, "false" ) )
	    return false;
    else if( !strcasecmp( str, "yes" ) )
	    return true;
    else if( !strcasecmp( str, "no" ) )
	    return false;
    else if( !strcasecmp( str, "y" ) )
	    return true;
    else if( !strcasecmp( str, "n" ) )
	    return false;
    else if( !strcasecmp( str, "on" ) )
	    return true;
    else if( !strcasecmp( str, "off" ) )
	    return false;
    else if( !strcasecmp( str, "enable" ) )
	    return true;
    else if( !strcasecmp( str, "disable" ) )
	    return false;
    else if( !strcasecmp( str, "enabled" ) )
	    return true;
    else if( !strcasecmp( str, "disabled" ) )
	    return false;
    else if( str2< int >( str ) )
	    return true;
    else
        return false;
}

template<>
int64 str2< int64 >( const char* str )
{
    int64 v = 0;
    sscanf( str, I64d, &v );
    return v;
}

template<>
uint64 str2< uint64 >( const char* str )
{
    uint64 v = 0;
    sscanf( str, I64u, &v );
    return v;
}

template<>
long double str2< long double >( const char* str )
{
    long double v = 0.0;
    sscanf( str, "%Lf", &v );
    return v;
}

