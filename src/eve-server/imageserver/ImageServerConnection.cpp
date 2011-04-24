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

#include "EVEServerPCH.h"

asio::const_buffers_1 ImageServerConnection::_responseOK = asio::buffer("HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
asio::const_buffers_1 ImageServerConnection::_responseNotFound = asio::buffer("HTTP/1.0 404 Not Found\r\n\r\n");

ImageServerConnection::ImageServerConnection(asio::io_service& io)
	: _socket(io)
{
}

asio::ip::tcp::socket& ImageServerConnection::socket()
{
	return _socket;
}

void ImageServerConnection::Process()
{
	// receive all HTTP headers from the client
	asio::async_read_until(_socket, _buffer, "\r\n\r\n", std::bind(&ImageServerConnection::ProcessHeaders, shared_from_this()));
}

void ImageServerConnection::ProcessHeaders()
{
	std::istream stream(&_buffer);
	std::string request;
	// every request line ends with \r\n
	std::getline(stream, request, '\r');

	if (!starts_with(request, "GET /"))
	{
		NotFound();
		return;
	}
}

void ImageServerConnection::NotFound()
{
	asio::async_write(_socket, _responseNotFound, std::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::Close()
{
	_socket.close();
}

bool ImageServerConnection::starts_with(std::string& haystack, char* needle)
{
	return haystack.substr(0, strlen(needle)).compare(needle) == 0;
}

std::shared_ptr<ImageServerConnection> ImageServerConnection::create(asio::io_service& io)
{
	return std::shared_ptr<ImageServerConnection>(new ImageServerConnection(io));
}