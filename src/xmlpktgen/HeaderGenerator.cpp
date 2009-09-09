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


#include "common.h"
#include "HeaderGenerator.h"
#include "../common/logsys.h"

ClassHeaderGenerator::ClassHeaderGenerator() {
    AllProcFMaps(ClassHeaderGenerator);
    ProcFMap(ClassHeaderGenerator, IDEntry, NULL);
}

void ClassHeaderGenerator::Process_root(FILE *into, TiXmlElement *element) {
    const char *name = element->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
        return;
    }

    TiXmlElement * main = element->FirstChildElement();
    if(main->NextSiblingElement() != NULL) {
        _log(COMMON__ERROR, "<element> at line %d contains more than one root element. skipping.", element->Row());
        return;
    }

    const char *encode_type = GetEncodeType(element);
    fprintf(into,
        "\nclass %s {\n"
        "public:\n"
        "   %s();\n"
        "   ~%s();\n"
        "   \n"
        "   void Dump(LogType type, const char *pfx = \"\") const;\n"
        "   bool Decode(PyRep **packet);    //consumes packet\n"
        "   bool Decode(%s **packet);\n"
        "   %s *Encode();\n"
        "   %s *FastEncode();\n"
        "   %s *Clone() const;\n"
        "   void CloneFrom(const %s *from);\n"
        "   \n",
        name,
        name,
        name,
        encode_type,
        encode_type,
        encode_type,
        name,
        name
        );

    m_namesUsed.clear();

    if(!ProcessFields(into, element, 1))
        return;

    fprintf(into, "\n\n};\n\n");
}

void ClassHeaderGenerator::Process_include(FILE *into, TiXmlElement *element) {
    const char *file = element->Attribute("file");
    if(file == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the file attribute, skipping.", element->Row());
        return;
    }
    fprintf(into, "#include \"%s\"\n", file);
}

bool ClassHeaderGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
    if(!ProcessFields(into, field))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
    if(!ProcessFields(into, field))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
    if(!ProcessFields(into, field))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
    //we dont really even care about this...
    const char *key = field->Attribute("key");
    if(key == NULL) {
        _log(COMMON__ERROR, "<IDEntry> at line %d is missing the key attribute, skipping.", field->Row());
        return false;
    }
    if(!ProcessFields(into, field, 1))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
    if(!ProcessFields(into, field, 1))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
    if(!ProcessFields(into, field, 1))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::map<std::string, PyRep *>\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::map<int32, PyRep *>\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
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

    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::map<%s, %s>\t%s;\n", key, value, name);
    return true;
}

bool ClassHeaderGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::vector<std::string>\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::vector<int32>\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::vector<int64>\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_element(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\t%s\t%s;\n", type, name);
    return true;
}

bool ClassHeaderGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\t%s\t*%s;\n", type, name);
    return true;
}

bool ClassHeaderGenerator::Process_none(FILE *into, TiXmlElement *field) {
    return true;
}

bool ClassHeaderGenerator::Process_object(FILE *into, TiXmlElement *field) {
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "object at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }
    fprintf(into, "\t/* object of type %s */\n", type);
    if(!ProcessFields(into, field, 1))
        return false;
    return true;
}

bool ClassHeaderGenerator::Process_object_ex(FILE *into, TiXmlElement *field)
{
    const char *name = field->Attribute( "name" );
    if( name == NULL )
	{
        _log( COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row() );
        return false;
    }
    if( m_namesUsed.find( name ) != m_namesUsed.end() )
	{
        _log( COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name );
        return false;
    }
	m_namesUsed.insert( name );

	const char *type = field->Attribute( "type" );
	if( type == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the type attribute.", field->Row() );
		return false;
	}

	fprintf( into, "\t%s\t*%s;\n", type, name );
    return true;
}

bool ClassHeaderGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tPyBuffer\t*%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_raw(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tPyRep\t\t*%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tPyList\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tPyTuple\t*%s;   //due to (somewhat false) sizing constructor, must be dynamic memory.\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_dict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tPyDict\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_bool(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tbool\t\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_int(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tint32\t\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_int64(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tint64\t\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_string(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tstd::string\t%s;\n", name);
    return true;
}

bool ClassHeaderGenerator::Process_real(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
    if(m_namesUsed.find(name) != m_namesUsed.end()) {
        _log(COMMON__ERROR, "field at line %d: the name '%s' is already used", field->Row(), name);
        return false;
    }
    m_namesUsed.insert(name);
    fprintf(into, "\tdouble\t\t%s;\n", name);
    return true;
}





















