
#include "DestructGenerator.h"
#include "../common/logsys.h"

ClassDestructGenerator::ClassDestructGenerator() {
	AllProcFMaps(ClassDestructGenerator);
	ProcFMap(ClassDestructGenerator, IDEntry, NULL);
}

void ClassDestructGenerator::Process_root(FILE *into, TiXmlElement *element) {
	const char *name = element->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
		return;
	}

	fprintf(into,
		"\n"
		"%s::~%s() {\n",
		name, name);
	
	if(!ProcessFields(into, element))
		return;
	
	fprintf(into,
		"}\n");
}

bool ClassDestructGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
	//we dont really even care about this...
	const char *key = field->Attribute("key");
	if(key == NULL) {
		_log(COMMON__ERROR, "<IDEntry> at line %d is missing the key attribute, skipping.", field->Row());
		return(false);
	}
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	std::map<std::string, PyRep *>::iterator %s_cur, %s_end;\n"
		"	//free any existing elements first\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		delete %s_cur->second;\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDestructGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	std::map<uint32, PyRep *>::iterator %s_cur, %s_end;\n"
		"	//free any existing elements first\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		delete %s_cur->second;\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDestructGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_element(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\tdelete %s;\n", name);
	return(true);
}

bool ClassDestructGenerator::Process_none(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_object(FILE *into, TiXmlElement *field) {
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDestructGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\tdelete %s;\n", name);
	return(true);
}

bool ClassDestructGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\tdelete %s;\n", name);
	return(true);
}

bool ClassDestructGenerator::Process_list(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\tdelete %s;\n", name);
	return(true);
}

bool ClassDestructGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_int(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_string(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDestructGenerator::Process_real(FILE *into, TiXmlElement *field) {
	return(true);
}


















