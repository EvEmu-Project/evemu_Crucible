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
	Author:		Zhur
*/

#include "CommonPCH.h"

#include "network/StreamPacketizer.h"

StreamPacketizer::~StreamPacketizer()
{
	ClearBuffers();
}

void StreamPacketizer::InputData( const Buffer& data )
{
    mBuffer.AppendSeq( data.begin<uint8>(), data.end<uint8>() );
}

void StreamPacketizer::Process()
{
    Buffer::const_iterator<uint8> cur, end;
    cur = mBuffer.begin<uint8>();
    end = mBuffer.end<uint8>();
    while( true )
    {
        if( sizeof( uint32 ) > ( end - cur ) )
            break;

        const Buffer::const_iterator<uint32> len = cur.As<uint32>();
        const Buffer::const_iterator<uint8> start = ( len + 1 ).As<uint8>();

        if( *len > (uint32)( end - start ) )
            break;

        mPackets.push( new Buffer( start, start + *len ) );
        cur = ( start + *len );
    }

    if( cur != mBuffer.begin<uint8>() )
        mBuffer.AssignSeq( cur, end );
}

Buffer* StreamPacketizer::PopPacket()
{
    Buffer* ret = NULL;
    if( !mPackets.empty() )
    {
        ret = mPackets.front();
        mPackets.pop();
    }

    return ret;
}

void StreamPacketizer::ClearBuffers()
{
    Buffer* buf;
    while( ( buf = PopPacket() ) )
        SafeDelete( buf );
}






