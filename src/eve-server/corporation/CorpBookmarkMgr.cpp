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
    Author:        Ubiquitatis
    Rewrite:    Allan
*/

/* the distinction between bookmark Mgrs (corp/personal) is made on-the-fly when methods are called in client.
 * most (if not all) will use same code and returns.  so far, i havent seen a difference between them.
 * this may be used to separate corp and personal bm in db on live.
 * not sure if im gonna do that here or not yet.
 */

#include "eve-server.h"

#include "Client.h"
#include "PyServiceCD.h"
#include "packets/Bookmarks.h"
#include "cache/ObjCacheService.h"
#include "corporation/CorpBookmarkMgr.h"

PyCallable_Make_InnerDispatcher(CorpBookmarkMgr)

CorpBookmarkMgr::CorpBookmarkMgr(PyServiceMgr* mgr)
: PyService(mgr, "corpBookmarkMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpBookmarkMgr, GetBookmarks);
    PyCallable_REG_CALL(CorpBookmarkMgr, UpdateBookmark);
    PyCallable_REG_CALL(CorpBookmarkMgr, UpdatePlayerBookmark);
    PyCallable_REG_CALL(CorpBookmarkMgr, MoveBookmarksToFolder);
    PyCallable_REG_CALL(CorpBookmarkMgr, CreateFolder);
    PyCallable_REG_CALL(CorpBookmarkMgr, UpdateFolder);
    PyCallable_REG_CALL(CorpBookmarkMgr, CopyBookmarks);
    PyCallable_REG_CALL(CorpBookmarkMgr, DeleteFolder);
    PyCallable_REG_CALL(CorpBookmarkMgr, MoveFoldersToDB);
    PyCallable_REG_CALL(CorpBookmarkMgr, DeleteBookmarks);
}

CorpBookmarkMgr::~CorpBookmarkMgr()
{
    delete m_dispatch;
}

PyResult CorpBookmarkMgr::Handle_GetBookmarks(PyCallArgs& call)
{
    /*
    ObjectCachedMethodID method_id(GetName(), "GetBookmarks");
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyTuple *tuple = new PyTuple(2);
            tuple->SetItem(0, m_db.GetBookmarks(call.client->GetCorporationID()));
            tuple->SetItem(1, m_db.GetFolders(call.client->GetCorporationID()));
        PyRep* rep = tuple;

        m_manager->cache_service->GiveCache(method_id, &rep);
    }

    return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
    */
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, m_db.GetBookmarks(call.client->GetCorporationID()));
    result->SetItem(1, m_db.GetFolders(call.client->GetCorporationID()));
    result->Dump(BOOKMARK__RSP_DUMP, "    ");
    return result;
}

PyResult CorpBookmarkMgr::Handle_UpdateBookmark(PyCallArgs& call) {
    call.Dump(BOOKMARK__CALL_DUMP);
    Call_UpdateBookmark args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.UpdateBookmark(args.bookmarkID, call.client->GetCorporationID(), PyRep::StringContent(args.memo), PyRep::StringContent(args.comment), args.folderID);

    return PyStatic.NewNone();
}

PyResult CorpBookmarkMgr::Handle_UpdatePlayerBookmark(PyCallArgs& call) {
    call.Dump(BOOKMARK__CALL_DUMP);
    Call_UpdateBookmark args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.UpdateBookmark(args.bookmarkID, call.client->GetCorporationID(), PyRep::StringContent(args.memo), PyRep::StringContent(args.comment), args.folderID);

    return PyStatic.NewNone();
}

PyResult CorpBookmarkMgr::Handle_MoveBookmarksToFolder(PyCallArgs& call)
{
    // rows = bookmarkMgr.MoveBookmarksToFolder(folderID, bookmarkIDs)
    call.Dump(BOOKMARK__CALL_DUMP);
    Call_MoveBookmarksToFolder args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    //args.Dump(BOOKMARK__CALL_DUMP, "    ");

    if (args.object->IsNone())
        return PyStatic.NewNone();

    PyList* bmList = args.object->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();

    std::vector<int32> bmIDs;
    for (size_t i = 0; i < bmList->size(); ++i)
        bmIDs.push_back(bmList->GetItem(i)->AsInt()->value());

    m_db.MoveBookmarkToFolder(args.folderID, &bmIDs);

    return m_db.GetBookmarksInFolder(args.folderID);
}

PyResult CorpBookmarkMgr::Handle_UpdateFolder(PyCallArgs& call)
{
    //if bookmarkMgr.UpdateFolder(folderID, folderName):
    call.Dump(BOOKMARK__CALL_DUMP);
    Call_UpdateFolder args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewFalse();
    }

    /** @todo sanitize name */
    if (!m_db.UpdateFolder(args.folderID, PyRep::StringContent(args.folderName)))
        return PyStatic.NewFalse();

    return PyStatic.NewTrue();
}

PyResult CorpBookmarkMgr::Handle_CreateFolder(PyCallArgs& call)
{
    //folder = bookmarkMgr.CreateFolder(folderName)
    call.Dump(BOOKMARK__CALL_DUMP);
    /** @todo sanitize name */
    std::string name = PyRep::StringContent(call.tuple->GetItem( 0 ));

    uint32 ownerID = call.client->GetCorporationID();
    Rsp_CreateFolder result;
        result.ownerID = ownerID;
        result.folderID = m_db.SaveNewFolder(name, ownerID);
        result.folderName = name;
        result.creatorID = call.client->GetCharacterID();

    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult CorpBookmarkMgr::Handle_CopyBookmarks(PyCallArgs& call)
{
    call.Dump(BOOKMARK__CALL_DUMP);
    return PyStatic.NewNone();
}

PyResult CorpBookmarkMgr::Handle_DeleteFolder(PyCallArgs& call)
{
    //deleteFolder, bookmarks = self.corpBookmarkMgr.DeleteFolder(folderID, bookmarkIDs)
    call.Dump(BOOKMARK__CALL_DUMP);
    uint32 folderID(PyRep::IntegerValueU32(call.tuple->GetItem(0)));

    // call db to get list of bmIDs in deleted folder.  return result with this data
    std::vector< int32 > bmIDs;
    bmIDs.clear();
    m_db.GetBookmarkByFolderID(folderID, bmIDs);

    PyList* list = new PyList();
    for (auto cur : bmIDs) {
        PyDict* dict = new PyDict();
        dict->SetItemString("bookmarkID", new PyInt(cur));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    m_db.DeleteFolder(folderID);

    return list;
}

PyResult CorpBookmarkMgr::Handle_MoveFoldersToDB(PyCallArgs& call)
{
    //rows, folders = self.corpBookmarkMgr.MoveFoldersToDB(info)
    call.Dump(BOOKMARK__CALL_DUMP);
    return PyStatic.NewNone();
}

PyResult CorpBookmarkMgr::Handle_DeleteBookmarks(PyCallArgs& call)
{
    //deletedBookmarks = self.corpBookmarkMgr.DeleteBookmarks(bookmarkIDs)
    call.Dump(BOOKMARK__CALL_DUMP);
    Call_DeleteBookmarks args;

    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    if (args.object->IsNone())
        return PyStatic.NewNone();

    PyList* bmList = args.object->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();

    PyList* list = new PyList();
    for (size_t i = 0; i < bmList->size(); ++i) {
        uint32 bmID = bmList->GetItem(i)->AsInt()->value();
        m_db.ChangeOwner(bmID, 1);
        PyDict* dict = new PyDict();
        dict->SetItemString("bookmarkID", new PyInt(bmID));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }
    return list;
}
