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

#include "HeaderGenerator.h"

ClassHeaderGenerator::ClassHeaderGenerator()
{
    AllGenProcRegs( ClassHeaderGenerator );
    GenProcReg( ClassHeaderGenerator, dictInlineEntry, NULL );
}

bool ClassHeaderGenerator::Process_elementDef( FILE* into, TiXmlElement* element )
{
    const char* name = element->Attribute("name");
    if( name == NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row() );
        return false;
    }

    TiXmlElement* main = element->FirstChildElement();
    if( main->NextSiblingElement() != NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d contains more than one root element. skipping.", element->Row() );
        return false;
    }

    const char* encode_type = GetEncodeType( element );
    fprintf( into,
		"class %s\n"
		"{\n"
        "public:\n"
        "    %s();\n"
        "    %s( const %s& oth );\n"
        "    ~%s();\n"
        "\n"
        "    void Dump( LogType type, const char* pfx = \"\" ) const;\n"
		"\n"
        "    bool Decode( PyRep* packet );\n"
        "    bool Decode( PyRep** packet );\n"
        "    bool Decode( %s** packet );\n"
        "    %s* Encode() const;\n"
        "    %s* FastEncode();\n"
		"\n"
        "    %s& operator=( const %s& oth );\n"
        "\n",
        name,

        name,
        name, name,
        name,

            encode_type,
        encode_type,
        encode_type,

        name, name
    );

    if( !Recurse( into, element, 1 ) )
        return false;

    fprintf( into,
		"};\n"
		"\n"
	);

    ClearNames();

	return true;
}

bool ClassHeaderGenerator::Process_element( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}
    const char* type = field->Attribute( "type" );
    if( type == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row() );
        return false;
    }

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    %s\t%s;\n",
		type, name
	);

    return true;
}

bool ClassHeaderGenerator::Process_elementPtr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}
    const char* type = field->Attribute( "type" );
    if( type == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row() );
        return false;
    }

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    %s*\t%s;\n",
		type, name
	);

    return true;
}

bool ClassHeaderGenerator::Process_raw( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    PyRep*\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_int( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    int32\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_long( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    int64\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_real( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    double\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_bool( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    bool\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_none( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::Process_buffer( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    PyBuffer*\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_string( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::string\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_stringInline( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::Process_wstring( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::string\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_token( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
	    "    PyToken*\t\t%s;\n",
	    name
    );

    return true;
}

bool ClassHeaderGenerator::Process_tokenInline( FILE* into, TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::Process_object( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( into,
        "    PyObject*\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::Process_objectInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 2 );
}

bool ClassHeaderGenerator::Process_objectEx( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}
	const char* type = field->Attribute( "type" );
	if( type == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the type attribute.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

	fprintf( into,
		"    %s*\t%s;\n",
		type, name
	);

    return true;
}

bool ClassHeaderGenerator::Process_tuple( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    PyTuple*\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_tupleInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field );
}

bool ClassHeaderGenerator::Process_list( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    PyList*\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_listInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field );
}

bool ClassHeaderGenerator::Process_listInt( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::vector<int32>\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_listLong( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::vector<int64>\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_listStr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::vector<std::string>\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_dict( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    PyDict*\t\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_dictInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field );
}

bool ClassHeaderGenerator::Process_dictInlineEntry( FILE* into, TiXmlElement* field )
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

bool ClassHeaderGenerator::Process_dictRaw( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* key = field->Attribute( "key" );
	if( key == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the key attribute, skipping.", field->Row() );
		return false;
	}
	const char* pykey = field->Attribute( "pykey" );
	if( pykey == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the pykey attribute, skipping.", field->Row() );
		return false;
	}
	const char* value = field->Attribute( "value" );
	if( value == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the value attribute, skipping.", field->Row() );
		return false;
	}
	const char* pyvalue = field->Attribute( "pyvalue" );
	if( pyvalue == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the pyvalue attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::map<%s, %s>\t%s;\n",
		key, value, name
	);

    return true;
}

bool ClassHeaderGenerator::Process_dictInt( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::map<int32, PyRep*>\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_dictStr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    if( !RegisterName( name, field->Row() ) )
        return false;

    fprintf( into,
		"    std::map<std::string, PyRep*>\t%s;\n",
		name
	);

    return true;
}

bool ClassHeaderGenerator::Process_substreamInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 1 );
}

bool ClassHeaderGenerator::Process_substructInline( FILE* into, TiXmlElement* field )
{
    return Recurse( into, field, 1 );
}





















