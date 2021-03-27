/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://github.com/evemuproject/evemu_server
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

#ifndef __DATABASE__DBCORE_H__INCL__
#define __DATABASE__DBCORE_H__INCL__

//this whole file could be interface-ized to support a different database
//if you can get over the SQL incompatibilities and mysql auto increment problems.

#include "memory/SafeMem.h"
#include "utils/Singleton.h"
#include "database/dbtype.h"
#include "threading/Mutex.h"

class DBcore;

class DBerror
{
public:
    DBerror();
    ~DBerror();

    uint32 GetErrNo() const { return mErrNo; }
    const char* GetError() const { return mErrStr.c_str(); }

    const char* c_str() const { return GetError(); }

protected:
    //for DBcore:
    friend class DBcore;
    void SetError( uint err, const char* str );
    void ClearError();

    std::string mErrStr;
    uint mErrNo;
};


class DBResultRow;
class DBQueryResult
{
public:
    DBQueryResult();
    ~DBQueryResult();

    /* error during the query, if RunQuery returned false. */
    DBerror error;

    bool GetRow( DBResultRow& into );
    size_t GetRowCount() { return (size_t)mResult->row_count; }
    // this should be called between multiple calls using same DBQueryResult object
    void Reset();

    uint32 ColumnCount() const { return mColumnCount; }
    const char* ColumnName( uint32 index ) const;
    DBTYPE ColumnType( uint32 index ) const;

    bool IsUnsigned( uint32 index ) const;
    bool IsBinary( uint32 index ) const;

protected:
    //for DBcore:
    friend class DBcore;
    void SetResult( MYSQL_RES* res, uint32 colCount );

    uint32 mColumnCount;
    MYSQL_RES* mResult;
    MYSQL_FIELD** mFields;

    static const DBTYPE MYSQL_DBTYPE_TABLE_SIGNED[];
    static const DBTYPE MYSQL_DBTYPE_TABLE_UNSIGNED[];
};

class DBResultRow
{
public:
    DBResultRow();
    ~DBResultRow() { /* do nothing here */ }

    bool IsNull( uint32 index ) const { return ( NULL == GetText( index ) ); }

    const char* GetText( uint32 index ) const { return mRow[ index ]; }
    int32 GetInt( uint32 index ) const;
    bool GetBool( uint32 index ) const;
    uint32 GetUInt( uint32 index ) const;
    int64 GetInt64( uint32 index ) const;
    float GetFloat( uint32 index ) const;
    double GetDouble( uint32 index ) const;

    //proxy methods up to our query result:
    uint32 ColumnCount() const { return mResult->ColumnCount(); }
    const char* ColumnName( uint32 index ) const { return mResult->ColumnName( index ); }
    DBTYPE ColumnType( uint32 index ) const { return mResult->ColumnType( index ); }
    uint32 ColumnLength( uint32 index ) const;

    bool IsUnsigned( uint32 index ) const { return mResult->IsUnsigned( index ); }
    bool IsBinary( uint32 index ) const { return mResult->IsBinary( index ); }

protected:
    //for DBQueryResult
    friend class DBQueryResult;
    void SetData( DBQueryResult* res, MYSQL_ROW& row, const ulong* lengths );

    MYSQL_ROW mRow;
    const ulong* mLengths;

    DBQueryResult* mResult;
};

class DBcore
: public Singleton<DBcore>
{
public:
    enum eStatus { Closed, Connected, Error };

    DBcore();
    ~DBcore() { /* do nothing here */ }

    void    Close();
    void    Initialize(std::string host, std::string user, std::string password, std::string database, bool compress=false, bool SSL=false,
                       int16 port=3306, bool socket=false, bool reconnect=false, bool profile=false);

    //new shorter syntax:
    //query which returns a result (error is stored in the result if it occurs)
    // NOTE:  result is cleared before populating with most recent data for multiple statements using same DBQueryResult object.
    bool    RunQuery(DBQueryResult &into, const char *query_fmt, ...);
    //query which returns no information except error status
    // NOTE:  result is cleared before populating with most recent data for multiple statements using same DBQueryResult object.
    bool    RunQuery(DBerror &err, const char *query_fmt, ...);
    //query which returns affected rows:
    // NOTE:  result is cleared before populating with most recent data for multiple statements using same DBQueryResult object.
    bool    RunQuery(DBerror &err, uint32 &affected_rows, const char *query_fmt, ...);
    //query which returns last insert ID:
    // NOTE:  result is cleared before populating with most recent data for multiple statements using same DBQueryResult object.
    bool    RunQueryLID(DBerror& err, uint32& last_insert_id, const char* query_fmt, ...);

    int32   DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen);
    void    DoEscapeString(std::string &to, const std::string &from);
    static bool IsSafeString(const char *str);
    static bool IsSafeString(std::string &s);
    // check for and remove slashes before sending string to db
    //static void ReplaceSlash(const char *str);
    void    ping();

    eStatus GetStatus() const { return pStatus; }

protected:
    MYSQL*  getMySQL()              { return mysql; }

    void Connect(uint* errnum = 0, char* errbuf = 0);

    bool Reconnect();
    //void CallShutdown();

private:
    //MDatabase must be locked before these calls:
    bool    DoQuery_locked(DBerror &err, const char *query, int querylen, bool retry = true);

    MYSQL*  mysql;
    Mutex   MDatabase;
    eStatus pStatus;

    bool    pCompress;
    bool    pProfile;
    bool    pReconnect;
    bool    pSocket;
    bool    pSSL;

    int16   pPort;

    std::string pHost;
    std::string pUser;
    std::string pPassword;
    std::string pDatabase;
};

#define sDatabase \
( DBcore::get() )

#endif /* !__DATABASE__DBCORE_H__INCL__ */
