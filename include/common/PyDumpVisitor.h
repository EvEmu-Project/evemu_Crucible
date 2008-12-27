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

#ifndef __PYDUMPVISITOR_H_INCL__
#define __PYDUMPVISITOR_H_INCL__

#include "PyVisitor.h"
#include "logsys.h"
#include <stdio.h>
#include <string>
#include <stack>

class PyDumpVisitor
: public PyVisitor {
public:
	PyDumpVisitor(bool full_lists);
	virtual ~PyDumpVisitor();
	
protected:
	virtual void _print(const char *str, ...) = 0;
	virtual void _hexDump(const uint8 *bytes, uint32 len) = 0;
	
	const bool m_full_lists;
	
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
	
	virtual void VisitPackedRow(const PyRepPackedRow *rep);

	virtual void VisitNewObject(const PyRepNewObject *rep);
	virtual void VisitNewObjectHeader(const PyRepNewObject *rep);
	virtual void VisitNewObjectList(const PyRepNewObject *rep);
	virtual void VisitNewObjectListElement(const PyRepNewObject *rep, uint32 index, const PyRep *ele);
	virtual void VisitNewObjectDict(const PyRepNewObject *rep);
	virtual void VisitNewObjectDictElement(const PyRepNewObject *rep, uint32 index, const PyRep *key, const PyRep *value);
	
	virtual void VisitSubStruct(const PyRepSubStruct *rep);
	virtual void VisitSubStream(const PyRepSubStream *rep);
	
	virtual void VisitDict(const PyRepDict *rep);
	virtual void VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value);
	
	virtual void VisitList(const PyRepList *rep);
	virtual void VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele);
	
	virtual void VisitTuple(const PyRepTuple *rep);
	virtual void VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele);

};

class PyLogsysDump : public PyDumpVisitor {
public:
	PyLogsysDump(LogType type, bool full_hex = false, bool full_lists = false);
	PyLogsysDump(LogType type, LogType hex_type, bool full_hex, bool full_lists);
	virtual ~PyLogsysDump() { }

protected:
	//overloaded for speed enhancements when disabled
	virtual void VisitDict(const PyRepDict *rep);
	virtual void VisitList(const PyRepList *rep);
	virtual void VisitTuple(const PyRepTuple *rep);
	virtual void VisitSubStream(const PyRepSubStream *rep);
	
	const LogType m_type;
	const LogType m_hex_type;
	const bool m_full_hex;
	virtual void _print(const char *str, ...);
	virtual void _hexDump(const uint8 *bytes, uint32 len);
};

class PyFileDump : public PyDumpVisitor {
public:
	PyFileDump(FILE *into, bool full_hex = false);
	virtual ~PyFileDump() { }

protected:
	FILE *const m_into;
	const bool m_full_hex;
	virtual void _print(const char *str, ...);
	virtual void _hexDump(const uint8 *bytes, uint32 len);
	void _pfxHexDump(const uint8 *bytes, uint32 len);
};

#endif



