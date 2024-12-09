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
    Author:        Almamu
*/

#ifndef EVEMU_SPARSEBOUND_H
#define EVEMU_SPARSEBOUND_H

#include "services/BoundService.h"
#include "packets/CorporationPkts.h"

class OnObjectPublicAttributesUpdated;

template <class Bound>
class SparseBound : public EVEBoundObject <Bound> {
public:
    SparseBound (EVEServiceManager& mgr, BoundServiceParent<Bound>& parent) :
        EVEBoundObject <Bound> (mgr, parent),
        m_header (nullptr)
    {
        this->Add("Fetch", &SparseBound <Bound>::Fetch);
        this->Add("FetchByKey", &SparseBound <Bound>::FetchByKey);
        this->Add("SelectByUniqueColumnValues", &SparseBound <Bound>::SelectByUniqueColumnValues);
    }

    ~SparseBound () {
        PyDecRef (this->m_header);
    }

    /**
     * Primes the index list and initializes the header
     */
    void Prime (PyList* headers) {
        auto cur = headers->begin();
        auto end = headers->end();

        for (; cur != end; cur ++) {
            if ((*cur)->IsString () == false) {
                codelog(SERVICE__ERROR, "SparseRowset: One of the header values is not a string");
                continue;
            }

            this->m_columns.push_back ((*cur)->AsString ()->content ().c_str ());
        }

        // fetch the keys first
        this->InitializeKeys();

        // build the required header
        PyDict* boundObjectArguments = new PyDict();

        boundObjectArguments->SetItem ("realRowCount", new PyInt (this->m_indexMap.size()));

        PyTuple* boundServiceInformation = new PyTuple(3);

        boundServiceInformation->SetItem (0, new PyString (this->GetIDString ()));
        boundServiceInformation->SetItem (1, boundObjectArguments);
        boundServiceInformation->SetItem (2, new PyLong (GetFileTimeNow ()));

        PyTuple* arguments = new PyTuple (3);

        arguments->SetItem (0, headers);
        arguments->SetItem (1, new PySubStruct (new PySubStream (boundServiceInformation)));
        arguments->SetItem (2, new PyInt (this->m_indexMap.size()));

        this->m_header = new PyObject ("util.SparseRowset", arguments);
        this->m_primed = true;
    }

    PyObject* GetHeader () { assert (this->m_primed); PyIncRef(this->m_header); return this->m_header; }

    /**
     * Notifies the bound clients about a new row on the sparse rowset's data
     */
    void NotifyNewRow (DBResultRow& row) {
        // re-fetch the keys
        this->InitializeKeys ();

        PyDict* values = new PyDict;

        for (int i = 0; i < row.ColumnCount (); i ++) {
            PyTuple* entry = new PyTuple(2);

            entry->SetItem (0, PyStatic.NewNone());
            entry->SetItem (1, DBColumnToPyRep (row, i));

            values->SetItem (row.ColumnName (i), entry);
        }

        OnObjectPublicAttributesUpdated update;

        update.bindID = this->GetIDString ();
        update.changes = values;
        update.realRowCount = this->m_indexMap.size();
        update.changePKIndexValue = row.GetInt64 (0);

        // let the inheriting class decide how to send the notification
        this->SendNotification (update);
    }
    /**
     * Notifies the bound clients about a deleted row on the sparse rowset's data
     */
    void NotifyRowDeleted (int64 primaryKey) {
        // re-fetch the keys
        this->InitializeKeys();

        PyDict* values = new PyDict;

        for (auto cur : this->m_columns) {
            PyTuple* entry = new PyTuple(2);

            entry->SetItem (0, new PyInt (1));
            entry->SetItem (1, PyStatic.NewNone());

            values->SetItem (new PyString (cur), entry);
        }

        OnObjectPublicAttributesUpdated update;

        update.bindID = this->GetIDString ();
        update.changes = values;
        update.realRowCount = this->m_indexMap.size();
        update.changePKIndexValue = primaryKey;

        // let the inheriting class decide how to send the notification
        this->SendNotification (update);
    }
protected:
    PyResult Fetch(PyCallArgs& call, PyInt* startPos, PyInt* fetchSize) {
        DBQueryResult res;
        DBResultRow row;

        if (!this->LoadFromDatabase (res, startPos->value(), fetchSize->value())) {
            codelog (SERVICE__ERROR, "Cannot fetch data for sparse rowset");
            return nullptr;
        }

        PyList* result = new PyList();

        while (res.GetRow (row)) {
            PyTuple* value = new PyTuple(2);

            value->SetItem (0, new PyLong (row.GetInt64 (0)));
            value->SetItem (1, DBRowToRow (row));

            result->AddItem (value);
        }

        return result;
    }

    PyResult FetchByKey(PyCallArgs& call, PyList* keys) {
        DBQueryResult res;
        DBResultRow row;

        if (!this->LoadFromDatabase (res, keys))  {
            codelog (SERVICE__ERROR, "Cannot fetch data for sparse rowset");
            return nullptr;
        }

        PyList* result = new PyList();

        while (res.GetRow (row)) {
            PyTuple* value = new PyTuple(3);

            // search the key value
            int64_t keyValue = row.GetInt64 (0);
            auto indexIt = this->m_indexMap.find (keyValue);

            if (indexIt == this->m_indexMap.end ()) {
                // the index cannot be found, show an error and continue on to the next one
                codelog(SERVICE__ERROR, "SparseRowset: Cannot find the key %lli in the index list", keyValue);
                continue;
            }

            value->SetItem (0, new PyLong (keyValue));
            value->SetItem (1, new PyInt (indexIt->second));
            value->SetItem (2, DBRowToRow (row));

            result->AddItem (value);
        }

        return result;
    }

    PyResult SelectByUniqueColumnValues(PyCallArgs& call, PyRep* columnName, PyList* values) {
        DBQueryResult res;
        DBResultRow row;

        if (!this->LoadFromDatabase (res, PyRep::StringContent (columnName).c_str (), values))  {
            codelog (SERVICE__ERROR, "Cannot fetch data for sparse rowset");
            return nullptr;
        }

        PyList* result = new PyList();

        while (res.GetRow (row)) {
            PyTuple* value = new PyTuple(3);

            // search the key value
            int64_t keyValue = row.GetInt64 (0);
            auto indexIt = this->m_indexMap.find (keyValue);

            if (indexIt == this->m_indexMap.end ()) {
                // the index cannot be found, show an error and continue on to the next one
                codelog(SERVICE__ERROR, "SparseRowset: Cannot find the key %lli in the index list", keyValue);
                continue;
            }

            value->SetItem (0, new PyLong (keyValue));
            value->SetItem (1, new PyInt (indexIt->second));
            value->SetItem (2, DBRowToRow (row));

            result->AddItem (value);
        }

        return result;
    }


    /**
     * Fetches the list of key values for the rowset
     *
     * @param res
     */
    virtual bool LoadIndexes (DBQueryResult& res) = 0;

    /**
     * Fetches data from the database with the specified limits
     *
     * The first value in the result must be the key
     *
     * @param res The database result that will store the data
     * @param startPos
     * @param fetchSize
     */
    virtual bool LoadFromDatabase (DBQueryResult& res, int startPos, int fetchSize) = 0;

    /**
     * Fetches data from the database with the specified limits
     *
     * The first value in the result must be the key
     *
     * @param res The database result that will store the data
     * @param keyList The values for the key we're going to search for
     */
    virtual bool LoadFromDatabase (DBQueryResult& row, PyList* keyList) = 0;

    /**
     * Fetches data from the database with the specified limits
     *
     * The first value in the result must be the key
     *
     * @param res The database result that will store the data
     * @param columnName The column to filter by
     * @param values The values for the key we're going to search for
     */
    virtual bool LoadFromDatabase (DBQueryResult& res, const std::string& columnName, PyList* values) = 0;

    /**
     * Sends the update notification
     *
     * @param update
     */
    virtual void SendNotification (OnObjectPublicAttributesUpdated& update) = 0;
private:
    void InitializeKeys () {
        this->m_indexMap.clear ();

        DBQueryResult res;
        DBResultRow row;
        size_t index = 0;

        if (!this->LoadIndexes (res)) {
            codelog (SERVICE__ERROR, "Cannot fetch indexes for the sparse bound");
            return;
        }

        while (res.GetRow (row)) {
            this->m_indexMap.insert_or_assign (row.GetInt64 (0), index ++);
        }
    }

    /** The rowset header used to send whoever binds this service */
    PyObject* m_header;
    /** Maps the key values to the position in the result list */
    std::map<uint64_t, uint32_t> m_indexMap;
    /** Column list for this rowset */
    std::vector <std::string> m_columns;
    /** Indicates if the sparse bound was primer already or not */
    bool m_primed;
};

#endif //EVEMU_SPARSEBOUND_H
