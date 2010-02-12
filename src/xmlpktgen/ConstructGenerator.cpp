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

#include "ConstructGenerator.h"

ClassConstructGenerator::ClassConstructGenerator()
{
	AllGenProcRegs( ClassConstructGenerator );
	GenProcReg( ClassConstructGenerator, dictInlineEntry, NULL );
}

bool ClassConstructGenerator::Process_elementDef( FILE* into, TiXmlElement* element )
{
	const char* name = element->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row() );
		return false;
	}

	fprintf( into,
		"%s::%s()\n"
		"{\n",
		name, name
    );

	if( !Recurse( into, element ) )
		return false;

	fprintf( into,
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

bool ClassConstructGenerator::Process_element( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_elementPtr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_raw( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_int( FILE* into, TiXmlElement* field )
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

	fprintf( into,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::Process_long( FILE* into, TiXmlElement* field )
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

	fprintf( into,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::Process_real( FILE* into, TiXmlElement* field )
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

	fprintf( into,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::Process_bool( FILE* into, TiXmlElement* field )
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

	fprintf( into,
		"    %s = %s;\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::Process_none( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_buffer( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_string( FILE* into, TiXmlElement* field )
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

	fprintf( into,
		"    %s = \"%s\";\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::Process_stringInline( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassConstructGenerator::Process_wstring( FILE* into, TiXmlElement* field )
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

	fprintf( into,
		"    %s = \"%s\";\n",
		name, def
	);

	return true;
}

bool ClassConstructGenerator::Process_token( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
	    "    %s = NULL;\n",
	    name
    );

    return true;
}

bool ClassConstructGenerator::Process_tokenInline( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassConstructGenerator::Process_object( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
        "    %s = NULL;\n"
        "\n",
        name
    );

    return true;
}

bool ClassConstructGenerator::Process_objectInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 2 );
}

bool ClassConstructGenerator::Process_objectEx( FILE* into, TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_tuple( FILE* into, TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_tupleInline( FILE* into, TiXmlElement* field )
{
	return Recurse( into, field );
}

bool ClassConstructGenerator::Process_list( FILE* into, TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_listInline( FILE* into, TiXmlElement* field )
{
	return Recurse( into, field );
}

bool ClassConstructGenerator::Process_listInt( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_listLong( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_listStr( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_dict( FILE* into, TiXmlElement* field )
{
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( into,
		"    %s = NULL;\n",
		name
	);

	return true;
}

bool ClassConstructGenerator::Process_dictInline( FILE* into, TiXmlElement* field )
{
	return Recurse( into, field );
}

bool ClassConstructGenerator::Process_dictInlineEntry( FILE* into, TiXmlElement* field )
{
	//we dont really even care about this...
	const char* key = field->Attribute( "key" );
	if( key == NULL )
    {
		_log( COMMON__ERROR, "<dictInlineEntry> at line %d is missing the key attribute, skipping.", field->Row() );
		return false;
	}

	return Recurse( into, field, 1 );
}

bool ClassConstructGenerator::Process_dictRaw( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_dictInt( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_dictStr( FILE* into, TiXmlElement* field )
{
	return true;
}

bool ClassConstructGenerator::Process_substreamInline( FILE* into, TiXmlElement* field )
{
	return Recurse( into, field, 1 );
}

bool ClassConstructGenerator::Process_substructInline( FILE* into, TiXmlElement* field )
{
	return Recurse( into, field, 1 );
}


















