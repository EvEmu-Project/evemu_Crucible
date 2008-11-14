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

#ifndef DBCORE_H
#define DBCORE_H

//this whole file could be interface-ized to support a different database
//if you can get over the SQL incompatibilities and mysql auto increment problems.

#include "common.h"
//#include <string>
#include <mysql/mysql.h>
#include "Mutex.h"

//#include <string>
//#include <vector>

class DBcore;

class DBerror {
public:
	DBerror();

	uint32 GetErrno() const { return(m_errno); }
	const char *GetError() const { return(m_str.c_str()); }
	const char *c_str() const { return(m_str.c_str()); }

protected:
	//for DBcore:
	friend class DBcore;
	void SetError(uint32 err, const char *str);
	void ClearError();
	
	std::string m_str;
	uint32 m_errno;
};


class DBResultRow;
class DBQueryResult {
public:
	//error during the query, if RunQuery returned false.
	DBerror error;

	DBQueryResult();
	~DBQueryResult();

	typedef enum {
		Int8,
		Int16,
		Int32,
		Int64,
		Real,
		DateTime,
		String,
		Binary
	} ColType;
	
	uint32 ColumnCount() const { return(m_col_count); }
	const char *ColumnName(uint32 column) const;
	ColType ColumnType(uint32 column) const;



	void Reset();
	bool GetRow(DBResultRow &into);
	
protected:
	//for DBcore:
	friend class DBcore;
	void SetResult(MYSQL_RES **res, uint32 colcount);

	uint32 m_col_count;
	MYSQL_RES *m_res;
	MYSQL_FIELD **m_fields;
};

class DBResultRow {
public:
	DBResultRow();

	uint32 GetColumnLength(uint32 column) const;
	
	bool IsNull(uint32 column) const { return(m_row[column] == NULL); }
	bool IsSigned(uint32 column) const { return(m_row[column][0] == '-'); }
	const char *GetText(uint32 column) const { return(m_row[column]); }
	int32 GetInt(uint32 column) const;
	uint32 GetUInt(uint32 column) const;
	int64 GetInt64(uint32 column) const;
	uint64 GetUInt64(uint32 column) const;
	float GetFloat(uint32 column) const;
	double GetDouble(uint32 column) const;
	uint32 GetBinary(uint32 column, byte *into, uint32 in_length) const;
	
	//proxy methods up to our query result:
	uint32 ColumnCount() const;
	const char *ColumnName(uint32 column) const;
	DBQueryResult::ColType ColumnType(uint32 column) const;
	
protected:
	//for DBQueryResult
	friend class DBQueryResult;
	void SetData(DBQueryResult *res, MYSQL_ROW &row, const uint32 *lengths);

	MYSQL_ROW m_row;
	const uint32 *m_lengths;
	DBQueryResult *m_result;
};

class DBcore {
public:
	enum eStatus { Closed, Connected, Error };
	
	DBcore();
	~DBcore();
	eStatus	GetStatus() const { return pStatus; }
	
	//new shorter syntax:
	//query which returns a result (error is stored in the result if it occurs)
	bool	RunQuery(DBQueryResult &into, const char *query_fmt, ...);
	//query which returns no information except error status
	bool	RunQuery(DBerror &err, const char *query_fmt, ...);
	//query which returns affected rows:
	bool	RunQuery(DBerror &err, uint32 &affected_rows, const char *query_fmt, ...);
	//query which returns last insert ID:
	bool	RunQueryLID(DBerror &err, uint32 &last_insert_id, const char *query_fmt, ...);
	
	//old style to be used with MakeAnyLengthString
	bool	RunQuery(const char* query, int32 querylen, char* errbuf = 0, MYSQL_RES** result = 0, int32* affected_rows = 0, int32* last_insert_id = 0, int32* errnum = 0, bool retry = true);
	
	int32	DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen);
	void	DoEscapeString(std::string &to, const std::string &from);
	static bool IsSafeString(const char *str);
	void	ping();
	
//	static bool	ReadDBINI(char *host, char *user, char *pass, char *db, int32 &port, bool &compress, bool *items);
	bool	Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, int16 iPort, int32* errnum = 0, char* errbuf = 0, bool iCompress = false, bool iSSL = false);
	bool	Open(DBerror &err, const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, int16 iPort, bool iCompress = false, bool iSSL = false);

protected:
	MYSQL*	getMySQL(){ return &mysql; }
	
private:
	//MDatabase must be locked before these calls:
	bool	Open_locked(int32* errnum = 0, char* errbuf = 0);
	bool	DoQuery_locked(DBerror &err, const char *query, int32 querylen, bool retry = true);
	
	MYSQL	mysql;
	Mutex	MDatabase;
	eStatus pStatus;
	
	std::string	pHost;
	std::string	pUser;
	std::string	pPassword;
	std::string	pDatabase;
	bool	pCompress;
	int16	pPort;
	bool	pSSL;
};

// helper class to implement a sequence concept.
// made an object because the mysql method for implementing them feels 
// like a hack and I didn't want it spread around the code.
class DBSequence {
public:
	DBSequence(DBcore *db, const char *table_name)
	: m_db(db), m_table(table_name) {}

	bool Init();	//setup without forcing the starting value. (sequence table must exist)
	bool Init(uint32 last_used_value);		//setup the sequence with the specified last used value.
	bool Init(const char *last_used_query, uint32 default_if_empty);	//determine the last used value by running the specified query.
	
	uint32 NextValue();
protected:
	DBcore *const m_db;
	const std::string m_table;
};

void ListToINString(const std::vector<uint32> &ints, std::string &into, const char *if_empty);

#endif
