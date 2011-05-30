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
    Author:     Zhur
*/

#include "EVECommonPCH.h"

#include "database/RowsetReader.h"
#include "database/RowsetToSQL.h"
#include "python/PyRep.h"

/************************************************************************/
/* BaseRowsetReader                                                     */
/************************************************************************/
size_t BaseRowsetReader::FindColumn( const char* name )
{
    const uint32 cc = columnCount();

    for( uint32 i = 0; i < cc; ++i )
    {
        if( 0 == strcmp( name, columnName( i ) ) )
            return i;
    }

    return cc;
}

/************************************************************************/
/* BaseRowsetReader::iterator                                           */
/************************************************************************/
BaseRowsetReader::iterator::iterator()
: mRowIndex( -1 )
{
}

std::string BaseRowsetReader::iterator::GetAsString( size_t index ) const
{
    const PyRep::PyType t = GetType( index );

    switch( t )
    {
    case PyRep::PyTypeNone:
        return "NULL";
    case PyRep::PyTypeBool:
        return itoa( GetBool( index ) ? 1 : 0 );
    case PyRep::PyTypeInt:
        return itoa( GetInt( index ) );
    case PyRep::PyTypeLong:
        return itoa( GetLong( index ) );
    case PyRep::PyTypeFloat:
        {
            char buf[64];
            snprintf( buf, 64, "%f", GetFloat( index ) );
            return buf;
        }
    case PyRep::PyTypeString:
        {
            std::string str = GetString( index );
            SearchReplace( str, "'", "\\'" );

            str.insert( str.begin(), '\'' );
            str.insert( str.end(),   '\'' );

            return str;
        }
    case PyRep::PyTypeWString:
        {
            std::string str = GetWString( index );
            SearchReplace( str, "'", "\\'" );

            str.insert( str.begin(), '\'' );
            str.insert( str.end(),   '\'' );

            return str;
        }
    default:
        {
            char buf[64];
            snprintf( buf, 64, "'UNKNOWN TYPE %u'", t );
            return buf;
        }
    }
}

const BaseRowsetReader::iterator& BaseRowsetReader::iterator::operator++()
{
    if( _baseReader()->rowCount() > _rowIndex() )
        _SetRow( _rowIndex() + 1 );

    return *this;
}

const BaseRowsetReader::iterator& BaseRowsetReader::iterator::operator--()
{
    if( 0 < _rowIndex() )
        _SetRow( _rowIndex() - 1 );

    return *this;
}

bool BaseRowsetReader::iterator::operator==( const iterator& other ) const
{
    if( _baseReader() != other._baseReader() )
        return false;
    else if( _rowIndex() != other._rowIndex() )
        return false;
    return true;
}

/************************************************************************/
/* PyRowsetReader::iterator                                             */
/************************************************************************/
bool PyRowsetReader::iterator::IsNone( size_t index ) const
{
    if( NULL == GetRep( index ) )
        return true;
    return BaseRowsetReader::iterator::IsNone( index );
}

bool PyRowsetReader::iterator::GetBool( size_t index ) const
{
    return GetRep( index )->AsBool()->value();
}

uint32 PyRowsetReader::iterator::GetInt( size_t index ) const
{
    return GetRep( index )->AsInt()->value();
}

uint64 PyRowsetReader::iterator::GetLong( size_t index ) const
{
    return GetRep( index )->AsLong()->value();
}

double PyRowsetReader::iterator::GetFloat( size_t index ) const
{
    return GetRep( index )->AsFloat()->value();
}

const char* PyRowsetReader::iterator::GetString( size_t index ) const
{
    return GetRep( index )->AsString()->content().c_str();
}

const char* PyRowsetReader::iterator::GetWString( size_t index ) const
{
    return GetRep( index )->AsWString()->content().c_str();
}

/************************************************************************/
/* RowsetReader                                                         */
/************************************************************************/
RowsetReader::RowsetReader( const util_Rowset& rowset )
: mSet( rowset )
{
}

RowsetReader::~RowsetReader()
{
}

/************************************************************************/
/* RowsetReader::iterator                                               */
/************************************************************************/
RowsetReader::iterator::iterator()
: mParent( NULL ),
  mRow( NULL )
{
}

RowsetReader::iterator::iterator( RowsetReader* parent, size_t rowIndex )
: mParent( parent ),
  mRow( NULL )
{
    _SetRow( rowIndex );
}

void RowsetReader::iterator::_SetRow( size_t rowIndex )
{
    if( _rowIndex() != rowIndex )
    {
        mRow = ( mParent->rowCount() > rowIndex
                 ? mParent->_GetRow( rowIndex )
                 : NULL );
    }

    PyRowsetReader::iterator::_SetRow( rowIndex );
}

/************************************************************************/
/* TuplesetReader                                                       */
/************************************************************************/
TuplesetReader::TuplesetReader( const util_Tupleset& tupleset )
: mSet( tupleset )
{
}

/************************************************************************/
/* TuplesetReader::iterator                                             */
/************************************************************************/
TuplesetReader::iterator::iterator()
: mParent( NULL )
{
}

TuplesetReader::iterator::iterator( TuplesetReader* parent, size_t rowIndex )
: mParent( parent )
{
    _SetRow( rowIndex );
}

void TuplesetReader::iterator::_SetRow( size_t rowIndex )
{
    if( _rowIndex() != rowIndex )
    {
        mRow = ( mParent->rowCount() > rowIndex
                 ? mParent->_GetRow( rowIndex )
                 : NULL );
    }

    PyRowsetReader::iterator::_SetRow( rowIndex );
}

/************************************************************************/
/* SetSQLDumper                                                         */
/************************************************************************/
SetSQLDumper::SetSQLDumper( const char* table, const char* keyField, FILE* out )
: mTable( table ),
  mKeyField( keyField ),
  mOut( out )
{
}

bool SetSQLDumper::VisitTuple( const PyTuple* rep )
{
    //first we want to check to see if this could possibly even be a tupleset.
    if(    2 == rep->size()
        && rep->GetItem( 0 )->IsList()
        && rep->GetItem( 1 )->IsList() )
    {
        const PyList* possible_header = rep->GetItem( 0 )->AsList();
        const PyList* possible_items = rep->GetItem( 1 )->AsList();

        //check each element of the lists to make sure they line up.
        bool valid = true;
        PyList::const_iterator cur, end;

        cur = possible_header->begin();
        end = possible_header->end();
        for(; valid && cur != end; ++cur)
        {
            if( !(*cur)->IsString() )
                valid = false;
        }

        cur = possible_items->begin();
        end = possible_items->end();
        for(; valid && cur != end; ++cur)
        {
            if( !(*cur)->IsList() )
                valid = false;

            //it would be possible I guess to check each element of each item to make sure
            //it is a terminal type (non-container), but I dont care right now.
        }

        if( valid )
        {
            //ok, it looks like a tupleset... nothing we can do now but interpret it as one...
            util_Tupleset rowset;

            //must be duplicated in order to be decoded ...
            PyTuple* dup = new PyTuple( *rep );
            if( !rowset.Decode( &dup ) )
                sLog.Error( "SetSQLDumper", "Unable to interpret tuple as a tupleset, it may not even be one." );
            else
            {
                TuplesetReader reader( rowset );
                if( ReaderToSQL<TuplesetReader>( mTable.c_str(), mKeyField.c_str(), mOut, reader ) )
                    return true;

                sLog.Error( "SetSQLDumper", "Failed to convert tupleset to SQL." );
            }
        }
    }

    //fallback
    return PyVisitor::VisitTuple( rep );
}

bool SetSQLDumper::VisitObject( const PyObject* rep )
{
    if( rep->type()->content() == "util.Rowset" )
    {
        //we found a friend, decode it
        util_Rowset rowset;

        //must be duplicated in order to be decoded ...
        PyObject* dup = new PyObject( *rep );
        if( !rowset.Decode( &dup ) )
            sLog.Error( "SetSQLDumper", "Unable to load a rowset from the object body!" );
        else
        {
            RowsetReader reader( rowset );
            if( ReaderToSQL<RowsetReader>( mTable.c_str(), mKeyField.c_str(), mOut, reader ) )
                return true;

            sLog.Error( "SetSQLDumper", "Failed to convert rowset to SQL." );
        }
    }

    //fallback
    return PyVisitor::VisitObject( rep );
}
