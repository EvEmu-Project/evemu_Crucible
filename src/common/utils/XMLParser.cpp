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

#include "CommonPCH.h"

#include "log/LogNew.h"
#include "utils/XMLParser.h"

bool XMLParser::ParseFile( const char* file )
{
	TiXmlDocument doc( file );
	if( !doc.LoadFile() )
    {
        sLog.Error( "XMLParser", "Unable to load '%s': %s.", file, doc.ErrorDesc() );
        return false;
	}

    TiXmlElement* root = doc.RootElement();
	if( root == NULL )
    {
        sLog.Error( "XMLParser", "Unable to find root in %s.", file );
		return false;
	}

    return Recurse( root );
}

bool XMLParser::ParseElement( const TiXmlElement* element )
{
    std::map<std::string, ElementParser>::const_iterator parser = mParsers.find( element->Value() );
    if( mParsers.end() == parser )
        return false;

    /*
     * This is kinda a sketchy operation here, since all of these
     * element handler methods will be functions in child classes.
     * This essentially causes us to do an un-checkable (and hence
     * un-handle-properly-able) cast down to the child class. This
     * WILL BREAK if any children classes do multiple inheritance.
     */
    return ( this->*( parser->second ) )( element );
}

bool XMLParser::Recurse( const TiXmlElement* element )
{
	const TiXmlNode* child = NULL;
	while( ( child = element->IterateChildren( child ) ) )
    {
		if( child->Type() == TiXmlNode::ELEMENT )
        {
		    const TiXmlElement* childElement = child->ToElement();

            if( !ParseElement( childElement ) )
                return false;
        }
	}
	
	return true;
}

const char* XMLParser::ParseTextBlock( const TiXmlNode* within, const char* name, bool optional )
{
	const TiXmlElement* txt = within->FirstChildElement( name );
	if( txt == NULL )
    {
		if( !optional )
            sLog.Error( "XMLParser", "Unable to find element '%s' in element '%s' at line %d.", name, within->Value(), within->Row() );
		return NULL;
	}

    return GetText( txt, optional );
}

const char* XMLParser::GetText( const TiXmlNode* within, bool optional )
{
	const TiXmlNode* contents = within->FirstChild();
	if( contents == NULL || contents->Type() != TiXmlNode::TEXT )
    {
		if( !optional )
            sLog.Error( "XMLParser", "Expected a text element in element '%s' at line %d.", within->Value(), within->Row() );
		return NULL;
	}

	return contents->Value();
}

