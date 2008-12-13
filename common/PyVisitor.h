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
class PyRepNewObject;
class PyRepPackedRow;

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
	
	virtual void VisitPackedRow(const PyRepPackedRow *rep);

	virtual void VisitNewObject(const PyRepNewObject *rep);
	virtual void VisitNewObjectHeader(const PyRepNewObject *rep);
	virtual void VisitNewObjectList(const PyRepNewObject *rep);
	virtual void VisitNewObjectListElement(const PyRepNewObject *rep, uint32 index, const PyRep *ele);
	virtual void VisitNewObjectDict(const PyRepNewObject *rep);
	virtual void VisitNewObjectDictElement(const PyRepNewObject *rep, uint32 index, const PyRep *key, const PyRep *value);
	
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



