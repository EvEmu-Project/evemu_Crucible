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

#ifndef __IMAGESERVERCONNECTION__H__INCL__
#define __IMAGESERVERCONNECTION__H__INCL__

#include <memory>
#include <asio.hpp>

class ImageServerConnection : public std::enable_shared_from_this<ImageServerConnection>
{
public:
	static std::shared_ptr<ImageServerConnection> create(asio::io_service& io);
	void Process();
	asio::ip::tcp::socket& socket();

private:
	ImageServerConnection(asio::io_service& io);
	void ProcessHeaders();
	void NotFound();
	void Close();

	static bool starts_with(std::string& haystack, char* needle);

	asio::streambuf _buffer;
	asio::ip::tcp::socket _socket;
	
	static asio::const_buffers_1 _responseOK;
	static asio::const_buffers_1 _responseNotFound;
};

#endif // __IMAGESERVERCONNECTION__H__INCL__
