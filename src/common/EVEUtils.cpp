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

#include "common.h"
#include "EVEUtils.h"
#include "MiscFunctions.h"

#include "../packets/General.h"

static const uint64 SECS_BETWEEN_EPOCHS = 11644473600LL;
static const uint64 SECS_TO_100NS = 10000000; // 10^7

uint64 UnixTimeToWin32Time( time_t sec, uint32 nsec ) {
	return(
		(((uint64) sec) + SECS_BETWEEN_EPOCHS) * SECS_TO_100NS
		+ (nsec / 100)
	);
}

void Win32TimeToUnixTime( uint64 win32t, time_t &unix_time, uint32 &nsec ) {
	win32t -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);
	nsec = (win32t % SECS_TO_100NS) * 100;
	win32t /= SECS_TO_100NS;
	unix_time = win32t;
}

std::string Win32TimeToString(uint64 win32t) {
	time_t unix_time;
	uint32 nsec;
	Win32TimeToUnixTime(win32t, unix_time, nsec);
	
	char buf[256];
	strftime(buf, 256,
#ifdef WIN32
		"%Y-%m-%d %X",
#else
		"%F %T",
#endif /* !WIN32 */
	localtime(&unix_time));

	return(buf);
}

uint64 Win32TimeNow() {
#ifdef WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
    return((uint64(ft.dwHighDateTime) << 32) | uint64(ft.dwLowDateTime));
#else
	return(UnixTimeToWin32Time(time(NULL), 0));
#endif
}


size_t strcpy_fake_unicode(wchar_t *into, const char *from) {
	size_t r;
	for(r = 0; *from != '\0'; r++) {
		*into = *from;
		into++;
		from++;
	}
	return(r*2);
}

PyRep *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args) {
	PyRep *pyType = new PyRepString(exceptionType);
	PyRep *pyArgs;
	if(args.empty())
		pyArgs = new PyRepNone;
	else {
		PyRepDict *d = new PyRepDict;

		std::map<std::string, PyRep *>::const_iterator cur, end;
		cur = args.begin();
		end = args.end();
		for(; cur != end; cur++)
			d->add(cur->first.c_str(), cur->second);

		pyArgs = d;
	}

	util_ObjectEx no;
	no.type = "ccp_exceptions.UserError";

	no.args = new PyRepTuple(2);
	no.args->items[0] = pyType;
	no.args->items[1] = pyArgs;

	no.keywords.add("msg", pyType->Clone());
	no.keywords.add("dict", pyArgs->Clone());

	return(no.FastEncode());
}

PyRep *MakeCustomError(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char *str = NULL;
	vasprintf(&str, fmt, va);
	va_end(va);

	std::map<std::string, PyRep *> args;
	args["error"] = new PyRepString(str);
	free(str);

	return(MakeUserError("CustomError", args));
}

uint8 GetTypeSize(DBTYPE t)
{
	switch(t) {
		case DBTYPE_I8:
		case DBTYPE_UI8:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_FILETIME:
			return 64;
		case DBTYPE_I4:
		case DBTYPE_UI4:
		case DBTYPE_R4:
			return 32;
		case DBTYPE_I2:
		case DBTYPE_UI2:
			return 16;
		case DBTYPE_I1:
		case DBTYPE_UI1:
			return 8;
		case DBTYPE_BOOL:
			return 1;
		case DBTYPE_BYTES:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
			return 0;
	}
	return 0;
}


