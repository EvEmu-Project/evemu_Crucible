/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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

#ifndef __PYLOOKUPDUMP_H_INCL__
#define __PYLOOKUPDUMP_H_INCL__

#include "python/PyDumpVisitor.h"

class PyLookupResolver
{
public:
	bool LoadIntFile(const char *file);
	bool LoadStringFile(const char *file);

	const char *LookupInt(uint64 value) const;
	const char *LookupString(const char *value) const;

protected:
	std::map<uint32, std::string> m_intRecords;
	std::map<std::string, std::string> m_strRecords;
	mutable std::string m_dateBuffer;	//not thread safe or reentrant...
};

class PyLookupDumpVisitor : public PyLogDumpVisitor
{
public:
    PyLookupDumpVisitor( PyLookupResolver* _resolver, LogType log_type, LogType log_hex_type, const char* pfx = "", bool full_nested = false, bool full_hex = false );

    PyLookupResolver* resolver() const { return mResolver; }

protected:
	bool VisitInteger( const PyInt* rep );
	bool VisitString( const PyString* rep );

private:
	PyLookupResolver* const mResolver;
};

#endif
