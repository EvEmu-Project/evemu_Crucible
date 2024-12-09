/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        caytchen
*/

#include "imageserver/ImageServerConnection.h"

boost::asio::const_buffers_1 ImageServerConnection::_responseOK = boost::asio::buffer("HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\n\r\n", 45);
boost::asio::const_buffers_1 ImageServerConnection::_responseNotFound = boost::asio::buffer("HTTP/1.0 404 Not Found\r\n\r\n", 26);
boost::asio::const_buffers_1 ImageServerConnection::_responseRedirectBegin = boost::asio::buffer("HTTP/1.0 301 Moved Permanently\r\nLocation: ", 42);
boost::asio::const_buffers_1 ImageServerConnection::_responseRedirectEnd = boost::asio::buffer("\r\n\r\n", 4);

ImageServerConnection::ImageServerConnection(boost::asio::io_context& io)
: _socket(io),
_id(0),
_size(0)
{
}

boost::asio::ip::tcp::socket& ImageServerConnection::socket()
{
    return _socket;
}

void ImageServerConnection::Process()
{
    // receive all HTTP headers from the client
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n", std::bind(&ImageServerConnection::ProcessHeaders, shared_from_this()));
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
    for (uint32 i = 0; i < ImageServer::CategoryCount; i++)
    {
        if (starts_with(request, ImageServer::Categories[i]))
        {
            found = true;
            _category = ImageServer::Categories[i];
            request = request.substr(static_cast<int>(strlen(ImageServer::Categories[i])));
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

    int del = static_cast<int>(request.find_first_of('_'));
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

    _imageData = sImageServer.GetImage(_category, _id, _size);
    if (!_imageData) {
        if (IsPlayerItem(_id)) {
            sLog.Error("     Image Server","Image for itemID %u not found.", _id);
            NotFound();
            return;
        } else if (IsCharacterID(_id)) {
            sLog.Error("     Image Server","Image for charID %u not found.", _id);
            NotFound();
            return;
        }
        Redirect();
        return;
    }

    // first we have to send the responseOK, then our actual result
    boost::asio::async_write(_socket, _responseOK, boost::asio::transfer_all(), std::bind(&ImageServerConnection::SendImage, shared_from_this()));
}

void ImageServerConnection::SendImage()
{
    boost::asio::async_write(_socket, boost::asio::buffer(*_imageData, _imageData->size()), boost::asio::transfer_all(), std::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::NotFound()
{
    boost::asio::async_write(_socket, _responseNotFound, boost::asio::transfer_all(), std::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::Redirect()
{
    boost::asio::async_write(_socket, _responseRedirectBegin, boost::asio::transfer_all(), std::bind(&ImageServerConnection::RedirectLocation, shared_from_this()));
}

void ImageServerConnection::RedirectLocation()
{
    sLog.Error("     Image Server"," RedirectLocation() called.");
    std::string extension = _category == "Character" ? "jpg" : "png";
    std::stringstream url;
    url << ImageServer::FallbackURL << _category << "/" << _id << "_" << _size << "." << extension;
    _redirectUrl = url.str();
    boost::asio::async_write(_socket, boost::asio::buffer(_redirectUrl), boost::asio::transfer_all(), std::bind(&ImageServerConnection::RedirectFinalize, shared_from_this()));
}

void ImageServerConnection::RedirectFinalize()
{
    boost::asio::async_write(_socket, _responseRedirectEnd, boost::asio::transfer_all(), std::bind(&ImageServerConnection::Close, shared_from_this()));
}

void ImageServerConnection::Close()
{
    _socket.close();
}

bool ImageServerConnection::starts_with(std::string& haystack, const char *const needle)
{
    return haystack.substr(0, strlen(needle)).compare(needle) == 0;
}

std::shared_ptr<ImageServerConnection> ImageServerConnection::create(boost::asio::io_context& io)
{
    return std::shared_ptr<ImageServerConnection>(new ImageServerConnection(io));
}
