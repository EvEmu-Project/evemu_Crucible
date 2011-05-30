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
	Author:		Zhur, Bloody.Rabbit
*/

#include "CommonPCH.h"

#include "log/LogNew.h"
#include "utils/Seperator.h"

/************************************************************************/
/* Seperator                                                            */
/************************************************************************/
Seperator::Seperator( const char* str, const char* divs, const char* quotes )
{
    size_t len = strlen( str );
    bool inQuote = false;
    std::string* cur = NULL;

    for(; 0 < len; ++str, --len )
    {
        const char c = *str;
        const bool isDiv = ( NULL != strchr( divs, c ) );
        const bool isQuote = ( NULL != strchr( quotes, c ) );

        if( !inQuote )
        {
            if( NULL == cur )
            {
                if( !isDiv )
                {
                    mArgs.push_back( "" );
                    cur = &mArgs.back();

                    if( isQuote )
                        inQuote = true;
                    else
                        cur->push_back( c );
                }
            }
            else
            {
                if( isDiv )
                    cur = NULL;
                else
                    cur->push_back( c );
            }
        }
        else
        {
            if( NULL != cur )
            {
                if( isQuote )
                    cur = NULL;
                else
                    cur->push_back( c );
            }
            else
            {
                if( isDiv )
                    inQuote = false;
                else
                    sLog.Error( "Seperator", "Invalid input." );
            }
        }
    }
}

