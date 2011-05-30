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
    Author:     Bloody.Rabbit
*/

#ifndef __UTILS__XML_PARSER_EX_H__INCL__
#define __UTILS__XML_PARSER_EX_H__INCL__

#include "utils/str2conv.h"
#include "utils/XMLParser.h"

/**
 * @brief A somewhat extended version of XMLParser.
 *
 * @author Bloody.Rabbit
 */
class XMLParserEx
: public XMLParser
{
public:
    template< typename T >
    class MemberElementParser;

    template< typename T >
    class ValueParser;

    /**
     * @brief Adds a member parser.
     *
     * @param[in] name     A name of element which the parser should parse.
     * @param[in] instance Instance of parser class.
     * @param[in] method   Parser method.
     */
    template< typename T >
    void AddMemberParser( const char* name, T& instance, bool ( T::* method )( const TiXmlElement* ) )
    {
        AddParser( name, new MemberElementParser< T >( instance, method ) );
    }

    /**
     * @brief Adds a value parser.
     *
     * @param[in] name  A name of element which the parser should parse.
     * @param[in] value A variable to which the result is stored.
     */
    template< typename T >
    void AddValueParser( const char* name, T& value )
    {
        AddParser( name, new ValueParser< T >( value ) );
    }

protected:
    /**
     * @brief Adds a member parser, assuming that instance is @a this.
     *
     * @param[in] name   A name of element which the parser should parse.
     * @param[in] method Parser method.
     */
    template< typename T >
    void AddMemberParser( const char* name, bool ( T::* method )( const TiXmlElement* ) )
    {
        AddMemberParser< T >( name, static_cast< T& >( *this ), method );
    }
};

/**
 * @brief An implementation of ElementParser for member method parsers.
 *
 * @author Bloody.Rabbit
 */
template< typename T >
class XMLParserEx::MemberElementParser
: public ElementParser
{
public:
    /// Type of class.
    typedef T Class;
    /// Type of method.
    typedef bool ( Class::* Method )( const TiXmlElement* );

    /**
     * @brief Primary constructor.
     *
     * @param[in] instance Instance of class.
     * @param[in] method   Member method.
     */
    MemberElementParser( Class& instance, const Method& method )
    : mInstance( instance ),
      mMethod( method )
    {
    }

    /**
     * @brief Invokes parser method.
     *
     * @param[in] field The element to be parsed.
     *
     * @retval true  Parsing successful.
     * @retval false Parsing failed.
     */
    bool Parse( const TiXmlElement* field )
    {
        return ( mInstance.*mMethod )( field );
    }

protected:
    /// The instance that the method should be invoked upon.
    Class& mInstance;
    /// The parser method.
    const Method mMethod;
};

/**
 * @brief Parses and stores a value.
 *
 * @author Bloody.Rabbit
 */
template< typename T >
class XMLParserEx::ValueParser
: public ElementParser
{
public:
    /// Type of parsed value.
    typedef T Value;

    /**
     * @brief Primary constructor.
     *
     * @param[in] value A variable to which the result is stored.
     */
    ValueParser( Value& value )
    : mValue( value )
    {
    }

    /**
     * @brief Parse the value and store it.
     *
     * @param[in] field The element to be parsed.
     *
     * @retval true  Parsing successful.
     * @retval false Parsing failed.
     */
    bool Parse( const TiXmlElement* field )
    {
        // obtain the text element
        const TiXmlNode* contents = field->FirstChild();
        if( NULL != contents ? TiXmlNode::TEXT != contents->Type() : false )
        {
            sLog.Error( "XMLParser", "Expected a text element in element '%s' at line %d.", field->Value(), field->Row() );
            return false;
        }

        // parse the text
        mValue = str2< Value >( contents->Value() );

        // return
        return true;
    }

protected:
    /// Reference to storage variable.
    Value& mValue;
};

#endif /* !__UTILS__XML_PARSER_EX_H__INCL__ */
