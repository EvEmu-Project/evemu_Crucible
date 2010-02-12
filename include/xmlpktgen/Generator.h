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
    GenProcDecl( elementDef ); \
    GenProcDecl( element ); \
    GenProcDecl( elementPtr ); \
    GenProcDecl( raw ); \
    GenProcDecl( int ); \
    GenProcDecl( long ); \
    GenProcDecl( real ); \
    GenProcDecl( bool ); \
    GenProcDecl( none ); \
    GenProcDecl( buffer ); \
    GenProcDecl( string ); \
    GenProcDecl( stringInline ); \
    GenProcDecl( wstring ); \
    GenProcDecl( token ); \
    GenProcDecl( tokenInline ); \
    GenProcDecl( object ); \
    GenProcDecl( objectInline ); \
    GenProcDecl( objectEx ); \
    GenProcDecl( tuple ); \
    GenProcDecl( tupleInline ); \
    GenProcDecl( list ); \
    GenProcDecl( listInline ); \
    GenProcDecl( listInt ); \
    GenProcDecl( listLong ); \
    GenProcDecl( listStr ); \
    GenProcDecl( dict ); \
    GenProcDecl( dictInline ); \
    GenProcDecl( dictRaw ); \
    GenProcDecl( dictInt ); \
    GenProcDecl( dictStr ); \
    GenProcDecl( substreamInline ); \
    GenProcDecl( substructInline )

#define AllGenProcRegs( c ) \
    GenProcReg( c, elementDef, PyRep ); \
    GenProcReg( c, element, PyRep ); \
    GenProcReg( c, elementPtr, PyRep ); \
    GenProcReg( c, raw, PyRep ); \
    GenProcReg( c, int, PyInt ); \
    GenProcReg( c, long, PyInt ); \
    GenProcReg( c, real, PyFloat ); \
    GenProcReg( c, bool, PyBool ); \
    GenProcReg( c, none, PyRep ); \
    GenProcReg( c, buffer, PyBuffer ); \
    GenProcReg( c, string, PyString ); \
    GenProcReg( c, stringInline, PyString ); \
    GenProcReg( c, wstring, PyWString ); \
    GenProcReg( c, token, PyToken ); \
    GenProcReg( c, tokenInline, PyToken ); \
    GenProcReg( c, object, PyObject ); \
    GenProcReg( c, objectInline, PyObject ); \
    GenProcReg( c, objectEx, PyObjectEx ); \
    GenProcReg( c, tuple, PyTuple ); \
    GenProcReg( c, tupleInline, PyTuple ); \
    GenProcReg( c, list, PyList ); \
    GenProcReg( c, listInline, PyList ); \
    GenProcReg( c, listInt, PyList ); \
    GenProcReg( c, listLong, PyList ); \
    GenProcReg( c, listStr, PyList ); \
    GenProcReg( c, dict, PyDict ); \
    GenProcReg( c, dictInline, PyDict ); \
    GenProcReg( c, dictRaw, PyDict ); \
    GenProcReg( c, dictInt, PyDict ); \
    GenProcReg( c, dictStr, PyDict ); \
    GenProcReg( c, substreamInline, PySubStream ); \
    GenProcReg( c, substructInline, PySubStruct )



#endif


