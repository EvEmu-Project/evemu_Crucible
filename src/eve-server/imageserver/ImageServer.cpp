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

#include "EVE_AgentHelpers.h"
#include "imageserver/ImageServer.h"
#include "imageserver/ImageServerListener.h"
#include <cstring>

const char *const ImageServer::FallbackURL = "https://images.evetech.net/";

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
        _basePath = "../image_cache";
    if (_basePath.back() != '/' && _basePath.back() != '\\')
        _basePath += "/";

    sLog.Cyan("      ImageServer", "Image Server URL: %s", _url.c_str());
    sLog.Cyan("      ImageServer", "Image Server path: %s", _basePath.c_str());

    // Always ensure category folders exist: CreateDirectory returns nonzero
    // when it successfully creates the base folder. The old code skipped
    // creating subfolders in that case.
    CreateDirectory(_basePath.c_str(), NULL);
    for (uint32 i = 0; i < CategoryCount; ++i) {
        const std::string subdir = _basePath + Categories[i];
        CreateDirectory(subdir.c_str(), NULL);
    }
    sLog.Blue("      ImageServer", "Image Server Initalized.");
}

void ImageServer::ReportNewImage(uint32 accountID, std::shared_ptr<std::vector<char> > imageData)
{
    sLog.Warning("      ImageServer"," ReportNewImage() called.");
    Lock lock(_limboLock);

    // insert() does not replace existing map entries. A second upload from
    // the same account must replace the stale image, not silently get ignored.
    if (imageData == nullptr || imageData->empty()) {
        sLog.Warning("      ImageServer", "Ignoring empty portrait upload for account %u.", accountID);
        return;
    }
    _limboImages[accountID] = imageData;
}

void ImageServer::ReportNewCharacter(uint32 creatorAccountID, uint32 characterID)
{
    sLog.Warning("      ImageServer"," ReportNewCharacter() called.");
    Lock lock(_limboLock);

    const auto it = _limboImages.find(creatorAccountID);
    if (it == _limboImages.end() || it->second == nullptr || it->second->empty()) {
        sLog.Warning("      ImageServer", "No valid portrait upload for characterID %u.", characterID);
        return;
    }

    std::string dirName = "Character";
    const std::string path = GetFilePath(dirName, characterID, 512);
    FILE* fp = fopen(path.c_str(), "wb");
    if (fp == nullptr) {
        sLog.Error("      ImageServer", "Could not open portrait file %s for writing.", path.c_str());
        return;
    }

    const std::shared_ptr<std::vector<char>>& data = it->second;
    const size_t bytesWritten = fwrite(data->data(), 1, data->size(), fp);
    const int closeResult = fclose(fp);
    if (bytesWritten != data->size() || closeResult != 0) {
        sLog.Error("      ImageServer", "Failed to write complete portrait for characterID %u.", characterID);
        remove(path.c_str());
        return;
    }

    // Only discard the upload after the portrait has been saved completely.
    _limboImages.erase(it);
    sLog.Green("      ImageServer", "Received image from %u and saved as %s",
               creatorAccountID, path.c_str());
}

std::shared_ptr<std::vector<char> > ImageServer::GetImage(std::string& category, uint32 id, uint32 size)
{
    sLog.Cyan("      ImageServer"," GetImage() called. Cat: %s, id: %u, size:%u", category.c_str(), id, size);

    if (!ValidateCategory(category) || !ValidateSize(category, size))
        return std::shared_ptr<std::vector<char> >();

    // Only disk files are used for station NPC agents. Exact-size historical
    // portraits take priority, followed by whatever other local JPEG size is
    // available. An importer's 256px JPEG is valid for Crucible's 50px URL.
    std::string path(GetFilePath(category, id, size));
    FILE * fp = fopen(path.c_str(), "rb");
    if (fp == NULL && category == "Character") {
        for (uint32 cachedSize : {512u, 256u, 128u, 64u, 50u, 40u, 32u, 1024u}) {
            if (cachedSize == size)
                continue;
            path = GetFilePath(category, id, cachedSize);
            fp = fopen(path.c_str(), "rb");
            if (fp != NULL)
                break;
        }
    }

    if (fp == NULL && EvEAgent::IsStationAgentPortraitRequest(category, id)) {
        // A custom operator-supplied local JPEG still takes precedence.
        path = _basePath + "Character/agent_missing_512.jpg";
        fp = fopen(path.c_str(), "rb");

        // Never rely on current working directory or archive installation
        // layout to show NPCs. CMake also packages the JPEG for operators,
        // but the image service always has this verified built-in copy.
        if (fp == NULL)
            return GetOfflineAgentPortrait();
    }
    if (fp == NULL)
        return std::shared_ptr<std::vector<char> >();
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return std::shared_ptr<std::vector<char>>();
    }
    const long fileLength = ftell(fp);
    if (fileLength < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return std::shared_ptr<std::vector<char>>();
    }

    const size_t length = static_cast<size_t>(fileLength);
    std::shared_ptr<std::vector<char>> ret = std::make_shared<std::vector<char>>(length);
    const size_t bytesRead = (length > 0) ? fread(ret->data(), 1, length, fp) : 0;
    fclose(fp);
    if (bytesRead != length)
        return std::shared_ptr<std::vector<char>>();
    return ret;
}

std::shared_ptr<std::vector<char> > ImageServer::GetOfflineAgentPortrait()
{
    // Exact bytes of assets/agent_portraits/agent_missing_512.jpg; the unit
    // test compares the decoded payload to that file. This is the original
    // generic fallback, not a claim to be authentic CCP historical artwork.
    static const std::shared_ptr<std::vector<char> > portrait = []() {
        const char* encoded =
        "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAsHCAkIBwsJCQkMCwsNEBoREA8PECAXGBMaJiIoKCYiJSQqMD0zKi05LiQlNUg1"
        "OT9BREVEKTNLUEpCTz1DREH/2wBDAQsMDBAOEB8RER9BLCUsQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFB"
        "QUFBQUFBQUH/wAARCACAAIADASIAAhEBAxEB/8QAGgAAAgMBAQAAAAAAAAAAAAAAAAECAwQFBv/EADgQAAICAQEFBQYEBAcA"
        "AAAAAAABAgMRBAUSITFREyJBYXEVQlJUkZMyM4HBFKGx0SQ0RIKDkuH/xAAYAQEBAQEBAAAAAAAAAAAAAAAAAQIDBP/EABYR"
        "AQEBAAAAAAAAAAAAAAAAAAARAf/aAAwDAQACEQMRAD8A8MMQz1uIGAFQDAAAAGAgGIAEMAEIYAIQxEUwAYAMQyoCUISm8RTb"
        "JV1px35y3Y+HWXoSlc8bta7OPPCfH6gHYqP5lkYeXNhjTrg5WS80kkVEq6rLXu1wlN88RWQJ/wCHfDNi83gOxjL8u2Mn0fAj"
        "ZTbVjtK5wzy3otZIASsrnW8Si0QLYXSit2Xfi+DiwnXGUd+t+sfGIFQhiAQDERQMQwAtphFpzseIR/m+hXFOTSXN8EW3yw1V"
        "F92HD1fiyohObnLL/RdBCGB0dlbNeqkrbU1Sn/2PQQhCuKjCKjFcklhEdNUqKK6ljEYpZSxnzLDOoUoqUXGSTT4NPxOHtfZf"
        "Z51Gnj3OcoL3fNeR3RSipRcZJNNYafiB40cZOMlKLw0Svr7K+yvOdyTjnrggaVZZGModrD0lHo/7FJbTZuT4/glwkvIjZB12"
        "OD8AICGIKAEMgv03dcrfgXD1fIqLY93Sya96ai/04lRUA0IAPY02K2qFkc4lFNZJnB2PtJUpaa5pQz3ZfD6neMoAA5G2NpqM"
        "ZaalpyfCcunkBx9TYrdRbZHKUpuSz5sqADSgtt79NdnRbj/YqLYd7S2J+61JfrwApEAEUhkRgX/6P/k/YqLae9RbDm0lJL+p"
        "UVDNOj0F+sb7KKUU8OUuCRZsnQPWXKU4vsYvvPlnyPSwhCuKhCKjFcklhEo5tGw9NDDtnOxrmuSf7/zN1Gmq0/CpSSxjDm2v"
        "o2XARFd1EL1u2b2OWFJrPrgw3bE0s8uDnW8cMPKz14nSADy+t2Zfo05tKdfxr9zGezlFSi4ySafBp+J5/bWz/wCHn29MMVS/"
        "El7r/sWq5ZbV/lrv9v8AUqLX3dIvBznn1S/9KKRABFIZEYFunmq7U5fhfB+grIOuxwfgyBt0UY6m6hNJyhOKafKUc9Aj0Wg0"
        "y0mlhUsZSzJrxfiaAAiAAAgAAAArvqjfTOqazGSwywAPGWVShfKl4coyceHXOCWpkt9Vx5QW76vxN21Yx0+uutwk21uJdWuL"
        "OWaUCAQUhkN5dUPej1X1IJlmmtdOortSzuSTxnGSnej8S+ob8fiX1KPdAcnY21KJaONd99cJ193vSSyvA3e0NF83R9xGWWgD"
        "P7Q0XzdH3EHtDRfN0fcQGgDP7Q0XzdH3EHtDRfN0fcQGgDP7Q0XzdH3EZ9obV01OlnKnUVTsaxFRmnx6gcLa1ru2hdJrGJbq"
        "Wc8uBkI78fiX1Dej8S+ppoxC3o9V9Rby6ogpAAMNAAAAAAAAAAAAAAAAAAAAAAAD/9k=";
        const char* alphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        auto decoded = std::make_shared<std::vector<char>>();
        decoded->reserve(1600);
        unsigned int value = 0;
        int bits = -8;
        for (const unsigned char ch : std::string(encoded)) {
            if (ch == '=')
                break;
            const char* at = std::strchr(alphabet, ch);
            if (at == nullptr)
                continue;
            value = (value << 6) | static_cast<unsigned int>(at - alphabet);
            bits += 6;
            if (bits >= 0) {
                decoded->push_back(static_cast<char>((value >> bits) & 0xff));
                bits -= 8;
            }
        }
        if (decoded->size() < 128
            || static_cast<unsigned char>((*decoded)[0]) != 0xff
            || static_cast<unsigned char>((*decoded)[1]) != 0xd8
            || static_cast<unsigned char>((*decoded)[decoded->size() - 2]) != 0xff
            || static_cast<unsigned char>((*decoded)[decoded->size() - 1]) != 0xd9)
            return std::shared_ptr<std::vector<char> >();
        return decoded;
    }();
    return portrait;
}

std::string ImageServer::GetFilePath(std::string& category, uint32 id, uint32 size)
{
    std::string extension = category == "Character" ? "jpg" : "png";

    // Preserve the requested size in the filename; GetImage() explicitly
    // falls back to local 512px portraits if a smaller size is absent.
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

    // Legacy Crucible clients also request 50px character portraits.
    if (category == "Character" && size == 50)
        return true;
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
