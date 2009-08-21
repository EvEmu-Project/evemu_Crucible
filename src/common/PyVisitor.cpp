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


#include "PyVisitor.h"
#include "PyRep.h"

//! primitive data visitors
void SubStreamDecoder::VisitInteger(const PyInt *rep) {}
void SubStreamDecoder::VisitLong(const PyLong *rep) {}
void SubStreamDecoder::VisitReal(const PyFloat *rep) {}
void SubStreamDecoder::VisitBoolean(const PyBool *rep) {}
void SubStreamDecoder::VisitNone(const PyNone *rep) {}
void SubStreamDecoder::VisitBuffer(const PyBuffer *rep) {}
void SubStreamDecoder:: VisitString(const PyString *rep) {}
//! PackedRow type visitor
void SubStreamDecoder::VisitPackedRow(const PyPackedRow *rep) {}
//! Object type visitor
void SubStreamDecoder::VisitObject(const PyObject *rep) {
    rep->arguments->visit(this);
}

void SubStreamDecoder::VisitObjectEx(const PyObjectEx *rep) {
    rep->header->visit(this);

    {
        PyObjectEx::const_list_iterator cur, end;
        cur = rep->list_data.begin();
        end = rep->list_data.end();
        for(; cur != end; ++cur)
            (*cur)->visit(this);
    }

    {
        PyObjectEx::const_dict_iterator cur, end;
        cur = rep->dict_data.begin();
        end = rep->dict_data.end();
        for(; cur != end; ++cur)
        {
            cur->first->visit(this);
            cur->second->visit(this);
        }
    }
}

void SubStreamDecoder::VisitSubStruct(const PySubStruct *rep) {
    rep->sub->visit(this);
}


//! the visitor needed for decoding the substream.
//! @note whe only need to decode, and then visit the data types inside
//! the substream, searching for nested substreams that needs decoding...
void SubStreamDecoder::VisitSubStream(const PySubStream *rep) {
    rep->DecodeData();
    if(rep->decoded != NULL)
       rep->decoded->visit(this);
}


void SubStreamDecoder::VisitChecksumedStream(const PyChecksumedStream *rep) {}

void SubStreamDecoder::VisitDict(const PyDict *rep) {

    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        cur->first->visit(this);
        cur->second->visit(this);
    }
}

void SubStreamDecoder::VisitList(const PyList *rep) {

    PyList::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        (*cur)->visit(this);
    }
}

void SubStreamDecoder::VisitTuple(const PyTuple *rep) {

    PyTuple::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        (*cur)->visit(this);
    }
}
// TODO: remove this after testing.

/*
void PyVisitor::VisitObject(const PyObject *rep) {
    rep->arguments->visit(this);
}

void PyVisitor::VisitPackedRow(const PyPackedRow *rep) { }

void PyVisitor::VisitObjectEx(const PyObjectEx *rep) {
    VisitObjectExHeader(rep);
    VisitObjectExList(rep);
    VisitObjectExDict(rep);
}

void PyVisitor::VisitObjectExHeader(const PyObjectEx *rep) {
    rep->header->visit(this);
}

void PyVisitor::VisitObjectExList(const PyObjectEx *rep) {
    PyObjectEx::const_list_iterator cur, end;
    cur = rep->list_data.begin();
    end = rep->list_data.end();
    for(int i = 0; cur != end; cur++, i++)
        VisitObjectExListElement(rep, i, *cur);
}

void PyVisitor::VisitObjectExListElement(const PyObjectEx *rep, uint32 index, const PyRep *ele) {
    ele->visit(this);
}

void PyVisitor::VisitObjectExDict(const PyObjectEx *rep) {
    PyObjectEx::const_dict_iterator cur, end;
    cur = rep->dict_data.begin();
    end = rep->dict_data.end();
    for(int i = 0; cur != end; cur++, i++)
        VisitObjectExDictElement(rep, i, cur->first, cur->second);
}

void PyVisitor::VisitObjectExDictElement(const PyObjectEx *rep, uint32 index, const PyRep *key, const PyRep *value) {
    key->visit(this);
    value->visit(this);
}

void PyVisitor::VisitSubStruct(const PySubStruct *rep) {
    rep->sub->visit(this);
}

void PyVisitor::VisitSubStream(const PySubStream *rep) {
    if(rep->decoded != NULL)
        rep->decoded->visit(this);
}

void PyVisitor::VisitChecksumedStream(const PyChecksumedStream *rep) {
    rep->stream->visit(this);
}

void PyVisitor::VisitDict(const PyDict *rep) {

    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    uint32 r;
    for(r = 0; cur != end; cur++, r++) {
        VisitDictElement(rep, r, cur->first, cur->second);
    }
}


void PyVisitor::VisitDictElement(const PyDict *rep, uint32 index, const PyRep *key, const PyRep *value) {
    key->visit(this);
    value->visit(this);
}


void PyVisitor::VisitList(const PyList *rep) {
    PyList::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    uint32 r;
    for(r = 0; cur != end; cur++, r++) {
        VisitListElement(rep, r, *cur);
    }
}

void PyVisitor::VisitListElement(const PyList *rep, uint32 index, const PyRep *ele) {
    ele->visit(this);
}

void PyVisitor::VisitTuple(const PyTuple *rep) {
    PyTuple::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    uint32 r;
    for(r = 0; cur != end; cur++, r++) {
        VisitTupleElement(rep, r, *cur);
    }
}

void PyVisitor::VisitTupleElement(const PyTuple *rep, uint32 index, const PyRep *ele) {
    ele->visit(this);
}*/
