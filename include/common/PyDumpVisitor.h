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

#ifndef __PYDUMPVISITOR_H_INCL__
#define __PYDUMPVISITOR_H_INCL__

#include "logsys.h"
#include <stdio.h>
#include <string>

class PyDumpVisitor
: public PyVisitorLvl {
public:
    PyDumpVisitor(bool full_lists);
    virtual ~PyDumpVisitor();

protected:
    virtual void _print(const char *str, ...) = 0;
    virtual void _print(const std::string &str, ...) = 0;
    virtual void _hexDump(const uint8 *bytes, uint32 len, const char * ident) = 0;

    const bool m_full_lists;

    //! primitive data visitors
    void VisitInteger(const PyRepInteger *rep, int64 lvl );
    void VisitReal(const PyRepReal *rep, int64 lvl );
    void VisitBoolean(const PyRepBoolean *rep, int64 lvl );
    void VisitNone(const PyRepNone *rep, int64 lvl );
    void VisitBuffer(const PyRepBuffer *rep, int64 lvl );
    void VisitString(const PyRepString *rep, int64 lvl );
    //! PackedRow type visitor
    void VisitPackedRow(const PyRepPackedRow *rep, int64 lvl );
    //! Object type visitor
    void VisitObject(const PyRepObject *rep, int64 lvl );
    void VisitObjectEx(const PyRepObjectEx *rep, int64 lvl );
    //! Structureated types Visitor
    void VisitSubStruct(const PyRepSubStruct *rep, int64 lvl );
        virtual void VisitSubStream(const PyRepSubStream *rep, int64 lvl );
    void VisitChecksumedStream(const PyRepChecksumedStream *rep, int64 lvl );
    //! the data types Visitor
        virtual void VisitDict(const PyRepDict *rep, int64 lvl );
        virtual void VisitList(const PyRepList *rep, int64 lvl );
        virtual void VisitTuple(const PyRepTuple *rep, int64 lvl );
};

class PyLogsysDump : public PyDumpVisitor {
public:
    PyLogsysDump(LogType type, bool full_hex = false, bool full_lists = false);
    PyLogsysDump(LogType type, LogType hex_type, bool full_hex, bool full_lists);
    virtual ~PyLogsysDump() { }

protected:
    //overloaded for speed enhancements when disabled
    void VisitDict(const PyRepDict *rep, int64 lvl );
    void VisitList(const PyRepList *rep, int64 lvl );
    void VisitTuple(const PyRepTuple *rep, int64 lvl );
    void VisitSubStream(const PyRepSubStream *rep, int64 lvl );

    const LogType m_type;
    const LogType m_hex_type;
    const bool m_full_hex;
    void _print(const char *str, ...);
    void _print(const std::string &str, ...);
    void _hexDump(const uint8 *bytes, uint32 len, const char * ident);
};

class PyFileDump : public PyDumpVisitor {
public:
    PyFileDump(FILE *into, bool full_hex = false);
    virtual ~PyFileDump() { }

protected:
    FILE *const m_into;
    const bool m_full_hex;
    void _print(const char *str, ...);
    void _print(const std::string &str, ...);
    void _hexDump(const uint8 *bytes, uint32 len, const char * ident);
    void _pfxHexDump(const uint8 *bytes, uint32 len, const char * ident);
};

#endif



