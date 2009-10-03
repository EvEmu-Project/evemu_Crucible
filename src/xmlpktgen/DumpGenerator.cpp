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

#include "DumpGenerator.h"

ClassDumpGenerator::ClassDumpGenerator()
{
    AllGenProcRegs(ClassDumpGenerator);
    GenProcReg(ClassDumpGenerator, IDEntry, NULL);
}

bool ClassDumpGenerator::Process_elementdef(FILE *into, TiXmlElement *element)
{
    const char *name = element->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
        return false;
    }

    fprintf(into,
        "void %s::Dump( LogType l_type, const char* pfx ) const\n"
		"{\n"
        "    _log( l_type, \"%%s%s\", pfx );\n"
		"\n",
        name, name
	);

    if(!Recurse(into, element))
        return false;

    fprintf(into,
		"}\n"
		"\n"
	);

	return true;
}

bool ClassDumpGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field)
{
    //do we want to display the tuple in the dump?
    return Recurse(into, field);
}

bool ClassDumpGenerator::Process_InlineList(FILE *into, TiXmlElement *field)
{
    //do we want to display the list in the dump?
    return Recurse(into, field);
}

bool ClassDumpGenerator::Process_InlineDict(FILE *into, TiXmlElement *field)
{
    //do we want to display the dict in the dump?
    return Recurse(into, field);
}

bool ClassDumpGenerator::Process_IDEntry(FILE *into, TiXmlElement *field)
{
    //we dont really even care about this...
    const char *key = field->Attribute("key");
    if(key == NULL) {
        _log(COMMON__ERROR, "<IDEntry> at line %d is missing the key attribute, skipping.", field->Row());
        return false;
    }

	return Recurse(into, field, 1);
}

bool ClassDumpGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field)
{
    //do we want to display the substream in the dump?
    return Recurse(into, field, 1);
}

bool ClassDumpGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field)
{
    return Recurse(into, field, 1);
}

bool ClassDumpGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    const char *key = field->Attribute("key");
    if(key == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the key attribute, skipping.", field->Row());
        return false;
    }
    const char *pykey = field->Attribute("pykey");
    if(pykey == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the pykey attribute, skipping.", field->Row());
        return false;
    }
    const char *value = field->Attribute("value");
    if(value == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the value attribute, skipping.", field->Row());
        return false;
    }
    const char *pyvalue = field->Attribute("pyvalue");
    if(pyvalue == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the pyvalue attribute, skipping.", field->Row());
        return false;
    }

    //TODO: un-kludge this with respect to printf placeholders/types
    //could make PyRep's out of them and use ->Dump, but thats annoying

    fprintf(into,
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

bool ClassDumpGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_element(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_none(FILE *into, TiXmlElement *field) {
    return true;
}

bool ClassDumpGenerator::Process_object(FILE *into, TiXmlElement *field) {
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }

    fprintf(into,
        "   _log( l_type, \"%%sObject of type %s:\", pfx );\n"
		"\n",
        type
	);

	return Recurse(into, field, 1);
}

bool ClassDumpGenerator::Process_object_ex(FILE *into, TiXmlElement *field)
{
    const char *name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }
	const char *type = field->Attribute( "type" );
	if( type == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the type attribute.", field->Row() );
		return false;
	}

	fprintf( into,
		"    _log( l_type, \"%%s%s (%s):\", pfx );\n"
		"\n"
		"    if( %s == NULL )\n"
		"        _log( l_type, \"%%s	NULL\", pfx );\n"
		"    else\n"
		"    {\n"
		"        std::string %s_n( pfx );\n"
		"        %s_n += \"	\";\n"
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

bool ClassDumpGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_raw(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_dict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
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

bool ClassDumpGenerator::Process_bool(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
		"    _log( l_type, \"%%s%s=%%s\", pfx, %s ? \"true\" : \"false\" );\n"
		"\n",
		name, name
	);
    return true;
}

bool ClassDumpGenerator::Process_int(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
		"    _log( l_type, \"%%s%s=%%u\", pfx, %s );\n"
		"\n",
		name, name
	);
    return true;
}

bool ClassDumpGenerator::Process_int64(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
		"    _log( l_type, \"%%s%s=\" I64d, pfx, %s );\n"
		"\n",
		name, name
	);
    return true;
}

bool ClassDumpGenerator::Process_string(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
		"    _log( l_type, \"%%s%s='%%s'\", pfx, %s.c_str() );\n"
		"\n",
		name, name
	);
    return true;
}

bool ClassDumpGenerator::Process_real(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into,
		"    _log( l_type, \"%%s%s=%%.13f\", pfx, %s );\n"
		"\n",
		name, name
	);
    return true;
}



















