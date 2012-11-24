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
    Author:     Captnoord
*/

#ifndef __EVE_MARSHAL_STRING_TABLE_H__INCL__
#define __EVE_MARSHAL_STRING_TABLE_H__INCL__

#include "python/PyRep.h"

/* Since returned index is always > 0, we may use 0 as error signal. */
#define STRING_TABLE_ERROR 0

/**
 * @brief a singleton data container for communication string lookup.
 *
 * this class is a data container for communication string lookup.
 * eventually this class should be available to every thread the unmarshal's.
 * so only until we have solved the entire mess.. this is a singleton with mutex locks.
 *
 * @todo when the object puzzle is solved this class should be available to every thread.
 * it doesn't take much mem. So it means the mutexes can be removed when every thread has its own resource.
 *
 * @author Captnoord
 * @date December 2008
 */
class MarshalStringTable
: public Singleton<MarshalStringTable>
{
public:
    MarshalStringTable();

    /**
     * @brief lookup a index nr using a string
     *
     * @param[in] string that needs a lookup for a index nr.
     *
     * @return the index number of the string that was given; STRING_TABLE_ERROR if string is not found.
     */
    uint8 LookupIndex( const std::string& str );

    /**
    * @brief lookup a index nr using a string
    *
    * @param[in] string that needs a lookup for a index nr.
    *
    * @return the index number of the string that was given; STRING_TABLE_ERROR if string is not found.
    */
    uint8 LookupIndex( const char* str );

    /**
     * @brief lookup a string using a index
     *
     * @param[in] index is the index of the string that needs to be looked up.
     *
     * @return if succeeds returns pointer to static string; if fails returns NULL.
     */
    const char* LookupString( uint8 index );

private:
    /**
     * @brief djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
     *
     * @param[in] oStr string that needs to be hashed.
     *
     * @return djb2 has of the string.
     */
    uint32 hash( const char* str )
    {
        uint32 hash = 5381;
        int c;

        while( ( c = *str++ ) )
            hash = ( ( hash << 5 ) + hash ) + c; /* hash * 33 + c */

        return hash;
    }

    typedef std::tr1::unordered_map<uint32, uint8>  StringTableMap;
    typedef StringTableMap::iterator                StringTableMapItr;
    typedef StringTableMap::const_iterator          StringTableMapConstItr;

    StringTableMap  mStringTableMap;

    /* we made up this list so we have efficient string communication with the client */
    static const char* const s_mStringTable[];

    /* the current string count of the string table */
    static const size_t s_mStringTableSize;
};

#define sMarshalStringTable \
    ( MarshalStringTable::get() )

#endif /* !__EVE_MARSHAL_STRING_TABLE_H__INCL__ */



