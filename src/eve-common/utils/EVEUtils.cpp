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
    Author:     Zhur
*/

#include "EVECommonPCH.h"

#include "python/classes/PyExceptions.h"
#include "utils/EVEUtils.h"

UserError *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args)
{
	UserError *err = new UserError( exceptionType );

	std::map<std::string, PyRep *>::const_iterator cur, end;
	cur = args.begin();
	end = args.end();
	for(; cur != end; cur++)
		err->AddKeyword( cur->first.c_str(), cur->second );

	return err;
}

UserError* MakeCustomError( const char* fmt, ... )
{
    va_list va;
    va_start( va, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, va );
    assert( str );

    va_end( va );

	UserError* err = new UserError( "CustomError" );
	err->AddKeyword( "error", new PyString( str ) );

    SafeFree( str );

    return err;
}

bool IsPrintable( const PyString* str )
{
    return IsPrintable( str->content() );
}

bool IsPrintable( const PyWString* str )
{
    // how to do it correctly?
    return IsPrintable( str->content() );
}

bool DBTYPE_IsCompatible( DBTYPE type, const PyRep* rep )
{
// Helper macro, checks type and range
#define CheckTypeRangeUnsigned( type, lower_bound, upper_bound ) \
    ( rep->Is##type() && (uint64)rep->As##type()->value() >= lower_bound && (uint64)rep->As##type()->value() <= upper_bound )
#define CheckTypeRange( type, lower_bound, upper_bound ) \
    ( rep->Is##type() && rep->As##type()->value() >= lower_bound && rep->As##type()->value() <= upper_bound )

    if( rep->IsNone() )
        // represents NULL
        return true;

    switch( type )
    {
        case DBTYPE_UI8:
        case DBTYPE_CY:
        case DBTYPE_FILETIME:
            return (   CheckTypeRangeUnsigned( Int, 0LL, 0xFFFFFFFFFFFFFFFFLL )
                    || CheckTypeRangeUnsigned( Long, 0LL, 0xFFFFFFFFFFFFFFFFLL )
                    || CheckTypeRangeUnsigned( Float, 0LL, 0xFFFFFFFFFFFFFFFFLL ) );
        case DBTYPE_UI4:
            return (   CheckTypeRangeUnsigned( Int, 0L, 0xFFFFFFFFL )
                    || CheckTypeRangeUnsigned( Long, 0L, 0xFFFFFFFFL )
                    || CheckTypeRangeUnsigned( Float, 0L, 0xFFFFFFFFL ) );
        case DBTYPE_UI2:
            return (   CheckTypeRangeUnsigned( Int, 0, 0xFFFF )
                    || CheckTypeRangeUnsigned( Long, 0, 0xFFFF )
                    || CheckTypeRangeUnsigned( Float, 0, 0xFFFF ) );
        case DBTYPE_UI1:
            return (   CheckTypeRangeUnsigned( Int, 0, 0xFF )
                    || CheckTypeRangeUnsigned( Long, 0, 0xFF )
                    || CheckTypeRangeUnsigned( Float, 0, 0xFF ) );

        case DBTYPE_I8:
            return (   CheckTypeRange( Int, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL )
                    || CheckTypeRange( Long, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL )
                    || CheckTypeRange( Float, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL ) );
        case DBTYPE_I4:
            return (   CheckTypeRange( Int, -0x7FFFFFFFL, 0x7FFFFFFFL )
                    || CheckTypeRange( Long, -0x7FFFFFFFL, 0x7FFFFFFFL )
                    || CheckTypeRange( Float, -0x7FFFFFFFL, 0x7FFFFFFFL ) );
        case DBTYPE_I2:
            return (   CheckTypeRange( Int, -0x7FFF, 0x7FFF )
                    || CheckTypeRange( Long, -0x7FFF, 0x7FFF )
                    || CheckTypeRange( Float, -0x7FFF, 0x7FFF ) );
        case DBTYPE_I1:
            return (   CheckTypeRange( Int, -0x7F, 0x7F )
                    || CheckTypeRange( Long, -0x7F, 0x7F )
                    || CheckTypeRange( Float, -0x7F, 0x7F ) );

        case DBTYPE_R8:
            return (   CheckTypeRange( Int, -DBL_MAX, DBL_MAX )
                    || CheckTypeRange( Long, -DBL_MAX, DBL_MAX )
                    || CheckTypeRange( Float, -DBL_MAX, DBL_MAX ) );
        case DBTYPE_R4:
            return (   CheckTypeRange( Int, -FLT_MAX, FLT_MAX )
                    || CheckTypeRange( Long, -FLT_MAX, FLT_MAX )
                    || CheckTypeRange( Float, -FLT_MAX, FLT_MAX ) );

        case DBTYPE_BOOL:
            return rep->IsBool();

        case DBTYPE_BYTES:
            return rep->IsBuffer();

        case DBTYPE_STR:
            return rep->IsString();

        case DBTYPE_WSTR:
            return rep->IsWString();
    }

    return false;

#undef CheckTypeRange
#undef CheckTypeRangeUnsigned
}
