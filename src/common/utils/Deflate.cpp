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

#include "log/logsys.h"
#include "utils/Deflate.h"

//#define MEMORY_DEBUG

#ifndef MEMORY_DEBUG
#  define e_alloc_func Z_NULL
#  define e_free_func Z_NULL
#else
//These functions only exist to make my memory profiler
voidpf e_alloc_func(voidpf opaque, uInt items, uInt size)
{
	voidpf tmp = new uint8[items * size];
	return(tmp);
}

void e_free_func(voidpf opaque, voidpf address)
{
	delete[] (uint8 *)address;
}
#endif//MEMORY_DEBUG

//returns ownership of buffer!
uint8* DeflateData( const uint8* data, uint32* length )
{
	if(data == NULL || length == NULL || *length == 0)
		return NULL;

	z_stream zstream;
	zstream.next_in   = const_cast<uint8 *>(data);
	zstream.avail_in  = *length;
	zstream.zalloc    = e_alloc_func;
	zstream.zfree     = e_free_func;
	zstream.opaque    = Z_NULL;

	//int zerror = deflateInit(&zstream, Z_DEFAULT_COMPRESSION);
	int zerror = deflateInit(&zstream, Z_DEFAULT_COMPRESSION);
	
	if(zerror != Z_OK) {
		_log(COMMON__ERROR, "Error: DeflatePacket: deflateInit() returned %d (%s).",
			zerror, (zstream.msg == NULL ? "No additional message" : zstream.msg));

		return NULL;
	}

	*length = deflateBound(&zstream, *length);
	uint8 *out_data = (uint8 *)malloc( *length );

	zstream.next_out  = out_data;
	zstream.avail_out = *length;

	zerror = deflate(&zstream, Z_FINISH);

	if(zerror == Z_STREAM_END) {
		//deflation successfully
		deflateEnd(&zstream);
		//truncate output buffer to necessary size
		*length = zstream.total_out;
		out_data = (uint8 *)realloc(out_data, *length);
		return out_data;
	}
	else
	{
		//error occurred
		_log(COMMON__ERROR, "Error: DeflatePacket: deflate() returned %d (%s).",
			zerror, (zstream.msg == NULL ? "No additional message" : zstream.msg));

		deflateEnd(&zstream);
		//delete output buffer
		*length = 0;
		free( out_data );

		return NULL;
	}
}

/** Inflate the packet and allocate enough memory for decompression
  * Note: returns ownership of buffer!
  */
uint8* InflateData( const uint8* data, uint32* length, bool quiet )
{
    u_long  sourcelen = (u_long)*length;
    Bytef * source = (Bytef *)data;

    /* One of the key things of the ZLIB stuff is that we 'sometimes' don't know the size of the uncompressed data.
     * My idea is to fix this regarding the first phase of the parsing of the data (the parser) is to go trough a
     * couple of output buffer size. First buffer size would be 4x the initial buffer size, implying that the
     * compression ratio is about 75%. The second buffer size would be 8x the initial buffer size, implying that the
     * compression ratio is about 87.5%. The third and last buffer size is 16x the initial buffer size implying that
     * the compression ratio is about 93.75%. This theory is really stupid because there is no way to actually know.
     */

    uint32 bufferMultiplier = 4;
    u_long outBufferLen = sourcelen * bufferMultiplier;
    u_long allocatedBufferLen = outBufferLen;

    Bytef * outBuffer = (Bytef *)malloc(outBufferLen);

    int zlibUncompressResult = uncompress(outBuffer, &outBufferLen, source, sourcelen);

    if( zlibUncompressResult == Z_BUF_ERROR )
    {
        for( int loop_limiter = 0; zlibUncompressResult == Z_BUF_ERROR; loop_limiter++ )
        {
            /* because this code is a possible fuck up, we limit the mount of tries */
            if( loop_limiter > 100 )
            {
                zlibUncompressResult = Z_MEM_ERROR;
                _log(COMMON__ERROR, "uncompress increase buffer overflow safe mechanism");
                break;
            }

            bufferMultiplier *= 2;
            outBufferLen = sourcelen * bufferMultiplier;
            allocatedBufferLen = outBufferLen;

            outBuffer = (Bytef*)realloc(outBuffer, outBufferLen); // resize the output buffer
            zlibUncompressResult = uncompress(outBuffer, &outBufferLen, source, sourcelen); // and try it again
        }
    }

    if( zlibUncompressResult != Z_OK )
    {
        _log(COMMON__ERROR, "uncompress went wrong ***PANIC***");
        free( outBuffer );
        return false;
    }

    *length = outBufferLen;
    return outBuffer;
}
