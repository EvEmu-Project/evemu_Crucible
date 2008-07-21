
#include "CloneGenerator.h"
#include "../common/logsys.h"

ClassCloneGenerator::ClassCloneGenerator() {
	AllProcFMaps(ClassCloneGenerator);
	ProcFMap(ClassCloneGenerator, IDEntry, "NULL");
}

void ClassCloneGenerator::Process_root(FILE *into, TiXmlElement *element) {
	const char *name = element->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
		return;
	}

	fprintf(into,
		"\n"
		"%s *%s::Clone() const {\n"
		"	%s *res = new %s;\n"
		"	res->CloneFrom(this);\n"
		"	return(res);\n"
		"}\n"
		"\n"
		"void %s::CloneFrom(const %s *from) {\n",
		name, name,
			name, name,
		name, name);
	
	if(!ProcessFields(into, element))
		return;
	
	fprintf(into,
		"	\n"
		"}\n"
		"\n");
}

bool ClassCloneGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassCloneGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassCloneGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassCloneGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
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

bool ClassCloneGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassCloneGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassCloneGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	std::map<std::string, PyRep *>::const_iterator %s_cur, %s_end;\n"
		"	//free any existing elements first\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		delete %s_cur->second;\n"
		"	}\n"
		"	%s.clear();\n"
		"	//now we can copy in the new ones...\n"
		"	%s_cur = from->%s.begin();\n"
		"	%s_end = from->%s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s[%s_cur->first] = %s_cur->second->Clone();\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name
	);
	return(true);
}

bool ClassCloneGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	std::map<uint32, PyRep *>::const_iterator %s_cur, %s_end;\n"
		"	//free any existing elements first\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		delete %s_cur->second;\n"
		"	}\n"
		"	%s.clear();\n"
		"	//now we can copy in the new ones...\n"
		"	%s_cur = from->%s.begin();\n"
		"	%s_end = from->%s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s[%s_cur->first] = %s_cur->second->Clone();\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name
	);
	return(true);
}

bool ClassCloneGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_element(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s.CloneFrom(&from->%s);\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	delete %s;\n"
		"	if(from->%s == NULL) {\n"
			//TODO: log an error
		"		%s = NULL;\n"
		"	} else {\n"
		"		%s = from->%s->Clone();\n"
		"	}\n",
		name,
		name,
			name,
			name, name);
	return(true);
}

bool ClassCloneGenerator::Process_none(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassCloneGenerator::Process_object(FILE *into, TiXmlElement *field) {
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "object at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t/* object of type %s */\n", type);
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassCloneGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	delete %s;\n"
		"	if(from->%s == NULL) {\n"
			//TODO: log an error
		"		%s = NULL;\n"
		"	} else {\n"
		"		%s = from->%s->TypedClone();\n"
		"	}\n",
		name,
		name,
			name,
			name, name);
	return(true);
}

bool ClassCloneGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	delete %s;\n"
		"	if(from->%s == NULL) {\n"
			//TODO: log an error
		"		%s = NULL;\n"
		"	} else {\n"
		"		%s = from->%s->Clone();\n"
		"	}\n",
		name,
		name,
			name,
			name, name);
	return(true);
}

bool ClassCloneGenerator::Process_list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s.CloneFrom(&from->%s);\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	delete %s;\n"
		"	if(from->%s == NULL) {\n"
			//TODO: log an error
		"		%s = NULL;\n"
		"	} else {\n"
		"		%s = from->%s->TypedClone();\n"
		"	}\n",
		name,
		name,
			name,
			name, name);
	return(true);
}

bool ClassCloneGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s.CloneFrom(&from->%s);\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_int(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_string(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}

bool ClassCloneGenerator::Process_real(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = from->%s;\n", name, name);
	return(true);
}


















