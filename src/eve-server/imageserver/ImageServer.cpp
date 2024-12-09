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
    urlBuilder << "http://" << sConfig.net.imageServer << ":" << sConfig.net.imageServerPort << "/";
    _url = urlBuilder.str();

    _basePath = sConfig.files.imageDir;
    if (_basePath[_basePath.size() - 1] != '/')
        _basePath += "/";

    sLog.Cyan("      ImageServer", "Image Server URL: %s", _url.c_str());
    sLog.Cyan("      ImageServer", "Image Server path: %s", _basePath.c_str());

    #ifdef _UNICODE
        wchar_t widePath[MAX_PATH];
        #ifdef _WIN32
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, widePath, MAX_PATH, _basePath.c_str(), _TRUNCATE);
        #else
            mbstowcs(widePath, _basePath.c_str(), MAX_PATH);
        #endif
        if (CreateDirectoryW(widePath, NULL) == 0) {
            for (int i = 0; i < CategoryCount; i++) {
                std::string subdir = _basePath;
                subdir.append(Categories[i]);
                #ifdef _WIN32
                    mbstowcs_s(&convertedChars, widePath, MAX_PATH, subdir.c_str(), _TRUNCATE);
                #else
                    mbstowcs(widePath, subdir.c_str(), MAX_PATH);
                #endif
                CreateDirectoryW(widePath, NULL);
            }
        }
    #else
        if (CreateDirectoryA(_basePath.c_str(), NULL) == 0) {
            for (int i = 0; i < CategoryCount; i++) {
                std::string subdir = _basePath;
                subdir.append(Categories[i]);
                CreateDirectoryA(subdir.c_str(), NULL);
            }
        }
    #endif

    sLog.Blue("      ImageServer", "Image Server Initalized.");
}

void ImageServer::ReportNewImage(uint32 accountID, std::shared_ptr<std::vector<char> > imageData)
{
    sLog.Warning("      ImageServer"," ReportNewImage() called.");
    Lock lock(_limboLock);

    if (_limboImages.find(accountID) != _limboImages.end()) {
        _limboImages.insert(std::pair<uint32,std::shared_ptr<std::vector<char> > >(accountID, imageData));
    } else {
        _limboImages[accountID] = imageData;
    }
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
    FILE* fp = nullptr;
    #ifdef _WIN32
        errno_t err = fopen_s(&fp, path.c_str(), "wb");
        if (err != 0 || fp == nullptr) {
            sLog.Error("      ImageServer", " Failed to open file for writing: %s", path.c_str());
            return;
        }
    #else
        fp = fopen(path.c_str(), "wb");
        if (fp == nullptr) {
            sLog.Error("      ImageServer", " Failed to open file for writing: %s", path.c_str());
            return;
        }
    #endif

    //stream.open(path, std::ios::binary | std::ios::trunc | std::ios::out);
    std::shared_ptr<std::vector<char> > data = _limboImages[creatorAccountID];

    fwrite(&((*data)[0]), 1, data->size(), fp);
    fclose(fp);

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

std::shared_ptr<std::vector<char> > ImageServer::GetImage(std::string& category, uint32 id, uint32 size)
{
    sLog.Cyan("      ImageServer"," GetImage() called. Cat: %s, id: %u, size:%u", category.c_str(), id, size);

    if (!ValidateCategory(category) || !ValidateSize(category, size))
        return std::shared_ptr<std::vector<char> >();

    //std::ifstream stream;
    std::string path(GetFilePath(category, id, size));
    FILE* fp = nullptr;
    #ifdef _WIN32
        errno_t err = fopen_s(&fp, path.c_str(), "rb");
        if (err != 0 || fp == nullptr)
            return std::shared_ptr<std::vector<char> >();
    #else
        fp = fopen(path.c_str(), "rb");
        if (fp == nullptr)
            return std::shared_ptr<std::vector<char> >();
    #endif
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    //stream.open(path, std::ios::binary | std::ios::in);
    // not found or other error
    //if (stream.fail())
    //    return std::shared_ptr<std::vector<char> >();

    // get length
    //stream.seekg(0, std::ios::end);
    //int length = stream.tellg();
    //stream.seekg(0, std::ios::beg);

    std::shared_ptr<std::vector<char> > ret = std::shared_ptr<std::vector<char> >(new std::vector<char>());
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
        return size == 256 || size == 128 || size == 64 || size == 32;

    if (category == "Corporation")
        return size == 256 || size == 128 || size == 64 || size == 32;

    // Render and Character
    return size == 1024 || size == 512 || size == 256 || size == 128 || size == 64 || size == 40 || size == 32;
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
