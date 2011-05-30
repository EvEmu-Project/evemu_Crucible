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

#include "CommonPCH.h"

#include "log/logsys.h"
#include "utils/utils_string.h"

const uint32 HEX_DUMP_PREVIEW_LIMIT = 1024;

void build_hex_line( const uint8* buffer, size_t length, size_t offset, char* ret, unsigned int padding )
{
    ret += snprintf( ret, length, "%0*X:", padding, offset );

    char printable[17];

    for( size_t i = 0; i < 16; i++ )
    {
        if( i == 8 )
        {
            ret += snprintf( ret, length, " -" );
        }

        if( ( i + offset ) < length )
        {
            uint8 c = *(const uint8*)( buffer + offset + i );

            ret += snprintf( ret, length, " %02X", c );
            printable[i] = ( IsPrintable( c ) ? (char)c : '.' );
        }
        else
        {
            ret += snprintf( ret, length, "   " );
            printable[i] = 0;
        }
    }

    snprintf( ret, length, "  | %.16s", printable );
}

void pfxHexDump( const char* pfx, FILE* into, const uint8* data, uint32 length )
{
    char buffer[80];

    for( uint32 offset = 0; offset < length; offset += 16 )
    {
        build_hex_line( data, length, offset, buffer, 4 );

        fprintf( into, "%s%s\n", pfx, buffer );
    }
}

void pfxHexDump( const char* pfx, LogType type, const uint8* data, uint32 length )
{
    char buffer[80];

    for( uint32 offset = 0; offset < length; offset += 16 )
    {
        build_hex_line( data, length, offset, buffer, 4 );

        _log( type, "%s%s", pfx, buffer );
    }
}

void pfxHexDumpPreview( const char* pfx, FILE* into, const uint8* data, uint32 length )
{
    char buffer[80];

    if( length > HEX_DUMP_PREVIEW_LIMIT )
    {
        pfxHexDump( pfx, into, data, HEX_DUMP_PREVIEW_LIMIT - 32 );
        fprintf( into, "%s ... truncated ...\n", pfx );

        build_hex_line( data, length, length - 16, buffer, 4 );
        fprintf( into, "%s%s\n", pfx, buffer );
    }
    else
        pfxHexDump( pfx, into, data, length );
}

void pfxHexDumpPreview( const char* pfx, LogType type, const uint8* data, uint32 length )
{
    char buffer[80];

    if( length > HEX_DUMP_PREVIEW_LIMIT )
    {
        pfxHexDump( pfx, type, data, HEX_DUMP_PREVIEW_LIMIT - 32 );
        _log( type, "%s ... truncated ...", pfx );

        build_hex_line( data, length, length - 16, buffer, 4 );
        _log( type, "%s%s", pfx, buffer );
    }
    else
        pfxHexDump( pfx, type, data, length );
}
