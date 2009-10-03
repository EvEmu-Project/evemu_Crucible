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

#include "EVECommonPCH.h"

#include "network/packet_dump.h"

void DumpPacketAscii(const uint8* buf, int32 size, int32 cols, int32 skip) {
	// Output as ASCII
	for(int32 i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0)
		        printf( "\n%3d:", i - skip );
		else if ((i-skip)%(cols/2)==0)
			printf( " - " );

		if (buf[i] > 32 && buf[i] < 127)
			printf( "%c", buf[i] );
		else
		        printf( "." );
	}

	printf( "\n\n" );
}

void DumpPacketHex(const uint8* buf, int32 size, int32 cols, int32 skip) {
	if (size == 0 || size > 39565)
		return;
	// Output as HEX
	int j = 0;
	char* ascii = new char[cols+1];
	memset(ascii, 0, cols+1);
	int32 i;
    for(i=skip; i<size; i++)
    {
		if ((i-skip)%cols==0) {
			if (i != skip)
			        printf( " | %s\n", ascii );
			printf( "%4d: ", i - skip );
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			printf( " - ");
		}
		printf( "%02X ", (unsigned char)buf[i] );

		if (buf[i] >= 32 && buf[i] < 127) {
			ascii[j++] = buf[i];
		}
		else {
			ascii[j++] = '.';
		}
    }
	int32 k = ((i-skip)-1)%cols;
	if (k < 8)
	        printf( "  " );
	for (int32 h = k+1; h < cols; h++)
		printf( "   " );

	printf( " | %s\n", ascii );
	SafeDeleteArray(ascii);
}

void DumpPacket(const uint8* buf, int32 size)
{
	DumpPacketHex(buf, size);
//	DumpPacketAscii(buf,size);
}

void DumpPacketBin(int32 data) {
	DumpPacketBin((uint8*)&data, sizeof(int32));
}

void DumpPacketBin(int16 data) {
	DumpPacketBin((uint8*)&data, sizeof(int16));
}

void DumpPacketBin(int8 data) {
	DumpPacketBin((uint8*)&data, sizeof(int8));
}


void DumpPacketBin(const void* iData, int32 len) {
	if (!len)
		return;
	const int8* data = (const int8*) iData;
	int32 k=0;
	for (k=0; k<len; k++) {
		if (k % 4 == 0) {
			if (k != 0) {
			        printf( " | %02X %02X %02X %02X\n",
				        (int) data[k-4],
				        (int) data[k-3],
				        (int) data[k-2],
				        (int) data[k-1] );
			}
			printf( "%04u:", k );
		}
		else if (k % 2 == 0)
		        printf( " " );

		printf( " %c%c%c%c%c%c%c%c",
			data[k] & 0x01 ? '1' : '0',
			data[k] & 0x02 ? '1' : '0',
			data[k] & 0x04 ? '1' : '0',
			data[k] & 0x08 ? '1' : '0',
			data[k] & 0x10 ? '1' : '0',
			data[k] & 0x20 ? '1' : '0',
			data[k] & 0x40 ? '1' : '0',
			data[k] & 0x80 ? '1' : '0' );
	}

	int8 tmp = (k % 4);
	if (!tmp)
		tmp = 4;

	if (tmp <= 3)
	        printf( "         " );
        if (tmp <= 2)
	        printf( "         " );
	if (tmp <= 1)
	        printf( "         " );

	printf( " | %02X", (int) data[k-4] );

	if (tmp > 1)
	        printf( " %02X", (int)data[k-3] );
	if (tmp > 2)
	        printf( " %02X", (int)data[k-2] );
	if (tmp > 3)
	        printf( " %02X", (int)data[k-1] );

	printf( "\n" );
}

static void _HexDump(const uint8 *data, uint32 length) {
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line((const char *)data,length,offset,buffer,4);
		printf("%s\n", buffer);
	}
}

void DumpPacketPreview(const uint8* data, int32 length) {
	char buffer[80];

	if(length > 256) {
		_HexDump(data, 256-32);
		printf(" ... truncated ...\n");
		build_hex_line((const char *)data,length,length-16,buffer,4);
		printf("%s\n", buffer);
	} else {
		_HexDump(data, length);
	}
}
