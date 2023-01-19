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
    Author:        Zhur, Aknor Jaden
    Rewrite:    Allan
*/


#include "eve-server.h"



#include "packets/Bookmarks.h"
#include "system/BookmarkService.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"

/*
BOOKMARK__ERROR
BOOKMARK__WARNING
BOOKMARK__MESSAGE
BOOKMARK__DEBUG
BOOKMARK__INFO
BOOKMARK__TRACE
BOOKMARK__CALL_DUMP
BOOKMARK__RSP_DUMP
*/

BookmarkService::BookmarkService() :
    Service("bookmark")
{
    this->Add("GetBookmarks", &BookmarkService::GetBookmarks);
    this->Add("CreateFolder", &BookmarkService::CreateFolder);
    this->Add("UpdateFolder", &BookmarkService::UpdateFolder);
    this->Add("DeleteFolder", &BookmarkService::DeleteFolder);
    this->Add("BookmarkLocation", &BookmarkService::BookmarkLocation);
    this->Add("BookmarkScanResult", &BookmarkService::BookmarkScanResult);
    this->Add("DeleteBookmarks", &BookmarkService::DeleteBookmarks);
    this->Add("MoveBookmarksToFolder", &BookmarkService::MoveBookmarksToFolder);
    this->Add("AddBookmarkFromVoucher", &BookmarkService::AddBookmarkFromVoucher);
    this->Add("CopyBookmarks", &BookmarkService::CopyBookmarks);
}

bool BookmarkService::LookupBookmark(uint32 bookmarkID, uint32& itemID, uint16& typeID, uint32& locationID, double& x, double& y, double& z) {
    return m_db.GetBookmarkInformation(bookmarkID, itemID, typeID, locationID, x, y, z);
}

PyResult BookmarkService::GetBookmarks(PyCallArgs &call) {
    PyTuple* result = new PyTuple(2);
        result->SetItem(0, m_db.GetBookmarks(call.client->GetCharacterID()));
        result->SetItem(1, m_db.GetFolders(call.client->GetCharacterID()));
    result->Dump(BOOKMARK__RSP_DUMP, "    ");
    return result;
}

PyResult BookmarkService::CreateFolder(PyCallArgs &call, PyRep* name) {
    call.Dump(BOOKMARK__CALL_DUMP);
    /** @todo sanitize name */
    std::string nameStr = PyRep::StringContent(name);

    uint32 ownerID = call.client->GetCharacterID();
    Rsp_CreateFolder result;
        result.ownerID = ownerID;
        result.folderID = m_db.SaveNewFolder(nameStr, ownerID, ownerID);
        result.folderName = nameStr;
        result.creatorID = ownerID;

    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult BookmarkService::UpdateFolder(PyCallArgs &call, PyInt* folderID, PyRep* folderName) {
    call.Dump(BOOKMARK__CALL_DUMP);
    /** @todo sanitize name */
    if (!m_db.UpdateFolder(folderID->value(), PyRep::StringContent(folderName)))
        return PyStatic.NewFalse();

    return PyStatic.NewTrue();
}

PyResult BookmarkService::DeleteFolder(PyCallArgs &call, PyInt* folderID) {
    // bookmarks = self.bookmarkMgr.DeleteFolder(folderID)
    call.Dump(BOOKMARK__CALL_DUMP);

    // call db to get list of bmIDs in deleted folder.  return result with this data
    std::vector< int32 > bmIDs;
    bmIDs.clear();
    m_db.GetBookmarkByFolderID(folderID->value(), bmIDs);

    PyList* list = new PyList();
    for (auto cur : bmIDs) {
        PyDict* dict = new PyDict();
        dict->SetItemString("bookmarkID", new PyInt(cur));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    m_db.DeleteFolder(folderID->value());

    return list;
}

PyResult BookmarkService::BookmarkLocation(PyCallArgs& call, PyInt* itemID, PyInt* ownerID, PyRep* memo, PyRep* comment, std::optional<PyInt*> folderID) {
  /*  bookmarkID, itemID, typeID, x, y, z, locationID = sm.RemoteSvc('bookmark').BookmarkLocation(itemID, ownerID, memo, comment, folderID)  */

    call.Dump(BOOKMARK__CALL_DUMP);

    BmData data = BmData();
    data.memo = PyRep::StringContent(memo);
    data.note = PyRep::StringContent(comment);
    data.ownerID = ownerID->value();
    data.creatorID = data.ownerID;
    data.created = GetFileTimeNow();

    // Check for presence of folderID in the packet
    if (call.byname.find("folderID") != call.byname.end())
        data.folderID = PyRep::IntegerValueU32(call.byname.find("folderID")->second);

    if (IsPlayerItem(itemID->value())) {      // entity #'s above 140m are player-owned.  player is in ship
        data.typeID = EVEDB::invTypes::SolarSystem;
        data.point = call.client->GetShipSE()->GetPosition();       // Get x,y,z location.  bm type is coordinate as "spot in xxx system"
        data.locationID = call.client->GetLocationID();       // locationID of bm is current sol system
        data.itemID = data.locationID;      //  itemID = locationID for coord bm.  shows jumps, s/c/r in bm window, green in system
    } else if (sDataMgr.IsStation(itemID->value())) {  // not player-owned, check for station.
        SystemEntity* pSE = call.client->SystemMgr()->GetSE(itemID->value());
        if (pSE == nullptr) {
            // send player error also
            return nullptr;
        }
        data.typeID = pSE->GetTypeID();
        data.itemID = itemID->value();  // this is stationID (for bm description)
        data.locationID = call.client->GetSystemID();       // get sol system of current station
    } else {      // char is passing systemID from map.  char is marking a solar systemID for bm
        if (IsRegionID(itemID->value())) {
            data.typeID = EVEDB::invTypes::Region;
        } else if (IsConstellationID(itemID->value())) {
            data.typeID = EVEDB::invTypes::Constellation;
        } else if (sDataMgr.IsSolarSystem(itemID->value())) {
            data.typeID = EVEDB::invTypes::SolarSystem;
        }
        data.locationID = itemID->value();  // this is systemID from map
        data.itemID = data.locationID;      //  itemID = locationID for coord bm.  shows jumps, s/c/r in bm window, green in system
    }

    m_db.SaveNewBookmark(data);

    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    Rsp_BookmarkLocation result;
        result.bookmarkID  = data.bookmarkID;
        result.itemID      = (data.typeID == EVEDB::invTypes::SolarSystem ? 0 : data.itemID);     // itemID = 0 when typeID is SolarSystem
        result.typeID      = data.typeID;
        result.x           = data.point.x;
        result.y           = data.point.y;
        result.z           = data.point.z;
        result.locationID  = data.locationID;
    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult BookmarkService::BookmarkScanResult(PyCallArgs &call, PyInt* locationID, PyRep* memo, PyRep* comment, PyString* resultID, PyInt* ownerID, std::optional<PyInt*> folderID)
{
    //  bookmarkID, itemID, typeID, x, y, z, locationID = self.bookmarkMgr.BookmarkScanResult(locationID, memo, comment, resultID, ownerID, folderID=0)

    call.Dump(BOOKMARK__CALL_DUMP);
    //args.Dump(BOOKMARK__CALL_DUMP, "    ");
    /*
    * 22:25:58 [SvcCallDump]       Tuple: 5 elements
    * 22:25:58 [SvcCallDump]         [ 0] Integer field: 30000053
    * 22:25:58 [SvcCallDump]         [ 1] String: 'test dungeon 2 '
    * 22:25:58 [SvcCallDump]         [ 2] WString: 'dungeon notes here'
    * 22:25:58 [SvcCallDump]         [ 3] String: 'XIG-040'
    * 22:25:58 [SvcCallDump]         [ 4] Integer field: 140000000
    */
    BmData data = BmData();
    data.memo = PyRep::StringContent(memo);
    data.note = PyRep::StringContent(comment);
    data.ownerID = ownerID->value();
    data.creatorID = data.ownerID;
    data.created = GetFileTimeNow();
    data.locationID = locationID->value();
    data.typeID = EVEDB::invTypes::SolarSystem;
    data.point = ManagerDB::GetAnomalyPos(resultID->content());

    if (call.byname.find("folderID") != call.byname.end())
        data.folderID = PyRep::IntegerValueU32(call.byname.find("folderID")->second);

    m_db.SaveNewBookmark(data);

    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    Rsp_BookmarkLocation result;
        result.bookmarkID  = data.bookmarkID;
        result.itemID      = data.itemID;             // scan bm returns none here
        result.typeID      = data.typeID;
        result.x           = data.point.x;
        result.y           = data.point.y;
        result.z           = data.point.z;
        result.locationID  = data.locationID;
    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult BookmarkService::DeleteBookmarks(PyCallArgs &call, std::optional <PyObjectEx*> bookmarks) {
    call.Dump(BOOKMARK__CALL_DUMP);
    if (bookmarks.has_value() == false)
        return PyStatic.NewNone();

    PyList* bmList = bookmarks.value()->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();
    //std::vector<int32> bmIDs;
    for (size_t i = 0; i < bmList->size(); ++i)
        m_db.ChangeOwner(bmList->GetItem(i)->AsInt()->value());
        //bmIDs.push_back(bmList->GetItem(i)->AsInt()->value());

    // player is deleting this bm from their pnp window.  there may be copies, so dont remove from db
    //  instead, to keep this in db and avoid appearance in players pnp window, just change owner to system
    // m_db.DeleteBookmarks(&bmIDs);
    return PyStatic.NewNone();
}


PyResult BookmarkService::MoveBookmarksToFolder(PyCallArgs &call, PyInt* folderID, std::optional <PyObjectEx*> bookmarkIDs) {
    // rows = bookmarkMgr.MoveBookmarksToFolder(folderID, bookmarkIDs)
    call.Dump(BOOKMARK__CALL_DUMP);

    //args.Dump(BOOKMARK__CALL_DUMP, "    ");

    if (bookmarkIDs.has_value() == false)
        return PyStatic.NewNone();

    PyList* bmList = bookmarkIDs.value()->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();

    std::vector<int32> bmIDs;
    for (size_t i = 0; i < bmList->size(); ++i)
        bmIDs.push_back(bmList->GetItem(i)->AsInt()->value());

    m_db.MoveBookmarkToFolder(folderID->value(), bmIDs);

    return m_db.GetBookmarksInFolder(folderID->value());
}

PyResult BookmarkService::AddBookmarkFromVoucher(PyCallArgs &call, PyInt* itemID, PyInt* ownerID, std::optional <PyInt*> folderID) {
    /** bookmark = sm.RemoteSvc('bookmark').AddBookmarkFromVoucher, itemID, ownerID, folderID, violateSafetyTimer=True)
     *
     *         02:04:15 [BM CallDump]   Call Arguments:
     *         02:04:15 [BM CallDump]      Tuple: 3 elements
     *         02:04:15 [BM CallDump]       [ 0]    Integer: 140007017
     *         02:04:15 [BM CallDump]       [ 1]    Integer: 90000001
     *         02:04:15 [BM CallDump]       [ 2]       None
     */

    call.Dump(BOOKMARK__CALL_DUMP);
    /*
     *    args.itemID
     *    args.ownerID
     *    args.folderID
     */
    InventoryItemRef iRef = sItemFactory.GetItemRef(itemID->value());
    if (iRef.get() == nullptr) {
        codelog(ITEM__ERROR, "%s: Failed to retrieve bookmark data for voucherID %u", call.client->GetName(), itemID->value());
        return nullptr;
    }

    // get existing bm data this is a copy of
    BmData data = BmData();
    data.bookmarkID = atoi(iRef->customInfo().c_str());
    m_db.GetVoucherData(data);

    // update folderID and ownerID
    data.folderID = folderID.has_value() ? folderID.value()->value() : 0;
    data.ownerID = ownerID->value();

    // create new bm entry for this copy
    m_db.SaveNewBookmark(data);

    // delete voucher  do we need to?
    //iRef->Delete();

    // return new bm data to client
    PyDict* dict = new PyDict();
    dict->SetItemString("bookmarkID", new PyInt(data.bookmarkID));
    dict->SetItemString("ownerID", new PyInt(data.ownerID));
    dict->SetItemString("itemID", new PyInt(data.itemID));
    dict->SetItemString("typeID", new PyInt(data.typeID));
    dict->SetItemString("flag", PyStatic.NewNone());
    dict->SetItemString("memo", new PyString(data.memo));
    dict->SetItemString("created", new PyLong(data.created));
    dict->SetItemString("x", new PyFloat(data.point.x));
    dict->SetItemString("y", new PyFloat(data.point.y));
    dict->SetItemString("z", new PyFloat(data.point.z));
    dict->SetItemString("locationID", new PyInt(data.locationID));
    dict->SetItemString("note", new PyString(data.note));
    dict->SetItemString("creatorID", new PyInt(data.creatorID));
    if (data.folderID > 0) {
        dict->SetItemString("folderID", new PyInt(data.folderID));
    } else {
        dict->SetItemString("folderID", PyStatic.NewNone());
    }

    return new PyObject("util.KeyVal", dict);
}


PyResult BookmarkService::CopyBookmarks(PyCallArgs &call, std::optional <PyObjectEx*> bookmarksToCopy, std::optional <PyInt*> folderID) {
    //newBookmarks, message = bookmarkMgr.CopyBookmarks(bookmarksToCopy, folderID)

  //{'FullPath': u'UI/Messages', 'messageID': 258505, 'label': u'CantTradeMissionBookmarksBody'}(u'You cannot trade or copy mission bookmarks.', None, None)

    // this is called when dropping bms on corp header in bm window

    call.Dump(BOOKMARK__CALL_DUMP);
    //args.Dump(BOOKMARK__CALL_DUMP, "    ");

    if (bookmarksToCopy.has_value() == false)
        return new_tuple(PyStatic.NewNone(), PyStatic.NewNone());

    PyList* bmList = bookmarksToCopy.value()->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();

    PyList* list = new PyList();
    for (size_t i = 0; i < bmList->size(); ++i) {
        InventoryItemRef iRef = sItemFactory.GetItemRef(bmList->GetItem(i)->AsInt()->value());
        if (iRef.get() == nullptr) {
            codelog(ITEM__ERROR, "%s: Failed to retrieve bookmark data for voucherID %u", call.client->GetName(), bmList->GetItem(i)->AsInt()->value());
            continue;
        }

        // get existing bm data this is a copy of
        BmData data = BmData();
        data.bookmarkID = atoi(iRef->customInfo().c_str());
        m_db.GetVoucherData(data);

        // update folderID and ownerID
        data.folderID = folderID.has_value() ? folderID.value()->value() : 0;
        data.ownerID = call.client->GetCharacterID();

        // create new bm entry for this copy
        m_db.SaveNewBookmark(data);

        // return new bm data to client
        PyDict* dict = new PyDict();
        dict->SetItemString("bookmarkID", new PyInt(data.bookmarkID));
        dict->SetItemString("ownerID", new PyInt(data.ownerID));
        dict->SetItemString("itemID", new PyInt(data.itemID));
        dict->SetItemString("typeID", new PyInt(data.typeID));
        dict->SetItemString("flag", PyStatic.NewNone());
        dict->SetItemString("memo", new PyString(data.memo));
        dict->SetItemString("created", new PyLong(data.created));
        dict->SetItemString("x", new PyFloat(data.point.x));
        dict->SetItemString("y", new PyFloat(data.point.y));
        dict->SetItemString("z", new PyFloat(data.point.z));
        dict->SetItemString("locationID", new PyInt(data.locationID));
        dict->SetItemString("note", new PyString(data.note));
        dict->SetItemString("creatorID", new PyInt(data.creatorID));
        if (data.folderID > 0) {
            dict->SetItemString("folderID", new PyInt(data.folderID));
        } else {
            dict->SetItemString("folderID", PyStatic.NewNone());
        }

        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, list);
    tuple->SetItem(1, PyStatic.NewNone());
    return tuple;
}
