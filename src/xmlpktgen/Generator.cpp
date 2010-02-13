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
    RegisterParser( "elementDef",      &Generator::ProcessElementDef );
    RegisterParser( "element",         &Generator::ProcessElement );
    RegisterParser( "elementPtr",      &Generator::ProcessElementPtr );

    RegisterParser( "raw",             &Generator::ProcessRaw );
    RegisterParser( "int",             &Generator::ProcessInt );
    RegisterParser( "long",            &Generator::ProcessLong );
    RegisterParser( "real",            &Generator::ProcessReal );
    RegisterParser( "bool",            &Generator::ProcessBool );
    RegisterParser( "none",            &Generator::ProcessNone );
    RegisterParser( "buffer",          &Generator::ProcessBuffer );

    RegisterParser( "string",          &Generator::ProcessString );
    RegisterParser( "stringInline",    &Generator::ProcessStringInline );
    RegisterParser( "wstring",         &Generator::ProcessWString );
    RegisterParser( "wstringInline",   &Generator::ProcessWStringInline );
    RegisterParser( "token",           &Generator::ProcessToken );
    RegisterParser( "tokenInline",     &Generator::ProcessTokenInline );

    RegisterParser( "object",          &Generator::ProcessObject );
    RegisterParser( "objectInline",    &Generator::ProcessObjectInline );
    RegisterParser( "objectEx",        &Generator::ProcessObjectEx );

    RegisterParser( "tuple",           &Generator::ProcessTuple );
    RegisterParser( "tupleInline",     &Generator::ProcessTupleInline );
    RegisterParser( "list",            &Generator::ProcessList );
    RegisterParser( "listInline",      &Generator::ProcessListInline );
    RegisterParser( "listInt",         &Generator::ProcessListInt );
    RegisterParser( "listLong",        &Generator::ProcessListLong );
    RegisterParser( "listStr",         &Generator::ProcessListStr );
    RegisterParser( "dict",            &Generator::ProcessDict );
    RegisterParser( "dictInline",      &Generator::ProcessDictInline );
    RegisterParser( "dictRaw",         &Generator::ProcessDictRaw );
    RegisterParser( "dictInt",         &Generator::ProcessDictInt );
    RegisterParser( "dictStr",         &Generator::ProcessDictStr );

    RegisterParser( "substreamInline", &Generator::ProcessSubStreamInline );
    RegisterParser( "substructInline", &Generator::ProcessSubStructInline );
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

