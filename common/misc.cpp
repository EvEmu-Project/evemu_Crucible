/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/



#include "common.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iostream>
#include <zlib.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/stat.h>
#endif
#include <time.h>
#include "misc.h"
#include "types.h"

using namespace std;


#ifndef WIN32
#if defined(FREEBSD) || defined(__CYGWIN__)
int print_stacktrace()
{
	printf("Insert stack trace here...\n");
	return(0);
}
#else //!WIN32 && !FREEBSD == linux
#include <execinfo.h>
int print_stacktrace()
{
  void *ba[20];
  int n = backtrace (ba, 20);
  if (n != 0)
    {
      char **names = backtrace_symbols (ba, n);
      if (names != NULL)
        {
          int i;
          cerr <<  "called from " << (char*)names[0] << endl;
          for (i = 1; i < n; ++i)
            cerr << "            " << (char*)names[i] << endl;
          free (names);
        }
    }
	return(0);
}
#endif //!FREEBSD
#endif //!WIN32


void dump_message_column(unsigned char *buffer, unsigned long length, string leader, FILE *to)
{
unsigned long i,j;
unsigned long rows,offset=0;
	rows=(length/16)+1;
	for(i=0;i<rows;i++) {
		fprintf(to, "%s%05ld: ",leader.c_str(),i*16);
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
}

string long2ip(unsigned long ip)
{
char temp[16];
union { unsigned long ip; struct { unsigned char a,b,c,d; } octet;} ipoctet;

	ipoctet.ip=ip;
	sprintf(temp,"%d.%d.%d.%d",ipoctet.octet.a,ipoctet.octet.b,ipoctet.octet.c,ipoctet.octet.d);

	return string(temp);
}

string string_from_time(string pattern, time_t now)
{
struct tm *now_tm;
char time_string[51];

	if (!now)
		time(&now);
	now_tm=localtime(&now);

	strftime(time_string,51,pattern.c_str(),now_tm);

	return string(time_string);
}

string timestamp(time_t now)
{
	return string_from_time("[%Y%m%d.%H%M%S] ",now);
}


string pop_arg(string &s, string seps, bool obey_quotes)
{
string ret;
unsigned long i;
bool in_quote=false;

	size_t length=s.length();
	for(i=0;i<length;i++) {
		char c=s[i];
		if (c=='"' && obey_quotes) {
			in_quote=!in_quote;
		}
		if (in_quote)
			continue;
		if (seps.find(c)!=string::npos) { 
			break;
		}
	}

	if (i==length) {
		ret=s;
		s="";
	} else {
		ret=s.substr(0,i);
		s.erase(0,i+1);
	}


	return ret;
}


string generate_key(int length)
{
string key;
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


uint32 GetFileLength(const char *file) {
#ifdef WIN32
	HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return(0);
	uint32 len = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return(len);
#else
	struct stat s;
	if(stat(file, &s) == -1)
		return(0);
	return(s.st_size);
#endif
}

bool ContainsNonPrintables(const char *c) {
	for(; *c != '\0'; c++) {
		if(*c < ' ' && *c != '\r' && *c != '\n') {
			return(true);
		} else if(*c > '~') {
			return(true);
		}
	}
	return(false);
}

bool ContainsNonPrintables(const char *c, uint32 length) {
	for(; length > 0; c++, length--) {
		if(*c < ' ' && *c != '\r' && *c != '\n') {
			return(true);
		} else if(*c > '~') {
			return(true);
		}
	}
	return(false);
}

void EscapeStringSequence(std::string &subject,  const std::string &find, const std::string &replace) {
	std::string::size_type pos = 0;
	while((pos = subject.find(find, pos)) != std::string::npos) {
		subject.replace(pos, find.length(), replace);
		pos += replace.length();
	}
}


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

/* I think this really returns a uint16
 *
 * this is functionally equivalent to python's binascii.crc_hqx
 */
uint32 CalcCRC16(byte *bin_data, int len, uint32 initial_crc) {
	uint32 crc;

	crc = initial_crc;

	while(len--) {
		crc=((crc<<8)&0xff00)^crc16_table[((crc>>8)&0xff)^*bin_data++];
	}

	return(crc);
}

#ifdef WIN32
// there has to be a better function for this...
//we do not support base 16 here... 
int64 strtoll(const char *str, const char **end, int8 base)
{
        int64 result = 0;
        int negative=0;

        while (*str == ' ' || *str == '\t' )
                str++;

		if(*str == '\0')
			return(0);

        if (*str == '+')
                str++;
        else if (*str == '-') {
                negative = 1;
                str++;
        }

		if(*str == '\0')
			return(0);

		if(base == 0) {
			if(*str == '0') {
				str++;
				//8 or 16...
				if(*str == 'x') {
					base = 16;
				} else {
					base = 8;
				}
			} else {
				base = 10;
			}
		}
	
		if(base == 16) {
			while (*str != '\0') {
				if(*str >= '0' && *str <= '9')
					result = (result*16)+(*str++ - '0');
				else if(*str >= 'A' && *str <= 'F')
					result = (result*16)+(*str++ - 'A' + 10);
				else
					break;
			}
		} else if(base == 8) {
			while (*str >= '0' && *str <= '7') {
				result = (result*base)+(*str++ - '0');
			}
		} else {
			while (*str >= '0' && *str <= '9') {
				result = (result*base)+(*str++ - '0');
			}
		}
		if(end != NULL)
			*end = str;
        return negative ? -result : result;
}

uint64 strtoull(const char *str, const char **end, int8 base)
{
        uint64 result = 0;

        while (*str == ' ' || *str == '\t')
                str++;

		if(*str == '\0')
			return(0);

        if (*str == '+')
                str++;
        else if (*str == '-') {
                return(0);
        }

		if(*str == '\0')
			return(0);

		if(base == 0) {
			if(*str == '0') {
				str++;
				//8 or 16...
				if(*str == 'x') {
					base = 16;
				} else {
					base = 8;
				}
			} else {
				base = 10;
			}
		}
	
		if(base == 16) {
			while (*str != '\0') {
				if(*str >= '0' && *str <= '9')
					result = (result*16)+(*str++ - '0');
				else if(*str >= 'A' && *str <= 'F')
					result = (result*16)+(*str++ - 'A' + 10);
				else
					break;
			}
		} else if(base == 8) {
			while (*str >= '0' && *str <= '7') {
				result = (result*base)+(*str++ - '0');
			}
		} else {
			while (*str >= '0' && *str <= '9') {
				result = (result*base)+(*str++ - '0');
			}
		}
		if(end != NULL)
			*end = str;
        return result;
}
#endif



