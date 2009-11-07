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

#include "DecodeGenerator.h"

#ifndef WIN32
#warning Decoder is not properly freeing old items in the case of object re-use
#endif

ClassDecodeGenerator::ClassDecodeGenerator()
: mName( NULL ),
  mItemNumber( 0 )
{
	AllGenProcRegs( ClassDecodeGenerator );
}

bool ClassDecodeGenerator::Process_elementDef( FILE* into, TiXmlElement* element )
{
	mName = element->Attribute( "name" );
	if( mName == NULL )
    {
		_log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row() );
		return false;
	}

	fprintf( into,
        "bool %s::Decode( PyRep* packet )\n"
        "{\n",
		mName
    );

	mItemNumber = 0;

	push( "packet" );

	if( !Recurse( into, element ) )
		return false;

	fprintf( into,
		"	return true;\n"
		"}\n"
		"\n"
		"bool %s::Decode( PyRep** packet )\n"
		"{\n"
        "    bool res = Decode( *packet );\n"
        "\n"
        "    PyDecRef( *packet );\n"
        "    *packet = NULL;\n"
        "\n"
        "    return res;\n"
        "}\n"
		"\n"
		"bool %s::Decode( %s** packet )\n"
		"{\n"
		"    //quick forwarder to avoid making the user cast it if they have a properly typed object\n"
        "    return Decode( (PyRep**)packet );\n"
		"}\n"
		"\n",
        mName,
        mName, GetEncodeType( element )
	);

	return true;
}

bool ClassDecodeGenerator::Process_element( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}

	const char* v = top();
	fprintf( into,
		"    if( !%s.Decode( %s ) )\n"
        "    {\n"
		"		_log( NET__PACKET_ERROR, \"Decode %s failed: unable to decode element %s\" );\n"
        "\n"
		"		return false;\n"
		"    }\n"
		"\n",
		name, v,
			mName, name
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_elementPtr( FILE* into, TiXmlElement* field )
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

	const char* v = top();
	fprintf( into,
        "    SafeDelete( %s );\n"
		"    %s = new %s;\n"
        "\n"
		"    if( !%s->Decode( %s ) )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: unable to decode element %s\" );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
		name,
		name, type,

		name, v,
			mName, name
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_raw( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* v = top();
	fprintf( into,
        "    PySafeDecRef( %s );\n"
		"    %s = %s;\n"
        "    PyIncRef( %s );\n"
		"\n",
		name,
		name, v,
		name
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_int( FILE* into, TiXmlElement* field )
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
			"    if( %s->IsNone() )\n"
			"        %s = %s;\n"
			"    else\n",
			v,
				name, none_marker
        );

	fprintf( into,
		"    if( %s->IsInt() )\n"
        "        %s = %s->AsInt()->value();\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not an int: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
        "    }\n"
		"\n",
        v,
            name, v,

            mName, name, v
    );

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_long( FILE* into, TiXmlElement* field )
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
			"    if( %s->IsNone() )\n"
			"        %s = %s;\n"
			"    else\n",
			v,
				name, none_marker
        );

	fprintf( into,
		"    if( %s->IsLong() )\n"
        "        %s = %s->AsLong()->value();\n"
        "    else if( %s->IsInt() )\n"
        "        %s = %s->AsInt()->value();\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a long int: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "\n",
		v,
			name, v,
		v,
			name, v,

			mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_real( FILE* into, TiXmlElement* field )
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
			"    if( %s->IsNone() )\n"
			"        %s = %s;\n"
			"    else\n",
			v,
				name, none_marker
        );

	fprintf( into,
		"    if( %s->IsFloat())\n"
        "        %s = %s->AsFloat()->value();\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a real: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
        v,
            name, v,

            mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_bool( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    bool soft = false;
    const char* soft_str = field->Attribute( "soft" );
    if( soft_str != NULL )
        soft = atobool( soft_str );

	const char* none_marker = field->Attribute( "none_marker" );

    const char* v = top();
	if( none_marker != NULL )
		fprintf( into,
			"    if( %s->IsNone() )\n"
			"        %s = %s;\n"
			"    else\n",
			v,
				name, none_marker
        );

    fprintf( into,
        "    if( %s->IsBool() )\n"
        "        %s = %s->AsBool()->value();\n"
        "    else\n",
        v,
            name, v
    );

	if( soft )
	    fprintf(into,
            "    if( %s->IsInt() )\n"
            "        %s = ( %s->AsInt()->value() != 0 );\n"
            "    else\n",
	        v,
		        name, v
        );

    fprintf( into,
        "    {\n"
        "		_log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a boolean: %%s\", %s->TypeString() );\n"
        "\n"
        "		return false;\n"
        "	}\n"
        "\n",
	    mName, name, v
    );

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_none( FILE* into, TiXmlElement* field )
{
	const char* v = top();
	fprintf( into,
		"    if( !%s->IsNone() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: expecting a None but got a %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
		v,
            mName, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_string( FILE* into, TiXmlElement* field )
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
			"    if( %s->IsNone() )\n"
			"        %s = \"%s\";\n"
			"    else\n",
			v,
				name, none_marker
        );


    fprintf( into,
		"    if( %s->IsString() )\n"
		"        %s = %s->AsString()->content();\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a string: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
        v,
            name, v,

            mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_stringInline( FILE* into, TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "String element at line %d has no value attribute.", field->Row() );
        return false;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "string_%u", mItemNumber++ );

    const char* v = top();
    fprintf( into,
        "    if( !%s->IsString() )\n"
        "    {\n"
        "        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a string: %%s\", %s->TypeString() );\n"
        "\n"
        "        return false;\n"
        "    }\n"
        "    PyString* %s = %s->AsString();\n"
        "\n"
        "    if( %s->content() != \"%s\" )\n"
        "    {\n"
        "        _log( NET__PACKET_ERROR, \"Decode %s failed: expected %s to be '%s', but it's '%%s'\", %s->content().c_str() );\n"
        "\n"
        "        return false;\n"
        "    }\n"
        "\n",
        v,
            mName, iname, v,
        iname, v,

        iname, value,
            mName, iname, value, iname
    );

    pop();
    return true;
}

bool ClassDecodeGenerator::Process_buffer( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}

	const char* v = top();
	fprintf( into,
        "    PySafeDecRef( %s );\n"
		"    if( %s->IsBuffer() )\n"
        "    {\n"
        "        %s = %s->AsBuffer();\n"
        "        PyIncRef( %s );\n"
		"    }\n"
        "    else if( %s->IsString() )\n"
		"        %s = new PyBuffer( *%s->AsString() );\n"
		"    else\n"
        "    {\n"
		"        _log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a buffer: %%s\", %s->TypeString());\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
        name,
		v,
			name, v,
			name,
		v,
			name, v,

			mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_token( FILE* into, TiXmlElement* field )
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

    fprintf( into,
        "    PySafeDecRef( %s );\n",
        name
    );

    const char* v = top();
    if( optional )
	    fprintf( into,
		    "    if( %s->IsNone() )\n"
		    "        %s = NULL;\n"
		    "    else\n",
		    v,
			    name
        );

    fprintf( into,
	    "    if( %s->IsToken() )\n"
        "    {\n"
        "        %s = %s->AsToken();\n"
        "        PyIncRef( %s );\n"
	    "    }\n"
        "    else\n"
        "    {\n"
	    "        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a token: %%s\", %s->TypeString() );\n"
        "\n"
	    "        return false;\n"
        "    }\n"
	    "\n",
        v,
            name, v,
            name,

            mName, name, v
    );

    pop();
    return true;
}

bool ClassDecodeGenerator::Process_tokenInline( FILE* into, TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "Token element at line %d has no value attribute.", field->Row() );
        return false;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "token_%u", mItemNumber++ );

    const char* v = top();
    fprintf( into,
        "    if( !%s->IsToken() )\n"
        "    {\n"
        "        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a token: %%s\", %s->TypeString() );\n"
        "\n"
        "        return false;\n"
        "    }\n"
        "    PyToken* %s = %s->AsToken();\n"
        "\n"
        "    if( %s->content() != \"%s\" )\n"
        "    {\n"
        "        _log( NET__PACKET_ERROR, \"Decode %s failed: expected %s to be '%s', but it's '%%s'\", %s->content().c_str() );\n"
        "\n"
        "        return false;\n"
        "    }\n"
        "\n",
        v,
            mName, iname, v,
        iname, v,

        iname, value,
            mName, iname, value, iname
    );

    pop();
    return true;
}

bool ClassDecodeGenerator::Process_object( FILE* into, TiXmlElement* field )
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

    fprintf( into,
        "    PySafeDecRef( %s );\n",
        name
    );

	const char* v = top();
    if( optional )
    {
        fprintf( into,
            "    if( %s->IsNone() )\n"
            "        %s = NULL;\n"
            "    else\n",
            v,
                name
        );
    }

	//make sure its an object
	fprintf( into,
		"    if( %s->IsObject() )\n"
        "    {\n"
        "        %s = %s->AsObject();\n"
        "        PyIncRef( %s );\n"
        "    }\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
        v,
            name, v,
            name,

            mName, name, v
	);

    pop();
    return true;
}

bool ClassDecodeGenerator::Process_objectInline( FILE* into, TiXmlElement* field )
{
    char iname[16];
    snprintf( iname, sizeof( iname ), "obj_%u", mItemNumber++ );

	//make sure its an object
	const char* v = top();
	fprintf( into,
		"    if( !%s->IsObject() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyObject* %s = %s->AsObject();\n"
		"\n",
		v,
            mName, iname, v,
        iname, v
	);

    char aname[32];
    snprintf( aname, sizeof( aname ), "%s->arguments()", iname );
    push( aname );

    char tname[32];
    snprintf( tname, sizeof( tname ), "%s->type()", iname );
    push( tname );

    if( !Recurse( into, field, 2 ) )
        return false;

    pop();
    return true;
}

bool ClassDecodeGenerator::Process_objectEx( FILE* into, TiXmlElement* field )
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

    fprintf( into,
        "    PySafeDecRef( %s );\n",
        name
    );

	const char* v = top();
	if( optional )
	{
		fprintf( into,
			"    if( %s->IsNone() )\n"
			"        %s = NULL;\n"
			"    else\n",
			v,
				name
		);
	}

	fprintf( into,
		"    if( %s->IsObjectEx() )\n"
		"    {\n"
		"        %s = (%s*)%s->AsObjectEx();\n"
        "        PyIncRef( %s );\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString() );\n"
		"\n"
		"        return false;\n"
		"    }\n"
        "\n",
		v,
			name, type, v,
			name,

			mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_tuple( FILE* into, TiXmlElement* field )
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

    fprintf( into,
        "    PySafeDecRef( %s );\n",
        name
    );

	const char* v = top();
	if( optional )
		fprintf( into,
			"    if( %s->IsNone() )\n"
			"        %s = NULL;\n"
			"    else\n",
			v,
				name
		);

	fprintf( into,
		"    if( %s->IsTuple() )\n"
        "    {\n"
        "        %s = %s->AsTuple();\n"
        "        PyIncRef( %s );\n"
		"    }\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a tuple: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
        "    }\n"
		"\n",
        v,
            name, v,
            name,

            mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_tupleInline( FILE* into, TiXmlElement* field )
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

	const char* v = top();
	//now we can generate the tuple decl
	fprintf( into,
		"    if( !%s->IsTuple() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyTuple* %s = %s->AsTuple();\n"
        "\n"
		"    if( %s->size() != %u )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong size: expected %d, but got %%lu\", %s->size() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
		v,
			mName, iname, v,
		iname, v,
		iname, count,
			mName, iname, count, iname
	);

	//now we need to queue up all the storage locations for the fields
    //need to be backward
	char varname[64];
	while( count-- > 0 )
    {
		snprintf( varname, sizeof( varname ), "%s->GetItem( %u )", iname, count );
		push( varname );
	}

	if( !Recurse( into, field ) )
		return false;

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_list( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	//this should be done better:
	bool optional = false;
	const char* optional_str = field->Attribute("optional");
	if( optional_str != NULL )
		optional = atobool( optional_str );

    fprintf( into,
        "    PySafeDecRef( %s );\n",
        name
    );

	const char* v = top();
	if( optional )
		fprintf( into,
			"    if( %s->IsNone() )\n"
			"        %s = NULL;\n"
			"    else\n",
			v,
				name
        );

	fprintf( into,
		"    if( %s->IsList() )\n"
        "    {\n"
        "        %s = %s->AsList();\n"
        "        PyIncRef( %s );\n"
		"    }\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
        "    }\n"
		"\n",
        v,
            name, v,
            name,

            mName, name, v
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_listInline( FILE* into, TiXmlElement* field )
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

	const char* v = top();
	//now we can generate the tuple decl
	fprintf( into,
		"    if( !%s->IsList() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyList* %s = %s->AsList();\n"
        "\n"
		"    if( %s->size() != %u )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong size: expected %d, but got %%lu\", %s->size() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
		"\n",
		v,
			mName, iname, v,
		iname, v,
		iname, count,
			mName, iname, count, iname
	);

	//now we need to queue up all the storage locations for the fields
    //need to be backward
	char varname[64];
	while( count-- > 0 )
    {
		snprintf( varname, sizeof( varname ), "%s->GetItem( %u )", iname, count );
		push( varname );
	}

	if( !Recurse( into, field ) )
		return false;

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_listInt( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	char iname[16];
	snprintf( iname, sizeof( iname ), "list_%u", mItemNumber++ );

	const char* v = top();
	//make sure its a list
	fprintf( into,
		"    if( !%s->IsList() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyList* %s = %s->AsList();\n"
        "\n"
		"    %s.clear();\n"
		"    PyList::const_iterator %s_cur, %s_end;\n"
		"    %s_cur = %s->begin();\n"
		"    %s_end = %s->end();\n"
		"    for( uint32 %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "    {\n"
		"        if( !(*%s_cur)->IsInt() )\n"
        "        {\n"
		"            _log(NET__PACKET_ERROR, \"Decode %s failed: Element %%u in list %s is not an integer: %%s\", %s_index, (*%s_cur)->TypeString());\n"
        "\n"
		"            return false;\n"
		"        }\n"
        "        const PyInt* t = (*%s_cur)->AsInt();\n"
        "\n"
        "        %s.push_back( t->value() );\n"
		"    }\n"
		"\n",
		v,
            mName, name, v,
		iname, v,
        name,
		iname, iname,
        iname, iname,
        iname, iname,
		iname, iname, iname, iname, iname,
		    iname,
                mName, iname, iname, iname,
            iname,
		    name
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_listLong( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}

	char iname[16];
	snprintf( iname, sizeof( iname ), "list_%u", mItemNumber++ );

	const char* v = top();
	//make sure its a list
	fprintf( into,
		"    if( !%s->IsList() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyList* %s = %s->AsList();\n"
        "\n"
		"    %s.clear();\n"
		"    PyList::const_iterator %s_cur, %s_end;\n"
		"    %s_cur = %s->begin();\n"
		"    %s_end = %s->end();\n"
		"    for( uint32 %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "    {\n"
		"        if( (*%s_cur)->IsLong() )\n"
        "        {\n"
        "            PyLong* t = (*%s_cur)->AsLong();\n"
        "\n"
        "            %s.push_back( t->value() );\n"
		"        }\n"
        "        else if( (*%s_cur)->IsInt() )\n"
        "        {\n"
        "            PyInt* t = (*%s_cur)->AsInt();\n"
        "\n"
        "            %s.push_back( t->value() );\n"
		"        }\n"
        "        else\n"
        "        {\n"
		"            _log( NET__PACKET_ERROR, \"Decode %s failed: Element %%u in list %s is not a long integer: %%s\", %s_index, (*%s_cur)->TypeString() );\n"
        "\n"
		"            return false;\n"
		"        }\n"
		"    }\n"
		"\n",
		v,
			mName, name, v,
		iname, v,
		name,
		iname, iname,
		iname, iname,
		iname, iname,
		iname, iname, iname, iname, iname,
		iname,
			iname,
			name,
		iname,
			iname,
			name,

			mName, iname, iname, iname
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_listStr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}

	char iname[16];
	snprintf( iname, sizeof( iname ), "list_%u", mItemNumber++ );

	const char* v = top();
	//make sure its a list
	fprintf( into,
		"    if( !%s->IsList() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyList* %s = %s->AsList();\n"
        "\n"
		"    %s.clear();\n"
		"    PyList::const_iterator %s_cur, %s_end;\n"
		"    %s_cur = %s->begin();\n"
		"    %s_end = %s->end();\n"
		"    for( uint32 %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "    {\n"
		"        if( !(*%s_cur)->IsString() )\n"
        "        {\n"
		"            _log( NET__PACKET_ERROR, \"Decode %s failed: Element %%u in list %s is not a string: %%s\", %s_index, (*%s_cur)->TypeString() );\n"
        "\n"
		"            return false;\n"
		"        }\n"
        "        const PyString* t = (*%s_cur)->AsString();\n"
        "\n"
		"        %s.push_back( t->content() );\n"
		"	}\n"
		"\n",
		v,
            mName, name, v,
		iname, v,
        name,
		iname, iname,
        iname, iname,
        iname, iname,
		iname, iname, iname, iname, iname,
            iname,
                mName, iname, iname, iname,
            iname,
            name
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_dict( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	//this should be done better:
	bool optional = false;
	const char* optional_str = field->Attribute( "optional" );
	if( optional_str != NULL )
		optional = atobool( optional_str );

    fprintf( into,
        "    PySafeDecRef( %s );\n",
        name
    );

	const char* v = top();
	if( optional )
		fprintf( into,
			"    if( %s->IsNone() )\n"
			"        %s = NULL;\n"
			"    else\n",
			v,
				name
        );

	fprintf( into,
		"    if( %s->IsDict() )\n"
        "    {\n"
        "        %s = %s->AsDict();\n"
        "        PyIncRef( %s );\n"
		"    }\n"
        "    else\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
        "    }\n"
		"\n",
        v,
            name, v,
            name,

            mName, name, v
	);

	pop();
	return true;
}

/*
 * this function could be improved quite a bit by not parsing and checking the
 * dictInlineEntry elements over and over again
 */
bool ClassDecodeGenerator::Process_dictInline( FILE* into, TiXmlElement* field )
{
    char iname[16];
	snprintf( iname, sizeof( iname ), "dict%u", mItemNumber++ );

    bool soft = false;
    const char* soft_str = field->Attribute( "soft" );
    if( soft_str != NULL )
        soft = atobool( soft_str );

	const char* v = top();
	//make sure its a dict
	fprintf( into,
		"    if( !%s->IsDict() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyDict* %s = %s->AsDict();\n"
		"\n",
		v,
            mName, iname, v,
		iname, v
	);

	//now generate the "found" flags for each expected element.
	bool empty = true;
    uint32 count = 0;
	TiXmlNode* i = NULL;
	while( ( i = field->IterateChildren( i ) ) )
    {
		if( i->Type() == TiXmlNode::ELEMENT )
        {
		    TiXmlElement* ele = i->ToElement();

		    //we only handle dictInlineEntry elements
		    if( strcmp( ele->Value(), "dictInlineEntry" ) != 0 )
            {
			    _log(COMMON__ERROR, "non-dictInlineEntry in <dictInline> at line %d, ignoring.", ele->Row());
			    continue;
		    }

		    fprintf( into,
			    "    bool %s_%u = false;\n",
                iname, count
            );
            ++count;

		    empty = false;
        }
	}

    fprintf( into,
        "\n"
    );

	if( empty )
		fprintf( into,
            "    // %s is empty from our perspective, not enforcing though.\n",
            iname
        );
	else
    {
		//setup the loop...
		fprintf( into,
			"    PyDict::const_iterator %s_cur, %s_end;\n"
			"    %s_cur = %s->begin();\n"
			"    %s_end = %s->end();\n"
			"    for(; %s_cur != %s_end; %s_cur++)\n"
            "    {\n"
			"        if( !%s_cur->first->IsString() )\n"
            "        {\n"
			"            _log( NET__PACKET_ERROR, \"Decode %s failed: a key in %s is the wrong type: %%s\", %s_cur->first->TypeString() );\n"
            "\n"
			"            return false;\n"
			"        }\n"
            "        const PyString* key_string__ = %s_cur->first->AsString();\n"
			"\n",
            iname, iname,
            iname, iname,
            iname, iname,
            iname, iname, iname,
                iname,
                    mName, iname, iname,
                iname
		);

        count = 0;
		while( ( i = field->IterateChildren( i ) ) )
        {
			if( i->Type() == TiXmlNode::COMMENT )
            {
				TiXmlComment* comm = i->ToComment();

				fprintf( into,
                    "    /* %s */\n",
                    comm->Value()
                );
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

			    //conditional prefix...
			    fprintf( into,
                    "        if( key_string__->content() == \"%s\" )\n"
                    "        {\n"
				    "            %s_%u = true;\n"
                    "\n",
				    key,
                        iname, count
                );
                ++count;

                //now process the data part, putting the value into `varname`
                //TODO: figure out a reasonable way to fix the indention here...
			    char vname[64];
			    snprintf( vname, sizeof( vname ), "%s_cur->second", iname );
			    push( vname );

			    if( !Recurse( into, ele, 1 ) )
				    return false;

			    //fixed suffix...
			    fprintf( into,
				    "        }\n"
                    "        else\n"
                );
            }
		}

		if( soft )
			fprintf( into,
				"        {\n"
                "            _log( NET__PACKET_ERROR, \"Decode %s failed: Unknown key string '%%s' in %s\", key_string__->content().c_str() );\n"
                "\n"
				"            return false;\n"
				"        }\n"
				"    }\n"
				"\n",
				mName, iname
            );
		else
			fprintf( into,
				"        {\n"
                "            /* do nothing, soft dict */\n"
				"        }\n"
                "    }\n"
				"\n"
            );

        //finally, check the "found" flags for each expected element.
        count = 0;
		while( ( i = field->IterateChildren( i ) ) )
        {
			if(i->Type() == TiXmlNode::ELEMENT)
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

                fprintf( into,
				    "    if( !%s_%u )\n"
                    "    {\n"
				    "        _log( NET__PACKET_ERROR, \"Decode %s failed: Missing dict entry '%s' in %s\" );\n"
                    "\n"
				    "        return false;\n"
				    "    }\n"
				    "\n",
				    iname, count,
                        mName, key, iname
                );
                ++count;
            }
		}
	}

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_dictRaw( FILE* into, TiXmlElement* field )
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

	char iname[16];
	snprintf( iname, sizeof( iname ), "dict_%u", mItemNumber++ );

	const char* v = top();
	fprintf( into,
		"    if( !%s->IsDict() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyDict* %s = %s->AsDict();\n"
        "\n"
		"    %s.clear();\n"
		"    PyDict::const_iterator %s_cur, %s_end;\n"
		"    %s_cur = %s->begin();\n"
		"    %s_end = %s->end();\n"
		"    for( uint32 %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "    {\n"
		"        if( !%s_cur->first->Is%s() )\n"
        "        {\n"
		"            _log( NET__PACKET_ERROR, \"Decode %s failed: Key %%u in dict %s is not %s: %%s\", %s_index, %s_cur->first->TypeString() );\n"
        "\n"
		"            return false;\n"
		"        }\n"
        "        const Py%s* k = %s_cur->first->As%s();\n"
        "\n"
		"        if( !%s_cur->second->Is%s() )\n"
        "        {\n"
		"            _log( NET__PACKET_ERROR, \"Decode %s failed: Value %%d in dict %s is not %s: %%s\", %s_index, %s_cur->second->TypeString() );\n"
        "\n"
		"            return false;\n"
		"        }\n"
        "        const Py%s *v = %s_cur->second->As%s();\n"
        "\n"
        "        %s[ k->value() ] = v->value();\n"
		"    }\n"
		"\n",
		v,
			mName, name, v,
		iname, v,
		name,
		iname, iname,
		iname, iname,
		iname, iname,
		iname, iname, iname, iname, iname,
			iname, pykey,
				mName, iname, pykey, iname, iname,
			pykey, iname, pykey,
			iname, pyvalue,
				mName, iname, pyvalue, iname, iname,
			pyvalue, iname, pyvalue,
			name
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_dictInt( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	char iname[16];
	snprintf( iname, sizeof( iname ), "dict_%u", mItemNumber++ );

	const char* v = top();
	fprintf( into,
		"    if( !%s->IsDict() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyDict* %s = %s->AsDict();\n"
        "\n"
		"    %s.clear();\n"
		"    PyDict::const_iterator %s_cur, %s_end;\n"
		"    %s_cur = %s->begin();\n"
		"    %s_end = %s->end();\n"
		"    for( uint32 %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "    {\n"
		"        if( !%s_cur->first->IsInt() )\n"
        "        {\n"
		"            _log( NET__PACKET_ERROR, \"Decode %s failed: Key %%u in dict %s is not an integer: %%s\", %s_index, %s_cur->first->TypeString() );\n"
        "\n"
		"            return false;\n"
		"        }\n"
        "        const PyInt* k = %s_cur->first->AsInt();\n"
        "\n"
        "        %s[ k->value() ] = %s_cur->second->Clone();\n"
		"    }\n"
		"\n",
		v,
			mName, name, v,
		iname, v,
		name,
		iname, iname,
		iname, iname,
		iname, iname,
		iname, iname, iname, iname, iname,
			iname,
				mName, iname, iname, iname,
			iname,
			name, iname
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_dictStr( FILE* into, TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	char iname[16];
	snprintf( iname, sizeof( iname ), "dict_%u", mItemNumber++ );

	const char* v = top();
	fprintf( into,
		"    if( !%s->IsDict() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PyDict* %s = %s->AsDict();\n"
        "\n"
		"    %s.clear();\n"
		"    PyDict::const_iterator %s_cur, %s_end;\n"
		"    %s_cur = %s->begin();\n"
		"    %s_end = %s->end();\n"
		"    for( uint32 %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "    {\n"
		"        if( !%s_cur->first->IsString() )\n"
        "        {\n"
		"            _log( NET__PACKET_ERROR, \"Decode %s failed: Key %%u in dict %s is not a string: %%s\", %s_index, %s_cur->first->TypeString() );\n"
        "\n"
		"            return false;\n"
		"        }\n"
        "        const PyString* k = %s_cur->first->AsString();\n"
        "\n"
		"        %s[ k->content() ] = %s_cur->second->Clone();\n"
		"    }\n"
		"\n",
		v,
			mName, name, v,
		iname, v,

		name,
		iname, iname,
		iname, iname,
		iname, iname,
		iname, iname, iname, iname, iname,
			iname,
				mName, iname, iname, iname,
			iname,
			name, iname
	);

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_substreamInline( FILE* into, TiXmlElement* field )
{
	char iname[16];
	snprintf( iname, sizeof( iname ), "ss_%u", mItemNumber++ );

	//make sure its a substream
	const char* v = top();
	fprintf( into,
		"    if( !%s->IsSubStream() )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a substream: %%s\", %s->TypeString() );\n"
        "\n"
		"        return false;\n"
		"    }\n"
        "    PySubStream* %s = %s->AsSubStream();\n"
		"\n"
		"    //make sure its decoded\n"
		"    %s->DecodeData();\n"
        "    if( %s->decoded() == NULL )\n"
        "    {\n"
		"        _log( NET__PACKET_ERROR, \"Decode %s failed: Unable to decode %s\" );\n"
        "\n"
		"        return false;\n"
		"	}\n"
		"\n",
		v,
            mName, iname, v,
        iname, v,
        iname,
        iname,
            mName, iname
	);

	char ssname[32];
	snprintf( ssname, sizeof( ssname ), "%s->decoded()", iname );
	push( ssname );

    //Decode the sub-element
	if( !Recurse( into, field, 1 ) )
		return false;

	pop();
	return true;
}

bool ClassDecodeGenerator::Process_substructInline( FILE* into, TiXmlElement* field )
{
	char iname[16];
	snprintf( iname, sizeof( iname ), "ss_%u", mItemNumber++ );

	//make sure its a substruct
	const char* v = top();
	fprintf( into,
		"    if( !%s->IsSubStruct() )\n"
        "    {\n"
		"		_log( NET__PACKET_ERROR, \"Decode %s failed: %s is not a substruct: %%s\", %s->TypeString() );\n"
        "\n"
		"		return false;\n"
		"	}\n"
        "	PySubStruct* %s = %s->AsSubStruct();\n"
		"\n",
		v,
			mName, iname, v,
		iname, v
	);

	char ssname[32];
	snprintf( ssname, sizeof( ssname ), "%s->sub()", iname );
	push( ssname );

    //Decode the sub-element
	if( !Recurse( into, field, 1 ) )
		return false;

	pop();
	return true;
}






















