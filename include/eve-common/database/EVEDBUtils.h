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
    Author:     Zhur
*/

#ifndef __EVEDBUTILS_H_INCL__
#define __EVEDBUTILS_H_INCL__

#include "database/dbcore.h"
#include "network/packet_types.h"

class PyRep;
class PyObject;
class PyTuple;
class PyList;
class PyDict;
class PyObjectEx;
class PyPackedRow;

/*typedef enum {
    StringContentsInteger,
    StringContentsReal,
    StringContentsString,
    StringContentsUnknown
} StringContentsType;
StringContentsType ClassifyStringContents(const char *str);*/

PyRep *DBColumnToPyRep(const DBResultRow &row, uint32 column_index);

PyObject *DBResultToRowset(DBQueryResult &result);
PyObject *DBResultToIndexRowset(DBQueryResult &result, const char *key);
PyObject *DBResultToIndexRowset(DBQueryResult &result, uint32 key_index);
PyTuple *DBResultToTupleSet(DBQueryResult &result);
PyTuple *DBResultToRowList(DBQueryResult &result, const char *type = "util.Row");
PyDict *DBResultToIntRowDict(DBQueryResult &result, uint32 key_index, const char *type = "util.Row");
PyDict *DBResultToIntIntDict(DBQueryResult &result);
void DBResultToIntIntDict(DBQueryResult &result, std::map<int32, int32> &into);
void DBResultToUIntUIntDict(DBQueryResult &result, std::map<uint32, uint32> &into);
void DBResultToIntIntlistDict(DBQueryResult &result, std::map<int32, PyRep *> &into);

PyList *DBResultToPackedRowList(DBQueryResult &result);
PyTuple *DBResultToPackedRowListTuple(DBQueryResult &result);
PyObjectEx *DBResultToCRowset(DBQueryResult &result);

//single rows:
PyObject *DBRowToKeyVal(DBResultRow &row);
PyObject *DBRowToRow(DBResultRow &row, const char *type = "util.Row");
PyPackedRow *DBRowToPackedRow(DBResultRow &row);


#endif




