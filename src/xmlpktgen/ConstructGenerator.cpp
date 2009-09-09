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
#include "ConstructGenerator.h"
#include "../common/logsys.h"

ClassConstructGenerator::ClassConstructGenerator() {
	AllProcFMaps(ClassConstructGenerator);
	ProcFMap(ClassConstructGenerator, IDEntry, NULL);
}

void ClassConstructGenerator::Process_root(FILE *into, TiXmlElement *element) {
	const char *name = element->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
		return;
	}

	fprintf(into,
		"\n"
		"%s::%s() {\n",
		name, name);
	
	if(!ProcessFields(into, element))
		return;
	
	fprintf(into,
		"}\n");
}

bool ClassConstructGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return false;
	return true;
}

bool ClassConstructGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return false;
	return true;
}

bool ClassConstructGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return false;
	return true;
}

bool ClassConstructGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
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

bool ClassConstructGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return false;
	return true;
}

bool ClassConstructGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return false;
	return true;
}

bool ClassConstructGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_element(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return true;
}

bool ClassConstructGenerator::Process_none(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_object(FILE *into, TiXmlElement *field) {
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

bool ClassConstructGenerator::Process_object_ex(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute( "name" );
	if( name == NULL )
	{
		_log( COMMON__ERROR, "field at line %d is missing the name attribute.", field->Row() );
		return false;
	}

	fprintf( into, "\t%s = NULL;\n", name );
	return true;
}

bool ClassConstructGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return true;
}

bool ClassConstructGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return true;
}

bool ClassConstructGenerator::Process_list(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return true;
}

bool ClassConstructGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	return true;
}

bool ClassConstructGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "false";
	fprintf(into, "\t%s = %s;\n", name, def);
	return true;
}

bool ClassConstructGenerator::Process_int(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "0";
	fprintf(into, "\t%s = %s;\n", name, def);
	return true;
}

bool ClassConstructGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "0";
	fprintf(into, "\t%s = %s;\n", name, def);
	return true;
}

bool ClassConstructGenerator::Process_string(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "";
	fprintf(into, "\t%s = \"%s\";\n", name, def);
	return true;
}

bool ClassConstructGenerator::Process_real(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return false;
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "0.0";
	fprintf(into, "\t%s = %s;\n", name, def);
	return true;
}


















