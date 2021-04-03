/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
#include <eve-compat.h>

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
int64 filesize( const char* filename );
/**
 * @brief Obtains filesize.
 *
 * @param[in] fd Descriptor of file to examine.
 *
 * @return Size of file.
 */
int64 filesize( FILE* fd );

/**
 * @brief Calculates next (greater or equal)
 *        power-of-two number.
 *
 * @param[in] num Base number.
 *
 * @return Power-of-two number which is greater than or
 *         equal to the base number.
 */
int64 npowof2( int64 num );

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

inline bool IsEven(int64 number)                        { return ((number %2) == 0); }
inline bool IsNaN(double x)                             { return x!= x; }

uint32 CreatePIDFile(const std::string& filename);

namespace EvE {
    // returns the minimum of x,y as double
    inline double min(double x, double y)               { return ((x < y) ? x : y); }
    // returns the minimum of x,y as 32b integer
    inline int32 min(int32 x, int32 y)                  { return ((x < y) ? x : y); }
    // returns the minimum of x,y and 1
    double min1(double x, double y);
    // returns the minimum of x,y and z
    double min(double x, double y, double z);
    // returns the max of x,y with y=0 by default
    inline int64 max(int64 x, int64 y=0)                { return ((x > y) ? x : y); }
    // returns the maximum of x,y with y=0 by default
    inline double max(double x, double y=0)             { return ((x > y) ? x : y); }
    // returns the maximum of x,y and z
    double max(double x, double y, double z);
    // prints the top10 items in stack to stdout
    void traceStack(void);
    // Find Case Insensitive Sub String in a given substring
    bool icontains(std::string data, std::string toSearch, size_t pos = 0);
    // format given time (in seconds) to month/week/day/hour/minute/second
    // <0 will return "Invalid Time".  0 will return "None"
    const char* FormatTime(int64 time=-1);
    // format given time (in seconds) to month/week/day/hour/minute/second
    // <0 will return "Invalid Time".  0 will return "None"
    const char* FormatTime(double time=-1);

    double trunicate2(double dig=0);
}

#endif /* !__MISC_H__INCL__ */
