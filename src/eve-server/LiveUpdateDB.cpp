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
    Author:     caytchen
*/

#include "eve-server.h"

#include "LiveUpdateDB.h"

PyList* LiveUpdateDB::GenerateUpdates()
{
    const char* query = "SELECT updateID, updateName, description, machoVersionMin, machoVersionMax, buildNumberMin, buildNumberMax, methodName, objectID, codeType, code FROM liveupdates";
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, query))
    {
        codelog(DATABASE__ERROR, "Couldn't get live updates from database: %s", res.error.c_str());
        return NULL;
    }

    // setup the descriptor
    DBRowDescriptor* header = new DBRowDescriptor();
    header->AddColumn("updateID", DBTYPE_I4);
    header->AddColumn("updateName", DBTYPE_WSTR);
    header->AddColumn("description", DBTYPE_WSTR);
    header->AddColumn("machoVersionMin", DBTYPE_I4);
    header->AddColumn("machoVersionMax", DBTYPE_I4);
    header->AddColumn("buildNumberMin", DBTYPE_I4);
    header->AddColumn("buildNumberMax", DBTYPE_I4);
    header->AddColumn("code", DBTYPE_STR);

    // we need to manually create PyPackedRows since we don't want everything from the query in them
    PyList* list = new PyList(res.GetRowCount());
    int listIndex = 0;
    DBResultRow row;
    while (res.GetRow(row))
    {
        PyPackedRow* packedRow = new PyPackedRow(header);
        for (int i = 0; i < 7; i++)
            packedRow->SetField(i, DBColumnToPyRep(row, i));
        PyIncRef(header);

        LiveUpdateInner inner;
        // binary data so we can't expect strlen to get it right
        inner.code = std::string(row.GetText(10), row.ColumnLength(10));
        inner.codeType = row.GetText(9);
        inner.objectID = row.GetText(8);
        inner.methodName = row.GetText(7);
        packedRow->SetField(static_cast<uint32>(7) /* code */, inner.Encode());

        list->SetItem(listIndex++, packedRow);
    }

    return list;
}
