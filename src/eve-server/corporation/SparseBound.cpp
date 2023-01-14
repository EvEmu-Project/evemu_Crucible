/*      SparseBound.cpp
 *      class to manage very specific data for corp offices
 *              ** cannot get this to work right.
 *
 * this class' response data is required for the following corp functions:
 * item lockdown
 * RAM where bp or mat'l are in corp hangars
 * some vote items
 * member hangar auditing
 *
 */

#include "corporation/SparseBound.h"
#include "station/StationDataMgr.h"

SparseBound::SparseBound(EVEServiceManager &mgr, CorporationDB& db, uint32 corpID) :
    EVEBoundObject(mgr, nullptr),
    m_db(db),
    m_corpID(corpID)
{
    this->Add("Fetch", &SparseBound::Fetch);
    this->Add("FetchByKey", &SparseBound::FetchByKey);
    this->Add("GetByKey", &SparseBound::GetByKey);
    this->Add("SelectByUniqueColumnValues", &SparseBound::SelectByUniqueColumnValues);
}

bool SparseBound::CanClientCall(Client* client) {
    return true; // TODO: properly implement this
}
PyResult SparseBound::Fetch(PyCallArgs &call, PyInt* startPos, PyInt* fetchSize)
{
    return m_db.Fetch(call.client->GetCorporationID(), startPos->value(), fetchSize->value());
}

// this is called in a few places to get officeID
//   uses a method chain from GetOffices() (from bound office object) and is only called when accessing containers and item(s) owned by corp.
//  however, for these items to show in containers' inventory, current code requires they have to be:
//        1) owned by the hangar's owner
//        2) owned by the viewer (whom shall also be hangar's owner)
//  this poses a problem for corp staff when viewing member's hangars.  not sure how to 'fix' this yet.
//      and i still havent gotten this to work dependably
PyResult SparseBound::SelectByUniqueColumnValues(PyCallArgs &call, PyRep* columnName, PyList* values)
{
    // no packet data for this call

    _log(CORP__CALL, "SparseBound::Handle_SelectByUniqueColumnValues()");
    call.Dump(CORP__WARNING);   //so this will dump on all calls

    std::string str = PyRep::StringContent(columnName);
    // could this be multiple locations?  doubt it.
    uint32 locationID = PyRep::IntegerValueU32(values->GetItem(0));

    std::vector<OfficeData> data;
    stDataMgr.GetStationOfficeIDs(locationID, data);    // this method checks for the type of locationID sent.

    /*  IndexError: list assignment index out of range
    PyTuple* header = new PyTuple(3);
        header->SetItemString(0, "officeID");
        header->SetItemString(1, "folderID");
        header->SetItemString(2, "stationID");
    PyList* lines = new PyList();
    for (auto cur : data) {
        PyTuple* line = new PyTuple(3);
            line->SetItem(0, new PyInt(cur.officeID));
            line->SetItem(1, new PyInt(cur.folderID));
            line->SetItem(2, new PyInt(cur.stationID));
        lines->AddItem(line);
    }

    PyDict* dict = new PyDict();
        dict->SetItemString("header", header);
        dict->SetItemString("RowClass", new PyToken("util.Row"));
        dict->SetItemString("lines", lines);

    PyRep* rsp = new PyObject("util.Rowset", dict);

    if (is_log_enabled(CORP__RSP_DUMP))
        rsp->Dump(CORP__RSP_DUMP, "");

    return rsp;
    */

    /*  IndexError: list assignment index out of range
     *
    PyList* rsp = new PyList();
    for (auto cur : data) {
        PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyInt(cur.officeID));
        tuple->SetItem(1, new PyInt(cur.folderID));
        tuple->SetItem(2, new PyInt(cur.stationID));
        rsp->AddItem(tuple);
    }
    */

    //*  original    -- TypeError: iteration over non-sequence

    PyList* rsp = new PyList();
    for (auto cur : data) {
        PyDict* dict = new PyDict();
        dict->SetItemString("stationID",       new PyInt(cur.stationID));
        dict->SetItemString("officeFolderID",  new PyInt(cur.folderID));
        dict->SetItemString("officeID",        new PyInt(cur.officeID));
        // keep this as KeyVal for proper indexing
        rsp->AddItem(new PyObject("util.KeyVal", dict));
    }

   //if (is_log_enabled(CORP__RSP_DUMP))
    //    rsp->Dump(CORP__RSP_DUMP, "");

    PySafeDecRef(rsp);
    
    return PyStatic.mtList();
}


PyResult SparseBound::FetchByKey(PyCallArgs &call, PyList* keys) {
    // this is something about getting member data....
    sLog.Error("SparseBound", "Handle_FetchByKey()");
    call.Dump(CORP__WARNING);   //so this will dump on all calls

    /*
     * call
     *          [PyTuple 4 items]
     *            [PyString "N=699765:12950"]
     *            [PyString "FetchByKey"]
     *            [PyTuple 1 items]
     *              [PyList 1 items]
     *                [PyInt 649670823]       // charID ?
     *
     * return
     *      [PySubStream 71 bytes]
     *        [PyList 1 items]
     *          [PyTuple 3 items]
     *            [PyInt 1661059544]
     *            [PyInt 0]
     *            [PyList 19 items]
     *              [PyInt 1661059544]
     *              [PyInt 98038978]
     *              [PyNone]
     *              [PyNone]
     *              [PyString ""]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 129492958800000000] // timestamp
     *              [PyNone]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 0]
     *              [PyIntegerVar 0]
     *              [PyInt 0]
     *              [PyInt 1004]
     *              [PyIntegerVar 129492958800000000] // timestamp
     *              [PyInt 0]
     */
    return nullptr;
}

PyResult SparseBound::GetByKey(PyCallArgs &call, PyInt* keys) {
    sLog.Error("SparseBound", "Handle_GetByKey()");
    call.Dump(CORP__WARNING);   //so this will dump on all calls

    // no packet data for this call

    return nullptr;
}
