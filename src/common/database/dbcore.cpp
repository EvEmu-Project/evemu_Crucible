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

#include "CommonPCH.h"

#include "database/dbcore.h"

#include "log/LogNew.h"
#include "log/logsys.h"
#include "utils/misc.h"

//#define COLUMN_BOUNDS_CHECKING

DBcore::DBcore(bool compress, bool ssl) : pCompress(compress), pSSL(ssl)
{
    mysql_init(&mysql);
    pStatus = Closed;
}

DBcore::~DBcore()
{
    mysql_close(&mysql);
}

// Sends the MySQL server a ping
void DBcore::ping()
{
    if (!MDatabase.trylock())
    {
        // well, if it's locked, someone's using it. If someone's using it, it doesn't need a ping
        return;
    }
    mysql_ping(&mysql);
    MDatabase.unlock();
}

//query which returns a result (error is stored in the result if it occurs)
bool DBcore::RunQuery(DBQueryResult &into, const char *query_fmt, ...) {
    LockMutex lock(&MDatabase);

    char query[16384];
    va_list vlist;
    va_start(vlist, query_fmt);
    uint32 querylen = vsnprintf(query, 16384, query_fmt, vlist);
    va_end(vlist);

    if(!DoQuery_locked(into.error, query, querylen)) {
        return false;
    }

    uint32 col_count = mysql_field_count(&mysql);
    if(col_count == 0) {
        into.error.SetError(0xFFFF, "DBcore::RunQuery: No Result");
        sLog.Error("DBCore Query", "Query: %s failed because did not return a result", query);
        return false;
    }

    MYSQL_RES *result = mysql_store_result(&mysql);

    //give them the result set.
    into.SetResult(&result, col_count);

    return true;
}

//query which returns no information except error status
bool DBcore::RunQuery(DBerror &err, const char *query_fmt, ...) {
    LockMutex lock(&MDatabase);

    va_list args;
    va_start(args, query_fmt);
    char *query = NULL;
    uint32 querylen = vasprintf(&query, query_fmt, args);
    va_end(args);

    if(!DoQuery_locked(err, query, querylen)) {
        free(query);
        return false;
    }

    free(query);
    return true;
}

//query which returns affected rows:
bool DBcore::RunQuery(DBerror &err, uint32 &affected_rows, const char *query_fmt, ...) {
    LockMutex lock(&MDatabase);

    va_list args;
    va_start(args, query_fmt);
    char *query = NULL;
    uint32 querylen = vasprintf(&query, query_fmt, args);
    va_end(args);

    if(!DoQuery_locked(err, query, querylen)) {
        free(query);
        return false;
    }
    free(query);

    affected_rows = mysql_affected_rows(&mysql);

    return true;
}

//query which returns last insert ID:
bool DBcore::RunQueryLID(DBerror &err, uint32 &last_insert_id, const char *query_fmt, ...) {
    LockMutex lock(&MDatabase);

    va_list args;
    va_start(args, query_fmt);
    char *query = NULL;
    uint32 querylen = vasprintf(&query, query_fmt, args);
    va_end(args);

    if(!DoQuery_locked(err, query, querylen)) {
        free(query);
        return false;
    }
    free(query);

    last_insert_id = mysql_insert_id(&mysql);

    return true;
}

bool DBcore::DoQuery_locked(DBerror &err, const char *query, int32 querylen, bool retry)
{
    if (pStatus != Connected)
        Open_locked();

    if (mysql_real_query(&mysql, query, querylen)) {
        int num = mysql_errno(&mysql);

        if (num == CR_SERVER_GONE_ERROR)
            pStatus = Error;

        if (retry && (num == CR_SERVER_LOST || num == CR_SERVER_GONE_ERROR))
        {
            sLog.Error("DBCore", "Lost connection, attempting to recover....");
            return DoQuery_locked(err, query, querylen, false);
        }

        pStatus = Error;
        err.SetError(num, mysql_error(&mysql));
        sLog.Error("DBCore Query", "#%d in '%s': %s", err.GetErrno(), query, err.c_str());
        return false;
    }

    err.ClearError();
    return true;
}


bool DBcore::RunQuery(const char* query, int32 querylen, char* errbuf, MYSQL_RES** result, int32* affected_rows, int32* last_insert_id, int32* errnum, bool retry) {
    if (errnum)
        *errnum = 0;
    if (errbuf)
        errbuf[0] = 0;
    LockMutex lock(&MDatabase);

    DBerror err;
    if(!DoQuery_locked(err, query, querylen, retry))
    {
        sLog.Error("DBCore Query", "Query: %s failed", query);
        if(errnum != NULL)
            *errnum = err.GetErrno();
        if(errbuf != NULL)
            strcpy(errbuf, err.c_str());
        return false;
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
            sLog.Error("DBCore Query", "Query: %s failed because it should return a result", query);
            return false;
        }
    }
    if (affected_rows)
        *affected_rows = mysql_affected_rows(&mysql);
    if (last_insert_id)
        *last_insert_id = mysql_insert_id(&mysql);
    return true;
}

int32 DBcore::DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen)
{
    return mysql_real_escape_string(&mysql, tobuf, frombuf, fromlen);
}

void DBcore::DoEscapeString(std::string &to, const std::string &from)
{
    uint32 len = (uint32)from.length();
    to.resize(len*2 + 1);   // make enough room
    uint32 esc_len = mysql_real_escape_string(&mysql, &to[0], from.c_str(), len);
    to.resize(esc_len+1); // optional.
}

//look for things in the string which might cause SQL problems
bool DBcore::IsSafeString(const char *str) {
    for(; *str != '\0'; str++) {
        switch(*str) {
        case '\'':
        case '\\':
            return false;
        }
    }
    return true;
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
        return false;
    }

    return true;
}


bool DBcore::Open_locked(int32* errnum, char* errbuf) {
    if (errbuf)
        errbuf[0] = 0;
    if (GetStatus() == Connected)
        return true;
    if (GetStatus() == Error)
        mysql_close(&mysql);    //do we need to call init again?
    if (pHost.empty())
        return false;

    sLog.Log("dbcore", "Connecting to\n\tDB:\t%s\n\tserver:\t%s:%d\n\tuser:\t%s", pDatabase.c_str(), pHost.c_str(), pPort, pUser.c_str());

    /*
    Quagmire - added CLIENT_FOUND_ROWS flag to the connect
    otherwise DB update calls would say 0 rows affected when the value already equaled
    what the function was trying to set it to, therefore the function would think it failed
    */
    int32 flags = CLIENT_FOUND_ROWS;
    if (pCompress)
        flags |= CLIENT_COMPRESS;
    if (pSSL)
        flags |= CLIENT_SSL;
    if (mysql_real_connect(&mysql, pHost.c_str(), pUser.c_str(), pPassword.c_str(), pDatabase.c_str(), pPort, 0, flags)) {
        pStatus = Connected;
    } else {
        pStatus = Error;
        if (errnum)
            *errnum = mysql_errno(&mysql);
        if (errbuf)
            snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
        return false;
    }

    // force MySQL to not convert utf8 to latin1, because
    // client wants unconverted raw utf8
    if(mysql_set_character_set(&mysql, "utf8") != 0) {
        pStatus = Error;
        if(errnum)
            *errnum = mysql_errno(&mysql);
        if(errbuf)
            snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
        return false;
    }

    return true;
}

/************************************************************************/
/* DBerror                                                              */
/************************************************************************/
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

/************************************************************************/
/* DBQueryResult                                                        */
/************************************************************************/
DBQueryResult::DBQueryResult()
: mColumnCount( 0 ),
  mResult( NULL ),
  mFields( NULL )
{
}

DBQueryResult::~DBQueryResult()
{
    SafeDeleteArray( mFields );

    if( NULL != mResult )
        mysql_free_result( mResult );
}

bool DBQueryResult::GetRow( DBResultRow& into )
{
    if( NULL == mResult )
        return false;

    MYSQL_ROW row = mysql_fetch_row( mResult );
    if( NULL == row )
        return false;

    uint32* lengths = (uint32*)mysql_fetch_lengths( mResult );
    if( NULL == lengths )
        return false;

    into.SetData( this, row, lengths );
    return true;
}

const char* DBQueryResult::ColumnName( uint32 index ) const
{
#ifdef COLUMN_BOUNDS_CHECKING
    if( index >= ColumnCount() )
    {
        sLog.Error( "DBCore Query Result", "ColumnName: Column index %d exceeds number of columns (%s) in row\n", column, ColumnCount() );
        return "(ERROR)";      //nothing better to do...
    }
#endif
    return mFields[ index ]->name;
}

/* mysql to DBTYPE convention table */
static const DBTYPE DBTYPE_MYSQL_TABLE_SIGNED[] =
{
    DBTYPE_I4,      //[0]MYSQL_TYPE_DECIMAL             /* assumption that this variable will never be bigger than 32 bits... if it will then we need to change it to I8 */
    DBTYPE_I1,      //[1]MYSQL_TYPE_TINY
    DBTYPE_I2,      //[2]MYSQL_TYPE_SHORT
    DBTYPE_I4,      //[3]MYSQL_TYPE_LONG
    DBTYPE_R4,      //[4]MYSQL_TYPE_FLOAT
    DBTYPE_R8,      //[5]MYSQL_TYPE_DOUBLE
    DBTYPE_STR,     //[6]MYSQL_TYPE_NULL               /* assumption that this is correct */
    DBTYPE_FILETIME,//[7]MYSQL_TYPE_TIMESTAMP
    DBTYPE_I8,      //[8]MYSQL_TYPE_LONGLONG
    DBTYPE_I4,      //[9]MYSQL_TYPE_INT24
    DBTYPE_FILETIME,//[10]MYSQL_TYPE_DATE
    DBTYPE_FILETIME,//[11]MYSQL_TYPE_TIME
    DBTYPE_FILETIME,//[12]MYSQL_TYPE_DATETIME
    DBTYPE_FILETIME,//[13]MYSQL_TYPE_YEAR
    DBTYPE_FILETIME,//[14]MYSQL_TYPE_NEWDATE
    DBTYPE_STR,     //[15]MYSQL_TYPE_VARCHAR
    DBTYPE_BOOL,    //[16]MYSQL_TYPE_BIT
    DBTYPE_STR,     //[17]MYSQL_TYPE_NEWDECIMAL=246     /* assumption and prob wrong */
    DBTYPE_STR,     //[18]MYSQL_TYPE_ENUM=247
    DBTYPE_STR,     //[19]MYSQL_TYPE_SET=248            /* unhandled */
    DBTYPE_STR,     //[20]MYSQL_TYPE_TINY_BLOB=249
    DBTYPE_STR,     //[21]MYSQL_TYPE_MEDIUM_BLOB=250
    DBTYPE_STR,     //[22]MYSQL_TYPE_LONG_BLOB=251
    DBTYPE_STR,     //[23]MYSQL_TYPE_BLOB=252
    DBTYPE_STR,     //[24]MYSQL_TYPE_VAR_STRING=253
    DBTYPE_STR,     //[25]MYSQL_TYPE_STRING=254
    DBTYPE_STR,     //[26]MYSQL_TYPE_GEOMETRY=255       /* unhandled */
};

static const DBTYPE DBTYPE_MYSQL_TABLE_UNSIGNED[] =
{
    DBTYPE_UI4,      //[0]MYSQL_TYPE_DECIMAL             /* assumption that this variable will never be bigger than 32 bits... if it will then we need to change it to I8 */
    DBTYPE_UI1,      //[1]MYSQL_TYPE_TINY
    DBTYPE_UI2,      //[2]MYSQL_TYPE_SHORT
    DBTYPE_UI4,      //[3]MYSQL_TYPE_LONG
    DBTYPE_R4,      //[4]MYSQL_TYPE_FLOAT
    DBTYPE_R8,      //[5]MYSQL_TYPE_DOUBLE
    DBTYPE_STR,     //[6]MYSQL_TYPE_NULL               /* assumption that this is correct */
    DBTYPE_FILETIME,//[7]MYSQL_TYPE_TIMESTAMP
    DBTYPE_UI8,      //[8]MYSQL_TYPE_LONGLONG
    DBTYPE_UI4,      //[9]MYSQL_TYPE_INT24
    DBTYPE_FILETIME,//[10]MYSQL_TYPE_DATE
    DBTYPE_FILETIME,//[11]MYSQL_TYPE_TIME
    DBTYPE_FILETIME,//[12]MYSQL_TYPE_DATETIME
    DBTYPE_FILETIME,//[13]MYSQL_TYPE_YEAR
    DBTYPE_FILETIME,//[14]MYSQL_TYPE_NEWDATE
    DBTYPE_STR,     //[15]MYSQL_TYPE_VARCHAR
    DBTYPE_BOOL,    //[16]MYSQL_TYPE_BIT
    DBTYPE_STR,     //[17]MYSQL_TYPE_NEWDECIMAL=246     /* assumption and prob wrong */
    DBTYPE_STR,     //[18]MYSQL_TYPE_ENUM=247
    DBTYPE_STR,     //[19]MYSQL_TYPE_SET=248            /* unhandled */
    DBTYPE_STR,     //[20]MYSQL_TYPE_TINY_BLOB=249
    DBTYPE_STR,     //[21]MYSQL_TYPE_MEDIUM_BLOB=250
    DBTYPE_STR,     //[22]MYSQL_TYPE_LONG_BLOB=251
    DBTYPE_STR,     //[23]MYSQL_TYPE_BLOB=252
    DBTYPE_STR,     //[24]MYSQL_TYPE_VAR_STRING=253
    DBTYPE_STR,     //[25]MYSQL_TYPE_STRING=254
    DBTYPE_STR,     //[26]MYSQL_TYPE_GEOMETRY=255       /* unhandled */
};

DBTYPE DBQueryResult::ColumnType( uint32 index ) const
{
#ifdef COLUMN_BOUNDS_CHECKING
    if( index >= ColumnCount() )
    {
        sLog.Error( "DBCore Query Result", "ColumnType: Column index %d exceeds number of columns (%s) in row\n", column, ColumnCount() );
        return DBTYPE_STR;     //nothing better to do...
    }
#endif

    uint32 columnType = mFields[ index ]->type;

    /* debug checks */
    /*assert(
        columnType != MYSQL_TYPE_NEWDECIMAL &&
        columnType != MYSQL_TYPE_NULL &&
        columnType != MYSQL_TYPE_BIT &&
        columnType != MYSQL_TYPE_ENUM &&
        columnType != MYSQL_TYPE_SET &&
        columnType != MYSQL_TYPE_TINY_BLOB &&
        columnType != MYSQL_TYPE_MEDIUM_BLOB &&
        columnType != MYSQL_TYPE_LONG_BLOB &&
        columnType != MYSQL_TYPE_BLOB &&
        columnType != MYSQL_TYPE_SET &&
        columnType != MYSQL_TYPE_GEOMETRY );*/

    if ( columnType > 245 )
        /* tricky needs to be checked */
        columnType-=229; // MYSQL_TYPE_NEWDECIMAL - MYSQL_TYPE_BIT

    DBTYPE type;
    if ( mFields[ index ]->flags & UNSIGNED_FLAG )
        type = DBTYPE_MYSQL_TABLE_UNSIGNED[ columnType ];
    else
        type = DBTYPE_MYSQL_TABLE_SIGNED[ columnType ];

    /* possible add tracing here... */
    assert( type );

    return type;
}

void DBQueryResult::SetResult( MYSQL_RES** res, uint32 colCount )
{
    SafeDeleteArray( mFields );

    if( NULL != mResult )
        mysql_free_result( mResult );

    mResult = *res;
    *res = NULL;
    mColumnCount = colCount;

    if( NULL != mResult )
	{
		mFields = new MYSQL_FIELD*[ ColumnCount() ];
	    
		// we are
		for( uint32 i = 0; i < ColumnCount(); ++i )
			mFields[ i ] = mysql_fetch_field( mResult );
	}
}

void DBQueryResult::Reset()
{
    if( NULL != mResult )
        mysql_data_seek( mResult, 0);
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
        return 0;
    return(m_result->ColumnCount());
}

const char *DBResultRow::ColumnName(uint32 column) const {
    if(m_result == NULL)
        return "NULL RESULT";
    return m_result->ColumnName(column);
}

DBTYPE DBResultRow::ColumnType(uint32 column) const {
    if(m_result == NULL)
    {
        sLog.Error("DBCore DBQueryResult", "unable to get ColumnType");
        return DBTYPE_STR;
    }
    return m_result->ColumnType(column);
}

uint32 DBResultRow::GetColumnLength(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetColumnLength: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif
    return m_lengths[column];
}


//these all assume that row is valid.. its your fault if it is not!

int32 DBResultRow::GetInt(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetInt: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif
    //use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
    return(strtol(m_row[column], NULL, 0));
}

uint32 DBResultRow::GetUInt(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetUInt: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif
    //use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
    return(strtoul(m_row[column], NULL, 0));
}
//_strtoi64
//strtoll, strtoull 
int64 DBResultRow::GetInt64(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetInt64: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif

    int64 value;
    sscanf( m_row[column], I64d, &value );

    return value;
}

uint64 DBResultRow::GetUInt64(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetUInt64: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif

    uint64 value;
    sscanf( m_row[column], I64u, &value );

    return value;
}

float DBResultRow::GetFloat(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetFloat: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif
#ifdef WIN32
    return (float)atof(m_row[column]);
#else
    return strtof(m_row[column], NULL);
#endif
}

double DBResultRow::GetDouble(uint32 column) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetDouble: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif
    return strtod(m_row[column], NULL);
}

uint32 DBResultRow::GetBinary(uint32 column, uint8 *into, uint32 in_length) const {
#ifdef COLUMN_BOUNDS_CHECKING
    if(column >= ColumnCount()) {
        sLog.Error("DBCore Result Row", "GetBinary: Column index %u exceeds number of columns (%u) in row", column, ColumnCount());
        return 0;       //nothing better to do...
    }
#endif
    if(in_length < m_lengths[column])
    {
        sLog.Error("DBCore Result Row", "GetBinary: insufficient buffer space provided for column %u of length %u (%u provided)", column, m_lengths[column], in_length);
        return 0;
    }
    memcpy(into, m_row[column], m_lengths[column]);
    return m_lengths[column];
}

