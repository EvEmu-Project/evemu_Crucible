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

void UnpackZeroCompressed(const byte *in_buf, uint32 in_length, std::vector<byte> &buffer) {
	buffer.clear();
	if(in_length == 0)
		return;
	
	buffer.reserve(in_length*2);	//just a reasonable start

	_log(NET__ZEROINFL, "Zero-inflating buffer of length %d", in_length);
	
	byte leftover_opcode = 0;
	bool have_leftover = false;

	const byte *end = in_buf + in_length;
	while(have_leftover || in_buf < end) {
		byte opcode;
		if(have_leftover) {
			opcode = leftover_opcode;
			have_leftover = false;
			_log(NET__ZEROINFL, "  Processing left over opcode 0x%x", opcode);
		} else {
			opcode = *in_buf;
			in_buf++;
			leftover_opcode = opcode >> 4;
			have_leftover = true;
			opcode &= 0xF;
			_log(NET__ZEROINFL, "  Processing opcode 0x%x (leaving 0x%x)", opcode, leftover_opcode);
		}

		if(opcode < 8) {
			byte count = (8-opcode);
			_log(NET__ZEROINFL, "    Opcode 0x%x yields %d data bytes at %d", opcode, count, buffer.size());
			//caution: if we are at the end of the buffer, its possible to be processing a '0' opcode, which should mean "8 bytes", but really means "end"
			for(; count > 0 && in_buf < end; count--) {
				buffer.push_back(*in_buf);
				in_buf++;
			}
		} else {
			byte count = opcode - 8 + 1;
			_log(NET__ZEROINFL, "    Opcode 0x%x yields %d zero bytes at %d", opcode, count, buffer.size());
			for(; count > 0; count--) {
				buffer.push_back(0);
			}
		}
	}

	_log(NET__ZEROINFL, "  Zero-inflating buffer yields %d inflated bytes", buffer.size());
}

void PackZeroCompressed(const byte *in_buf, uint32 in_length, std::vector<byte> &out_buf) {
	out_buf.clear();
	if(in_length == 0)
		return;

	_log(NET__ZEROCOMP, "Zero-compressing buffer of length %d", in_length);

	out_buf.reserve(in_length);	//its unlikely that we will exceed this

    const byte *last_data_start = NULL;
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
	}

	_log(NET__ZEROCOMP, "  Zero-compressing buffer resulted in %d bytes", out_buf.size());
}



#ifdef ZC_TEST

int main() {
	byte input[] = {
        //0x5d, 0xf0, 0x3f, 0x06, 0x7a, 0x03, 0x7a, 0xf2, 0x78, 0x22, 0x78, 0x01
		0x7f, 0x06, 0x7a, 0x04, 0x4a, 0xb6, 0x01, 0x34, 0x0d
		//0x70, 0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xa9, 0x3f, 0x06, 0x7a, 0x01, 0x4a, 0xb6, 0x01, 0xd1, 0x2c
	};
	
	printf("Original: %d bytes\n", sizeof(input));
	uint32 r;
	for(r = 0; r < sizeof(input); r++) {
		printf("%02X ", input[r]);
	}
	printf("\n");

	std::vector<byte> unpacked;
	UnpackZeroCompressed(input, sizeof(input), unpacked);

	printf("Unpacked: %d bytes\n", unpacked.size());
	for(r = 0; r < unpacked.size(); r++) {
		printf("%02X ", unpacked[r]);
	}
	printf("\n");

	std::vector<byte> repacked;
	PackZeroCompressed(&unpacked[0], unpacked.size(), repacked);
	
	printf("Re-packed: %d bytes\n", repacked.size());
	for(r = 0; r < repacked.size(); r++) {
		if(repacked[r] != input[r])
			printf("(%02X) ", repacked[r]);
		else
			printf("%02X ", repacked[r]);
	}
	printf("\n");
}
#endif


















