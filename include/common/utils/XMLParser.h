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
         * @retval true  Parsing successfull.
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
     * @retval true  Parsing successfull.
     * @retval false Error occurred during parsing.
     */
    bool ParseFile( const char* file ) const;
    /**
     * @brief Parses element using registered parsers.
     *
     * @param[in] element Element to be parsed.
     *
     * @retval true  Parsing successfull.
     * @retval false Error occurred during parsing.
     */
    bool ParseElement( const TiXmlElement* element ) const;
    /**
     * @brief Parses element's children using registered parsers.
     *
     * @param[in] element Element the children of which should be parsed.
     * @param[in] max     The maximal count of children to be processed; 0 means all.
     *
     * @retval true  Parsing successfull.
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

private:
    /** Parser storage. */
    std::map<std::string, ElementParser*> mParsers;
};

/**
 * @brief A somewhat extended version of XMLParser.
 *
 * @author Bloody.Rabbit
 */
class XMLParserEx
: public XMLParser
{
public:
    /**
     * @brief An implementation of ElementParser for member method parsers.
     *
     * @author Bloody.Rabbit
     */
    template<typename T>
    class MemberElementParser
    : public ElementParser
    {
    public:
        /** Type of class. */
        typedef T Class;
        /** Type of method. */
        typedef bool ( Class::* Method )( const TiXmlElement* );

        /**
         * @brief Primary constructor.
         *
         * @param[in] instance Instance of class.
         * @param[in] method   Member method.
         */
        MemberElementParser( Class* instance, const Method& method )
        : mInstance( instance ),
          mMethod( method )
        {
        }

        /**
         * @brief Invokes parser method.
         *
         * @param[in] field The element to be parsed.
         *
         * @retval true  Parsing successfull.
         * @retval false Parsing failed.
         */
        bool Parse( const TiXmlElement* field )
        {
            return ( mInstance->*mMethod )( field );
        }

    protected:
        /** The instance that the method should be invoked upon. */
        Class* mInstance;
        /** The parser method. */
        Method mMethod;
    };

    /**
     * @brief Adds a member parser.
     *
     * @param[in] name     A name of element which the parser should parse.
     * @param[in] instance Instance of parser class.
     * @param[in] method   Parser method.
     */
    template<typename T>
    void AddMemberParser( const char* name, T* instance, bool ( T::* method )( const TiXmlElement* ) )
    {
        AddParser( name, new MemberElementParser<T>( instance, method ) );
    }

protected:
    /**
     * @brief Adds a member parser, assuming that instance is @a this.
     *
     * @param[in] name   A name of element which the parser should parse.
     * @param[in] method Parser method.
     */
    template<typename T>
    void AddMemberParser( const char* name, bool ( T::* method )( const TiXmlElement* ) )
    {
        AddMemberParser<T>( name, static_cast< T* >( this ), method );
    }
};

#endif /* !__XML_PARSER_H__INCL__ */




