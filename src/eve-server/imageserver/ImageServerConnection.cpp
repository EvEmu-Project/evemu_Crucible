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

#ifndef MSVC
    // This is needed to build the server under linux using GCC
    #include <tr1/functional>
#endif

asio::const_buffers_1 ImageServerConnection::_responseOK = asio::buffer("HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\n\r\n", 45);
asio::const_buffers_1 ImageServerConnection::_responseNotFound = asio::buffer("HTTP/1.0 404 Not Found\r\n\r\n", 26);
asio::const_buffers_1 ImageServerConnection::_responseRedirectBegin = asio::buffer("HTTP/1.0 301 Moved Permanently\r\nLocation: ", 42);
asio::const_buffers_1 ImageServerConnection::_responseRedirectEnd = asio::buffer("\r\n\r\n", 4);

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
    asio::async_read_until(_socket, _buffer, "\r\n\r\n", std::tr1::bind(&ImageServerConnection::ProcessHeaders, shared_from_this()));
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
	request = request.substr(5);

	bool found = false;
	for (int i = 0; i < ImageServer::CategoryCount; i++)
	{
		if (starts_with(request, ImageServer::Categories[i]))
		{
			found = true;
			_category = ImageServer::Categories[i];
			request = request.substr(strlen(ImageServer::Categories[i]));
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

	_imageData = ImageServer::get().GetImage(_category, _id, _size);
	if (!_imageData)
	{
		Redirect();
		return;
	}

	// first we have to send the responseOK, then our actual result
	asio::async_write(_socket, _responseOK, asio::transfer_all(), std::tr1::bind(&ImageServerConnection::SendImage, shared_from_this()));
}

void ImageServerConnection::SendImage()
{
	asio::async_write(_socket, asio::buffer(*_imageData, _imageData->size()), asio::transfer_all(), std::tr1::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::NotFound()
{
	asio::async_write(_socket, _responseNotFound, asio::transfer_all(), std::tr1::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::Redirect()
{
	asio::async_write(_socket, _responseRedirectBegin, asio::transfer_all(), std::tr1::bind(&ImageServerConnection::RedirectLocation, shared_from_this()));
}

void ImageServerConnection::RedirectLocation()
{
    std::string extension = _category == "Character" ? "jpg" : "png";
	std::stringstream url;
	url << ImageServer::FallbackURL << _category << "/" << _id << "_" << _size << "." << extension;
	_redirectUrl = url.str();
	asio::async_write(_socket, asio::buffer(_redirectUrl), asio::transfer_all(), std::tr1::bind(&ImageServerConnection::RedirectFinalize, shared_from_this()));
}

void ImageServerConnection::RedirectFinalize()
{
	asio::async_write(_socket, _responseRedirectEnd, asio::transfer_all(), std::tr1::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::Close()
{
	_socket.close();
}

bool ImageServerConnection::starts_with(std::string& haystack, const char *const needle)
{
	return haystack.substr(0, strlen(needle)).compare(needle) == 0;
}

std::tr1::shared_ptr<ImageServerConnection> ImageServerConnection::create(asio::io_service& io)
{
	return std::tr1::shared_ptr<ImageServerConnection>(new ImageServerConnection(io));
}
