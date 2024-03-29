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
*/

#include "eve-xmlpktgen.h"

#include "DestructGenerator.h"

ClassDestructGenerator::ClassDestructGenerator( FILE* outputFile )
: Generator( outputFile )
{
    RegisterProcessors();
}

void ClassDestructGenerator::RegisterProcessors()
{
    Generator::RegisterProcessors();

    AddMemberParser( "dictInlineEntry", &ClassDestructGenerator::ProcessDictInlineEntry );
}

bool ClassDestructGenerator::ProcessElementDef( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessElementDef <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "%s::~%s()\n"
        "{\n",
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

bool ClassDestructGenerator::ProcessElement( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessElementPtr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessElementPtr <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    SafeDelete( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessRaw <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    PySafeDecRef( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessInt( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessLong( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessReal( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessBool( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessNone( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessBuffer( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessBuffer <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
             "    //SafeDelete(%s);\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessString( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessWString( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessToken( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessToken <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    //PySafeDecRef( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessObject( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessObject <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    //PySafeDecRef( %s );\n"
        "\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessObjectInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 2 );
}

bool ClassDestructGenerator::ProcessObjectEx( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessObjectEx <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    //PySafeDecRef( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessTuple( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessTuple <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    //PySafeDecRef( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessTupleInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassDestructGenerator::ProcessList( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessList <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    //PySafeDecRef( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessListInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassDestructGenerator::ProcessListInt( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessListLong( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessListStr( const TiXmlElement* field ) {
    return true;
}

bool ClassDestructGenerator::ProcessDict( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessDict <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    //PySafeDecRef( %s );\n",
        name
    );

    return true;
}

bool ClassDestructGenerator::ProcessDictInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassDestructGenerator::ProcessDictInlineEntry( const TiXmlElement* field )
{
    //we dont really even care about this...
    const char* key = field->Attribute( "key" );
    if (key == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessDictInlineEntry <key> field at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }

    return ParseElementChildren( field, 1 );
}

bool ClassDestructGenerator::ProcessDictRaw( const TiXmlElement* field )
{
    return true;
}

bool ClassDestructGenerator::ProcessDictInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr)  {
        std::cout << std::endl <<  "DtorGen::ProcessDictInt <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
             "    std::map<int32, PyRep*>::const_iterator %s_cur = %s.begin();\n"
             "    for (; %s_cur != %s.end(); ++%s_cur)\n"
             "        PyDecRef(%s_cur->second);\n"
            "\n",
             name, name,
             name, name, name,
             name
    );

    return true;
}

bool ClassDestructGenerator::ProcessDictStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "DtorGen::ProcessDictStr <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
            "    std::map<std::string, PyRep*>::const_iterator %s_cur = %s.begin();\n"
            "    for (; %s_cur != %s.end(); ++%s_cur)\n"
            "        PyDecRef(%s_cur->second);\n"
            "\n",
             name, name,
             name, name, name,
             name
    );

    return true;
}

bool ClassDestructGenerator::ProcessSubStreamInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}

bool ClassDestructGenerator::ProcessSubStructInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}

