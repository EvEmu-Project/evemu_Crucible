/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
    Rewrite:    Allan
*/

/* updated to (mostly)C++0x11  -allan 19 March 2016 */


#include "eve-xmlpktgen.h"

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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    %s.Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name, name, name,
        name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessElementPtr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%sERROR: ElementPtr = nullptr.\", %s_n.c_str() );\n"
        "\n",
        name, name, name, name,
        name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%sERROR: raw = nullptr.\", %s_n.c_str() );\n"
        "\n",
        name, name, name, name,
        name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
             "    _log( l_type, \"%%s%s=%%lli\", pfx, %s );\n"
             "\n",
             name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessReal( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: \", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    %s->Dump( l_type, %s_n.c_str() );\n"
        "\n",
        name, name, name,
        name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessString( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
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
    if (value == nullptr) {
        std::cout << std::endl <<  "string element at line " << field->Row() << " has no value attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
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
    if (value == nullptr) {
        std::cout << std::endl <<  "wstring element at line " << field->Row() << " has no value attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%sERROR: token = nullptr.\", %s_n.c_str() );\n"
        "\n",
        name, name, name, name,
        name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    const char* value = field->Attribute( "value" );
    if (value == nullptr) {
        std::cout << std::endl <<  "token element at line " << field->Row() << " has no value attribute, skipping.";
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else \n"
        "        _log( l_type, \"%%s    nullptr\", pfx );\n"
        "\n",
        name, name, name,
        name, name, name
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* type = field->Attribute( "type" );
    if (type == nullptr) {
        std::cout << std::endl <<  "field at line " << field->Row() << " is missing the type attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s (%s):\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%s    nullptr\", pfx );\n"
        "\n",
        name, type, name,
        name, name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessTuple( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%sERROR: tuple = nullptr.\", %s_n.c_str() );\n"
        "\n",
        name, name, name,
        name, name, name, name
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%sERROR: list = nullptr.\", %s_n.c_str() );\n"
        "\n",
        name, name, name,
        name, name, name, name
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
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Integer list with %%lu entries\", pfx, %s.size() );\n"
        "\n"
        "    std::vector<int32>::const_iterator cur = %s.begin();\n"
        "    for (int index = 0; cur != %s.end(); ++cur, ++index )\n"
        "        _log( l_type, \"%%s   [%%02d] %%d\", pfx, index, (*cur) );\n"
        "\n",
        name, name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessListLong( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Integer list with %%lu entries\", pfx, %s.size() );\n"
        "\n"
        "    std::vector<int64>::const_iterator cur = %s.begin();\n"
        "    for (int index = 0; cur != %s.end(); ++cur, ++index )\n"
        "        _log( l_type, \"%%s   [%%02d] %%lli\", pfx, index, (*cur) );\n"
        "\n",
        name, name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessListStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    std::vector<std::string>::const_iterator %s_cur = %s.begin(), %s_end = %s.end();\n"
        "    for (int index = 0; %s_cur != %s_end; ++%s_cur, ++index )\n"
        "        _log( l_type, \"%%s   [%%02d] %%s\", pfx, index, ( *%s_cur ).c_str() );\n"
        "\n",
        name, name, name, name,
        name, name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessDict( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s:\", pfx );\n"
        "\n"
        "    std::string %s_n( pfx );\n"
        "    %s_n += \"    \";\n"
        "    if (%s != nullptr)\n"
        "        %s->Dump( l_type, %s_n.c_str() );\n"
        "    else\n"
        "        _log( l_type, \"%%sERROR: dict = nullptr.\", %s_n.c_str() );\n"
        "\n",
        name, name, name,
        name, name, name, name
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
    if (key == nullptr) {
        std::cout << std::endl <<  "<dictInlineEntry> at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }

    return ParseElementChildren( field, 1 );
}

bool ClassDumpGenerator::ProcessDictRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* key = field->Attribute( "key" );
    if (key == nullptr) {
        std::cout << std::endl <<  "field at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }
    const char* pykey = field->Attribute( "pykey" );
    if (pykey == nullptr) {
        std::cout << std::endl <<  "field at line " << field->Row() << " is missing the pykey attribute, skipping.";
        return false;
    }
    const char* value = field->Attribute( "value" );
    if (value == nullptr) {
        std::cout << std::endl <<  "field at line " << field->Row() << " is missing the value attribute, skipping.";
        return false;
    }
    const char* pyvalue = field->Attribute( "pyvalue" );
    if (pyvalue == nullptr) {
        std::cout << std::endl <<  "field at line " << field->Row() << " is missing the pyvalue attribute, skipping.";
        return false;
    }

    //TODO: un-kludge this with respect to printf placeholders/types
    //could make PyRep's out of them and use ->Dump, but thats annoying

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Dictionary with %%lu entries\", pfx, %s.size() );\n"
        "\n"
        "    /* total crap casting here since we do not know the correct printf format */\n"
        "    for (auto cur : %s)\n"
        "        _log( l_type, \"%%s   Key: %%u -> Value: %%u\", pfx, uint32(cur.first), uint32(cur.second));\n"
        "\n",
        name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessDictInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Dictionary with %%lu entries\", pfx, %s.size() );\n"
        "\n"
        "    for (auto cur : %s) {\n"
        "        _log( l_type, \"%%s   Key: %%u\", pfx, cur.first );\n"
        "        std::string n( pfx );\n"
        "        n += \"        \";\n"
        "        cur.second->Dump( l_type, n.c_str() );\n"
        "    }\n"
        "\n",
        name, name, name
    );

    return true;
}

bool ClassDumpGenerator::ProcessDictStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "name field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    _log( l_type, \"%%s%s: Dictionary with %%lu entries\", pfx, %s.size() );\n"
        "\n"
        "    for (auto cur : %s) {\n"
        "        _log( l_type, \"%%s Key: %%s\", pfx, cur.first.c_str() );\n"
        "        std::string n( pfx );\n"
        "        n += \"      \";\n"
        "        cur.second->Dump( l_type, n.c_str() );\n"
        "    }\n"
        "\n",
        name, name, name
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
