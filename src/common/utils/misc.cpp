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
    Author:     Zhur
*/

#include "CommonPCH.h"

#include "utils/misc.h"

const char *MakeUpperString(const char *source) {
    static char str[128];
    if (!source)
	    return NULL;
    MakeUpperString(source, str);
    return str;
}

void MakeUpperString(const char *source, char *target) {
    if (!source || !target) {
	*target=0;
        return;
    }
    while (*source)
    {
        *target = toupper(*source);
        target++;source++;
    }
    *target = 0;
}

const char *MakeLowerString(const char *source) {
    static char str[128];
    if (!source)
	    return NULL;
    MakeLowerString(source, str);
    return str;
}

void MakeLowerString(const char *source, char *target) {
    if (!source || !target) {
	*target=0;
        return;
    }
    while (*source)
    {
        *target = tolower(*source);
        target++;source++;
    }
    *target = 0;
}

#ifdef WIN32
int	asprintf(char** strp, const char* fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	int res = vasprintf(strp, fmt, argptr);
	va_end(argptr);

	return res;
}

int	vasprintf(char** strp, const char* fmt, va_list ap)
{
    //va_list ap_temp;
    //va_copy(ap_temp, ap);
	//int size = vsnprintf(NULL, 0, fmt, ap);
    int size = 0x4000;
    char* buff = (char*)malloc(size+1);

	if (buff == NULL)
    {
        assert(false);
		return -1;
    }

	size = vsnprintf(buff, size, fmt, ap);

    buff[size] = '\0';
	(*strp) = buff;
	return size;
}
#endif//WIN32

int32 AppendAnyLenString(char** ret, int32* bufsize, int32* strlen, const char* format, ...) {
	if (*bufsize == 0)
		*bufsize = 256;
	if (*ret == 0)
		*strlen = 0;
	int chars = -1;
	char* oldret = 0;
	va_list argptr;
	va_start(argptr, format);
	while (chars == -1 || chars >= (int32)(*bufsize-*strlen)) {
		if (chars == -1)
			*bufsize += 256;
		else
			*bufsize += chars + 25;
		oldret = *ret;
		*ret = new char[*bufsize];
		if (oldret) {
			if (*strlen)
				memcpy(*ret, oldret, *strlen);
			SafeDelete(oldret);
		}
		chars = vsnprintf(&(*ret)[*strlen], (*bufsize-*strlen), format, argptr);
	}
	va_end(argptr);
	*strlen += chars;
	return *strlen;
}

bool atobool(const char* iBool) {
	if (!strcasecmp(iBool, "true"))
		return true;
	if (!strcasecmp(iBool, "false"))
		return false;
	if (!strcasecmp(iBool, "yes"))
		return true;
	if (!strcasecmp(iBool, "no"))
		return false;
	if (!strcasecmp(iBool, "on"))
		return true;
	if (!strcasecmp(iBool, "off"))
		return false;
	if (!strcasecmp(iBool, "enable"))
		return true;
	if (!strcasecmp(iBool, "disable"))
		return false;
	if (!strcasecmp(iBool, "enabled"))
		return true;
	if (!strcasecmp(iBool, "disabled"))
		return false;
	if (!strcasecmp(iBool, "y"))
		return true;
	if (!strcasecmp(iBool, "n"))
		return false;
	if (atoi(iBool))
		return true;
	return false;
}

void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding)
{
char *ptr=out_buffer;
int i;
char printable[17];
    ptr+=sprintf(ptr,"%0*lu:",padding,offset);
    for(i=0;i<16; i++) {
        if (i==8) {
            strcpy(ptr," -");
            ptr+=2;
        }
        if (i+offset < length) {
            unsigned char c=*(const unsigned char *)(buffer+offset+i);
            ptr+=sprintf(ptr," %02x",c);
            printable[i]=isprint(c) ? c : '.';
        } else {
            ptr+=sprintf(ptr,"   ");
            printable[i]=0;
        }
    }
    sprintf(ptr,"  | %.16s",printable);
}

/*void dump_message_column(unsigned char *buffer, unsigned long length, string leader, FILE *to)
{
unsigned long i,j;
unsigned long rows,offset=0;
    rows=(length/16)+1;
    for(i=0;i<rows;i++) {
        fprintf(to, "%s%05lu: ",leader.c_str(),i*16);
        for(j=0;j<16;j++) {
            if(j == 8)
                fprintf(to, "- ");
            if (offset+j<length)
                fprintf(to, "%02x ",*(buffer+offset+j));
            else
                fprintf(to, "   ");
        }
        fprintf(to, "| ");
        for(j=0;j<16;j++,offset++) {
            if (offset<length) {
                char c=*(buffer+offset);
                fprintf(to, "%c",isprint(c) ? c : '.');
            }
        }
        fprintf(to, "\n");
    }
}*/

void EscapeStringSequence(std::string &subject,  const std::string &find, const std::string &replace) {
    std::string::size_type pos = 0;
    while((pos = subject.find(find, pos)) != std::string::npos) {
        subject.replace(pos, find.length(), replace);
        pos += replace.length();
    }
}

uint64 filesize( const char* filename )
{
    FILE* fd = fopen( filename, "r" );
    if( fd == NULL )
        return 0;

    return filesize( fd );
}

uint64 filesize( FILE* fd )
{
#ifdef WIN32
	return _filelength( _fileno( fd ) );
#else
	struct stat file_stat;
	fstat( fileno( fd ), &file_stat );
	return file_stat.st_size;
#endif
}

std::string generate_key(int length)
{
    std::string key;
//TODO: write this for win32...
#ifndef WIN32
    int i;
    timeval now;
    static const char *chars="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for(i=0;i<length;i++) {
        gettimeofday(&now,NULL);
        srand(now.tv_sec^now.tv_usec);
        key+=(char)chars[(int) (36.0*rand()/(RAND_MAX+1.0))];
    }
#endif
    return key;
}

int32 hextoi(char* num) {
	size_t len = strlen(num);
	if (len < 3)
		return 0;

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X'))
		return 0;

	int32 ret = 0;
	int mul = 1;
	for (size_t i=len-1; i>=2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F')
			ret += ((num[i] - 'A') + 10) * mul;
		else if (num[i] >= 'a' && num[i] <= 'f')
			ret += ((num[i] - 'a') + 10) * mul;
		else if (num[i] >= '0' && num[i] <= '9')
			ret += (num[i] - '0') * mul;
		else
			return 0;
		mul *= 16;
	}
	return ret;
}

int64 hextoi64(char* num) {
	size_t len = strlen(num);
	if (len < 3)
		return 0;

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X'))
		return 0;

	int64 ret = 0;
	int mul = 1;
	for (size_t i=len-1; i>=2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F')
			ret += ((num[i] - 'A') + 10) * mul;
		else if (num[i] >= 'a' && num[i] <= 'f')
			ret += ((num[i] - 'a') + 10) * mul;
		else if (num[i] >= '0' && num[i] <= '9')
			ret += (num[i] - '0') * mul;
		else
			return 0;
		mul *= 16;
	}
	return ret;
}

uint64 npowof2( uint64 num )
{
    --num;
    num |= ( num >> 1 );
    num |= ( num >> 2 );
    num |= ( num >> 4 );
    num |= ( num >> 8 );
    num |= ( num >> 16 );
    num |= ( num >> 32 );
    return ++num;
}

// normal strncpy doesn't put a null term on copied strings, this one does
// ref: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecrt/htm/_wcecrt_strncpy_wcsncpy.asp
char* strn0cpy(char* dest, const char* source, int32 size) {
	if (!dest)
		return 0;
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return dest;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return dest;
}

// String N w/null Copy Truncated?
// return value =true if entire string(source) fit, false if it was truncated
bool strn0cpyt(char* dest, const char* source, int32 size) {
	if (!dest)
		return 0;
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return true;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return (bool) (source[strlen(dest)] == 0);
}

//I didn't even look to see if windows supports random();
#ifdef WIN32
#   define SeedRandom srand
#   define GenerateRandom rand
#else
#   define SeedRandom srandom
#   define GenerateRandom random
#endif

int MakeRandomInt(int low, int high)
{
	return (int)MakeRandomFloat( (double)low, (double)high );
}

double MakeRandomFloat(double low, double high)
{
    if( low > high )
        std::swap( low, high );

    double diff = high - low;
	if( diff == 0 )
		return low;

	static bool seeded = false;
	if( !seeded )
	{
		SeedRandom( (unsigned int)time(0) * (unsigned int)( time(0) % (int)diff ) );
		seeded = true;
	}

    return ( low + diff * ( (double)GenerateRandom() / (double)RAND_MAX ) );
}

#define _ITOA_BUFLEN 25

const char* itoa(int num)
{
    static char buf[ _ITOA_BUFLEN ];

    memset( buf, 0, _ITOA_BUFLEN );
    snprintf( buf, _ITOA_BUFLEN, "%d", num );

    return buf;
}

#ifndef WIN32
const char * itoa(int num, char* a,int b) {
		static char temp[_ITOA_BUFLEN];
		memset(temp,0,_ITOA_BUFLEN);
		snprintf(temp,_ITOA_BUFLEN,"%d",num);
		return temp;
		return temp;
}
#endif

#ifndef WIN32
void print_stacktrace()
{
#if defined( FREEBSD ) || defined( __CYGWIN__ )
    printf("Insert stack trace here...\n");
#else
    void* ba[20];
    int n = backtrace( ba, 20 );
    if( n != 0 )
    {
        char** names = backtrace_symbols( ba, n );
        if( names != NULL )
        {
            int i;
            printf( "called from %s\n", (char*)names[0] );
            for( i = 1; i < n; ++i )
                printf( "            %s\n", (char*)names[i] );
        }
        SafeFree( names );
    }
#endif //!FREEBSD
}
#endif //!WIN32

static uint16 crc16_table[256] = {
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
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

uint16 crc_hqx(const uint8* data, size_t len, uint16 crc)
{
    while( len-- )
        crc = ( crc << 8 ) ^ crc16_table[ ( crc >> 8 ) ^ ( *data++ ) ];

    return crc;
}
