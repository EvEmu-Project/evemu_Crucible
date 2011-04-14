/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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

#include "CommonPCH.h"

#include "log/LogNew.h"
#include "utils/XMLParser.h"

/************************************************************************/
/* XMLParser                                                            */
/************************************************************************/
XMLParser::XMLParser()
{
}

XMLParser::~XMLParser()
{
    ClearParsers();
}

bool XMLParser::ParseFile( const char* file ) const
{
    TiXmlDocument doc( file );
    if( !doc.LoadFile() )
    {
        sLog.Error( "XMLParser", "Unable to load '%s': %s.", file, doc.ErrorDesc() );
        return false;
    }

    TiXmlElement* root = doc.RootElement();
    if( NULL == root )
    {
        sLog.Error( "XMLParser", "Unable to find root in '%s'.", file );
        return false;
    }

    return ParseElement( root );
}

bool XMLParser::ParseElement( const TiXmlElement* element ) const
{
    std::map<std::string, ElementParser*>::const_iterator res = mParsers.find( element->Value() );
    if( mParsers.end() == res )
    {
        sLog.Error( "XMLParser", "Unknown element '%s' at line %d.", element->Value(), element->Row() );
        return false;
    }

    return res->second->Parse( element );
}

bool XMLParser::ParseElementChildren( const TiXmlElement* element, size_t max ) const
{
    const TiXmlNode* child = NULL;

    size_t count = 0;
    while( ( child = element->IterateChildren( child ) ) )
    {
        if( TiXmlNode::ELEMENT == child->Type() )
        {
            const TiXmlElement* childElement = child->ToElement();

            if( 0 < max && max <= count )
            {
                sLog.Error( "XMLParser", "Maximal children count %lu exceeded"
                                         " in element '%s' at line %d"
                                         " by element '%s' at line %d.",
                                         max,
                                         element->Value(), element->Row(),
                                         childElement->Value(), childElement->Row() );

                return false;
            }

            if( !ParseElement( childElement ) )
                return false;

            ++count;
        }
    }

    return true;
}

void XMLParser::AddParser( const char* name, ElementParser* parser )
{
    mParsers[ name ] = parser;
}

void XMLParser::RemoveParser( const char* name )
{
    std::map<std::string, ElementParser*>::iterator res = mParsers.find( name );
    if( mParsers.end() != res )
    {
        SafeDelete( res->second );
        mParsers.erase( res );
    }
}

void XMLParser::ClearParsers()
{
    std::map<std::string, ElementParser*>::iterator cur, end;
    cur = mParsers.begin();
    end = mParsers.end();
    for(; cur != end; ++cur )
        SafeDelete( cur->second );

    mParsers.clear();
}

