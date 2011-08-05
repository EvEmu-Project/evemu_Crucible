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
    _CurrentRowSetColumnString = "";
}

std::tr1::shared_ptr<std::string> APIServiceManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIServiceManager::ProcessCall()", "EVEmu API - Default Service Manager");

    // EXAMPLE OF USING ALL FEATURES OF THE INHERITED APISERVICEMANAGER XML BUILDING HELPER FUNCTIONS:
    /*
    std::vector<std::string> rowset;
    _BuildXMLHeader();
    {
        _BuildErrorXMLTag( "500", "EVEmu: Invalid Service Manager Specified." );

        _BuildXMLTag( "attributes" );
        {
            _BuildXMLTag( "Aknor_Jaden" );
            {
                _BuildSingleXMLTag( "intelligence", "6" );
                _BuildSingleXMLTag( "memory", "4" );
                _BuildSingleXMLTag( "charisma", "7" );
                _BuildSingleXMLTag( "perception", "12" );
                _BuildSingleXMLTag( "willpower", "10" );
            }
            _CloseXMLTag(); // close "Aknor_Jaden" tag
        }
        _CloseXMLTag(); // close "attributes" tag

        rowset.push_back("typeID");
        rowset.push_back("skillpoints");
        rowset.push_back("level");
        rowset.push_back("published");
        _BuildXMLRowSet( "skills", "typeID", &rowset );
        {
            rowset.clear();
            rowset.push_back("3431");
            rowset.push_back("8000");
            rowset.push_back("3");
            rowset.push_back("1");
            _BuildXMLRow( &rowset );
            rowset.clear();
            rowset.push_back("3413");
            rowset.push_back("8000");
            rowset.push_back("3");
            rowset.push_back("1");
            _BuildXMLRow( &rowset );
            rowset.clear();
            rowset.push_back("21059");
            rowset.push_back("500");
            rowset.push_back("1");
            rowset.push_back("1");
            _BuildXMLRow( &rowset );
            rowset.clear();
            rowset.push_back("3416");
            rowset.push_back("8000");
            rowset.push_back("3");
            rowset.push_back("1");
            _BuildXMLRow( &rowset );
            rowset.clear();
            rowset.push_back("3445");
            rowset.push_back("512000");
            rowset.push_back("5");
            rowset.push_back("0");
            _BuildXMLRow( &rowset );
        }
        _CloseXMLRowSet();  // close rowset "skils"

        _BuildXMLTag( "attributeEnhancers" );
        {
            _BuildXMLTag( "memoryBonus" );
            {
                _BuildSingleXMLTag( "augmentatorName", "Memory Augmentation - Basic" );
                _BuildSingleXMLTag( "augmentatorValue", "3" );
            }
            _CloseXMLTag(); // close tag "memoryBonus"
            _BuildXMLTag( "perceptionBonus" );
            {
                _BuildSingleXMLTag( "augmentatorName", "Ocular Filter - Basic" );
                _BuildSingleXMLTag( "augmentatorValue", "5" );
            }
            _CloseXMLTag(); // close tag "perceptionBonus"
        }
        _CloseXMLTag(); // close tag "attributeEnhancers"
    }
    _CloseXMLHeader( API_CACHE_STYLE_MODIFIED );
    */

    _BuildXMLHeader();
    {
        _BuildErrorXMLTag( "500", "EVEmu: Invalid Service Manager Specified." );
    }
    _CloseXMLHeader( API_CACHE_STYLE_MODIFIED );

    TiXmlPrinter xmlPrinter;
    _XmlDoc.Accept( &xmlPrinter );

    return std::tr1::shared_ptr<std::string>(new std::string(xmlPrinter.CStr()));
}

std::tr1::shared_ptr<std::string> APIServiceManager::BuildErrorXMLResponse(std::string errorCode, std::string errorMessage)
{
    _BuildXMLHeader();
    {
        _BuildErrorXMLTag( errorCode, errorMessage );
    }
    _CloseXMLHeader( API_CACHE_STYLE_MODIFIED );

    TiXmlPrinter xmlPrinter;
    _XmlDoc.Accept( &xmlPrinter );

    return std::tr1::shared_ptr<std::string>(new std::string(xmlPrinter.CStr()));
}

bool APIServiceManager::_AuthenticateFullAPIQuery(std::string userID, std::string apiKey)
{
    std::string apiFullKey;
    std::string apiLimitedKey;
    uint32 apiRole;

    bool status = m_db.GetApiAccountInfoUsingUserID(userID, &apiFullKey, &apiLimitedKey, &apiRole);

    if( (apiKey.compare( apiFullKey )) && (status) )
        return true;
    else
        return false;
}

bool APIServiceManager::_AuthenticateLimitedAPIQuery(std::string userID, std::string apiKey)
{
    std::string apiFullKey;
    std::string apiLimitedKey;
    uint32 apiRole;

    bool status = m_db.GetApiAccountInfoUsingUserID(userID, &apiFullKey, &apiLimitedKey, &apiRole);

    if( (apiKey.compare( apiLimitedKey )) && (status) )
        return true;
    else
        return false;
}

void APIServiceManager::_BuildXMLHeader()
{
    // Build header at beginning of XML document, so clear existing xml document
    _XmlDoc.Clear();
    // object pointed to by '_pXmlDocOuterTag' is automatically deleted by the TinyXML system with the above call
    if( _pXmlElementStack != NULL )
    {
        delete _pXmlElementStack;
        _pXmlElementStack = NULL;
    }
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
    TiXmlElement * rowset = new TiXmlElement( "rowset" );

    std::vector<std::string>::const_iterator current, end;
    rowset->SetAttribute( "name", name.c_str() );
    rowset->SetAttribute( "key", key.c_str() );
    current = columns->begin();
    end = columns->end();
    _CurrentRowSetColumnString = *current;
    ++current;
	for(; current != end; ++current)
    {
        _CurrentRowSetColumnString += ",";
        _CurrentRowSetColumnString += *current;
    }
    rowset->SetAttribute( "columns", _CurrentRowSetColumnString.c_str() );

    _pXmlElementStack->push( rowset );
}

void APIServiceManager::_CloseXMLRowSet()
{
    _CloseXMLTag();
}

void APIServiceManager::_BuildXMLRow(const std::vector<std::string> * columns)
{
    TiXmlElement * row = new TiXmlElement( "row" );

    std::vector<std::string>::const_iterator current, end;
    std::string column_string = _CurrentRowSetColumnString;
    std::string column_name;
    int pos=0;
    int pos2=0;
    current = columns->begin();
    end = columns->end();
	for(; current != end; ++current)
    {
        pos = column_string.find_first_of(",");
        column_name = column_string.substr(0,pos);
        column_string = column_string.substr(pos+1);
        row->SetAttribute( column_name.c_str(), current->c_str() );
    }
    _pXmlElementStack->top()->LinkEndChild( row );
}

void APIServiceManager::_BuildXMLTag(std::string name)
{
    TiXmlElement * tag = new TiXmlElement( name.c_str() );
    _pXmlElementStack->push( tag );
}

void APIServiceManager::_BuildXMLTag(std::string name, const std::vector<std::pair<std::string, std::string> > * params)
{
    TiXmlElement * tag = new TiXmlElement( name.c_str() );

    std::vector<std::pair<std::string, std::string> >::const_iterator current, end;
    current = params->begin();
    end = params->end();
	for(; current != end; ++current)
        tag->SetAttribute( current->first.c_str(), current->second.c_str() );

    _pXmlElementStack->push( tag );
}

void APIServiceManager::_BuildXMLTag(std::string name, const std::vector<std::pair<std::string, std::string> > * params, std::string value)
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
        _pNextTopElement->LinkEndChild( _pTopElement );
    }
}
