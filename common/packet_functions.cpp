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


#include <string.h>
#include <zlib.h>

#include "../common/common.h"
#include "../common/logsys.h"

#include "packet_dump.h"
#include "packet_functions.h"

#ifndef WIN32
	#include <netinet/in.h>
#endif

//#define MEMORY_DEBUG

#ifndef MEMORY_DEBUG
#define e_alloc_func Z_NULL
#define e_free_func Z_NULL
#else
//These functions only exist to make my memory profiler
voidpf e_alloc_func(voidpf opaque, uInt items, uInt size);
void e_free_func(voidpf opaque, voidpf address);

voidpf e_alloc_func(voidpf opaque, uInt items, uInt size) {
	voidpf tmp = new byte[items * size];
	return(tmp);
}

void e_free_func(voidpf opaque, voidpf address) {
	delete[] (byte *)address;
}
#endif

//returns ownership of buffer!
byte *DeflatePacket(const byte *data, uint32 &length) {
#ifdef REUSE_ZLIB
	static bool inited = false;
	static z_stream zstream;
    int zerror;
    
    if(in_data == NULL && out_data == NULL && in_length == 0 && max_out_length == 0) {
    	//special delete state
    	deflateEnd(&zstream);
    	return(0);
    }
    if(!inited) {
		memset(&zstream, 0, sizeof(zstream));
		zstream.zalloc    = e_alloc_func;
		zstream.zfree     = e_free_func;
		zstream.opaque    = Z_NULL;
		deflateInit(&zstream, Z_FINISH);
    }
	
	zstream.next_in   = const_cast<unsigned char *>(in_data);
	zstream.avail_in  = in_length;
/*	zstream.zalloc    = Z_NULL;
	zstream.zfree     = Z_NULL;
	zstream.opaque    = Z_NULL;
	deflateInit(&zstream, Z_FINISH);*/
	zstream.next_out  = out_data;
	zstream.avail_out = max_out_length;
	zerror = deflate(&zstream, Z_FINISH);
	
	deflateReset(&zstream);
	
	if (zerror == Z_STREAM_END)
	{
//		deflateEnd(&zstream);
		return zstream.total_out;
	}
	else
	{
//		zerror = deflateEnd(&zstream);
		return 0;
	}
#else
	if(data == NULL || length == 0)
		return(0);

	z_stream zstream;
	zstream.next_in   = const_cast<byte *>(data);
	zstream.avail_in  = length;
	zstream.zalloc    = e_alloc_func;
	zstream.zfree     = e_free_func;
	zstream.opaque    = Z_NULL;

	int zerror = deflateInit(&zstream, Z_DEFAULT_COMPRESSION);
	if(zerror != Z_OK) {
		_log(COMMON__ERROR, "Error: DeflatePacket: deflateInit() returned %d (%s).",
			zerror, (zstream.msg == NULL ? "No additional message" : zstream.msg));

		return(0);
	}

	length = deflateBound(&zstream, length);
	byte *out_data = new byte[length];

	zstream.next_out  = out_data;
	zstream.avail_out = length;

	zerror = deflate(&zstream, Z_FINISH);

	if(zerror == Z_STREAM_END) {
		//deflation successfull
		deflateEnd(&zstream);
		//truncate output buffer to necessary size
		length = zstream.total_out;
		out_data = (byte *)realloc(out_data, length);

		return(out_data);
	} else {
		//error occured
		_log(COMMON__ERROR, "Error: DeflatePacket: deflate() returned %d (%s).",
			zerror, (zstream.msg == NULL ? "No additional message" : zstream.msg));

		deflateEnd(&zstream);
		//delete output buffer
		length = 0;
		delete out_data;

		return(NULL);
	}
#endif
}

//returns ownership of buffer!
byte *InflatePacket(const byte *data, uint32 &length, bool quiet) {
#ifdef REUSE_ZLIB
	static bool inited = false;
	static z_stream zstream;
    int zerror;
    
    if(indata == NULL && outdata == NULL && indatalen == 0 && outdatalen == 0) {
    	//special delete state
    	inflateEnd(&zstream);
    	return(0);
    }
    if(!inited) {
		zstream.zalloc    = e_alloc_func;
		zstream.zfree     = e_free_func;
		zstream.opaque    = Z_NULL;
		inflateInit2(&zstream, 15);
    }

	zstream.next_in		= const_cast<unsigned char *>(indata);
	zstream.avail_in	= indatalen;
	zstream.next_out	= outdata;
	zstream.avail_out	= outdatalen;
	zstream.zalloc    = e_alloc_func;
	zstream.zfree     = e_free_func;
	zstream.opaque		= Z_NULL;
	
	i = inflateInit2( &zstream, 15 ); 
	if (i != Z_OK) { 
		return 0;
	}
	
	zerror = inflate( &zstream, Z_FINISH );
	
	inflateReset(&zstream);
	
	if(zerror == Z_STREAM_END) {
		return zstream.total_out;
	}
	else {
		if (!iQuiet) {
			cout << "Error: InflatePacket: inflate() returned " << zerror << " '";
			if (zstream.msg)
				cout << zstream.msg;
			cout << "'" << endl;
		}
		
		if (zerror == -4 && zstream.msg == 0)
		{
			return 0;
		}
		
		return 0;
	}
#else
	if(data == NULL || length == 0)
		return(0);

	z_stream zstream;	
	zstream.next_in   = const_cast<byte *>(data);
	zstream.avail_in  = length;
	zstream.zalloc    = e_alloc_func;
	zstream.zfree     = e_free_func;
	zstream.opaque    = Z_NULL;

	int zerror = inflateInit2(&zstream, 15);
	if(zerror != Z_OK) {
		if(!quiet)
			_log(COMMON__ERROR, "Error: InflatePacket: inflateInit2() returned %d (%s).",
				zerror, (zstream.msg == NULL ? "No additional message" : zstream.msg));

		return(0);
	}

	byte *out_data = NULL;
	zstream.total_out = 0;
	do {
		length *= 2;	//increase buffer size
		out_data = (byte *)realloc(out_data, length);	//(re)allocate output buffer twice as big as before

		zstream.next_out  = &out_data[zstream.total_out];
		zstream.avail_out = length - zstream.total_out;

		zerror = inflate(&zstream, Z_FINISH);
	} while(zerror == Z_BUF_ERROR);	//continue while we get "not enough room in buffer" error

	if(zerror == Z_STREAM_END) {
		//inflation successfull
		inflateEnd(&zstream);
		//truncate output buffer to necessary size
		length = zstream.total_out;
		out_data = (byte *)realloc(out_data, length);

		return(out_data);
	} else {
		//error occured
		if(!quiet)
			_log(COMMON__ERROR, "Error: InflatePacket: inflate() returned %d (%s).",
				zerror, (zstream.msg == NULL ? "No additional message" : zstream.msg));

		inflateEnd(&zstream);
		//delete output buffer
		length = 0;
		delete out_data;

		return(NULL);
	}
#endif
}

int32 roll(int32 in, int8 bits) {
	return ((in << bits) | (in >> (32-bits)));
}

int64 roll(int64 in, int8 bits) {
	return ((in << bits) | (in >> (64-bits)));
}

int32 rorl(int32 in, int8 bits) {
	return ((in >> bits) | (in << (32-bits)));
}

int64 rorl(int64 in, int8 bits) {
	return ((in >> bits) | (in << (64-bits)));
}



