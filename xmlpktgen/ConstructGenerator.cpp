
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
		return(false);
	return(true);
}

bool ClassConstructGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassConstructGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field))
		return(false);
	return(true);
}

bool ClassConstructGenerator::Process_IDEntry(FILE *into, TiXmlElement *field) {
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

bool ClassConstructGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassConstructGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	if(!ProcessFields(into, field, 1))
		return(false);
	return(true);
}

bool ClassConstructGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_element(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return(true);
}

bool ClassConstructGenerator::Process_none(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_object(FILE *into, TiXmlElement *field) {
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

bool ClassConstructGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return(true);
}

bool ClassConstructGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return(true);
}

bool ClassConstructGenerator::Process_list(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = NULL;\n", name);
	return(true);
}

bool ClassConstructGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	return(true);
}

bool ClassConstructGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "false";
	fprintf(into, "\t%s = %s;\n", name, def);
	return(true);
}

bool ClassConstructGenerator::Process_int(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "0";
	fprintf(into, "\t%s = %s;\n", name, def);
	return(true);
}

bool ClassConstructGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "0";
	fprintf(into, "\t%s = %s;\n", name, def);
	return(true);
}

bool ClassConstructGenerator::Process_string(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "";
	fprintf(into, "\t%s = \"%s\";\n", name, def);
	return(true);
}

bool ClassConstructGenerator::Process_real(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *def = field->Attribute("default");
	if(def == NULL)
		def = "0.0";
	fprintf(into, "\t%s = %s;\n", name, def);
	return(true);
}


















