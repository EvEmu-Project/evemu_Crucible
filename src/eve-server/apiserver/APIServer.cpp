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
	Author:		Aknor Jaden, adapted from ImageServer.h authored by caytchen
*/

#include "EVEServerPCH.h"
#include <iostream>
#include <fstream>

#ifndef MSVC
    // This is needed to build the server under linux using GCC
    #include <tr1/functional>
#endif

const char *const APIServer::FallbackURL = "http://api.eveonline.com/";

APIServer::APIServer()
{
    runonce = false;
    std::stringstream urlBuilder;
	urlBuilder << "http://" << sConfig.net.apiServer << ":" << (sConfig.net.apiServerPort) << "/";
	_url = urlBuilder.str();
}

void APIServer::CreateServices(const PyServiceMgr &services)
{
    if( !runonce )
    {
        m_APIServiceManagers.insert(std::make_pair("base", new APIServiceManager(services)));
        m_APIServiceManagers.insert(std::make_pair("account", new APIAccountManager(services)));
        m_APIServiceManagers.insert(std::make_pair("admin", new APIAdminManager(services)));
        m_APIServiceManagers.insert(std::make_pair("char", new APICharacterManager(services)));
        m_APIServiceManagers.insert(std::make_pair("corp", new APICorporationManager(services)));
        m_APIServiceManagers.insert(std::make_pair("eve", new APIEveSystemManager(services)));
        m_APIServiceManagers.insert(std::make_pair("map", new APIMapManager(services)));
        m_APIServiceManagers.insert(std::make_pair("server", new APIServerManager(services)));
    }

    runonce = true;
}

std::tr1::shared_ptr<std::vector<char> > APIServer::GetXML(const APICommandCall * pAPICommandCall)
{
    //if( m_APIServiceManagers.find(pAPICommandCall->at(0).first) != m_APIServiceManagers.end() )
    if( pAPICommandCall->find( "service" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APIserver::GetXML()", "Cannot find 'service' specifier in pAPICommandCall packet" );
		return std::tr1::shared_ptr<std::vector<char> >(new std::vector<char>() );
        //return std::tr1::shared_ptr<std::string>(new std::string(""));
    }

    if( m_APIServiceManagers.find(pAPICommandCall->find( "service" )->second) != m_APIServiceManagers.end() )
    {
        // Get reference to service manager object and call ProcessCall() with the pAPICommandCall packet
        //m_xmlString = m_APIServiceManagers.find("base")->second->ProcessCall(pAPICommandCall);
        m_xmlString = m_APIServiceManagers.find( pAPICommandCall->find( "service" )->second )->second->ProcessCall( pAPICommandCall );

        // Convert the std::string to the std::vector<char>:
	    std::tr1::shared_ptr<std::vector<char> > ret = std::tr1::shared_ptr<std::vector<char> >(new std::vector<char>());
        unsigned long len = m_xmlString->length();
        for(size_t i=0; i<m_xmlString->length(); i++)
            ret->push_back(m_xmlString->at(i));

        return ret;
    }
    else
    {
        // Service call not found, so return NULL:
		return std::tr1::shared_ptr<std::vector<char> >(new std::vector<char>() );
        //return NULL;
        //m_xmlString = m_APIServiceManagers.find("base")->second->ProcessCall(pAPICommandCall);
    }
}

std::string& APIServer::url()
{
	return _url;
}

void APIServer::Run()
{
    _ioThread = std::unique_ptr<asio::thread>(new asio::thread(std::tr1::bind(&APIServer::RunInternal, this)));
}

void APIServer::RunInternal()
{
    _io = std::unique_ptr<asio::io_service>(new asio::io_service());
	_listener = std::unique_ptr<APIServerListener>(new APIServerListener(*_io));
	_io->run();
}

APIServer::Lock::Lock(asio::detail::mutex& mutex)
	: _mutex(mutex)
{
	_mutex.lock();
}

APIServer::Lock::~Lock()
{
	_mutex.unlock();
}
