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
	
	virtual void VisitInteger(const PyRepInteger *rep);
	virtual void VisitReal(const PyRepReal *rep);
	virtual void VisitBoolean(const PyRepBoolean *rep);
	virtual void VisitNone(const PyRepNone *rep);
	virtual void VisitBuffer(const PyRepBuffer *rep);
	virtual void VisitString(const PyRepString *rep);
	
	virtual void VisitObject(const PyRepObject *rep);
	
	virtual void VisitSubStruct(const PyRepSubStruct *rep);
	virtual void VisitSubStream(const PyRepSubStream *rep);
	
	virtual void VisitDict(const PyRepDict *rep);
	virtual void VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value);
	
	virtual void VisitList(const PyRepList *rep);
	virtual void VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele);
	
	virtual void VisitTuple(const PyRepTuple *rep);
	virtual void VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele);

};

#endif



