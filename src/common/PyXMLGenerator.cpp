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

#include "PyXMLGenerator.h"
#include "PyRep.h"

static const char *indent_amount = "  ";

PyXMLGenerator::PyXMLGenerator(FILE *into)
: PyVisitor(),
  m_into(into),
  m_item(0)
{
    push(indent_amount);
}

PyXMLGenerator::~PyXMLGenerator() {
}

void PyXMLGenerator::VisitInteger(const PyInt *rep) {
    fprintf(m_into, "%s<%s name=\"integer%d\" />\n", top(),
        (rep->value() > 0xFFFFFFFFLL)?"int64":"int",
        m_item++
        );
}

void PyXMLGenerator::VisitReal(const PyFloat *rep) {
    fprintf(m_into, "%s<real name=\"real%d\" />\n", top(),
        m_item++
        );
}

void PyXMLGenerator::VisitBoolean(const PyBool *rep) {
    fprintf(m_into, "%s<bool name=\"bool%d\" />\n", top(),
        m_item++
        );
}

void PyXMLGenerator::VisitNone(const PyNone *rep) {
    fprintf(m_into, "%s<none />\n", top());
}

void PyXMLGenerator::VisitBuffer(const PyBuffer *rep) {
    fprintf(m_into, "%s<buffer name=\"buffer%d\" />\n", top(),
        m_item++
        );
}

void PyXMLGenerator::VisitString(const PyString *rep) {
    fprintf(m_into, "%s<string name=\"string%d\" />\n", top(),
        m_item++
        );
}


void PyXMLGenerator::VisitObject(const PyObject *rep) {
    //do not visit the type:

    fprintf(m_into, "%s<object type=\"%s\">\n", top(),
        rep->type().c_str()
        );

    std::string indent(top());
    indent += indent_amount;
    push(indent.c_str());
    rep->arguments()->visit(this);
    pop();

    fprintf(m_into, "%s</object>\n", top()
        );

}

void PyXMLGenerator::VisitSubStruct(const PySubStruct *rep) {
    fprintf(m_into, "%s<InlineSubStruct>\n", top()
        );

    std::string indent(top());
    indent += indent_amount;
    push(indent.c_str());
    rep->sub()->visit(this);
    pop();

    fprintf(m_into, "%s</InlineSubStruct>\n", top()
        );
}

void PyXMLGenerator::VisitSubStream(const PySubStream *rep) {
    fprintf(m_into, "%s<InlineSubStream>\n", top()  );

    rep->DecodeData();
    if(rep->decoded() != NULL) {
        std::string indent(top());
        indent += indent_amount;
        push(indent.c_str());
        rep->decoded()->visit(this);
        pop();
    } else {
        fprintf(m_into, "%s  <!-- UNABLE TO DECODE SUBSTREAM! -->\n", top());
    }

    fprintf(m_into, "%s</InlineSubStream>\n", top() );
}

void PyXMLGenerator::VisitDict(const PyDict *rep) {

    enum {
        DictInline,
        DictStringKey,
        DictIntKey,
        DictRaw
    } ktype;
    enum {
        ValueUnknown,
        ValueString,
        ValueInt,
        ValueReal,
        ValueMixed
    } vtype;

    ktype = DictInline;
    vtype = ValueUnknown;

    //this is kinda a hack, but we want to try and classify the contents of this dict:
    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur)
    {
        if(cur->first->IsString()) {
            if(ktype == DictIntKey) {
                //we have varying key types, raw dict it is.
                ktype = DictRaw;
                break;
            } else if(ktype == DictInline) {
                ktype = DictStringKey;
            }
        } else if(cur->first->IsInt()) {
            if(ktype == DictStringKey) {
                //we have varying key types, raw dict it is.
                ktype = DictRaw;
                break;
            } else if(ktype == DictInline) {
                ktype = DictIntKey;
            }
        } else {
            //anything but those key types is more than we can think about, keep it raw.
            ktype = DictRaw;
            break;
        }

        if(cur->second->IsString()) {
            if(vtype == ValueInt || vtype == ValueReal) {
                vtype = ValueMixed;
            } else if(vtype == ValueUnknown)
                vtype = ValueString;
        } else if(cur->second->IsInt()) {
            if(vtype == ValueString || vtype == ValueReal) {
                vtype = ValueMixed;
            } else if(vtype == ValueUnknown)
                vtype = ValueInt;
        } else if(cur->second->IsFloat()) {
            if(vtype == ValueString || vtype == ValueInt) {
                vtype = ValueMixed;
            } else if(vtype == ValueUnknown)
                vtype = ValueReal;
        } else {
            vtype = ValueMixed;
        }
    }

    if(ktype == DictRaw) {
        fprintf(m_into, "%s<dict name=\"dict%d\" />\n", top(), m_item++);
        return;
    } else if(ktype == DictIntKey) {
        //cant do an inline dict, but can try a vector
        switch(vtype) {
        case ValueString:
            fprintf(m_into, "%s<intdict name=\"dict%d\" type=\"string\" />\n", top(), m_item++);
            break;
        case ValueInt:
            fprintf(m_into, "%s<intdict name=\"dict%d\" type=\"int\" />\n", top(), m_item++);
            break;
        case ValueReal:
            fprintf(m_into, "%s<intdict name=\"dict%d\" type=\"real\" />\n", top(), m_item++);
            break;
        case ValueUnknown:
        case ValueMixed:
            fprintf(m_into, "%s<rawdict name=\"dict%d\" />\n", top(), m_item++);
            break;
        }
        return;
    }



    fprintf(m_into, "%s<InlineDict>\n", top()
        );

    std::string indent(top());
    indent += indent_amount;
    push(indent.c_str());

    //! visit dict elements.
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {

        if(!cur->first->IsString()) {
            fprintf(m_into, "%s<!-- non-string dict key of type %s -->\n", top(), cur->first->TypeString());
            return;
        }
        PyString *str = (PyString *) cur->first;

        fprintf(m_into, "%s<IDEntry key=\"%s\">\n", top(), str->content());

        std::string indent(top());
        indent += indent_amount;
        push(indent.c_str());
        cur->second->visit(this);
        pop();

        fprintf(m_into, "%s</IDEntry>\n", top() );
    }

    pop();

    fprintf(m_into, "%s</InlineDict>\n", top() );
}

void PyXMLGenerator::VisitList(const PyList *rep) {

    //for now presume we cant do anything useful with lists that contain
    //more than a few things...
    if(rep->items.size() < 5) {
        fprintf(m_into, "%s<InlineList>\n", top()
            );

        std::string indent(top());
        indent += indent_amount;
        push(indent.c_str());

        //! visit the list elements.
        PyList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(uint32 i = 0; cur != end; ++cur, ++i) {
            fprintf(m_into, "%s<!-- %d -->\n", top(), i );
            (*cur)->visit(this);
        }

        pop();

        fprintf(m_into, "%s</InlineList>\n", top() );
    }
    else
    {
        enum {
            TypeUnknown,
            TypeString,
            TypeInteger,
            TypeReal,
            TypeMixed
        } eletype;
        eletype = TypeUnknown;

        //scan the list to see if we can classify the contents.
        PyList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(; cur != end; cur++) {
            if((*cur)->IsString()) {
                if(eletype == TypeInteger || eletype == TypeReal) {
                    eletype = TypeMixed;
                    break;
                } else if(eletype == TypeUnknown)
                    eletype = TypeString;
            } else if((*cur)->IsInt()) {
                if(eletype == TypeString || eletype == TypeReal) {
                    eletype = TypeMixed;
                    break;
                } else if(eletype == TypeUnknown)
                    eletype = TypeInteger;
            } else if((*cur)->IsFloat()) {
                if(eletype == TypeString || eletype == TypeInteger) {
                    eletype = TypeMixed;
                    break;
                } else if(eletype == TypeUnknown)
                    eletype = TypeReal;
            } else {
                eletype = TypeMixed;
                break;
            }
        }
        switch(eletype) {
        case TypeString:
            fprintf(m_into, "%s<stringlist name=\"list%d\" />\n", top(), m_item++);
            break;
        case TypeInteger:
            fprintf(m_into, "%s<intlist name=\"list%d\" />\n", top(), m_item++);
            break;
        case TypeReal:
            fprintf(m_into, "%s<reallist name=\"list%d\" />\n", top(), m_item++);
            break;
        case TypeUnknown:
        case TypeMixed:
            fprintf(m_into, "%s<list name=\"list%d\" />\n", top(), m_item++);
            break;
        }
    }
}

void PyXMLGenerator::VisitTuple(const PyTuple *rep) {

    fprintf(m_into, "%s<InlineTuple>\n", top()
        );

    std::string indent(top());
    indent += indent_amount;
    push(indent.c_str());

    //! visits the tuple elements.
    PyTuple::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(uint32 i = 0; cur != end; ++cur, ++i) {
        fprintf(m_into, "%s<!-- %d -->\n", top(), i );
        (*cur)->visit(this);
    }

    pop();

    fprintf(m_into, "%s</InlineTuple>\n", top() );
}




















