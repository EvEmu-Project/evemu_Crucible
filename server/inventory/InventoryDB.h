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


#ifndef __INVENTORYDB_H_INCL__
#define __INVENTORYDB_H_INCL__

#include "../ServiceDB.h"
#include "../common/tables/invGroups.h"
#include "../common/tables/invCategories.h"

class EVEAttributeMgr;

class InventoryDB
: public ServiceDB {
public:
	InventoryDB(DBcore *db);
	virtual ~InventoryDB();

	/*
	 * Type stuff
	 */
	bool GetCategory(EVEItemCategories category,
		std::string &name, std::string &desc, bool &published);

	bool GetGroup(uint32 groupID, EVEItemCategories &category,
		std::string &name, std::string &desc, bool &useBasePrice, bool &allowManufacture, bool &allowRecycler,
		bool &anchored, bool &anchorable, bool &fittableNonSingleton, bool &published);

	bool GetType(uint32 typeID, uint32 &groupID,
		std::string &name, std::string &desc, double &radius, double &mass, double &volume, double &capacity, uint32 &portionSize,
		EVERace &raceID, double &basePrice, bool &published, uint32 &marketGroupID, double &chanceOfDuplicating);

	bool GetBlueprintType(uint32 blueprintTypeID,
		uint32 &parentBlueprintTypeID, uint32 &productTypeID, uint32 &productionTime, uint32 &techLevel, uint32 &researchProductivityTime,
		uint32 &researchMaterialTime, uint32 &researchCopyTime, uint32 &researchTechTime, uint32 &productivityModifier,
		uint32 &materialModifier, double &wasteFactor, double &chanceOfReverseEngineering, uint32 &maxProductionLimit);

	// attributes
	bool LoadTypeAttributes(uint32 typeID, EVEAttributeMgr &into);

	/*
	 * Item stuff
	 */
	bool GetItem(uint32 itemID,
		std::string &name, uint32 &typeID, uint32 &ownerID, uint32 &locationID, EVEItemFlags &flag, bool &contraband, bool &singleton,
		uint32 &quantity, GPoint &position, std::string &customInfo);

	uint32 NewItem(const char *name, uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, bool contraband, bool singleton, uint32 quantity, const GPoint &pos, const char *customInfo);
	bool SaveItem(uint32 itemID, const char *name, uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, bool contraband, bool singleton, uint32 quantity, const GPoint &pos, const char *customInfo);
	bool DeleteItem(uint32 itemID);

	/*
	 * Blueprint stuff
	 */
	bool GetBlueprint(uint32 blueprintID,
		bool &copy, uint32 &materialLevel, uint32 &productivityLevel, int32 &licensedProductionRunsRemaining);

	bool NewBlueprint(uint32 blueprintID, bool copy, uint32 materialLevel, uint32 productivityLevel, int32 licensedProductionRunsRemaining);
	bool SaveBlueprint(uint32 blueprintID, bool copy, uint32 materialLevel, uint32 productivityLevel, int32 licensedProductionRunsRemaining);
	bool DeleteBlueprint(uint32 blueprintID);

	// attributes
	bool LoadItemAttributes(uint32 itemID, EVEAttributeMgr &into);
	bool UpdateAttribute_int(uint32 itemID, uint32 attributeID, int v);
	bool UpdateAttribute_double(uint32 itemID, uint32 attributeID, double v);
	bool EraseAttribute(uint32 itemID, uint32 attributeID);
	bool EraseAttributes(uint32 itemID);

	// other
	bool GetItemContents(InventoryItem *item, std::vector<uint32> &items);
};





#endif


