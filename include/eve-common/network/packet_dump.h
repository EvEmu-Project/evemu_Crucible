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

#ifndef PACKET_DUMP_H
#define PACKET_DUMP_H

void DumpPacketAscii(const uint8* buf, int32 size, int32 cols=16, int32 skip=0);
void DumpPacketHex(const uint8* buf, int32 size, int32 cols=16, int32 skip=0);
void DumpPacketBin(const void* data, int32 len);
void DumpPacket(const uint8* buf, int32 size);
void DumpPacketBin(int32 data);
void DumpPacketBin(int16 data);
void DumpPacketBin(int8 data);
void DumpPacketPreview(const uint8* buf, int32 size);

#endif
