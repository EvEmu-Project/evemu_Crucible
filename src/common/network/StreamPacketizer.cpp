/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
    mBuffer << data;
}

void StreamPacketizer::Process()
{
    while(   mBuffer.isAvailable<uint32>()
          && mBuffer.isAvailable( sizeof( uint32 ) + mBuffer.Peek<uint32>() ) )
    {
        // Push new packet to the queue
        uint32 len = mBuffer.Read<uint32>();

        mPackets.push( new Buffer( mBuffer.Read<uint8>( len ), len ) );
    }

    mBuffer.DropRead();
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






