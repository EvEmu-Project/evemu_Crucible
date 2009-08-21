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


#ifndef PY_VISITOR_H
#define PY_VISITOR_H

#include "./types.h"

class PyRep;
class PyInt;
class PyFloat;
class PyBool;
class PyNone;
class PyBuffer;
class PyString;
class PyObject;
class PyObjectEx;
class PySubStruct;
class PySubStream;
class PyChecksumedStream;
class PyDict;
class PyList;
class PyTuple;
class PyPackedRow;

class PyVisitorLvl {
public:
    virtual ~PyVisitorLvl() {}

    //! primitive data visitors
    virtual void VisitInteger(const PyInt *rep, int64 lvl ) = 0;
    virtual void VisitReal(const PyFloat *rep, int64 lvl ) = 0;
    virtual void VisitBoolean(const PyBool *rep, int64 lvl ) = 0;
    virtual void VisitNone(const PyNone *rep, int64 lvl ) = 0;
    virtual void VisitBuffer(const PyBuffer *rep, int64 lvl ) = 0;
    virtual void VisitString(const PyString *rep, int64 lvl ) = 0;
    //! PackedRow type visitor
    virtual void VisitPackedRow(const PyPackedRow *rep, int64 lvl ) = 0;
    //! Object type visitor
    virtual void VisitObject(const PyObject *rep, int64 lvl ) = 0;
    virtual void VisitObjectEx(const PyObjectEx *rep, int64 lvl ) = 0;
    //! Structureated types Visitor
    virtual void VisitSubStruct(const PySubStruct *rep, int64 lvl ) = 0;
    virtual void VisitSubStream(const PySubStream *rep, int64 lvl ) = 0;
    virtual void VisitChecksumedStream(const PyChecksumedStream *rep, int64 lvl ) = 0;
    //! the data types Visitor
    virtual void VisitDict(const PyDict *rep, int64 lvl ) = 0;
    virtual void VisitList(const PyList *rep, int64 lvl ) = 0;
    virtual void VisitTuple(const PyTuple *rep, int64 lvl ) = 0;
};

class PyVisitor {
public:
    virtual ~PyVisitor() {}

    //! primitive data visitors
    virtual void VisitInteger(const PyInt *rep) = 0;
    virtual void VisitReal(const PyFloat *rep) = 0;
    virtual void VisitBoolean(const PyBool *rep) = 0;
    virtual void VisitNone(const PyNone *rep) = 0;
    virtual void VisitBuffer(const PyBuffer *rep) = 0;
    virtual void VisitString(const PyString *rep) = 0;
    //! PackedRow type visitor
    virtual void VisitPackedRow(const PyPackedRow *rep) = 0;
    //! Object type visitor
    virtual void VisitObject(const PyObject *rep) = 0;
    virtual void VisitObjectEx(const PyObjectEx *rep) = 0;
    //! Structureated types Visitor
    virtual void VisitSubStruct(const PySubStruct *rep) = 0;
    virtual void VisitSubStream(const PySubStream *rep) = 0;
    virtual void VisitChecksumedStream(const PyChecksumedStream *rep) = 0;
    //! the data types Visitor
    virtual void VisitDict(const PyDict *rep) = 0;
    virtual void VisitList(const PyList *rep) = 0;
    virtual void VisitTuple(const PyTuple *rep) = 0;
};


class SubStreamDecoder : public PyVisitor {
public:
    virtual ~SubStreamDecoder() {}

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

    //! the visitor needed for decoding the substream.
    void VisitSubStream(const PySubStream *rep);

    void VisitChecksumedStream(const PyChecksumedStream *rep);
    //! the data types Visitor
    void VisitDict(const PyDict *rep);
    void VisitList(const PyList *rep);
    void VisitTuple(const PyTuple *rep);
};


#endif



