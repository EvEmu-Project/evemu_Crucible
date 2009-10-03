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
	Author:		BloodyRabbit
*/

#include "EVECommonPCH.h"

#include "marshal/EVEZeroCompress.h"

// ZeroCompress OpCode
struct ZCOpCode {
	// first part
	uint8 f_len : 3;
	bool f_isZero : 1;

	// second part
	uint8 s_len : 3;
	bool s_isZero : 1;
};

void UnpackZeroCompressed(const uint8 *in_buf, uint32 in_length, std::vector<uint8> &buffer) {
	buffer.clear();
	if(in_buf == NULL || in_length == 0)
		return;
	
	_log(NET__ZEROINFL, "Zero-inflating buffer of length %u",
		in_length
	);
	
	ZCOpCode opcode;
	const uint8 *end = in_buf + in_length;

	while(in_buf < end) {
		opcode = *(ZCOpCode *)(in_buf++);

		if(opcode.f_isZero) {
			uint8 count = opcode.f_len+1;
			_log(NET__ZEROINFL, "    Opcode 0x%x (first part) yields %u zero bytes at %lu", opcode, count, buffer.size());
			for(; count > 0; count--) {
				buffer.push_back(0);
			}
		} else {
			uint8 count = 8-opcode.f_len;
			_log(NET__ZEROINFL, "    Opcode 0x%x (first part) yields %u data bytes at %lu", opcode, count, buffer.size());
			//caution: if we are at the end of the buffer, its possible to be processing a '0' opcode, which should mean "8 bytes", but really means "end"
			for(; count > 0 && in_buf < end; count--) {
				buffer.push_back(*in_buf++);
			}
		}

		if(opcode.s_isZero) {
			uint8 count = opcode.s_len+1;
			_log(NET__ZEROINFL, "    Opcode 0x%x (second part) yields %u zero bytes at %lu", opcode, count, buffer.size());
			for(; count > 0; count--) {
				buffer.push_back(0);
			}
		} else {
			uint8 count = 8-opcode.s_len;
			_log(NET__ZEROINFL, "    Opcode 0x%x (second part) yields %u data bytes at %lu", opcode, count, buffer.size());
			//caution: if we are at the end of the buffer, its possible to be processing a '0' opcode, which should mean "8 bytes", but really means "end"
			for(; count > 0 && in_buf < end; count--) {
				buffer.push_back(*in_buf++);
			}
		}
	}

	_log(NET__ZEROINFL, "  Zero-inflating buffer yields %lu inflated bytes (ratio %.02f)",
		buffer.size(), double(in_length) / double(buffer.size())
	);
}

void PackZeroCompressed(const uint8 *in_buf, uint32 in_length, std::vector<uint8> &out_buf) {
	out_buf.clear();
	if(in_buf == NULL || in_length == 0)
		return;

	_log(NET__ZEROCOMP, "Zero-compressing buffer of length %u",
		in_length
	);

	const uint8 *end = in_buf + in_length;

	while(in_buf < end) {
		// we need to have enough room without moving (otherwise
		// it would invalidate our pointer obtained below); size
		// is 1 byte of opcode + at most 2x 8 bytes
		out_buf.reserve(out_buf.size() + 1 + 16);

		// insert opcode
		out_buf.push_back(0);	// insert opcode placeholder
		ZCOpCode *opcode = (ZCOpCode *)&out_buf.back();	// obtain pointer to it

		// encode first part
		if(*in_buf == 0x00) {
			//we are starting with zero, hunting for non-zero
			opcode->f_isZero = true;
			opcode->f_len = 0;

			in_buf++;
			for(; in_buf < end && *in_buf == 0x00 && opcode->f_len < 7; opcode->f_len++)
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
			for(; in_buf < end && *in_buf == 0x00 && opcode->s_len < 7; opcode->s_len++)
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
		"  Zero-compressing buffer resulted in %lu bytes (ratio %.02f)",
		out_buf.size(), double(out_buf.size()) / double(in_length)
	);
}


















