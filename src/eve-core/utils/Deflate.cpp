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
    Author:        Zhur
*/


#include <zlib.h>

#include "eve-core.h"

#include "utils/Deflate.h"

const uint8 DeflateHeaderByte = 0x78; //'x'

bool IsDeflated( const Buffer& data )
{
    return ( DeflateHeaderByte == data[0] );
}

bool DeflateData( Buffer& data )
{
    Buffer dataDeflated;
    if( !DeflateData( data, dataDeflated ) )
        return false;

    data = dataDeflated;
    return true;
}

bool DeflateData( const Buffer& input, Buffer& output )
{
    const Buffer::iterator<uint8> out = output.end<uint8>();

    // 检查输入大小是否超出 uLong 范围
    if (input.size() > static_cast<size_t>(ULONG_MAX)) {
        sLog.Error("Deflate", "Input size %zu exceeds maximum allowed value", input.size());
        return false;
    }

    // 计算所需的输出缓冲区大小
    uLong inputSize = static_cast<uLong>(input.size());
    size_t outputSize = compressBound(inputSize);

    // 检查输出大小是否超出 uLong 范围
    if (outputSize > static_cast<size_t>(ULONG_MAX)) {
        sLog.Error("Deflate", "Required output size %zu exceeds maximum allowed value", outputSize);
        return false;
    }

    output.ResizeAt(out, outputSize);

    uLong compressedSize = static_cast<uLong>(outputSize);
    int res = compress(&*out, &compressedSize, &input[0], inputSize);

    if (Z_OK == res) {
        output.ResizeAt(out, static_cast<size_t>(compressedSize));
        return true;
    } else {
        output.ResizeAt(out, 0);
        return false;
    }
}

bool InflateData( Buffer& data )
{
    Buffer dataInflated;
    if( !InflateData( data, dataInflated ) )
        return false;

    data = dataInflated;
    return true;
}

bool InflateData( const Buffer& input, Buffer& output )
{
    const Buffer::iterator<uint8> out = output.end<uint8>();

    // 检查输入大小是否超出 uLong 范围
    if (input.size() > static_cast<size_t>(ULONG_MAX)) {
        sLog.Error("Deflate", "Input size %zu exceeds maximum allowed value", input.size());
        return false;
    }

    size_t outputSize = 0;
    size_t sizeMultiplier = 0;
    uLong inputSize = static_cast<uLong>(input.size());

    int res = 0;
    do {
        outputSize = (input.size() << ++sizeMultiplier);
        
        // 检查输出大小是否超出 uLong 范围
        if (outputSize > static_cast<size_t>(ULONG_MAX)) {
            sLog.Error("Deflate", "Required output size %zu exceeds maximum allowed value", outputSize);
            return false;
        }

        output.ResizeAt(out, outputSize);
        uLong uncompressedSize = static_cast<uLong>(outputSize);

        res = uncompress(&*out, &uncompressedSize, &input[0], inputSize);
        
        if (res == Z_OK) {
            outputSize = static_cast<size_t>(uncompressedSize);
        }
    } while (Z_BUF_ERROR == res);

    if (Z_OK == res) {
        output.ResizeAt(out, outputSize);
        return true;
    } else {
        output.ResizeAt(out, 0);
        return false;
    }
}
