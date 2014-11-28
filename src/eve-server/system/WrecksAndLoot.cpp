/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Aknor Jaden, Allan
*/

#include "eve-server.h"

#include "system/WrecksAndLoot.h"
#include "system/SystemDB.h"


// ////////////////////// DGM_Types_to_Wrecks_Table Class ////////////////////////////
DGM_Types_to_Wrecks_Table::DGM_Types_to_Wrecks_Table()
{
    m_WrecksToTypesMap.clear();
}

DGM_Types_to_Wrecks_Table::~DGM_Types_to_Wrecks_Table()
{
}

int DGM_Types_to_Wrecks_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Types_to_Wrecks_Table::_Populate()
{
    uint32 wreckID, typeID;

    //first get list of all effects from dgmEffects table
    DBQueryResult *res = new DBQueryResult();
    SystemDB::GetWrecksToTypes(*res);

    //counter
	uint32 total_wreck_count = 0;
	uint32 error_count = 0;

	//go through and populate each effect
    DBResultRow row;
    while( res->GetRow(row) )
    {
        typeID = row.GetInt(0);
        wreckID = row.GetInt(1);
		m_WrecksToTypesMap.insert(std::pair<uint32, uint32>(typeID,wreckID));

		total_wreck_count++;
    }

	sLog.Log("DGM_Types_to_Wrecks_Table", "..........%u total wreck objects loaded", total_wreck_count);

    //cleanup
    delete res;
    res = NULL;
}

uint32 DGM_Types_to_Wrecks_Table::GetWreckID(uint32 typeID)
{
    std::map<uint32, uint32>::iterator mWrecksMapIterator;

    if( (mWrecksMapIterator = m_WrecksToTypesMap.find(typeID)) == m_WrecksToTypesMap.end() )
        return 0;
    else
    {
        return mWrecksMapIterator->second;
    }
}

// ////////////////////// DGM_Loot_Groups_Table Class ////////////////////////////
DGM_Loot_Groups_Table::DGM_Loot_Groups_Table()
{
    m_LootGroupMap.clear();
    m_LootGroupTypeMap.clear();
}

DGM_Loot_Groups_Table::~DGM_Loot_Groups_Table()
{
    m_LootGroupMap.clear();
    m_LootGroupTypeMap.clear();
}

int DGM_Loot_Groups_Table::Initialize()
{
    _Populate();
    return 1;
}

void DGM_Loot_Groups_Table::_Populate()
{
    DBQueryResult *res = new DBQueryResult();

    //first get all loot groups from LootGroup table
    SystemDB::GetLootGroups(*res);
    DBResultRow row;
    DBLootGroup LootGroup;
    while( res->GetRow(row) ) {
        LootGroup.groupID = row.GetInt(0);
        LootGroup.lootGroupID = row.GetInt(1);
        LootGroup.dropChance = row.GetDouble(2);
        m_LootGroupMap.push_back(LootGroup);
    }

    res->Reset();

    //second get all types from LootGroupTypes table
    SystemDB::GetLootGroupTypes(*res);
    DBLootGroupType GroupType;
    while( res->GetRow(row) ) {
        GroupType.lootGroupID = row.GetInt(0);
        GroupType.typeID =  row.GetInt(1);
        GroupType.chance = row.GetDouble(2);
        GroupType.minQuantity = row.GetInt(3);
        GroupType.maxQuantity = row.GetInt(4);
        m_LootGroupTypeMap.push_back(GroupType);
    }

    sLog.Log("       Loot_Table", "%u loot group and %u loot type definitions loaded", m_LootGroupMap.size(), m_LootGroupTypeMap.size());
}

void DGM_Loot_Groups_Table::GetLoot(uint32 groupID, LootGroupTypeDef &lootList) {
    double start = GetTimeMSeconds();
    double randChance = 0.0;

    DBLootGroupType loot_list1;
    LootGroupTypeDef loot_list2;

    LootGroupItr curGroupItr = m_LootGroupMap.begin();
    LootGroupTypeItr curTypeItr = m_LootGroupTypeMap.begin();

    while (curGroupItr != m_LootGroupMap.end()) {
        if (curGroupItr->groupID == groupID) {
            randChance = gen_random_float(0.00, 1.00);      // used to determine initial loot groups
            if (randChance < curGroupItr->dropChance) {
                while (curTypeItr != m_LootGroupTypeMap.end()) {
                    if (curTypeItr->lootGroupID == curGroupItr->lootGroupID) {
                        loot_list1.lootGroupID = curTypeItr->lootGroupID;
                        loot_list1.typeID = curTypeItr->typeID;
                        loot_list1.chance = curTypeItr->chance;
                        loot_list1.minQuantity = curTypeItr->minQuantity;
                        loot_list1.maxQuantity = curTypeItr->maxQuantity;
                        loot_list2.push_back(loot_list1);
                    }
                    ++curTypeItr;
                }
                curTypeItr = m_LootGroupTypeMap.begin();
            }
        }
        ++curGroupItr;
    }

    if (!loot_list2.empty()) {
        uint16 group = 0;
        float random = gen_random_float(0, 1);
        float lootChance = 0.0f;
        DBLootGroupType loot_list;
        LootGroupTypeItr curLootListItr = loot_list2.begin();

        /*  chance here is additive.  if the first item doesnt meet the chance, the next item in the list is loaded
         * then THAT chance is added to the first, then checked against the randChance roll, and looped again.
         * with multiple items in the group, there must be ONE and ONLY ONE item from the group chosen.
         * to do this, once the item is chosen, the groupID is saved in the 'group' variable, and the loop continues.
         * if the current item belongs to the same group as an item already chosen for that group, it is bypassed,
         * and the loop continues.
         * the random chance is rolled ONCE for each group, and reset (along with lootChance variable) after an item
         * is chosen from the current group.  after an item is chosen, it is inserted into a vector, then this vector
         * is then sent to the caller (in Damage.cpp) for addition into the wreck container
         */

        while (curLootListItr != loot_list2.end()) {
            if (curLootListItr->lootGroupID != group) {
                lootChance += curLootListItr->chance;
                if (random < lootChance) {
                    loot_list.lootGroupID = curLootListItr->lootGroupID;
                    loot_list.typeID = curLootListItr->typeID;
                    loot_list.chance = curLootListItr->chance;
                    loot_list.minQuantity = curLootListItr->minQuantity;
                    loot_list.maxQuantity = curLootListItr->maxQuantity;
                    lootList.push_back(loot_list);
                    // reset for next loot group
                    lootChance = 0;
                    group = curLootListItr->lootGroupID;
                    random = gen_random_float(0, 1);
                }
            }
            ++curLootListItr;
        }
    }

    double timer = (GetTimeMSeconds() - start);
    sLog.Log("        GetLoot()", "Took %f s to iterate thru %u loops, with %u loot items passed and %u returned", timer, m_LootGroupMap.size(), loot_list2.size(), lootList.size());
}
