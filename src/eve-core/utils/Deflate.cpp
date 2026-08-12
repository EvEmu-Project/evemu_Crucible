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
#include "network/ProtocolLimits.h"

const uint8 DeflateHeaderByte = 0x78; //'x'

bool IsDeflated( const Buffer& data )
{
    return data.size() != 0 && DeflateHeaderByte == data[0];
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

    size_t outputSize = compressBound( input.size() );
    output.ResizeAt( out, outputSize );

    int res = compress( &*out, (uLongf*)&outputSize, &input[0], input.size() );

    if( Z_OK == res )
    {
        output.ResizeAt( out, outputSize );
        return true;
    }
    else
    {
        output.ResizeAt( out, 0 );
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
    if (input.size() == 0 || input.size() > EveProtocol::MAX_PACKET_SIZE)
        return false;

    const Buffer::iterator<uint8> out = output.end<uint8>();

    size_t outputSize = input.size();
    if (outputSize < EveProtocol::MAX_PACKET_SIZE / 2)
        outputSize *= 2;
    else
        outputSize = EveProtocol::MAX_PACKET_SIZE;

    while (true) {
        output.ResizeAt( out, outputSize );

        uLongf uncompressedSize = static_cast<uLongf>(outputSize);
        const int res = uncompress(
            &*out,
            &uncompressedSize,
            &input[0],
            static_cast<uLong>(input.size()));

        if (Z_OK == res) {
            output.ResizeAt(out, static_cast<size_t>(uncompressedSize));
            return true;
        }

        if (res != Z_BUF_ERROR || outputSize == EveProtocol::MAX_PACKET_SIZE)
            break;

        output.ResizeAt(out, 0);
        outputSize = std::min(
            outputSize * 2,
            EveProtocol::MAX_PACKET_SIZE);
    }

    output.ResizeAt(out, 0);
    return false;
}
