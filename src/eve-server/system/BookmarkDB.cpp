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
	Author:		Bloody.Rabbit
*/

#include "EVEServerPCH.h"


PyObject *BookmarkDB::GetBookmarks(uint32 ownerID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" bookmarkID,"
		" ownerID,"
		" itemID,"
		" typeID,"
		" flag,"
		" memo,"
		" created,"
		" x, y, z,"
		" locationID"
		" FROM bookmarks"
		" WHERE ownerID = %u",
		ownerID))
	{
        sLog.Error( "BookmarkDB::GetBookmarks()", "Failed to query bookmarks for owner %u: %s.", ownerID, res.error.c_str() );
		return(NULL);
	}

	return DBResultToRowset(res);
}


uint32 BookmarkDB::GetNextAvailableBookmarkID()
{
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	bookmarkID "
		" FROM bookmarks "
		" WHERE bookmarkID >= %u ", 0))
	{
        sLog.Error( "BookmarkDB::GetNextAvailableBookmarkID()", "Error in query: %s", res.error.c_str() );
		return 0;
	}

	uint32 currentBookmarkID = 0;

	// Traverse through the rows in the query result until the first gap is found
	// and return the value that would be first (or only one) in the gap as the next
	// free bookmark ID:
	DBResultRow row;
	while( res.GetRow(row) )
	{
        const uint32 bookmarkID = row.GetUInt( 0 );

        if( currentBookmarkID < bookmarkID )
            return currentBookmarkID;

        ++currentBookmarkID;
	}

        // Check to make sure that the next available bookmarkID is not equal to the Maximum bookmarkID value
	if( currentBookmarkID <= BookmarkService::MAX_BOOKMARK_ID )
        return currentBookmarkID;
	else
        return 0;	// No free bookmarkIDs found (this should never happen as there are way too many IDs to exhaust)
}


uint32 BookmarkDB::FindBookmarkTypeID(uint32 itemID)
{
	DBQueryResult res, res2;
	DBResultRow row, row2;

    // Find the correct TypeID of the item specified by itemID, searching through
    // several places: entity table, mapDenormalize table, and mapJumps table:

    // The most common bookmark made is a location in space, so let's check this first and exit
    // quickly for this high use case:
	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	typeID "
		" FROM entity "
		" WHERE itemID = %u ", itemID))
	{
        sLog.Error( "BookmarkDB::FindBookmarkTypeID()", "Error in query: %s", res.error.c_str() );
		return 0;
	}

	if (res.GetRow(row))
    {
        // itemID exists in 'entity' table, now let's check to see what type it is:
	    if (!sDatabase.RunQuery(res2, 
		    " SELECT "
		    "	groupID "
		    " FROM invTypes "
            " WHERE typeID = %u ", row.GetUInt(0)))
	    {
            sLog.Error( "BookmarkDB::FindBookmarkTypeID()", "Error in query: %s", res2.error.c_str() );
		    return 0;
	    }

        if (res2.GetRow(row2))
            if (row2.GetUInt(0) == 15)
                return ( row.GetUInt(0) );  // Return typeID of "Station" from 'entity'

        return 5;   // Return typeID of "Solar System" from 'invTypes'
    }

    // No match yet, so next let's check the 'mapDenormalize' table for a Sun, Planet, Moon, Asteroid Belt, or Stargate:
	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	typeID, groupID "
		" FROM mapDenormalize "
		" WHERE itemID = %u ", itemID))
	{
        sLog.Error( "BookmarkDB::FindBookmarkTypeID()", "Error in query: %s", res.error.c_str() );
		return 0;
	}

	if (res.GetRow(row))
        return ( row.GetUInt(0) );

    // No matches whatsoever, so this is unknown, return 0:
    return 0;
}


bool BookmarkDB::GetBookmarkInformation(uint32 bookmarkID, uint32 &ownerID, uint32 &itemID, uint32 &typeID,
                                        uint32 &flag, std::string &memo, uint64 &created, double &x, double &y,
                                        double &z, uint32 &locationID)
{
	DBQueryResult res;
   	DBResultRow row;

    // Query database 'bookmarks' table for the supplied bookmarkID and retrieve entire row:
	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	bookmarkID, "
        "   ownerID, "
        "   itemID, "
        "   typeID, "
        "   flag, "
        "   memo, "
        "   created, "
        "   x, "
        "   y, "
        "   z, "
        "   locationID "
		" FROM bookmarks "
		" WHERE bookmarkID = %u ", bookmarkID))
	{
        sLog.Error( "BookmarkDB::GetBookmarkInformation()", "Error in query: %s", res.error.c_str() );
		return false;
	}

    // Query went through, but check to see if there were zero rows, ie bookmarkID was invalid:
	if ( !(res.GetRow(row)) )
        return false;

    // Bookmark 'bookmarkID' was found, Send back bookmark information:
    ownerID = row.GetUInt(1);
    itemID = row.GetUInt(2);
    typeID = row.GetUInt(3);
    flag = row.GetUInt(4);
    memo = row.GetText(5);
    created = row.GetUInt64(6);
    x = row.GetDouble(7);
    y = row.GetDouble(8);
    z = row.GetDouble(9);
    locationID = row.GetUInt(10);

    return true;
}


bool BookmarkDB::SaveNewBookmarkToDatabase(uint32 &bookmarkID, uint32 ownerID, uint32 itemID,
                               uint32 typeID, uint32 flag, std::string memo, uint64 created,
                               double x, double y, double z, uint32 locationID)
{
	DBQueryResult res;
	DBerror err;
	DBResultRow row;

    bookmarkID = GetNextAvailableBookmarkID();

	if (!sDatabase.RunQuery(err,
		" INSERT INTO bookmarks "
		" (bookmarkID, ownerID, itemID, typeID, flag, memo, created, x, y, z, locationID)"
        " VALUES (%u, %u, %u, %u, %u, '%s', "I64u", %f, %f, %f, %u) ",
        bookmarkID, ownerID, itemID, typeID, flag, memo.c_str(), created, x, y, z, locationID
		))
	{
        sLog.Error( "BookmarkDB::SaveNewBookmarkToDatabase()", "Error in query, Bookmark content couldn't be saved: %s", err.c_str() );
		return 0;
	}
	else
		return 1;
}


bool BookmarkDB::DeleteBookmarksFromDatabase(uint32 ownerID, std::vector<unsigned long> * bookmarkList)
{
	DBerror err;
	bool ret = true;
    std::stringstream st;
    std::string listString;
    uint32 i, size;

    size = bookmarkList->size();
    for(i=0; i<size; i++)
    {
        st << bookmarkList->at(i);
        if (i<(size-1))
            st << ", ";
    }

	if (!sDatabase.RunQuery(err,
		" DELETE FROM bookmarks "
        " WHERE ownerID = %u AND bookmarkID IN (%s)", ownerID, st.str().c_str()
		))
	{
        sLog.Error( "BookmarkDB::DeleteBookmarksFromDatabase()", "Error in query: %s", err.c_str() );
		ret = false;
	}

	return ret;
}


bool BookmarkDB::UpdateBookmarkInDatabase(uint32 bookmarkID, uint32 ownerID, std::string memo)
{
	DBerror err;
    std::string new_password;

	if (!sDatabase.RunQuery(err,
		" UPDATE bookmarks "
		" SET "
        " memo = '%s', "
        " WHERE bookmarkID = %u AND ownerID = %u",
        memo.c_str(),
        bookmarkID,
        ownerID
		))
	{
        sLog.Error( "BookmarkDB::UpdateBookmarkInDatabase()", "Error in query: %s", err.c_str() );
		return false;
	}

	return true;
}
