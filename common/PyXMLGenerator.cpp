/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

void PyXMLGenerator::VisitInteger(const PyRepInteger *rep) {
	fprintf(m_into, "%s<%s name=\"integer%d\" />\n", top(),
		(rep->value > 0xFFFFFFFFLL)?"int64":"int",
		m_item++
		);
}

void PyXMLGenerator::VisitReal(const PyRepReal *rep) {
	fprintf(m_into, "%s<real name=\"real%d\" />\n", top(),
		m_item++
		);
}

void PyXMLGenerator::VisitBoolean(const PyRepBoolean *rep) {
	fprintf(m_into, "%s<bool name=\"bool%d\" />\n", top(),
		m_item++
		);
}

void PyXMLGenerator::VisitNone(const PyRepNone *rep) {
	fprintf(m_into, "%s<none />\n", top());
}

void PyXMLGenerator::VisitBuffer(const PyRepBuffer *rep) {
	fprintf(m_into, "%s<buffer name=\"buffer%d\" />\n", top(),
		m_item++
		);
}

void PyXMLGenerator::VisitString(const PyRepString *rep) {
	fprintf(m_into, "%s<string name=\"string%d\" />\n", top(),
		m_item++
		);
}

	
void PyXMLGenerator::VisitObject(const PyRepObject *rep) {
	//do not visit the type:

	fprintf(m_into, "%s<object type=\"%s\">\n", top(),
		rep->type.c_str()
		);
	
	std::string indent(top());
	indent += indent_amount;
	push(indent.c_str());
	rep->arguments->visit(this);
	pop();
	
	fprintf(m_into, "%s</object>\n", top()
		);
	
}

void PyXMLGenerator::VisitSubStruct(const PyRepSubStruct *rep) {
	fprintf(m_into, "%s<InlineSubStruct>\n", top()
		);
	
	std::string indent(top());
	indent += indent_amount;
	push(indent.c_str());
	rep->sub->visit(this);
	pop();
	
	fprintf(m_into, "%s</InlineSubStruct>\n", top()
		);
}

void PyXMLGenerator::VisitSubStream(const PyRepSubStream *rep) {
	fprintf(m_into, "%s<InlineSubStream>\n", top()
		);
	
	rep->DecodeData();
	if(rep->decoded != NULL) {
		std::string indent(top());
		indent += indent_amount;
		push(indent.c_str());
		rep->decoded->visit(this);
		pop();
	} else {
		fprintf(m_into, "%s  <!-- UNABLE TO DECODE SUBSTREAM! -->\n", top());
	}
	
	fprintf(m_into, "%s</InlineSubStream>\n", top()
		);
}

void PyXMLGenerator::VisitDict(const PyRepDict *rep) {

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
	PyRepDict::const_iterator cur, end;
	cur = rep->begin();
	end = rep->end();
	for(; cur != end; cur++) {
		if(cur->first->CheckType(PyRep::String)) {
			if(ktype == DictIntKey) {
				//we have varying key types, raw dict it is.
				ktype = DictRaw;
				break;
			} else if(ktype == DictInline) {
				ktype = DictStringKey;
			}
		} else if(cur->first->CheckType(PyRep::Integer)) {
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

		if(cur->second->CheckType(PyRep::String)) {
			if(vtype == ValueInt || vtype == ValueReal) {
				vtype = ValueMixed;
			} else if(vtype == ValueUnknown)
				vtype = ValueString;
		} else if(cur->second->CheckType(PyRep::Integer)) {
			if(vtype == ValueString || vtype == ValueReal) {
				vtype = ValueMixed;
			} else if(vtype == ValueUnknown)
				vtype = ValueInt;
		} else if(cur->second->CheckType(PyRep::Real)) {
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
	PyVisitor::VisitDict(rep);
	pop();
	
	fprintf(m_into, "%s</InlineDict>\n", top()
		);
}

void PyXMLGenerator::VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value) {
	if(!key->CheckType(PyRep::String)) {
		fprintf(m_into, "%s<!-- non-string dict key of type %s -->\n", top(), key->TypeString());
		return;
	}
	PyRepString *str = (PyRepString *) key;
	
	fprintf(m_into, "%s<IDEntry key=\"%s\">\n", top(), str->value.c_str());

	std::string indent(top());
	indent += indent_amount;
	push(indent.c_str());
	value->visit(this);
	pop();
	
	fprintf(m_into, "%s</IDEntry>\n", top()
		);
}

void PyXMLGenerator::VisitList(const PyRepList *rep) {
	
	//for now presume we cant do anything useful with lists that contain 
    //more than a few things...
	if(rep->items.size() < 5) {
		fprintf(m_into, "%s<InlineList>\n", top()
			);
		
		std::string indent(top());
		indent += indent_amount;
		push(indent.c_str());
		PyVisitor::VisitList(rep);
		pop();
		
		fprintf(m_into, "%s</InlineList>\n", top()
			);
	} else {
		enum {
			TypeUnknown,
			TypeString,
			TypeInteger,
			TypeReal,
			TypeMixed
		} eletype;
		eletype = TypeUnknown;
		
		//scan the list to see if we can classify the contents.
		PyRepList::const_iterator cur, end;
		cur = rep->begin();
		end = rep->end();
		for(; cur != end; cur++) {
			if((*cur)->CheckType(PyRep::String)) {
				if(eletype == TypeInteger || eletype == TypeReal) {
					eletype = TypeMixed;
					break;
				} else if(eletype == TypeUnknown)
					eletype = TypeString;
			} else if((*cur)->CheckType(PyRep::Integer)) {
				if(eletype == TypeString || eletype == TypeReal) {
					eletype = TypeMixed;
					break;
				} else if(eletype == TypeUnknown)
					eletype = TypeInteger;
			} else if((*cur)->CheckType(PyRep::Real)) {
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

void PyXMLGenerator::VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele) {
	fprintf(m_into, "%s<!-- %d -->\n", top(), index );
	PyVisitor::VisitListElement(rep, index, ele);
}

void PyXMLGenerator::VisitTuple(const PyRepTuple *rep) {
	
	fprintf(m_into, "%s<InlineTuple>\n", top()
		);
	
	std::string indent(top());
	indent += indent_amount;
	push(indent.c_str());
	PyVisitor::VisitTuple(rep);
	pop();
	
	fprintf(m_into, "%s</InlineTuple>\n", top()
		);
}

void PyXMLGenerator::VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele) {
	fprintf(m_into, "%s<!-- %d -->\n", top(), index );
	PyVisitor::VisitTupleElement(rep, index, ele);
}





















