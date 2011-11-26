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

#ifndef __APISERVER__H__INCL__
#define __APISERVER__H__INCL__

#include <memory>
#include <string>
#if defined( MSVC )
	#include <unordered_map>
#else
	#include <tr1/unordered_map>
#endif

#include <asio.hpp>

#include "APIServerConnection.h"
#include "EVEServerPCH.h"

class APIServerListener;
class APIServiceManager;

/**
 * \class APIServer
 *
 * @brief Handles queries from external clients for many kinds of returned xml structures according to CCP's API plus extensions for EVEmu
 *
 * A very limited HTTP server that can efficiently deliver many different xml structured documents to clients
 * Uses asio for efficient asynchronous network communication
 *
 * @author Aknor Jaden
 * @date July 2011
 */
class APIServer : public Singleton<APIServer>
{
public:
	APIServer();
	void Run();
    void CreateServices(const PyServiceMgr &services);

	std::string& url();

    std::tr1::shared_ptr<std::vector<char> > GetXML(const APICommandCall * pAPICommandCall);

	// used when the ImageServer can't find the image requested
	// this way we don't have to transfer over all the static NPC images
	static const char *const FallbackURL;

private:
	void RunInternal();

	std::unique_ptr<asio::thread> _ioThread;
	std::unique_ptr<asio::io_service> _io;
	std::unique_ptr<APIServerListener> _listener;
	std::string _url;
	std::string _basePath;
	asio::detail::mutex _limboLock;
    bool runonce;

    std::tr1::shared_ptr<std::string> m_xmlString;

    std::map<std::string, APIServiceManager *> m_APIServiceManagers;    // We own these

	class Lock
	{
	public:
		Lock(asio::detail::mutex& mutex);
		~Lock();
	private:
		asio::detail::mutex& _mutex;
	};
};

#define sAPIServer \
    ( APIServer::get() )

#endif // __APISERVER__H__INCL__
