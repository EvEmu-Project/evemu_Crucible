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
#include "EVEUtils.h"
#include "MiscFunctions.h"

#include "../server/inventory/InventoryItem.h"

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
#ifdef WIN32
	SYSTEMTIME st;
	GetSystemTime(&st);

	char buf[256];
	snprintf(buf, 255, "%d-%d-%d %d:%d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return(buf);
#else
	struct tm ut;
	time_t unix_time;
	uint32 nsec;
	Win32TimeToUnixTime(win32t, unix_time, nsec);
	
	memcpy(&ut, localtime(&unix_time), sizeof(ut));

	char buf[256];
	strftime(buf, 255, "%F %T", &ut);
	return(buf);
#endif
}

uint64 Win32TimeNow() {
#ifdef WIN32
	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
    return((int64(ft.dwHighDateTime) << 32) | int64(ft.dwLowDateTime));
#else
	return(UnixTimeToWin32Time(time(NULL), 0));
#endif
}


int strcpy_fake_unicode(uint16 *into, const char *from) {
	int r;
	for(r = 0; *from != '\0'; r++) {
		*into = *from;
		into++;
		from++;
	}
	return(r*2);
}

int GetSkillLevel(const std::vector<const InventoryItem *> &skills, const uint32 skillID) {
	std::vector<const InventoryItem *>::const_iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++)
		if((*cur)->typeID() == skillID)
			return((*cur)->skillLevel());
	return 0;
}

PyRep *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args) {
	PyRepPackedObject1 *res = new PyRepPackedObject1("ccp_exceptions.UserError");

	PyRep *py_args;
	if(args.empty())
		py_args = new PyRepNone;
	else {
		PyRepDict *d = new PyRepDict;
		py_args = d;
		std::map<std::string, PyRep *>::const_iterator cur, end;
		cur = args.begin();
		end = args.end();
		for(; cur != end; cur++)
			d->add(cur->first.c_str(), cur->second);
	}

	res->args = new PyRepTuple(2);
	res->args->items[0] = new PyRepString(exceptionType);
	res->args->items[1] = py_args;

	res->keywords.add("msgkey", new PyRepString(exceptionType));
	// here should be used PySavedStreamElement, but our marshaling doesnt support it yet
	res->keywords.add("dict", py_args->Clone());

	return(res);
}

PyRep *MakeCustomError(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, va);
	va_end(va);

	std::map<std::string, PyRep *> args;
	args["error"] = new PyRepString(str);
	delete[] str;

	return(MakeUserError("CustomError", args));
}





