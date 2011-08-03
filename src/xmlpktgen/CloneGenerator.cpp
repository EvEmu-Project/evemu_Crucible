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

#include "CloneGenerator.h"

ClassCloneGenerator::ClassCloneGenerator( FILE* outputFile )
: Generator( outputFile )
{
    RegisterProcessors();
}

void ClassCloneGenerator::RegisterProcessors()
{
    Generator::RegisterProcessors();

    AddMemberParser( "dictInlineEntry", &ClassCloneGenerator::ProcessDictInlineEntry );
}

bool ClassCloneGenerator::ProcessElementDef( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "%s& %s::operator=( const %s& oth )\n"
		"{\n",
        name, name, name
    );

    if( !ParseElementChildren( field ) )
        return false;

    fprintf( mOutputFile,
        "    return *this;\n"
        "}\n"
        "\n"
	);

	return true;
}

bool ClassCloneGenerator::ProcessElement( const TiXmlElement* field )
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

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessElementPtr( const TiXmlElement* field )
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

    fprintf( mOutputFile,
        "    SafeDelete( %s );\n"
        "    if( oth.%s == NULL )\n"
            //TODO: log an error
        "        %s = NULL;\n"
        "    else\n"
        "        %s = new %s( *oth.%s );\n"
        "\n",
        name,
        name,
            name,

            name, type, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "   PySafeDecRef( %s );\n"
        "   if( oth.%s == NULL )\n"
        "       %s = NULL;\n"  //TODO: log an error
        "   else\n"
        "       %s = oth.%s->Clone();\n"
        "\n",
        name,
        name,
            name,
            name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessLong( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessReal( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessBool( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessNone( const TiXmlElement* field )
{
    return true;
}

bool ClassCloneGenerator::ProcessBuffer( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    PySafeDecRef( %s );\n"
        "    if( oth.%s == NULL )\n"
        "        %s = NULL;\n" //TODO: log an error
        "    else\n"
		"        %s = new PyBuffer( *oth.%s );\n"
        "\n",
        name,
        name,
            name,

            name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessString( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassCloneGenerator::ProcessWString( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassCloneGenerator::ProcessToken( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "   PySafeDecRef( %s );\n"
        "   if( oth.%s == NULL )\n"
        "       %s = NULL;\n"  //TODO: log an error
        "   else\n"
        "       %s = oth.%s->Clone();\n"
        "\n",
        name,
        name,
            name,
            name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    return true;
}

bool ClassCloneGenerator::ProcessObject( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    PySafeDecRef( %s );\n"
        "    if( NULL == oth.%s )\n"
        "        %s = NULL;\n"
        "    else\n"
        "        %s = new PyObject( *oth.%s );\n"
        "\n",
        name,
        name,
            name,

            name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessObjectInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 2 );
}

bool ClassCloneGenerator::ProcessObjectEx( const TiXmlElement* field )
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

	fprintf( mOutputFile,
		"    PySafeDecRef( %s );\n"
		"    if( oth.%s == NULL )\n"
		"        %s = NULL;\n"
		"    else\n"
		"        %s = new %s( *oth.%s );\n",
		name,
		name,
			name,

			name, type, name
	);

    return true;
}

bool ClassCloneGenerator::ProcessTuple( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
		"   PySafeDecRef( %s );\n"
        "   if( oth.%s == NULL )\n"
        "       %s = NULL;\n" //TODO: log an error
        "   else\n"
        "       %s = new PyTuple( *oth.%s );\n"
        "\n",
        name,
        name,
            name,

            name, name
	);

    return true;
}

bool ClassCloneGenerator::ProcessTupleInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassCloneGenerator::ProcessList( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    PySafeDecRef( %s );\n"
        "    if( oth.%s == NULL )\n"
        "        %s = NULL;\n"
        "    else\n"
        "        %s = new PyList( *oth.%s );\n"
        "\n",
        name,
        name,
            name,

            name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessListInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassCloneGenerator::ProcessListInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessListLong( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessListStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessDict( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf(mOutputFile,
        "    PySafeDecRef( %s );\n"
        "    if( oth.%s == NULL )\n"
        "        %s = NULL;\n"
        "    else\n"
        "        %s = new PyDict( *oth.%s );\n"
        "\n",
        name,
        name,
            name,

            name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessDictInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassCloneGenerator::ProcessDictInlineEntry( const TiXmlElement* field )
{
    //we dont really even care about this...
    const char* key = field->Attribute( "key" );
    if( key == NULL )
    {
        _log(COMMON__ERROR, "<dictInlineEntry> at line %d is missing the key attribute, skipping.", field->Row());
        return false;
    }

    return ParseElementChildren( field, 1 );
}

bool ClassCloneGenerator::ProcessDictRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessDictInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    std::map<int32, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "\n"
        "    //free any existing elements first\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        PyDecRef( %s_cur->second );\n"
        "    %s.clear();\n"
        "\n"
        "    //now we can copy in the new ones...\n"
        "    %s_cur = oth.%s.begin();\n"
        "    %s_end = oth.%s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s[ %s_cur->first ] = %s_cur->second->Clone();\n"
        "\n",
        name, name,

        name, name,
        name, name,
        name, name, name,
            name,
        name,

        name, name,
        name, name,
        name, name, name,
            name, name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessDictStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    std::map<std::string, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "\n"
        "    //free any existing elements first\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        PyDecRef( %s_cur->second );\n"
        "    %s.clear();\n"
        "\n"
        "    //now we can copy in the new ones...\n"
        "    %s_cur = oth.%s.begin();\n"
        "    %s_end = oth.%s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s[ %s_cur->first ] = %s_cur->second->Clone();\n"
        "\n",
        name, name,

        name, name,
        name, name,
        name, name, name,
            name,
        name,

        name, name,
        name, name,
        name, name, name,
            name, name, name
    );

    return true;
}

bool ClassCloneGenerator::ProcessSubStreamInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}

bool ClassCloneGenerator::ProcessSubStructInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}


















