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
#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include "utils/Buffer.h"

/**
 * @brief converts a multi byte string to a unicode string.
 *
 * @param[out] wcstr the destination wide character (unicode) string.
 * @param[in]  mbstr the source multi byte character string.
 * @param[in]  max   the max amount of characters to be converted.
 *
 * @return the amount of converted characters, returns 0 if a error occurred.
 */
size_t py_mbstowcs( uint16* wcstr, const char* mbstr, size_t max );
/**
 * @brief Decodes string escapes into actual characters.
 *
 * Based on PyString_DecodeEscape from Python.
 *
 * @param[in]  str  Escaped string.
 * @param[out] into The result will be put here.
 *
 * @retval true  Decode ran OK.
 * @retval false Error occured during decoding.
 */
bool py_decode_escape( const char* str, Buffer& into );

bool ContainsNonPrintables( const char* c, uint32 length );
bool ContainsNonPrintables( const std::string& str );

/**
 * function to convert a vector of numbers to a string containing string representatives of those
 * numbers.
 *
 * @param[in]  ints     contains the numbers that need to converted.
 * @param[in]  if_empty is the default value added if ints is empty.
 * @param[out] into     contains the string representatives of the numbers.
 */
void ListToINString( const std::vector<int32>& ints, std::string& into, const char* if_empty = "" );

#endif // UTILS_STRING_H
