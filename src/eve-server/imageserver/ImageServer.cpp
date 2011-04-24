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

ImageServer::ImageServer()
{
	std::stringstream urlBuilder;
	urlBuilder << "http://" << sConfig.net.imageServer << ":" << (sConfig.net.port +1) << "/";
	_url = urlBuilder.str();

	sLog.Log("image server", "our URL: %s", _url.c_str());
}

std::string& ImageServer::url()
{
	return _url;
}

void ImageServer::Run()
{
	_ioThread = std::auto_ptr<asio::thread>(new asio::thread(std::bind(&ImageServer::RunInternal, this)));
}

void ImageServer::RunInternal()
{
	_io = std::auto_ptr<asio::io_service>(new asio::io_service());
	_listener = std::auto_ptr<ImageServerListener>(new ImageServerListener(*_io));
	_io->run();
}
