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
class PyRepSubStruct;
class PyRepSubStream;
class PyRepChecksumedStream;
class PyRepDict;
class PyRepList;
class PyRepTuple;
class PyRepPackedRow;
class PyRepPackedRowHeader;
class PyRepPackedResultSet;

class PyVisitor {
public:
	virtual ~PyVisitor() {}
	virtual void VisitInteger(const PyRepInteger *rep);
	virtual void VisitReal(const PyRepReal *rep);
	virtual void VisitBoolean(const PyRepBoolean *rep);
	virtual void VisitNone(const PyRepNone *rep);
	virtual void VisitBuffer(const PyRepBuffer *rep);
	virtual void VisitString(const PyRepString *rep);
	
	virtual void VisitObject(const PyRepObject *rep);
	
	virtual void VisitPacked(const PyRepPackedRow *rep);
	virtual void VisitPackedRowHeader(const PyRepPackedRowHeader *rep);
	virtual void VisitPackedRowHeaderElement(const PyRepPackedRowHeader *rep, uint32 index, const PyRepPackedRow *element);
	virtual void VisitPackedResultSet(const PyRepPackedResultSet *rep);
	virtual void VisitPackedResultSetElement(const PyRepPackedResultSet *rep, uint32 index, const PyRepPackedRow *element);
	
	virtual void VisitSubStruct(const PyRepSubStruct *rep);
	virtual void VisitSubStream(const PyRepSubStream *rep);
	virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep);
	
	virtual void VisitDict(const PyRepDict *rep);
	virtual void VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value);
	
	virtual void VisitList(const PyRepList *rep);
	virtual void VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele);
	
	virtual void VisitTuple(const PyRepTuple *rep);
	virtual void VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele);
};


class SubStreamDecoder : public PyVisitor {
public:
	virtual ~SubStreamDecoder() {}
	virtual void VisitSubStream(const PyRepSubStream *rep);
};


#endif



