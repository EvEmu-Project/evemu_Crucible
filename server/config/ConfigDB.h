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


#ifndef __CONFIGDB_H_INCL__
#define __CONFIGDB_H_INCL__

#include "../ServiceDB.h"
#include <vector>

class PyRep;

class ConfigDB
: public ServiceDB {
public:
	ConfigDB(DBcore *db);
	virtual ~ConfigDB();

	PyRep *GetMultiOwnersEx(const std::vector<uint32> &entityIDs);
	PyRep *GetMultiLocationsEx(const std::vector<uint32> &entityIDs);
	PyRep *GetMultiAllianceShortNamesEx(const std::vector<uint32> &entityIDs);
	PyRep *GetMultiCorpTickerNamesEx(const std::vector<uint32> &entityIDs);
	PyRep *GetMultiGraphicsEx(const std::vector<uint32> &entityIDs);
	PyRep *GetMultiInvTypesEx(const std::vector<uint32> &typeIDs);
	PyRepObject *GetUnits();
	PyRepObject *GetMapObjects(uint32 entityID, bool wantRegions, bool wantConstellations, bool wantSystems, bool wantStations);
	PyRepObject *GetMap(uint32 solarSystemID);
	PyRepObject *ListLanguages();
	PyRep *GetStationSolarSystemsByOwner(uint32 ownerID);
	PyRep *GetCelestialStatistic(uint32 celestialID);
	PyRep *GetTextsForGroup(const std::string & langID, uint32 textgroup);
	
	
protected:
};

#endif



