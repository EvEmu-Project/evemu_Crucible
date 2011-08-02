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
#include <vector>
#include <asio.hpp>

/**
 * \class ImageServerConnection
 *
 * @brief Handles a client connection to the image server
 *
 * Handles exactly one client; does all the protocol related stuff. Very limited HTTP handling.
 *
 * @author caytchen
 * @date April 2011
 */
class ImageServerConnection : public std::tr1::enable_shared_from_this<ImageServerConnection>
{
public:
    static std::tr1::shared_ptr<ImageServerConnection> create(asio::io_service& io);
	void Process();
	asio::ip::tcp::socket& socket();

private:
	ImageServerConnection(asio::io_service& io);
	void ProcessHeaders();
	void SendImage();
	void NotFound();
	void Close();
	void Redirect();
	void RedirectLocation();
	void RedirectFinalize();

	static bool starts_with(std::string& haystack, const char *const needle);

	// request data
	std::string _category;
	uint32 _id;
	uint32 _size;
	std::string _redirectUrl;

	asio::streambuf _buffer;
	asio::ip::tcp::socket _socket;
    std::tr1::shared_ptr<std::vector<char> > _imageData;
	
	static asio::const_buffers_1 _responseOK;
	static asio::const_buffers_1 _responseNotFound;
	static asio::const_buffers_1 _responseRedirectBegin;
	static asio::const_buffers_1 _responseRedirectEnd;
};

#endif // __IMAGESERVERCONNECTION__H__INCL__
