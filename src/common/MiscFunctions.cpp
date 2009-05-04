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
	Author:		Zhur
*/

#include "../common/common.h"
#include "MiscFunctions.h"
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef WIN32
#include <varargs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <iostream>
#include <iomanip>
#ifdef WIN32
	#include <io.h>
#define vsnprintf _vsnprintf
#endif
#include "../common/timer.h"

using namespace std;

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
int	asnprintf(char** strp, const char* fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	int res = vasprintf(strp, fmt, argptr);
	va_end(argptr);

	return res;
}

int	vasprintf(char** strp, const char* fmt, va_list ap)
{
	int size = vsnprintf(NULL, 0, fmt, ap);

	*strp = (char*)malloc(size+1);
	if (*strp == NULL)
		return -1;

	size = vsnprintf(*strp, size+1, fmt, ap);
	(*strp)[size] = '\0';
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

bool atobool(char* iBool) {
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

int32 filesize(FILE* fp) {
#ifdef WIN32
	return _filelength(_fileno(fp));
#else
	struct stat file_stat;
	fstat(fileno(fp), &file_stat);
	return (int32) file_stat.st_size;
/*	int32 tmp = 0;
	while (!feof(fp)) {
		fseek(fp, tmp++, SEEK_SET);
	}
	return tmp;*/
#endif
}

int32 ResolveIP(const char* hostname, char* errbuf) {
#ifdef WIN32
	static InitWinsock ws;
#endif
	if (errbuf)
		errbuf[0] = 0;
	if (hostname == 0) {
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "ResolveIP(): hostname == 0");
		return 0;
	}
    struct sockaddr_in	server_sin;
#ifdef WIN32
	PHOSTENT phostent = NULL;
#else
	struct hostent *phostent = NULL;
#endif
	server_sin.sin_family = AF_INET;
	if ((phostent = gethostbyname(hostname)) == NULL) {
#ifdef WIN32
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %i", WSAGetLastError());
#else
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %s", strerror(errno));
#endif
		return 0;
	}
#ifdef WIN32
	memcpy ((char FAR *)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#else
	memcpy ((char*)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#endif
	return server_sin.sin_addr.s_addr;
}

/*bool ParseAddress(const char* iAddress, int32* oIP, int16* oPort, char* errbuf) {
	Seperator sep(iAddress, ':', 2, 250, false, 0, 0);
	if (sep.argnum == 1 && sep.IsNumber(1)) {
		*oIP = ResolveIP(sep.arg[0], errbuf);
		if (*oIP == 0)
			return false;
		if (oPort)
			*oPort = atoi(sep.arg[1]);
		return true;
	}
	return false;
}*/

#ifdef WIN32
InitWinsock::InitWinsock() {
	WORD version = MAKEWORD (1,1);
	WSADATA wsadata;
	WSAStartup (version, &wsadata);
}

InitWinsock::~InitWinsock() {
	WSACleanup();
}

#endif


const char * itoa(int num) {
		static char temp[_ITOA_BUFLEN];
		memset(temp,0,_ITOA_BUFLEN);
		snprintf(temp,_ITOA_BUFLEN,"%d",num);
		return temp;
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

//I didn't even look to see if windows supports random();
#ifdef WIN32
	#define SeedRandom srand
	#define GenerateRandom rand
#else
	#define SeedRandom srandom
	#define GenerateRandom random
#endif

/* 
 * solar: generate a random integer in the range low-high 
 * this should be used instead of the rand()%limit method
 *
 * Doing all our work as doubles theoretically maximizes our
 * use of the random bits generated, instead of throwing many
 * of them away like the rand()%limit
 */
int MakeRandomInt(int low, int high)
{
	if(low == high)
		return(low);
	return (int)MakeRandomFloat((double)low, (double)high + 0.999);
}

double MakeRandomFloat(double low, double high)
{
//	_CP(MakeRandomFloat);
	if(low == high)
		return(low);
	
	static bool seeded=0;
	double diff = high - low;
  
	if(!diff) return low;
	if(diff < 0)
		diff = 0 - diff;

	if(!seeded)
	{
		SeedRandom((unsigned int)time(0) * (unsigned int)(time(0) % (int)diff));
		seeded = true;
	}
  
	return (GenerateRandom() / (double)RAND_MAX * diff + (low > high ? high : low));
}
