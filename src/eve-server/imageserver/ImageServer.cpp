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

/** @todo  boost is the only system in this code that does NOT leak */

#include "imageserver/ImageServer.h"
#include "imageserver/ImageServerListener.h"

#include <cerrno>
#include <cstdio>

const char *const ImageServer::FallbackURL = "https://image.eveonline.com/";

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
    urlBuilder << "http://" << sConfig.net.imageServer << ":" << sConfig.net.imageServerPort << "/";
    _url = urlBuilder.str();

    _basePath = sConfig.files.imageDir;
    if (_basePath.empty())
        throw std::invalid_argument("Image server directory is required");
    if (_basePath.back() != '/')
        _basePath += "/";

    sLog.Cyan("      ImageServer", "Image Server URL: %s", _url.c_str());
    sLog.Cyan("      ImageServer", "Image Server path: %s", _basePath.c_str());

    if (CreateDirectory( _basePath.c_str(), NULL ) == 0) {
        for (int i = 0; i < CategoryCount; i++) {
            std::string subdir = _basePath;
            subdir.append(Categories[i]);
            CreateDirectory( subdir.c_str(), NULL );
        }
    } /* else directory probably exists */
    sLog.Blue("      ImageServer", "Image Server Initalized.");
}

bool ImageServer::ReportNewImage(uint32 accountID, std::shared_ptr<std::vector<char> > imageData)
{
    if (!imageData || imageData->empty() ||
        imageData->size() > ImageServerLimits::MAX_IMAGE_BYTES) {
        sLog.Warning("      ImageServer", "Rejected image upload size.");
        return false;
    }

    Lock lock(_limboLock);

    if (_limboImages.find(accountID) == _limboImages.end() &&
        _limboImages.size() >= ImageServerLimits::MAX_PENDING_UPLOADS) {
        sLog.Warning("      ImageServer", "Rejected image upload capacity.");
        return false;
    }

    _limboImages[accountID] = imageData;
    return true;
}

void ImageServer::ReportNewCharacter(uint32 creatorAccountID, uint32 characterID)
{
    sLog.Warning("      ImageServer"," ReportNewCharacter() called.");
    Lock lock(_limboLock);

    // check if we received an image from this account previously
    if (_limboImages.find(creatorAccountID) == _limboImages.end()) {
        sLog.Error("      ImageServer"," Image not received for characterID %u.", characterID);
        /** @todo  need to get client here, and send msg about emailing char pic and name to charPics@eve.alasiya.net for manual insertion */
        return;
    }

    // we have, so save it
    //std::ofstream stream;
    std::string dirName = "Character";
    std::string path(GetFilePath(dirName, characterID, 512));
    FILE * fp = fopen(path.c_str(), "wb");

    //stream.open(path, std::ios::binary | std::ios::trunc | std::ios::out);
    std::shared_ptr<std::vector<char> > data = _limboImages[creatorAccountID];
    if (!data || data->empty() ||
        data->size() > ImageServerLimits::MAX_IMAGE_BYTES || fp == NULL) {
        if (fp != NULL)
            fclose(fp);
        _limboImages.erase(creatorAccountID);
        sLog.Error("      ImageServer", "Unable to save uploaded image.");
        return;
    }

    const std::size_t written = fwrite(
        data->data(), 1, data->size(), fp);
    const int closeResult = fclose(fp);
    if (written != data->size() || closeResult != 0) {
        _limboImages.erase(creatorAccountID);
        sLog.Error("      ImageServer", "Unable to save uploaded image.");
        return;
    }

    //std::copy(data->begin(), data->end(), std::ostream_iterator<char>(stream));
    //stream.flush();
    //stream.close();

    /** @todo  we will need to make size 64 and size 40 images, and possibably 128/256 of char portaits */
    // github.com/nothings/stb/blob/master/stb_image_resize.h
    // github.com/nothings/stb/blob/master/stb_image.h

    // and delete it from our limbo map
    _limboImages.erase(creatorAccountID);

    sLog.Green("      ImageServer", "Received image from %u and saved as %s", creatorAccountID, path.c_str());
}

bool ImageServer::RemoveCharacterImage(uint32 characterID)
{
    std::string category = "Character";
    const std::string path = GetFilePath( category, characterID, 512 );
    if ( std::remove( path.c_str() ) == 0 || errno == ENOENT )
        return true;

    sLog.Warning(
        "      ImageServer",
        "Unable to remove character image for %u.",
        characterID );
    return false;
}

std::shared_ptr<std::vector<char> > ImageServer::GetImage(std::string& category, uint32 id, uint32 size)
{
    sLog.Cyan("      ImageServer"," GetImage() called. Cat: %s, id: %u, size:%u", category.c_str(), id, size);

    if (!ValidateCategory(category) || !ValidateSize(category, size))
        return std::shared_ptr<std::vector<char> >();

    //std::ifstream stream;
    std::string path(GetFilePath(category, id, size));
    FILE * fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
        return std::shared_ptr<std::vector<char> >();
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return std::shared_ptr<std::vector<char> >();
    }

    const long fileLength = ftell(fp);
    if (fileLength <= 0 ||
        static_cast<std::size_t>(fileLength) >
            ImageServerLimits::MAX_IMAGE_BYTES ||
        fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return std::shared_ptr<std::vector<char> >();
    }

    const std::size_t length = static_cast<std::size_t>(fileLength);

    //stream.open(path, std::ios::binary | std::ios::in);
    // not found or other error
    //if (stream.fail())
    //    return std::shared_ptr<std::vector<char> >();

    // get length
    //stream.seekg(0, std::ios::end);
    //int length = stream.tellg();
    //stream.seekg(0, std::ios::beg);

    std::shared_ptr<std::vector<char> > ret;
    try {
        ret = std::make_shared<std::vector<char> >(length);
    } catch (const std::bad_alloc&) {
        fclose(fp);
        throw;
    }

    // HACK
    //stream.read(&((*ret)[0]), length);
    const std::size_t read = fread(ret->data(), 1, length, fp);
    const int closeResult = fclose(fp);
    if (read != length || closeResult != 0)
        return std::shared_ptr<std::vector<char> >();

    return ret;
}

std::string ImageServer::GetFilePath(std::string& category, uint32 id, uint32 size)
{
    std::string extension = category == "Character" ? "jpg" : "png";

    std::stringstream builder;
    builder << _basePath << category << "/" << id << "_" << size << "." << extension;
    return builder.str();
}

bool ImageServer::ValidateSize(std::string& category, uint32 size)
{
    if (category == "InventoryType")
        return size == 64 || size == 32;

    if (category == "Alliance")
        return size == 256 || size == 128 || size == 64 || size == 32;

    if (category == "Corporation")
        return size == 256 || size == 128 || size == 64 || size == 32;

    // Render and Character
    return size == 1024 || size == 512 || size == 256 || size == 128 || size == 64 || size == 40 || size == 32;
}

bool ImageServer::ValidateCategory(std::string& category)
{
    for (uint32 i = 0; i < CategoryCount; i++)
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
    _ioThread = std::shared_ptr<boost::asio::detail::thread>(new boost::asio::detail::thread(std::bind(&ImageServer::RunInternal, this)));
}

void ImageServer::Stop()
{
    _io->stop();
    _ioThread->join();
}

void ImageServer::RunInternal()
{
    _io = std::shared_ptr<boost::asio::io_context>(new boost::asio::io_context());
    _listener = std::shared_ptr<ImageServerListener>(new ImageServerListener(*_io));
    _io->run();
}

ImageServer::Lock::Lock(boost::asio::detail::mutex& mutex)
    : _mutex(mutex)
{
    _mutex.lock();
}

ImageServer::Lock::~Lock()
{
    _mutex.unlock();
}
