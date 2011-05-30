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

#include "XMLPktGenPCH.h"

#include "Generator.h"

/************************************************************************/
/* Generator                                                            */
/************************************************************************/
bool Generator::smEncTypesLoaded = false;
std::map<std::string, std::string> Generator::smEncTypes;

Generator::Generator( FILE* outputFile )
: mOutputFile( outputFile )
{
    LoadEncTypes();
}

void Generator::RegisterProcessors()
{
    AddMemberParser( "elementDef",      &Generator::ProcessElementDef );
    AddMemberParser( "element",         &Generator::ProcessElement );
    AddMemberParser( "elementPtr",      &Generator::ProcessElementPtr );

    AddMemberParser( "raw",             &Generator::ProcessRaw );
    AddMemberParser( "int",             &Generator::ProcessInt );
    AddMemberParser( "long",            &Generator::ProcessLong );
    AddMemberParser( "real",            &Generator::ProcessReal );
    AddMemberParser( "bool",            &Generator::ProcessBool );
    AddMemberParser( "none",            &Generator::ProcessNone );
    AddMemberParser( "buffer",          &Generator::ProcessBuffer );

    AddMemberParser( "string",          &Generator::ProcessString );
    AddMemberParser( "stringInline",    &Generator::ProcessStringInline );
    AddMemberParser( "wstring",         &Generator::ProcessWString );
    AddMemberParser( "wstringInline",   &Generator::ProcessWStringInline );
    AddMemberParser( "token",           &Generator::ProcessToken );
    AddMemberParser( "tokenInline",     &Generator::ProcessTokenInline );

    AddMemberParser( "object",          &Generator::ProcessObject );
    AddMemberParser( "objectInline",    &Generator::ProcessObjectInline );
    AddMemberParser( "objectEx",        &Generator::ProcessObjectEx );

    AddMemberParser( "tuple",           &Generator::ProcessTuple );
    AddMemberParser( "tupleInline",     &Generator::ProcessTupleInline );
    AddMemberParser( "list",            &Generator::ProcessList );
    AddMemberParser( "listInline",      &Generator::ProcessListInline );
    AddMemberParser( "listInt",         &Generator::ProcessListInt );
    AddMemberParser( "listLong",        &Generator::ProcessListLong );
    AddMemberParser( "listStr",         &Generator::ProcessListStr );
    AddMemberParser( "dict",            &Generator::ProcessDict );
    AddMemberParser( "dictInline",      &Generator::ProcessDictInline );
    AddMemberParser( "dictRaw",         &Generator::ProcessDictRaw );
    AddMemberParser( "dictInt",         &Generator::ProcessDictInt );
    AddMemberParser( "dictStr",         &Generator::ProcessDictStr );

    AddMemberParser( "substreamInline", &Generator::ProcessSubStreamInline );
    AddMemberParser( "substructInline", &Generator::ProcessSubStructInline );
}

const char* Generator::GetEncodeType( const TiXmlElement* element )
{
	std::map<std::string, std::string>::const_iterator res = smEncTypes.find( element->Value() );
	if( res == smEncTypes.end() )
		return "PyRep";

	return res->second.c_str();
}

void Generator::LoadEncTypes()
{
    if( !smEncTypesLoaded )
    {
        smEncTypes[ "elementDef" ] = "PyRep";
        smEncTypes[ "element" ] = "PyRep";
        smEncTypes[ "elementPtr" ] = "PyRep";

        smEncTypes[ "raw" ] = "PyRep";
        smEncTypes[ "int" ] = "PyInt";
        smEncTypes[ "long" ] = "PyLong";
        smEncTypes[ "real" ] = "PyFloat";
        smEncTypes[ "bool" ] = "PyBool";
        smEncTypes[ "none" ] = "PyNone";
        smEncTypes[ "buffer" ] = "PyBuffer";

        smEncTypes[ "string" ] = "PyString";
        smEncTypes[ "stringInline" ] = "PyString";
        smEncTypes[ "wstring" ] = "PyWString";
        smEncTypes[ "wstringInline" ] = "PyWString";
        smEncTypes[ "token" ] = "PyToken";
        smEncTypes[ "tokenInline" ] = "PyToken";

        smEncTypes[ "object" ] = "PyObject";
        smEncTypes[ "objectInline" ] = "PyObject";
        smEncTypes[ "objectEx" ] = "PyObjectEx";

        smEncTypes[ "tuple" ] = "PyTuple";
        smEncTypes[ "tupleInline" ] = "PyTuple";
        smEncTypes[ "list" ] = "PyList";
        smEncTypes[ "listInline" ] = "PyList";
        smEncTypes[ "listInt" ] = "PyList";
        smEncTypes[ "listLong" ] = "PyList";
        smEncTypes[ "listStr" ] = "PyList";
        smEncTypes[ "dict" ] = "PyDict";
        smEncTypes[ "dictInline" ] = "PyDict";
        smEncTypes[ "dictRaw" ] = "PyDict";
        smEncTypes[ "dictInt" ] = "PyDict";
        smEncTypes[ "dictStr" ] = "PyDict";

        smEncTypes[ "substreamInline" ] = "PySubStream";
        smEncTypes[ "substructInline" ] = "PySubStruct";

        smEncTypesLoaded = true;
    }
}
