/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
*/

#include "eve-xmlpktgen.h"

#include "EncodeGenerator.h"

ClassEncodeGenerator::ClassEncodeGenerator(FILE* outputFile)
: Generator(outputFile),
  mItemNumber(0),
  mName(nullptr)
{
    RegisterProcessors();
}

bool ClassEncodeGenerator::ProcessElementDef(const TiXmlElement* field)
{
    mName = field->Attribute("name");
    if (mName == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessElementDef <name> at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const TiXmlElement* main = field->FirstChildElement();
    if (main->NextSiblingElement() != nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessElementDef <element> at line " << field->Row() << " contains more than one root element, skipping.";
        return false;
    }

    const char* encode_type = GetEncodeType(main);
    fprintf(mOutputFile,
        "%s* %s::Encode() const\n"
        "{\n"
        "    %s* res(nullptr);\n"
        "\n",
        encode_type, mName,
            encode_type
   );

    mItemNumber = 0;
    clear();

    push("res");
    if (!ParseElement(main))
        return false;

    fprintf(mOutputFile,
        "    return res;\n"
        "}\n"
        "\n"
   );

    return true;
}

bool ClassEncodeGenerator::ProcessElement(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessElement <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf(mOutputFile,
        "    %s = %s.Encode();\n"
        "\n",
        top(), name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessElementPtr(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessElementPtr <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* v = top();
    fprintf(mOutputFile,
        "    if (%s != nullptr) {\n"
        "        %s = %s->Encode();\n"
        "    } else {\n"
        "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null. Encoding a PyNone\");\n"
        "        %s = PyStatic.NewNone();\n"
        "    }\n"
        "\n",
        name, v, name,
        mName, name, v
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessRaw(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessRaw <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* v = top();
    fprintf(mOutputFile,
        "    if (%s != nullptr) {\n"
        "        PyIncRef(%s);\n"
        "        %s = %s;\n"
        "    } else {\n"
        "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding a PyNone\");\n"
        "        %s = PyStatic.NewNone();\n"
        "    }\n"
        "\n",
        name,
            name,
            v, name,
            mName, name,
            v
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessInt(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessInt <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* none_marker = field->Attribute("none_marker");
    const char* v = top();
    if (none_marker != nullptr)
        fprintf(mOutputFile,
                "    if (%s == %s) {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
                name, none_marker,
                v
       );

    fprintf(mOutputFile,
            "    %s = new PyInt(%s);\n",
            v, name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessLong(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessLong <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* none_marker = field->Attribute("none_marker");
    const char* v = top();
    if (none_marker != nullptr)
        fprintf(mOutputFile,
                 "    if (%s == %s) {\n"
                 "        %s = PyStatic.NewNone();\n"
                 "    } else\n",
                 name, none_marker,
                 v
       );

    fprintf(mOutputFile,
             "    %s = new PyLong(%s);\n",
             v, name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessReal(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessReal <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* none_marker = field->Attribute("none_marker");
    const char* v = top();
    if (none_marker != nullptr)
        fprintf(mOutputFile,
                "    if (%s == %s) {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
                name, none_marker,
                v
       );

    fprintf(mOutputFile,
            "    %s = new PyFloat(%s);\n",
            v, name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessBool(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessBool <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    fprintf(mOutputFile,
        "        %s = new PyBool(%s);\n",
        top(), name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessNone(const TiXmlElement* field)
{
    fprintf(mOutputFile,
        "    %s = PyStatic.NewNone();\n",
        top()
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessBuffer(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessBuffer <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* v = top();
    fprintf(mOutputFile,
        "    if (%s != nullptr) {\n"
        "        PyIncRef(%s);\n"
        "        %s = %s;\n"
        "    } else {\n"
        "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding an empty buffer.\");\n"
        "        %s = new PyBuffer(0);\n"
        "    }\n"
        "\n",
        name,
            name,
            v, name,
            mName, name,
            v
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessString(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl << "EncodeGen::ProcessString <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const char* none_marker = field->Attribute("none_marker");
    const char* v = top();
    if (none_marker != nullptr)
        fprintf(mOutputFile,
                "    if (%s == \"%s\") {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
                name, none_marker,
                v
       );

    fprintf(mOutputFile,
            "    %s = new PyString(%s);\n",
            v, name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessStringInline(const TiXmlElement* field)
{
    const char* value = field->Attribute("value");
    if (value == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessStringInline String element at line " << field->Row() << " has no value attribute, skipping.";
        return false;
    }

    const char* v = top();
    fprintf(mOutputFile,
        "    %s = new PyString(\"%s\");\n",
        v, value
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessWString(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessWString <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* none_marker = field->Attribute("none_marker");
    const char* v = top();
    if (none_marker != nullptr)
        fprintf(mOutputFile,
                "    if (%s == \"%s\") {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
                name, none_marker,
                v
       );

    fprintf(mOutputFile,
            "    %s = new PyWString(%s);\n",
            v, name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessWStringInline(const TiXmlElement* field)
{
    const char* value = field->Attribute("value");
    if (value == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessWStringInline WString element at line " << field->Row() << " has no value attribute, skipping.";
        return false;
    }

    const char* v = top();
    fprintf(mOutputFile,
            "    %s = new PyWString(\"%s\", %zu);\n",
            v, value, strlen(value)
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessToken(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessToken <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    bool optional(false);
    const char* optional_str = field->Attribute("optional");
    if (optional_str != nullptr)
        optional = str2<bool>(optional_str);

    const char* v = top();
    if (optional) {
        fprintf(mOutputFile,
                "    if (%s == nullptr) {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
                name,
                v
       );
    } else {
        fprintf(mOutputFile,
                "    if (%s == nullptr) {\n"
                "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding a PyNone\");\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
                name,
                mName, name,
                v
       );
    }

    fprintf(mOutputFile,
             "    {\n"
             "        %s = %s;\n"
             "        PyIncRef(%s);\n"
             "    }\n"
             "\n",
             v, name,
             name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessTokenInline(const TiXmlElement* field)
{
    const char* value = field->Attribute("value");
    if (value == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessTokenInline Token element at line " << field->Row() << " has no type attribute, skipping.";
        return false;
    }

    const char* v = top();
    fprintf(mOutputFile,
        "    %s = new PyToken(\"%s\");\n",
        v, value
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessObject(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessObject  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if (optional_str != nullptr)
        optional = str2<bool>(optional_str);

    const char* v = top();
    if (optional) {
        fprintf(mOutputFile,
                "    if (%s == nullptr) {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
            name,
                v
       );
    } else {
        fprintf(mOutputFile,
            "    if (%s == nullptr) {\n"
            "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding a PyNone\");\n"
            "        %s = PyStatic.NewNone();\n"
            "    } else\n",
            name,
                mName, name,
            v
       );
    }

    fprintf(mOutputFile,
             "    {\n"
             "        %s = %s;\n"
             "        PyIncRef(%s);\n"
             "    }\n"
             "\n",
             v, name,
             name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessObjectInline(const TiXmlElement* field)
{
    const uint32 num = mItemNumber++;

    char tname[32];
    snprintf(tname, sizeof(tname), "obj%u_type", num);

    char aname[32];
    snprintf(aname, sizeof(aname), "obj%u_args", num);

    fprintf(mOutputFile,
        "    PyString* %s(nullptr);\n"
        "    PyRep* %s(nullptr);\n"
        "\n",
        tname,
        aname
   );

    push(aname);
    push(tname);

    if (!ParseElementChildren(field, 2))
        return false;

    const char* v = top();
    fprintf(mOutputFile,
        "    %s = new PyObject(%s, %s);\n",
        v, tname, aname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessObjectEx(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessObjectEx  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* type = field->Attribute("type");
    if (type == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessObjectEx  <name> field at line " << field->Row() << " is missing the type attribute, skipping.";
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if (optional_str != nullptr)
        optional = str2<bool>(optional_str);

    const char *v = top();
    if (optional) {
        fprintf(mOutputFile,
                "    if (%s == nullptr) {\n"
                "        %s = PyStatic.NewNone();\n"
                "    } else\n",
            name,
                v
       );
    } else {
        fprintf(mOutputFile,
            "    if (%s == nullptr) {\n"
            "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding a PyNone\");\n"
            "        %s = PyStatic.NewNone();\n"
            "    } else\n",
            name,
                mName, name,
                v
       );
    }

    fprintf(mOutputFile,
             "    {\n"
             "        %s = %s;\n"
             "        PyIncRef(%s);\n"
             "    }\n"
             "\n",
             v, name,
             name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessTuple(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessTuple  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if (optional_str != nullptr)
        optional = str2<bool>(optional_str);

    const char* v = top();
    fprintf(mOutputFile,
        "    if (%s == nullptr) {\n"
        "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding an empty tuple.\");\n"
        "        %s = new PyTuple(0);\n"
        "    } else\n",
        name,
            mName, name,
            v
   );

    if (optional)
        fprintf(mOutputFile,
            "    if (%s->empty()) {\n"
            "        %s = PyStatic.NewNone();\n"
            "    } else\n",
            name,
                v
       );

    fprintf(mOutputFile,
             "    {\n"
             "        %s = %s;\n"
             "        PyIncRef(%s);\n"
             "    }\n"
             "\n",
             v, name,
             name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessTupleInline(const TiXmlElement* field)
{
    //first, we need to know how many elements this tuple has:
    const TiXmlNode* i = nullptr;

    uint32 count = 0;
    while((i = field->IterateChildren(i)))
    {
        if (i->Type() == TiXmlNode::TINYXML_ELEMENT)
            count++;
    }

    char iname[16];
    snprintf(iname, sizeof(iname), "tuple%u", mItemNumber++);

    //now we can generate the tuple decl
    fprintf(mOutputFile,
        "    PyTuple* %s = new PyTuple(%u);\n",
        iname, count
   );

    //now we need to queue up all the storage locations for the fields
    //need to be backward
    char varname[64];
    while(count-- > 0)
    {
        snprintf(varname, sizeof(varname), "%s->items[ %u ]", iname, count);
        push(varname);
    }

    if (!ParseElementChildren(field))
        return false;

    fprintf(mOutputFile,
        "    %s = %s;\n"
        "\n",
        top(), iname
   );


    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessList(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessList  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if (optional_str != nullptr)
        optional = str2<bool>(optional_str);

    const char* v = top();
    fprintf(mOutputFile,
        "    if (%s == nullptr) {\n"
        "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding an empty list.\");\n"
        "        %s = new PyList();\n"
        "    } else\n",
        name,
            mName, name,
            v
   );

    if (optional)
        fprintf(mOutputFile,
            "    if (%s->empty()) {\n"
            "        %s = PyStatic.NewNone();\n"
            "    } else\n",
            name,
                v
       );

    fprintf(mOutputFile,
             "    {\n"
             "        %s = %s;\n"
             "        PyIncRef(%s);\n"
             "    }\n"
             "\n",
             v, name,
             name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessListInline(const TiXmlElement* field)
{
    //first, we need to know how many elements this tuple has:
    const TiXmlNode* i = nullptr;

    uint32 count = 0;
    while((i = field->IterateChildren(i)))
    {
        if (i->Type() == TiXmlNode::TINYXML_ELEMENT)
            count++;
    }

    char iname[16];
    snprintf(iname, sizeof(iname), "list%u", mItemNumber++);

    //now we can generate the list decl
    fprintf(mOutputFile,
        "    PyList* %s = new PyList(%u);\n",
        iname, count
   );

    //now we need to queue up all the storage locations for the fields
    //need to be backward
    char varname[64];
    while(count-- > 0)
    {
        snprintf(varname, sizeof(varname), "%s->items[ %u ]", iname, count);
        push(varname);
    }

    if (!ParseElementChildren(field))
        return false;

    fprintf(mOutputFile,
        "    %s = %s;\n"
        "\n",
        top(), iname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessListInt(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessListInt  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

    fprintf(mOutputFile,
        "    PyList* %s = new PyList();\n"
        "    for (auto cur : %s)\n"
        "        %s->AddItemInt(cur);\n"
        "    %s = %s;\n"
        "\n",
        rname, name, rname,
        top(), rname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessListLong(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessListLong  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

    fprintf(mOutputFile,
        "    PyList *%s = new PyList();\n"
        "    for (auto cur : %s)\n"
        "        %s->AddItemLong(cur);\n"
        "    %s = %s;\n"
        "\n",
        rname, name,
        rname,
        top(), rname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessListStr(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessListStr  <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "list%u", mItemNumber++);

    fprintf(mOutputFile,
             "    PyList *%s = new PyList();\n"
             "    for (auto cur : %s)\n"
             "        %s->AddItemString(cur.c_str());\n"
             "    %s = %s;\n"
             "\n",
             rname, name,
             rname,
             top(), rname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessDict(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDict <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    bool optional = false;
    const char* optional_str = field->Attribute("optional");
    if (optional_str != nullptr)
        optional = str2<bool>(optional_str);

    const char* v = top();
    fprintf(mOutputFile,
        "    if (%s == nullptr) {\n"
        "        _log(NET__PACKET_WARNING, \"Encode %s: %s is null.  Encoding an empty dict.\");\n"
        "        %s = new PyDict();\n"
        "    } else\n",
        name,
            mName, name,
            v
   );

    if (optional)
        fprintf(mOutputFile,
            "    if (%s->empty()) {\n"
            "        %s = PyStatic.NewNone();\n"
            "    } else\n",
            name,
                v
       );

    fprintf(mOutputFile,
             "    {\n"
             "        %s = %s;\n"
             "        PyIncRef(%s);\n"
             "    }\n"
             "\n",
             v, name,
             name
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessDictInline(const TiXmlElement* field)
{
    //first, create the dict container
    char iname[16];
    snprintf(iname, sizeof(iname), "dict%u", mItemNumber++);

    fprintf(mOutputFile,
        "    PyDict* %s = new PyDict();\n",
        iname
   );

    //now we process each element, putting it into the dict:
    const TiXmlNode* i = nullptr;

    uint32 count = 0;
    while ((i = field->IterateChildren(i))) {
        if (i->Type() == TiXmlNode::TINYXML_ELEMENT) {
            const TiXmlElement* ele = i->ToElement();

            //we only handle dictInlineEntry elements
            if (strcmp(ele->Value(), "dictInlineEntry") != 0)            {
                std::cout << std::endl <<  "EncodeGen::ProcessDictInline non-dictInlineEntry in <dictInline> at line " << field->Row() << ", ignoring.";
                continue;
            }
            const char* key = ele->Attribute("key");
            if (key == nullptr) {
                std::cout << std::endl <<  "EncodeGen::ProcessDictInline <dictInlineEntry> at line " << field->Row() << " is missing the key attribute, skipping.";
                return false;
            }

            bool keyTypeInt = false, keyTypeLong = false;
            const char* keyType = ele->Attribute("key_type");
            if (keyType != nullptr) {
                keyTypeInt = (strcmp(keyType, "int") == 0);
                keyTypeLong = (strcmp(keyType, "long") == 0);
            }

            char vname[32];
            snprintf(vname, sizeof(vname), "%s_%u", iname, count);
            ++count;

            fprintf(mOutputFile,
                "    PyRep* %s(nullptr);\n",
                vname
           );
            push(vname);

            //now process the data part, putting the value into `varname`
            if (!ParseElementChildren(ele, 1))
                return false;

            //now store the result in the dict:
            //taking the keyType into account
            if (keyTypeInt) {
                fprintf(mOutputFile,
                         "    %s->SetItem(new PyInt(%s), %s);\n"
                         "    PyIncRef(%s);\n",
                         iname, key, vname,
                         vname
               );
            } else if (keyTypeLong) {
                    fprintf(mOutputFile,
                             "    %s->SetItem(new PyLong(%s), %s);\n"
                             "    PyIncRef(%s);\n",
                             iname, key, vname,
                             vname
                   );
            } else {
                fprintf(mOutputFile,
                         "    %s->SetItemString(\"%s\", %s);\n"
                         "    PyIncRef(%s);\n",
                         iname, key, vname,
                         vname
               );
            }
        }
    }

    fprintf(mOutputFile,
        "    %s = %s;\n"
        "\n",
        top(), iname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessDictRaw(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictRaw <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* key = field->Attribute("key");
    if (key == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictRaw <key> field at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }
    const char* pykey = field->Attribute("pykey");
    if (pykey == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictRaw <pykey> field at line " << field->Row() << " is missing the pykey attribute, skipping.";
        return false;
    }
    const char* value = field->Attribute("value");
    if (value == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictRaw <value> field at line " << field->Row() << " is missing the value attribute, skipping.";
        return false;
    }
    const char* pyvalue = field->Attribute("pyvalue");
    if (pyvalue == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictRaw <pyvalue> field at line " << field->Row() << " is missing the pyvalue attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf(rname, sizeof(rname), "dict%u", mItemNumber++);

    fprintf(mOutputFile,
        "    PyDict* %s = new PyDict();\n"
        "    for (auto cur : %s) \n"
        "        %s->SetItem(new Py%s(cur.first), new Py%s(cur.second));\n"
        "    %s = %s;\n"
        "\n",
        rname,
        name,
            rname, pykey, pyvalue,
        top(), rname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessDictInt(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictInt <name> field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char iname[16];
    snprintf(iname, sizeof(iname), "dict%u", mItemNumber++);

    fprintf(mOutputFile,
        "    PyDict* %s = new PyDict();\n"
        "    for (auto cur : %s) {\n"
        "        PyIncRef(cur.second);\n"
        "        %s->SetItem(new PyInt(cur.first), cur.second);\n"
        "    }\n"
        "\n"
        "    %s = %s;\n",
        iname,
        name,
            iname,

        top(), iname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessDictStr(const TiXmlElement* field)
{
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "EncodeGen::ProcessDictStr field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char iname[16];
    snprintf(iname, sizeof(iname), "dict%d", mItemNumber++);

    fprintf(mOutputFile,
        "    PyDict* %s = new PyDict();\n"
        "    for (auto cur : %s) {\n"
        "        PyIncRef(cur.second);\n"
        "        %s->SetItemString(cur.first.c_str(), cur.second);\n"
        "    }\n"
        "\n"
        "    %s = %s;\n",
        iname,
        name,
            iname,

        top(), iname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessSubStreamInline(const TiXmlElement* field)
{
    char varname[16];
    snprintf(varname, sizeof(varname), "ss_%d", mItemNumber++);

    //encode the sub-element into a temp
    fprintf(mOutputFile,
        "    PyRep* %s;\n",
        varname
   );

    push(varname);
    if (!ParseElementChildren(field, 1))
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf(mOutputFile,
        "    %s = new PySubStream(%s);\n",
        top(), varname
   );

    pop();
    return true;
}

bool ClassEncodeGenerator::ProcessSubStructInline(const TiXmlElement* field)
{
    char varname[16];
    snprintf(varname, sizeof(varname), "ss_%d", mItemNumber++);

    //encode the sub-element into a temp
    fprintf(mOutputFile,
        "    PyRep* %s;\n",
        varname
   );

    push(varname);
    if (!ParseElementChildren(field, 1))
        return false;

    //now make a substream from the temp at store it where it is needed
    fprintf(mOutputFile,
        "    %s = new PySubStruct(%s);\n",
        top(), varname
   );

    pop();
    return true;
}
