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

#ifndef __CLONEGENERATOR_H_INCL__
#define __CLONEGENERATOR_H_INCL__

#include "Generator.h"

class ClassCloneGenerator
: public Generator
{
public:
	ClassCloneGenerator( FILE* outputFile = NULL );

protected:
    void RegisterProcessors();

    bool ProcessElementDef( const TiXmlElement* field );
    bool ProcessElement( const TiXmlElement* field );
    bool ProcessElementPtr( const TiXmlElement* field );

    bool ProcessRaw( const TiXmlElement* field );
    bool ProcessInt( const TiXmlElement* field );
    bool ProcessLong( const TiXmlElement* field );
    bool ProcessReal( const TiXmlElement* field );
    bool ProcessBool( const TiXmlElement* field );
    bool ProcessNone( const TiXmlElement* field );
    bool ProcessBuffer( const TiXmlElement* field );

    bool ProcessString( const TiXmlElement* field );
    bool ProcessStringInline( const TiXmlElement* field );
    bool ProcessWString( const TiXmlElement* field );
    bool ProcessWStringInline( const TiXmlElement* field );
    bool ProcessToken( const TiXmlElement* field );
    bool ProcessTokenInline( const TiXmlElement* field );

    bool ProcessObject( const TiXmlElement* field );
    bool ProcessObjectInline( const TiXmlElement* field );
    bool ProcessObjectEx( const TiXmlElement* field );

    bool ProcessTuple( const TiXmlElement* field );
    bool ProcessTupleInline( const TiXmlElement* field );
    bool ProcessList( const TiXmlElement* field );
    bool ProcessListInline( const TiXmlElement* field );
    bool ProcessListInt( const TiXmlElement* field );
    bool ProcessListLong( const TiXmlElement* field );
    bool ProcessListStr( const TiXmlElement* field );
    bool ProcessDict( const TiXmlElement* field );
    bool ProcessDictInline( const TiXmlElement* field );
    bool ProcessDictInlineEntry( const TiXmlElement* field );
    bool ProcessDictRaw( const TiXmlElement* field );
    bool ProcessDictInt( const TiXmlElement* field );
    bool ProcessDictStr( const TiXmlElement* field );

    bool ProcessSubStreamInline( const TiXmlElement* field );
    bool ProcessSubStructInline( const TiXmlElement* field );
};

#endif


