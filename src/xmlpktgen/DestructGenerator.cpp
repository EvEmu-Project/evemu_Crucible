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

#include "DestructGenerator.h"

ClassDestructGenerator::ClassDestructGenerator()
{
    AllGenProcRegs( ClassDestructGenerator );
    GenProcReg( ClassDestructGenerator, dictInlineEntry, NULL );
}

bool ClassDestructGenerator::Process_elementDef( FILE* into, TiXmlElement* element )
{
    const char* name = element->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row() );
        return false;
    }

    fprintf( into,
        "%s::~%s()\n"
		"{\n",
        name, name
	);

    if( !Recurse( into, element ) )
        return false;

    fprintf( into,
		"}\n"
		"\n"
	);

	return true;
}

bool ClassDestructGenerator::Process_element( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_elementPtr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( into,
		"    SafeDelete( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_raw( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( into,
		"    PySafeDecRef( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_int( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_long( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_real( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_bool( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_none( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_buffer( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( into,
		"    PySafeDecRef( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_string( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_stringInline( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_wstring( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_token( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
	    "    PySafeDecRef( %s );\n",
	    name
    );

    return true;
}

bool ClassDestructGenerator::Process_tokenInline( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_object( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
        "    PySafeDecRef( %s );\n"
        "\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::Process_objectInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 2 );
}

bool ClassDestructGenerator::Process_objectEx( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

	fprintf( into,
		"    PySafeDecRef( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_tuple( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
		"    PySafeDecRef( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_tupleInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field );
}

bool ClassDestructGenerator::Process_list( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
		"    PySafeDecRef( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_listInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field );
}

bool ClassDestructGenerator::Process_listInt( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_listLong( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_listStr( FILE* into, TiXmlElement* field ) {
    return true;
}

bool ClassDestructGenerator::Process_dict( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
		"    PySafeDecRef( %s );\n",
		name
	);

    return true;
}

bool ClassDestructGenerator::Process_dictInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field );
}

bool ClassDestructGenerator::Process_dictInlineEntry( FILE* into, TiXmlElement* field )
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

bool ClassDestructGenerator::Process_dictRaw( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::Process_dictInt( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
        "    std::map<int32, PyRep*>::iterator %s_cur, %s_end;\n"
        "    //free any existing elements first\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
		"        PyDecRef( %s_cur->second );\n"
        "\n",
        name, name,
		name, name,
		name, name,
        name, name, name,
        name
    );

    return true;
}

bool ClassDestructGenerator::Process_dictStr( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
        "    std::map<std::string, PyRep*>::iterator %s_cur, %s_end;\n"
        "    //free any existing elements first\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
		"        PyDecRef( %s_cur->second );\n"
        "\n",
        name, name,
		name, name,
		name, name,
        name, name, name,
        name
    );

    return true;
}

bool ClassDestructGenerator::Process_substreamInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 1 );
}

bool ClassDestructGenerator::Process_substructInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 1 );
}


















