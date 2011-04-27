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
    Author:     Captnoord, Bloody.Rabbit
*/

#include "CommonPCH.h"

#include "utils/utils_string.h"

const std::string NULL_STRING = "NULL";

std::string GenerateKey( size_t length )
{
    static const char CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static const size_t CHARS_COUNT = sizeof( CHARS ) / sizeof( char );

    std::string key;

    for(; 0 < length; --length)
        key += CHARS[ MakeRandomInt( 0, CHARS_COUNT - 1 ) ];

    return key;
}

bool IsNumber( char c )
{
    return 0 != isdigit( c );
}

bool IsNumber( const char* str, size_t len )
{
    // skip sign if there is one
    if( len >= 1 )
    {
        if(    '-' == str[0]
            || '+' == str[0] )
        {
            str += 1;
            len -= 1;
        }
    }

    if( 0 == len )
        return false;

    bool seenDec = false;
    for(; len > 0; ++str, --len)
    {
        if( !IsNumber( *str ) )
        {
            if( !seenDec && '.' == *str )
                seenDec = true;
            else
                return false;
        }
    }

    return true;
}

bool IsNumber( const std::string& str )
{
    return IsNumber( str.c_str(), str.length() );
}

bool IsHexNumber( char c )
{
    return 0 != isxdigit( c );
}

bool IsHexNumber( const char* str, size_t len )
{
    // skip sign if there is one
    if( 1 >= len )
    {
        if(    '-' == str[0]
            || '+' == str[0] )
        {
            str += 1;
            len -= 1;
        }
    }

    // skip "0x" or "0X" prefix if there is one
    if( 2 >= len )
    {
        if(    '0' == str[0]
            && (    'x' == str[1]
                 || 'X' == str[1] ) )
        {
            str += 2;
            len -= 2;
        }
    }

    if( 0 == len )
        return false;

    for(; len > 0; ++str, --len)
    {
        if( !IsHexNumber( *str ) )
            return false;
    }

    return true;
}

bool IsHexNumber( const std::string& str )
{
    return IsHexNumber( str.c_str(), str.length() );
}

bool IsPrintable( char c )
{
    // They seem to expect it unsigned ...
    const unsigned char _c = c;

    return ( isgraph( _c ) || isspace( _c ) );
}

bool IsPrintable( const char* str, size_t len )
{
    for(; len > 0; ++str, --len)
    {
        if( !IsPrintable( *str ) )
            return false;
    }

    return true;
}

bool IsPrintable( const std::string& str )
{
    return IsPrintable( str.c_str(), str.size() );
}

#define _ITOA_BUFLEN 21

const char* itoa( int64 num )
{
    static char buf[ _ITOA_BUFLEN ];
    memset( buf, 0, _ITOA_BUFLEN );

    snprintf( buf, _ITOA_BUFLEN, I64d, num );

    return buf;
}

void ListToINString( const std::vector<int32>& ints, std::string& into, const char* if_empty )
{
    if( ints.empty() )
    {
        into = if_empty;
        return;
    }

    /*
     * Some small theory about numbers to strings
     *
     * the max size of a number converted to
     * a string is:
     * uint32 -1 results in
     * "4294967295" which is 10 characters.
     */
    size_t format_index = into.size();
    into.resize( format_index + ints.size() * ( 10 + 1 ) );

    std::vector<int32>::const_iterator cur, end;
    cur = ints.begin();
    end = ints.end();
    for(; cur != end; ++cur)
    {
        if( ( cur + 1 ) != end )
            format_index += snprintf( &into[ format_index ], 12, "%d,", *cur );
        else
            // last value to be printed
            format_index += snprintf( &into[ format_index ], 11, "%d", *cur );
    }
}

void MakeUpperString( const char* source, char* target )
{
    if( !target )
        return;

    for(; *source; ++target, ++source )
        *target = toupper( *source );

    *target = 0;
}

void MakeLowerString( const char* source, char* target )
{
    if( !target )
        return;

    for(; *source; ++target, ++source )
        *target = tolower( *source );

    *target = 0;
}

bool PyDecodeEscape( const char* str, Buffer& into )
{
    const size_t len = strlen( str );
    const char* const end = str + len;

    while( str < end )
    {
        if( *str != '\\' )
        {
            into.Append< char >( *str++ );
            continue;
        }

        if( ++str == end )
            //ended with a \ char
            return false;

        int c;
        switch( *str++ )
        {
            /* XXX This assumes ASCII! */
            case '\n':                                break; /* ? */
            case '\\': into.Append< char >( '\\' );   break;
            case '\'': into.Append< char >( '\'' );   break;
            case '\"': into.Append< char >( '\"' );   break;
            case 'b':  into.Append< char >( '\b' );   break;
            case 'f':  into.Append< char >( '\014' ); break; /* FF */
            case 't':  into.Append< char >( '\t' );   break;
            case 'n':  into.Append< char >( '\n' );   break;
            case 'r':  into.Append< char >( '\r' );   break;
            case 'v':  into.Append< char >( '\013' ); break; /* VT */
            case 'a':  into.Append< char >( '\007' ); break; /* BEL, not classic C */

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                c = str[-1] - '0';
                if( '0' <= *str && *str <= '7' )
                {
                    c = ( c << 3 ) + *str++ - '0';
                    if( '0' <= *str && *str <= '7' )
                        c = ( c << 3 ) + *str++ - '0';
                }
                into.Append< uint8 >( c );
                break;

        case 'x':
            if( isxdigit( str[0] ) && isxdigit( str[1] ) )
            {
                unsigned int x = 0;
                c = *str++;

                if( isdigit(c) )
                    x = c - '0';
                else if( islower(c) )
                    x = 10 + c - 'a';
                else
                    x = 10 + c - 'A';

                x = x << 4;
                c = *str++;

                if( isdigit(c) )
                    x += c - '0';
                else if( islower(c) )
                    x += 10 + c - 'a';
                else
                    x += 10 + c - 'A';

                into.Append< uint8 >( x );
                break;
            }
            //"invalid \\x escape");
            return false;

        default:
            return false;
        }
    }

    return true;
}

void SearchReplace( std::string& subject, const std::string& search, const std::string& replace )
{
    std::string::size_type pos = 0;
    while( ( pos = subject.find( search, pos ) ) != std::string::npos )
    {
        subject.replace( pos, search.length(), replace );
        pos += replace.length();
    }
}

void SplitPath( const std::string& path, std::vector<std::string>& into )
{
    const char* p = path.c_str();
    const char* begin = p;
    size_t len = 0;

    for(; *p != '\0'; ++p)
    {
        if( *p == '/' || *p == '\\' )
        {
            into.push_back( std::string( begin, len ) );
            len = 0;
            begin = p + 1;
        }
        else
        {
            ++len;
        }
    }

    if( begin < p )
        into.push_back( std::string( begin, len ) );
}

int sprintf( std::string& into, const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    int code = vsprintf( into, fmt, ap );

    va_end( ap );

    return code;
}

int vsprintf( std::string& into, const char* fmt, va_list ap )
{
    char* buf = NULL;

    int code = vasprintf( &buf, fmt, ap );
    if( 0 <= code )
        into = buf;

    SafeFree( buf );
    return code;
}
