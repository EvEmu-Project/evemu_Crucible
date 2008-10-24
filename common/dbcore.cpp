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


#include "../common/common.h"

#include "dbcore.h"

#include <errmsg.h>
#include <mysqld_error.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include "logsys.h"
#include "MiscFunctions.h"
#include "misc.h"

#define COLUMN_BOUNDS_CHECKING

DBcore::DBcore() {
	mysql_init(&mysql);
	pCompress = false;
	pSSL = false;
	pStatus = Closed;
}

DBcore::~DBcore() {
	mysql_close(&mysql);
}

// Sends the MySQL server a keepalive
void DBcore::ping() {
	if (!MDatabase.trylock()) {
		// well, if's it's locked, someone's using it. If someone's using it, it doesnt need a keepalive
		return;
	}
	mysql_ping(&mysql);
	MDatabase.unlock();
}

//query which returns a result (error is stored in the result if it occurs)
bool DBcore::RunQuery(DBQueryResult &into, const char *query_fmt, ...) {
	LockMutex lock(&MDatabase);

	va_list args;
	va_start(args, query_fmt);
	char *query = NULL;
	uint32 querylen = vaMakeAnyLenString(&query, query_fmt, args);
	va_end(args);
	
	if(!DoQuery_locked(into.error, query, querylen)) {
		delete[] query;
		return(false);
	}

	uint32 col_count = mysql_field_count(&mysql);
	if(col_count == 0) {
		into.error.SetError(0xFFFF, "DBcore::RunQuery: No Result");
		_log(DATABASE__QUERIES, "Query failed due to no result");
		_log(DATABASE__ALL_ERRORS, "DB Query '%s' did not return a result, but the caller requested them.", query);
		delete[] query;
		return(false);
	}
	delete[] query;

	MYSQL_RES *result = mysql_store_result(&mysql);

	/*                                                                              
     * Debug logging
    */
	if(is_log_enabled(DATABASE__RESULTS)) {
		MYSQL_ROW row;
		_log(DATABASE__RESULTS, "Query Results:");
		std::string c;
		
		uint32 r;
		MYSQL_FIELD *fields = mysql_fetch_fields(result);
		c = "| ";
		for(r = 0; r < col_count; r++) {
			c += fields[r].name;
			c += " | ";
		}
		_log(DATABASE__RESULTS, "%s", c.c_str());

		
		while((row = mysql_fetch_row(result))) {
			c = "| ";
			for(r = 0; r < col_count; r++) {
				if(row[r] == NULL) {
					c += "NULL";
				} else {
					switch(fields[r].type) {
					case FIELD_TYPE_TINY_BLOB:
					case FIELD_TYPE_MEDIUM_BLOB:
					case FIELD_TYPE_LONG_BLOB:
					case FIELD_TYPE_BLOB:
						c += "(BINARY)";
						break;
					default:
						c += row[r];
						break;
					}
				}
				c += " | ";
			}
			_log(DATABASE__RESULTS, "%s", c.c_str());
		}
		
		//reset the result.
		mysql_data_seek(result, 0);
	}

	//give them the result set.
	into.SetResult(&result, col_count);
	
	return(true);
}

//query which returns no information except error status
bool DBcore::RunQuery(DBerror &err, const char *query_fmt, ...) {
	LockMutex lock(&MDatabase);

	va_list args;
	va_start(args, query_fmt);
	char *query = NULL;
	uint32 querylen = vaMakeAnyLenString(&query, query_fmt, args);
	va_end(args);

	if(!DoQuery_locked(err, query, querylen)) {
		delete[] query;
		return(false);
	}
	
	delete[] query;
	return(true);
}

//query which returns affected rows:
bool DBcore::RunQuery(DBerror &err, uint32 &affected_rows, const char *query_fmt, ...) {
	LockMutex lock(&MDatabase);

	va_list args;
	va_start(args, query_fmt);
	char *query = NULL;
	uint32 querylen = vaMakeAnyLenString(&query, query_fmt, args);
	va_end(args);

	if(!DoQuery_locked(err, query, querylen)) {
		delete[] query;
		return(false);
	}
	delete[] query;
	
	affected_rows = mysql_affected_rows(&mysql);
	
	return(true);
}

//query which returns last insert ID:
bool DBcore::RunQueryLID(DBerror &err, uint32 &last_insert_id, const char *query_fmt, ...) {
	LockMutex lock(&MDatabase);

	va_list args;
	va_start(args, query_fmt);
	char *query = NULL;
	uint32 querylen = vaMakeAnyLenString(&query, query_fmt, args);
	va_end(args);
	
	if(!DoQuery_locked(err, query, querylen)) {
		delete[] query;
		return(false);
	}
	delete[] query;
	
	last_insert_id = mysql_insert_id(&mysql);
	
	return(true);
}

bool DBcore::DoQuery_locked(DBerror &err, const char *query, int32 querylen, bool retry) {
	if (pStatus != Connected)
		Open_locked();

	_log(DATABASE__QUERIES, "Executing query: %s", query);
	
	if (mysql_real_query(&mysql, query, querylen)) {
		int num = mysql_errno(&mysql);
		
		if (num == CR_SERVER_GONE_ERROR)
			pStatus = Error;
		
		if (retry && (num == CR_SERVER_LOST || num == CR_SERVER_GONE_ERROR)) {
			_log(DATABASE__ERROR, "Database Error: Lost connection, attempting to recover....");
			return(DoQuery_locked(err, query, querylen, false));
		}
			
		pStatus = Error;
		err.SetError(num, mysql_error(&mysql));
		_log(DATABASE__ALL_ERRORS, "DB Query Error #%d in '%s': %s", err.GetErrno(), query, err.c_str());
		return(false);
	}
	
	err.ClearError();
	return(true);
}
	

bool DBcore::RunQuery(const char* query, int32 querylen, char* errbuf, MYSQL_RES** result, int32* affected_rows, int32* last_insert_id, int32* errnum, bool retry) {
	if (errnum)
		*errnum = 0;
	if (errbuf)
		errbuf[0] = 0;
	LockMutex lock(&MDatabase);

	_log(DATABASE__QUERIES, "Running query: %s", query);
	
	DBerror err;
	if(!DoQuery_locked(err, query, querylen, retry)) {
		_log(DATABASE__QUERIES, "Query failed");
		if(errnum != NULL)
			*errnum = err.GetErrno();
		if(errbuf != NULL)
			strcpy(errbuf, err.c_str());
		return(false);
	}
	
	if (result) {
		if(mysql_field_count(&mysql)) {
			*result = mysql_store_result(&mysql);
		} else {
			*result = NULL;
			if (errnum)
				*errnum = UINT_MAX;
			if (errbuf)
				strcpy(errbuf, "DBcore::RunQuery: No Result");
			_log(DATABASE__QUERIES, "Query failed due to no result");
			_log(DATABASE__ALL_ERRORS, "DB Query '%s' did not return a result, but the caller requested them.", query);
			return(false);
		}
	}
	if (affected_rows)
		*affected_rows = mysql_affected_rows(&mysql);
	if (last_insert_id)
		*last_insert_id = mysql_insert_id(&mysql);
	_log(DATABASE__QUERIES, "Query successful");
	return(true);
}

int32 DBcore::DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen) {
//	No good reason to lock the DB, we only need it in the first place to check char encoding.
//	LockMutex lock(&MDatabase);
	return mysql_real_escape_string(&mysql, tobuf, frombuf, fromlen);
}

void DBcore::DoEscapeString(std::string &to, const std::string &from) {
	uint32 len = from.length();
	char *buf = new char[len*2 + 1];
	len = mysql_real_escape_string(&mysql, buf, from.c_str(), len);
	to.assign(buf, len);
	delete[] buf;
}

//look for things in the string which might cause SQL problems
bool DBcore::IsSafeString(const char *str) {
	for(; *str != '\0'; str++) {
		switch(*str) {
		case '\'':
		case '\\':
			return(false);
		}
	}
	return(true);
}

bool DBcore::Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, int16 iPort, int32* errnum, char* errbuf, bool iCompress, bool iSSL) {
	LockMutex lock(&MDatabase);
	
	pHost = iHost;
	pUser = iUser;
	pPassword = iPassword;
	pDatabase = iDatabase;
	pCompress = iCompress;
	pPort = iPort;
	pSSL = iSSL;
	
	return Open_locked(errnum, errbuf);
}

bool DBcore::Open(DBerror &err, const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, int16 iPort, bool iCompress, bool iSSL) {
	LockMutex lock(&MDatabase);
	
	pHost = iHost;
	pUser = iUser;
	pPassword = iPassword;
	pDatabase = iDatabase;
	pCompress = iCompress;
	pPort = iPort;
	pSSL = iSSL;

	int32 errnum;
	char errbuf[1024];
	
	if(!Open_locked(&errnum, errbuf)) {
		err.SetError(errnum, errbuf);
		return(false);
	}

	return(true);
}


bool DBcore::Open_locked(int32* errnum, char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	if (GetStatus() == Connected)
		return true;
	if (GetStatus() == Error)
		mysql_close(&mysql);	//do we need to call init again?
	if (pHost.empty())
		return false;

	_log(DATABASE__MESSAGE, "Connecting to DB %s on MySQL server %s:%d as user %s", pDatabase.c_str(), pHost.c_str(), pPort, pUser.c_str());
	
	/*
	Quagmire - added CLIENT_FOUND_ROWS flag to the connect
	otherwise DB update calls would say 0 rows affected when the value already equalled
	what the function was tring to set it to, therefore the function would think it failed 
	*/
	int32 flags = CLIENT_FOUND_ROWS;
	if (pCompress)
		flags |= CLIENT_COMPRESS;
	if (pSSL)
		flags |= CLIENT_SSL;
	if (mysql_real_connect(&mysql, pHost.c_str(), pUser.c_str(), pPassword.c_str(), pDatabase.c_str(), pPort, 0, flags)) {
		pStatus = Connected;
		return true;
	}
	else {
		if (errnum)
			*errnum = mysql_errno(&mysql);
		if (errbuf)
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
		pStatus = Error;
		return false;
	}
}


DBerror::DBerror() {
	ClearError();
}

void DBerror::SetError(uint32 err, const char *str) {
	m_str = str;
	m_errno = err;
}

void DBerror::ClearError() {
	m_str = "No Error";
	m_errno = 0;
}

DBQueryResult::DBQueryResult()
: m_res(NULL)
{
}

DBQueryResult::~DBQueryResult() {
	if(m_res != NULL)
		mysql_free_result(m_res);
}

bool DBQueryResult::GetRow(DBResultRow &into) {
	if(m_res == NULL)
		return(false);
	MYSQL_ROW row = mysql_fetch_row(m_res);
	if(row == NULL)
		return(false);
	uint32 *lengths = (uint32*)mysql_fetch_lengths(m_res);
	if(lengths == NULL)
		return(false);
	into.SetData(this, row, lengths);
	return(true);
}

const char *DBQueryResult::ColumnName(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "ColumnName: Column index %d exceeds number of columns (%s) in row", column, ColumnCount());
		return("(ERROR)");		//nothing better to do...
	}
#endif
	return(m_fields[column].name);
}

DBQueryResult::ColType DBQueryResult::ColumnType(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "ColumnType: Column index %d exceeds number of columns (%s) in row", column, ColumnCount());
		return(String);		//nothing better to do...
	}
#endif
	switch(m_fields[column].type) {
	case FIELD_TYPE_TINY:
		return(Int8);
	case FIELD_TYPE_SHORT:
		return(Int16);
	case FIELD_TYPE_INT24:	//3-byte medium int
	case FIELD_TYPE_LONG:
		return(Int32);
	case FIELD_TYPE_LONGLONG:
		return(Int64);
	case FIELD_TYPE_FLOAT:
	case FIELD_TYPE_DOUBLE:
	case FIELD_TYPE_DECIMAL:	//fixed-point number
		return(Real);
	case FIELD_TYPE_TIMESTAMP:
	case FIELD_TYPE_DATE:
	case FIELD_TYPE_TIME:
	case FIELD_TYPE_DATETIME:
	case FIELD_TYPE_YEAR:
	case FIELD_TYPE_NEWDATE:
		return(DateTime);
	case FIELD_TYPE_TINY_BLOB:
	case FIELD_TYPE_MEDIUM_BLOB:
	case FIELD_TYPE_LONG_BLOB:
	case FIELD_TYPE_BLOB:
		return(Binary);
	case FIELD_TYPE_NULL:
	case FIELD_TYPE_SET:		//unhandled
	case FIELD_TYPE_GEOMETRY:	//unhandled
	case FIELD_TYPE_VAR_STRING:
	case FIELD_TYPE_STRING:
	case FIELD_TYPE_ENUM:
		return(String);
	}
	//..? safest answer:
	return(String);
}

void DBQueryResult::SetResult(MYSQL_RES **res, uint32 colcount) {
	if(m_res != NULL)
		mysql_free_result(m_res);
	m_res = *res;
	*res = NULL;
	m_col_count = colcount;
	if(m_res != NULL)
		m_fields = mysql_fetch_fields(m_res);
	else
		m_fields = NULL;
}

void DBQueryResult::Reset() {
	if(m_res != NULL)
		mysql_data_seek(m_res, 0);
}

DBResultRow::DBResultRow()
: m_row(NULL),
  m_lengths(NULL),
  m_result(NULL)
{
}

void DBResultRow::SetData(DBQueryResult *res, MYSQL_ROW &row, const uint32 *lengths) {
	m_row = row;
	m_result = res;
	m_lengths = lengths;
}

uint32 DBResultRow::ColumnCount() const {
	if(m_result == NULL)
		return(0);
	return(m_result->ColumnCount());
}

const char *DBResultRow::ColumnName(uint32 column) const {
	if(m_result == NULL)
		return("NULL RESULT");
	return(m_result->ColumnName(column));
}

DBQueryResult::ColType DBResultRow::ColumnType(uint32 column) const {
	if(m_result == NULL)
		return(DBQueryResult::String);
	return(m_result->ColumnType(column));
}

uint32 DBResultRow::GetColumnLength(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetColumnLength: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	return(m_lengths[column]);
}


//these all assume that row is valid.. its your fault if it is not!

int32 DBResultRow::GetInt(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetInt: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	//use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
	return(strtol(m_row[column], NULL, 0));
}

uint32 DBResultRow::GetUInt(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetUInt: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	//use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
	return(strtoul(m_row[column], NULL, 0));
}

int64 DBResultRow::GetInt64(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetInt: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	//use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
	return(strtoll(m_row[column], NULL, 0));
}

uint64 DBResultRow::GetUInt64(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetUInt: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	//use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
	return(strtoull(m_row[column], NULL, 0));
}

float DBResultRow::GetFloat(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetFloat: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
#ifdef WIN32
	return(atof(m_row[column]));
#else
	return(strtof(m_row[column], NULL));
#endif
}

double DBResultRow::GetDouble(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetDouble: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	return(strtod(m_row[column], NULL));
}

uint32 DBResultRow::GetBinary(uint32 column, byte *into, uint32 in_length) const {
#ifdef COLUMN_BOUNDS_CHECKING
	if(column >= ColumnCount()) {
		_log(DATABASE__ERROR, "GetBinary: Column index %d exceeds number of columns (%d) in row", column, ColumnCount());
		return(0);		//nothing better to do...
	}
#endif
	if(in_length < m_lengths[column]) {
		_log(DATABASE__ERROR, "GetBinary: insufficient buffer space provided for column %d of length %d (%d provided)", column, m_lengths[column], in_length);
		return(0);
	}
	memcpy(into, m_row[column], m_lengths[column]);
	return(m_lengths[column]);
}


void ListToINString(const std::vector<uint32> &ints, std::string &into, const char *if_empty) {
	if(ints.empty()) {
		into = if_empty;
		return;
	}
	
	char *inbuffer = new char[ints.size()*13];

	std::vector<uint32>::const_iterator cur, end;
	char *ptr = inbuffer;
	cur = ints.begin();
	end = ints.end();
	for(; cur != end; cur++) {
		if(ptr != inbuffer) {
			*ptr = ',';
			ptr++;
		}
		ptr += snprintf(ptr, 15, "%lu", *cur);
	}

	into = inbuffer;
	delete[] inbuffer;
}


bool DBSequence::Init() {
	return(true);
}

bool DBSequence::Init(uint32 last_used_value) {
	DBerror err;
	if(!m_db->RunQuery(err, 
		"CREATE TABLE IF NOT EXISTS %s (last_used INT NOT NULL)",
		m_table.c_str()
	))
	{
		_log(DATABASE__ERROR, "Failed to create table for sequence '%s': %s", m_table.c_str(), err.c_str());
		return(false);
	}
	
	if(!m_db->RunQuery(err, 
		"REPLACE INTO %s (last_used) VALUES(%lu)",
		m_table.c_str(), last_used_value
	))
	{
		_log(DATABASE__ERROR, "Failed to prime value for sequence '%s': %s", m_table.c_str(), err.c_str());
		return(false);
	}
	return(true);
}
bool DBSequence::Init(const char *last_used_query, uint32 default_if_empty) {
	DBQueryResult res;
	if(!m_db->RunQuery(res, 
		"%s",
		last_used_query
	))
	{
		_log(DATABASE__ERROR, "Failed to query prime value for sequence '%s': %s", m_table.c_str(), res.error.c_str());
		return(false);
	}
	DBResultRow row;
	if(!res.GetRow(row)) {
		//_log(DATABASE__ERROR, "Failed to query prime value for sequence '%s': Query '%s' returned no results.", m_table.c_str(), last_used_query);
		//return(false);
		return(Init( default_if_empty ));
	}
	return(Init( row.GetUInt(0) ));
}

uint32 DBSequence::NextValue() {
	DBerror err;
	uint32 last_insert_id;
	if(!m_db->RunQueryLID(err, last_insert_id,
		"UPDATE %s SET last_used=LAST_INSERT_ID(last_used+1)",
		m_table.c_str()
	))
	{
		_log(DATABASE__ERROR, "Failed to query value for sequence '%s': %s", m_table.c_str(), err.c_str());
		return(0);
	}
	return(last_insert_id);
}










