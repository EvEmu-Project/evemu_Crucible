
#include "DumpGenerator.h"
#include "../common/logsys.h"





ClassDumpGenerator::ClassDumpGenerator() {
	AllProcFMaps(ClassDumpGenerator);
	ProcFMap(ClassDumpGenerator, IDEntry, NULL);
}

void ClassDumpGenerator::Process_root(FILE *into, TiXmlElement *element) {
	const char *name = element->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
		return;
	}

	fprintf(into,
		"\n"
		"void %s::Dump(LogType l_type, const char *pfx) const {\n"
		"	_log(l_type, \"%%s%s\", pfx);\n",
		name, name);
	
	if(!ProcessFields(into, element))
		return;
	
	fprintf(into, "}\n");
}

bool ClassDumpGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
	//do we want to display the tuple in the dump?
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassDumpGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	//do we want to display the list in the dump?
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassDumpGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	//do we want to display the dict in the dump?
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassDumpGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
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

bool ClassDumpGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	//do we want to display the substream in the dump?
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDumpGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDumpGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	_log(l_type, \"%%s%s: Dictionaty with %%d enetires\", pfx, %s.size());\n"
		"	std::map<std::string, PyRep *>::const_iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		_log(l_type, \"%%s Key: %%s\", pfx, %s_cur->first.c_str());\n"
		"		std::string n(pfx);\n"
		"		n += \"      \";\n"
		"		%s_cur->second->Dump(stdout, n.c_str());\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDumpGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	_log(l_type, \"%%s%s: Dictionaty with %%d enetires\", pfx, %s.size());\n"
		"	std::map<uint32, PyRep *>::const_iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		_log(l_type, \"%%s   Key: %%lu\", pfx, %s_cur->first);\n"
		"		std::string n(pfx);\n"
		"		n += \"        \";\n"
		"		%s_cur->second->Dump(stdout, n.c_str());\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDumpGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *key = field->Attribute("key");
	if(key == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the key attribute, skipping.", field->Row());
		return(false);
	}
	const char *pykey = field->Attribute("pykey");
	if(pykey == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the pykey attribute, skipping.", field->Row());
		return(false);
	}
	const char *value = field->Attribute("value");
	if(value == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the value attribute, skipping.", field->Row());
		return(false);
	}
	const char *pyvalue = field->Attribute("pyvalue");
	if(pyvalue == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the pyvalue attribute, skipping.", field->Row());
		return(false);
	}

	//TODO: un-kludge this with respect to printf placeholders/types
	//could make PyRep's out of them and use ->Dump, but thats annoying
	
	fprintf(into, 
		"	_log(l_type, \"%%s%s: Dictionaty with %%d enetires\", pfx, %s.size());\n"
		"	std::map<%s, %s>::const_iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		//total crap casting here since we do not know the correct printf format\n"
		"		_log(l_type, \"%%s   Key: %%lu -> Value: %%lu\", pfx, uint32(%s_cur->first), uint32(%s_cur->second));\n"
		"	}\n"
		"	\n", 
		name, name, 
		key, value, name, name,
		name, name,
		name, name, 
		name, name, name,
			name, name
	);
	return(true);
}

bool ClassDumpGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	_log(l_type, \"%%s%s: String list with %%d enetires\", pfx, %s.size());\n"
		"	std::vector<std::string>::const_iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		_log(l_type, \"%%s   [%%02d] %%s\", pfx, %s_index, (*%s_cur).c_str());\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDumpGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	_log(l_type, \"%%s%s: Integer list with %%d enetires\", pfx, %s.size());\n"
		"	std::vector<uint32>::const_iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		_log(l_type, \"%%s   [%%02d] %%d\", pfx, %s_index, *%s_cur);\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDumpGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	_log(l_type, \"%%s%s: Integer list with %%d enetires\", pfx, %s.size());\n"
		"	std::vector<uint64>::const_iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		_log(l_type, \"%%s   [%%02d] \" I64u, pfx, %s_index, *%s_cur);\n"
		"	}\n"
		"	\n", 
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name, name, name,
		name
	);
	return(true);
}

bool ClassDumpGenerator::Process_element(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s:\", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	%s.Dump(l_type, %s_n.c_str());\n",
		name, name, name, name, name);
	return(true);
}

bool ClassDumpGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s:\", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	if(%s == NULL) {\n"
		"		_log(l_type, \"%%sERROR: NULL OBJECT!\", %s_n.c_str());\n"
		"	} else {\n"
		"		%s->Dump(l_type, %s_n.c_str());\n"
		"	}\n",
		name,
		name,
		name,
		name,
		name,
		name,
		name);
	return(true);
}

bool ClassDumpGenerator::Process_none(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassDumpGenerator::Process_object(FILE *into, TiXmlElement *field) {
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%sObject of type %s:\", pfx);\n",
		type);
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassDumpGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s: \", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	%s->Dump(l_type, %s_n.c_str());\n",
		name, name, name, name, name);
	return(true);
}

bool ClassDumpGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s:\", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	if(%s == NULL) {\n"
		"		_log(l_type, \"%%sERROR: NULL REP!\", %s_n.c_str());\n"
		"	} else {\n"
		"		%s->Dump(l_type, %s_n.c_str());\n"
		"	}\n",
		name,
		name,
		name,
		name,
		name,
		name,
		name);
	return(true);
}

bool ClassDumpGenerator::Process_list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s: \", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	%s.Dump(l_type, %s_n.c_str());\n",
		name, name, name, name, name);
	return(true);
}

bool ClassDumpGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s:\", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	if(%s == NULL) {\n"
		"		_log(l_type, \"%%sERROR: NULL TUPLE!\", %s_n.c_str());\n"
		"	} else {\n"
		"		%s->Dump(l_type, %s_n.c_str());\n"
		"	}\n",
		name,
		name,
		name,
		name,
		name,
		name,
		name);
	return(true);
}

bool ClassDumpGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into,
		"	_log(l_type, \"%%s%s: \", pfx);\n"
		"	std::string %s_n(pfx);\n"
		"	%s_n += \"    \";\n"
		"	%s.Dump(l_type, %s_n.c_str());\n",
		name, name, name, name, name);
	return(true);
}

bool ClassDumpGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t_log(l_type, \"%%s%s=%%s\", pfx, %s?\"true\":\"false\");\n", name, name);
	return(true);
}

bool ClassDumpGenerator::Process_int(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t_log(l_type, \"%%s%s=%%lu\", pfx, %s);\n", name, name);
	return(true);
}

bool ClassDumpGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t_log(l_type, \"%%s%s=\" I64u, pfx, %s);\n", name, name);
	return(true);
}

bool ClassDumpGenerator::Process_string(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t_log(l_type, \"%%s%s='%%s'\", pfx, %s.c_str());\n", name, name);
	return(true);
}

bool ClassDumpGenerator::Process_real(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t_log(l_type, \"%%s%s=%%.13f\", pfx, %s);\n", name, name);
	return(true);
}



















