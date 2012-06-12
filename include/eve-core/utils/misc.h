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

#ifndef __MISC_H__INCL__
#define __MISC_H__INCL__

/**
 * This is functionally equivalent to python's binascii.crc_hqx.
 *
 * @param[in] data Binary data to be checksumed.
 * @param[in] len  Length of binary data.
 * @param[in] crc  CRC value to start with.
 *
 * @return CRC-16 checksum.
 */
uint16 crc_hqx( const uint8* data, size_t len, uint16 crc = 0 );

/**
 * @brief Obtains filesize.
 *
 * @param[in] filename Name of file to examine.
 *
 * @return Size of file.
 */
uint64 filesize( const char* filename );
/**
 * @brief Obtains filesize.
 *
 * @param[in] fd Descriptor of file to examine.
 *
 * @return Size of file.
 */
uint64 filesize( FILE* fd );

/**
 * @brief Calculates next (greater or equal)
 *        power-of-two number.
 *
 * @param[in] num Base number.
 *
 * @return Power-of-two number which is greater than or
 *         equal to the base number.
 */
uint64 npowof2( uint64 num );

/**
 * @brief Generates random integer from interval [low; high].
 *
 * @param[in] low  Low boundary of interval.
 * @param[in] high High boundary of interval.
 *
 * @return The generated integer.
 */
int64 MakeRandomInt( int64 low = 0, int64 high = RAND_MAX );
/**
 * @brief Generates random real from interval [low; high].
 *
 * @param[in] low  Low boundary of interval.
 * @param[in] high High boundary of interval.
 *
 * @return The generated real.
 */
double MakeRandomFloat( double low = 0, double high = 1 );

#endif /* !__MISC_H__INCL__ */
