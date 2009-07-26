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
    Author:     Zhur
*/

#ifndef __EVEDBUTILS_H_INCL__
#define __EVEDBUTILS_H_INCL__

#include <map>
#include <string>
#include <vector>

#include "common.h"
#include "packet_types.h"
#include "dbcore.h"

class blue_DBRowDescriptor;

class PyRep;
class PyRepObject;
class PyRepTuple;
class PyRepList;
class PyRepDict;
class PyRepObjectEx;
class PyRepPackedRow;

/*typedef enum {
    StringContentsInteger,
    StringContentsReal,
    StringContentsString,
    StringContentsUnknown
} StringContentsType;
StringContentsType ClassifyStringContents(const char *str);*/

PyRep *DBColumnToPyRep(const DBResultRow &row, uint32 column_index);

PyRepObject *DBResultToRowset(DBQueryResult &result);
PyRepObject *DBResultToIndexRowset(DBQueryResult &result, const char *key);
PyRepObject *DBResultToIndexRowset(DBQueryResult &result, uint32 key_index);
PyRepTuple *DBResultToTupleSet(DBQueryResult &result);
PyRepTuple *DBResultToRowList(DBQueryResult &result, const char *type = "util.Row");
PyRepDict *DBResultToIntRowDict(DBQueryResult &result, uint32 key_index, const char *type = "util.Row");
PyRepDict *DBResultToIntIntDict(DBQueryResult &result);
void DBResultToIntIntDict(DBQueryResult &result, std::map<int32, int32> &into);
void DBResultToUIntUIntDict(DBQueryResult &result, std::map<uint32, uint32> &into);
void DBResultToIntIntlistDict(DBQueryResult &result, std::map<int32, PyRep *> &into);

/**
 * This routine returns DBTYPE based on DBQueryResult::ColType.
 *
 * @param[in] colType DBQueryResult::ColType.
 * @return DBTYPE.
 */
DBTYPE GetPackedColumnType(DBQueryResult::ColType colType);

/**
 * This routine builds blue_DBRowDescriptor from DBQueryResult.
 *
 * @param[in] res DBQueryResult to build from.
 * @return blue_DBRowDescriptor.
 */
PyRepObjectEx *DBResultToRowDescriptor(const DBQueryResult &res);
/**
 * This routine builds blue_DBRowDescriptor from DBResultRow.
 *
 * @param[in] row DBResultRow to build from.
 * @return blue_DBRowDescriptor.
 */
PyRepObjectEx *DBRowToRowDescriptor(const DBResultRow &row);

PyRepList *DBResultToPackedRowList(DBQueryResult &result);
PyRepTuple *DBResultToPackedRowListTuple(DBQueryResult &result);
PyRepObjectEx *DBResultToCRowset(DBQueryResult &result);

//single rows:
PyRepObject *DBRowToKeyVal(DBResultRow &row);
PyRepObject *DBRowToRow(DBResultRow &row, const char *type = "util.Row");
PyRepPackedRow *DBRowToPackedRow(DBResultRow &row);


#endif




