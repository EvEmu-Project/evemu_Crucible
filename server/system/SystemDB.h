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

#ifndef __SYSTEMDB_H_INCL__
#define __SYSTEMDB_H_INCL__

#include "../ServiceDB.h"
#include "../common/gpoint.h"
#include <string>
#include <vector>

class DBSystemEntity {
public:
	uint32 itemID;
	uint32 typeID;
	uint32 groupID;
	uint32 orbitID;
	GPoint position;
	double radius;
	double security;
	std::string itemName;
};

class DBSystemDynamicEntity {
public:
	uint32 itemID;
	uint32 typeID;
	uint32 groupID;
	uint32 categoryID;
};

class SystemDB
: public ServiceDB {
public:
	SystemDB(DBcore *db);
	virtual ~SystemDB();
	
	bool LoadSystemEntities(uint32 systemID, std::vector<DBSystemEntity> &into);
	bool LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity> &into);
	std::string GetSystemSecurity(uint32 sysid);

	PyRepObject *ListFactions();
	PyRepObject *ListJumps(uint32 stargateID);
	
protected:
};





#endif


