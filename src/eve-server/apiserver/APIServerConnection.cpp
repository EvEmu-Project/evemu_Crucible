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
#include <tr1/functional>

asio::const_buffers_1 APIServerConnection::_responseOK = asio::buffer("HTTP/1.0 200 OK\r\nContent-Type: text/xml\r\n\r\n", 45);
asio::const_buffers_1 APIServerConnection::_responseNotFound = asio::buffer("HTTP/1.0 404 Not Found\r\n\r\n", 26);
asio::const_buffers_1 APIServerConnection::_responseRedirectBegin = asio::buffer("HTTP/1.0 301 Moved Permanently\r\nLocation: ", 42);
asio::const_buffers_1 APIServerConnection::_responseRedirectEnd = asio::buffer("\r\n\r\n", 4);

APIServerConnection::APIServerConnection(asio::io_service& io)
	: _socket(io)
{
}

asio::ip::tcp::socket& APIServerConnection::socket()
{
	return _socket;
}

void APIServerConnection::Process()
{
	// receive all HTTP headers from the client
    asio::async_read_until(_socket, _buffer, "\r\n\r\n", std::tr1::bind(&APIServerConnection::ProcessHeaders, shared_from_this()));
}

void APIServerConnection::ProcessHeaders()
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
	request = request.substr(5);

	bool found = false;
	for (int i = 0; i < APIServer::CategoryCount; i++)
	{
		if (starts_with(request, APIServer::Categories[i]))
		{
			found = true;
			_category = APIServer::Categories[i];
			request = request.substr(strlen(APIServer::Categories[i]));
			break;
		}
	}
	if (!found)
	{
		NotFound();
		return;
	}

	if (!starts_with(request, "/"))
	{
		NotFound();
		return;
	}
	request = request.substr(1);

	int del = request.find_first_of('_');
	if (del == std::string::npos)
	{
		NotFound();
		return;
	}

	// might have some extra data but atoi shouldn't care
	std::string idStr = request.substr(0, del);
	std::string sizeStr = request.substr(del + 1);
	_id = atoi(idStr.c_str());
	_size = atoi(sizeStr.c_str());

	_xmlData = APIServer::get().GetXML(_category, _id, _size);
	if (!_xmlData)
	{
		Redirect();
		return;
	}

	// first we have to send the responseOK, then our actual result
	asio::async_write(_socket, _responseOK, asio::transfer_all(), std::tr1::bind(&APIServerConnection::SendXML, shared_from_this()));
}

void APIServerConnection::SendXML()
{
	asio::async_write(_socket, asio::buffer(*_xmlData, _xmlData->size()), asio::transfer_all(), std::tr1::bind(&APIServerConnection::Close, shared_from_this()));
}

void APIServerConnection::NotFound()
{
	asio::async_write(_socket, _responseNotFound, asio::transfer_all(), std::tr1::bind(&APIServerConnection::Close, shared_from_this()));
}

void APIServerConnection::Redirect()
{
	asio::async_write(_socket, _responseRedirectBegin, asio::transfer_all(), std::tr1::bind(&APIServerConnection::RedirectLocation, shared_from_this()));
}

void APIServerConnection::RedirectLocation()
{
    //std::string extension = _category == "Character" ? "jpg" : "png";
	std::stringstream url;
	url << APIServer::FallbackURL << _category << "/" << _id;
	_redirectUrl = url.str();
	asio::async_write(_socket, asio::buffer(_redirectUrl), asio::transfer_all(), std::tr1::bind(&APIServerConnection::RedirectFinalize, shared_from_this()));
}

void APIServerConnection::RedirectFinalize()
{
	asio::async_write(_socket, _responseRedirectEnd, asio::transfer_all(), std::tr1::bind(&APIServerConnection::Close, shared_from_this()));
}

void APIServerConnection::Close()
{
	_socket.close();
}

bool APIServerConnection::starts_with(std::string& haystack, const char *const needle)
{
	return haystack.substr(0, strlen(needle)).compare(needle) == 0;
}

std::tr1::shared_ptr<APIServerConnection> APIServerConnection::create(asio::io_service& io)
{
	return std::tr1::shared_ptr<APIServerConnection>(new APIServerConnection(io));
}
