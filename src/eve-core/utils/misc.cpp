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

#include "eve-core.h"

#include "utils/misc.h"
#include "utils/utils_string.h"

static uint16 crc16_table[ 256 ] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

uint16 crc_hqx( const uint8* data, size_t len, uint16 crc )
{
    while( len-- )
        crc = ( crc << 8 ) ^ crc16_table[ ( crc >> 8 ) ^ ( *data++ ) ];

    return crc;
}

int64 filesize( const char* filename )
{
    FILE* fd = fopen( filename, "r" );
    if( fd == NULL )
        return 0;

    return filesize( fd );
}

int64 filesize( FILE* fd )
{
#ifdef HAVE_SYS_STAT_H
    struct stat st;
    ::fstat( ::fileno( fd ), &st );
    return st.st_size;
#else /* !HAVE_SYS_STAT_H */
    return ::filelength( ::fileno( fd ) );
#endif /* !HAVE_SYS_STAT_H */
}

int64 npowof2( int64 num )
{
    --num;
    num |= ( num >>  1 );
    num |= ( num >>  2 );
    num |= ( num >>  4 );
    num |= ( num >>  8 );
    num |= ( num >> 16 );
    num |= ( num >> 32 );
    ++num;

    return num;
}

int64 MakeRandomInt( int64 low, int64 high )
{
    return (int64)MakeRandomFloat( (double)low, (double)high );
}

double MakeRandomFloat( double low, double high )
{
    if( low == high )
        return low;

    static bool seeded = false;
    if( !seeded )
    {
        time_t x = ::time( NULL );
        ::srand( x * ( x % (time_t)( high - low ) ) );
        seeded = true;
    }

    return low + ( high - low ) * ::rand() / RAND_MAX;
}

/// create PID file
uint32 CreatePIDFile(const std::string& filename)
{
    FILE* pid_file = fopen (filename.c_str(), "w" );
    if (pid_file == NULL)
        return 0;

    #ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
    #else
    pid_t pid = getpid();
    #endif

    fprintf(pid_file, "%u", pid );
    fclose(pid_file);

    return (uint32)pid;
}

// EvE namespace
double EvE::min1(double x, double y)
{
    double min = ((x < y) ? x : y);
    return  ((min > 1) ? 1 : min);
}

double EvE::min(double x, double y, double z)
{
    double min = ((x < y) ? x : y);
    return  ((min > z) ? z : min);
}

double EvE::max(double x, double y, double z)
{
    double max = ((x > y) ? x : y);
    return  ((max < z) ? z : max);
}


void EvE::traceStack(void)
{
    uint8 j(0), nptrs(0);
    #define SIZE 100
    void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, SIZE);
    printf("backtrace() returned %i addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
     *       would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (!strings) {
        printf("backtrace symbols error");
        return;
    }

    for (j = 0; j < nptrs; ++j)
        printf("%s\n", strings[j]);

    free(strings);
}

bool EvE::icontains(std::string data, std::string toSearch, size_t pos/*0*/)
{
    // Convert complete given String to lower case
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    // Convert complete given Sub String to lower case
    std::transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);
    // Find sub string in given string
    return (data.find(toSearch, pos) != std::string::npos);
}

const char* EvE::FormatTime(int64 time/*-1*/) {
    if (time < 0)
        return "Invalid Time";
    if (time < 1)
        return "None";
    double seconds = time;
    double minutes = seconds / 60.0;
    float hours = minutes / 60.0;
    float days = hours / 24.0;
    float weeks = days / 7.0;
    float months = days / 30.0;

    int s(fmod(seconds, 60.0));
    int m(fmod(minutes, 60.0));
    int h(fmod(hours, 24.0));
    int d(fmod(days, 7.0));
    int w(fmod(weeks, 4.0));
    int M(fmod(months, 12.0));

    std::ostringstream uptime;
    if (M)
        uptime << M << "M" << w << "w" << d << "d" << h << "h" << m << "m" << s << "s";
    else if (w)
        uptime << w << "w" << d << "d" << h << "h" << m << "m" << s << "s";
    else if (d)
        uptime << d << "d" << h << "h" << m << "m" << s << "s";
    else if (h)
        uptime << h << "h" << m << "m" << s << "s";
    else if (m)
        uptime << m << "m" << s << "s";
    else
        uptime << s << "s";

    return uptime.str().c_str();
}

const char* EvE::FormatTime(double time/*-1*/) {
    if (time < 0)
        return "Invalid Time";
    if (time < 1)
        return "None";
    double seconds = time;
    double minutes = seconds / 60.0;
    float hours = minutes / 60.0;
    float days = hours / 24.0;
    float weeks = days / 7.0;
    float months = days / 30.0;

    int s(fmod(seconds, 60.0));
    int m(fmod(minutes, 60.0));
    int h(fmod(hours, 24.0));
    int d(fmod(days, 7.0));
    int w(fmod(weeks, 4.0));
    int M(fmod(months, 12.0));

    std::ostringstream uptime;
    if (M)
        uptime << M << "M" << w << "w" << d << "d" << h << "h" << m << "m" << s << "s";
    else if (w)
        uptime << w << "w" << d << "d" << h << "h" << m << "m" << s << "s";
    else if (d)
        uptime << d << "d" << h << "h" << m << "m" << s << "s";
    else if (h)
        uptime << h << "h" << m << "m" << s << "s";
    else if (m)
        uptime << m << "m" << s << "s";
    else
        uptime << s << "s";

    return uptime.str().c_str();
}

double EvE::trunicate2(double dig)
{
    int64 first = dig * 100;
    double ret = (float)first / 100;
    return ret;
    //return (double)((int)dig*100)/100;
}
