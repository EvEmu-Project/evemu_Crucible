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

#include "EncodeGenerator.h"

ClassEncodeGenerator::ClassEncodeGenerator( FILE* outputFile )
: Generator( outputFile ),
  mItemNumber( 0 ),
  mName( NULL )
{
    RegisterProcessors();
}

bool ClassEncodeGenerator::ProcessElementDef( const TiXmlElement* field )
{
    mName = field->Attribute( "name" );
    if( mName == NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    const TiXmlElement* main = field->FirstChildElement();
    if( main->NextSiblingElement() != NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d contains more than one root element. skipping.", field->Row() );
        return false;
    }

    const char* encode_type = GetEncodeType( main );
    fprintf( mOutputFile,
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
    if( !ParseElement( main ) )
        return false;

    fprintf( mOutputFile,
		"    return res;\n"
		"}\n"
		"\n"
	);

	return true;
}

bool ClassEncodeGenerator::ProcessElement( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    %s = %s.Encode();\n"
		"\n",
		top(), name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessElementPtr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    const char* v = top();
    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessRaw( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* v = top();
    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessInt( const TiXmlElement* field )
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
        fprintf( mOutputFile,
			"    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf( mOutputFile,
	    "        %s = new PyInt( %s );\n"
		"\n",
	    v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessLong( const TiXmlElement* field )
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
        fprintf( mOutputFile,
		    "    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf( mOutputFile,
		"        %s = new PyLong( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessReal( const TiXmlElement* field )
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
        fprintf( mOutputFile,
			"    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf( mOutputFile,
		"        %s = new PyFloat( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessBool( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"        %s = new PyBool( %s );\n"
		"\n",
		top(), name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessNone( const TiXmlElement* field )
{
    fprintf( mOutputFile,
		"    %s = new PyNone;\n"
		"\n",
		top()
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessBuffer( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

	const char* v = top();
    fprintf( mOutputFile,
        "    if( NULL == %s )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty buffer.\");\n"
        "        %s = new PyBuffer( 0 );\n"
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

bool ClassEncodeGenerator::ProcessString( const TiXmlElement* field )
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
        fprintf( mOutputFile,
			"    if( %s == \"%s\" )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
		);

    fprintf( mOutputFile,
		"        %s = new PyString( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessStringInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "String element at line %d has no value attribute.", field->Row() );
        return false;
    }

    const char* v = top();
    fprintf( mOutputFile,
        "    %s = new PyString( \"%s\" );\n"
        "\n",
        v, value
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessWString( const TiXmlElement* field )
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
        fprintf( mOutputFile,
			"    if( %s == \"%s\" )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
		);

    fprintf( mOutputFile,
		"        %s = new PyWString( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "WString element at line %d has no value attribute.", field->Row() );
        return false;
    }

    const char* v = top();
    fprintf( mOutputFile,
        "    %s = new PyWString( \"%s\", %lu );\n"
        "\n",
        v, value, strlen( value )
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessToken( const TiXmlElement* field )
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
        optional = str2<bool>( optional_str );

    const char* v = top();
    if( optional )
        fprintf( mOutputFile,
            "    if( %s == NULL )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
	    );
    else
        fprintf( mOutputFile,
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

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "Token element at line %d has no type attribute.", field->Row() );
        return false;
    }

    const char* v = top();
    fprintf( mOutputFile,
        "    %s = new PyToken( \"%s\" );\n"
        "\n",
        v, value
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessObject( const TiXmlElement* field )
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
		optional = str2<bool>( optional_str );

    const char* v = top();
    if( optional )
        fprintf( mOutputFile,
            "    if( NULL == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
        );
    else
        fprintf( mOutputFile,
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

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessObjectInline( const TiXmlElement* field )
{
    const uint32 num = mItemNumber++;

    char tname[32];
    snprintf( tname, sizeof( tname ), "obj%u_type", num );

    char aname[32];
    snprintf( aname, sizeof( aname ), "obj%u_args", num );

    fprintf( mOutputFile,
        "    PyString* %s = NULL;\n"
        "    PyRep* %s = NULL;\n"
        "\n",
        tname,
        aname
    );

    push( aname );
    push( tname );

    if( !ParseElementChildren( field, 2 ) )
        return false;

    const char* v = top();
    fprintf( mOutputFile,
        "    %s = new PyObject(\n"
        "        %s, %s );\n"
        "\n",
        v,
            tname, aname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessObjectEx( const TiXmlElement* field )
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
		optional = str2<bool>( optional_str );

	const char *v = top();
	if( optional )
		fprintf( mOutputFile,
			"    if( %s == NULL )\n"
			"        %s = new PyNone;\n"
			"    else\n",
			name,
				v
		);
	else
		fprintf( mOutputFile,
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

	fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessTuple( const TiXmlElement* field )
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
        optional = str2<bool>( optional_str );

	const char* v = top();
    fprintf( mOutputFile,
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
        fprintf( mOutputFile,
		    "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
		);

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessTupleInline( const TiXmlElement* field )
{
    //first, we need to know how many elements this tuple has:
    const TiXmlNode* i = NULL;

    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) )
    {
        if( i->Type() == TiXmlNode::ELEMENT )
            count++;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "tuple%u", mItemNumber++ );

    //now we can generate the tuple decl
    fprintf( mOutputFile,
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

    if( !ParseElementChildren( field ) )
        return false;

    fprintf( mOutputFile,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);


    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessList( const TiXmlElement* field )
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
        optional = str2<bool>( optional_str );

	const char* v = top();
    fprintf( mOutputFile,
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
        fprintf( mOutputFile,
            "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
			"    else\n",
            name,
                v
		);

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessListInline( const TiXmlElement* field )
{
    //first, we need to know how many elements this tuple has:
    const TiXmlNode* i = NULL;

    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) )
    {
        if( i->Type() == TiXmlNode::ELEMENT )
            count++;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "list%u", mItemNumber++ );

    //now we can generate the list decl
    fprintf( mOutputFile,
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

    if( !ParseElementChildren( field ))
        return false;

    fprintf( mOutputFile,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessListInt( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessListLong( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessListStr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessDict( const TiXmlElement* field )
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
        optional = str2<bool>( optional_str );

	const char* v = top();
    fprintf( mOutputFile,
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
        fprintf( mOutputFile,
            "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
		);

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessDictInline( const TiXmlElement* field )
{
    //first, create the dict container
    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%u", mItemNumber++ );

    fprintf( mOutputFile,
		"    PyDict* %s = new PyDict;\n"
		"\n",
		iname
	);

    //now we process each element, putting it into the dict:
    const TiXmlNode* i = NULL;

    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) )
	{
        if( i->Type() == TiXmlNode::ELEMENT )
        {
            const TiXmlElement* ele = i->ToElement();

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

            fprintf( mOutputFile,
			    "    PyRep* %s;\n"
			    "\n",
			    vname
		    );
            push( vname );

            //now process the data part, putting the value into `varname`
            if( !ParseElementChildren( ele, 1 ) )
                return false;

            //now store the result in the dict:
            //taking the keyType into account
            if( keyTypeInt )
                fprintf( mOutputFile,
			        "    %s->SetItem(\n"
                    "        new PyInt( %s ), %s\n"
                    "    );\n"
				    "\n",
				    iname,
				        key, vname
			    );
            else
                fprintf( mOutputFile,
			         "    %s->SetItemString(\n"
                     "        \"%s\", %s\n"
                     "    );\n"
				     "\n",
				     iname,
				        key, vname
			    );
        }
    }

    fprintf( mOutputFile,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessDictRaw( const TiXmlElement* field )
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

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessDictInt( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%u", mItemNumber++ );

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessDictStr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%d", mItemNumber++ );

    fprintf( mOutputFile,
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

bool ClassEncodeGenerator::ProcessSubStreamInline( const TiXmlElement* field )
{
    char varname[16];
    snprintf( varname, sizeof( varname ), "ss_%d", mItemNumber++ );

    //encode the sub-element into a temp
    fprintf( mOutputFile,
		"    PyRep* %s;\n"
		"\n",
		varname
	);

    push( varname );
    if( !ParseElementChildren( field, 1 ) )
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf( mOutputFile,
		"    %s = new PySubStream( %s );\n"
		"\n",
		top(), varname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessSubStructInline( const TiXmlElement* field )
{
    char varname[16];
    snprintf( varname, sizeof( varname ), "ss_%d", mItemNumber++ );

    //encode the sub-element into a temp
    fprintf( mOutputFile,
		"    PyRep* %s;\n"
		"\n",
		varname
	);

    push( varname );
    if( !ParseElementChildren( field, 1 ) )
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf( mOutputFile,
		"    %s = new PySubStruct( %s );\n"
		"\n",
		top(), varname
	);

    pop();
    return true;
}

















