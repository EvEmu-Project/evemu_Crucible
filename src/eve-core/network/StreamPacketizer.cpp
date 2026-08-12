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

#include "eve-core.h"

#include "network/StreamPacketizer.h"

StreamPacketizer::~StreamPacketizer()
{
    ClearBuffers();
}

bool StreamPacketizer::InputData( const Buffer& data )
{
    if (mInvalid || mBuffer.size() > EveProtocol::MAX_BUFFERED_PACKET_BYTES)
        return false;

    const std::size_t available =
        EveProtocol::MAX_BUFFERED_PACKET_BYTES - mBuffer.size();
    if (data.size() > available) {
        mInvalid = true;
        return false;
    }

    mBuffer.AppendSeq( data.begin<uint8>(), data.end<uint8>() );

    if (mBuffer.size() >= sizeof(uint32)) {
        const uint32 length = *mBuffer.begin<uint32>();
        if (length == 0 || length > EveProtocol::MAX_PACKET_SIZE) {
            mInvalid = true;
            return false;
        }
    }

    return true;
}

bool StreamPacketizer::Process()
{
    if (mInvalid)
        return false;

    Buffer::const_iterator<uint8> cur = mBuffer.begin<uint8>(), end = mBuffer.end<uint8>();
    while( true ) {
        if (sizeof(uint32) > (end - cur))
            break;

        const Buffer::const_iterator<uint32> len = cur.As<uint32>();
        if (*len == 0 || *len > EveProtocol::MAX_PACKET_SIZE) {
            mInvalid = true;
            return false;
        }

        const Buffer::const_iterator<uint8> start = ( len + 1 ).As<uint8>();

        if (*len > (uint32)(end - start))
            break;

        if (mPackets.size() >= EveProtocol::MAX_QUEUED_PACKET_COUNT ||
            static_cast<std::size_t>(*len) >
                EveProtocol::MAX_QUEUED_PACKET_BYTES - mQueuedBytes) {
            mInvalid = true;
            return false;
        }

        mPackets.push( new Buffer(start, start + *len));
        mQueuedBytes += *len;
        cur = (start + *len);
    }

    if( cur != mBuffer.begin<uint8>() )
        mBuffer.AssignSeq( cur, end );

    if (mBuffer.size() > EveProtocol::MAX_PACKET_SIZE + sizeof(uint32)) {
        mInvalid = true;
        return false;
    }

    return true;
}

Buffer* StreamPacketizer::PopPacket()
{
    Buffer* buf(nullptr);
    if (!mPackets.empty()) {
        buf = mPackets.front();
        mPackets.pop();
        mQueuedBytes -= buf->size();
    }

    return buf;
}

void StreamPacketizer::ClearBuffers()
{
    Buffer* buf(nullptr);
    while ((buf = PopPacket()))
        SafeDelete( buf );

    mBuffer.Resize<uint8>(0);
    mQueuedBytes = 0;
    mInvalid = false;
}
