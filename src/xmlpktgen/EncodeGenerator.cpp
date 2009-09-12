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
#include "EncodeGenerator.h"
#include "../common/logsys.h"
#include "MiscFunctions.h"

ClassEncodeGenerator::ClassEncodeGenerator()
: mFast( false ),
  mItemNumber( 0 ),
  mName( NULL )
{
    AllGenProcRegs( ClassEncodeGenerator );
}

bool ClassEncodeGenerator::Process_elementdef(FILE *into, TiXmlElement *element)
{
    mName = element->Attribute("name");
    if(mName == NULL) {
        _log(COMMON__ERROR, "<element> at line %d is missing the name attribute, skipping.", element->Row());
        return false;
    }

    TiXmlElement* main = element->FirstChildElement();
    if(main->NextSiblingElement() != NULL) {
        _log(COMMON__ERROR, "<element> at line %d contains more than one root element. skipping.", element->Row());
        return false;
    }

    const char *encode_type = GetEncodeType(element);
    fprintf(into,
        "%s* %s::Encode() const\n"
		"{\n"
        "    %s* res = NULL;\n"
		"\n",
        encode_type, mName,
		    encode_type
	);
/*
        "\n"
        "bool %s::Encode(PyRep **in_packet) {\n"
        "   PyRep *packet = *in_packet;\n"
        "   *in_packet = NULL;\n\n",
*/

    mItemNumber = 0;
	clear();

    mFast = false;

	push( "res" );
    if(!Recurse(into, element))
        return false;


    fprintf(into,
		"    return res;\n"
		"}\n"
		"\n"
	);

    /*
     * Now build the fast encode
     *
    */
    fprintf(into,
        "%s* %s::FastEncode()\n"
		"{\n"
        "    %s* res = NULL;\n"
		"\n",
        encode_type, mName,
		    encode_type
	);
/*
        "\n"
        "bool %s::Encode(PyRep **in_packet) {\n"
        "   PyRep *packet = *in_packet;\n"
        "   *in_packet = NULL;\n\n",
*/

    mItemNumber = 0;
	clear();

    mFast = true;

    push("res");
    if(!Recurse(into, element))
        return false;

    fprintf(into,
		"    return res;\n"
		"}\n"
		"\n"
	);

	return true;
}

bool ClassEncodeGenerator::Process_InlineTuple(FILE *into, TiXmlElement *field) {
    //first, we need to know how many elements this tuple has:
    TiXmlNode* i = NULL;
    uint32 count = 0;
    while( (i = field->IterateChildren( i )) ) {
        if(i->Type() == TiXmlNode::ELEMENT)
            count++;
    }

    char iname[16];
    snprintf( iname, sizeof(iname), "tuple%u", mItemNumber++ );

    //now we can generate the tuple decl
    fprintf(into,
		"    PyTuple* %s = new PyTuple( %u );\n"
		"\n",
		iname, count
	);

    //now we need to queue up all the storage locations for the fields
    //need to be backward
    char varname[64];
    while( count-- > 0 )
	{
        snprintf(varname, sizeof(varname), "%s->items[ %u ]", iname, count);
        push(varname);
    }

    if(!Recurse(into, field))
        return false;

    fprintf(into,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);


    pop();
    return true;
}

bool ClassEncodeGenerator::Process_InlineList(FILE *into, TiXmlElement *field) {
    //first, we need to know how many elements this tuple has:
    TiXmlNode* i = NULL;
    uint32 count = 0;
    while( (i = field->IterateChildren( i )) ) {
        if(i->Type() == TiXmlNode::ELEMENT)
            count++;
    }

    char iname[16];
    snprintf(iname, sizeof(iname), "list%u", mItemNumber++);

    //now we can generate the list decl
    fprintf(into,
		"    PyList* %s = new PyList( %u );\n"
		"\n",
		iname, count
	);

    //now we need to queue up all the storage locations for the fields
    //need to be backward
    char varname[64];
    while( count-- > 0 )
	{
        snprintf( varname, sizeof(varname), "%s->items[ %u ]", iname, count );
        push(varname);
    }

    if(!Recurse(into, field))
        return false;

    fprintf(into,
		"    %s = %s;\n"
		"\n",
		top(), iname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_InlineDict(FILE *into, TiXmlElement *field) {

    //first, create the dict container
	uint32 item_num = mItemNumber++;
    char dictname[16];
    snprintf(dictname, sizeof(dictname), "dict%u", item_num);

    fprintf(into,
		"    PyDict* %s = new PyDict;\n"
		"\n",
		dictname
	);

    //now we process each element, putting it into the dict:
    TiXmlNode* i = NULL;
    uint32 count = 0;
    char varname[32];
    while( ( i = field->IterateChildren( i ) ) )
	{
        if(i->Type() == TiXmlNode::COMMENT) {
            TiXmlComment *com = i->ToComment();
            fprintf(into, "\t/* %s */\n", com->Value());
            continue;
        }
        if(i->Type() != TiXmlNode::ELEMENT)
            continue;   //skip crap we dont care about

        TiXmlElement* ele = i->ToElement();
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

        snprintf(varname, sizeof(varname), "dict%u_%u", item_num, count);
        count++;

        fprintf(into,
			"    PyRep* %s;\n"
			"\n",
			varname
		);
        push(varname);

        //now process the data part, putting the value into `varname`
        if(!Recurse(into, ele, 1))
            return false;

        //now store the result in the dict:
        //taking the keyType into account
        if( keyType != NULL && !strcmp( keyType, "int" ) )
            fprintf(into,
			    "    %s->SetItem(\n"
                "        new PyInt( %s ), %s\n"
                "    );\n"
				"\n",
				dictname,
				key, varname
			);
        else
            fprintf(into,
			     "     %s->SetItemString(\n"
                 "          \"%s\", %s\n"
                 "     );\n"
				 "\n",
				 dictname,
				 key, varname
			);
    }

    fprintf(into,
		"     %s = %s;\n"
		"\n",
		top(), dictname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_InlineSubStream(FILE *into, TiXmlElement *field) {
    char varname[16];
    snprintf(varname, sizeof(varname), "ss_%d", mItemNumber++);

    //encode the sub-element into a temp
    fprintf(into,
		"    PyRep* %s;\n"
		"\n",
		varname
	);

    push(varname);
    if(!Recurse(into, field, 1))
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf(into,
		"    %s = new PySubStream( %s );\n"
		"\n",
		top(), varname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_InlineSubStruct(FILE *into, TiXmlElement *field) {
    char varname[16];
    snprintf(varname, sizeof(varname), "ss_%d", mItemNumber++);

    //encode the sub-element into a temp
    fprintf(into,
		"    PyRep* %s;\n"
		"\n",
		varname
	);

    push(varname);
    if(!Recurse(into, field, 1))
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf(into,
		"    %s = new PySubStruct( %s );\n"
		"\n",
		top(), varname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_strdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "dict%d", mItemNumber++);

    fprintf(into,
        "    PyDict* %s = new PyDict;\n"
        "    std::map<std::string, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n",
        rname,
        name, name,
        name, name,
        name, name
    );

    if( mFast )
        fprintf(into,
            "    for(; %s_cur != %s_end; %s_cur++)\n"
            "        %s->SetItemString(\n"
			"            %s_cur->first.c_str(), %s_cur->second\n"
			"        );\n"
            "    %s.clear();\n",
            name, name, name,
                rname,
                    name,
                name,
            name
        );
	else
        fprintf(into,
            "    for(; %s_cur != %s_end; %s_cur++)\n"
            "        %s->SetItemString(\n"
			"            %s_cur->first.c_str(), %s_cur->second->Clone()\n"
			"        );\n",
            name, name, name,
                rname,
                    name,
                name
        );

    fprintf(into,
        "    %s = %s;\n"
        "\n",
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_intdict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "dict%u", mItemNumber++);

    fprintf(into,
        "    PyDict* %s = new PyDict;\n"
        "    std::map<int32, PyRep*>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n",
        rname,
        name, name,
        name, name,
        name, name
    );

    if( mFast )
        fprintf(into,
            "    for(; %s_cur != %s_end; %s_cur++)\n"
            "        %s->SetItem(\n"
            "            new PyInt( %s_cur->first ), %s_cur->second\n"
			"        );\n"
            "    %s.clear();\n",
            name, name, name,
                rname,
                    name,
                name,
            name
        );
    else
        fprintf(into,
            "    for(; %s_cur != %s_end; %s_cur++)\n"
            "        %s->SetItem(\n"
            "            new PyInt( %s_cur->first ), %s_cur->second->Clone()\n"
			"        );\n",
            name, name, name,
                rname,
                    name,
                name
        );

    fprintf(into,
        "    %s = %s;\n"
        "\n",
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_primdict(FILE *into, TiXmlElement *field) {
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

    char rname[16];
    snprintf(rname, sizeof(rname), "dict%u", mItemNumber++);

    fprintf(into,
        "    PyDict* %s = new PyDict;\n"
        "    std::map<%s, %s>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->SetItem(\n"
        "            new Py%s( %s_cur->first ), new Py%s( %s_cur->second )\n"
		"        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        key, value, name, name,
        name, name,
        name, name,
        name, name, name,
            rname,
                pykey, name, pyvalue, name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_strlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

    fprintf(into,
        "    PyList* %s = new PyList;\n"
        "    std::vector<std::string>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->AddItemString(\n"
		"            %s_cur->c_str()\n"
        "        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name, name,
		name, name,
		name, name,
        name, name, name,
        rname,
		    name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_intlist(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

    fprintf(into,
        "    PyList* %s = new PyList;\n"
        "    std::vector<int32>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->AddItemInt(\n"
        "            *%s_cur\n"
        "        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name, name,
		name, name,
		name, name,
        name, name, name,
        rname,
		    name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_int64list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

    fprintf(into,
        "    PyList *%s = new PyList;\n"
        "    std::vector<int64>::const_iterator %s_cur, %s_end;\n"
        "    %s_cur = %s.begin();\n"
        "    %s_end = %s.end();\n"
        "    for(; %s_cur != %s_end; %s_cur++)\n"
        "        %s->AddItemLong(\n"
        "            *%s_cur\n"
        "        );\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name, name,
		name, name,
		name, name,
        name, name, name,
        rname,
		    name,
        top(), rname
    );

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_element(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    if( mFast )
        fprintf(into,
            "    %s = %s.FastEncode();\n"
			"\n",
			top(), name
		);
    else
        fprintf(into,
            "    %s = %s.Encode();\n"
			"\n",
			top(), name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_elementptr(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    fprintf(into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\");\n"
        "        %s = new PyNone();\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            top()
    );

    if( mFast )
        fprintf(into,
            "        %s = %s->FastEncode();\n"
			"\n",
			top(), name
		);
    else
        fprintf(into,
            "        %s = %s->Encode();\n"
			"\n",
			top(), name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_none(FILE *into, TiXmlElement *field) {
    fprintf(into,
		"    %s = new PyNone();\n"
		"\n",
		top()
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_object(FILE *into, TiXmlElement *field) {
    const char *type = field->Attribute("type");
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute, skipping.", field->Row());
        return false;
    }

    char iname[16];
    snprintf(iname, sizeof(iname), "args%u", mItemNumber++);

    //now we can generate the args decl
    fprintf(into,
		"    PyRep* %s;\n"
		"\n",
		iname
	);

    push(iname);
    if(!Recurse(into, field, 1))
        return false;

    fprintf(into,
        "    %s = new PyObject(\n"
        "             \"%s\", %s\n"
        "         );\n"
        "\n",
		top(),
		    type, iname
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_object_ex(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }
	const char* type = field->Attribute( "type" );
    if(type == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the type attribute.", field->Row());
        return false;
    }

	bool optional = false;
	const char *optional_str = field->Attribute( "optional" );
	if( optional_str != NULL )
		optional = atobool( optional_str );

	const char *v = top();
	if( optional )
	{
		fprintf( into,
			"    if( %s == NULL )\n"
			"        %s = new PyNone;\n"
			"    else\n",
			name,
				v
		);
	}
	else
	{
		fprintf(into,
			"    if( %s == NULL )\n"
			"    {\n"
			"        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\");\n"
			"        %s = new PyNone();\n"
			"    }\n"
			"    else\n",
			name,
				mName, name,
				v
		);
	}

	if( mFast )
	{
		fprintf( into,
			"    {\n"
			"        %s = %s;\n"
			"        %s = NULL;\n"
			"    }\n"
			"\n",
			v, name,
			name
		);
	}
	else
	{
		fprintf( into,
			"        %s = new %s( *%s );\n"
			"\n",
			v, type, name
		);
	}

	pop();
    return true;
}

bool ClassEncodeGenerator::Process_buffer(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

	const char* v = top();
    fprintf(into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty buffer.\");\n"
        "        %s = new PyBuffer( 0 );\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            v
    );

    if( mFast )
        fprintf(into,
		    "    {\n"
		    "        %s = %s;\n"
            "        %s = NULL;\n"
			"    }\n"
			"\n",
			v, name,
			name
		);
    else
        fprintf(into,
		    "        %s = %s->Clone();\n"
			"\n",
			v, name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_raw(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

	const char* v = top();
    fprintf(into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in a PyNone\");\n"
        "        %s = new PyNone();\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            v
    );

    if( mFast )
        fprintf(into,
		    "    {\n"
		    "        %s = %s;\n"
            "        %s = NULL;\n"
			"    }\n"
			"\n",
			v, name,
			name
		);
	else
        fprintf(into,
		    "        %s = %s->Clone();\n"
			"\n",
			v, name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_list(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if( optional_str != NULL )
        optional = atobool( optional_str );

	const char* v = top();
    if( optional )
	{
        fprintf(into,
			"    if( %s.empty() )\n"
            "        %s = new PyNone;\n"
			"    else\n",
            name,
                v
		);
    }

    if( mFast )
	{
        char rname[16];
        snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

        fprintf(into,
			"    {\n"
            "        PyList* %s = new PyList;\n"
			"\n"
            "        %s->items = %s.items;\n"    //steal the items
            "        %s.items.clear();\n"
			"\n"
            "        %s = %s;\n"
			"    }\n"
			"\n",
			rname,
            rname, name,
            name,
            v, rname
		);
    }
	else
        fprintf(into,
		    "        %s = new PyList( %s );\n"
			"\n",
			v, name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_tuple(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if( optional_str != NULL )
        optional = atobool( optional_str );

	const char* v = top();
    fprintf(into,
        "    if( %s == NULL )\n"
		"    {\n"
        "        _log(NET__PACKET_ERROR, \"Encode %s: %s is NULL! hacking in an empty tuple.\");\n"
        "        %s = new PyTuple( 0 );\n"
        "    }\n"
		"    else\n",
        name,
            mName, name,
            v
    );

    if( optional )
        fprintf(into,
		    "    if( %s->empty() )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
		);

    if( mFast )
	{
        char rname[16];
        snprintf(rname, sizeof(rname), "tuple%u", mItemNumber++);

        fprintf(into,
			"    {\n"
            "        PyTuple* %s = new PyTuple( 0 );\n"
			"\n"
            "        %s->items = %s->items;\n"   //steal the items
            "        %s->items.clear();\n"
			"\n"
            "        %s = %s;\n"
			"    }\n"
			"\n",
            rname,
            rname, name,
            name,
            v, rname
		);
	}
    else
        fprintf(into,
		    "        %s = new PyTuple( *%s );\n"
			"\n",
			v, name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_dict(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if( optional_str != NULL )
        optional = atobool( optional_str );

	const char* v = top();
    if( optional )
        fprintf(into,
		    "    if( %s.empty() )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name,
                v
		);

    if(mFast)
	{
        char rname[16];
        snprintf(rname, sizeof(rname), "dict%u", mItemNumber++);

        fprintf(into,
			"    {\n"
            "        PyDict* %s = new PyDict;\n"
			"\n"
            "        %s->items = %s.items;\n"    //steal the items
            "        %s.items.clear();\n"
			"\n"
            "        %s = %s;\n"
			"    }\n"
			"\n",
            rname,
            rname, name,
            name,
            v, rname
		);
    } else
        fprintf(into,
		    "        %s = new PyDict( %s );\n"
			"\n",
			v, name
		);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_bool(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    fprintf(into,
		"    %s = new PyBool( %s );\n"
		"\n",
		top(), name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_int(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    //this should be done better:
    const char* none_marker = field->Attribute("none_marker");

	const char* v = top();
    if( none_marker != NULL )
        fprintf(into,
			"    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf(into,
	    "    %s = new PyInt( %s );\n"
		"\n",
	    v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_int64(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    //this should be done better:
    const char* none_marker = field->Attribute("none_marker");

	const char* v = top();
    if( none_marker != NULL )
        fprintf(into,
		    "    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );

    fprintf(into,
		"    %s = new PyLong( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_string(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

	bool type1 = false;
    const char* type1_str = field->Attribute("type1");
	if( type1_str != NULL )
		type1 = atobool( type1_str );

    const char* none_marker = field->Attribute("none_marker");

	const char* v = top();
    if( none_marker != NULL )
	{
        fprintf(into,
			"    if( %s == \"%s\" )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
		);
    }

    fprintf(into,
		"    %s = new PyString( %s, %s );\n"
		"\n",
		v, name, type1 ? "true" : "false"
	);

    pop();
    return true;
}

bool ClassEncodeGenerator::Process_real(FILE *into, TiXmlElement *field) {
    const char *name = field->Attribute("name");
    if(name == NULL) {
        _log(COMMON__ERROR, "field at line %d is missing the name attribute, skipping.", field->Row());
        return false;
    }

    //this should be done better:
    const char* none_marker = field->Attribute("none_marker");

	const char* v = top();
    if( none_marker != NULL )
	{
        fprintf(into, 
			"    if( %s == %s )\n"
            "        %s = new PyNone;\n"
            "    else\n",
            name, none_marker,
                v
        );
    }

    fprintf(into,
		"    %s = new PyFloat( %s );\n"
		"\n",
		v, name
	);

    pop();
    return true;
}

















