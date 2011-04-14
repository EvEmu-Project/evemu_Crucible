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

#ifndef __ROWSETREADER_H_INCL__
#define __ROWSETREADER_H_INCL__

#include "packets/General.h"
#include "python/PyVisitor.h"

class BaseRowsetReader
{
public:
    virtual ~BaseRowsetReader() {}

    /* Required set of operations:
    iterator begin();
    iterator end();
    */

    virtual size_t columnCount() const = 0;
    virtual const char* columnName( size_t index ) const = 0;
    virtual size_t FindColumn( const char* name );

    virtual size_t rowCount() const = 0;

protected:
    class iterator
    {
    public:
        virtual PyRep::PyType GetType( size_t index ) const = 0;
        virtual bool IsNone( size_t index ) const { return ( PyRep::PyTypeNone == GetType( index ) ); }

        virtual bool GetBool( size_t index ) const = 0;
        virtual uint32 GetInt( size_t index ) const = 0;
        virtual uint64 GetLong( size_t index ) const = 0;
        virtual double GetFloat( size_t index ) const = 0;
        virtual const char* GetString( size_t index ) const = 0;
        virtual const char* GetWString( size_t index ) const = 0;

        //convert whatever we have into a string
        std::string GetAsString( size_t index ) const;

        const iterator& operator++();
        const iterator& operator++(int) { return ++*this; }
        const iterator& operator--();
        const iterator& operator--(int) { return --*this; }

        bool operator==( const iterator& oth ) const;
        bool operator!=( const iterator& oth ) const { return !( *this == oth ); }

    protected:
        iterator();

        virtual size_t _rowIndex() const { return mRowIndex; }
        virtual BaseRowsetReader* _baseReader() const = 0;

        virtual void _SetRow( size_t rowIndex ) { mRowIndex = rowIndex; }

        size_t mRowIndex;
    };
};

class PyRowsetReader : public BaseRowsetReader
{
protected:
    class iterator : public BaseRowsetReader::iterator
    {
    public:
        virtual PyRep* GetRep( size_t index ) const = 0;

        PyRep::PyType GetType( size_t index ) const { return GetRep( index )->GetType(); }
        bool IsNone( size_t index ) const;

        bool GetBool( size_t index ) const;
        uint32 GetInt( size_t index ) const;
        uint64 GetLong( size_t index ) const;
        double GetFloat( size_t index ) const;
        const char* GetString( size_t index ) const;
        const char* GetWString( size_t index ) const;
    };
};

class RowsetReader : public PyRowsetReader
{
public:
    RowsetReader( const util_Rowset& rowset );
    ~RowsetReader();

    class iterator : public PyRowsetReader::iterator
    {
        friend class RowsetReader;
    public:
        iterator();

        PyRep* GetRep( size_t index ) const { return mRow->GetItem( index ); }

    protected:
        iterator( RowsetReader* parent, size_t rowIndex );

        BaseRowsetReader* _baseReader() const { return mParent; }

        void _SetRow( size_t rowIndex );

        RowsetReader* mParent;
        PyList* mRow;
    };

    size_t columnCount() const { return mSet.header.size(); }
    const char* columnName( size_t index ) const { return mSet.header[ index ].c_str(); }

    size_t rowCount() const { return mSet.lines->size(); }

    iterator begin() { return iterator( this, 0 ); }
    iterator end() { return iterator( this, rowCount() ); }

protected:
    PyList* _GetRow( size_t index ) const { return mSet.lines->GetItem( index )->AsList(); }

    const util_Rowset& mSet;
};

class TuplesetReader : public PyRowsetReader
{
public:
    TuplesetReader( const util_Tupleset& tupleset );

    class iterator : public PyRowsetReader::iterator
    {
        friend class TuplesetReader;
    public:
        iterator();

        PyRep* GetRep( size_t index ) const { return mRow->GetItem( index ); }

    protected:
        iterator( TuplesetReader* parent, size_t rowIndex );

        BaseRowsetReader* _baseReader() const { return mParent; }

        void _SetRow( size_t rowIndex );

        TuplesetReader* mParent;
        PyList* mRow;
    };

    size_t columnCount() const { return mSet.header.size(); }
    const char* columnName( size_t index ) const { return mSet.header[ index ].c_str(); }

    size_t rowCount() const { return mSet.lines->size(); }

    iterator begin() { return iterator( this, 0 ); }
    iterator end() { return iterator( this, rowCount() ); }

protected:
    PyList* _GetRow( size_t index ) const { return mSet.lines->GetItem( index )->AsList(); }

    const util_Tupleset& mSet;       //we do NOT own this pointer
};

/**
 * @brief Python object SQL dumper.
 *
 * Scans visited PyRep for any dumpable rowsets or
 * tuplesets; if any is found, it's dumped using
 * RowsetToSQL.
 *
 * @author Zhur, Bloody.Rabbit
 */
class SetSQLDumper 
: public PyVisitor
{
public:
    SetSQLDumper( const char* table, const char* keyField, FILE* out );

    bool VisitTuple( const PyTuple* rep );

    bool VisitObject( const PyObject* rep );

protected:
    const std::string mTable;
    const std::string mKeyField;
    FILE* const mOut;
};

#endif


