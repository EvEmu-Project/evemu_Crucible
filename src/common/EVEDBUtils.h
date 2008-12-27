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
	Author:		Zhur
*/

#ifndef __EVEDBUTILS_H_INCL__
#define __EVEDBUTILS_H_INCL__

#include "common.h"
#include "packet_types.h"

#include <map>
#include <string>
#include <vector>

class blue_DBRowDescriptor;

class DBQueryResult;
class DBResultRow;

class PyRep;
class PyRepObject;
class PyRepTuple;
class PyRepList;
class PyRepDict;
class PyRepNewObject;
class PyRepPackedRow;

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
class DBPackedColumnList {
public:
	enum Order {
		orderByIndex,
		orderByType
	};
	struct ColumnInfo {
		uint32 index;
		std::string name;
		DBTYPE type;
	};
	typedef std::vector<ColumnInfo>::iterator iterator;
	typedef std::vector<ColumnInfo>::const_iterator const_iterator;

	//default ordering is by index
	DBPackedColumnList(const DBQueryResult &res);
	DBPackedColumnList(const DBResultRow &row);
	DBPackedColumnList(const blue_DBRowDescriptor &desc);

	void Encode(blue_DBRowDescriptor &into) const;
	PyRep *Encode() const;

	Order order() const { return(m_order); }
	void OrderBy(Order o);

	iterator begin() { return(m_columnList.begin()); }
	const_iterator begin() const { return(m_columnList.begin()); }
	iterator end() { return(m_columnList.end()); }
	const_iterator end() const { return(m_columnList.end()); }

	ColumnInfo &at(size_t index) { return(m_columnList.at(index)); }
	const ColumnInfo &at(size_t index) const { return(m_columnList.at(index)); }
	ColumnInfo &operator[](size_t index) { return(m_columnList[index]); }
	const ColumnInfo &operator[](size_t index) const { return(m_columnList[index]); }

	size_t Count() const { return(m_columnList.size()); }
	size_t CountBufferTypes() const;
	size_t CountPyRepTypes() const;

protected:
	std::vector<ColumnInfo> m_columnList;
	Order m_order;
};

//this routine is used to order the fields in a packed row.
uint8 GetTypeSize(DBTYPE t);
//returns true if given DBTYPE goes encoded to buffer
bool IsBufferType(DBTYPE t);
//returns true if given DBTYPE goes encoded as PyRep following buffer
bool IsPyRepType(DBTYPE t);

PyRepList *DBResultToPackedRowList(DBQueryResult &result);
PyRepTuple *DBResultToPackedRowListTuple(DBQueryResult &result);
PyRepNewObject *DBResultToCRowset(DBQueryResult &result);

//single rows:
PyRepObject *DBRowToKeyVal(DBResultRow &row);
PyRepObject *DBRowToRow(DBResultRow &row, const char *type = "util.Row");
PyRepPackedRow *DBRowToPackedRow(DBResultRow &row);


#endif




