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
}

std::tr1::shared_ptr<std::string> APIServiceManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIServiceManager::ProcessCall()", "EVEmu API - Default Service Manager");

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

    TiXmlElement * cachedUntil = new TiXmlElement( "cachedUntil" );
    cachedUntil->LinkEndChild( new TiXmlText( Win32TimeToString(Win32TimeNow() + 5*Win32Time_Minute).c_str() ));
    eveapi->LinkEndChild( cachedUntil );

    TiXmlPrinter xmlPrinter;
    retXml.Accept( &xmlPrinter );

    return std::tr1::shared_ptr<std::string>(new std::string(xmlPrinter.CStr()));
}

bool APIServiceManager::_AuthenticateAPIQuery(std::string userID, std::string apiKey)
{
    return true;
}

void APIServiceManager::_BuildXMLHeader(uint32 cacheStyle)
{
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

void APIServiceManager::_BuildXMLTag(std::string name, const std::vector<std::string> * params)
{
}

void APIServiceManager::_CloseXMLTag()
{
}

void APIServiceManager::_BuildXMLErrorTag(uint32 error)
{
}
