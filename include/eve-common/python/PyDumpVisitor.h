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
    Author:     Zhur, mmcs
*/

#ifndef __PYDUMPVISITOR_H_INCL__
#define __PYDUMPVISITOR_H_INCL__

#include "python/PyVisitor.h"

class PyDumpVisitor : public PyPfxVisitor
{
public:
    PyDumpVisitor( const char* pfx = "", bool full_nested = false );

    bool fullNested() const { return mFullNested; }

protected:
    // Output functions
    virtual void _print( const char* fmt, ... ) = 0;
    virtual void _dump( const char* pfx, const uint8* data, size_t len ) = 0;

    //! primitive data visitors
    bool VisitInteger( const PyInt* rep );
    bool VisitLong( const PyLong* rep );
    bool VisitReal( const PyFloat* rep );
    bool VisitBoolean( const PyBool* rep );
    bool VisitNone( const PyNone* rep );
    bool VisitBuffer( const PyBuffer* rep );
    bool VisitString( const PyString* rep );
    bool VisitWString( const PyWString* rep );
    bool VisitToken( const PyToken* rep );

    //! the nested types Visitor
    bool VisitTuple( const PyTuple* rep );
    bool VisitList( const PyList* rep );
    bool VisitDict( const PyDict* rep );

    //! Object type visitor
    bool VisitObject( const PyObject* rep );
    bool VisitObjectEx( const PyObjectEx* rep );
    //! PackedRow type visitor
    bool VisitPackedRow( const PyPackedRow* rep );
    //! wrapper types Visitor
    bool VisitSubStruct( const PySubStruct* rep );
    bool VisitSubStream( const PySubStream* rep );
    bool VisitChecksumedStream( const PyChecksumedStream* rep );

private:
    const bool mFullNested;
};

class PyLogDumpVisitor : public PyDumpVisitor
{
public:
    PyLogDumpVisitor( LogType log_type, LogType log_hex_type, const char* pfx = "", bool full_nested = false, bool full_hex = false );

    bool fullHex() const { return mFullHex; }

    LogType logType() const { return mLogType; }
    LogType logHexType() const { return mLogHexType; }

protected:
    void _print( const char* fmt, ... );
    void _dump( const char* pfx, const uint8* data, size_t len );

private:
    const bool mFullHex;

    const LogType mLogType;
    const LogType mLogHexType;
};

class PyFileDumpVisitor : public PyDumpVisitor
{
public:
    PyFileDumpVisitor( FILE* _file, const char* pfx = "", bool full_nested = false, bool full_hex = false );

    bool fullHex() const { return mFullHex; }

    FILE* file() const { return mFile; }

protected:
    void _print( const char* fmt, ... );
    void _dump( const char* pfx, const uint8* data, size_t len );

private:
    const bool mFullHex;

    FILE* const mFile;
};

#endif



