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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

// Set the maximum number for any user-created bookmark.
const uint32 BookmarkService::MAX_BOOKMARK_ID = 0xFFFFFFFF;

PyCallable_Make_InnerDispatcher(BookmarkService)


BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(BookmarkService, GetBookmarks)
	PyCallable_REG_CALL(BookmarkService, BookmarkLocation)
    PyCallable_REG_CALL(BookmarkService, DeleteBookmarks)
    PyCallable_REG_CALL(BookmarkService, UpdateBookmark)

    nextBookmarkID = 1000;
}


BookmarkService::~BookmarkService() {
	delete m_dispatch;
}


bool BookmarkService::LookupBookmark(uint32 characterID, uint32 bookmarkID, uint32 &itemID, uint32 &typeID, double &x, double &y, double &z)
{
    // Retrieve bookmark information for external use:
    uint32 ownerID;
    uint32 flag;
    std::string memo;
    uint64 created;
    uint32 locationID;

    return m_db.GetBookmarkInformation(bookmarkID,ownerID,itemID,typeID,flag,memo,created,x,y,z,locationID);
}


PyResult BookmarkService::Handle_GetBookmarks(PyCallArgs &call) {
	return(m_db.GetBookmarks(call.client->GetCharacterID()));
}


PyResult BookmarkService::Handle_BookmarkLocation(PyCallArgs &call)
{
    std::string label;
    std::string note;
    std::string memo;
    uint32 bookmarkID = 0;
    uint32 ownerID = call.client->GetCharacterID();
    uint32 itemID = 0;
    uint32 typeID = 0;
    uint32 flag = 0;
    uint64 created = 0;
    uint32 locationID = 0;
    GPoint point;

    ////////////////////////////////////////
    // Verify expected packet structure:
    //
    // call.tuple
    //       |
    //       |--> [0] PyInt:      itemID for bookmark location (itemID from entity table, stargateID from mapjumps table, etc)
    //       |--> [1] PyNone:     (unknown)
    //       |--> [2] PyWString:  label for the bookmark
    //       \--> [3] PyWString:  text for the "note" field in the bookmark (NOTE: this type will be PyString when it equals "")
    ////////////////////////////////////////

    if ( !(call.tuple->IsTuple()) )
    {
		sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple is of the wrong type: '%s'.  Expected PyTuple type.", call.client->GetName(), call.tuple->TypeString() );
		return NULL;
    }

    if ( !(call.tuple->GetItem(0)->IsInt()) )
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(0) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->GetItem(0)->TypeString() );
		return NULL;
    }
    else
        itemID = call.tuple->GetItem( 0 )->AsInt()->value();

    if ( call.tuple->GetItem( 2 )->IsString() )
        label = call.tuple->GetItem( 2 )->AsString()->content();
    else if ( call.tuple->GetItem( 2 )->IsWString() )
        label = call.tuple->GetItem( 2 )->AsWString()->content();
    else
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(2) is of the wrong type: '%s'.  Expected PyString or PyWString type.", call.client->GetName(), call.tuple->GetItem(2)->TypeString() );
		return NULL;
    }

    if ( call.tuple->GetItem( 3 )->IsString() )
        note = call.tuple->GetItem( 3 )->AsString()->content();
    else if ( call.tuple->GetItem( 3 )->IsWString() )
        note = call.tuple->GetItem( 3 )->AsWString()->content();
    else
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(3) is of the wrong type: '%s'.  Expected PyString or PyWString type.", call.client->GetName(), call.tuple->GetItem(3)->TypeString() );
		return NULL;
    }

    // Get typeID for this itemID:
    typeID = m_db.FindBookmarkTypeID(itemID);

    // Get x,y,z location in space of the character:
    point = call.client->GetPosition();

    // Get locationID for this character:
    locationID = call.client->GetLocationID();


    ////////////////////////////////////////
    // Save bookmark info to database:
    ////////////////////////////////////////
    flag = 0;                                           // Don't know what to do with this value
    created = Win32TimeNow();
    memo = label + note;
    m_db.SaveNewBookmarkToDatabase
    (
        bookmarkID,
        call.client->GetCharacterID(),
        itemID,
        typeID,
        flag,
        memo,
        created,
        point.x,
        point.y,
        point.z,
        locationID
    );


    ////////////////////////////////////////
	// Build return packet:
    //
    // (bookmarkID, itemID, typeID, x, y, z, locationID,) 
    ////////////////////////////////////////

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 7 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyNone();                      // in some cases, this is the itemID
    tuple0->items[ 2 ] = new PyInt( typeID );               // typeID from invtypes
    tuple0->items[ 3 ] = new PyInt( (uint32)(point.x) );    // X coordinate
    tuple0->items[ 4 ] = new PyInt( (uint32)(point.y) );    // Y coordinate
    tuple0->items[ 5 ] = new PyInt( (uint32)(point.z) );    // Z coordinate
    tuple0->items[ 6 ] = new PyInt( locationID );           // systemID

    res = tuple0;

    return res;
}


PyResult BookmarkService::Handle_DeleteBookmarks(PyCallArgs &call)
{
    if ( !(call.tuple->GetItem( 0 )->IsList()) )
    {
		sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "%s: call.tuple is of the wrong type: '%s'.  Expected PyList type.", call.client->GetName(), call.tuple->TypeString() );
		return NULL;
    }

    PyList *list = call.tuple->GetItem( 0 )->AsList();
    uint32 i;
    uint32 bookmarkID;
    std::vector<unsigned long> bookmarkIDs;

    if( list->size() > 0 )
    {
        for(i=0; i<(list->size()); i++)
        {
            bookmarkID = call.tuple->GetItem( 0 )->AsList()->GetItem(i)->AsInt()->value();
            bookmarkIDs.push_back( bookmarkID );
        }

        m_db.DeleteBookmarksFromDatabase( call.client->GetCharacterID(),&bookmarkIDs );
    }
    else
    {
		sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "%s: call.tuple->GetItem( 0 )->AsList()->size() == 0.  Expected size >= 1.", call.client->GetName() );
		return NULL;
    }

    return(new PyNone());
}


PyResult BookmarkService::Handle_UpdateBookmark(PyCallArgs &call)
{
    uint32 bookmarkID;
    uint32 ownerID;
    uint32 itemID;
    uint32 typeID;
    uint32 flag;
    std::string memo;
    uint64 created;
    double x;
    double y;
    double z;
    uint32 locationID;
    std::string newLabel;
    std::string newNote;

    // bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value()
    if ( !(call.tuple->GetItem( 0 )->IsInt()) )
    {
		sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.tuple->GetItem( 0 ) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->GetItem( 0 )->TypeString() );
		return NULL;
    }
    else
        bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value();

    // newLabel = call.tuple->GetItem( 3 )->AsWString()->content()
    if ( !(call.tuple->GetItem( 3 )->IsWString()) )
    {
		sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.tuple->GetItem( 3 ) is of the wrong type: '%s'.  Expected PyWString type.", call.client->GetName(), call.tuple->GetItem( 3 )->TypeString() );
		return NULL;
    }
    else
        newLabel = call.tuple->GetItem( 3 )->AsWString()->content();

    //call.byname.find("note") == call.byname.end()
    //newNote = call.byname.find("note")->second->AsWString() when note != ""
    //newNote = call.byname.find("note")->second->AsString() when note == ""
    if( call.byname.find("note") == call.byname.end() )
    {
		sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.byname.find(\"note\") could not be found.", call.client->GetName() );
		return NULL;
    }
    else
    {
        if( call.byname.find("note")->second->IsWString() )
            newNote = call.byname.find("note")->second->AsWString()->content();
        else if( call.byname.find("note")->second->IsString() )
            newNote = call.byname.find("note")->second->AsString()->content();
        else
        {
		    sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.byname.find(\"note\")->second is of the wrong type: '%s'.  Expected PyWString or PyString type.", call.client->GetName(), call.byname.find("note")->second->TypeString() );
		    return NULL;
        }
    }

    // Take newLabel and newNote to create new 'memo' value:
    memo = newLabel + newNote;

    m_db.GetBookmarkInformation
    (
        bookmarkID,
        ownerID,
        itemID,
        typeID,
        flag,
        memo,
        created,
        x,
        y,
        z,
        locationID
    );

    m_db.UpdateBookmarkInDatabase(bookmarkID,call.client->GetCharacterID(),memo);

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 7 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyNone();                      // in some cases, this is the itemID
    tuple0->items[ 2 ] = new PyInt( typeID );               // typeID from invtypes
    tuple0->items[ 3 ] = new PyInt( (uint32)x );    // Sample X coordinate
    tuple0->items[ 4 ] = new PyInt( (uint32)y );    // Sample Y coordinate
    tuple0->items[ 5 ] = new PyInt( (uint32)z );    // Sample Z coordinate
    tuple0->items[ 6 ] = new PyInt( locationID );           // Sample systemID

    res = tuple0;

    return res;
}
