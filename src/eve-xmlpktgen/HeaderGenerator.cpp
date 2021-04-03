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

#include "HeaderGenerator.h"


ClassHeaderGenerator::ClassHeaderGenerator( FILE* outputFile )
: Generator( outputFile )
{
    RegisterProcessors();
}

bool ClassHeaderGenerator::RegisterName( const char* name, uint32 row )
{
    if (mNamesUsed.find( name ) != mNamesUsed.end() )
    {
        std::cout << std::endl <<  "ClassHeaderGenerator::RegisterName: Field at line " << row << ": The name '" << name << "' is already used.";

        return false;
    }

    mNamesUsed.insert( name );
    return true;
}

void ClassHeaderGenerator::ClearNames()
{
    mNamesUsed.clear();
}

void ClassHeaderGenerator::RegisterProcessors()
{
    Generator::RegisterProcessors();

    AddMemberParser( "dictInlineEntry", &ClassHeaderGenerator::ProcessDictInlineEntry );
}

bool ClassHeaderGenerator::ProcessElementDef( const TiXmlElement* field )
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessElementDef: <element> at line " << field->Row() << " contains more than one root element, skipping.";
        return false;
    }

    const TiXmlElement* main = field->FirstChildElement();
    if (main->NextSiblingElement() != nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessElementDef: <element> at line " << field->Row() << " contains more than one root element, skipping.";
        return false;
    }

    const char* encode_type = GetEncodeType( main );
    fprintf( mOutputFile,
        "class %s\n"
        "{\n"
        "public:\n"
        "    %s();\n"
        "    %s(const %s& oth);\n"
        "    ~%s();\n"
        "\n"
        "    void Dump(LogType type, const char* pfx = \" \") const;\n"
        "\n"
        "    bool Decode(PyRep* packet);\n"
        "    bool Decode(PyRep** packet);\n"
        "    bool Decode(%s** packet);\n"
        "    %s* Encode() const;\n"
        "\n"
        "    %s& operator=(const %s& oth);\n"
        "\n",
        name,

        name,
        name, name,
        name,

            encode_type,
        encode_type,

        name, name
    );

    if (!ParseElement( main ) )
        return false;

    fprintf( mOutputFile,
        "};\n"
        "\n"
    );

    ClearNames();

    return true;
}

bool ClassHeaderGenerator::ProcessElement( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessElement: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* type = field->Attribute( "type" );
    if (type == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessElement: field at line " << field->Row() << " is missing the type attribute, skipping.";
        return false;
    }

    if (!RegisterName( name, field->Row() ))
        return false;

    fprintf( mOutputFile,
        "    %s\t%s;\n",
        type, name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessElementPtr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessElementPtr: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* type = field->Attribute( "type" );
    if (type == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessElementPtr: field at line " << field->Row() << " is missing the type attribute, skipping.";
        return false;
    }

    if (!RegisterName( name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    %s*\t%s;\n",
        type, name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessRaw: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName( name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    PyRep*\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessInt: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
             "    int32\t\t%s;\n",
             name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessLong( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessLong: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
             "    int64\t\t%s;\n",
             name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessReal( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessReal: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    double\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessBool( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessBool: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    bool\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessNone( const TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::ProcessBuffer( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessBuffer: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    PyBuffer*\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessString( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessString: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::string\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::ProcessWString( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessWString: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::string\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::ProcessToken( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessToken: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    PyToken*\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessTokenInline( const TiXmlElement* field )
{
    return true;
}

bool ClassHeaderGenerator::ProcessObject( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessObject: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf( mOutputFile,
        "    PyObject*\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessObjectInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 2 );
}

bool ClassHeaderGenerator::ProcessObjectEx( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessObjectEx: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* type = field->Attribute( "type" );
    if (type == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessObjectEx: field at line " << field->Row() << " is missing the type attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    %s*\t%s;\n",
        type, name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessTuple( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessTuple: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    PyTuple*\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessTupleInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassHeaderGenerator::ProcessList( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessList: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    PyList*\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessListInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassHeaderGenerator::ProcessListInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessListInt: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::vector<int32>\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessListLong( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessListLong: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::vector<int64>\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessListStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessListStr: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::vector<std::string>\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessDict( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDict: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    PyDict*\t\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessDictInline( const TiXmlElement* field )
{
    return ParseElementChildren( field );
}

bool ClassHeaderGenerator::ProcessDictInlineEntry( const TiXmlElement* field )
{
    //we dont really even care about this...
    const char* key = field->Attribute( "key" );
    if (!key) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictInlineEntry: field at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }

    return ParseElementChildren( field, 1 );
}

bool ClassHeaderGenerator::ProcessDictRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictRaw: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* key = field->Attribute( "key" );
    if (key == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictRaw: field at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }
    const char* pykey = field->Attribute( "pykey" );
    if (pykey == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictRaw: field at line " << field->Row() << " is missing the pykey attribute, skipping.";
        return false;
    }
    const char* value = field->Attribute( "value" );
    if (value == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictRaw: field at line " << field->Row() << " is missing the value attribute, skipping.";
        return false;
    }
    const char* pyvalue = field->Attribute( "pyvalue" );
    if (pyvalue == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictRaw: field at line " << field->Row() << " is missing the pyvalue attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::map<%s, %s>\t%s;\n",
        key, value, name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessDictInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictInt: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::map<int32, PyRep*>\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessDictStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassHeaderGenerator::ProcessDictStr: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    if (!RegisterName(name, field->Row()))
        return false;

    fprintf( mOutputFile,
        "    std::map<std::string, PyRep*>\t%s;\n",
        name
    );

    return true;
}

bool ClassHeaderGenerator::ProcessSubStreamInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}

bool ClassHeaderGenerator::ProcessSubStructInline( const TiXmlElement* field )
{
    return ParseElementChildren( field, 1 );
}

