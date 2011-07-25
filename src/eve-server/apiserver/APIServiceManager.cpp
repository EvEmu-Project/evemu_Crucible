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
	Author:		Aknor Jaden
*/


#include "EVEServerPCH.h"


APIServiceManager::APIServiceManager()
{
    _pXmlDocOuterTag = NULL;
    _pXmlElementStack = NULL;
}

std::tr1::shared_ptr<std::string> APIServiceManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIServiceManager::ProcessCall()", "EVEmu API - Default Service Manager");

/*
    TiXmlDocument retXml;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
    retXml.LinkEndChild( decl );

    TiXmlElement * eveapi = new TiXmlElement( "eveapi" );
    retXml.LinkEndChild( eveapi );
    eveapi->SetAttribute( "version", "2" );

    TiXmlElement * currentTime = new TiXmlElement( "currentTime" );
    currentTime->LinkEndChild( new TiXmlText( Win32TimeToString(Win32TimeNow()).c_str() ));
    eveapi->LinkEndChild( currentTime );

    TiXmlElement * error = new TiXmlElement( "error" );
    error->SetAttribute( "code", "500" );
    error->LinkEndChild( new TiXmlText( "EVEmu: Invalid Service Manager Specified." ));
    eveapi->LinkEndChild( error );

    TiXmlElement * tag = new TiXmlElement( "attributeEnhancers" );
    TiXmlElement * innerTag1 = new TiXmlElement( "intelligence" );
    innerTag1->LinkEndChild( new TiXmlText( "6" ));
    TiXmlElement * innerTag2 = new TiXmlElement( "memory" );
    innerTag2->LinkEndChild( new TiXmlText( "4" ));
    TiXmlElement * innerTag3 = new TiXmlElement( "charisma" );
    innerTag3->LinkEndChild( new TiXmlText( "7" ));
    TiXmlElement * innerTag4 = new TiXmlElement( "perception" );
    innerTag4->LinkEndChild( new TiXmlText( "12" ));
    TiXmlElement * innerTag5 = new TiXmlElement( "willpower" );
    innerTag5->LinkEndChild( new TiXmlText( "10" ));
    tag->LinkEndChild( innerTag1 );
    tag->LinkEndChild( innerTag2 );
    tag->LinkEndChild( innerTag3 );
    tag->LinkEndChild( innerTag4 );
    tag->LinkEndChild( innerTag5 );
    eveapi->LinkEndChild( tag );

    TiXmlElement * cachedUntil = new TiXmlElement( "cachedUntil" );
    cachedUntil->LinkEndChild( new TiXmlText( Win32TimeToString(Win32TimeNow() + 5*Win32Time_Minute).c_str() ));
    eveapi->LinkEndChild( cachedUntil );

    TiXmlPrinter xmlPrinter;
    retXml.Accept( &xmlPrinter );
*/

    _BuildXMLHeader();
    _BuildErrorXMLTag( "500", "EVEmu: Invalid Service Manager Specified." );
    _BuildXMLTag( "attributeEnhancers" );
    _BuildSingleXMLTag( "intelligence", "6" );
    _BuildSingleXMLTag( "memory", "4" );
    _BuildSingleXMLTag( "charisma", "7" );
    _BuildSingleXMLTag( "perception", "12" );
    _BuildSingleXMLTag( "willpower", "10" );
    _CloseXMLTag(); // close "attributeEnhancers" tag
    _CloseXMLHeader( API_CACHE_STYLE_MODIFIED );

    TiXmlPrinter xmlPrinter;
    _XmlDoc.Accept( &xmlPrinter );

    return std::tr1::shared_ptr<std::string>(new std::string(xmlPrinter.CStr()));
}

bool APIServiceManager::_AuthenticateAPIQuery(std::string userID, std::string apiKey)
{
    return true;
}

void APIServiceManager::_BuildXMLHeader()
{
    // Build header at beginning of XML document, so clear existing xml document
    _XmlDoc.Clear();
    if( _pXmlDocOuterTag != NULL )
        delete _pXmlDocOuterTag;
    if( _pXmlElementStack != NULL )
        delete _pXmlElementStack;
    _pXmlElementStack = new std::stack<TiXmlElement *>();

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
    _XmlDoc.LinkEndChild( decl );

    _pXmlDocOuterTag = new TiXmlElement( "eveapi" );
    _XmlDoc.LinkEndChild( _pXmlDocOuterTag );
    _pXmlDocOuterTag->SetAttribute( "version", "2" );

    TiXmlElement * currentTime = new TiXmlElement( "currentTime" );
    currentTime->LinkEndChild( new TiXmlText( Win32TimeToString(Win32TimeNow()).c_str() ));
    _pXmlDocOuterTag->LinkEndChild( currentTime );
}

void APIServiceManager::_CloseXMLHeader(uint32 cacheStyle)
{
    switch( cacheStyle )
    {
        case API_CACHE_STYLE_SHORT:
            // 2 hour cache timer
            _BuildSingleXMLTag( "cachedUntil", Win32TimeToString(Win32TimeNow() + 120*Win32Time_Minute).c_str() );
            break;
        case API_CACHE_STYLE_LONG:
            // 5 minute cache timer
            _BuildSingleXMLTag( "cachedUntil", Win32TimeToString(Win32TimeNow() + 5*Win32Time_Minute).c_str() );
            break;
        case API_CACHE_STYLE_MODIFIED:
            // 15 minute cache timer
            _BuildSingleXMLTag( "cachedUntil", Win32TimeToString(Win32TimeNow() + 15*Win32Time_Minute).c_str() );
            break;
    }
}

void APIServiceManager::_BuildXMLRowSet(std::string name, std::string key, const std::vector<std::string> * columns)
{
}

void APIServiceManager::_CloseXMLRowSet()
{
}

void APIServiceManager::_BuildXMLRow(const std::vector<std::string> * columns)
{
}

void APIServiceManager::_BuildXMLTag(std::string name)
{
    TiXmlElement * tag = new TiXmlElement( name.c_str() );
    _pXmlElementStack->push( tag );
}

void APIServiceManager::_BuildXMLTag(std::string name, const std::vector<std::pair<std::string, std::string>> * params)
{
    TiXmlElement * tag = new TiXmlElement( name.c_str() );

    std::vector<std::pair<std::string, std::string>>::const_iterator current, end;
    current = params->begin();
    end = params->end();
	for(; current != end; ++current)
        tag->SetAttribute( current->first.c_str(), current->second.c_str() );

    _pXmlElementStack->push( tag );
}

void APIServiceManager::_BuildXMLTag(std::string name, const std::vector<std::pair<std::string, std::string>> * params, std::string value)
{
    _BuildXMLTag( name, params );
    _pXmlElementStack->top()->LinkEndChild( new TiXmlText( value.c_str() ));
}

void APIServiceManager::_BuildSingleXMLTag(std::string name, std::string value)
{
    TiXmlElement * tag = new TiXmlElement( name.c_str() );
    tag->LinkEndChild( new TiXmlText( value.c_str() ));

    if( _pXmlElementStack->empty() )
        _pXmlDocOuterTag->LinkEndChild( tag );
    else
        _pXmlElementStack->top()->LinkEndChild( tag );
}

void APIServiceManager::_BuildErrorXMLTag(std::string code, std::string param)
{
    TiXmlElement * error = new TiXmlElement( "error" );
    error->SetAttribute( "code", code.c_str() );
    error->LinkEndChild( new TiXmlText( param.c_str() ));

    if( _pXmlElementStack->empty() )
        _pXmlDocOuterTag->LinkEndChild( error );
    else
        _pXmlElementStack->top()->LinkEndChild( error );
}

void APIServiceManager::_CloseXMLTag()
{
    if( _pXmlElementStack->empty() )
        return;

    TiXmlElement * _pTopElement = _pXmlElementStack->top();
    _pXmlElementStack->pop();

    if( _pXmlElementStack->empty() )
        _pXmlDocOuterTag->LinkEndChild( _pTopElement );
    else
    {
        TiXmlElement * _pNextTopElement = _pXmlElementStack->top();
        _pTopElement->LinkEndChild( _pTopElement );
    }
}
