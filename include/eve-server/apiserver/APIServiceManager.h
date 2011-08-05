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

#ifndef __APISERVICEMANAGER__H__INCL__
#define __APISERVICEMANAGER__H__INCL__

#include "EVEServerPCH.h"

enum
{
    API_CACHE_STYLE_SHORT,
    API_CACHE_STYLE_LONG,
    API_CACHE_STYLE_MODIFIED
};

/**
 * \class APIServiceManager
 *
 * @brief Generic Base Class used to derive classes for specific service handlers (character, corporation, etc)
 *
 * This class implements basic functions to build the xml documents needed to be returned from the specific
 * service handlers for the API Server.  It is used as the base class for polymorphism container in APIServer::GetXML()
 * call to route the API Command Call package to the appropriate service handler using the service category.
 *
 * @author Aknor Jaden
 * @date July 2011
 */
class APIServiceManager
{
public:
	APIServiceManager();

	// Common call shared to all derived classes called via polymorphism
	virtual std::tr1::shared_ptr<std::string> ProcessCall(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> BuildErrorXMLResponse(std::string errorCode, std::string errorMessage);

protected:
    bool _AuthenticateFullAPIQuery(std::string userID, std::string apiKey);
    bool _AuthenticateLimitedAPIQuery(std::string userID, std::string apiKey);

	// Internal Utility functions used to make it easier to build the xml document to send back to the client:
	void _BuildXMLHeader();
    void _CloseXMLHeader(uint32 cacheStyle);
	void _BuildXMLRowSet(std::string name, std::string key, const std::vector<std::string> * columns);
	void _CloseXMLRowSet();
	void _BuildXMLRow(const std::vector<std::string> * columns);
    void _BuildXMLTag(std::string name);
    void _BuildXMLTag(std::string name, const std::vector<std::pair<std::string, std::string> > * params);
    void _BuildXMLTag(std::string name, const std::vector<std::pair<std::string, std::string> > * params, std::string value);
	void _CloseXMLTag();
    void _BuildSingleXMLTag(std::string name, std::string param);
    void _BuildErrorXMLTag(std::string code, std::string param);

    APIServiceDB m_db;

	TiXmlDocument _XmlDoc;
    TiXmlElement * _pXmlDocOuterTag;
    std::string _CurrentRowSetColumnString;
    std::stack<TiXmlElement *> * _pXmlElementStack;
};

#endif // __APISERVICEMANAGER__H__INCL__
