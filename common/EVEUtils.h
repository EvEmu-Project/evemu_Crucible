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

#ifndef __EVEUTILS_H__
#define __EVEUTILS_H__

#include <string>
#include <vector>
#include <map>

#include "PyRep.h"

class InventoryItem;

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
extern int strcpy_fake_unicode(uint16 *into, const char *from);

static const uint64 Win32Time_Second = 10000000L;
static const uint64 Win32Time_Minute = Win32Time_Second*60;
static const uint64 Win32Time_Hour = Win32Time_Minute*60;
static const uint64 Win32Time_Day = Win32Time_Hour*24;
static const uint64 Win32Time_Month = Win32Time_Day*30;	//according to the eve client
static const uint64 Win32Time_Year = Win32Time_Month*12;	//according to the eve client

int GetSkillLevel(const std::vector<const InventoryItem *> &skills, const uint32 skillID);

//these may be later converted to classes, but now it's easier this way
//makes ccp_exceptions.UserError exception
PyRep *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args = std::map<std::string, PyRep *>());
//makes UserError with type "CustomError"
PyRep *MakeCustomError(const char *fmt, ...);

#endif



