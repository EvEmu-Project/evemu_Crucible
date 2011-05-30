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

#include "ConstructGenerator.h"

ClassConstructGenerator::ClassConstructGenerator( FILE* outputFile )
: Generator( outputFile )
{
    RegisterProcessors();
}

void ClassConstructGenerator::RegisterProcessors()
{
    Generator::RegisterProcessors();

    AddMemberParser( "dictInlineEntry", &ClassConstructGenerator::ProcessDictInlineEntry );
}

bool ClassConstructGenerator::ProcessElementDef( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"%s::%s()\n"
		"{\n",
		name, name
    );

	if( !ParseElementChildren( field ) )
		return false;

	fprintf( mOutputFile,
		"}\n"
		"\n"
        "%s::%s( const %s& oth )\n"
        "{\n"
        "    *this = oth;\n"
        "}\n"
        "\n",
        name, name, name
	);

	return true;
}

bool ClassConstructGenerator::ProcessElement( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessElementPtr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessRaw( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessInt( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* def = field->Attribute( "default" );
	if( def == NULL )
		def = "0";

	fprintf( mOutputFile,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::ProcessLong( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* def = field->Attribute( "default" );
	if( def == NULL )
		def = "0";

	fprintf( mOutputFile,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::ProcessReal( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* def = field->Attribute( "default" );
	if( def == NULL )
		def = "0.0";

	fprintf( mOutputFile,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::ProcessBool( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* def = field->Attribute( "default" );
	if( def == NULL )
		def = "false";

	fprintf( mOutputFile,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::ProcessNone( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessBuffer( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessString( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* def = field->Attribute( "default" );
	if( def == NULL )
		def = "";

	fprintf( mOutputFile,
		"    %s = \"%s\";\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::ProcessStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassConstructGenerator::ProcessWString( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* def = field->Attribute( "default" );
	if( def == NULL )
		def = "";

	fprintf( mOutputFile,
		"    %s = \"%s\";\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassConstructGenerator::ProcessToken( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
	    "    %s = NULL;\n",
	    name
    );

    return true;
}

bool ClassConstructGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    return true;
}

bool ClassConstructGenerator::ProcessObject( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = NULL;\n"
        "\n",
        name
    );

    return true;
}

bool ClassConstructGenerator::ProcessObjectInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 2 );
}

bool ClassConstructGenerator::ProcessObjectEx( const TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessTuple( const TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessTupleInline( const TiXmlElement* field )
{
	return ParseElementChildren( field );
}

bool ClassConstructGenerator::ProcessList( const TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessListInline( const TiXmlElement* field )
{
	return ParseElementChildren( field );
}

bool ClassConstructGenerator::ProcessListInt( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessListLong( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessListStr( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessDict( const TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( mOutputFile,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::ProcessDictInline( const TiXmlElement* field )
{
	return ParseElementChildren( field );
}

bool ClassConstructGenerator::ProcessDictInlineEntry( const TiXmlElement* field )
{
	//we dont really even care about this...
	const char* key = field->Attribute( "key" );
	if( key == NULL )
    {
		_log( COMMON__ERROR, "<dictInlineEntry> at line %d is missing the key attribute, skipping.", field->Row() );
		return false;
	}

	return ParseElementChildren( field, 1 );
}

bool ClassConstructGenerator::ProcessDictRaw( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessDictInt( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessDictStr( const TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::ProcessSubStreamInline( const TiXmlElement* field )
{
	return ParseElementChildren( field, 1 );
}

bool ClassConstructGenerator::ProcessSubStructInline( const TiXmlElement* field )
{
	return ParseElementChildren( field, 1 );
}


















