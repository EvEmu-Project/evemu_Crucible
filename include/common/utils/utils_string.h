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
 * @brief Appends string to end of another one.
 *
 * @param[in,out] ret     Pointer to the original string; result is stored here.
 * @param[in,out] bufsize Amount of allocated memory pointed by ret.
 * @param[in,out] strlen  Length of string in ret.
 * @param[in]     fmt     Format of string to append.
 *
 * @return New length of string.
 */
size_t AppendAnyLenString( char** ret, size_t* bufsize, size_t* strlen, const char* fmt, ... );

/**
 * @brief Converts string to boolean.
 *
 * @param[in] str String to be converted.
 *
 * @return Resulting boolean.
 */
bool atobool( const char* str );

/**
 * @brief Escapes string.
 *
 * @param[in,out] subject String to be escaped.
 * @param[in]     find    String which shall be replaced.
 * @param[in]     replace String with which to replace.
 */
void EscapeString( std::string& subject, const std::string& find, const std::string& replace );

/**
 * @brief Generates random key.
 *
 * @param[in] length Length of key to generate.
 *
 * @return Generated key.
 */
std::string generate_key( size_t length );

/**
 * @brief Converts string with hex number to the number itself.
 *
 * @param[in] num Hex string.
 *
 * @return The number.
 */
template<typename X>
X hexto( char* num );

/**
 * @brief Checks whether character is printable.
 *
 * @param[in] c The character to be checked.
 *
 * @retval true  The character is printable.
 * @retval false The character is not printable.
 */
bool IsPrintable( char c );
/**
 * @brief Checks whether string is printable.
 *
 * @param[in] str    String to be checked.
 * @param[in] length Length of string.
 *
 * @retval true  The string is printable.
 * @retval false The string is not printable.
 */
bool IsPrintable( const char* str, size_t length );
/**
 * @brief Checks whether string is printable.
 *
 * @param[in] str String to be checked.
 *
 * @retval true  The string is printable.
 * @retval false The string is not printable.
 */
bool IsPrintable( const std::string& str );

/**
 * @brief Convers num to string.
 *
 * @param[in] num The number to be converted.
 *
 * @return Pointer to static array where result is stored.
 */
const char* itoa( int num );

/**
 * function to convert a vector of numbers to a string containing string representatives of those
 * numbers.
 *
 * @param[in]  ints     contains the numbers that need to converted.
 * @param[in]  if_empty is the default value added if ints is empty.
 * @param[out] into     contains the string representatives of the numbers.
 */
void ListToINString( const std::vector<int32>& ints, std::string& into, const char* if_empty = "" );

/**
 * @brief toupper() for strings.
 *
 * @param[in]  source Source string.
 * @param[out] target Array which receives result. 
 */
void MakeUpperString( const char* source, char* target );
/**
 * @brief tolower() for strings.
 *
 * @param[in]  source Source string.
 * @param[out] target Array which receives result. 
 */
void MakeLowerString( const char* source, char* target );

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

/**
 * @brief Splits path to its components.
 *
 * @param[in]  path Path to split.
 * @param[out] into Vector which receives components of the path.
 */
void split_path( const std::string& path, std::vector<std::string>& into );

/**
 * @brief Copies given amount of characters from source to dest.
 *
 * Unlike normal strncpy this one puts null terminator to the end
 * of the string.
 * ref: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecrt/htm/_wcecrt_strncpy_wcsncpy.asp
 *
 * @param[out] dest   Array which receives result.
 * @param[in]  source Source string.
 * @param[in]  size   Number of characters to copy.
 *
 * @return Pointer to destination.
 */
char* strn0cpy( char* dest, const char* source, size_t size );
/**
 * @brief Copies given amount of characters from source to dest.
 *
 * Unlike normal strncpy this one puts null terminator to the end
 * of the string.
 * ref: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecrt/htm/_wcecrt_strncpy_wcsncpy.asp
 *
 * @param[out] dest   Array which receives result.
 * @param[in]  source Source string.
 * @param[in]  size   Number of characters to copy.
 *
 * @retval true  String was truncated during copying.
 * @retval false No string truncation occurred.
 */
bool strn0cpyt( char* dest, const char* source, size_t size );

template<typename X>
EVEMU_INLINE X hexto( char* num )
{
	const size_t len = strlen( num );

	X ret = 0;

	for( size_t order = 0; order < len; ++order )
    {
        const size_t i = ( len - ( order + 1 ) );

		if( num[i] >= 'A' && num[i] <= 'F' )
			ret += ( ( ( num[i] - 'A' ) + 10 ) << ( order << 2 ) );
		else if( num[i] >= 'a' && num[i] <= 'f' )
			ret += ( ( ( num[i] - 'a' ) + 10 ) << ( order << 2 ) );
		else if( num[i] >= '0' && num[i] <= '9' )
			ret += ( ( num[i] - '0' ) << ( order << 2 ) );
		else
			return 0;
	}

	return ret;
}

#endif // UTILS_STRING_H
