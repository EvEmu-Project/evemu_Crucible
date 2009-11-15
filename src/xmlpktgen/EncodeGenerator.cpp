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

#include "EncodeGenerator.h"

ClassEncodeGenerator::ClassEncodeGenerator()
: mItemNumber( 0 ),
  mName( NULL )
{
    AllGenProcRegs( ClassEncodeGenerator );
}

bool ClassEncodeGenerator::Process_elementDef( FILE* into, TiXmlElement* element )
{
    mName = element->Attribute( "name" );
    if( mName == NULL )
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
        "%s* %s::Encode() const\n"
		"{\n"
        "    %s* res = NULL;\n"
		"\n",
        encode_type, mName,
		    encode_type
	);

    mItemNumber = 0;
	clear();

	push( "res" );
    if( !Recurse( into, element ) )
        return false;

    fprintf( into,
		"    return res;\n"
		"}\n"
		"\n"
	);

	return true;
}

bool ClassEncodeGenerator::Process_element( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( into,
        "    %s = %s.Encode();\n"
		"\n",
		top(), name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_elementPtr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    const char* v = top();
    fprintf( into,
        "    if( NULL == %s )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\");\n"
        "        %s = new PyNone;\n"
        "    }\n"
		"    else\n"
        "        %s = %s->Encode();\n"
		"\n",
        name,
            mName, name,
            v,

		    v, name
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_raw( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* v = top();
    fprintf( into,
        "    if( NULL == %s )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\");\n"
        "        %s = new PyNone;\n"
        "    }\n"
		"    else\n"
	    "    {\n"
	    "        %s = %s;\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"\n",
        name,
            mName, name,
            v,

			v, name,
			name
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_int( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    //this should be done better:
    const char* none_marker = field->Attribute( "none_marker" );

	const char* v = top();
    if( none_marker != NULL )
        fprintf( into,
			"    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf( into,
	    "        %s = new PyInt( %s );\n"
		"\n",
	    v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_long( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    //this should be done better:
    const char* none_marker = field->Attribute( "none_marker" );

	const char* v = top();
    if( none_marker != NULL )
        fprintf( into,
		    "    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf( into,
		"        %s = new PyLong( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_real( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    //this should be done better:
    const char* none_marker = field->Attribute( "none_marker" );

	const char* v = top();
    if( none_marker != NULL )
        fprintf( into,
			"    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf( into,
		"        %s = new PyFloat( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_bool( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( into,
		"        %s = new PyBool( %s );\n"
		"\n",
		top(), name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_none( FILE* into, TiXmlElement* field )
{
    fprintf( into,
		"    %s = new PyNone;\n"
		"\n",
		top()
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_buffer( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* v = top();
    fprintf( into,
        "    if( NULL == %s )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty buffer.\");\n"
        "        %s = new PyBuffer( (uint8*)NULL, 0 );\n"
        "    }\n"
		"    else\n"
	    "    {\n"
	    "        %s = %s;\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"\n",
        name,
            mName, name,
            v,

			v, name,
			name
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_string( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    const char* none_marker = field->Attribute( "none_marker" );

	const char* v = top();
    if( none_marker != NULL )
        fprintf( into,
			"    if( %s == \"%s\" )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
		);

    fprintf( into,
		"        %s = new PyString( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_stringInline( FILE* into, TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "String element at line %d has no type attribute.", field->Row() );
        return false;
    }

    const char* v = top();
    fprintf( into,
        "    %s = new PyString( \"%s\" );\n"
        "\n",
        v, value
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_wstring( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    const char* none_marker = field->Attribute( "none_marker" );

	const char* v = top();
    if( none_marker != NULL )
        fprintf( into,
			"    if( %s == \"%s\" )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
		);

    fprintf( into,
		"        %s = new PyWString( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_token( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if( optional_str != NULL )
        optional = atobool( optional_str );

    const char* v = top();
    if( optional )
        fprintf( into,
            "    if( %s == NULL )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
	    );
    else
        fprintf( into,
            "    if( %s == NULL )\n"
	        "    {\n"
            "        _log( NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\" );\n"
            "        %s = new PyNone;\n"
            "    }\n"
	        "    else\n",
            name,
                mName, name,
                v
        );

    fprintf( into,
	    "    {\n"
        "        %s = %s;\n"
        "        PyIncRef( %s );\n"
	    "    }\n"
	    "\n",
        v, name,
        name
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_tokenInline( FILE* into, TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "Token element at line %d has no type attribute.", field->Row() );
        return false;
    }

    const char* v = top();
    fprintf( into,
        "    %s = new PyToken( \"%s\" );\n"
        "\n",
        v, value
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_object( FILE* into, TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

	bool optional = false;
	const char* optional_str = field->Attribute( "optional" );
	if( NULL != optional_str )
		optional = atobool( optional_str );

    const char* v = top();
    if( optional )
        fprintf( into,
            "    if( NULL == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
        );
    else
        fprintf( into,
            "    if( NULL == %s )\n"
            "    {\n"
            "        _log( NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\" );\n"
            "        %s = new PyNone;\n"
            "    }\n"
            "    else\n",
            name,
                mName, name,
            v
        );

    fprintf( into,
        "    {\n"
        "        %s = %s;\n"
        "        PyIncRef( %s );\n"
        "    }\n"
        "\n",
        v, name,
        name
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_objectInline( FILE* into, TiXmlElement* field )
{
    const uint32 num = mItemNumber++;

    char tname[32];
    snprintf( tname, sizeof( tname ), "obj%u_type", num );

    char aname[32];
    snprintf( aname, sizeof( aname ), "obj%u_args", num );

    fprintf( into,
        "    PyString* %s = NULL;\n"
        "    PyRep* %s = NULL;\n"
        "\n",
        tname,
        aname
    );

    push( aname );
    push( tname );

    if( !Recurse( into, field, 2 ) )
        return false;

    const char* v = top();
    fprintf( into,
        "    %s = new PyObject(\n"
        "        %s, %s );\n"
        "\n",
        v,
            tname, aname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_objectEx( FILE* into, TiXmlElement* field )
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

	bool optional = false;
	const char* optional_str = field->Attribute( "optional" );
	if( optional_str != NULL )
		optional = atobool( optional_str );

	const char *v = top();
	if( optional )
		fprintf( into,
			"    if( %s == NULL )\n"
			"        %s = new PyNone;\n"
			"    else\n",
			name,
				v
		);
	else
		fprintf( into,
			"    if( %s == NULL )\n"
			"    {\n"
			"        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\");\n"
			"        %s = new PyNone;\n"
			"    }\n"
			"    else\n",
			name,
				mName, name,
				v
		);

	fprintf( into,
		"    {\n"
		"        %s = %s;\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"\n",
		v, name,
		name
	);

	pop();
    return true;
}

bool ClassEncodeGenerator::Process_tuple( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if( optional_str != NULL )
        optional = atobool( optional_str );

	const char* v = top();
    fprintf( into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty tuple.\");\n"
        "        %s = new PyTuple( 0 );\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            v
    );

    if( optional )
        fprintf( into,
		    "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
		);

    fprintf( into,
		"    {\n"
        "        %s = %s;\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"\n",
        v, name,
        name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_tupleInline( FILE* into, TiXmlElement* field )
{
    //first, we need to know how many elements this tuple has:
    TiXmlNode* i = NULL;

    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) )
    {
        if( i->Type() == TiXmlNode::ELEMENT )
            count++;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "tuple%u", mItemNumber++ );

    //now we can generate the tuple decl
    fprintf( into,
		"    PyTuple* %s = new PyTuple( %u );\n"
		"\n",
		iname, count
	);

    //now we need to queue up all the storage locations for the fields
    //need to be backward
    char varname[64];
    while( count-- > 0 )
	{
        snprintf( varname, sizeof( varname ), "%s->items[ %u ]", iname, count );
        push( varname );
    }

    if( !Recurse( into, field ) )
        return false;

    fprintf( into,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);


    pop();
    return true;
}

bool ClassEncodeGenerator::Process_list( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    bool optional = false;
    const char* optional_str = field->Attribute( "optional" );
    if( optional_str != NULL )
        optional = atobool( optional_str );

	const char* v = top();
    fprintf( into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty list.\");\n"
        "        %s = new PyList;\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            v
    );

    if( optional )
        fprintf( into,
            "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
			"    else\n",
            name,
                v
		);

    fprintf( into,
		"    {\n"
        "        %s = %s;\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"\n",
		v, name,
        name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_listInline( FILE* into, TiXmlElement* field )
{
    //first, we need to know how many elements this tuple has:
    TiXmlNode* i = NULL;
    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) )
    {
        if( i->Type() == TiXmlNode::ELEMENT )
            count++;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "list%u", mItemNumber++ );

    //now we can generate the list decl
    fprintf( into,
		"    PyList* %s = new PyList( %u );\n"
		"\n",
		iname, count
	);

    //now we need to queue up all the storage locations for the fields
    //need to be backward
    char varname[64];
    while( count-- > 0 )
	{
        snprintf( varname, sizeof( varname ), "%s->items[ %u ]", iname, count );
        push( varname );
    }

    if( !Recurse( into, field ))
        return false;

    fprintf( into,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_listInt( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( into,
        "    PyList* %s = new PyList;\n"
        "    std::vector<int32>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->AddItemInt(\n"
        "            *%s_cur\n"
        "        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name, name,
		name, name,
		name, name,
        name, name, name,
        rname,
		    name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_listLong( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( into,
        "    PyList *%s = new PyList;\n"
        "    std::vector<int64>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->AddItemLong(\n"
        "            *%s_cur\n"
        "        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name, name,
		name, name,
		name, name,
        name, name, name,
        rname,
		    name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_listStr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( into,
        "    PyList* %s = new PyList;\n"
        "    std::vector<std::string>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->AddItemString(\n"
		"            %s_cur->c_str()\n"
        "        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name, name,
		name, name,
		name, name,
        name, name, name,
        rname,
		    name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_dict( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if( optional_str != NULL )
        optional = atobool( optional_str );

	const char* v = top();
    fprintf( into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty dict.\");\n"
        "        %s = new PyDict;\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            v
    );

    if( optional )
        fprintf( into,
            "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
		);

    fprintf( into,
		"    {\n"
        "        %s = %s;\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"\n",
        v, name,
        name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_dictInline( FILE* into, TiXmlElement* field )
{
    //first, create the dict container
    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%u", mItemNumber++ );

    fprintf( into,
		"    PyDict* %s = new PyDict;\n"
		"\n",
		iname
	);

    //now we process each element, putting it into the dict:
    uint32 count = 0;
    TiXmlNode* i = NULL;
    while( ( i = field->IterateChildren( i ) ) )
	{
        if( i->Type() == TiXmlNode::COMMENT )
        {
            TiXmlComment* com = i->ToComment();
            fprintf( into, "\t/* %s */\n", com->Value() );

            continue;
        }
        else if( i->Type() == TiXmlNode::ELEMENT )
        {
            TiXmlElement* ele = i->ToElement();

            //we only handle dictInlineEntry elements
            if( strcmp( ele->Value(), "dictInlineEntry" ) != 0 )
            {
                _log( COMMON__ERROR, "non-dictInlineEntry in <dictInline> at line %d, ignoring.", ele->Row() );
                continue;
            }
		    const char* key = ele->Attribute( "key" );
		    if( key == NULL )
            {
			    _log( COMMON__ERROR, "<dictInlineEntry> at line %d lacks a key attribute", ele->Row() );
			    return false;
		    }

            bool keyTypeInt = false;
            const char* keyType = ele->Attribute( "key_type" );
            if( keyType != NULL )
                keyTypeInt = ( strcmp( keyType, "int" ) == 0 );

            char vname[32];
            snprintf( vname, sizeof( vname ), "%s_%u", iname, count );
            ++count;

            fprintf( into,
			    "    PyRep* %s;\n"
			    "\n",
			    vname
		    );
            push( vname );

            //now process the data part, putting the value into `varname`
            if( !Recurse( into, ele, 1 ) )
                return false;

            //now store the result in the dict:
            //taking the keyType into account
            if( keyTypeInt )
                fprintf( into,
			        "    %s->SetItem(\n"
                    "        new PyInt( %s ), %s\n"
                    "    );\n"
				    "\n",
				    iname,
				        key, vname
			    );
            else
                fprintf( into,
			         "    %s->SetItemString(\n"
                     "        \"%s\", %s\n"
                     "    );\n"
				     "\n",
				     iname,
				        key, vname
			    );
        }
    }

    fprintf( into,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_dictRaw( FILE* into, TiXmlElement* field )
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

    char rname[16];
    snprintf( rname, sizeof( rname ), "dict%u", mItemNumber++ );

    fprintf( into,
        "    PyDict* %s = new PyDict;\n"
        "    std::map<%s, %s>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->SetItem(\n"
        "            new Py%s( %s_cur->first ), new Py%s( %s_cur->second )\n"
		"        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        key, value, name, name,
        name, name,
        name, name,
        name, name, name,
            rname,
                pykey, name, pyvalue, name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_dictInt( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%u", mItemNumber++ );

    fprintf( into,
        "    PyDict* %s = new PyDict;\n"
        "    std::map<int32, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "    {\n"
        "        PyIncRef( %s_cur->second );\n"
        "\n"
        "        %s->SetItem(\n"
        "            new PyInt( %s_cur->first ), %s_cur->second\n"
		"        );\n"
        "    }\n"
        "    %s = %s;\n"
        "\n",
        iname,
        name, name,
        name, name,
        name, name,
        name, name, name,
            name,

            iname,
                name, name,

        top(), iname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_dictStr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%d", mItemNumber++ );

    fprintf( into,
        "    PyDict* %s = new PyDict;\n"
        "    std::map<std::string, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "    {\n"
        "        PyIncRef( %s_cur->second );\n"
        "\n"
        "        %s->SetItemString(\n"
		"            %s_cur->first.c_str(), %s_cur->second\n"
		"        );\n"
        "    }\n"
        "    %s = %s;\n"
        "\n",
        iname,
        name, name,
        name, name,
        name, name,
        name, name, name,
            name,

            iname,
                name, name,

        top(), iname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_substreamInline( FILE* into, TiXmlElement* field )
{
    char varname[16];
    snprintf( varname, sizeof( varname ), "ss_%d", mItemNumber++ );

    //encode the sub-element into a temp
    fprintf( into,
		"    PyRep* %s;\n"
		"\n",
		varname
	);

    push( varname );
    if( !Recurse( into, field, 1 ) )
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf( into,
		"    %s = new PySubStream( %s );\n"
		"\n",
		top(), varname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_substructInline( FILE* into, TiXmlElement* field )
{
    char varname[16];
    snprintf( varname, sizeof( varname ), "ss_%d", mItemNumber++ );

    //encode the sub-element into a temp
    fprintf( into,
		"    PyRep* %s;\n"
		"\n",
		varname
	);

    push( varname );
    if( !Recurse( into, field, 1 ) )
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf( into,
		"    %s = new PySubStruct( %s );\n"
		"\n",
		top(), varname
	);

    pop();
    return true;
}

















