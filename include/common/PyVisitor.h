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
class PyRepInteger;
class PyRepReal;
class PyRepBoolean;
class PyRepNone;
class PyRepBuffer;
class PyRepString;
class PyRepObject;
class PyRepObjectEx;
class PyRepSubStruct;
class PyRepSubStream;
class PyRepChecksumedStream;
class PyRepDict;
class PyRepList;
class PyRepTuple;
class PyRepPackedRow;

class PyVisitorLvl {
public:
    virtual ~PyVisitorLvl() {}

    //! primitive data visitors
    virtual void VisitInteger(const PyRepInteger *rep, int64 lvl ) = 0;
    virtual void VisitReal(const PyRepReal *rep, int64 lvl ) = 0;
    virtual void VisitBoolean(const PyRepBoolean *rep, int64 lvl ) = 0;
    virtual void VisitNone(const PyRepNone *rep, int64 lvl ) = 0;
    virtual void VisitBuffer(const PyRepBuffer *rep, int64 lvl ) = 0;
    virtual void VisitString(const PyRepString *rep, int64 lvl ) = 0;
    //! PackedRow type visitor
    virtual void VisitPackedRow(const PyRepPackedRow *rep, int64 lvl ) = 0;
    //! Object type visitor
    virtual void VisitObject(const PyRepObject *rep, int64 lvl ) = 0;
    virtual void VisitObjectEx(const PyRepObjectEx *rep, int64 lvl ) = 0;
    //! Structureated types Visitor
    virtual void VisitSubStruct(const PyRepSubStruct *rep, int64 lvl ) = 0;
    virtual void VisitSubStream(const PyRepSubStream *rep, int64 lvl ) = 0;
    virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep, int64 lvl ) = 0;
    //! the data types Visitor
    virtual void VisitDict(const PyRepDict *rep, int64 lvl ) = 0;
    virtual void VisitList(const PyRepList *rep, int64 lvl ) = 0;
    virtual void VisitTuple(const PyRepTuple *rep, int64 lvl ) = 0;
};

class PyVisitor {
public:
    virtual ~PyVisitor() {}

    //! primitive data visitors
    virtual void VisitInteger(const PyRepInteger *rep) = 0;
    virtual void VisitReal(const PyRepReal *rep) = 0;
    virtual void VisitBoolean(const PyRepBoolean *rep) = 0;
    virtual void VisitNone(const PyRepNone *rep) = 0;
    virtual void VisitBuffer(const PyRepBuffer *rep) = 0;
    virtual void VisitString(const PyRepString *rep) = 0;
    //! PackedRow type visitor
    virtual void VisitPackedRow(const PyRepPackedRow *rep) = 0;
    //! Object type visitor
    virtual void VisitObject(const PyRepObject *rep) = 0;
    virtual void VisitObjectEx(const PyRepObjectEx *rep) = 0;
    //! Structureated types Visitor
    virtual void VisitSubStruct(const PyRepSubStruct *rep) = 0;
    virtual void VisitSubStream(const PyRepSubStream *rep) = 0;
    virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep) = 0;
    //! the data types Visitor
    virtual void VisitDict(const PyRepDict *rep) = 0;
    virtual void VisitList(const PyRepList *rep) = 0;
    virtual void VisitTuple(const PyRepTuple *rep) = 0;
};


class SubStreamDecoder : public PyVisitor {
public:
    virtual ~SubStreamDecoder() {}

    //! primitive data visitors
    void VisitInteger(const PyRepInteger *rep);
    void VisitReal(const PyRepReal *rep);
    void VisitBoolean(const PyRepBoolean *rep);
    void VisitNone(const PyRepNone *rep);
    void VisitBuffer(const PyRepBuffer *rep);
    void VisitString(const PyRepString *rep);
    //! PackedRow type visitor
    void VisitPackedRow(const PyRepPackedRow *rep);
    //! Object type visitor
    void VisitObject(const PyRepObject *rep);
    void VisitObjectEx(const PyRepObjectEx *rep);
    void VisitSubStruct(const PyRepSubStruct *rep);

    //! the visitor needed for decoding the substream.
    void VisitSubStream(const PyRepSubStream *rep);

    void VisitChecksumedStream(const PyRepChecksumedStream *rep);
    //! the data types Visitor
    void VisitDict(const PyRepDict *rep);
    void VisitList(const PyRepList *rep);
    void VisitTuple(const PyRepTuple *rep);
};


#endif



