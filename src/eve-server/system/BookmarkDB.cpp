/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Bloody.Rabbit
    Updated:        Allan (Zhy)   18Jan14
    Again:      Allan 2Feb20 (Bookmark system overhaul)
*/


#include "eve-server.h"

#include "system/BookmarkDB.h"
#include "system/BookmarkService.h"

PyRep* BookmarkDB::GetBookmarksInFolder(uint32 folderID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  bookmarkID,"
        "  folderID"
        " FROM bookmarks"
        " WHERE folderID = %u",
        folderID))
    {
        sLog.Error( "BookmarkDB::GetBMData()", "Failed to query bookmarks for folderID %u: %s.", folderID, res.error.c_str() );
        return nullptr;
    }

    //return DBResultToCRowset(res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
        dict->SetItemString("bookmarkID", new PyInt(row.GetInt(0)));
        if (row.IsNull(1) or (row.GetInt(1) == 0)) {
            dict->SetItemString("folderID", PyStatic.NewNone());
        } else {
            dict->SetItemString("folderID", new PyInt(row.GetInt(1)));
        }
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

void BookmarkDB::GetBookmarkByFolderID(int32 folderID, std::vector< int32 >& bmIDs)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT bookmarkID FROM bookmarks WHERE folderID = %u", folderID)) {
        sLog.Error( "BookmarkDB::GetBookmarkByFolderID()", "Failed to query bookmarks for folderID %u: %s.", folderID, res.error.c_str() );
        return;
    }

    DBResultRow row;
    while (res.GetRow(row))
        bmIDs.push_back(row.GetInt(0));
}

PyRep *BookmarkDB::GetBookmarks(uint32 ownerID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  bookmarkID,"
        "  ownerID,"
        "  itemID,"
        "  typeID,"
        "  memo,"
        "  created,"
        "  x, y, z,"
        "  locationID,"
        "  note,"
        "  creatorID,"
        "  folderID"
        " FROM bookmarks"
        " WHERE ownerID = %u",
        ownerID))
    {
        sLog.Error( "BookmarkDB::GetBookmarks()", "Failed to query bookmarks for owner %u: %s.", ownerID, res.error.c_str() );
        return nullptr;
    }

    // Corp bookmarks are read differently and should be as a dict of bookmarks with the key being bookmarkID
    if (IsCorp(ownerID)) {
        PyDict* corpBookmarks = new PyDict();
        DBResultRow row;
        while (res.GetRow(row)) {
            PyDict* dict = new PyDict();
            dict->SetItemString("bookmarkID", new PyInt(row.GetInt(0)));
            dict->SetItemString("ownerID", new PyInt(row.GetInt(1)));
            dict->SetItemString("itemID", new PyInt(row.GetInt(2)));
            dict->SetItemString("typeID", new PyInt(row.GetInt(3)));
            dict->SetItemString("memo", new PyString(row.GetText(4)));
            dict->SetItemString("created", new PyLong(row.GetInt64(5)));
            dict->SetItemString("x", new PyFloat(row.GetFloat(6)));
            dict->SetItemString("y", new PyFloat(row.GetFloat(7)));
            dict->SetItemString("z", new PyFloat(row.GetFloat(8)));
            dict->SetItemString("locationID", new PyInt(row.GetInt(9)));
            if (row.IsNull(10)) {
                dict->SetItemString("note", PyStatic.NewNone());
            } else {
                dict->SetItemString("note", new PyInt(row.GetInt(10)));
            }
            dict->SetItemString("creatorID", new PyInt(row.GetInt(11)));
            if (row.IsNull(12) or (row.GetInt(12) == 0)) {
                dict->SetItemString("folderID", PyStatic.NewNone());
            } else {
                dict->SetItemString("folderID", new PyInt(row.GetInt(12)));
            }
            corpBookmarks->SetItem(new PyInt(row.GetInt(0)), new PyObject("util.KeyVal", dict));
        }
        return corpBookmarks;
    }

    //return DBResultToCRowset(res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
        dict->SetItemString("bookmarkID", new PyInt(row.GetInt(0)));
        dict->SetItemString("ownerID", new PyInt(row.GetInt(1)));
        dict->SetItemString("itemID", new PyInt(row.GetInt(2)));
        dict->SetItemString("typeID", new PyInt(row.GetInt(3)));
        dict->SetItemString("memo", new PyString(row.GetText(4)));
        dict->SetItemString("created", new PyLong(row.GetInt64(5)));
        dict->SetItemString("x", new PyFloat(row.GetFloat(6)));
        dict->SetItemString("y", new PyFloat(row.GetFloat(7)));
        dict->SetItemString("z", new PyFloat(row.GetFloat(8)));
        dict->SetItemString("locationID", new PyInt(row.GetInt(9)));
        if (row.IsNull(10)) {
            dict->SetItemString("note", PyStatic.NewNone());
        } else {
            dict->SetItemString("note", new PyInt(row.GetInt(10)));
        }
        dict->SetItemString("creatorID", new PyInt(row.GetInt(11)));
        if (row.IsNull(12) or (row.GetInt(12) == 0)) {
            dict->SetItemString("folderID", PyStatic.NewNone());
        } else {
            dict->SetItemString("folderID", new PyInt(row.GetInt(12)));
        }
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

PyRep *BookmarkDB::GetFolders(uint32 ownerID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  ownerID,"
        "  folderID,"
        "  folderName,"
        "  creatorID"
        " FROM bookmarkFolders"
        " WHERE ownerID = %u",
        ownerID))
    {
        sLog.Error( "BookmarkDB::GetFolders()", "Failed to query bookmarks for owner %u: %s.", ownerID, res.error.c_str() );
        return nullptr;
    }

    //return DBResultToCRowset(res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
        dict->SetItemString("ownerID", new PyInt(row.GetInt(0)));
        dict->SetItemString("folderID", new PyInt(row.GetInt(1)));
        dict->SetItemString("folderName", new PyString(row.GetText(2)));
        dict->SetItemString("creatorID", new PyInt(row.GetInt(3)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

const char* BookmarkDB::GetBookmarkName(uint32 bookmarkID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT memo FROM bookmarks WHERE bookmarkID = %u", bookmarkID)) {
        sLog.Error( "BookmarkDB::GetBookmarkName()", "Failed to query bookmarkID %u: %s.", bookmarkID, res.error.c_str() );
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    return row.GetText(0);
}

PyTuple* BookmarkDB::GetBookmarkDescription(uint32 bookmarkID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT memo, note FROM bookmarks WHERE bookmarkID = %u", bookmarkID)) {
        sLog.Error( "BookmarkDB::GetBookmarkName()", "Failed to query bookmarkID %u: %s.", bookmarkID, res.error.c_str() );
        return nullptr;
    }

    PyTuple* tuple = new PyTuple(2);
    DBResultRow row;
    if (res.GetRow(row)) {
        tuple->SetItem(0, new PyString(row.GetText(0)));
        if (row.IsNull(1)) {
            tuple->SetItem(1, PyStatic.NewNone());
        } else {
            tuple->SetItem(1, new PyString(row.GetText(1)));
        }
    } else {
        tuple->SetItem(0, PyStatic.NewNone());
        tuple->SetItem(1, PyStatic.NewNone());
    }

    return tuple;
}

bool BookmarkDB::GetBookmarkInformation(uint32 bookmarkID, uint32& itemID, uint16& typeID, uint32& locationID, double& x, double& y, double& z)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID,"
        "  typeID,"
        "  locationID,"
        "  x, y, z"
        " FROM bookmarks "
        " WHERE bookmarkID = %u ", bookmarkID))
    {
        sLog.Error( "BookmarkDB::GetBookmarkInformation()", "Error in query: %s", res.error.c_str() );
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return false;

    itemID = row.GetUInt(0);
    typeID = row.GetUInt(1);
    locationID = row.GetUInt(2);
    x = row.GetDouble(3);
    y = row.GetDouble(4);
    z = row.GetDouble(5);

    return true;
}


void BookmarkDB::SaveNewBookmark( BmData& data )
{
    std::string eMemo, eNote;
    sDatabase.DoEscapeString(eMemo, data.memo.c_str());
    sDatabase.DoEscapeString(eNote, data.note.c_str());

    DBerror err;
    if (data.folderID > 0) {
        if (!sDatabase.RunQueryLID(err, data.bookmarkID,
            "INSERT INTO bookmarks"
            " (ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID, folderID)"
            " VALUES (%u, %u, %u, '%s', %f, %f, %f, %f, %u, '%s', %u, %u) ",
            data.ownerID, data.itemID, data.typeID, eMemo.c_str(), GetFileTimeNow(), data.point.x, data.point.y,
            data.point.z, data.locationID, eNote.c_str(), data.creatorID, data.folderID ))
        {
            sLog.Error( "BookmarkDB::SaveNewBookmarkToDatabase(1)", "Error in query, Bookmark content couldn't be saved: %s", err.c_str() );
        }
    } else {
        if (!sDatabase.RunQueryLID(err, data.bookmarkID,
            "INSERT INTO bookmarks"
            " (ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID)"
            " VALUES (%u, %u, %u, '%s', %f, %f, %f, %f, %u, '%s', %u) ",
            data.ownerID, data.itemID, data.typeID, eMemo.c_str(), GetFileTimeNow(), data.point.x, data.point.y,
            data.point.z, data.locationID, eNote.c_str(), data.creatorID ))
        {
            sLog.Error( "BookmarkDB::SaveNewBookmarkToDatabase(2)", "Error in query, Bookmark content couldn't be saved: %s", err.c_str() );
        }
    }
}

bool BookmarkDB::DeleteBookmark(uint32 ownerID, uint32 bookmarkID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "DELETE FROM bookmarks"
        " WHERE ownerID = %u AND bookmarkID = %u", ownerID, bookmarkID))
    {
        sLog.Error( "BookmarkDB::DeleteBookmarkFromDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

bool BookmarkDB::DeleteBookmarks(std::vector<int32>* bookmarkList)
{
    std::stringstream st;
    std::string listString;

    std::size_t size = bookmarkList->size();
    for (int8 i=0; i<size; ++i) {
        st << bookmarkList->at(i);
        if (i < (size-1))
            st << ", ";
    }

    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM bookmarks WHERE bookmarkID IN (%s)", st.str().c_str())) {
        sLog.Error( "BookmarkDB::DeleteBookmarksFromDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

void BookmarkDB::ChangeOwner(uint32 bookmarkID, uint32 ownerID/*1*/) {
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE bookmarks SET ownerID = %u WHERE bookmarkID = %u", ownerID, bookmarkID);
}


bool BookmarkDB::UpdateBookmark(uint32 bookmarkID, uint32 ownerID, std::string memo, std::string note, uint32 folderID/*0*/)
{
    std::string memo_fixed = "";
    sDatabase.DoEscapeString(memo_fixed, memo.c_str());

    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE bookmarks SET memo = '%s', note = '%s', folderID = %u WHERE bookmarkID = %u AND ownerID = %u",
        memo_fixed.c_str(), note.c_str(), folderID, bookmarkID, ownerID))
    {
        sLog.Error( "BookmarkDB::UpdateBookmarkInDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

uint32 BookmarkDB::SaveNewFolder(std::string folderName, uint32 ownerID)
{
    uint32 folderID(0);
    std::string eName = "";
    sDatabase.DoEscapeString( eName, folderName.c_str());

    DBerror err;
    if (!sDatabase.RunQueryLID(err, folderID, "INSERT INTO bookmarkFolders (folderName, ownerID, creatorID) VALUES ('%s', %u, %u) ",
                            eName.c_str(), ownerID, ownerID ))
    {
        sLog.Error( "BookmarkDB::SaveNewFolderToDatabase()", "Error in query, Folder couldn't be saved: %s", err.c_str() );
        return 0;
    }

    return folderID;
}

bool BookmarkDB::UpdateFolder(int32 folderID, std::string folderName)
{
    std::string eName = "";
    sDatabase.DoEscapeString( eName, folderName.c_str());

    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE bookmarkFolders SET folderName = '%s' WHERE folderID = %u",
                               eName.c_str(), folderID))
    {
        sLog.Error( "BookmarkDB::UpdateFolder()", "Error in query, Folder couldn't be saved: %s", err.c_str() );
        return false;
    }

    return true;
}

bool BookmarkDB::DeleteFolder(int32 folderID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM bookmarkFolders WHERE folderID = %u", folderID))
        sLog.Error( "BookmarkDB::DeleteFolder(1)", "Error in query: %s", err.c_str() );

    // these bms may have copies....cannot delete yet
    if (!sDatabase.RunQuery(err, "UPDATE bookmarks SET ownerID = 1 WHERE folderID = %u", folderID))
        sLog.Error( "BookmarkDB::DeleteFolder(2)", "Error in query: %s", err.c_str() );

    return true;
}

void BookmarkDB::MoveBookmarkToFolder(int32 folderID, std::vector<int32>* bookmarkList)
{
    std::stringstream st;
    std::size_t size = bookmarkList->size();
    for (int8 i=0; i<size; ++i) {
        st << bookmarkList->at(i);
        if (i < (size-1))
            st << ", ";
    }

    DBerror err;
    sDatabase.RunQuery(err, "UPDATE bookmarks SET folderID = %i WHERE bookmarkID IN (%s)", folderID, st.str().c_str());
}

void BookmarkDB::GetVoucherData( BmData& data ) {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT ownerID, itemID, typeID, memo, created, x, y, z, "
        "locationID, note, creatorID, folderID "
        "FROM bookmarks WHERE bookmarkID = %u", data.bookmarkID))
    {
        sLog.Error( "BookmarkDB::GetVoucherData()", "Failed to query data for Bookmark %u: %s.", data.bookmarkID, res.error.c_str() );
        return;
    }


    DBResultRow row;
    if (res.GetRow(row)) {
        data.ownerID = row.GetUInt(0);
        data.itemID = row.GetUInt(1);
        data.typeID = row.GetUInt(2);
        data.memo = row.GetText(3);
        data.created = row.GetInt64(4);
        data.point.x = row.GetFloat(5);
        data.point.y = row.GetFloat(6);
        data.point.z = row.GetFloat(7);
        data.locationID = row.GetUInt(8);
        if (row.IsNull(9)) {
            data.note = "";
        } else {
            data.note = row.GetText(9);
        }
        data.creatorID = row.GetUInt(10);
        if (row.IsNull(11)){
            data.folderID = 0;
        } else {
            data.folderID = row.GetUInt(11);
        }
    }
}
