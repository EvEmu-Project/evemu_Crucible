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
    Author:		caytchen
*/

#ifndef __IMAGESERVER__H__INCL__
#define __IMAGESERVER__H__INCL__

#include "utils/Singleton.h"

#include <memory>
#include <string>
#if defined( MSVC )
	#include <unordered_map>
#else
	#include <tr1/unordered_map>
	#include <tr1/memory>
#endif
#include <asio.hpp>

class ImageServerListener;

/**
 * \class ImageServer
 *
 * @brief Handles distribution of character and related game images
 *
 * A very limited HTTP server that can efficiently deliver character and other images to clients
 * Uses asio for efficient asynchronous network communication
 *
 * @author caytchen
 * @date April 2011
 */
class ImageServer : public Singleton<ImageServer>
{
public:
	ImageServer();
	void Run();

	std::string& url();

    void ReportNewImage(uint32 accountID, std::tr1::shared_ptr<std::vector<char> > imageData);
	void ReportNewCharacter(uint32 creatorAccountID, uint32 characterID);

	std::string GetFilePath(std::string& category, uint32 id, uint32 size);
    std::tr1::shared_ptr<std::vector<char> > GetImage(std::string& category, uint32 id, uint32 size);

	static const char *const Categories[];
	static const uint32 CategoryCount;

	// used when the ImageServer can't find the image requested
	// this way we don't have to transfer over all the static NPC images
	static const char *const FallbackURL;

private:
	void RunInternal();
	bool ValidateCategory(std::string& category);
	bool ValidateSize(std::string& category, uint32 size);
	static bool CreateNewDirectory(std::string& path);

    std::tr1::unordered_map<uint32 /*accountID*/, std::tr1::shared_ptr<std::vector<char> > /*imageData*/> _limboImages;
	std::auto_ptr<asio::thread> _ioThread;
	std::auto_ptr<asio::io_service> _io;
	std::auto_ptr<ImageServerListener> _listener;
	std::string _url;
	std::string _basePath;
	asio::detail::mutex _limboLock;

	class Lock
	{
	public:
		Lock(asio::detail::mutex& mutex);
		~Lock();
	private:
		asio::detail::mutex& _mutex;
	};
};

#endif // __IMAGESERVER__H__INCL__
