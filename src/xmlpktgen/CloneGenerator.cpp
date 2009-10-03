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

#include "CloneGenerator.h"

ClassCloneGenerator::ClassCloneGenerator()
{
    AllGenProcRegs(ClassCloneGenerator);
    GenProcReg(ClassCloneGenerator, IDEntry, NULL);
}

bool ClassCloneGenerator::Process_elementdef(FILE *into, TiXmlElement *element)
{
    const char *name = element->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
        return false;
    }

    fprintf(into,
        "%s& %s::operator=( const %s& oth )\n"
		"{\n",
        name, name, name
    );

    if(!Recurse(into, element))
        return false;

    fprintf(into,
        "    return *this;\n"
        "}\n"
        "\n"
	);

	return true;
}

bool ClassCloneGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field)
{
    return Recurse(into, field);
}

bool ClassCloneGenerator::Process_InlineList(FILE *into, TiXmlElement *field)
{
    return Recurse(into, field);
}

bool ClassCloneGenerator::Process_InlineDict(FILE *into, TiXmlElement *field)
{
    return Recurse(into, field);
}

bool ClassCloneGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
    //we dont really even care about this...
    const char *key = field->Attribute("key");
    if(key == NULL) {
        _log(COMMON__ERROR, "<IDEntry> at line %d is missing the key attribute, skipping.", field->Row());
        return false;
    }

    return Recurse(into, field, 1);
}

bool ClassCloneGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field)
{
    return Recurse(into, field, 1);
}

bool ClassCloneGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field)
{
    return Recurse(into, field, 1);
}

bool ClassCloneGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassCloneGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassCloneGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_element(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassCloneGenerator::Process_none(FILE *into, TiXmlElement *field) {
    return true;
}

bool ClassCloneGenerator::Process_object(FILE *into, TiXmlElement *field) {
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "object at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }

    fprintf(into,
        "    /* object of type %s */\n",
        type
    );

    return Recurse(into, field, 1);
}

bool ClassCloneGenerator::Process_object_ex(FILE *into, TiXmlElement *field)
{
    const char *name = field->Attribute( "name" );
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

	fprintf( into,
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

bool ClassCloneGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassCloneGenerator::Process_raw(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "   PySafeDecRef( %s );\n"
        "   if( oth.%s == NULL )\n"
        "       %s = NULL;\n"  //TODO: log an error
        "   else\n"
        "       %s = oth.%s->Clone();\n"
        "\n",
        name,
        name,
            name,
            name, name);
    return true;
}

bool ClassCloneGenerator::Process_list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassCloneGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    fprintf(into,
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

bool ClassCloneGenerator::Process_dict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassCloneGenerator::Process_bool(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_int(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_int64(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_string(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}

bool ClassCloneGenerator::Process_real(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
        "    %s = oth.%s;\n"
        "\n",
        name, name
    );
    return true;
}


















