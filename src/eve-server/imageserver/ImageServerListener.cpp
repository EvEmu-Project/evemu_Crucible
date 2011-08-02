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
#include <tr1/functional>

ImageServerListener::ImageServerListener(asio::io_service& io)
{
    _acceptor = new asio::ip::tcp::acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), sConfig.net.imageServerPort));
    sLog.Log("image server", "listening on port %i", (sConfig.net.imageServerPort));
	StartAccept();
}

ImageServerListener::~ImageServerListener()
{
	delete _acceptor;
}

void ImageServerListener::StartAccept()
{
    std::tr1::shared_ptr<ImageServerConnection> connection = ImageServerConnection::create(_acceptor->get_io_service());
	_acceptor->async_accept(connection->socket(), std::tr1::bind(&ImageServerListener::HandleAccept, this, connection));
}

void ImageServerListener::HandleAccept(std::tr1::shared_ptr<ImageServerConnection> connection)
{
	connection->Process();
	StartAccept();
}
