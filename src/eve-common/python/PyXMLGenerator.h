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

#ifndef __PYXMLGENERATOR_H_INCL__
#define __PYXMLGENERATOR_H_INCL__

#include "python/PyVisitor.h"

class PyXMLGenerator
: public PyPfxVisitor
{
public:
    PyXMLGenerator( FILE* into, const char* pfx = "" );

protected:
    //! primitive data visitors
    bool VisitInteger( const PyInt* rep );
    bool VisitLong( const PyLong* rep );
    bool VisitReal( const PyFloat* rep );
    bool VisitBoolean( const PyBool* rep );
    bool VisitNone( const PyNone* rep );
    bool VisitBuffer( const PyBuffer* rep );
    bool VisitString( const PyString* rep );

    //! PackedRow type visitor
    bool VisitPackedRow( const PyPackedRow* rep );

    //! Object type visitor
    bool VisitObject( const PyObject* rep );
    bool VisitObjectEx( const PyObjectEx* rep );

    bool VisitSubStruct( const PySubStruct* rep );
    bool VisitSubStream( const PySubStream* rep );
    bool VisitChecksumedStream( const PyChecksumedStream* rep );

    bool VisitDict( const PyDict* rep );
    bool VisitList( const PyList* rep );
    bool VisitTuple( const PyTuple* rep );

private:
    FILE* const mInto;
    uint32 mItem;
};

#endif
