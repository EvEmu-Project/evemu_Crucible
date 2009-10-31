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

/**
 * @brief Utility for parsing XML files.
 *
 * See note in XMLParser::ParseFile() before inheriting this class.
 *
 * @author Zhur, Bloody.Rabbit
 */
class XMLParser
{
public:
    /**
     * @brief Parses file using registered parsers.
     *
     * @param[in] file     File to parse.
     * @param[in] root_ele Root element of file to be parsed.
     *
     * @retval true  Parsing successfull.
     * @retval false Error occurred during parsing.
     */
    bool ParseFile( const char* file, const char* root_ele );
	
protected:
    /**
     * @brief Registers a parser.
     *
     * @param[in] name   Name of element to be parsed by the parser.
     * @param[in] parser The parser itself.
     */
    template<typename X>
    void RegisterParser( const char* name, bool ( X::* parser )( TiXmlElement* ele ) )
    {
        mParsers[ name ] = static_cast<ElementParser>( parser );
    }

    /**
     * @brief Extracts text from given node.
     *
     * Function finds first child of given name and gives
     * it to function GetText.
     *
     * @param[in] within   The node to extract the text from.
     * @param[in] name     Name of child to extract the text from.
     * @param[in] optional If set to true, no error is printed when
     *                     an error occurs.
     *
     * @return Extracted text; NULL if an error occurred.
     */
	static const char* ParseTextBlock( TiXmlNode* within, const char* name, bool optional = false );
    /**
     * @brief Extracts text from given node.
     *
     * Function obtains first child and if it's text element,
     * returns its content.
     *
     * @param[in] within   The node to extract text from.
     * @param[in] optional If set to true, no error is printed when
     *                     an error occurs.
     *
     * @return Extracted text; NULL if an error occurred.
     */
	static const char* GetText( TiXmlNode* within, bool optional = false );

private:
    /** Type of element parsers (casted). */
    typedef bool ( XMLParser::* ElementParser )( TiXmlElement* ele );
    /** Parser storage. */
    std::map<std::string, ElementParser> mParsers;
};


#endif /* !__XML_PARSER_H__INCL__ */




