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

#ifndef __XML_PARSER_H__INCL__
#define __XML_PARSER_H__INCL__

/**
 * @brief Utility for parsing XML files.
 *
 * @author Zhur, Bloody.Rabbit
 */
class XMLParser
{
public:
    /**
     * @brief This virtual interface must be implemented by all parsers.
     *
     * @author Bloody.Rabbit
     */
    class ElementParser
    {
    public:
        /**
         * @brief Parses an element.
         *
         * @param[in] field The element to be parsed.
         *
         * @retval true  Parsing successful.
         * @retval false Parsing failed.
         */
        virtual bool Parse( const TiXmlElement* field ) = 0;
    };

    /**
     * @brief Primary constructor.
     */
    XMLParser();
    /**
     * @brief A destructor.
     */
    ~XMLParser();

    /**
     * @brief Parses file using registered parsers.
     *
     * @param[in] file File to parse.
     *
     * @retval true  Parsing successful.
     * @retval false Error occurred during parsing.
     */
    bool ParseFile( const char* file ) const;
    /**
     * @brief Parses element using registered parsers.
     *
     * @param[in] element Element to be parsed.
     *
     * @retval true  Parsing successful.
     * @retval false Error occurred during parsing.
     */
    bool ParseElement( const TiXmlElement* element ) const;
    /**
     * @brief Parses element's children using registered parsers.
     *
     * @param[in] element Element the children of which should be parsed.
     * @param[in] max     The maximal count of children to be processed; 0 means all.
     *
     * @retval true  Parsing successful.
     * @retval false Error occurred during parsing.
     */
    bool ParseElementChildren( const TiXmlElement* element, size_t max = 0 ) const;

    /**
     * @brief Adds a parser.
     *
     * @param[in] name   Name of element to be parsed by the parser.
     * @param[in] parser The parser itself.
     */
    void AddParser( const char* name, ElementParser* parser );
    /**
     * @brief Removes a parser.
     *
     * @param[in] name Name of element to be parsed by the parser.
     */
    void RemoveParser( const char* name );
    /**
     * @brief Clears all parsers.
     */
    void ClearParsers();

private:
    /** Parser storage. */
    std::map<std::string, ElementParser*> mParsers;
};

#endif /* !__XML_PARSER_H__INCL__ */




