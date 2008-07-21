
#include "EncodeGenerator.h"
#include "../common/logsys.h"



ClassEncodeGenerator::ClassEncodeGenerator()
: m_fast(false)
{
	AllProcFMaps(ClassEncodeGenerator);
}

void ClassEncodeGenerator::Process_root(FILE *into, TiXmlElement *element) {
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
		"\n"
		"%s *%s::Encode() {\n"
		"	%s *res = NULL;\n",
		encode_type, name, encode_type);
/*
		"\n"
		"bool %s::Encode(PyRep **in_packet) {\n"
		"	PyRep *packet = *in_packet;\n"
		"	*in_packet = NULL;\n\n",*/

	m_itemNumber = 0;
	while(!m_variableQueue.empty())
		m_variableQueue.pop();
	m_variableQueue.push("res");
	m_name = name;

	m_fast = false;
	if(!ProcessFields(into, element))
		return;

	
	fprintf(into, "\n\treturn(res);\n}\n");

	/*
     * Now build the fast encode
     *
    */
	fprintf(into,
		"\n"
		"%s *%s::FastEncode() {\n"
		"	%s *res = NULL;\n",
		encode_type, name, encode_type);
/*
		"\n"
		"bool %s::Encode(PyRep **in_packet) {\n"
		"	PyRep *packet = *in_packet;\n"
		"	*in_packet = NULL;\n\n",*/

	m_itemNumber = 0;
	while(!m_variableQueue.empty())
		m_variableQueue.pop();
	m_variableQueue.push("res");
	m_name = name;

	m_fast = true;
	if(!ProcessFields(into, element))
		return;

	
	fprintf(into, "\n\treturn(res);\n}\n");
}

bool ClassEncodeGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
	//first, we need to know how many elements this tuple has:
	TiXmlNode *i = NULL;
	int count = 0;
	while( (i = field->IterateChildren( i )) ) {
		if(i->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		count++;
	}

	int num = m_itemNumber++;
	char iname[16];
	snprintf(iname, sizeof(iname), "tuple%d", num);
	//now we can generate the tuple decl
	fprintf(into, "\tPyRepTuple *%s = new PyRepTuple(%d);\n", iname, count);

	//now we need to queue up all the storage locations for the fields
    //need to be backward
	char varname[64];
	for(count--; count >= 0; count--) {
		snprintf(varname, sizeof(varname), "%s->items[%d]", iname, count);
		push(varname);
	}
	
	if(!ProcessFields(into, field))
		return(false);

	fprintf(into, "\t%s = %s;\n\t\n", top(), iname);

	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	//first, we need to know how many elements this tuple has:
	TiXmlNode *i = NULL;
	int count = 0;
	while( (i = field->IterateChildren( i )) ) {
		if(i->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		count++;
	}

	int num = m_itemNumber++;
	char iname[16];
	snprintf(iname, sizeof(iname), "list%d", num);
	//now we can generate the tuple decl
	fprintf(into, 
		"\tPyRepList *%s = new PyRepList();\n"
		"\t%s->items.resize(%d, NULL);\n", iname, iname, count);

	//now we need to queue up all the storage locations for the fields
    //need to be backward
	char varname[64];
	for(count--; count >= 0; count--) {
		snprintf(varname, sizeof(varname), "%s->items[%d]", iname, count);
		push(varname);
	}
	
	if(!ProcessFields(into, field))
		return(false);

	fprintf(into, "\t%s = %s;\n\t\n", top(), iname);

	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	
	//first, create the dict container
	int num = m_itemNumber++;
	char dictname[16];
	snprintf(dictname, sizeof(dictname), "dict%d", num);
	fprintf(into, "\tPyRepDict *%s = new PyRepDict();\n", dictname);
	
	//now we process each element, putting it into the dict:
	TiXmlNode *i = NULL;
	int count = 0;
	char varname[32];
	while((i = field->IterateChildren( i )) ) {
		if(i->Type() == TiXmlNode::COMMENT) {
			TiXmlComment *com = i->ToComment();
			fprintf(into, "\t/* %s */\n", com->Value());
			continue;
		}
		if(i->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about

		TiXmlElement *ele = i->ToElement();
		//we only handle IDEntry elements
		if(std::string("IDEntry") != ele->Value()) {
			_log(COMMON__ERROR, "non-IDEntry in <InlineDict> at line %d, ignoring.", ele->Row());
			continue;
		}
		const char *key = ele->Attribute("key");
		const char *keyType = ele->Attribute("key_type");
		if(key == NULL) {
			_log(COMMON__ERROR, "<IDEntry> at line %d is missing the key attribute, skipping.", ele->Row());
			continue;
		}

		snprintf(varname, sizeof(varname), "dict%d_%d", num, count);
		count++;
		
		fprintf(into, "\tPyRep *%s;\n", varname);
		push(varname);

		//now process the data part, putting the value into `varname`
		if(!ProcessFields(into, ele, 1))
			return(false);

		//now store the result in the dict:
		//taking the keyType into account
		if (keyType != NULL && !strcmp(keyType, "int")) {
			fprintf(into, "\t%s->items[\n"
						  "\t\tnew PyRepInteger(%s)\n"
						  "\t] = %s;\n", dictname, key, varname);
		} else {
			fprintf(into, "\t%s->items[\n"
						  "\t\tnew PyRepString(\"%s\")\n"
						  "\t] = %s;\n", dictname, key, varname);
		}

	}
	fprintf(into, "\t%s = %s;\n\t\n", top(), dictname);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	char varname[16];
	int num = m_itemNumber++;
	snprintf(varname, sizeof(varname), "ss_%d", num);

	//encode the sub-element into a temp
	fprintf(into, "\tPyRep *%s;\n", varname);
	push(varname);
	if(!ProcessFields(into, field, 1))
		return(false);

	//now make a substream from the temp at store it where it is needed
	fprintf(into, "\t%s = new PyRepSubStream(%s);\n", top(), varname);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	char varname[16];
	int num = m_itemNumber++;
	snprintf(varname, sizeof(varname), "ss_%d", num);

	//encode the sub-element into a temp
	fprintf(into, "\tPyRep *%s;\n", varname);
	push(varname);
	if(!ProcessFields(into, field, 1))
		return(false);
	
	//now make a substream from the temp at store it where it is needed
	fprintf(into, "\t%s = new PyRepSubStruct(%s);\n", top(), varname);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	int num = m_itemNumber++;
	char rname[16];
	snprintf(rname, sizeof(rname), "dict%d", num);
	fprintf(into, "\t\n");
	
	fprintf(into, 
		"	PyRepDict *%s = new PyRepDict();\n"
		"	std::map<std::string, PyRep *>::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s->items[\n"
		"			new PyRepString(%s_cur->first)\n",
		rname, 
		name, name,
		name, name,
		name, name,
		name, name, name,
			rname,
				name
		);
	if(m_fast) {
		fprintf(into, 
		"		] = %s_cur->second;\n"
		"	}\n"
		"	%s.clear();\n",
			name,
			name
			);
	} else {
		fprintf(into, 
		"		] = %s_cur->second->Clone();\n"
		"	}\n",
			name
			);
	}
	fprintf(into, 
		"	%s = %s;\n"
		"	\n",
		top(), rname
	);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	int num = m_itemNumber++;
	char rname[16];
	snprintf(rname, sizeof(rname), "dict%d", num);
	fprintf(into, "\t\n");
	
	fprintf(into, 
		"	PyRepDict *%s = new PyRepDict();\n"
		"	std::map<uint32, PyRep *>::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s->items[\n"
		"			new PyRepInteger(%s_cur->first)\n",
		rname, 
		name, name,
		name, name,
		name, name,
		name, name, name,
			rname,
				name
		);
	if(m_fast) {
		fprintf(into, 
		"		] = %s_cur->second;\n"
		"	}\n"
		"	%s.clear();\n",
			name,
			name
			);
	} else {
		fprintf(into, 
		"		] = %s_cur->second->Clone();\n"
		"	}\n",
			name
			);
	}
	fprintf(into, 
		"	%s = %s;\n"
		"	\n",
		top(), rname
	);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
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
	
	int num = m_itemNumber++;
	char rname[16];
	snprintf(rname, sizeof(rname), "dict%d", num);
	fprintf(into, "\t\n");
	
	fprintf(into, 
		"	PyRepDict *%s = new PyRepDict();\n"
		"	std::map<%s, %s>::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s->items[\n"
		"			new PyRep%s(%s_cur->first)\n"
		"		] = new PyRep%s(%s_cur->second);\n"
		"	}\n"
		"	%s = %s;\n"
		"	\n",
		rname, 
		key, value, name, name,
		name, name,
		name, name,
		name, name, name,
			rname,
				pykey, name,
			pyvalue, name,
		top(), rname
	);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	int num = m_itemNumber++;
	char rname[16];
	snprintf(rname, sizeof(rname), "list%d", num);
	fprintf(into, "\t\n");
	
	fprintf(into, 
		"	PyRepList *%s = new PyRepList();\n"
		"	std::vector<std::string>::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s->items.push_back(\n"
		"			new PyRepString(*%s_cur)\n"
		"		);\n"
		"	}\n"
		"	%s = %s;\n"
		"	\n",
		rname, 
		name, name, name,
		name, name, name,
		name, name, name,
		rname, name,
		top(), rname
	);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	int num = m_itemNumber++;
	char rname[16];
	snprintf(rname, sizeof(rname), "list%d", num);
	fprintf(into, "\t\n");
	
	fprintf(into, 
		"	PyRepList *%s = new PyRepList();\n"
		"	std::vector<uint32>::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s->items.push_back(\n"
		"			new PyRepInteger(*%s_cur)\n"
		"		);\n"
		"	}\n"
		"	%s = %s;\n"
		"	\n",
		rname, 
		name, name, name,
		name, name, name,
		name, name, name,
		rname, name,
		top(), rname
	);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	int num = m_itemNumber++;
	char rname[16];
	snprintf(rname, sizeof(rname), "list%d", num);
	fprintf(into, "\t\n");
	
	fprintf(into, 
		"	PyRepList *%s = new PyRepList();\n"
		"	std::vector<uint64>::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s.begin();\n"
		"	%s_end = %s.end();\n"
		"	for(; %s_cur != %s_end; %s_cur++) {\n"
		"		%s->items.push_back(\n"
		"			new PyRepInteger(*%s_cur)\n"
		"		);\n"
		"	}\n"
		"	%s = %s;\n"
		"	\n",
		rname, 
		name, name, name,
		name, name, name,
		name, name, name,
		rname, name,
		top(), rname
	);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_element(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	if(m_fast) {
		fprintf(into,
			"		%s = %s.FastEncode();\n", top(), name);
	} else {
		fprintf(into,
			"		%s = %s.Encode();\n", top(), name);
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	if(%s == NULL) {\n"
		"		_log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyRepNone\");\n"
		"		%s = new PyRepNone();\n"
		"	} else {\n",
		name,
			m_name, name,
			top()
		);
	if(m_fast) {
		fprintf(into,
			"		%s = %s->FastEncode();\n"
			"	}\n", top(), name);
	} else {
		fprintf(into,
			"		%s = %s->Encode();\n"
			"	}\n", top(), name);
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_none(FILE *into, TiXmlElement *field) {
	fprintf(into, "\t%s = new PyRepNone();\n", top());
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_object(FILE *into, TiXmlElement *field) {
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}
	
	int num = m_itemNumber++;
	char iname[16];
	snprintf(iname, sizeof(iname), "args%d", num);
	//now we can generate the tuple decl
	fprintf(into, "\tPyRep *%s;\n", iname);

	push(iname);
	
	if(!ProcessFields(into, field, 1))
		return(false);

	fprintf(into,
		"	%s = new PyRepObject(\n"
		"			\"%s\",\n"
		"			%s\n"
		"		);\n"
		"	\n", top(), type, iname);
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	if(%s == NULL) {\n"
		"		_log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty buffer.\");\n"
		"		%s = new PyRepBuffer(0);\n"
		"	}\n",
		name,
			m_name, name,
			name
		);
	if(m_fast) {
		fprintf(into, "\t%s = %s;\n"
					  "\t%s = NULL;\n", top(), name, name);
	} else {
		fprintf(into, "\t%s = %s->Clone();\n", top(), name);
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, 
		"	if(%s == NULL) {\n"
		"		_log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyRepNone\");\n"
		"		%s = new PyRepNone();\n"
		"	}\n",
		name,
			m_name, name,
			name
		);
	if(m_fast) {
		fprintf(into, "\t%s = %s;\n"
					  "\t%s = NULL;\n", top(), name, name);
	} else {
		fprintf(into, "\t%s = %s->Clone();\n", top(), name);
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *optional = field->Attribute("optional");
	bool is_optional = false;
	if(optional != NULL && std::string("true") == optional)
		is_optional = true;
	if(is_optional) {
		fprintf(into, "	if(%s.empty()) {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n", 
			name,
				top());
	}
	
	if(m_fast) {
		int num = m_itemNumber++;
		char rname[16];
		snprintf(rname, sizeof(rname), "list%d", num);
		fprintf(into, "\t\n");
		
		fprintf(into, 
			"	PyRepList *%s = new PyRepList();\n"
			"	%s->items = %s.items;\n"	//steal the items
			"	%s.items.clear();\n"
			"	%s = %s;\n",
			rname,
			rname, name,
			name,
			top(), rname);
	} else {
		fprintf(into, "\t%s = %s.Clone();\n", top(), name);
	}
	
	if(is_optional) {
		fprintf(into, "	}\n" );
	}
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	fprintf(into, 
		"	if(%s == NULL) {\n"
		"		_log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty tuple.\");\n"
		"		%s = new PyRepTuple(0);\n"
		"	}\n",
		name,
			m_name, name,
			name
		);
	
	const char *optional = field->Attribute("optional");
	bool is_optional = false;
	if(optional != NULL && std::string("true") == optional)
		is_optional = true;
	if(is_optional) {
		fprintf(into, "	if(%s->empty()) {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n", 
			name,
				top());
	}
	
	if(m_fast) {
		int num = m_itemNumber++;
		char rname[16];
		snprintf(rname, sizeof(rname), "list%d", num);
		fprintf(into, "\t\n");
		
		fprintf(into, 
			"	PyRepTuple *%s = new PyRepTuple(0);\n"
			"	%s->items = %s->items;\n"	//steal the items
			"	%s->items.clear();\n"
			"	%s = %s;\n",
			rname,
			rname, name,
			name,
			top(), rname);
	} else {
		fprintf(into, "\t%s = %s->Clone();\n", top(), name);
	}
	
	if(is_optional) {
		fprintf(into, "	}\n" );
	}
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *optional = field->Attribute("optional");
	bool is_optional = false;
	if(optional != NULL && std::string("true") == optional)
		is_optional = true;
	if(is_optional) {
		fprintf(into, "	if(%s->empty()) {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n", 
			name,
				top());
	}
	
	if(m_fast) {
		int num = m_itemNumber++;
		char rname[16];
		snprintf(rname, sizeof(rname), "dict%d", num);
		fprintf(into, "\t\n");
		
		fprintf(into, 
			"	PyRepDict *%s = new PyRepDict();\n"
			"	%s->items = %s.items;\n"	//steal the items
			"	%s.items.clear();\n"
			"	%s = %s;\n",
			rname,
			rname, name,
			name,
			top(), rname);
	} else {
		fprintf(into, "\t%s = %s.Clone();\n", top(), name);
	}
	
	if(is_optional) {
		fprintf(into, "	}\n" );
	}
	
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	fprintf(into, "\t%s = new PyRepBoolean(%s);\n", top(), name);
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_int(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	if(none_marker != NULL) {
		fprintf(into, "	if(%s == %s) {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n"
					  "		%s = new PyRepInteger(%s);\n"
					  "	}\n",
			name, none_marker,
				top(), 
				top(), name);
	} else {
		fprintf(into, "\t%s = new PyRepInteger(%s);\n", top(), name);
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	if(none_marker != NULL) {
		fprintf(into, "	if(%s == %s) {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n"
					  "		%s = new PyRepInteger(%s);\n"
					  "	}\n",
			name, none_marker,
				top(), 
				top(), name);
	} else {
		fprintf(into, "\t%s = new PyRepInteger(%s);\n", top(), name);
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_string(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	std::string type1_s = "";
	const char *type1 = field->Attribute("type1");
	if(type1 != NULL && std::string("true") == type1) {
		type1_s = ", ";
		type1_s += type1;
	}
	
	const char *none_marker = field->Attribute("none_marker");
	if(none_marker != NULL) {
		fprintf(into, "	if(%s == \"%s\") {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n"
					  "		%s = new PyRepString(%s%s);\n"
					  "	}\n",
			name, none_marker,
				top(), 
				top(), name, type1_s.c_str());
	} else {
		fprintf(into, "\t%s = new PyRepString(%s%s);\n", top(), name, type1_s.c_str());
	}
	pop();
	return(true);
}

bool ClassEncodeGenerator::Process_real(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	if(none_marker != NULL) {
		fprintf(into, "	if(%s == %s) {\n"
					  "		%s = new PyRepNone();\n"
					  "	} else {\n"
					  "		%s = new PyRepReal(%s);\n"
					  "	}\n",
			name, none_marker,
				top(), 
				top(), name);
	} else {
		fprintf(into, "\t%s = new PyRepReal(%s);\n", top(), name);
	}
	pop();
	return(true);
}

















