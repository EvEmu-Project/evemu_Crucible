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


#ifndef __CONFIG_SERVICE_H_INCL__
#define __CONFIG_SERVICE_H_INCL__

#include "../PyService.h"
#include "ConfigDB.h"
#include <vector>

class ConfigService : public PyService {
public:
	ConfigService(PyServiceMgr *mgr, DBcore *db);
	virtual ~ConfigService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	ConfigDB m_db;

	PyCallable_DECL_CALL(GetMultiOwnersEx)
	PyCallable_DECL_CALL(GetMultiLocationsEx)
	PyCallable_DECL_CALL(GetMultiAllianceShortNamesEx)
	PyCallable_DECL_CALL(GetMultiCorpTickerNamesEx)
	PyCallable_DECL_CALL(GetUnits)
	PyCallable_DECL_CALL(GetMapObjects)
	PyCallable_DECL_CALL(GetMap)
	PyCallable_DECL_CALL(GetMapConnections)
	PyCallable_DECL_CALL(GetMultiGraphicsEx)
	PyCallable_DECL_CALL(GetMultiInvTypesEx)
	PyCallable_DECL_CALL(GetStationSolarSystemsByOwner)
	PyCallable_DECL_CALL(GetCelestialStatistic)
	
};





#endif


