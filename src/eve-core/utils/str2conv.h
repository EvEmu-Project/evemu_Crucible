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
    Author:     Bloody.Rabbit
*/

#ifndef __UTILS__STR2CONV_H__INCL__
#define __UTILS__STR2CONV_H__INCL__

/**
 * @brief Generic string conversion template.
 *
 * Try to use T( const char* ) directly.
 *
 * @param[in] str String to be converted.
 *
 * @param A value corresponding to content of @a str.
 */
template< typename T >
T str2( const char* str ) { return T( str ); }
/**
 * @brief Generic string conversion template.
 *
 * Redirect to str2( const char* ).
 *
 * @param[in] str String to be converted.
 *
 * @param A value corresponding to content of @a str.
 */
template< typename T >
T str2( const std::string& str ) { return str2< T >( str.c_str() ); }

/**
 * @brief Converts string to boolean.
 *
 * @param[in] str String to be converted.
 *
 * @param A boolean corresponding to content of @a str.
 */
template<>
bool str2< bool >( const char* str );

/**
 * @brief Converts string to integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An integer corresponding to content of @a str.
 */
template<>
int8 str2< int8 >( const char* str );
/**
 * @brief Converts string to integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An integer corresponding to content of @a str.
 */
template<>
int16 str2< int16 >( const char* str );
/**
 * @brief Converts string to integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An integer corresponding to content of @a str.
 */
template<>
int32 str2< int32 >( const char* str );
/**
 * @brief Converts string to integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An integer corresponding to content of @a str.
 */
template<>
int64 str2< int64 >( const char* str );

/**
 * @brief Converts string to unsigned integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An unsigned integer corresponding to content of @a str.
 */
template<>
uint8 str2< uint8 >( const char* str );
/**
 * @brief Converts string to unsigned integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An unsigned integer corresponding to content of @a str.
 */
template<>
uint16 str2< uint16 >( const char* str );
/**
 * @brief Converts string to unsigned integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An unsigned integer corresponding to content of @a str.
 */
template<>
uint32 str2< uint32 >( const char* str );
/**
 * @brief Converts string to unsigned integer.
 *
 * @param[in] str String to be converted.
 *
 * @return An unsigned integer corresponding to content of @a str.
 */
template<>
uint64 str2< uint64 >( const char* str );

/**
 * @brief Converts string to real number.
 *
 * @param[in] str String to be converted.
 *
 * @return A real number corresponding to content of @a str.
 */
template<>
float str2< float >( const char* str );
/**
 * @brief Converts string to real number.
 *
 * @param[in] str String to be converted.
 *
 * @return A real number corresponding to content of @a str.
 */
template<>
double str2< double >( const char* str );
/**
 * @brief Converts string to real number.
 *
 * @param[in] str String to be converted.
 *
 * @return A real number corresponding to content of @a str.
 */
template<>
long double str2< long double >( const char* str );

#endif /* !__UTILS__STR2CONV_H__INCL__ */
