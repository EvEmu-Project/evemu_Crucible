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
    Author:     Zhur, Bloody.Rabbit
*/

#ifndef __ROWSETTOSQL_H_INCL__
#define __ROWSETTOSQL_H_INCL__

#include "python/PyRep.h"

/** The largest amount of rows to be inserted by single query. */
extern const size_t INSERT_QUERY_ROW_LIMIT;

/**
 * @brief Helper class for column description.
 *
 * Describes single column.
 *
 * @author Zhur, Bloody.Rabbit
 */
class ReaderColumnContentDesc
{
public:
    /** Known column types. */
    enum ColumnType
    {
        TYPE_INTEGER,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_WSTRING,

        TYPE_UNKNOWN,
        TYPE_ERROR
    };

    /** Primary constructor. */
    ReaderColumnContentDesc();

    /** @return Type of column. */
    ColumnType type() const { return mType; }
    /** @return Whether column may be NULL. */
    bool isNull() const { return mIsNull; }

    /** @return Type of column, in SQL. */
    const char* typeString() const { return smTypeStrings[ type() ]; }
    /** @return Whether column may be NULL, in SQL. */
    const char* nullString() const { return smNullStrings[ isNull() ]; }

    /**
     * @brief Processes PyRep type to determine type of column.
     *
     * @param[in] t PyRep type.
     */
    void Process( const PyRep::PyType t );

protected:
    /** Type of column. */
    ColumnType mType;
    /** Whether column may be NULL. */
    bool mIsNull;

    /** Known types of column, in SQL. */
    static const char* const smTypeStrings[];
    /** Whether column may be NULL, in SQL. */
    static const char* const smNullStrings[];
};

/**
 * @brief Classifies columns using ReaderColumnContentDesc.
 *
 * @param[out] into   Array of ReaderColumnContentDesc objects, which receive column descriptions.
 * @param[in]  reader Rowset reader to use.
 */
template<typename _Reader>
void ClassifyColumnTypes( std::vector<ReaderColumnContentDesc>& into, _Reader& reader )
{
    const size_t cc = reader.columnCount();

    into.clear();
    into.resize( cc );

    typename _Reader::iterator cur, end;
    cur = reader.begin();
    end = reader.end();
    for(; cur != end; ++cur)
        for( size_t col = 0; col < cc; ++col )
            into[col].Process( cur.GetType( col ) );
}

/**
 * @brief Dumps rowset to SQL.
 *
 * @param[in]  table_name Name of table to be created.
 * @param[in]  key_field  Key field of table.
 * @param[out] out        Output file.
 * @param[in]  reader     Rowset reader to use.
 */
template<typename _Reader>
bool ReaderToSQL( const char* table_name, const char* key_field, FILE* out, _Reader& reader )
{
    const size_t cc = reader.columnCount();

    const size_t key_col = reader.FindColumn( key_field );
    if( cc == key_col )
    {
        sLog.Error( "ReaderToSQL", "Unable to find key column '%s'.", key_field );
        return false;
    }

    std::vector<ReaderColumnContentDesc> types;
    ClassifyColumnTypes<_Reader>( types, reader );

    fprintf( out,
             "--- %s schema ---\n"
             "\n"
             "CREATE TABLE `%s` (\n",
             table_name,
             table_name
    );

    std::string field_list;
    for( size_t col = 0; col < cc; ++col )
    {
        const char* colName = reader.columnName( col );
        const ReaderColumnContentDesc& colDesc = types[col];

        fprintf( out,
                 "%s  `%s` %s %s%s",
                 ( 0 != col ? ",\n" : "" ),
                 colName,
                 colDesc.typeString(),
                 colDesc.nullString(),
                 ( key_col == col ? " PRIMARY KEY" : "" )
        );

        if( 0 != col  )
            field_list += ",";
        field_list += "`";
        field_list += colName;
        field_list += "`";
    }

    fprintf( out,
             "%s\n"
             ");\n"
             "\n"
             "--- end %s schema ---\n"
             "\n"
             "--- %s data ---\n"
             "\n",
             ( NULL == key_field ? ",\n  PRIMARY KEY()" : "" ),
             table_name,
             table_name
    );

    typename _Reader::iterator cur, end;
    cur = reader.begin();
    end = reader.end();
    for( size_t rowIndex = 0; cur != end; ++cur, ++rowIndex )
    {
        if( 0 == ( rowIndex % INSERT_QUERY_ROW_LIMIT ) )
        {
            if( 0 != rowIndex )
                fprintf( out, ";\n" );

            fprintf( out, "INSERT INTO `%s`(%s) VALUES ", table_name, field_list.c_str() );
        }
        else
            fprintf( out, "," );

        fprintf( out, "(" );
        for( size_t col = 0; col < cc; ++col )
        {
            if( col != 0 )
                fprintf( out, "," );

            fprintf( out, "%s", cur.GetAsString( col ).c_str() );
        }
        fprintf( out, ")" );
    }

    fprintf( out,
             ";\n"
             "\n"
             "--- end %s data ---\n",
             table_name
    );

    return true;
}

#endif



