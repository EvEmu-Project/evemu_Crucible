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
    Author:     Zhur, mmcs, Bloody.Rabbit
*/

#ifndef __PY_VISITOR_H__INCL__
#define __PY_VISITOR_H__INCL__

class PyRep;
class PyInt;
class PyLong;
class PyFloat;
class PyBool;
class PyNone;
class PyBuffer;
class PyString;
class PyWString;
class PyToken;
class PyObject;
class PyObjectEx;
class PySubStruct;
class PySubStream;
class PyChecksumedStream;
class PyDict;
class PyList;
class PyTuple;
class PyPackedRow;

class PyVisitor
{
public:
    virtual ~PyVisitor() {}

    //! primitive data visitors
    virtual bool VisitInteger( const PyInt* rep ) { return true; }
    virtual bool VisitLong( const PyLong* rep ) { return true; }
    virtual bool VisitReal( const PyFloat* rep ) { return true; }
    virtual bool VisitBoolean( const PyBool* rep ) { return true; }
    virtual bool VisitNone( const PyNone* rep ) { return true; }
    virtual bool VisitBuffer( const PyBuffer* rep ) { return true; }
    virtual bool VisitString( const PyString* rep ) { return true; }
    virtual bool VisitWString( const PyWString* rep ) { return true; }
    virtual bool VisitToken( const PyToken* rep ) { return true; }

    //! the nested types Visitor
    virtual bool VisitTuple( const PyTuple* rep );
    virtual bool VisitList( const PyList* rep );
    virtual bool VisitDict( const PyDict* rep );

    //! Object type visitor
    virtual bool VisitObject( const PyObject* rep );
    virtual bool VisitObjectEx( const PyObjectEx* rep );

    //! PackedRow type visitor
    virtual bool VisitPackedRow( const PyPackedRow* rep );

    //! wrapper types Visitor
    virtual bool VisitSubStruct( const PySubStruct* rep );
    virtual bool VisitSubStream( const PySubStream* rep );
    virtual bool VisitChecksumedStream( const PyChecksumedStream* rep );
};

class PyPfxVisitor : public PyVisitor
{
public:
    PyPfxVisitor( const char* pfx = "" );

protected:
    const char* _pfx() const { return mPfxStack.top().c_str(); }
    void _pfxExtend( const char* fmt, ... );
    void _pfxWithdraw() { mPfxStack.pop(); }

    std::stack<std::string> mPfxStack;
};

#endif /* !__PY_VISITOR_H__INCL__ */
