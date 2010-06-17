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
	Author:		Captnoord
*/

#include "EVEServerPCH.h"
#include "ship/dgmtypeattributeinfo.h"

dgmtypeattributemgr::dgmtypeattributemgr()
{
    // load shit from db
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,"SELECT * FROM dgmtypeattributes ORDER BY typeID")) {
        sLog.Error("DgmTypeAttrMgr", "Error in db load query: %s", res.error.c_str());
        return;
    }
    
    uint32 currentID = 0;
    DgmTypeAttributeSet * entry = NULL;
    DBResultRow row;
    //while(1) {
        /*SpawnGroup *g = new SpawnGroup(
            row.GetUInt(0),
            row.GetText(1),
            row.GetUInt(2)
            );
        into[ g->id ] = g;*/
        /*uint32 typeID = row.GetUInt(0);
        if (typeID != currentID)
        {
            // hack check so I don't have todo complicated shit
            if (currentID != 0)
                mDgmTypeAttrInfo.insert(std::make_pair(currentID, entry)); // we have a different entry now so store the current one hehe
            
            entry = new DgmTypeAttributeSet;
            currentID = typeID;
        }*/

        /*entry = new DgmTypeAttributeSet;

        while(res.GetRow(row)) {
            if(row.GetUInt(0) != currentID && currentID != 0)
                break;
            currentID = row.GetUInt(0);
            DmgTypeAttribute * attr_entry = new DmgTypeAttribute;
            attr_entry->attributeID = row.GetUInt(1);
            if (row.IsNull(2) == true) {
                attr_entry->number = EvilNumber(row.GetFloat(3));
            } else {
                attr_entry->number = EvilNumber(row.GetInt(2));
            }

            entry->attributeset.push_back(attr_entry);
        }

        mDgmTypeAttrInfo.insert(std::make_pair(currentID, entry));
    }*/

    int amount = res.GetRowCount();
    for (int i = 0; i < amount; i++)
    {
        res.GetRow(row);
        uint32 typeID = row.GetUInt(0);

        // need a better sollution for this
        if (currentID == 0) {
            currentID = typeID;
            entry = new DgmTypeAttributeSet;
        }

        if (currentID != typeID) {
            mDgmTypeAttrInfo.insert(std::make_pair(currentID, entry));
            currentID = typeID;
            entry = new DgmTypeAttributeSet;
        }

        DmgTypeAttribute * attr_entry = new DmgTypeAttribute();
        attr_entry->attributeID = row.GetUInt(1);
        if (row.IsNull(2) == true) {
            attr_entry->number = EvilNumber(row.GetFloat(3));
        } else {
            attr_entry->number = EvilNumber(row.GetInt(2));
        }

        entry->attributeset.push_back(attr_entry);
    }
}

DgmTypeAttributeSet* dgmtypeattributemgr::GetDmgTypeAttributeSet( uint32 typeID )
{
    DgmTypeAttributeMapItr itr;
    itr = mDgmTypeAttrInfo.find(typeID);
    if (itr == mDgmTypeAttrInfo.end())
    {
        sLog.Error("DgmTypeAttrMgr", "unable to find typeID: %u", typeID);
        return NULL;
    }

    if (!itr->second)
    {
        sLog.Error("DgmTypeAttrMgr", "something went majorly wrong with typeID: %u, db problem.... maybe", typeID);
        return NULL;
    }

    // whooo we found it :D
    return itr->second;
}