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

#include "EVEZeroCompress.h"

#ifdef ZC_TEST
	#include <stdio.h>
	#define _log(x, str, ...) \
		printf(str "\n", ##__VA_ARGS__)
#else
	#include "logsys.h"
#endif

// ZeroCompress OpCode
struct ZCOpCode {
	// first part
	uint8 f_len : 3;
	bool f_isZero : 1;

	// second part
	uint8 s_len : 3;
	bool s_isZero : 1;
};

void UnpackZeroCompressed(const byte *in_buf, uint32 in_length, std::vector<byte> &buffer) {
	buffer.clear();
	if(in_buf == NULL || in_length == 0)
		return;
	
	_log(NET__ZEROINFL, "Zero-inflating buffer of length %d",
		in_length
	);
	
	ZCOpCode opcode;
	const byte *end = in_buf + in_length;

	while(in_buf < end) {
		opcode = *(ZCOpCode *)(in_buf++);

		if(opcode.f_isZero) {
			byte count = opcode.f_len+1;
			_log(NET__ZEROINFL, "    Opcode 0x%x (first part) yields %d zero bytes at %d", opcode, count, buffer.size());
			for(; count > 0; count--) {
				buffer.push_back(0);
			}
		} else {
			byte count = 8-opcode.f_len;
			_log(NET__ZEROINFL, "    Opcode 0x%x (first part) yields %d data bytes at %d", opcode, count, buffer.size());
			//caution: if we are at the end of the buffer, its possible to be processing a '0' opcode, which should mean "8 bytes", but really means "end"
			for(; count > 0 && in_buf < end; count--) {
				buffer.push_back(*in_buf++);
			}
		}

		if(opcode.s_isZero) {
			byte count = opcode.s_len+1;
			_log(NET__ZEROINFL, "    Opcode 0x%x (second part) yields %d zero bytes at %d", opcode, count, buffer.size());
			for(; count > 0; count--) {
				buffer.push_back(0);
			}
		} else {
			byte count = 8-opcode.s_len;
			_log(NET__ZEROINFL, "    Opcode 0x%x (second part) yields %d data bytes at %d", opcode, count, buffer.size());
			//caution: if we are at the end of the buffer, its possible to be processing a '0' opcode, which should mean "8 bytes", but really means "end"
			for(; count > 0 && in_buf < end; count--) {
				buffer.push_back(*in_buf++);
			}
		}
	}

	_log(NET__ZEROINFL, "  Zero-inflating buffer yields %d inflated bytes (ratio %.02f)",
		buffer.size(), double(in_length) / double(buffer.size())
	);
}

void PackZeroCompressed(const byte *in_buf, uint32 in_length, std::vector<byte> &out_buf) {
	out_buf.clear();
	if(in_buf == NULL || in_length == 0)
		return;

	_log(NET__ZEROCOMP, "Zero-compressing buffer of length %d",
		in_length
	);

    /*const byte *last_data_start = NULL;
	byte last_opcode = 0;
	//only one of these two is ever non-zero at a time:
	byte last_data_count = 0;
	byte last_zero_count = 0;
	
	const byte *end = in_buf + in_length;
	while(in_buf < end) {
		if(*in_buf == 0) {
			//we are starting with zero, hunting for non-zero

			//figure out how many zeros in a row we have, up to 8
			byte zero_count = 0;
			while(in_buf < end && *in_buf == 0 && zero_count < 8) {
				zero_count++;
				in_buf++;
			}

			byte this_opcode = 0x8 | (zero_count - 1);
			_log(NET__ZEROCOMP, "  Found %d zeros, opcode=0x%x", zero_count, this_opcode);
			
			if(last_data_count != 0 || last_zero_count != 0) {
				//we had something before this, now we have more data, write it
				byte opcode = (this_opcode << 4) | last_opcode;
				_log(NET__ZEROCOMP, "    Previous opcode 0x%x yields stream byte 0x%02x at %d", last_opcode, opcode, out_buf.size());
				out_buf.push_back(opcode);

				
				if(last_data_count != 0) {
					//we had previous data, push it out
					_log(NET__ZEROCOMP, "    Writing %d previous bytes at %d", last_data_count, out_buf.size());
					for(; last_data_count > 0; last_data_count--) {
						out_buf.push_back(*last_data_start);
						last_data_start++;
					}
					last_data_count = 0;	//just for clarity
				} else {
					//else, it was previous zeros, do nothing
					last_zero_count = 0;
				}

				//we do nothing to represent the current zeros 
			} else {
				//we had nothing pending before us, save it for next block.
				last_zero_count = zero_count;
				last_opcode = this_opcode;
			}
		} else {
			//we are starting with data, hunting for zero

			//figure out how many non-zero bytes we have in a row, up to 8
			byte data_count = 0;
			const byte *data_start = in_buf;
			while(in_buf < end && *in_buf != 0 && data_count < 8) {
				data_count++;
				in_buf++;
			}
			
			
			byte this_opcode = 8 - data_count;
			_log(NET__ZEROCOMP, "  Found %d data bytes, opcode=0x%x", data_count, this_opcode);
			
			if(last_data_count != 0 || last_zero_count != 0) {
				//we had something before this, now we have more data, write it
				byte opcode = (this_opcode << 4) | last_opcode;
				_log(NET__ZEROCOMP, "    Previous opcode 0x%x yields stream byte 0x%02x at %d", last_opcode, opcode, out_buf.size());
				out_buf.push_back(opcode);
				
				if(last_data_count != 0) {
					//we had previous data, push it out
					_log(NET__ZEROCOMP, "    Writing %d previous bytes at %d", last_data_count, out_buf.size());
					for(; last_data_count > 0; last_data_count--) {
						out_buf.push_back(*last_data_start);
						last_data_start++;
					}
					last_data_count = 0;	//just for clarity
				} else {
					//else, it was previous zeros, do nothing
					last_zero_count = 0;
				}
				
                //now we push out the current data
				_log(NET__ZEROCOMP, "    Writing %d current bytes at %d", data_count, out_buf.size());
				for(; data_count > 0; data_count--) {
					out_buf.push_back(*data_start);
					data_start++;
				}
			} else {
				//we had nothing pending before us, save it for next block.
				last_data_count = data_count;
				last_data_start = data_start;
				last_opcode = this_opcode;
			}
		} //end "did not start with a 0"
	} //end input loop

	//if there were pending zeros, we truncate them
    //but pending data needs to be written.
	if(last_data_count != 0) {
		//this seems wrong to me, putting a 0 in the high nibble (should mean there are 8 more bytes of data)
		byte opcode = last_opcode;
		_log(NET__ZEROCOMP, "  Remnant data bytes produces opcode 0x%x at %d", opcode, out_buf.size());
		out_buf.push_back(opcode);
		_log(NET__ZEROCOMP, "    Writing %d remnant bytes at %d", last_data_count, out_buf.size());
		for(; last_data_count > 0; last_data_count--) {
			out_buf.push_back(*last_data_start);
			last_data_start++;
		}
	}*/

	const byte *end = in_buf + in_length;
	out_buf.reserve(in_length);

	while(in_buf < end) {
		// insert opcode
		out_buf.push_back(0);	// insert opcode placeholder
		ZCOpCode *opcode = (ZCOpCode *)&out_buf.back();	// obtain pointer to it

		// encode first part
		if(*in_buf == 0x00) {
			//we are starting with zero, hunting for non-zero
			opcode->f_isZero = true;
			opcode->f_len = 0;
			in_buf++;

			for(; in_buf < end && *in_buf == 0x00 && opcode->f_len < 8; opcode->f_len++)
				in_buf++;
		} else {
			//we are starting with data, hunting for zero
			opcode->f_isZero = false;
			opcode->f_len = 7;
			out_buf.push_back(*in_buf++);

			for(; in_buf < end && *in_buf != 0x00 && opcode->f_len > 0; opcode->f_len--)
				out_buf.push_back(*in_buf++);
		}

		if(in_buf >= end)
			break;

		// encode second part
		if(*in_buf == 0x00) {
			//we are starting with zero, hunting for non-zero
			opcode->s_isZero = true;
			opcode->s_len = 0;
			in_buf++;

			for(; in_buf < end && *in_buf == 0x00 && opcode->s_len < 8; opcode->s_len++)
				in_buf++;
		} else {
			//we are starting with data, hunting for zero
			opcode->s_isZero = false;
			opcode->s_len = 7;
			out_buf.push_back(*in_buf++);

			for(; in_buf < end && *in_buf != 0x00 && opcode->s_len > 0; opcode->s_len--)
				out_buf.push_back(*in_buf++);
		}
	}

	_log(NET__ZEROCOMP,
		"  Zero-compressing buffer resulted in %d bytes (ratio %.02f)",
		out_buf.size(), double(out_buf.size()) / double(in_length)
	);
}


















