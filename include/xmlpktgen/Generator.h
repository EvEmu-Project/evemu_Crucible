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
	Author:		Zhur
*/

#ifndef __GENERATOR_H_INCL__
#define __GENERATOR_H_INCL__

#include <tinyxml.h>
#include "../common/logsys.h"
#include <stdio.h>
#include <map>
#include <string>

class Generator
{
public:
    virtual ~Generator() {}
    virtual void Generate(FILE *into, TiXmlElement *root);

protected:
    virtual void Process_root(FILE *into, TiXmlElement *element) = 0;
    virtual void Process_include(FILE *into, TiXmlElement *element);

    const char *GetEncodeType(TiXmlElement *element) const;
    std::map<std::string, std::string> m_eleTypes;
};

class ClassHeaderGenerator;

template <class T>
class TmplGenerator : public Generator {
public:
    virtual ~TmplGenerator() {}

protected:
    typedef bool (T:: * fProcPtr)(FILE *into, TiXmlElement *field);
    typedef typename std::map<std::string, fProcPtr>::iterator iterator;

    bool ProcessFields(FILE *into, TiXmlElement *element, int max=0) {
        TiXmlNode *field = NULL;
        int count = 0;
        while((field = element->IterateChildren( field )) ) {
            //pass through comments.
            if(field->Type() == TiXmlNode::COMMENT) {
                TiXmlComment *com = field->ToComment();
                fprintf(into, "\t/* %s */\n", com->Value());
                continue;
            }
            if(field->Type() != TiXmlNode::ELEMENT)
                continue;   //skip crap we dont care about

            count++;
            if(max > 0 && count > max) {
                _log(COMMON__ERROR, "Element at line %d has too many children elements, at most %d expected.", element->Row(), max);
                return false;
            }
            if(!DispatchField(into, field->ToElement()))
                return false;
        }
        return true;
    }

    bool DispatchField(FILE *into, TiXmlElement *field) {
        iterator res;
        res = m_procs.find(field->Value());
        if(res == m_procs.end()) {
            _log(COMMON__ERROR, "Unexpected field type '%s' on line %d", field->Value(), field->Row());
            return false;
        }
        fProcPtr p = res->second;
        T *t = (T *) this;

        return( (t->*p)(into, field) );
    }



    std::map<std::string, fProcPtr> m_procs;
};

#define ProcFDecl(t) \
    bool Process_##t(FILE *into, TiXmlElement *field)
#define ProcFMap(c, ename, type) \
    m_eleTypes[ #ename ] = #type; m_procs[ #ename ] = &c::Process_##ename

#define AllProcFDecls() \
    ProcFDecl(InlineTuple); \
    ProcFDecl(InlineDict); \
    ProcFDecl(InlineList); \
    ProcFDecl(InlineSubStream); \
    ProcFDecl(InlineSubStruct); \
    ProcFDecl(strdict); \
    ProcFDecl(intdict); \
    ProcFDecl(primdict); \
    ProcFDecl(strlist); \
    ProcFDecl(intlist); \
    ProcFDecl(int64list); \
    ProcFDecl(element); \
    ProcFDecl(elementptr); \
    ProcFDecl(none); \
    ProcFDecl(object); \
    ProcFDecl(object_ex); \
    ProcFDecl(buffer); \
    ProcFDecl(raw); \
    ProcFDecl(dict); \
    ProcFDecl(list); \
    ProcFDecl(tuple); \
    ProcFDecl(int); \
    ProcFDecl(bool); \
    ProcFDecl(int64); \
    ProcFDecl(real); \
    ProcFDecl(string)

#define AllProcFMaps(c) \
    ProcFMap(c, InlineTuple, PyTuple); \
    ProcFMap(c, InlineDict, PyDict); \
    ProcFMap(c, InlineList, PyList); \
    ProcFMap(c, InlineSubStream, PySubStream); \
    ProcFMap(c, InlineSubStruct, PySubStruct); \
    ProcFMap(c, strdict, PyDict); \
    ProcFMap(c, intdict, PyDict); \
    ProcFMap(c, primdict, PyDict); \
    ProcFMap(c, strlist, PyList); \
    ProcFMap(c, intlist, PyList); \
    ProcFMap(c, int64list, PyList); \
    ProcFMap(c, element, PyRep); \
    ProcFMap(c, elementptr, PyRep); \
    ProcFMap(c, none, PyRep); \
    ProcFMap(c, object, PyObject); \
    ProcFMap(c, object_ex, PyObjectEx); \
    ProcFMap(c, buffer, PyBuffer); \
    ProcFMap(c, raw, PyRep); \
    ProcFMap(c, list, PyList); \
    ProcFMap(c, tuple, PyTuple); \
    ProcFMap(c, dict, PyDict); \
    ProcFMap(c, int, PyInt); \
    ProcFMap(c, bool, PyBool); \
    ProcFMap(c, int64, PyInt); \
    ProcFMap(c, real, PyFloat); \
    ProcFMap(c, string, PyString)



#endif


