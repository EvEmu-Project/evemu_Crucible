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

#include "DumpGenerator.h"

ClassDumpGenerator::ClassDumpGenerator( FILE* outputFile )
: Generator( outputFile )
{
    RegisterProcessors();
}

void ClassDumpGenerator::RegisterProcessors()
{
    Generator::RegisterProcessors();

    AddMemberParser( "dictInlineEntry", &ClassDumpGenerator::ProcessDictInlineEntry );
}

bool ClassDumpGenerator::ProcessElementDef( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "void %s::Dump( LogType l_type, const char* pfx ) const\n"
		"{\n"
        "    _log( l_type, \"%%s%s\", pfx );\n"
		"\n",
        name, name
	);

    if( !ParseElementChildren( field ) )
        return false;

    fprintf( mOutputFile,
		"}\n"
		"\n"
	);

	return true;
}

bool ClassDumpGenerator::ProcessElement( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    %s.Dump( l_type, %s_n.c_str() );\n"
		"\n",
        name,
		name,
		name,
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessElementPtr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    if( %s == NULL )\n"
		"        _log( l_type, \"%%sERROR: NULL OBJECT!\", %s_n.c_str() );\n"
        "    else\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name,
        name,
        name,
        name,
        name,
        name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessRaw( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    if( %s == NULL )\n"
        "        _log( l_type, \"%%sERROR: NULL REP!\", %s_n.c_str() );\n"
        "    else\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name,
        name,
        name,
        name,
        name,
        name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessInt( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"    _log( l_type, \"%%s%s=%%u\", pfx, %s );\n"
		"\n",
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessLong( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"    _log( l_type, \"%%s%s=\" I64d, pfx, %s );\n"
		"\n",
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessReal( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"    _log( l_type, \"%%s%s=%%.13f\", pfx, %s );\n"
		"\n",
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessBool( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"    _log( l_type, \"%%s%s=%%s\", pfx, %s ? \"true\" : \"false\" );\n"
		"\n",
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessNone( const TiXmlElement* field )
{
    return true;
}

bool ClassDumpGenerator::ProcessBuffer( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: \", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    %s->Dump( l_type, %s_n.c_str() );\n"
		"\n",
        name,
		name,
		name,
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessString( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"    _log( l_type, \"%%s%s='%%s'\", pfx, %s.c_str() );\n"
		"\n",
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessStringInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "String element at line %d has no value attribute.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%sString '%s'\", pfx );\n"
        "\n",
        value
    );

    return true;
}

bool ClassDumpGenerator::ProcessWString( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
		"    _log( l_type, \"%%s%s='%%s'\", pfx, %s.c_str() );\n"
		"\n",
		name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "WString element at line %d has no value attribute.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%sWString '%s'\", pfx );\n"
        "\n",
        value
    );

    return true;
}

bool ClassDumpGenerator::ProcessToken( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
    {
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
	    "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
	    "\n"
        "    if( %s == NULL )\n"
        "        _log( l_type, \"%%sNULL\", %s_n.c_str() );\n"
        "    else\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name,

        name,
        name,

        name,
            name,

            name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if( NULL == value )
    {
        _log( COMMON__ERROR, "Token element at line %d has no type attribute.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%sToken '%s'\", pfx );\n"
        "\n",
        value
    );

    return true;
}

bool ClassDumpGenerator::ProcessObject( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    if( NULL == %s )\n"
        "        _log( l_type, \"%%s    NULL\", pfx );\n"
        "    else\n"
        "    {\n"
        "        std::string %s_n( pfx );\n"
        "        %s_n += \"    \";\n"
        "\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    }\n"
        "\n",
        name,

        name,

            name,
            name,
            name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessObjectInline( const TiXmlElement* field )
{
    fprintf( mOutputFile,
        "    _log( l_type, \"%%sObject:\", pfx );\n"
        "\n"
    );

    return ParseElementChildren( field, 2 );
}

bool ClassDumpGenerator::ProcessObjectEx( const TiXmlElement* field )
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
		"    _log( l_type, \"%%s%s (%s):\", pfx );\n"
		"\n"
		"    if( %s == NULL )\n"
		"        _log( l_type, \"%%s    NULL\", pfx );\n"
		"    else\n"
		"    {\n"
		"        std::string %s_n( pfx );\n"
		"        %s_n += \"    \";\n"
		"\n"
		"        %s->Dump( l_type, %s_n.c_str() );\n"
		"    }\n"
		"\n",
		name, type,
		name,

			name,
			name,
			name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessTuple( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    if( %s == NULL )\n"
        "        _log( l_type, \"%%sERROR: NULL TUPLE!\", %s_n.c_str() );\n"
        "    else\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name,

        name,
        name,

        name,
            name,

            name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessTupleInline( const TiXmlElement* field )
{
    //do we want to display the tuple in the dump?
    return ParseElementChildren( field );
}

bool ClassDumpGenerator::ProcessList( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    if( %s == NULL )\n"
        "        _log( l_type, \"%%sERROR: NULL LIST!\", %s_n.c_str() );\n"
        "    else\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name,

        name,
        name,

        name,
            name,

            name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessListInline( const TiXmlElement* field )
{
    //do we want to display the list in the dump?
    return ParseElementChildren( field );
}

bool ClassDumpGenerator::ProcessListInt( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Integer list with %%lu entries\", pfx, %s.size() );\n"
		"\n"
        "    std::vector<int32>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for( int %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "        _log( l_type, \"%%s   [%%02d] %%d\", pfx, %s_index, *%s_cur );\n"
        "\n",
        name, name,
		name, name,
		name, name,
        name, name,
		name, name, name, name, name,
		name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessListLong( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Integer list with %%lu entries\", pfx, %s.size() );\n"
		"\n"
        "    std::vector<int64>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for( int %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "        _log( l_type, \"%%s   [%%02d] \" I64d, pfx, %s_index, *%s_cur );\n"
        "\n",
        name, name,
		name, name,
		name, name,
        name, name,
		name, name, name, name, name,
		name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessListStr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: String list with %%lu entries\", pfx, %s.size() );\n"
		"\n"
        "    std::vector<std::string>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for( int %s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++ )\n"
        "        _log( l_type, \"%%s   [%%02d] %%s\", pfx, %s_index, ( *%s_cur ).c_str() );\n"
        "\n",
        name, name,
		name, name,
		name, name,
        name, name,
		name, name, name, name, name,
		name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessDict( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
		"\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
		"\n"
        "    if( %s == NULL )\n"
        "        _log( l_type, \"%%sERROR: NULL DICT!\", %s_n.c_str() );\n"
        "    else\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name,

        name,
        name,

        name,
            name,

            name, name
	);

    return true;
}

bool ClassDumpGenerator::ProcessDictInline( const TiXmlElement* field )
{
    //do we want to display the dict in the dump?
    return ParseElementChildren( field );
}

bool ClassDumpGenerator::ProcessDictInlineEntry( const TiXmlElement* field )
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

bool ClassDumpGenerator::ProcessDictRaw( const TiXmlElement* field )
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

    //TODO: un-kludge this with respect to printf placeholders/types
    //could make PyRep's out of them and use ->Dump, but thats annoying

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Dictionary with %%lu entries\", pfx, %s.size() );\n"
		"\n"
        "    std::map<%s, %s>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        //total crap casting here since we do not know the correct printf format\n"
        "        _log( l_type, \"%%s   Key: %%u -> Value: %%u\", pfx, uint32( %s_cur->first ), uint32( %s_cur->second ) );\n"
        "\n",
        name, name,
        key, value, name, name,
        name, name,
        name, name,
        name, name, name,
            name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessDictInt( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Dictionary with %%lu entries\", pfx, %s.size() );\n"
		"\n"
        "    std::map<int32, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
		"    {\n"
        "        _log( l_type, \"%%s   Key: %%u\", pfx, %s_cur->first );\n"
		"\n"
        "        std::string n( pfx );\n"
        "        n += \"        \";\n"
		"\n"
        "        %s_cur->second->Dump( l_type, n.c_str() );\n"
        "    }\n"
        "\n",
        name, name, name,
        name, name, name,
        name, name, name,
        name, name, name,
        name
    );

    return true;
}

bool ClassDumpGenerator::ProcessDictStr( const TiXmlElement* field )
{
	const char* name = field->Attribute( "name" );
	if( name == NULL )
    {
		_log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
		return false;
	}

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Dictionary with %%lu entries\", pfx, %s.size() );\n"
		"\n"
        "    std::map<std::string, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
		"    {\n"
        "        _log( l_type, \"%%s Key: %%s\", pfx, %s_cur->first.c_str() );\n"
		"\n"
        "        std::string n( pfx );\n"
        "        n += \"      \";\n"
		"\n"
        "        %s_cur->second->Dump( l_type, n.c_str() );\n"
        "    }\n"
        "\n",
        name, name, name,
        name, name, name,
        name, name, name,
        name, name, name,
        name
    );

    return true;
}

bool ClassDumpGenerator::ProcessSubStreamInline( const TiXmlElement* field )
{
    //do we want to display the substream in the dump?
    return ParseElementChildren( field, 1 );
}

bool ClassDumpGenerator::ProcessSubStructInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}



















