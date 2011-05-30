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

#ifndef __UTILS__UTILS_STRING_H__INCL__
#define __UTILS__UTILS_STRING_H__INCL__

#include "utils/Buffer.h"

/// std::string to use where you would use NULL for const char*.
extern const std::string NULL_STRING;

/**
 * @brief Generates random key.
 *
 * @param[in] length Length of key to generate.
 *
 * @return Generated key.
 */
std::string GenerateKey( size_t length );

/**
 * @brief Checks whether character is a number.
 *
 * @param[in] c The character to be checked.
 *
 * @retval true  The character is a number.
 * @retval false The character is not a number.
 */
bool IsNumber( char c );
/**
 * @brief Checks whether string is a number.
 *
 * @param[in] str The string to be checked.
 * @param[in] len Length of the string.
 *
 * @retval true  The string is a number.
 * @retval false The string is not a number.
 */
bool IsNumber( const char* str, size_t len );
/**
 * @brief Checks whether string is a number.
 *
 * @param[in] str The string to be checked.
 *
 * @retval true  The string is a number.
 * @retval false The string is not a number.
 */
bool IsNumber( const std::string& str );

/**
 * @brief Checks whether character is a hexadecimal number.
 *
 * @param[in] c The character to be checked.
 *
 * @retval true  The character is a hexadecimal number.
 * @retval false The character is not a hexadecimal number.
 */
bool IsHexNumber( char c );
/**
 * @brief Checks whether string is a hexadecimal number.
 *
 * @param[in] str The string to be checked.
 * @param[in] len Length of the string.
 *
 * @retval true  The string is a hexadecimal number.
 * @retval false The string is not a hexadecimal number.
 */
bool IsHexNumber( const char* str, size_t len );
/**
 * @brief Checks whether string is a hexadecimal number.
 *
 * @param[in] str The string to be checked.
 *
 * @retval true  The string is a hexadecimal number.
 * @retval false The string is not a hexadecimal number.
 */
bool IsHexNumber( const std::string& str );

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
 * @param[in] str String to be checked.
 * @param[in] len Length of string.
 *
 * @retval true  The string is printable.
 * @retval false The string is not printable.
 */
bool IsPrintable( const char* str, size_t len );
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
const char* itoa( int64 num );

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
bool PyDecodeEscape( const char* str, Buffer& into );

/**
 * @brief Does search & replace on @a subject.
 *
 * @param[in,out] subject String to be searched.
 * @param[in]     search  String which shall be replaced.
 * @param[in]     replace String with which to replace.
 */
void SearchReplace( std::string& subject, const std::string& search, const std::string& replace );

/**
 * @brief Splits path to its components.
 *
 * @param[in]  path Path to split.
 * @param[out] into Vector which receives components of the path.
 */
void SplitPath( const std::string& path, std::vector<std::string>& into );

/**
 * @brief sprintf() for std::string.
 *
 * @param[in] into Where to store the result.
 * @param[in] fmt  The format string.
 * @param[in] ...  Arguments.
 *
 * @return A value returned by vasprintf().
 */
int sprintf( std::string& into, const char* fmt, ... );
/**
 * @brief vsprintf() for std::string.
 *
 * @param[in] into Where to store the result.
 * @param[in] fmt  The format string.
 * @param[in] ap   Arguments.
 *
 * @return A value returned by vasprintf().
 */
int vsprintf( std::string& into, const char* fmt, va_list ap );

#endif /* !__UTILS__UTILS_STRING_H__INCL__ */
