
#include "DecodeGenerator.h"
#include "../common/logsys.h"

#ifndef WIN32
#warning Decoder is not properly freeing old items in the case of object re-use
#endif

ClassDecodeGenerator::ClassDecodeGenerator()
: m_itemNumber(0),
  m_name(NULL)
{
	AllProcFMaps(ClassDecodeGenerator);
}

void ClassDecodeGenerator::Process_root(FILE *into, TiXmlElement *element) {
	const char *name = element->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
		return;
	}

	fprintf(into,
		"\n"
		"bool %s::Decode(%s **in_packet) {\n"
		"	//quick forwarder to avoid making the user cast it if they have a properly typed object\n"
		"	PyRep *packet = *in_packet;\n"
		"	*in_packet = NULL;\n"
		"	return(Decode(&packet));\n"
		"}\n"
		"\n"
		"bool %s::Decode(PyRep **in_packet) {\n"
		"	PyRep *packet = *in_packet;\n"
		"	*in_packet = NULL;\n\n",
		name, GetEncodeType(element), name);

	m_itemNumber = 0;
	m_variableQueue.push("packet");
	m_name = name;
	
	if(!ProcessFields(into, element))
		return;

	m_name = NULL;
	
	fprintf(into, "\n"
				  "	delete packet;\n"
				  "	return(true);\n}\n");
}

bool ClassDecodeGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
	//first, we need to know how many elements this tuple has:
	TiXmlNode *i = NULL;
	int count = 0;
	while( (i = field->IterateChildren( i )) ) {
		if(i->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		count++;
	}

	const char *v = top();
	
	int num = m_itemNumber++;
	char iname[16];
	snprintf(iname, sizeof(iname), "tuple%d", num);
	//now we can generate the tuple decl
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Tuple)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepTuple *%s = (PyRepTuple *) %s;\n"
		"	if(%s->items.size() != %d) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong size: expected %d, but got %%d\", %s->items.size());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"\n",
		v, 
			m_name, iname, v, 
		iname, v, 
		iname, count, 
			m_name, iname, count, iname
	);

	//now we need to queue up all the storage locations for the fields
    //need to be backward
	char varname[64];
	for(count--; count >= 0; count--) {
		snprintf(varname, sizeof(varname), "%s->items[%d]", iname, count);
		push(varname);
	}
	
	if(!ProcessFields(into, field))
		return(false);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
	//first, we need to know how many elements this tuple has:
	TiXmlNode *i = NULL;
	int count = 0;
	while( (i = field->IterateChildren( i )) ) {
		if(i->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		count++;
	}

	const char *v = top();
	
	int num = m_itemNumber++;
	char iname[16];
	snprintf(iname, sizeof(iname), "list%d", num);
	//now we can generate the tuple decl
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::List)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepList *%s = (PyRepList *) %s;\n"
		"	if(%s->items.size() != %d) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong size: expected %d, but got %%d\", %s->items.size());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"\n",
		v, 
			m_name, iname, v, 
		iname, v, 
		iname, count, 
			m_name, iname, count, iname
	);

	//now we need to queue up all the storage locations for the fields
    //need to be backward
	char varname[64];
	for(count--; count >= 0; count--) {
		snprintf(varname, sizeof(varname), "%s->items[%d]", iname, count);
		push(varname);
	}
	
	if(!ProcessFields(into, field))
		return(false);
	
	pop();
	return(true);
}

/*
 *this function could be improved quite a bit by not parsing and checking the
 *IDEntry elements over and over again
 *
*/
bool ClassDecodeGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {
	TiXmlNode *i = NULL;
	
	int num = m_itemNumber++;
	char iname[16];
	snprintf(iname, sizeof(iname), "dict%d", num);
	const char *v = top();

	//make sure its a dict
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Dict)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}"
		"\n",
		v, m_name, iname, v
	);

	bool nonempty = false;
	//now generate the "found" flags for each expected element.
	while( (i = field->IterateChildren( i )) ) {
		if(i->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		TiXmlElement *ele = i->ToElement();
		//we only handle IDEntry elements
		if(std::string("IDEntry") != ele->Value()) {
			_log(COMMON__ERROR, "non-IDEntry in <InlineDict> at line %d, ignoring.", ele->Row());
			continue;
		}
		TiXmlElement *val = ele->FirstChildElement();
		if(val == NULL) {
			_log(COMMON__ERROR, "<IDEntry> at line %d lacks a value element", ele->Row());
			return(false);
		}
		const char *vname = val->Attribute("name");
		if(vname == NULL) {
			_log(COMMON__ERROR, "<IDEntry>'s value element at line %d lacks a name", val->Row());
			return(false);
		}
		fprintf(into,
			"\tbool %s_%s = false;\n", iname, vname);
		nonempty = true;
	}

	if(!nonempty) {
		fprintf(into, "	//%s is empty from our perspective, not enforcing though.\n", iname);
	} else {
		//setup the loop...
		fprintf(into, 
			"	PyRepDict *%s = (PyRepDict *) %s;\n"
			"	\n"
			"	PyRepDict::iterator %s_cur, %s_end;\n"
			"	%s_cur = %s->items.begin();\n"
			"	%s_end = %s->items.end();\n"
			"	for(; %s_cur != %s_end; %s_cur++) {\n"
			"		PyRep *key__ = %s_cur->first;\n"
			"		if(!key__->CheckType(PyRep::String)) {\n"
			"			_log(NET__PACKET_ERROR, \"Decode %s failed: a key in %s is the wrong type: %%s\", key__->TypeString());\n"
			"			delete packet;\n"
			"			return(false);\n"
			"		}\n"
			"		PyRepString *key_string__ = (PyRepString *) key__;\n"
			"		\n",
			iname, v, iname, iname, iname, iname, iname, iname, iname, iname, iname, iname, m_name, iname
		);

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
			if(key == NULL) {
				_log(COMMON__ERROR, "<IDEntry> at line %d lacks a key attribute", ele->Row());
				return(false);
			}
			TiXmlElement *val = ele->FirstChildElement();
			if(val == NULL) {
				_log(COMMON__ERROR, "<IDEntry> at line %d lacks a value element", ele->Row());
				return(false);
			}
			const char *vname = val->Attribute("name");
			if(vname == NULL) {
				_log(COMMON__ERROR, "<IDEntry>'s value element at line %d lacks a name", val->Row());
				return(false);
			}
	
			//conditional prefix...
			fprintf(into, 
				"		if(key_string__->value == \"%s\") {\n"
				"			%s_%s = true;\n",
				key, iname, vname );
			
			//now process the data part, putting the value into `varname`
            //TODO: figure out a reasonable way to fix the indention here...
			char vvname[64];
			snprintf(vvname, sizeof(vvname), "%s_cur->second", iname);
			push(vvname);
			
			if(!ProcessFields(into, ele, 1))
				return(false);
			//fixed suffix...
			fprintf(into, 
				"		} else\n");
		}

		if(field->Attribute("soft") == NULL || field->Attribute("soft") != std::string("true")) {
			fprintf(into, 
				"		{\n"
				"			_log(NET__PACKET_ERROR, \"Decode %s failed: Unknown key string '%%s' in %s\", key_string__->value.c_str());\n"
				"			delete packet;\n"
				"			return(false);\n"
				"		}\n"
				"	}\n"
				"	\n",
				m_name, iname);
		} else {
			fprintf(into, 
				"		{ /* do nothing, soft dict */ }\n"
				"	}\n"
				"	\n" );
		}
		
		//finally, check the "found" flags for each expected element.
		while( (i = field->IterateChildren( i )) ) {
			if(i->Type() != TiXmlNode::ELEMENT)
				continue;	//skip crap we dont care about
			TiXmlElement *ele = i->ToElement();
			//we only handle IDEntry elements
			if(std::string("IDEntry") != ele->Value()) {
				_log(COMMON__ERROR, "non-IDEntry in <InlineDict> at line %d, ignoring.", ele->Row());
				continue;
			}
			TiXmlElement *val = ele->FirstChildElement();
			if(val == NULL) {
				_log(COMMON__ERROR, "<IDEntry> at line %d lacks a value element", ele->Row());
				return(false);
			}
			const char *vname = val->Attribute("name");
			if(vname == NULL) {
				_log(COMMON__ERROR, "<IDEntry>'s value element at line %d lacks a name", val->Row());
				return(false);
			}
			
			fprintf(into, 
				"	if(!%s_%s) {\n"
				"		_log(NET__PACKET_ERROR, \"Decode %s failed: Missing dict entry for '%s' in %s\");\n"
				"		delete packet;\n"
				"		return(false);\n"
				"	}\n"
				"	\n",
				iname, vname, m_name, vname, iname);
		}
	}
	

	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "ss_%d", num);
	const char *v = top();

	//make sure its a substream
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::SubStream)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a substream: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}"
		"	\n"
		"	PyRepSubStream *%s = (PyRepSubStream *) %s;\n"
		"	//make sure its decoded\n"
		"	%s->DecodeData();\n"
		"	if(%s->decoded == NULL) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: Unable to decode %s\");\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	\n",
		v, m_name, iname, v, iname, v, iname, iname, m_name, iname
	);
	char ssname[32];
	snprintf(ssname, sizeof(ssname), "%s->decoded", iname);
	//Decode the sub-element
	push(ssname);
	if(!ProcessFields(into, field, 1))
		return(false);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "ss_%d", num);
	const char *v = top();

	//make sure its a substream
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::SubStruct)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a substruct: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}"
		"	\n"
		"	PyRepSubStruct *%s = (PyRepSubStruct *) %s;\n"
		"	\n",
		v, 
			m_name, iname, v, 
		iname, v
	);
	char ssname[32];
	snprintf(ssname, sizeof(ssname), "%s->sub", iname);
	//Decode the sub-element
	push(ssname);
	if(!ProcessFields(into, field, 1))
		return(false);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "dict_%d", num);
	const char *v = top();
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Dict)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s.clear();\n"
		"	PyRepDict *%s = (PyRepDict *) %s;\n"
		"	PyRepDict::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s->items.begin();\n"
		"	%s_end = %s->items.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		if(!%s_cur->first->CheckType(PyRep::String)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Key %%d in dict %s is not a string: %%s\", %s_index, %s_cur->first->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		PyRepString *k = (PyRepString *) %s_cur->first;\n"
		"		%s[k->value] = %s_cur->second->Clone();\n"
		"	}\n"
		"	\n",
		v,
			m_name, name, v,
		name,
		iname, v,
		name, name,
		name, iname, 
		name, iname,
		name,
		name, name, name, name, name,
			name, 
				m_name, name, name, name, 
			name,
			name, name
	);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "dict_%d", num);
	const char *v = top();
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Dict)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s.clear();\n"
		"	PyRepDict *%s = (PyRepDict *) %s;\n"
		"	PyRepDict::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s->items.begin();\n"
		"	%s_end = %s->items.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		if(!%s_cur->first->CheckType(PyRep::Integer)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Key %%d in dict %s is not an integer: %%s\", %s_index, %s_cur->first->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		PyRepInteger *k = (PyRepInteger *) %s_cur->first;\n"
		"		if(k->value > 0xFFFFFFFFLL) {\n"
		"			_log(NET__PACKET_WARNING, \"Decode %s: truncating 64 bit into into 32 bit int in key of entry %%d in field %s\", %s_index);\n"
		"		}\n"
		"		%s[k->value] = %s_cur->second->Clone();\n"
		"	}\n"
		"	\n",
		v,
			m_name, name, v,
		name,
		iname, v,
		name, name,
		name, iname, 
		name, iname,
		name,
		name, name, name, name, name,
			name, 
				m_name, name, name, name, 
			name,
				m_name, name, name, 
			name, name
	);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
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
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "dict_%d", num);
	const char *v = top();
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Dict)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s.clear();\n"
		"	PyRepDict *%s = (PyRepDict *) %s;\n"
		"	PyRepDict::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s->items.begin();\n"
		"	%s_end = %s->items.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		if(!%s_cur->first->CheckType(PyRep::%s)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Key %%d in dict %s is not %s: %%s\", %s_index, %s_cur->first->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		if(!%s_cur->second->CheckType(PyRep::%s)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Value %%d in dict %s is not %s: %%s\", %s_index, %s_cur->second->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		PyRep%s *k = (PyRep%s *) %s_cur->first;\n"
		"		PyRep%s *v = (PyRep%s *) %s_cur->second;\n"
		"		%s[k->value] = v->value;\n"
		"	}\n"
		"	\n",
		v,
			m_name, name, v,
		name,
		iname, v,
		name, name,
		name, iname, 
		name, iname,
		name,
		name, name, name, name, name,
			name, pykey, 
				m_name, name, pykey, name, name, 
			name, pyvalue, 
				m_name, name, pyvalue, name, name, 
			pykey, pykey, name,
			pyvalue, pyvalue, name,
			name
	);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "list_%d", num);
	const char *v = top();

	
	//make sure its a dict
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::List)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s.clear();\n"
		"	PyRepList *%s = (PyRepList *) %s;\n"
		"	PyRepList::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s->items.begin();\n"
		"	%s_end = %s->items.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		if(!(*%s_cur)->CheckType(PyRep::String)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Element %%d in list %s is not a string: %%s\", %s_index, (*%s_cur)->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		PyRepString *t = (PyRepString *) (*%s_cur);\n"
		"		%s.push_back(t->value);\n"
		"	}\n"
		"\n",
		v, m_name, name, v, name,
		iname, v,
		name, name, name, iname, name, iname,
		name, name, name, name, name, name,
		name, m_name, name, name, name, name, name
	);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "list_%d", num);
	const char *v = top();

	
	//make sure its a dict
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::List)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s.clear();\n"
		"	PyRepList *%s = (PyRepList *) %s;\n"
		"	PyRepList::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s->items.begin();\n"
		"	%s_end = %s->items.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		if(!(*%s_cur)->CheckType(PyRep::Integer)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Element %%d in list %s is not an integer: %%s\", %s_index, (*%s_cur)->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		PyRepInteger *t = (PyRepInteger *) (*%s_cur);\n"
		"		if(t->value > 0xFFFFFFFFLL) {\n"
		"			_log(NET__PACKET_WARNING, \"Decode %s: truncating 64 bit into into 32 bit int for item %%d in field %s\", %s_index);\n"
		"		}\n"
		"		%s.push_back(t->value);\n"
		"	}\n"
		"\n",
		v, m_name, name, v, name,
		iname, v,
		name, name, name, iname, name, iname,
		name, name, name, name, name, name,
		name, m_name, name, name, name, name, 
		m_name, name, name,
		name
	);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "list_%d", num);
	const char *v = top();

	
	//make sure its a dict
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::List)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s.clear();\n"
		"	PyRepList *%s = (PyRepList *) %s;\n"
		"	PyRepList::iterator %s_cur, %s_end;\n"
		"	%s_cur = %s->items.begin();\n"
		"	%s_end = %s->items.end();\n"
		"	int %s_index;\n"
		"	for(%s_index = 0; %s_cur != %s_end; %s_cur++, %s_index++) {\n"
		"		if(!(*%s_cur)->CheckType(PyRep::Integer)) {\n"
		"			_log(NET__PACKET_ERROR, \"Decode %s failed: Element %%d in list %s is not an integer: %%s\", %s_index, (*%s_cur)->TypeString());\n"
		"			delete packet;\n"
		"			return(false);\n"
		"		}\n"
		"		PyRepInteger *t = (PyRepInteger *) (*%s_cur);\n"
		"		%s.push_back(t->value);\n"
		"	}\n"
		"\n",
		v, m_name, name, v, name,
		iname, v,
		name, name, name, iname, name, iname,
		name, name, name, name, name, name,
		name, m_name, name, name, name, name, name
	);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_element(FILE *into, TiXmlElement *field) {
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

	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "rep_%d", num);
	
	const char *v = top();
	fprintf(into, 
		"	PyRep *%s = %s;\n"	//in case it is typed for some reason.
		"	%s = NULL;\n"	//consume it
		"	if(!%s.Decode(&%s)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: unable to decode element %s\");\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	\n",
		iname, v,
		v,
		name, iname,
			m_name, name
		);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
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

	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "rep_%d", num);
	
	const char *v = top();
	fprintf(into, 
		"	PyRep *%s = %s;\n"	//in case it is typed for some reason.
		"	%s = NULL;\n"
		"	delete %s;\n"
		"	%s = new %s;\n"
		"	if(!%s->Decode(&%s)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: unable to decode element %s\");\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	\n",
		iname, v,
		v,
		name,
		name, type,
		name, iname,
			m_name, name
		);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_none(FILE *into, TiXmlElement *field) {
	
	const char *v = top();
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::None)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: expecting a None but got a %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	\n",
		v, m_name, v
		);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_object(FILE *into, TiXmlElement *field) {
	const char *type = field->Attribute("type");
	if(type == NULL) {
		_log(COMMON__ERROR, "object at line %d is missing the type attribute, skipping.", field->Row());
		return(false);
	}

	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "obj_%d", num);
	const char *v = top();

	//make sure its a substream
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Object)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong type: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepObject *%s = (PyRepObject *) %s;\n"
		"	\n"
		"	if(%s->type != \"%s\") {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is the wrong object type. Expected '%s', got '%%s'\", %s->type.c_str());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	\n",
		v, m_name, iname, v, iname, v, iname, type, m_name, iname  , type, iname
	);
	char ssname[32];
	snprintf(ssname, sizeof(ssname), "%s->arguments", iname);
	//Decode the sub-element
	push(ssname);
	if(!ProcessFields(into, field, 1))
		return(false);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *v = top();
	fprintf(into, 
		"	if(%s->CheckType(PyRep::Buffer)) {\n"
		"		%s = (PyRepBuffer *) %s;\n"
		"		%s = NULL;\n"
		"	} else if(%s->CheckType(PyRep::String)) {\n"
		"		PyRepString *__sss = (PyRepString *) %s;\n"
		"		%s = NULL;\n"
		"		%s = new PyRepBuffer((const byte *) __sss->value.c_str(), __sss->value.length());\n"
		"		delete __sss;\n"
		"	} else {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a buffer: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	\n",
			v,
				name, v, 
				v,
			v,
				v, 
				v,
				name,
		
				m_name, name, v
		);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_raw(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *v = top();
	fprintf(into, 
		"	delete %s;\n"
		"	%s = %s;\n"
		"	%s = NULL;\n"
		"	\n",
		name,
		name, v,
		v
		);
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_list(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *v = top();
	
	//this should be done better:
	const char *optional = field->Attribute("optional");
	bool is_optional = false;
	if(optional != NULL && std::string("true") == optional)
		is_optional = true;
	if(is_optional) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s.clear();\n"
			"	} else {\n",
			v,
				name);
		is_optional = true;
	}
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::List)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a list: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepList *list_%s = (PyRepList *) %s;"
		"	%s.items = list_%s->items;\n"
		"	list_%s->items.clear();\n"
		"	\n",
		v, m_name, name, v, 
		name, v,
		name, name,
		name
		);
	
	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *v = top();
	
	//this should be done better:
	const char *optional = field->Attribute("optional");
	bool is_optional = false;
	if(optional != NULL && std::string("true") == optional)
		is_optional = true;
	if(is_optional) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s.clear();\n"
			"	} else {\n",
			v,
				name);
		is_optional = true;
	}
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Tuple)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a tuple: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	%s = (PyRepTuple *) %s;\n"
		"	%s = NULL;\n"
		"	\n",
		v, m_name, name, v, name, v, v
		);
	
	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_dict(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *v = top();
	
	//this should be done better:
	const char *optional = field->Attribute("optional");
	bool is_optional = false;
	if(optional != NULL && std::string("true") == optional)
		is_optional = true;
	if(is_optional) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s.clear();\n"
			"	} else {\n",
			v,
				name);
		is_optional = true;
	}
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Dict)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a dict: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepDict *list_%s = (PyRepDict *) %s;"
		"	%s.items = list_%s->items;\n"
		"	list_%s->items.clear();\n"
		"	\n",
		v, m_name, name, v, 
		name, v,
		name, name,
		name
		);
	
	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_bool(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "bool_%d", num);
	
	const char *v = top();
	
	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	bool is_optional = false;
	if(none_marker != NULL) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s = %s;\n"
			"	} else {\n",
			v,
				name, none_marker);
		is_optional = true;
	}
	
	if(field->Attribute("soft") == NULL || field->Attribute("soft") != std::string("true")) {
		fprintf(into, 
			"	if(!%s->CheckType(PyRep::Boolean)) {\n"
			"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a boolean: %%s\", %s->TypeString());\n"
			"		delete packet;\n"
			"		return(false);\n"
			"	}\n"
			"	PyRepBoolean *%s = (PyRepBoolean *) %s;\n"
			"	%s = %s->value;\n"
			"",
			v, m_name, name, v, iname, v, name, iname
			);
	} else {
			fprintf(into, 
			"	if(%s->CheckType(PyRep::Boolean)) {\n"
			"		PyRepBoolean *%s = (PyRepBoolean *) %s;\n"
			"		%s = %s->value;\n"
			"	} else if(%s->CheckType(PyRep::Integer)) {\n"
			"		PyRepInteger *%s = (PyRepInteger *) %s;\n"
			"		%s = (%s->value != 0);\n"
			"	} else {\n"
			"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a boolean (or int): %%s\", %s->TypeString());\n"
			"		delete packet;\n"
			"		return(false);\n"
			"	}\n"
			"",
			v, 
				iname, v, 
				name, iname,
			v, 
				iname, v, 
				name, iname,
				m_name, name, v
			);
	}
	
	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_int(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	
	const char *v = top();
	
	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	bool is_optional = false;
	if(none_marker != NULL) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s = %s;\n"
			"	} else {\n",
			v,
				name, none_marker);
		is_optional = true;
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "int_%d", num);
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Integer)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not an int: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepInteger *%s = (PyRepInteger *) %s;\n"
		"	if(%s->value > 0xFFFFFFFF) {\n"
		"		_log(NET__PACKET_WARNING, \"Decode %s: truncating 64 bit into into 32 bit int for field %s\");\n"
		"	}\n"
		"	%s = %s->value;\n"
		"",
		v, m_name, name, v, iname, v, iname, m_name, name, 
		name, iname
		);

	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_int64(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *v = top();
	
	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	bool is_optional = false;
	if(none_marker != NULL) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s = %s;\n"
			"	} else {\n",
			v,
				name, none_marker);
		is_optional = true;
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "int64_%d", num);
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Integer)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not an int: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepInteger *%s = (PyRepInteger *) %s;\n"
		"	%s = %s->value;\n"
		"",
		v, m_name, name, v, iname, v, name, iname
		);

	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_string(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *type1 = field->Attribute("type1");
	
	const char *v = top();

	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	bool is_optional = false;
	if(none_marker != NULL) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s = \"%s\";\n"
			"	} else {\n",
			v,
				name, none_marker);
		is_optional = true;
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "string_%d", num);
	
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::String)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a string: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepString *%s = (PyRepString *) %s;\n"
		"	%s = %s->value;\n"
		"",
		v,
			m_name, name, v,
		iname, v,
		name, iname
		);
	if(type1 != NULL) {
		//do a check on the type... however thus far we do not care either 
		// way, so dont fail the whole packet just for this
		fprintf(into, 
			"	if(%s->is_type_1 != %s) {\n"
			"		_log(NET__PACKET_ERROR, \"Decode %s: String type mismatch on %s: expected %%d got %%d. Continuing anyhow.\", %s, %s->is_type_1);\n"
			"	}\n"
			"",
			iname, type1,
				m_name, name, type1, iname
			);
	}
	
	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}

bool ClassDecodeGenerator::Process_real(FILE *into, TiXmlElement *field) {
	const char *name = field->Attribute("name");
	if(name == NULL) {
		_log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
		return(false);
	}
	const char *v = top();

	//this should be done better:
	const char *none_marker = field->Attribute("none_marker");
	bool is_optional = false;
	if(none_marker != NULL) {
		fprintf(into, 
			"	if(%s->CheckType(PyRep::None)) {\n"
			"		%s = %s;\n"
			"	} else {\n",
			v,
				name, none_marker);
		is_optional = true;
	}
	
	char iname[16];
	int num = m_itemNumber++;
	snprintf(iname, sizeof(iname), "real_%d", num);
	
	fprintf(into, 
		"	if(!%s->CheckType(PyRep::Real)) {\n"
		"		_log(NET__PACKET_ERROR, \"Decode %s failed: %s is not a real: %%s\", %s->TypeString());\n"
		"		delete packet;\n"
		"		return(false);\n"
		"	}\n"
		"	PyRepReal *%s = (PyRepReal *) %s;\n"
		"	%s = %s->value;\n"
		"",
		v, m_name, name, v, iname, v, name, iname
		);
	
	if(is_optional) {
		fprintf(into, "	}\n");
	}
	
	pop();
	return(true);
}






















