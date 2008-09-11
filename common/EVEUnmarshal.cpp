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

#include <math.h>	//for pow()

#include "common.h"
#include "PyRep.h"
#include "logsys.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "../common/misc.h"
#include "EVEMarshalOpcodes.h"
#include "EVEZeroCompress.h"
#include <stdio.h>
#include <string>

using std::string;

class UnmarshalState {
public:
	UnmarshalState(uint8 save_count_in)
	: count_packedobj(0),
	  save_count(save_count_in) 
	{
	}

	uint32 count_packedobj;
	
	const uint8 save_count;
	vector<PyRep *> saves;	//we do not own the pointers in this list
};

static uint32 UnmarshalData(UnmarshalState *state, const byte *packet, uint32 len, PyRep *&res, const char *indent);
//static byte *UnpackZeroCompressedDup(const byte *in_buf, uint32 in_length, uint32 *unpacked_length);	//retuns ownership of bytes


//returns ownership
PyRep *InflateAndUnmarshal(const byte *body, uint32 body_len) {
	const byte *const orig_body = body;
	const uint32 orig_body_len = body_len;
	
	if(*body != SubStreamHeaderByte) {
		if(body_len > sizeof(uint32) && *((const uint32 *) body) == 0) {
			//winging it here...
			uint32 buflen = body_len*10;	//bullshit multiplier!
			uchar *buf = new uchar[buflen];
			body_len = InflatePacket(body+12, body_len-12, buf, buflen);
			if(body_len == 0) {
				delete[] buf;
				_log(NET__PRES_ERROR, "Failed to inflate special packet!");
				_log(NET__PRES_DEBUG, "Raw Hex Dump:");
				_hex(NET__PRES_DEBUG, body, body_len);
				return(NULL);
			} else {
				body = buf;
				_log(NET__UNMARSHAL_ERROR, "Special Inflated packet of len %d to length %d\n", orig_body_len, body_len);
			}
		} else {
			uint32 buflen = body_len*10;	//bullshit multiplier!
			uchar *buf = new uchar[buflen];
			body_len = InflatePacket(body, body_len, buf, buflen);
			if(body_len == 0) {
				delete[] buf;
				_log(NET__PRES_ERROR, "Failed to inflate packet!");
				_log(NET__PRES_DEBUG, "Raw Hex Dump:");
				_hex(NET__PRES_DEBUG, body, body_len);
				return(NULL);
			} else {
				body = buf;
				_log(NET__PRES_TRACE, "Inflated packet of len %d to length %d", orig_body_len, body_len);
			}
		}
	} //end inflation conditional

	const byte *post_inflate_body = body;
	
	_log(NET__PRES_RAW, "Raw Hex Dump (post-inflation):");
	phex(NET__PRES_RAW, body, body_len);

	//skip SubStreamHeaderByte
	body++;
	body_len--;

	byte save_count = *body;
	//no idea what the other three bytes are, this might be a uint32, but that would be stupid
	body += 4;
	body_len -= 4;

	UnmarshalState *state = new UnmarshalState(save_count);
	
	PyRep *rep;
	uint32 used_len = UnmarshalData(state, body, body_len, rep, "    ");
	if(rep == NULL) {
		_log(NET__PRES_ERROR, "Failed to unmarshal data!");
		if(post_inflate_body != orig_body)
			delete[] post_inflate_body;
		delete state;
		return(NULL);
	}

	//total shit:
    if(state->count_packedobj > 0) {
		uint32 index = 0;
		uint32 rlen = body_len-used_len;
		for(index = 0; index < state->count_packedobj; index++) {
			if(rlen < sizeof(uint32)) {
				_log(NET__UNMARSHAL_TRACE, "Insufficient length for hack 0x2d trailer %d/%d", index+1, state->count_packedobj);
			} else {
				uint32 val = *((const uint32 *) (body+used_len));
				
				_log(NET__UNMARSHAL_TRACE, "Hack 0x23/Obj2 value[%d]: 0x%lx", index, val);
				
				used_len += sizeof(uint32);
				rlen -= sizeof(uint32);
			}
		}
    }

	delete state;
	
	if(used_len != body_len) {
		_log(NET__UNMARSHAL_TRACE, "Unmarshal did not consume entire data: %d/%d used", used_len, body_len);
		_hex(NET__UNMARSHAL_TRACE, body+used_len, body_len-used_len);
	}
	
	if(post_inflate_body != orig_body)
		delete[] post_inflate_body;
	return(rep);
}



static uint32 UnmarshalData(UnmarshalState *state, const byte *packet, uint32 len, PyRep *&res, const char *pfx) {
	res = NULL;
	uint32 len_used = 0;
	if(len < 1) {
		_log(NET__PRES_ERROR, "Empty packet received by UnmarshalData\n");
		return(len_used);
	}
	
	byte raw_opcode = *packet;
	packet++;
	len--;
	len_used++;
	
	if(raw_opcode & PyRepSaveMask) {
		_log(NET__UNMARSHAL_TRACE, "Raw opcode 0x%x has PyRepSaveMask set", raw_opcode);
	}
	if(raw_opcode & PyRepUnknownMask) {
		_log(NET__UNMARSHAL_TRACE, "Raw opcode 0x%x has PyRepUnknownMask set", raw_opcode);
	}
	
	byte opcode = raw_opcode & PyRepOpcodeMask;
	
//	_log(NET__UNMARSHAL_TRACE, "%sOpcode 0x%02x", pfx, opcode);
	
	switch(opcode) {
	case Op_PyNone: {
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyNone", pfx, opcode);
		res = new PyRepNone();
		break; }
	
	case Op_PyByteString: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for string length (missing length and data)\n");
			break;
		}
		byte str_len = *packet;
		packet++;
		len--;
		len_used++;
	
		if(len < str_len) {
			_log(NET__UNMARSHAL_ERROR, "Ran out of data in string of length %d, %d bytes remain.\n", str_len, len);
			break;
		}

		PyRepString *r = new PyRepString(packet, str_len, true);
		
		if(ContainsNonPrintables(r->value.c_str())) {
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString(len=%d, <binary>)", pfx, opcode, str_len);
			_log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
			phex(NET__UNMARSHAL_BUFHEX, packet, str_len);
		} else
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString(len=%d, \"%s\")", pfx, opcode, str_len, r->value.c_str());

		res = r;
		
		len_used += str_len;
		packet += str_len;
		len -= str_len;
		
		break; }
	
	case Op_PyLongLong: {
		if(len < 8) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for long long argument\n");
			break;
		}
		uint64 data = *((const uint64 *) packet);
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyLongLong %llu", pfx, opcode, data);
		
		res = new PyRepInteger(data);
		
		packet += sizeof(uint64);
		len -= sizeof(uint64);
		len_used += sizeof(uint64);
		
		break; }
	
	case Op_PyLong: {
	
		if(len < sizeof(uint32)) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for long arg\n");
			break;
		}
		uint32 value = *((const uint32 *) packet);
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyLong %lu", pfx, opcode, value);
		
		res = new PyRepInteger(value);
		
		packet += sizeof(value);
		len -= sizeof(value);
		len_used += sizeof(value);
		
		break; }
	
	case Op_PySignedShort: {
	
		if(len < sizeof(uint16)) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for short arg\n");
			break;
		}
		sint16 value = *((const sint16 *) packet);
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyShort %d", pfx, opcode, value);
		
		res = new PyRepInteger(value);
		
		packet += sizeof(value);
		len -= sizeof(value);
		len_used += sizeof(value);
		
		break; }
	
	case Op_PyByte: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for byte integer arg\n");
			break;
		}
		byte value = *packet;
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByte %u", pfx, opcode, value);
		
		res = new PyRepInteger(value);
		
		packet++;
		len--;
		len_used++;
		
		break; }
	
	case Op_PyInfinite:
		{ 
			//.text:1005D11D loc_1005D11D:
			//this looks like it is a 0-length opcode based on the asm.
			// 
			// might want to check out the agentMgr.GetAgents cached object for this one...
			
			//TODO: I think this can be used for floats and ints... further, I think
			//we need a better internal representation of this...
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyInfinite", pfx, opcode);
			res = new PyRepInteger(INT_MAX);
		break; }
	
	case Op_PyZeroInteger: {
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyZeroInteger", pfx, opcode);
		res = new PyRepInteger(0);
		break; }
	
	case Op_PyOneInteger: {
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyOneInteger", pfx, opcode);
		res = new PyRepInteger(1);
		break; }
	
	case Op_PyReal: {
	
		if(len < sizeof(double)) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for double arg\n");
			break;
		}
		double value = *((const double *) packet);
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyReal %.13f", pfx, opcode, value);
	
		res = new PyRepReal(value);
		
		packet += sizeof(value);
		len -= sizeof(value);
		len_used += sizeof(value);
		
		break; }
	
	case Op_PyZeroReal:
		{
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyZeroReal", pfx, opcode);
			res = new PyRepReal(0.0);
		break; }

	//0xC?
	
	case Op_PyBuffer: {
		
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for data buffer's length byte\n");
			break;
		}
		uint32 data_length = *packet;
		packet++;
		len--;
		len_used++;
	
		if(data_length == 0xFF) {
			//extended length data buffer
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for data buffer length 4-byte element\n");
				break;
			}
			data_length = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
	
		if(len < data_length) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for data buffer's data, %d needed but %d remain\n", data_length, len);
			break;
		}
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyBuffer length %d", pfx, opcode, data_length);
		_log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
		phex(NET__UNMARSHAL_BUFHEX, packet, data_length);
		
		res = new PyRepBuffer(packet, data_length);
		
		packet += data_length;
		len -= data_length;
		len_used += data_length;
		
		break; }
	
	case Op_PyEmptyString: {
		//.text:1005D1EF loc_1005D1EF
		// 
		//not 100% sure about this one.
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyString", pfx, opcode);
		res = new PyRepString("");
		break; }
	
	case Op_PyCharString: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for byte data\n");
			break;
		}
		char value[2];
		value[0] = *packet;
		value[1] = '\0';
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyCharString %c", pfx, opcode, value[0]);

		res = new PyRepString(value);
		
		packet++;
		len--;
		len_used++;
		
		break; }
	
	case Op_PyByteString2: {	//SHORT_BINSTRING
		
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for string length (missing length and data)\n");
			break;
		}
		uint32 str_len = *packet;
		packet++;
		len--;
		len_used++;
		
		if(str_len == 0xFF) {
			//extended length
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for string length 4-byte element");
				break;
			}
			str_len = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		if(len < str_len) {
			_log(NET__UNMARSHAL_ERROR, "Ran out of data in string of length %d, %d bytes remain.\n", str_len, len);
			break;
		}
		
		PyRepString *r = new PyRepString(packet, str_len, false);
		res = r;

		if(ContainsNonPrintables(r->value.c_str())) {
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString2(len=%d, <binary>)", pfx, opcode, str_len);
			_log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
			phex(NET__UNMARSHAL_BUFHEX, packet, str_len);
		} else
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString2(len=%d, \"%s\")", pfx, opcode, str_len, r->value.c_str());
		
		packet += str_len;
		len -= str_len;
		len_used += str_len;
		
		break; }
	
	case Op_PyStringTableItem: {	//an item from the string table
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for String Table Item argument\n");
			break;
		}
		byte value = *packet;

		const char *v = PyRepString::GetStringTable()->LookupIndex(value);
		if(v == NULL) {
			_log(NET__UNMARSHAL_ERROR, "String Table Item %d is out of range!\n", value);
			char ebuf[64];
			snprintf(ebuf, 64, "Invalid String Table Item %d", value);
			res = new PyRepString(ebuf);
		} else {
			res = new PyRepString(v);
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyStringTableItem(%d) = %s", pfx, opcode, value, v);
		}
		
		packet++;
		len--;
		len_used++;
		
		break; }
	
	case Op_PyUnicodeByteString: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for wchar_t string argument\n");
			break;
		}
		byte value = *packet;
		
		packet++;
		len--;
		len_used++;

		//prolly supports length extension...

		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeByteString of len %d*2", pfx, opcode, value);
		_log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
		phex(NET__UNMARSHAL_BUFHEX, packet, value*2);
		
		res = new PyRepString(packet, value*2, false);
		
		packet += value*2;
		len -= value*2;
		len_used += value*2;
		
		break; }
	
	case 0x13:
		{ _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
			phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
	/*
	.text:1005D3E8 loc_1005D3E8:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D3E8                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D3E8                 push    esi             ; case 0x13
	.text:1005D3E9                 mov     ecx, edi
	.text:1005D3EB                 call    sub_10058D60
	PyString_FromStringAndSize
	.text:1005D3F0                 jmp     loc_1005D935
	.text:1005D3F5 ; ---------------------------------------------------------------------------
	.text:1005D3F5
	*/
		break; }

	/*                                                                              
    *
    *   Variable length tuple
    *
    */
	case Op_PyTuple: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for tuple length (missing length and data)\n");
			break;
		}
		uint32 data_len = *packet;
		packet++;
		len--;
		len_used++;
		
		if(data_len == 0xFF) {
			//extended length data buffer
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for tuple length 4-byte element\n");
				break;
			}
			data_len = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}

		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyTuple(%d)", pfx, opcode, data_len);
		
		PyRepTuple *tuple = new PyRepTuple(data_len);
		res = tuple;

		char t[15];
		
		uint32 r;
		for(r = 0; r < data_len; r++) {
			std::string n(pfx);
			snprintf(t, 14, "  [%2ld] ", r);
			n += t;
			uint32 field_len = UnmarshalData(state, packet, len, tuple->items[r], n.c_str());
			if(tuple->items[r] == NULL) {
				delete tuple;
				res = NULL;
				break;
			}
			packet += field_len;
			len -= field_len;
			len_used += field_len;
		}
		
		break; }
	
	/*                                                                              
    *
    *   Variable length list
    *
    */
	case Op_PyList: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for list length (missing length and data)\n");
			break;
		}
		uint32 data_len = *packet;
		packet++;
		len--;
		len_used++;
		
		if(data_len == 0xFF) {
			//extended length data buffer
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for list length 4-byte element\n");
				break;
			}
			data_len = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyList(%d)", pfx, opcode, data_len);
		
		PyRepList *rep = new PyRepList();
		res = rep;
	
		char t[15];
		uint32 r;
		for(r = 0; r < data_len; r++) {
			std::string n(pfx);
			snprintf(t, 14, "  [%2ld] ", r);
			n += t;
			PyRep *rep2;
			uint32 field_len = UnmarshalData(state, packet, len, rep2, n.c_str());
			if(rep2 == NULL) {
				delete rep;
				res = NULL;
				break;
			}
			rep->items.push_back(rep2);
			packet += field_len;
			len -= field_len;
			len_used += field_len;
		}
		
		break; }
	
	/*                                                                              
    *
    *   Dictionary                                                                 
    *
    */
	case Op_PyDict: {
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for element count in new dict\n");
			break;
		}
		uint32 data_len = *packet;
		packet++;
		len--;
		len_used++;
		
		if(data_len == 0xFF) {
			//extended length data buffer
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for dict length 4-byte element\n");
				break;
			}
			data_len = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyDict(%d)", pfx, opcode, data_len);

		//total hack until we understand this 0x55 thing better
		//this seems to only happen if the first byte adter 0x7e(~) is 1
        /*if(*packet == 0x55) {
			packet++;
			len--;
			len_used++;
			
			if(len < 1) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for hack 0x55\n");
				break;
			}
			byte unknown = *packet;
			packet++;
			len--;
			len_used++;
	
			_log(NET__UNMARSHAL_TRACE, "%s  Detected 0x55 save marker with arg 0x%x", pfx, unknown);

			//not sure how to get this number yet...
			data_len = 0xa;
		}*/
		
		
		PyRepDict *rep = new PyRepDict();
		res = rep;
		
		char t[17];
		uint32 r;
		for(r = 0; r < data_len; r++) {
			PyRep *key;
			PyRep *value;
			
			std::string m(pfx);
			snprintf(t, 16, "  [%2ld] Value: ", r);
			m += t;
			uint32 field_len = UnmarshalData(state, packet, len, value, m.c_str());
			if(value == NULL) {
				delete rep;
				res = NULL;
				break;
			}
			packet += field_len;
			len -= field_len;
			len_used += field_len;
			
			std::string n(pfx);
			snprintf(t, 16, "  [%2ld] Key: ", r);
			n += t;
			field_len = UnmarshalData(state, packet, len, key, n.c_str());
			if(key == NULL) {
				delete rep;
				delete value;
				res = NULL;
				break;
			}
			packet += field_len;
			len -= field_len;
			len_used += field_len;

			rep->items[key] = value;
		}
		break; }
	
	/*                                                                              
    *
    *   "Object".. not sure on this one.
    *
    */
	case Op_PyObject: {
		PyRep *type;
		PyRep *arguments;
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyObject", pfx, opcode);
		
		std::string n(pfx);
		n += "  Type: ";
		
		uint32 type_len = UnmarshalData(state, packet, len, type, n.c_str());
		if(type == NULL)
			break;
		packet += type_len;
		len -= type_len;
		len_used += type_len;

		std::string typestr;
		if(!type->CheckType(PyRep::String)) {
			typestr = "NON-STRING-TYPE";
		} else {
			PyRepString *s = (PyRepString *) type;
			typestr = s->value;
		}
		delete type;
	
		if(len == 0) {
			_log(NET__UNMARSHAL_ERROR, "Ran out of length in dict entry for key!\n");
			break;
		}

		n = pfx;
		n += "  Args: ";
		uint32 value_len = UnmarshalData(state, packet, len, arguments, n.c_str());
		if(arguments == NULL)
			break;
		packet += value_len;
		len -= value_len;
		len_used += value_len;

		res = new PyRepObject(typestr, arguments);
		break; }
	
	case 0x18:
		{ _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
			phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
	/*
	.text:1005D774 loc_1005D774:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D774                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D774                 lea     eax, [esp+3Ch+str_data] ; case 0x18
	.text:1005D778                 push    eax
	.text:1005D779                 mov     ecx, esi
	.text:1005D77B                 call    sub_10058C50
	.text:1005D780                 test    al, al
	.text:1005D782                 jz      loc_1005DA4B
	.text:1005D788                 mov     edi, [esp+3Ch+str_data]
	.text:1005D78C                 push    edi
	.text:1005D78D                 lea     ecx, [esp+40h+str_data]
	.text:1005D791                 push    ecx
	.text:1005D792                 mov     ecx, esi
	.text:1005D794                 call    sub_10040A30
	.text:1005D799                 test    al, al
	.text:1005D79B                 jz      loc_1005DA4B
	.text:1005D7A1                 lea     ecx, [esp+3Ch+var_1C]
	.text:1005D7A5                 call    sub_100575A0
	.text:1005D7AA                 mov     edx, [esp+3Ch+str_data]
	.text:1005D7AE                 push    edi
	.text:1005D7AF                 push    edx
	.text:1005D7B0                 lea     ecx, [esp+44h+var_1C]
	.text:1005D7B4                 call    sub_10004D80
	.text:1005D7B9                 mov     eax, BeROT
	.text:1005D7BE                 lea     edx, [esp+3Ch+var_1C]
	.text:1005D7C2                 push    edx
	.text:1005D7C3                 xor     ebp, ebp
	.text:1005D7C5                 push    ebp
	.text:1005D7C6                 lea     edx, [esp+44h+var_28]
	.text:1005D7CA                 push    edx
	.text:1005D7CB                 mov     [esp+48h+var_28], ebp
	.text:1005D7CF                 mov     ecx, [eax]
	.text:1005D7D1                 push    eax
	.text:1005D7D2                 call    dword ptr [ecx+5Ch]
	.text:1005D7D5                 test    al, al
	.text:1005D7D7                 jz      short loc_1005D819
	.text:1005D7D9                 mov     edx, [esp+4Ch+var_38]
	.text:1005D7DD                 mov     eax, PyOS
	.text:1005D7E2                 mov     ecx, [eax]
	.text:1005D7E4                 push    0FFFFFFFFh
	.text:1005D7E6                 push    edx
	.text:1005D7E7                 push    eax
	.text:1005D7E8                 call    dword ptr [ecx+14h]
	.text:1005D7EB                 mov     edi, eax
	.text:1005D7ED                 mov     eax, dword ptr [esp+58h+var_44]
	.text:1005D7F1                 mov     ecx, [eax]
	.text:1005D7F3                 push    eax
	.text:1005D7F4                 mov     [esp+5Ch+var_18], edi
	.text:1005D7F8                 call    dword ptr [ecx+0Ch]
	.text:1005D7FB                 cmp     edi, ebp
	.text:1005D7FD                 jz      short loc_1005D810
	.text:1005D7FF                 test    bl, 40h
	.text:1005D802                 jz      short loc_1005D82E
	.text:1005D804                 push    edi
	.text:1005D805                 mov     ecx, esi
	.text:1005D807                 call    sub_10056110
	.text:1005D80C                 test    al, al
	.text:1005D80E                 jnz     short loc_1005D82E
	.text:1005D810
	.text:1005D810 loc_1005D810:                           ; CODE XREF: Marshal_Something+88Dj
	.text:1005D810                 lea     ecx, [esp+5Ch+var_1C]
	.text:1005D814                 call    sub_100057D0
	.text:1005D819
	.text:1005D819 loc_1005D819:                           ; CODE XREF: Marshal_Something+867j
	.text:1005D819                 lea     ecx, [esp+5Ch+var_3C]
	.text:1005D81D                 call    sub_10004D60
	.text:1005D822                 pop     edi
	.text:1005D823                 pop     esi
	.text:1005D824                 pop     ebp
	.text:1005D825                 xor     eax, eax
	.text:1005D827                 pop     ebx
	.text:1005D828                 add     esp, 2Ch
	.text:1005D82B                 retn    4
	.text:1005D82E ; ---------------------------------------------------------------------------
	.text:1005D82E
	.text:1005D82E loc_1005D82E:                           ; CODE XREF: Marshal_Something+892j
	.text:1005D82E                                         ; Marshal_Something+89Ej
	.text:1005D82E                 lea     ecx, [esp+5Ch+var_1C]
	.text:1005D832                 mov     [esp+5Ch+var_1C], ebp
	.text:1005D836                 call    sub_100057D0
	.text:1005D83B                 lea     ecx, [esp+5Ch+var_3C]
	.text:1005D83F                 call    sub_10004D60
	.text:1005D844                 mov     eax, edi
	.text:1005D846                 pop     edi
	.text:1005D847                 pop     esi
	.text:1005D848                 pop     ebp
	.text:1005D849                 pop     ebx
	.text:1005D84A                 add     esp, 2Ch
	.text:1005D84D                 retn    4
	.text:1005D850 ; ---------------------------------------------------------------------------
	.text:1005D850
	*/
		break; }
	
	case Op_PySubStruct: {

        // This is really a remote object specification
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PySubStruct", pfx, opcode);
		
		std::string n(pfx);
		n += "  ";
	
		PyRep *ss;
		uint32 value_len = UnmarshalData(state, packet, len, ss, n.c_str());
		if(ss == NULL)
			break;
		packet += value_len;
		len -= value_len;
		len_used += value_len;

		res = new PyRepSubStruct(ss);
		
		break; }
	
	case 0x1A:
		{ _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
			phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
	/*
	.text:1005D8D7 loc_1005D8D7:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D8D7                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D8D7                 lea     ecx, [esp+3Ch+str_data] ; case 0x1A
	.text:1005D8DB                 push    ecx
	.text:1005D8DC                 mov     ecx, esi
	.text:1005D8DE                 call    sub_10058C50
	.text:1005D8E3                 test    al, al
	.text:1005D8E5                 jz      loc_1005DA4B
	.text:1005D8EB                 push    edi
	.text:1005D8EC                 mov     ecx, esi
	.text:1005D8EE                 call    sub_100574D0
	.text:1005D8F3                 test    eax, eax
	.text:1005D8F5                 jz      loc_1005DA4B
	.text:1005D8FB                 push    0
	.text:1005D8FD                 push    offset aLoad    ; "load"
	.text:1005D902                 push    eax
	.text:1005D903                 call    PyObject_CallMethod
	.text:1005D908                 add     esp, 0Ch
	.text:1005D90B                 pop     edi
	.text:1005D90C                 pop     esi
	.text:1005D90D                 pop     ebp
	.text:1005D90E                 pop     ebx
	.text:1005D90F                 add     esp, 2Ch
	.text:1005D912                 retn    4
	.text:1005D915 ; ---------------------------------------------------------------------------
	.text:1005D915
	*/
		break; }
	
	case Op_PySavedStreamElement:
		{ 
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for saved reference index\n");
			break;
		}
		byte save_index = *packet;
		packet++;
		len--;
		len_used++;

		if(save_index > state->save_count) {
			_log(NET__UNMARSHAL_ERROR, "Save index %d is higher than the save count %d for this stream\n", save_index, state->save_count);
			break;
		} else if(save_index == 0) {
			_log(NET__UNMARSHAL_ERROR, "Save index 0 is invalid");
			break;
		}

		save_index--;

		if(save_index >= state->saves.size()) {
			_log(NET__UNMARSHAL_ERROR, "Save index %d is higher than the number of saved elements %d for this stream so far\n", save_index+1, state->saves.size());
			break;
		}

		//.text:1005D982
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PySavedStreamElement with index 0x%x", pfx, opcode, save_index+1);

		res = state->saves[save_index]->Clone();
		
		break; }
	
	case Op_PyChecksumedStream: {
		if(len < sizeof(uint32)) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for checksum argument\n");
			break;
		}
		uint32 lval = *((const uint32 *) packet);
		
		packet += sizeof(uint32);
		len -= sizeof(uint32);
		len_used += sizeof(uint32);
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyChecksumedStream w/ sum %lu", pfx, opcode, lval);
		
		std::string n(pfx);
		n += "  ";
		
		PyRep *stream;
		uint32 sslen = UnmarshalData(state, packet, len, stream, n.c_str());
		if(stream == NULL)
			break;
		
		packet += sslen;
		len -= sslen;
		len_used += sslen;

		res = new PyRepChecksumedStream(lval, stream);
		
		break; }
	
	case Op_PyTrue: {	//_Py_TrueStruct (which is Py_True)
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyTrue", pfx, opcode);
		res = new PyRepBoolean(true);
		break; }
	
	case Op_PyFalse: {	//_Py_ZeroStruct (which is Py_False)
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyFalse", pfx, opcode);
		res = new PyRepBoolean(false);
		break; }
	
	case 0x21:
		{ _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
			phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
	/*
	.text:1005D915 loc_1005D915:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D915                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D915                 push    edi             ; case 0x21
	.text:1005D916                 mov     ecx, esi
	.text:1005D918                 call    sub_10057460
	.text:1005D91D                 test    eax, eax
	.text:1005D91F                 jz      loc_1005DA4B
	.text:1005D925                 push    0
	.text:1005D927                 push    offset aLoad    ; "load"
	.text:1005D92C                 push    eax
	.text:1005D92D                 call    PyObject_CallMethod
	.text:1005D932                 add     esp, 0Ch
	.text:1005D935
	.text:1005D935 loc_1005D935:                           ; CODE XREF: Marshal_Something+480j
	.text:1005D935                 mov     edi, eax
	.text:1005D937                 test    edi, edi
	.text:1005D939                 mov     [esp+3Ch+str_data], edi
	.text:1005D93D                 jz      short loc_1005D950
	.text:1005D93F                 test    bl, 40h
	.text:1005D942                 jz      short loc_1005D965
	.text:1005D944                 mov     ecx, esi
	.text:1005D946                 push    edi
	.text:1005D947                 call    sub_10056110
	.text:1005D94C                 test    al, al
	.text:1005D94E                 jnz     short loc_1005D965
	.text:1005D950
	.text:1005D950 loc_1005D950:                           ; CODE XREF: Marshal_Something+4ABj
	.text:1005D950                                         ; Marshal_Something+4C0j ...
	.text:1005D950                 lea     ecx, [esp+3Ch+str_data]
	.text:1005D954
	.text:1005D954 loc_1005D954:                           ; CODE XREF: Marshal_Something+4D4j
	.text:1005D954                                         ; Marshal_Something+544j
	.text:1005D954                 call    sub_100057D0
	.text:1005D959                 pop     edi
	.text:1005D95A                 pop     esi
	.text:1005D95B                 pop     ebp
	.text:1005D95C                 xor     eax, eax
	.text:1005D95E                 pop     ebx
	.text:1005D95F                 add     esp, 2Ch
	.text:1005D962                 retn    4
	.text:1005D965 ; ---------------------------------------------------------------------------
	.text:1005D965
	.text:1005D965 loc_1005D965:                           ; CODE XREF: Marshal_Something+9D2j
	.text:1005D965                                         ; Marshal_Something+9DEj
	.text:1005D965                 lea     ecx, [esp+3Ch+str_data]
	.text:1005D969                 mov     [esp+3Ch+str_data], 0
	.text:1005D971                 call    sub_100057D0
	.text:1005D976                 mov     eax, edi
	.text:1005D978                 pop     edi
	.text:1005D979                 pop     esi
	.text:1005D97A                 pop     ebp
	.text:1005D97B                 pop     ebx
	.text:1005D97C                 add     esp, 2Ch
	.text:1005D97F                 retn    4
	.text:1005D982 ; ---------------------------------------------------------------------------
	.text:1005D982
	*/
		break; }
	
	case Op_PackedRowHeader:
		{
		
		PyRep *type;
		PyRep *arguments;
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PackedRowHeader", pfx, opcode);
		
		std::string n(pfx);
		n += "  RowHeader: ";
		
		uint32 key_len = UnmarshalData(state, packet, len, type, n.c_str());
		if(type == NULL)
			break;
		packet += key_len;
		len -= key_len;
		len_used += key_len;

		//this is such crap...
		if(!type->CheckType(PyRep::Tuple)) {
			_log(NET__UNMARSHAL_ERROR, "Non-tuple body in PackedRowHeader: %s", type->TypeString());
			delete type;
			break;
		}

		PyRepTuple *t = (PyRepTuple *) type;
		if(t->items.size() != 2) {
			_log(NET__UNMARSHAL_ERROR, "Non-two length on PackedRowHeader tuple: %d", t->items.size());
			delete type;
			break;
		}
		type = t->items[0];
		arguments = t->items[1];
		
		PyRepPackedRowHeader *rh = new PyRepPackedRowHeader(PyRepPackedRowHeader::RowList, type, arguments);
		
		
		//ok, we have decoded the tuple now. Time for the data loop...
		n = pfx;
		n += " PackedData: ";
		int empty_count = 0;
		bool seen_data = false;
		while(len > 0 && empty_count < 2) {
			PyRep *stream;
			
			uint32 sslen = UnmarshalData(state, packet, len, stream, n.c_str());
			if(stream == NULL)
				break;
			packet += sslen;
			len -= sslen;
			len_used += sslen;
			if(stream->CheckType(PyRep::None)) {	//really an 0x2d
				empty_count++;
				delete stream;
				continue;
			}
			if(!stream->CheckType(PyRep::PackedRow)) {
				_log(NET__UNMARSHAL_ERROR, "Unhandled type %s in PackedRowHeader", stream->TypeString());
				delete stream;
				continue;	//we are screwed anyways...
			}
			//else, this is some useful data...
			
			if(seen_data) {
				if(empty_count == 1 && rh->format == PyRepPackedRowHeader::RowList) {
					_log(NET__UNMARSHAL_ERROR, "PackedRowHeader contains data in the RowList AND RowDict section. What does this even mean?", stream->TypeString());
				}
			} else {
				//set the format based on the first data seen.
				if(empty_count == 0)
					rh->format = PyRepPackedRowHeader::RowList;
				else
					rh->format = PyRepPackedRowHeader::RowDict;
				seen_data = true;
			}
			rh->rows.push_back((PyRepPackedRow *) stream);
		}

		res = rh;
		
            /*_log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
            _hex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);*/
	/*
	.text:1005D9DC loc_1005D9DC:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D9DC                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D9DC                 shr     ebx, 6          ; case 0x22
	.text:1005D9DF                 and     bl, 1
	.text:1005D9E2                 mov     ecx, edi
	.text:1005D9E4                 push    ebx
	.text:1005D9E5                 push    esi
	.text:1005D9E6                 call    sub_10057630
	"Shared object table overflow"
	PyTuple_GetItem
	PyDict_Update
	.text:1005D9EB                 pop     edi
	.text:1005D9EC pop esi
	.text:1005D9ED pop     ebp
	.text:1005D9EE                 pop ebx
	.text:1005D9EF add esp, 2Ch
	.text:1005D9F2 retn    4 .text:1005D9F5 ;
	---------------------------------------------------------------------------
	.text:1005D9F5
	*/
		break; }
	
	case Op_PackedResultSet: {
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PackedResultSet", pfx, opcode);
		
		std::string n(pfx);
		n += " Header: ";
		PyRep *header;
		uint32 e_len = UnmarshalData(state, packet, len, header, n.c_str());
		if(header == NULL)
			break;
		packet += e_len;
		len -= e_len;
		len_used += e_len;
		
		PyRepPackedResultSet *rs = new PyRepPackedResultSet(PyRepPackedResultSet::RowList, header);
		
		/*
         * It seems as though there is actually two types of payload here,
         * and this is what the two unmarshal sequences are all about. Each
         * sequence is terminated with an 0x2d
         *
         * If you do not encounter an 0x2d before data, then you are likely
         * dealing with a dbutil.RowList, and you will just have a stream of
         * PyPacked data.
         *
         * If you encounter an 0x2d first, you likely have a dbutil.RowDict,
         * in which case you will encounter sequences of data as a key value
         * followed by a PyPacked data entry.
         *
         *
        */
		
		n += " Rows: ";
		int empty_count = 0;
		bool seen_data = false;
		while(len > 0 && empty_count < 2) {
			PyRep *stream;
			
			uint32 sslen = UnmarshalData(state, packet, len, stream, n.c_str());
			if(stream == NULL)
				break;
			packet += sslen;
			len -= sslen;
			len_used += sslen;
			if(stream->CheckType(PyRep::None)) {	//really an 0x2d
				empty_count++;
				delete stream;
				continue;
			}
			if(!stream->CheckType(PyRep::PackedRow)) {
				_log(NET__UNMARSHAL_ERROR, "Unhandled type %s in hack 0x23", stream->TypeString());
				delete stream;
				continue;	//we are screwed anyways...
			}
			//else, this is some useful data...
			
			if(seen_data) {
				if(empty_count == 1 && rs->format == PyRepPackedResultSet::RowList) {
					_log(NET__UNMARSHAL_ERROR, "PackedRowHeader contains data in the RowList AND RowDict section. What does this even mean?", stream->TypeString());
				}
			} else {
				//set the format based on the first data seen.
				if(empty_count == 0)
					rs->format = PyRepPackedResultSet::RowList;
				else
					rs->format = PyRepPackedResultSet::RowDict;
				seen_data = true;
			}
			rs->rows.push_back((PyRepPackedRow *) stream);
		}

		res = rs;
//		_hex(NET__UNMARSHAL_TRACE, packet, len>32?32:len);
	/*
	.text:1005D9F5 loc_1005D9F5:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D9F5                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D9F5                 shr     ebx, 6          ; case 0x23
	.text:1005D9F8                 and     bl, 1
	.text:1005D9FB                 mov     ecx, edi
	.text:1005D9FD                 push    ebx
	.text:1005D9FE                 push    esi
	.text:1005D9FF                 call    sub_10057850
	"Shared object table overflow"
	PyTuple_GetItem
	...
	
	.text:1005DA04                 pop     edi
	.text:1005DA05                 pop     esi
	.text:1005DA06                 pop     ebp
	.text:1005DA07                 pop     ebx
	.text:1005DA08                 add     esp, 2Ch
	.text:1005DA0B                 retn    4
	.text:1005DA0E ; ---------------------------------------------------------------------------
	.text:1005DA0E
	*/
		break; }
	
	case Op_PyEmptyTuple: {
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyTuple", pfx, opcode);
		res = new PyRepTuple(0);
		break; }
	
	case Op_PyOneTuple: {
		//TUPLE1
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyOneTuple", pfx, opcode);
		std::string n(pfx);
		n += "  [0] ";

		PyRep *i;
		uint32 key_len = UnmarshalData(state, packet, len, i, n.c_str());
		if(i == NULL)
			break;
		packet += key_len;
		len -= key_len;
		len_used += key_len;

		PyRepTuple *tuple = new PyRepTuple(1);
		tuple->items[0] = i;
		res = tuple;
		
		break; }
	
	case Op_PyEmptyList: {
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyList", pfx, opcode);
		res = new PyRepList();
		break; }

	//single element list.
	case Op_PyOneList: {
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyOneList", pfx, opcode);
		std::string n(pfx);
		n += "  [0] ";
		
		PyRep *i;
		uint32 key_len = UnmarshalData(state, packet, len, i, n.c_str());
		if(i == NULL)
			break;
		packet += key_len;
		len -= key_len;
		len_used += key_len;
	
		PyRepList *l = new PyRepList();
		l->items.push_back(i);
		res = l;
		
		break; }

	//empty unicode string.
	case Op_PyEmptyUnicodeString: {	//'('
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyUnicodeString", pfx, opcode);
		res = new PyRepString("");
		break; }

	//single wchar_t unicode string
	case Op_PyUnicodeCharString: {	//')'
		if(len < sizeof(uint16)) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for unicode char arg\n");
			break;
		}
		uint16 lval = *((const uint16 *) packet);
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeCharString 0x%x", pfx, opcode, lval);
		
		res = new PyRepString((const byte *) &lval, sizeof(uint16), false);
		
		packet += sizeof(uint16);
		len -= sizeof(uint16);
		len_used += sizeof(uint16);
		
		break; }
	
	case Op_PyPackedRow:	//*
		{
        /*
         * As far as I can tell, this opcode is really a packed
         * form of blue.DBRow, which takes a row descriptor and a
         * data stream (the body here)
         *
         */
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyPackedRow", pfx, opcode);
			
#ifdef PACKED_ROW_HEADER_HACK
		/*
         * These first two bytes are really an entire object
         * in the form of an 0x1B (Op_PySavedStreamElement)
         * and its associated index. This index points back to
         * the blue.DBRowDescriptor for this DBRow...
         *
        */
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for packed u1\n");
			break;
		}
		byte u1 = *packet;
		packet++;
		len--;
		len_used++;
		if(u1 != Op_PySavedStreamElement) {
			_log(NET__UNMARSHAL_ERROR, "Unexpected u1 opcode (0x%x) in Op_PyPacked. This should have been a saved stream element", u1);
			break;
		}
		
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for packed type");
			break;
		}
		byte header_element = *packet;
		packet++;
		len--;
		len_used++;

        /* end "this should be a recursive unmarshal" hack */
#else
        /* screw efficiency for now, just unmarshal the damned thing. */
		
		std::string n(pfx);
		n += "  Header ";
		PyRep *header_element;
		uint32 header_len = UnmarshalData(state, packet, len, header_element, n.c_str());
		if(header_element == NULL) {
			break;
		}
		packet += header_len;
		len -= header_len;
		len_used += header_len;
#endif

		
		
		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for packed length (missing length and data)\n");
			break;
		}
		uint32 data_len = *packet;
		packet++;
		len--;
		len_used++;

		//assumed, not actually observed:
		if(data_len == 0xFF) {
			//extended length
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for packed length 4-byte element");
				break;
			}
			data_len = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		if(len < data_len) {
			_log(NET__UNMARSHAL_ERROR, "Ran out of data in packed of length %d, %d bytes remain.\n", data_len, len);
			break;
		}
		
		_log(NET__UNMARSHAL_BUFHEX, "%s  Packed Contents: len=%lu", pfx, data_len);
		phex(NET__UNMARSHAL_BUFHEX, packet, data_len);

		//TODO: we can theoretically calculate the unpacked length
        //based on the dbrowdescriptor for this packed data... and
        //we need this information to determine how many zeros to fill
        //the end of the buffer with, but im leaving that to somebody
        //else at a higher layer which understands the dbrowdesc.
		std::vector<byte> unpacked;
		UnpackZeroCompressed(packet, data_len, unpacked);
		packet += data_len;
		len -= data_len;
		len_used += data_len;
		
		_log(NET__UNMARSHAL_TRACE, "%s  Unpacked Contents: len=%lu", pfx, unpacked.size());
		phex(NET__UNMARSHAL_TRACE, &unpacked[0], unpacked.size());
		
#ifdef PACKED_ROW_HEADER_HACK
		do something...
#else
		res = new PyRepPackedRow(&unpacked[0], unpacked.size(), true, header_element);
#endif
	/*
	.text:1005DA0E loc_1005DA0E:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005DA0E                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005DA0E                 push    esi             ; case 0x2A
	.text:1005DA0F                 push    edi
	.text:1005DA10                 call    sub_100460E0
	Marshal_Something
	... Marshal_Something
	.text:1005DA15                 add     esp, 8
	.text:1005DA18                 pop     edi
	.text:1005DA19                 pop     esi
	.text:1005DA1A                 pop     ebp
	.text:1005DA1B                 pop     ebx
	.text:1005DA1C                 add     esp, 2Ch
	.text:1005DA1F                 retn    4
	.text:1005DA22 ; ---------------------------------------------------------------------------
	.text:1005DA22
	*/
		break; }
	
	case Op_PySubStream: {	//'+'
	
		if(len < 6) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for substream\n");
			break;
		}
	
		uint32 data_length = *packet;
		packet++;
		len--;
		len_used++;
			
		//special extended length substream
		if(data_length == 0xFF) {
			if(len < 10) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for a long substream\n");
				break;
			}
			
			data_length = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		if(*packet != SubStreamHeaderByte) {
			_log(NET__UNMARSHAL_ERROR, "Invalid header byte on substream data 0x%02x\n", *packet);
			break;
		}
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PySubStream of length %lu", pfx, opcode, data_length);

		res = new PyRepSubStream(packet, data_length);
		
		packet += data_length;
		len -= data_length;
		len_used += data_length;
		
		break; }

	//TUPLE2
	case Op_PyTwoTuple: {

		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyTwoTuple", pfx, opcode);
		std::string n(pfx);
		n += "  [0] ";
		
		PyRep *i;
		uint32 key_len = UnmarshalData(state, packet, len, i, n.c_str());
		if(i == NULL)
			break;
		packet += key_len;
		len -= key_len;
		len_used += key_len;

		n = pfx;
		n += "  [1] ";
		PyRep *j;
		uint32 val_len = UnmarshalData(state, packet, len, j, n.c_str());
		if(j == NULL) {
			delete i;
			break;
		}
		packet += val_len;
		len -= val_len;
		len_used += val_len;

		PyRepTuple *tuple = new PyRepTuple(2);
		tuple->items[0] = i;
		tuple->items[1] = j;
		res = tuple;
		
		break; }
	
	case Op_PackedTerminator:
		{

        /*
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_SomeUnkownMarker66 total hack", pfx,opcode);
        std::string n(pfx); n += " 0x2D ";
		
		PyRep *i;
		uint32 e_len = UnmarshalData(state, packet, len, i, n.c_str());
		if(i == NULL)
			break;
		packet += e_len;
		len -= e_len;
		len_used += e_len;*/
		
        /*_log(NET__UNMARSHAL_TRACE, "(0x%x)Op_SomeUnkownMarker2D total hack", opcode);

		state->count_0x2d++;
		
		PyRep *i;
		uint32 key_len = UnmarshalData(state, packet, len, i, pfx);
		if(i == NULL)
			break;
		packet += key_len;
		len -= key_len;
		len_used += key_len;
        res = i;*/

			
		//hacking... should be its own type, but the only other acceptable thing 
		// is an OP_Packed, so this works fine.
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PackedTerminator", pfx,opcode);
		res = new PyRepNone();

		
/* total crap... */
/*		if(len < 1) {
			_log(NET__UNMARSHAL_ERROR, "Not enough data for saved reference index\n");
			break;
		}
		byte save_index = *packet;
		packet++;
		len--;
		len_used++;

		//.text:1005D982
		
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_SomeUnkownMarker total hack 2", pfx, opcode, save_index+1);
		int r;
		PyRep *i;
		for(r = 0; r < 2; r++) {
			
			std::string n(pfx);
			n += " Key1: ";
			
			uint32 e_len = UnmarshalData(state, packet, len, i, n.c_str());
			if(i == NULL)
				break;
			packet += e_len;
			len -= e_len;
			len_used += e_len;
	
			n = pfx;
			n += " Value1: ";
			PyRep *i2;
			e_len = UnmarshalData(state, packet, len, i2, n.c_str());
			if(i2 == NULL)
				break;
			packet += e_len;
			len -= e_len;
			len_used += e_len;
		}
		res = i;*/
/*vicious hack trying to understand this..
  it seems like this opcode is frequently repeated, not sure what
  it means yet.
		_log(NET__UNMARSHAL_TRACE, "%s(0x%x) UNKNOWN", pfx, opcode);
		std::string n(pfx);
		n += "  -0-> ";
		
		PyRep *i;
		uint32 key_len = UnmarshalData(state, packet, len, i, n.c_str());
		if(i == NULL)
			break;
		packet += key_len;
		len -= key_len;
		len_used += key_len;

		
		n = pfx;
		n += "  -1-> ";
		PyRep *j;
		uint32 val_len = UnmarshalData(state, packet, len, j, n.c_str());
		if(j == NULL) {
			delete i;
			break;
		}
		packet += val_len;
		len -= val_len;
		len_used += val_len;

		
		n = pfx;
		n += "  -2-> ";
		PyRep *k;
		uint32 gg_len = UnmarshalData(state, packet, len, k, n.c_str());
		if(k == NULL) {
			delete i;
			delete j;
			break;
		}
		packet += gg_len;
		len -= gg_len;
		len_used += gg_len;

			
        res = new PyRepNone();
        */
//			_log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x (maybe code related)", opcode);
//          _hex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
	/*
	.text:1005D02E loc_1005D02E:                           ; CODE XREF: Marshal_Something+3Cj
	.text:1005D02E                                         ; DATA XREF: .text:off_1005DA58o
	.text:1005D02E                 mov     eax, [edi+6Ch]  ; case 0x2D
	.text:1005D031                 pop     edi
	.text:1005D032                 pop     esi
	.text:1005D033                 pop     ebp
	.text:1005D034                 pop     ebx
	.text:1005D035                 add     esp, 2Ch
	.text:1005D038                 retn    4
	.text:1005D03B ; ---------------------------------------------------------------------------
	.text:1005D03B
	*/
		break; }
	
	case Op_PyUnicodeString: {
	
		uint32 data_length = *packet;
		packet++;
		len--;
		len_used++;
			
		//special extended length substream
		if(data_length == 0xFF) {
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for a long unicode string\n");
				break;
			}
			
			data_length = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		if(len < data_length) {
			_log(NET__UNMARSHAL_ERROR, "Ran out of data in BINUNICODE string of length %d, %d bytes remain.\n", data_length, len);
			break;
		}
		len_used += data_length;

		PyRepString *r = new PyRepString(packet, data_length, false);
		
		if(ContainsNonPrintables(r->value.c_str())) {
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeString(len=%d, <binary>)", pfx, opcode, data_length);
			_log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
			phex(NET__UNMARSHAL_BUFHEX, packet, data_length);
		} else
			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeString(len=%d, \"%s\")", pfx, opcode, data_length, r->value.c_str());

		
		res = r;
		
		break; }

	
	case Op_PyVarInteger: {

        /* this is one of the stranger fields I have found, it seems to be a variable
         * length integer field (somewhat of a 'bigint' style data type), but it gets
         * used at times for integers which would fit into the other primitive data
         * types.... I would really like to see the criteria they use to determine
         * what gets marshales as what...
         */

		uint32 data_length = *packet;
		packet++;
		len--;
		len_used++;
			
		//special extended length (I assume this wont happen for integers..., at least I hope not..)
		if(data_length == 0xFF) {
			if(len < sizeof(uint32)) {
				_log(NET__UNMARSHAL_ERROR, "Not enough data for an 0x2F\n");
				break;
			}
			
			data_length = *((const uint32 *) packet);
			packet += sizeof(uint32);
			len -= sizeof(uint32);
			len_used += sizeof(uint32);
		}
		
		if(len < data_length) {
			_log(NET__UNMARSHAL_ERROR, "Ran out of data in 0x2F of length %d, %d bytes remain.\n", data_length, len);
			break;
		}
		len_used += data_length;

		if(data_length <= sizeof(uint64)) {
			uint64 intval = pow(double(256), double(data_length)) - 1;
			intval &= *((const uint64 *) packet);

			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyVarInteger(len=%d) = " I64u, pfx, opcode, data_length, intval);
			res = new PyRepInteger(intval);
		} else {
			//uint64 is not big enough
			//just pass it up to the application layer as a buffer...
			PyRepBuffer *r = new PyRepBuffer(packet, data_length);

			_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyVarInteger(len=%d)", pfx, opcode, data_length);
			_log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
			_hex(NET__UNMARSHAL_BUFHEX, packet, data_length);
			
			res = r;
		}
		
		break; }
	
	default:
		{ _log(NET__UNMARSHAL_ERROR, "Unhandled (default) field type 0x%x\n", opcode);
			phex(NET__UNMARSHAL_ERROR, packet-1, len>32?32:len);
		break;
	/*
	.text:1005DA36 loc_1005DA36:                           ; CODE XREF: Marshal_Something+36j
	.text:1005DA36                                         ; Marshal_Something+3Cj
	.text:1005DA36                                         ; DATA XREF: ...
	.text:1005DA36                 mov     ecx, PyExc_RuntimeError ; default
	.text:1005DA3C                 push    ebx
	.text:1005DA3D                 push    offset aInvalidTypeTag ; "Invalid type tag %d in stream."
	.text:1005DA42                 push    ecx
	.text:1005DA43                 call    PyErr_Format
	.text:1005DA48                 add     esp, 0Ch
	.text:1005DA4B
	.text:1005DA4B loc_1005DA4B:                           ; CODE XREF: Marshal_Something+55j
	.text:1005DA4B                                         ; Marshal_Something+108j ...
	.text:1005DA4B                 pop     edi
	.text:1005DA4C                 pop     esi
	.text:1005DA4D                 pop     ebp
	.text:1005DA4E                 xor     eax, eax
	.text:1005DA50                 pop     ebx
	.text:1005DA51                 add     esp, 2Ch
	.text:1005DA54                 retn    4
	.text:1005DA54 Marshal_Something endp
	*/
		}
		}	//end switch
	if(raw_opcode & PyRepSaveMask) {
		//save off this pointer. We do not need to clone it since this function is the only
		//thing with a pointer to it, and we know it will remain valid until we are done.
		state->saves.push_back(res);
	}
	
	return(len_used);
}

//retuns ownership of bytes
//this could be implemented a lot more efficiently, but I dont care right now
#ifdef NOT_USED_RIGHT_NOW
static byte *UnpackZeroCompressedDup(const byte *in_buf, uint32 in_length, uint32 *unpacked_length) {
	std::vector<byte> buffer;

	UnpackZeroCompressed(in_buf, in_length, buffer);

	*unpacked_length = buffer.size();
	byte *out_buffer = new byte[*unpacked_length];
	memcpy(out_buffer, &buffer[0], *unpacked_length);
	return(out_buffer);
}
#endif



















