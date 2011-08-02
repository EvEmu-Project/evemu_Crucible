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
#include <iostream>
#include <fstream>
#include <tr1/functional>

const char *const ImageServer::FallbackURL = "http://image.eveonline.com/";

const char *const ImageServer::Categories[] = {
    "Alliance",
	"Corporation",
	"Character", 
	"InventoryType",
	"Render" };

const uint32 ImageServer::CategoryCount = 5;

ImageServer::ImageServer()
{
	std::stringstream urlBuilder;
    urlBuilder << "http://" << sConfig.net.imageServer << ":" << (sConfig.net.imageServerPort) << "/";
	_url = urlBuilder.str();

	_basePath = sConfig.files.imageDir;
	if (_basePath[_basePath.size() - 1] != '/')
		_basePath += "/";

	CreateNewDirectory(_basePath);

	for (int i = 0; i < CategoryCount; i++) {
		std::string subdir = _basePath;
		subdir.append(Categories[i]);
		CreateNewDirectory(subdir);
	}

	sLog.Log("image server", "our URL: %s", _url.c_str());
	sLog.Log("image server", "our base: %s", _basePath.c_str());
}

bool ImageServer::CreateNewDirectory(std::string& path)
{
	return mkdir(path.c_str(), 0777) == 0;
}

void ImageServer::ReportNewImage(uint32 accountID, std::tr1::shared_ptr<std::vector<char> > imageData)
{
	Lock lock(_limboLock);

	if (_limboImages.find(accountID) != _limboImages.end())
		_limboImages.insert(std::pair<uint32,std::tr1::shared_ptr<std::vector<char> > >(accountID, imageData));
	else
		_limboImages[accountID] = imageData;
}

void ImageServer::ReportNewCharacter(uint32 creatorAccountID, uint32 characterID)
{
	Lock lock(_limboLock);

	// check if we received an image from this account previously
	if (_limboImages.find(creatorAccountID) == _limboImages.end())
		return;

	// we have, so save it
	//std::ofstream stream;
	std::string dirName = "Character";
	std::string path(GetFilePath(dirName, characterID, 512));
    FILE * fp = fopen(path.c_str(), "wb");
    
	//stream.open(path, std::ios::binary | std::ios::trunc | std::ios::out);
	std::tr1::shared_ptr<std::vector<char> > data = _limboImages[creatorAccountID];

    fwrite(&((*data)[0]), 1, data->size(), fp);
    fclose(fp);
    
	//std::copy(data->begin(), data->end(), std::ostream_iterator<char>(stream));
	//stream.flush();
	//stream.close();

	// and delete it from our limbo map
	_limboImages.erase(creatorAccountID);

	sLog.Log("image server", "saved image from %i as %s", creatorAccountID, path.c_str());
}

std::tr1::shared_ptr<std::vector<char> > ImageServer::GetImage(std::string& category, uint32 id, uint32 size)
{
	if (!ValidateCategory(category) || !ValidateSize(category, size))
		return std::tr1::shared_ptr<std::vector<char> >();

	//std::ifstream stream;
	std::string path(GetFilePath(category, id, size));
    FILE * fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
        return std::tr1::shared_ptr<std::vector<char> >();
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

	//stream.open(path, std::ios::binary | std::ios::in);
	// not found or other error
	//if (stream.fail())
	//	return std::tr1::shared_ptr<std::vector<char> >();

	// get length
	//stream.seekg(0, std::ios::end);
	//int length = stream.tellg();
	//stream.seekg(0, std::ios::beg);

	std::tr1::shared_ptr<std::vector<char> > ret = std::tr1::shared_ptr<std::vector<char> >(new std::vector<char>());
	ret->resize(length);

	// HACK
	//stream.read(&((*ret)[0]), length);
    fread(&((*ret)[0]), 1, length, fp);

	return ret;
}

std::string ImageServer::GetFilePath(std::string& category, uint32 id, uint32 size)
{
    std::string extension = category == "Character" ? "jpg" : "png";

	// HACK: We don't have any other
	size = 512;

	std::stringstream builder;
	builder << _basePath << category << "/" << id << "_" << size << "." << extension;
	return builder.str();
}

bool ImageServer::ValidateSize(std::string& category, uint32 size)
{
	if (category == "InventoryType")
		return size == 64 || size == 32;

	if (category == "Alliance")
		return size == 128 || size == 64 || size == 32;

	if (category == "Corporation")
		return size == 256 || size == 128 || size == 64 || size == 32;

	// Render and Character
	return size == 512 || size == 256 || size == 128 || size == 64 || size == 32;
}

bool ImageServer::ValidateCategory(std::string& category)
{
	for (int i = 0; i < 5; i++)
		if (category == Categories[i])
			return true;
	return false;
}

std::string& ImageServer::url()
{
	return _url;
}

void ImageServer::Run()
{
    _ioThread = std::auto_ptr<asio::thread>(new asio::thread(std::tr1::bind(&ImageServer::RunInternal, this)));
}

void ImageServer::RunInternal()
{
    _io = std::auto_ptr<asio::io_service>(new asio::io_service());
	_listener = std::auto_ptr<ImageServerListener>(new ImageServerListener(*_io));
	_io->run();
}

ImageServer::Lock::Lock(asio::detail::mutex& mutex)
	: _mutex(mutex)
{
	_mutex.lock();
}

ImageServer::Lock::~Lock()
{
	_mutex.unlock();
}
