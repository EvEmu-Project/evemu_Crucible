/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://evemu.dev
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Zhur
 */

#include "eve-core.h"

#include "database/dbcore.h"

#include "log/LogNew.h"
#include "log/logsys.h"
#include "utils/misc.h"
#include "utils/utils_time.h"
//#include "../eve-server/Profiler.h"

#define COLUMN_BOUNDS_CHECKING

// this is to enable profile tracking for db
#define sProfiler ( Profiler::get() )

class Profiler
: public Singleton<Profiler>
{
public:
    void AddTime(uint8 key, double value);
};


DBcore::DBcore()
: mysql(nullptr),
pSocket(false),
pStatus(Closed),
pReconnect(false),
pProfile(false),
pCompress(false),
pSSL(false),
pPort(3306)
{
    mysql_thread_init();    // this is for each thread used for db connections
    mysql = mysql_init(nullptr);
}

void DBcore::Connect(uint* errnum, char* errbuf)
{
    sLog.Cyan("          DB User", " %s", pUser.c_str());
    sLog.Cyan("         DataBase", " %s", pDatabase.c_str());

    // options should be called BEFORE mysql_real_connect()
    if (pSocket) {
        enum mysql_protocol_type prot_type = MYSQL_PROTOCOL_SOCKET;
        if (mysql_options(mysql, MYSQL_OPT_PROTOCOL, (void*)&prot_type) == 0) {
            sLog.Cyan("        DB Server", " Unix Socket Connection");
        } else {
            sLog.Error("        DB Server", " Unix Socket Connection Option Failed");
            enum mysql_protocol_type prot_type = MYSQL_PROTOCOL_TCP;
            if (mysql_options(mysql, MYSQL_OPT_PROTOCOL, (void*)&prot_type) == 0)
                sLog.Cyan("        DB Server", " %s:%d", pHost.c_str(), pPort);
            else
                sLog.Error("        DB Server", " TCP Connection Option Failed");
        }
    } else {
        enum mysql_protocol_type prot_type = MYSQL_PROTOCOL_TCP;
        if (mysql_options(mysql, MYSQL_OPT_PROTOCOL, (void*)&prot_type) == 0)
            sLog.Cyan("        DB Server", " %s:%d", pHost.c_str(), pPort);
        else
            sLog.Error("        DB Server", " TCP Connection Option Failed");
    }

    int32 flags = CLIENT_FOUND_ROWS; //2
    if (pCompress)
        flags |= CLIENT_COMPRESS; //32
    // sql-ssl  needs more info/settings to properly use....however, not needed when using socket under linux
    if (pSSL and !pSocket)
        flags |= CLIENT_SSL;
    sLog.Cyan("    Connect Flags", " %x", flags);
    /*
     *    unsigned int conn_timeout = 2;
     *    // not sure if this one will really be used here
     *    if (mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (void*)&conn_timeout) == 0) {
     *        if (conn_timeout > 60)
     *            sLog.Error(" DataBase Manager", "Connection Timeout set to %us", conn_timeout);
     *        else if (conn_timeout > 40)
     *            sLog.Yellow(" DataBase Manager", "Connection Timeout set to %us", conn_timeout);
     *        else if (conn_timeout > 30)
     *            sLog.Cyan(" DataBase Manager", "Connection Timeout set to %us", conn_timeout);
     *        else
     *            sLog.Green(" DataBase Manager", "Connection Timeout set to %us", conn_timeout);
} else
    sLog.Error(" DataBase Manager", "Connection Timeout Option Failed");
*/
    if (pReconnect) {
        my_bool reconnect = true;
        if (mysql_options(mysql, MYSQL_OPT_RECONNECT, (void*)&reconnect) == 0) // this will enable auto-reconnect...and render my Reconnect() worthless
            sLog.Green(" DataBase Manager", "DataBase AutoReconnect Enabled");
        else
            sLog.Error(" DataBase Manager", "DataBase AutoReconnect Option Failed");
    } else
        sLog.Yellow(" DataBase Manager", "DataBase AutoReconnect Disabled");

    if (mysql_real_connect(mysql, pHost.c_str(), pUser.c_str(), pPassword.c_str(), pDatabase.c_str(), pPort, 0, flags) == nullptr) {
        pStatus = Error;
        *errnum = mysql_errno(mysql);
        if (errbuf != nullptr)
            snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mysql), mysql_error(mysql));
        DBerror err;
        err.SetError(*errnum, errbuf);
        sLog.Error( "       ServerInit", "Unable to connect to the database: %s", err.c_str() );
        return;
    } else {
        pStatus = Connected;
        //mysql_get_socket();
        sLog.Blue(" DataBase Manager", "DataBase Connected");
    }

    // Setup character set we wish to use
    if (mysql_set_character_set(mysql, "utf8") == 0)
        sLog.Cyan(" DataBase Manager", "DataBase Character set: %s", mysql_character_set_name(mysql));
}

bool DBcore::Reconnect()
{
    _log(DATABASE__MESSAGE, "DBCore attempting to recover...");
    Close();
    mysql = mysql_init(nullptr);
    uint errnum = 0;
    char errbuf[1024];
    errbuf[0] = 0;
    MutexLock lock(MDatabase);
    Connect(&errnum, errbuf);

    if (pStatus == Connected)
        _log(DATABASE__MESSAGE, "DBCore recovery successful.  Continuing.");

    return pStatus;
}

void DBcore::Initialize(std::string host, std::string user, std::string password, std::string database, bool compress/*false*/,
                        bool SSL/*false*/, int16 port/*3306*/, bool socket/*false*/, bool reconnect/*false*/, bool profile/*false*/)
{
    if (mysql == nullptr)
        mysql = mysql_init(nullptr);    // try again
    if (mysql == nullptr) {
        sLog.Error( "       ServerInit", "Unable to connect to the database:  mysql_init returned null");
        return;
    }
    if (pStatus == Connected)
        return;

    pHost = host;
    pUser = user;
    pPassword = password;
    pDatabase = database;
    pPort = port;
    pSSL = SSL;
    pCompress = compress;
    pSocket = socket;
    pReconnect = reconnect;
    pProfile = profile;

    if (pHost.empty() or pUser.empty() or pPassword.empty() or pDatabase.empty()) {
        sLog.Error( "       ServerInit", "Unable to connect to the database:  required connect field(s) are empty.");
        return;
    }

    uint errnum = 0;
    char errbuf[1024];
    errbuf[0] = 0;

    MutexLock lock(MDatabase);

    Connect(&errnum, errbuf);
    sLog.Blue(" DataBase Manager", "DataBase Manager Initialized");
}

void DBcore::Close() {
    pStatus = Closed;
    mysql_close(mysql);
    mysql_server_end();
    mysql_thread_end();   // this is for each thread used for db connections
}

/*
void DBcore::CallShutdown()
{
    _log(DATABASE__MESSAGE, "DBCore recovery failed.  Server restarting.");
    std::vector<Client*> list;
    sEntityList.GetClients(list);
    for (auto cur : list)
        cur->SendInfoModalMsg("DBCore lost connection and recovery failed.  Server restarting.");
    Sleep(4000);    // pause running thread to allow players (if any) to view msg
    sConsole.HaltServer(true);
} */

// Sends the MySQL server a ping
void DBcore::ping()
{
    // well, if it's locked, someone's using it. If someone's using it, it doesn't need a ping
    if ( MDatabase.TryLock() ) {
        mysql_ping(mysql);
        MDatabase.Unlock();
    }
}

//query which returns a result (error is stored in the result if it occurs)
bool DBcore::RunQuery(DBQueryResult &into, const char *query_fmt, ...) {
    MutexLock lock(MDatabase);

    char query[4096];
    va_list vlist;
    va_start(vlist, query_fmt);
    int querylen = std::vsnprintf(query, 4096, query_fmt, vlist);
    va_end(vlist);

    if (!DoQuery_locked(into.error, query, querylen))
        return false;

    uint col_count = mysql_field_count(mysql);
    if (col_count == 0) {
        into.error.SetError(0xFFFF, "DBcore::RunQuery: No Result");
        codelog(DATABASE__ERROR, "DBCore::RunQuery: %s failed because it did not return a result", query);
        EvE::traceStack();
        return false;
    }

    into.SetResult(mysql_store_result(mysql), col_count);

    return true;
}

//query which returns only error status
bool DBcore::RunQuery(DBerror &err, const char *query_fmt, ...) {
    MutexLock lock(MDatabase);

    va_list args;
    va_start(args, query_fmt);
    char* query(nullptr);
    int querylen = vasprintf(&query, query_fmt, args);
    va_end(args);

    if (!DoQuery_locked(err, query, querylen)) {
        free(query);
        return false;
    }

    free(query);
    return true;
}

//query which returns affected rows:  (not used)
bool DBcore::RunQuery(DBerror &err, uint32 &affected_rows, const char *query_fmt, ...) {
    MutexLock lock(MDatabase);

    va_list args;
    va_start(args, query_fmt);
    char* query(nullptr);
    int querylen = vasprintf(&query, query_fmt, args);
    va_end(args);

    if (!DoQuery_locked(err, query, querylen)) {
        free(query);
        return false;
    }
    free(query);

    affected_rows = (uint32)mysql_affected_rows(mysql);

    return true;
}

//query which returns last insert ID:
bool DBcore::RunQueryLID(DBerror &err, uint32 &last_insert_id, const char *query_fmt, ...) {
    MutexLock lock(MDatabase);

    va_list args;
    va_start(args, query_fmt);
    char* query(nullptr);
    int querylen = vasprintf(&query, query_fmt, args);
    va_end(args);

    if (!DoQuery_locked(err, query, querylen)) {
        free(query);
        return false;
    }
    free(query);

    last_insert_id = (uint32)mysql_insert_id(mysql);

    return true;
}

bool DBcore::DoQuery_locked(DBerror &err, const char *query, int querylen, bool retry/*true*/)
{
    double profileStartTime = GetTimeUSeconds();

    if (mysql == nullptr) {
        pStatus = Error;
        codelog(DATABASE__ERROR, "DBCore - mysql = null");
        if (!Reconnect())
            return false;
    }

    if (pStatus != Connected) {
        codelog(DATABASE__ERROR, "DBCore - Status != Connected");
        _log(DATABASE__MESSAGE, "DBCore error detected.  Look for error msgs in logs prior to this point.");
        if (!Reconnect())
            return false;
    }

    if (is_log_enabled(DATABASE__QUERIES))
        _log(DATABASE__QUERIES, "DBcore Query - %s", query);

    if (mysql_real_query(mysql, query, querylen)) {
        uint num = mysql_errno(mysql);
        if (num > 0)
            pStatus = Error;

        // there are many correctable errors to check for
        if ((num == CR_SERVER_LOST) or (num == CR_SERVER_GONE_ERROR)) {
            _log(DATABASE__ERROR, "DBCore error - server lost or gone.");
            if (!Reconnect())
                return false;
        }

        if ((pStatus == Connected) and retry)
            return DoQuery_locked(err, query, querylen, retry);

        err.SetError(num, mysql_error(mysql));
        codelog(DATABASE__ERROR, "DBCore Query - #%u in '%s': %s", err.GetErrNo(), query, err.c_str());
        return false;
    }

    err.ClearError();

    if (pProfile)
        sProfiler.AddTime(9, GetTimeUSeconds() - profileStartTime);

    return true;
}


int32 DBcore::DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen)
{
    return mysql_real_escape_string(mysql, tobuf, frombuf, fromlen);
}

void DBcore::DoEscapeString(std::string &to, const std::string &from)
{
    assert(mysql);
    uint32 len = (uint32)from.length();
    to.resize(len * 2);   // make enough room
    uint32 esc_len = mysql_real_escape_string(mysql, &to[0], from.c_str(), len);
    to.resize(esc_len + 1); // optional.
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

bool DBcore::IsSafeString(std::string &s) {
    for (uint i = 0; i < s.length(); ++i) {
        switch (s[i]) {
            case '\'':
            case '\\':
                return false;
        }
    }
    return true;
}


/* this doesnt work right....look into later...
void DBcore::ReplaceSlash(const char *str) {
    for(; *str != '\0'; str++) {
        switch(*str) {
            case '\'':
            case '\\':
                *str = '0';
        }
    }
} */

/************************************************************************/
/* DBerror                                                              */
/************************************************************************/
DBerror::DBerror()
{
    ClearError();
}

DBerror::~DBerror()
{
    mErrStr.clear();
}

void DBerror::SetError( uint err, const char* str )
{
    mErrStr = str;
    mErrNo = err;
}

void DBerror::ClearError()
{
    std::string errStr = "No Error";
    mErrStr = errStr;   // this gives mem leak.  not sure why yet...  test making string and clearing after set.
    mErrNo = 0;
    errStr.clear();
}

/************************************************************************/
/* DBQueryResult                                                        */
/************************************************************************/
/* mysql to DBTYPE convention table */
/* treating all strings as wide isn't probably the best solution but it's
 *   the easiest one which preserves wide strings. */
const DBTYPE DBQueryResult::MYSQL_DBTYPE_TABLE_SIGNED[] =
{
    DBTYPE_ERROR,   //[ 0]MYSQL_TYPE_DECIMAL            /* DECIMAL or NUMERIC field */
    DBTYPE_I1,      //[ 1]MYSQL_TYPE_TINY               /* TINYINT field */
    DBTYPE_I2,      //[ 2]MYSQL_TYPE_SHORT              /* SMALLINT field */
    DBTYPE_I4,      //[ 3]MYSQL_TYPE_LONG               /* INTEGER field */
    DBTYPE_R4,      //[ 4]MYSQL_TYPE_FLOAT              /* FLOAT field */
    DBTYPE_R8,      //[ 5]MYSQL_TYPE_DOUBLE             /* DOUBLE or REAL field */
    DBTYPE_ERROR,   //[ 6]MYSQL_TYPE_NULL               /* NULL-type field */
    DBTYPE_FILETIME,//[ 7]MYSQL_TYPE_TIMESTAMP          /* TIMESTAMP field */
    DBTYPE_I8,      //[ 8]MYSQL_TYPE_LONGLONG           /* BIGINT field */
    DBTYPE_I4,      //[ 9]MYSQL_TYPE_INT24              /* MEDIUMINT field */
    DBTYPE_ERROR,   //[10]MYSQL_TYPE_DATE               /* DATE field */
    DBTYPE_ERROR,   //[11]MYSQL_TYPE_TIME               /* TIME field */
    DBTYPE_ERROR,   //[12]MYSQL_TYPE_DATETIME           /* DATETIME field */
    DBTYPE_ERROR,   //[13]MYSQL_TYPE_YEAR               /* YEAR field */
    DBTYPE_ERROR,   //[14]MYSQL_TYPE_NEWDATE            /* ??? */
    DBTYPE_ERROR,   //[15]MYSQL_TYPE_VARCHAR            /* ??? */
    DBTYPE_BOOL,    //[16]MYSQL_TYPE_BIT                /* BIT field (MySQL 5.0.3 and up) */
    DBTYPE_R8,      //[17]MYSQL_TYPE_NEWDECIMAL=246     /* Precision math DECIMAL or NUMERIC field (MySQL 5.0.3 and up) */
    DBTYPE_ERROR,   //[18]MYSQL_TYPE_ENUM=247           /* ENUM field */
    DBTYPE_ERROR,   //[19]MYSQL_TYPE_SET=248            /* SET field */
    DBTYPE_WSTR,    //[20]MYSQL_TYPE_TINY_BLOB=249      /* TINYBLOB or TINYTEXT field */
    DBTYPE_WSTR,    //[21]MYSQL_TYPE_MEDIUM_BLOB=250    /* MEDIUMBLOB or MEDIUMTEXT field */
    DBTYPE_WSTR,    //[22]MYSQL_TYPE_LONG_BLOB=251      /* LONGBLOB or LONGTEXT field */
    DBTYPE_WSTR,    //[23]MYSQL_TYPE_BLOB=252           /* BLOB or TEXT field */
    DBTYPE_WSTR,    //[24]MYSQL_TYPE_VAR_STRING=253     /* VARCHAR or VARBINARY field */
    DBTYPE_STR,     //[25]MYSQL_TYPE_STRING=254         /* CHAR or BINARY field */
    DBTYPE_ERROR,   //[26]MYSQL_TYPE_GEOMETRY=255       /* Spatial field */
};

const DBTYPE DBQueryResult::MYSQL_DBTYPE_TABLE_UNSIGNED[] =
{
    DBTYPE_ERROR,   //[ 0]MYSQL_TYPE_DECIMAL            /* DECIMAL or NUMERIC field */
    DBTYPE_UI1,     //[ 1]MYSQL_TYPE_TINY               /* TINYINT field */
    DBTYPE_UI2,     //[ 2]MYSQL_TYPE_SHORT              /* SMALLINT field */
    DBTYPE_UI4,     //[ 3]MYSQL_TYPE_LONG               /* INTEGER field */
    DBTYPE_R4,      //[ 4]MYSQL_TYPE_FLOAT              /* FLOAT field */
    DBTYPE_R8,      //[ 5]MYSQL_TYPE_DOUBLE             /* DOUBLE or REAL field */
    DBTYPE_ERROR,   //[ 6]MYSQL_TYPE_NULL               /* NULL-type field */
    DBTYPE_FILETIME,//[ 7]MYSQL_TYPE_TIMESTAMP          /* TIMESTAMP field */
    DBTYPE_UI8,     //[ 8]MYSQL_TYPE_LONGLONG           /* BIGINT field */
    DBTYPE_UI4,     //[ 9]MYSQL_TYPE_INT24              /* MEDIUMINT field */
    DBTYPE_ERROR,   //[10]MYSQL_TYPE_DATE               /* DATE field */
    DBTYPE_ERROR,   //[11]MYSQL_TYPE_TIME               /* TIME field */
    DBTYPE_ERROR,   //[12]MYSQL_TYPE_DATETIME           /* DATETIME field */
    DBTYPE_ERROR,   //[13]MYSQL_TYPE_YEAR               /* YEAR field */
    DBTYPE_ERROR,   //[14]MYSQL_TYPE_NEWDATE            /* ??? */
    DBTYPE_ERROR,   //[15]MYSQL_TYPE_VARCHAR            /* ??? */
    DBTYPE_BOOL,    //[16]MYSQL_TYPE_BIT                /* BIT field (MySQL 5.0.3 and up) */
    DBTYPE_R8,      //[17]MYSQL_TYPE_NEWDECIMAL=246     /* Precision math DECIMAL or NUMERIC field (MySQL 5.0.3 and up) */
    DBTYPE_ERROR,   //[18]MYSQL_TYPE_ENUM=247           /* ENUM field */
    DBTYPE_ERROR,   //[19]MYSQL_TYPE_SET=248            /* SET field */
    DBTYPE_WSTR,    //[20]MYSQL_TYPE_TINY_BLOB=249      /* TINYBLOB or TINYTEXT field */
    DBTYPE_WSTR,    //[21]MYSQL_TYPE_MEDIUM_BLOB=250    /* MEDIUMBLOB or MEDIUMTEXT field */
    DBTYPE_WSTR,    //[22]MYSQL_TYPE_LONG_BLOB=251      /* LONGBLOB or LONGTEXT field */
    DBTYPE_WSTR,    //[23]MYSQL_TYPE_BLOB=252           /* BLOB or TEXT field */
    DBTYPE_WSTR,    //[24]MYSQL_TYPE_VAR_STRING=253     /* VARCHAR or VARBINARY field */
    DBTYPE_STR,     //[25]MYSQL_TYPE_STRING=254         /* CHAR or BINARY field */
    DBTYPE_ERROR,   //[26]MYSQL_TYPE_GEOMETRY=255       /* Spatial field */
};

DBQueryResult::DBQueryResult()
: mColumnCount(0),
mResult(nullptr),
mFields(nullptr)
{
}

DBQueryResult::~DBQueryResult()
{
    SafeDeleteArray( mFields );

    if (mResult != nullptr)
        mysql_free_result( mResult );
}

void DBQueryResult::Reset()
{
    mColumnCount = 0;
    SafeDeleteArray( mFields );

    if (mResult != nullptr)
        mysql_free_result(mResult);
}

bool DBQueryResult::IsUnsigned( uint32 index ) const
{
    return ((mFields[index]->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG);
}

bool DBQueryResult::IsBinary( uint32 index ) const
{
    // According to MySQL C API Documentation, binary string
    // fields like BLOB or VAR_BINARY have charset "63".
    return (mFields[index]->charsetnr == 63);
}

void DBQueryResult::SetResult( MYSQL_RES* res, uint32 colCount )
{
    Reset();

    mResult = res;
    mColumnCount = colCount;

    if (mResult != nullptr) {
        mFields = new MYSQL_FIELD*[ mColumnCount ];
        for( uint16 i = 0; i < mColumnCount; ++i )
            mFields[ i ] = mysql_fetch_field( mResult );
    }
}

bool DBQueryResult::GetRow( DBResultRow& into )
{
    if (mResult == nullptr)
        return false;

    MYSQL_ROW row = mysql_fetch_row( mResult );
    if (row == nullptr)
        return false;

    const ulong* lengths = mysql_fetch_lengths( mResult );
    if (lengths == nullptr)
        return false;

    into.SetData( this, row, lengths );
    return true;
}

const char* DBQueryResult::ColumnName( uint32 index ) const
{
    if (index >= mColumnCount) {
        _log(DATABASE__ERROR,  "DBCore ColumnName: Column index %u exceeds number of columns in row (%u)", index, mColumnCount );
        EvE::traceStack();
        return "(ERROR)";
    }

    return mFields[ index ]->name;
}

DBTYPE DBQueryResult::ColumnType( uint32 index ) const
{
    if (index >= mColumnCount) {
        _log(DATABASE__ERROR,  "DBCore ColumnType: Column index %u exceeds number of columns in row (%u)", index, mColumnCount );
        EvE::traceStack();
        return DBTYPE_STR;
    }

    uint16 columnType = mFields[ index ]->type;

    if ( columnType > MYSQL_TYPE_BIT )
        columnType -= 229;  //( MYSQL_TYPE_NEWDECIMAL - MYSQL_TYPE_BIT - 1 )

    DBTYPE result = ( IsUnsigned( index ) ? MYSQL_DBTYPE_TABLE_UNSIGNED : MYSQL_DBTYPE_TABLE_SIGNED )[ columnType ];

    // test for numeric type and set to signed 64b integer.  may have to revise this to float/double depending on what's found.
    if (result == DBTYPE_ERROR)
        if (IS_NUM(columnType))
            result = DBTYPE_I8;

    /* if result is (wide) binary string, set result to DBTYPE_BYTES. */
    if (((DBTYPE_STR == result) or (DBTYPE_WSTR == result)) and IsBinary(index))
        result = DBTYPE_BYTES;

    /* debug check */
    assert( result != DBTYPE_ERROR);
    return result;
}


DBResultRow::DBResultRow()
: mRow( nullptr ),
mLengths( nullptr ),
mResult( nullptr )
{
}

void DBResultRow::SetData( DBQueryResult* res, MYSQL_ROW& row, const ulong* lengths )
{
    mRow = row;
    mResult = res;
    mLengths = lengths;
}

uint32 DBResultRow::ColumnLength( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetColumnLength: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return 0;
    }

    return mLengths[ index ];
}

int32 DBResultRow::GetInt( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetInt: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return 0;
    }

    //use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
    return strtol( mRow[index], nullptr, 0 );
}

bool DBResultRow::GetBool( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetBool: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return false;
    }

    return (mRow[index][0] != 0);
}

uint32 DBResultRow::GetUInt( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetUInt: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return 0;
    }

    //use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
    return strtoul( mRow[index], nullptr, 0 );
}

int64 DBResultRow::GetInt64( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetInt64: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return 0;
    }

    //use base 0 on the obscure chance that this is a string column with an 0x hex number in it.
    return strtoll( mRow[index], nullptr, 0 );
}

float DBResultRow::GetFloat( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetFloat: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return 0.0f;
    }

    return strtof( mRow[index], nullptr );
}

double DBResultRow::GetDouble( uint32 index ) const
{
    if (index >= mResult->ColumnCount()) {
        _log(DATABASE__ERROR,  "   DBCore::GetDouble: Column index %u exceeds number of columns in row (%u)", index, mResult->ColumnCount() );
        EvE::traceStack();
        return 0.0;
    }

    return strtod( mRow[index], nullptr );
}
