/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
    Author:     Captnoord
*/

#include "CommonPCH.h"

#include "utils/utils_string.h"

size_t py_mbstowcs( uint16 * wcstr, const char * mbstr, size_t max )
{
    size_t i;
    for (i = 0; i < max; i++, wcstr++, mbstr++) {
        *wcstr = *mbstr;
        if (*wcstr == '\0') {
            break;
        }
    }
    return i;
}

bool py_decode_escape( const char* str, Buffer& into )
{
    int len = (int)strlen( str );
	const char* end = str + len;
	while( str < end )
    {
	    int c;

		if( *str != '\\' )
        {
			into.Write<char>( *str++ );
			continue;
		}
        if( ++str == end )
            //ended with a \ char
			return false;

        switch( *str++ )
        {
		/* XXX This assumes ASCII! */
		case '\n': break;	//?
		case '\\': into.Write<char>('\\'); break;
		case '\'': into.Write<char>('\''); break;
		case '\"': into.Write<char>('\"'); break;
		case 'b':  into.Write<char>('\b'); break;
		case 'f':  into.Write<char>('\014'); break; /* FF */
		case 't':  into.Write<char>('\t'); break;
		case 'n':  into.Write<char>('\n'); break;
		case 'r':  into.Write<char>('\r'); break;
		case 'v':  into.Write<char>('\013'); break; /* VT */
		case 'a':  into.Write<char>('\007'); break; /* BEL, not classic C */
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c = str[-1] - '0';
			if( '0' <= *str && *str <= '7' )
            {
				c = (c<<3) + *str++ - '0';
				if( '0' <= *str && *str <= '7' )
					c = (c<<3) + *str++ - '0';
			}
			into.Write<uint8>( c );
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

				into.Write<uint8>( x );
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

/************************************************************************/
/* ContainsNonPrintables                                                */
/************************************************************************/
bool ContainsNonPrintables( const char *c, uint32 length ) {
    for(; length > 0; c++, length--) {
        if( !isgraph( *c ) && !isspace( *c ) )
            return true;
    }
    return false;
}

bool ContainsNonPrintables( const std::string& str )
{
    return ContainsNonPrintables( str.c_str(), str.size() );
}

