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
#ifndef __ROWSETREADER_H_INCL__
#define __ROWSETREADER_H_INCL__

#include <map>
#include <string>
#include "../packets/General.h"
#include "PyVisitor.h"
//#include "logsys.h"

/*
 * this object is incredibly slow, so dont use it a lot...
 */
class BaseRowsetReader {
public:
	virtual ~BaseRowsetReader() {}

    /* Required set of operations:
	iterator begin();
	iterator end();
	bool empty() const;
	uint32 ColumnCount() const;
	const char *ColumnName(uint32 index) const;
	*/
	virtual uint32 size() const = 0;	//number of rows
	
	//void Dump(LogType type);
	
protected:
	//internally used constant
	static const uint32 InvalidRow = 0xFFFFFFFF;
	
	class base_iterator {
	public:
		const char *GetString(const char *fieldname) const;
		bool IsNone(const char *fieldname) const { return(IsNone(_find(fieldname))); }
		uint32 GetInt(const char *fieldname) const { return(GetInt(_find(fieldname))); }
		uint64 GetInt64(const char *fieldname) const { return(GetInt64(_find(fieldname))); }
		double GetReal(const char *fieldname) const { return(GetReal(_find(fieldname))); }
		const char *GetString(uint32 index) const;
		bool IsNone(uint32 index) const;
		uint32 GetInt(uint32 index) const;
		uint64 GetInt64(uint32 index) const;
		double GetReal(uint32 index) const;

		//these Gets convert whatever we have into a string
		std::string GetAsString(const char *fieldname) const;
		std::string GetAsString(uint32 index) const;

		PyRep::Type GetType(uint32 index) const;

		const base_iterator &operator++();
	protected:
		base_iterator(const BaseRowsetReader *parent, bool is_end);
		
		bool _baseEqual(const base_iterator &other) const;
		uint32 _find(const char *name) const;
		PyRep *_find(uint32 index) const;
		
		bool m_end;
		uint32 m_index;
	private:
		const BaseRowsetReader *m_baseReader;	//we do NOT own this
	};

	//for vc6 compat (access to _getRow())
	friend class base_iterator;
	virtual PyRep *_getRow(uint32 index) const = 0;
	
	std::map<std::string, uint32> m_fieldMap;
};


class RowsetReader : public BaseRowsetReader {
public:
	RowsetReader(const util_Rowset *rowset);
	virtual ~RowsetReader() {}

	class iterator : public base_iterator {
		friend class RowsetReader;
	public:
		iterator();
		
		bool operator==(const iterator &other);
		bool operator!=(const iterator &other);

	protected:
		//private constructor for RowsetReader
		iterator(RowsetReader *parent, bool is_end);
		RowsetReader *m_parent;
	};

	iterator begin();
	iterator end() { return(iterator(this, true)); }
	uint32 size() const;	//number of rows
	bool empty() const;
	uint32 ColumnCount() const;
	const char *ColumnName(uint32 index) const;

protected:
	virtual PyRep *_getRow(uint32 index) const;
	const util_Rowset *const m_set;		//we do NOT own this pointer
};


class TuplesetReader : public BaseRowsetReader {
public:
	TuplesetReader(const util_Tupleset *rowset);
	virtual ~TuplesetReader() {}

	class iterator : public base_iterator {
		friend class TuplesetReader;
	public:
		iterator();
		
		bool operator==(const iterator &other);
		bool operator!=(const iterator &other);

	protected:
		//private constructor for RowsetReader
		iterator(TuplesetReader *parent, bool is_end);
		TuplesetReader *m_parent;
	};

	iterator begin();
	iterator end() { return(iterator(this, true)); }
	uint32 size() const;	//number of rows
	bool empty() const;
	uint32 ColumnCount() const;
	const char *ColumnName(uint32 index) const;

protected:
	virtual PyRep *_getRow(uint32 index) const;
	const util_Tupleset *const m_set;		//we do NOT own this pointer
};

//this object scans a pyrep for rowsets or tuplesets, and uses ReaderToSQL on them
class SetSQLDumper : public PyVisitor {
public:
	SetSQLDumper(FILE *f, const char *tablename = "UNKNOWN");
	virtual ~SetSQLDumper() {}
	virtual void VisitObject(const PyRepObject *rep);
	virtual void VisitTuple(const PyRepTuple *rep);
protected:
	FILE *const m_file;	//we do not own this file
	const std::string m_tablename;
};

#endif


