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

#ifndef __EVEUTILS_H__
#define __EVEUTILS_H__

#include <string>
#include <vector>
#include <map>

#include "PyRep.h"

/*                                                                              
 *
SEC = 10000000L
MIN = (SEC * 60L)
HOUR = (MIN * 60L)
DAY = (HOUR * 24L)
MONTH = (30 * DAY)
YEAR = (12 * MONTH)                                                                              
*/
extern uint64 UnixTimeToWin32Time( time_t sec, uint32 nsec );
extern uint64 Win32TimeNow();
extern void Win32TimeToUnixTime( uint64 win32t, time_t &unix_time, uint32 &nsec );
extern std::string Win32TimeToString(uint64 win32t);
extern size_t strcpy_fake_unicode(wchar_t *into, const char *from);

static const uint64 Win32Time_Second = 10000000L;
static const uint64 Win32Time_Minute = Win32Time_Second*60;
static const uint64 Win32Time_Hour = Win32Time_Minute*60;
static const uint64 Win32Time_Day = Win32Time_Hour*24;
static const uint64 Win32Time_Month = Win32Time_Day*30;	//according to the eve client
static const uint64 Win32Time_Year = Win32Time_Month*12;	//according to the eve client

//these may be later converted to classes, but now it's easier this way
//makes ccp_exceptions.UserError exception
extern PyRep *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args = std::map<std::string, PyRep *>());
//makes UserError with type "CustomError"
extern PyRep *MakeCustomError(const char *fmt, ...);

/**
 * Returns length of field of given type.
 *
 * @param[in] type Type of fields.
 * @return Length of field (in bits).
 */
extern uint8 DBTYPE_SizeOf(DBTYPE type);
/**
 * Checks compatibility between DBTYPE and PyRep.
 *
 * @param[in] txpe DBTYPE to check.
 * @param[in] rep PyRep to check.
 * @return True if arguments are compatible, false if not.
 */
extern bool DBTYPE_IsCompatible(DBTYPE type, const PyRep &rep);

#endif



