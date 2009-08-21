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
    Author:     Zhur, mmcs
*/

#ifndef __PYXMLGENERATOR_H_INCL__
#define __PYXMLGENERATOR_H_INCL__

#include "PyVisitor.h"
#include <stdio.h>
#include <string>
#include <stack>

class PyXMLGenerator
: public PyVisitor {
public:
    PyXMLGenerator(FILE *into);
    virtual ~PyXMLGenerator();

protected:
    FILE *const m_into;
    int m_item;

    std::stack<const char *> m_indentStack;

    inline const char *top() const { return(m_indentStack.top()); }
    inline void pop() { m_indentStack.pop(); }
    inline void push(const char *v) { m_indentStack.push(v); }

    //! primitive data visitors
    void VisitInteger(const PyInt *rep);
    void VisitReal(const PyFloat *rep);
    void VisitBoolean(const PyBool *rep);
    void VisitNone(const PyNone *rep);
    void VisitBuffer(const PyBuffer *rep);
    void VisitString(const PyString *rep);
    //! PackedRow type visitor
    void VisitPackedRow(const PyPackedRow *rep);
    //! Object type visitor
    void VisitObject(const PyObject *rep);
    void VisitObjectEx(const PyObjectEx *rep);

    void VisitSubStruct(const PySubStruct *rep);
    void VisitSubStream(const PySubStream *rep);
    void VisitChecksumedStream(const PyChecksumedStream *rep);

    void VisitDict(const PyDict *rep);
    void VisitList(const PyList *rep);
    void VisitTuple(const PyTuple *rep);

};

#endif
