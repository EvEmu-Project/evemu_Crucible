/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef PACKET_DUMP_H
#define PACKET_DUMP_H

#include "../common/types.h"

void DumpPacketAscii(const uchar* buf, int32 size, int32 cols=16, int32 skip=0);
void DumpPacketHex(const uchar* buf, int32 size, int32 cols=16, int32 skip=0);
void DumpPacketBin(const void* data, int32 len);
void DumpPacket(const uchar* buf, int32 size);
void DumpPacketBin(int32 data);
void DumpPacketBin(int16 data);
void DumpPacketBin(int8 data);
void DumpPacketPreview(const uchar* buf, int32 size);

#endif
