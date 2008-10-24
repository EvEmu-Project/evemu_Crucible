/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __EVEDBUTILS_H_INCL__
#define __EVEDBUTILS_H_INCL__

#include "common.h"
#include "packet_types.h"

#include <map>
#include <string>
#include <vector>

class DBQueryResult;
class DBResultRow;

class PyRep;
class PyRepObject;
class PyRepTuple;
class PyRepList;
class PyRepDict;
class PyRepPackedRow;
class PyRepPackedObject2;

/*typedef enum {
	StringContentsInteger,
	StringContentsReal,
	StringContentsString,
	StringContentsUnknown
} StringContentsType;
StringContentsType ClassifyStringContents(const char *str);*/

PyRep *DBColumnToPyRep(DBResultRow &row, uint32 column_index);

PyRepObject *DBResultToRowset(DBQueryResult &result);
PyRepObject *DBResultToIndexRowset(const char *key, DBQueryResult &result);
PyRepTuple *DBResultToTupleSet(DBQueryResult &result);
PyRepTuple *DBResultToRowList(DBQueryResult &result, const char *type = "util.Row");
PyRepDict *DBResultToIntRowDict(DBQueryResult &result, uint32 key_index, const char *type = "util.Row");
PyRepDict *DBResultToIntIntDict(DBQueryResult &result);
void DBResultToIntIntDict(DBQueryResult &result, std::map<uint32, uint32> &into);
void DBResultToIntIntlistDict(DBQueryResult &result, std::map<uint32, PyRep *> &into);

//new packed stuff:
PyRepList *DBResultToPackedRowList(DBQueryResult &result);
PyRepTuple *DBResultToPackedRowListTuple(DBQueryResult &result);
PyRepPackedObject2 *DBResultToPackedRowset(DBQueryResult &result, const char *type = "dbutil.CRowset");

PyRepObject *DBRowToKeyVal(DBResultRow &row);
PyRepObject *DBRowToRow(DBResultRow &row, const char *type = "util.Row");
PyRepPackedRow *DBRowToPackedRow(DBResultRow &row);


#endif




