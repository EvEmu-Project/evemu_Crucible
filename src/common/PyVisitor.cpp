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
void SubStreamDecoder::VisitInteger(const PyRepInteger *rep) {}
void SubStreamDecoder::VisitReal(const PyRepReal *rep) {}
void SubStreamDecoder::VisitBoolean(const PyRepBoolean *rep) {}
void SubStreamDecoder::VisitNone(const PyRepNone *rep) {}
void SubStreamDecoder::VisitBuffer(const PyRepBuffer *rep) {}
void SubStreamDecoder:: VisitString(const PyRepString *rep) {}
//! PackedRow type visitor
void SubStreamDecoder::VisitPackedRow(const PyRepPackedRow *rep) {}
//! Object type visitor
void SubStreamDecoder::VisitObject(const PyRepObject *rep) {
    rep->arguments->visit(this);
}

void SubStreamDecoder::VisitObjectEx(const PyRepObjectEx *rep) {
    rep->header->visit(this);

    {
        PyRepObjectEx::const_list_iterator cur, end;
        cur = rep->list_data.begin();
        end = rep->list_data.end();
        for(; cur != end; ++cur)
            (*cur)->visit(this);
    }

    {
        PyRepObjectEx::const_dict_iterator cur, end;
        cur = rep->dict_data.begin();
        end = rep->dict_data.end();
        for(; cur != end; ++cur)
        {
            cur->first->visit(this);
            cur->second->visit(this);
        }
    }
}

void SubStreamDecoder::VisitSubStruct(const PyRepSubStruct *rep) {
    rep->sub->visit(this);
}


//! the visitor needed for decoding the substream.
//! @note whe only need to decode, and then visit the data types inside
//! the substream, searching for nested substreams that needs decoding...
void SubStreamDecoder::VisitSubStream(const PyRepSubStream *rep) {
    rep->DecodeData();
    if(rep->decoded != NULL)
       rep->decoded->visit(this);
}


void SubStreamDecoder::VisitChecksumedStream(const PyRepChecksumedStream *rep) {}

void SubStreamDecoder::VisitDict(const PyRepDict *rep) {

    PyRepDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        cur->first->visit(this);
        cur->second->visit(this);
    }
}

void SubStreamDecoder::VisitList(const PyRepList *rep) {

    PyRepList::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        (*cur)->visit(this);
    }
}

void SubStreamDecoder::VisitTuple(const PyRepTuple *rep) {

    PyRepTuple::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        (*cur)->visit(this);
    }
}
// TODO: remove this after testing.

/*
void PyVisitor::VisitObject(const PyRepObject *rep) {
    rep->arguments->visit(this);
}

void PyVisitor::VisitPackedRow(const PyRepPackedRow *rep) { }

void PyVisitor::VisitObjectEx(const PyRepObjectEx *rep) {
    VisitObjectExHeader(rep);
    VisitObjectExList(rep);
    VisitObjectExDict(rep);
}

void PyVisitor::VisitObjectExHeader(const PyRepObjectEx *rep) {
    rep->header->visit(this);
}

void PyVisitor::VisitObjectExList(const PyRepObjectEx *rep) {
    PyRepObjectEx::const_list_iterator cur, end;
    cur = rep->list_data.begin();
    end = rep->list_data.end();
    for(int i = 0; cur != end; cur++, i++)
        VisitObjectExListElement(rep, i, *cur);
}

void PyVisitor::VisitObjectExListElement(const PyRepObjectEx *rep, uint32 index, const PyRep *ele) {
    ele->visit(this);
}

void PyVisitor::VisitObjectExDict(const PyRepObjectEx *rep) {
    PyRepObjectEx::const_dict_iterator cur, end;
    cur = rep->dict_data.begin();
    end = rep->dict_data.end();
    for(int i = 0; cur != end; cur++, i++)
        VisitObjectExDictElement(rep, i, cur->first, cur->second);
}

void PyVisitor::VisitObjectExDictElement(const PyRepObjectEx *rep, uint32 index, const PyRep *key, const PyRep *value) {
    key->visit(this);
    value->visit(this);
}

void PyVisitor::VisitSubStruct(const PyRepSubStruct *rep) {
    rep->sub->visit(this);
}

void PyVisitor::VisitSubStream(const PyRepSubStream *rep) {
    if(rep->decoded != NULL)
        rep->decoded->visit(this);
}

void PyVisitor::VisitChecksumedStream(const PyRepChecksumedStream *rep) {
    rep->stream->visit(this);
}

void PyVisitor::VisitDict(const PyRepDict *rep) {

    PyRepDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    uint32 r;
    for(r = 0; cur != end; cur++, r++) {
        VisitDictElement(rep, r, cur->first, cur->second);
    }
}


void PyVisitor::VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value) {
    key->visit(this);
    value->visit(this);
}


void PyVisitor::VisitList(const PyRepList *rep) {
    PyRepList::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    uint32 r;
    for(r = 0; cur != end; cur++, r++) {
        VisitListElement(rep, r, *cur);
    }
}

void PyVisitor::VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele) {
    ele->visit(this);
}

void PyVisitor::VisitTuple(const PyRepTuple *rep) {
    PyRepTuple::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    uint32 r;
    for(r = 0; cur != end; cur++, r++) {
        VisitTupleElement(rep, r, *cur);
    }
}

void PyVisitor::VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele) {
    ele->visit(this);
}*/
