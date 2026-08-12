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
#include "network/ImageRequestParser.h"

namespace {

constexpr char RESPONSE_JPEG[] =
    "HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\n\r\n";
constexpr char RESPONSE_PNG[] =
    "HTTP/1.0 200 OK\r\nContent-Type: image/png\r\n\r\n";

}

std::atomic<std::size_t> ImageServerConnection::_activeConnections(0);

boost::asio::const_buffers_1 ImageServerConnection::_responseJpeg =
    boost::asio::buffer(RESPONSE_JPEG, sizeof(RESPONSE_JPEG) - 1);
boost::asio::const_buffers_1 ImageServerConnection::_responsePng =
    boost::asio::buffer(RESPONSE_PNG, sizeof(RESPONSE_PNG) - 1);
boost::asio::const_buffers_1 ImageServerConnection::_responseNotFound = boost::asio::buffer("HTTP/1.0 404 Not Found\r\n\r\n", 26);
boost::asio::const_buffers_1 ImageServerConnection::_responseRedirectBegin = boost::asio::buffer("HTTP/1.0 301 Moved Permanently\r\nLocation: ", 42);
boost::asio::const_buffers_1 ImageServerConnection::_responseRedirectEnd = boost::asio::buffer("\r\n\r\n", 4);

ImageServerConnection::ImageServerConnection(boost::asio::io_context& io)
: _buffer(ImageServerLimits::MAX_HEADER_BYTES),
  _socket(io),
  _timer(io),
_id(0),
_size(0)
{
}

ImageServerConnection::~ImageServerConnection()
{
    boost::system::error_code error;
    _timer.cancel(error);
    _socket.close(error);
    _activeConnections.fetch_sub(1);
}

bool ImageServerConnection::AtCapacity()
{
    return _activeConnections.load() >= ImageServerLimits::MAX_CONNECTIONS;
}

boost::asio::ip::tcp::socket& ImageServerConnection::socket()
{
    return _socket;
}

void ImageServerConnection::Process()
{
    _timer.expires_after(ImageServerLimits::REQUEST_TIMEOUT);
    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    _timer.async_wait([self](const boost::system::error_code& error) {
        if (!error)
            self->Close();
    });

    boost::asio::async_read_until(
        _socket,
        _buffer,
        "\r\n\r\n",
        [self](
            const boost::system::error_code& error,
            std::size_t bytesTransferred) {
            self->ProcessHeaders(error, bytesTransferred);
        });
}

void ImageServerConnection::ProcessHeaders(
    const boost::system::error_code& error,
    std::size_t bytesTransferred)
{
    (void)bytesTransferred;
    if (error) {
        Close();
        return;
    }

    std::istream stream(&_buffer);
    std::string request;

    // every request line ends with \r\n
    std::getline(stream, request, '\r');

    ImageRequest parsed = { "", 0, 0 };
    if (!ParseImageRequest(request, parsed)) {
        NotFound();
        return;
    }
    _category = parsed.category;
    _id = parsed.id;
    _size = parsed.size;

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
    const boost::asio::const_buffers_1& response =
        _category == "Character" ? _responseJpeg : _responsePng;
    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    boost::asio::async_write(
        _socket,
        response,
        [self](const boost::system::error_code& error, std::size_t) {
            if (error)
                self->Close();
            else
                self->SendImage();
        });
}

void ImageServerConnection::SendImage()
{
    if (!_imageData || _imageData->empty()) {
        Close();
        return;
    }

    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    boost::asio::async_write(
        _socket,
        boost::asio::buffer(*_imageData, _imageData->size()),
        [self](const boost::system::error_code&, std::size_t) {
            self->Close();
        });
}

void ImageServerConnection::NotFound()
{
    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    boost::asio::async_write(
        _socket,
        _responseNotFound,
        [self](const boost::system::error_code&, std::size_t) {
            self->Close();
        });
}

void ImageServerConnection::Redirect()
{
    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    boost::asio::async_write(
        _socket,
        _responseRedirectBegin,
        [self](const boost::system::error_code& error, std::size_t) {
            if (error)
                self->Close();
            else
                self->RedirectLocation();
        });
}

void ImageServerConnection::RedirectLocation()
{
    std::string extension = _category == "Character" ? "jpg" : "png";
    std::stringstream url;
    url << ImageServer::FallbackURL << _category << "/" << _id << "_" << _size << "." << extension;
    _redirectUrl = url.str();
    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    boost::asio::async_write(
        _socket,
        boost::asio::buffer(_redirectUrl),
        [self](const boost::system::error_code& error, std::size_t) {
            if (error)
                self->Close();
            else
                self->RedirectFinalize();
        });
}

void ImageServerConnection::RedirectFinalize()
{
    std::shared_ptr<ImageServerConnection> self = shared_from_this();
    boost::asio::async_write(
        _socket,
        _responseRedirectEnd,
        [self](const boost::system::error_code&, std::size_t) {
            self->Close();
        });
}

void ImageServerConnection::Close()
{
    boost::system::error_code error;
    _timer.cancel(error);
    _socket.close(error);
}

std::shared_ptr<ImageServerConnection> ImageServerConnection::create(boost::asio::io_context& io)
{
    std::size_t current = _activeConnections.load();
    while (current < ImageServerLimits::MAX_CONNECTIONS &&
           !_activeConnections.compare_exchange_weak(current, current + 1)) {
    }
    if (current >= ImageServerLimits::MAX_CONNECTIONS)
        return std::shared_ptr<ImageServerConnection>();

    try {
        return std::shared_ptr<ImageServerConnection>(
            new ImageServerConnection(io));
    } catch (const std::bad_alloc&) {
        _activeConnections.fetch_sub(1);
        throw;
    }
}
