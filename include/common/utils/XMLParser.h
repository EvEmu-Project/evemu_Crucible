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

#ifndef __XML_PARSER_H__INCL__
#define __XML_PARSER_H__INCL__

#include "log/LogNew.h"

/**
 * @brief Utility for parsing XML files.
 *
 * See note in XMLParser::ParseElement() before inheriting this class.
 *
 * @author Zhur, Bloody.Rabbit
 */
template<typename _Self>
class XMLParser
{
public:
    /**
     * @brief Parses file using registered parsers.
     *
     * @param[in] file File to parse.
     *
     * @retval true  Parsing successfull.
     * @retval false Error occurred during parsing.
     */
    bool ParseFile( const char* file )
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
            sLog.Error( "XMLParser", "Unable to find root in '%s'.", file );
            return false;
        }

        return ParseElement( root );
    }

    /**
     * @brief Parses element using registered parsers.
     *
     * @param[in] element Element to be parsed.
     *
     * @retval true  Parsing successfull.
     * @retval false Error occurred during parsing.
     */
    bool ParseElement( const TiXmlElement* element )
    {
        typename std::map<std::string, ElementParser>::const_iterator res = mParsers.find( element->Value() );
        if( mParsers.end() == res )
        {
            sLog.Error( "XMLParser", "Unknown element '%s' at line %d.", element->Value(), element->Row() );
            return false;
        }

        /*
         * This is kinda a sketchy operation here, since all of these
         * element handler methods will be functions in child classes.
         * This essentially causes us to do an un-checkable (and hence
         * un-handle-properly-able) cast down to the child class. This
         * WILL BREAK if any children classes do multiple inheritance.
         */
        return ( static_cast<self_t*>( this )->*( res->second ) )( element );
    }

    /**
     * @brief Parses element's children using registered parsers.
     *
     * @param[in] element Element the children of which should be parsed.
     * @param[in] max     The maximal count of children to be processed; 0 means all.
     *
     * @retval true  Parsing successfull.
     * @retval false Error occurred during parsing.
     */
    bool ParseElementChildren( const TiXmlElement* element, size_t max = 0 )
    {
        const TiXmlNode* child = NULL;

        size_t count = 0;
        while( ( child = element->IterateChildren( child ) ) )
        {
            if( child->Type() == TiXmlNode::ELEMENT )
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

protected:
    /** Type of our child which is deriving from us. */
    typedef _Self self_t;
    /** Type of element parsers. */
    typedef bool ( self_t::* ElementParser )( const TiXmlElement* ele );

    /**
     * @brief Registers a parser.
     *
     * @param[in] name   Name of element to be parsed by the parser.
     * @param[in] parser The parser itself.
     */
    void RegisterParser( const char* name, const ElementParser& parser )
    {
        mParsers[ name ] = parser;
    }

private:
    /** Parser storage. */
    std::map<std::string, ElementParser> mParsers;
};


#endif /* !__XML_PARSER_H__INCL__ */




