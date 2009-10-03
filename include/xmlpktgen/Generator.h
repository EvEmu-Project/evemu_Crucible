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

#ifndef __GENERATOR_H_INCL__
#define __GENERATOR_H_INCL__

template<typename X>
class Generator
{
public:
    typedef bool ( X::*GenProc )( FILE* into, TiXmlElement* field );

	Generator() {}
	virtual ~Generator() {}

    bool Generate( FILE* into, TiXmlElement* root )
    {
        typename std::map<std::string, GenProc>::iterator res = mGenProcs.find( root->Value() );
        if( res == mGenProcs.end() )
		{
            _log(COMMON__ERROR, "Unexpected field type '%s' on line %d", root->Value(), root->Row());
            return false;
        }

        GenProc& p = res->second;
        X* t = (X*) this;

        return ( t->*p )( into, root );
    }

protected:
    bool Recurse( FILE* into, TiXmlElement* root, uint32 max = 0 )
    {
        TiXmlNode* field = NULL;
        uint32 count = 0;

        while( ( field = root->IterateChildren( field ) ) )
        {
            //pass through comments.
            if( field->Type() == TiXmlNode::COMMENT )
            {
                TiXmlComment* comm = field->ToComment();

                fprintf( into, "    /* %s */\n", comm->Value() );
            }
            //handle elements
            else if( field->Type() == TiXmlNode::ELEMENT )
            {
                if( max > 0 && count > max )
                {
                    _log(COMMON__ERROR, "Element at line %d has too many children elements, at most %u expected.", root->Row(), max);
                    return false;
                }
                ++count;

                TiXmlElement* elem = field->ToElement();

                if( !Generate( into, elem ) )
                    return false;
            }
        }

        return true;
    }

	void RegisterGenProc( const char* name, const GenProc& proc )
	{
		mGenProcs[ name ] = proc;
	}
    std::map<std::string, GenProc> mGenProcs;

    const char* GetEncodeType( TiXmlElement* element ) const
	{
		TiXmlElement* main = element->FirstChildElement();
		if( main == NULL )
			return "PyRep";

		std::map<std::string, std::string>::const_iterator res = mEncTypes.find( main->Value() );
		if( res == mEncTypes.end() )
			return "PyRep";

		return res->second.c_str();
	}
	void RegisterEncodeType( const char* name, const char* encodeType )
	{
		mEncTypes[ name ] = encodeType;
	}
    std::map<std::string, std::string> mEncTypes;
};

#define GenProcDecl( t ) \
    bool Process_##t( FILE* into, TiXmlElement* field )

#define GenProcReg( c, ename, type ) \
	RegisterEncodeType( #ename, #type ); \
	RegisterGenProc( #ename, &c::Process_##ename )

#define AllGenProcDecls \
    GenProcDecl(InlineTuple); \
    GenProcDecl(InlineDict); \
    GenProcDecl(InlineList); \
    GenProcDecl(InlineSubStream); \
    GenProcDecl(InlineSubStruct); \
    GenProcDecl(strdict); \
    GenProcDecl(intdict); \
    GenProcDecl(primdict); \
    GenProcDecl(strlist); \
    GenProcDecl(intlist); \
    GenProcDecl(int64list); \
    GenProcDecl(element); \
	GenProcDecl(elementdef); \
    GenProcDecl(elementptr); \
    GenProcDecl(none); \
    GenProcDecl(object); \
    GenProcDecl(object_ex); \
    GenProcDecl(buffer); \
    GenProcDecl(raw); \
    GenProcDecl(dict); \
    GenProcDecl(list); \
    GenProcDecl(tuple); \
    GenProcDecl(int); \
    GenProcDecl(bool); \
    GenProcDecl(int64); \
    GenProcDecl(real); \
    GenProcDecl(string)

#define AllGenProcRegs( c ) \
    GenProcReg(c, InlineTuple, PyTuple); \
    GenProcReg(c, InlineDict, PyDict); \
    GenProcReg(c, InlineList, PyList); \
    GenProcReg(c, InlineSubStream, PySubStream); \
    GenProcReg(c, InlineSubStruct, PySubStruct); \
    GenProcReg(c, strdict, PyDict); \
    GenProcReg(c, intdict, PyDict); \
    GenProcReg(c, primdict, PyDict); \
    GenProcReg(c, strlist, PyList); \
    GenProcReg(c, intlist, PyList); \
    GenProcReg(c, int64list, PyList); \
    GenProcReg(c, element, PyRep); \
	GenProcReg(c, elementdef, PyRep); \
    GenProcReg(c, elementptr, PyRep); \
    GenProcReg(c, none, PyRep); \
    GenProcReg(c, object, PyObject); \
    GenProcReg(c, object_ex, PyObjectEx); \
    GenProcReg(c, buffer, PyBuffer); \
    GenProcReg(c, raw, PyRep); \
    GenProcReg(c, list, PyList); \
    GenProcReg(c, tuple, PyTuple); \
    GenProcReg(c, dict, PyDict); \
    GenProcReg(c, int, PyInt); \
    GenProcReg(c, bool, PyBool); \
    GenProcReg(c, int64, PyInt); \
    GenProcReg(c, real, PyFloat); \
    GenProcReg(c, string, PyString)



#endif


