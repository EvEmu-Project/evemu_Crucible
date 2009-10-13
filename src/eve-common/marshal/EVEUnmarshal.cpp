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
    Author:     BloodyRabbit, Captnoord, Zhur
*/

#include "EVECommonPCH.h"

#include "python/classes/DBRowDescriptor.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"

#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalReferenceMap.h"
#include "marshal/EVEMarshalStringTable.h"
#include "marshal/EVEZeroCompress.h"

#include "utils/EVEUtils.h"

static uint32 UnmarshalData(UnmarshalReferenceMap& state, const uint8* packet, uint32 len, PyRep** res, const char* pfx);

PyRep* Unmarshal(const uint8* data, uint32 len)
{
    if( data[0] != MarshalHeaderByte )
    {
        sLog.Error( "Unmarshal", "Invalid stream of length %u received (header byte 0x%X).", len, data[0] );
        return NULL;
    }

    //skip MarshalHeaderByte
    data++;
    len--;

    uint32 save_count = *(uint32 *)data;
    data += sizeof( uint32 );
    len -= sizeof( uint32 ) + save_count * sizeof( uint32 );

    UnmarshalReferenceMap state( save_count, (uint32 *)&data[ len ] );

    PyRep* rep;
    uint32 used_len = UnmarshalData( state, data, len, &rep, "    " );
    if( rep == NULL )
    {
        sLog.Error( "Unmarshal", "Failed to unmarshal stream of length %u.", len );
        return NULL;
    }

    if( used_len != len )
        sLog.Warning( "Unmarshal", "Only %u/%u bytes has been used.", used_len, len );

    return rep;
}

PyRep* InflateUnmarshal(const uint8* data, uint32 len)
{
    bool inflated = false;

    if( data[0] == DeflateHeaderByte )
    {
        // The stream is deflated
        data = InflateData( data, &len );
        if( data == NULL )
        {
            sLog.Error( "Unmarshal", "Failed to inflate stream of length %u.", len );
            return NULL;
        }
        // We have inflated the stream
        inflated = true;
    }

    PyRep* rep = Unmarshal( data, len );

    if( inflated )
        // data has been inflated, delete it
        SafeDeleteArray( data );

    return rep;
}

static uint32 UnmarshalData(UnmarshalReferenceMap& state, const uint8* packet, uint32 len, PyRep** res, const char* pfx)
{
/************************************************************************/
/*                                                                      */
/************************************************************************/

#define GetByType(x) \
    *(x *)packet; \
    IncreaseIndex( sizeof( x ) );

#define IncreaseIndex(count) \
    packet += count; \
    len -= count; \
    len_used += count;

#define Getint8() GetByType(int8)
#define Getint16() GetByType(int16)
#define Getint32() GetByType(int32)
#define Getint64() GetByType(int64)

#define Getuint8() GetByType(uint8)
#define Getuint16() GetByType(uint16)
#define Getuint32() GetByType(uint32)
#define Getuint64() GetByType(uint64)

#define Getchar() GetByType(char)
#define GetDouble() GetByType(double)

/************************************************************************/
/*                                                                      */
/************************************************************************/

    *res = NULL;
    uint32 len_used = 0;
    if(len == 0)
    {
        _log(NET__PRES_ERROR, "Empty packet received by UnmarshalData.");
        return len_used;
    }

    uint8 raw_opcode = Getuint8();

	uint32 index = 0;
    if( raw_opcode & PyRepSaveMask )
    {
        _log(NET__UNMARSHAL_TRACE, "Raw opcode 0x%x has PyRepSaveMask set", raw_opcode);

		index = state.ReserveObjectSpace();
    }

    if( raw_opcode & PyRepUnknownMask )
        _log(NET__UNMARSHAL_TRACE, "Raw opcode 0x%x has PyRepUnknownMask set", raw_opcode);

    uint8 opcode = raw_opcode & PyRepOpcodeMask;
    _log(NET__UNMARSHAL_TRACE, "%sOpcode 0x%02x", pfx, opcode);

    switch(opcode)
    {
    case Op_PyNone:
    {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyNone", pfx, opcode);

        *res = new PyNone();

        break; }

    case Op_PyByteString:
    {
        if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for string length (missing length and data)\n");
            break;
        }
        uint8 str_len = Getuint8();

        if(len < str_len) {
            _log(NET__UNMARSHAL_ERROR, "Ran out of data in string of length %d, %d bytes remain.\n", str_len, len);
            break;
        }

		PyString *r = new PyString( std::string( (const char *)packet, str_len ), true);

        if(ContainsNonPrintables( r )) {
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString(len=%d, <binary>)", pfx, opcode, str_len);
            _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
            phex(NET__UNMARSHAL_BUFHEX, packet, str_len);
        } else
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString(len=%d, \"%s\")", pfx, opcode, str_len, r->content().c_str());

		IncreaseIndex( str_len );

        *res = r;

        break; }

    case Op_PyLongLong: {
        if(len < sizeof(int64)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for long long argument\n");
            break;
        }
		int64 data = Getint64();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyLongLong "I64d, pfx, opcode, data);

        *res = new PyLong(data);

        break; }

    case Op_PyLong: {
        if(len < sizeof(int32)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for long arg\n");
            break;
        }
		uint32 value = Getint32();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyLong %u", pfx, opcode, value);

        *res = new PyInt(value);

        break; }

    case Op_PySignedShort: {
        if(len < sizeof(int16)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for short arg\n");
            break;
        }
        int16 value = Getint16();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyShort %d", pfx, opcode, value);

        *res = new PyInt(value);

        break; }

    case Op_PyByte: {
        if(len < sizeof(int8)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for byte integer arg\n");
            break;
        }
		int8 value = Getint8();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByte %u", pfx, opcode, value);

        *res = new PyInt(value);

        break; }

    case Op_PyMinusOne:
        {
            //.text:1005D11D loc_1005D11D:
            //this looks like it is a 0-length opcode based on the asm.
            //
            // might want to check out the agentMgr.GetAgents cached object for this one...

            //TODO: I think this can be used for floats and ints... further, I think
            //we need a better internal representation of this...
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyMinusOne", pfx, opcode);
            *res = new PyInt(INT_MAX);
        break; }

    case Op_PyZeroInteger: {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyZeroInteger", pfx, opcode);
        *res = new PyInt(0);
        break; }

    case Op_PyOneInteger: {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyOneInteger", pfx, opcode);
        *res = new PyInt(1);
        break; }

    case Op_PyReal: {
        if(len < sizeof(double)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for double arg\n");
            break;
        }
		double value = GetDouble();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyReal %.13f", pfx, opcode, value);

        *res = new PyFloat(value);

        break; }

    case Op_PyZeroReal:
        {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyZeroReal", pfx, opcode);
        *res = new PyFloat(0.0);
        break; }

    //0xC?

    case Op_PyBuffer: {

        if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for data buffer's length byte\n");
            break;
        }
		uint32 data_length = Getuint8();

        if(data_length == 0xFF) {
            //extended length data buffer
            if(len < sizeof(uint32)) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for data buffer length 4-byte element\n");
                break;
            }
			data_length = Getuint32();
        }

        if(len < data_length) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for data buffer's data, %d needed but %d remain\n", data_length, len);
            break;
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyBuffer length %d", pfx, opcode, data_length);
        _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
        phex(NET__UNMARSHAL_BUFHEX, packet, data_length);

        *res = new PyBuffer(packet, data_length);

        IncreaseIndex( data_length );

        break; }

    case Op_PyEmptyString: {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyString", pfx, opcode);
        *res = new PyString("");
        break; }

    case Op_PyCharString: {
        if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for byte data\n");
            break;
        }
        char value[2];
        value[0] = Getchar();
        value[1] = '\0';

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyCharString %c", pfx, opcode, value[0]);

        *res = new PyString(value);

        break; }

    case Op_PyShortString: {    //SHORT_BINSTRING

        if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for string length (missing length and data)\n");
            break;
        }

        uint8 str_len = Getuint8();
        if(len < str_len) {
            _log(NET__UNMARSHAL_ERROR, "Ran out of data in string of length %d, %d bytes remain.\n", str_len, len);
            break;
        }

		PyString* r = new PyString( std::string( (const char *)packet, str_len ), false);

        if(ContainsNonPrintables( r )) {
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString2(len=%u, <binary>)", pfx, opcode, str_len);
            _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
            phex(NET__UNMARSHAL_BUFHEX, packet, str_len);
        } else {
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyByteString2(len=%u, \"%s\")", pfx, opcode, str_len, r->content().c_str());
        }

        IncreaseIndex(str_len);

        *res = r;

        break; }

    case Op_PyStringTableItem: {    //an item from the string table
        if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for String Table Item argument\n");
            break;
        }
        uint8 value = Getuint8();

        const PyString* sharedString = sMarshalStringTable.LookupPyString( value );
        if( sharedString == NULL )
        {
            assert(false);
            sLog.Error("unmarshal", "String Table Item %d is out of range!\n", value);

            char ebuf[64];
            snprintf(ebuf, 64, "Invalid String Table Item %u", value);
            *res = new PyString( ebuf );
        }
        else
        {
            *res = new PyString( *sharedString );
        }

        break; }

    case Op_PyUnicodeByteString: {
        if(len < 1)
        {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for wchar_t string argument\n");
            break;
        }

        uint32 stringLength = Getuint8();

        if (stringLength == 0xFF)
        {
            if (len < 4)
            {
                //_log(NET__UNMARSHAL_ERROR, "Not enough data for wchar_t string argument\n");
                break;
            }
            stringLength = Getuint32();
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeByteString of len %d*2", pfx, opcode, stringLength);
        _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
        phex(NET__UNMARSHAL_BUFHEX, packet, stringLength * 2);

		*res = new PyString( std::string( (const char *)packet, stringLength * 2 ), false);

        IncreaseIndex(stringLength*2);
        break; }

    case Op_PyLongString: {
        uint32 strLen = Getuint8();
        if (strLen == 0xFF)
        {
            if (len < 4)
            {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for long string argument\n");
                break;
            }
            strLen = Getuint32();
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyString of len %u", pfx, opcode, strLen);
        _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
        phex(NET__UNMARSHAL_BUFHEX, packet, strLen);

		*res = new PyString( std::string( (const char *)packet, strLen ), false);

        IncreaseIndex(strLen);
        break; }

    /*
    *
    *   Variable length tuple
    *
    */
    case Op_PyTuple: {
        if(len < sizeof(uint8)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for tuple length (missing length and data)\n");
            break;
        }
        uint32 data_len = Getuint8();

        if(data_len == 0xFF) {
            //extended length data buffer
            if(len < sizeof(uint32)) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for tuple length 4-byte element\n");
                break;
            }
            data_len = Getuint32();
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyTuple(%d)", pfx, opcode, data_len);

        PyTuple* tuple = new PyTuple( data_len );

        char t[15];

        for(uint32 r = 0; r < data_len; r++) {
            std::string n(pfx);
            snprintf(t, 14, "  [%2u] ", r);
            n += t;

            PyRep* rep = NULL;
            uint32 field_len = UnmarshalData(state, packet, len, &rep, n.c_str());
            if(rep == NULL)
            {
                PyDecRef( tuple );
                break;
            }

            tuple->SetItem( r, rep );
            IncreaseIndex( field_len );
        }

        *res = tuple;
        break; }

    /*
    *
    *   Variable length list
    *
    */
    case Op_PyList: {
        if(len < sizeof(uint8)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for list length (missing length and data)\n");
            break;
        }
        uint32 data_len = Getuint8();

        if(data_len == 0xFF) {
            //extended length data buffer
            if(len < sizeof(uint32)) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for list length 4-byte element\n");
                break;
            }
            data_len = Getuint32();
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyList(%d)", pfx, opcode, data_len);

        PyList* list = new PyList( data_len );

        char t[15];
        for(uint32 r = 0; r < data_len; r++) {
            std::string n(pfx);
            snprintf(t, 14, "  [%2u] ", r);
            n += t;

            PyRep* rep;
            uint32 field_len = UnmarshalData(state, packet, len, &rep, n.c_str());
            if(rep == NULL)
            {
                PyDecRef( list );
                break;
            }

            list->SetItem( r, rep );
            IncreaseIndex( field_len );
        }

        *res = list;
        break; }

    /*
    *
    *   Dictionary
    *
    */
    case Op_PyDict: {
        if(len < sizeof(uint8)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for element count in new dict\n");
            break;
        }
        uint32 data_len = Getuint8();

        if(data_len == 0xFF) {
            //extended length data buffer
            if(len < sizeof(uint32)) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for dict length 4-byte element\n");
                break;
            }
            data_len = Getuint32();
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyDict(%d)", pfx, opcode, data_len);

        PyDict *dict = new PyDict();

        char t[17];

        for(uint32 r = 0; r < data_len; r++) {
            PyRep *key;
            PyRep *value;

            std::string m(pfx);
            snprintf(t, 16, "  [%2u] Value: ", r);
            m += t;

            uint32 field_len = UnmarshalData(state, packet, len, &value, m.c_str());
            if(value == NULL)
            {
                PyDecRef( dict );
                break;
            }

            IncreaseIndex( field_len );

            std::string n(pfx);
            snprintf(t, 16, "  [%2u] Key: ", r);
            n += t;

            field_len = UnmarshalData(state, packet, len, &key, n.c_str());
            if(key == NULL)
            {
                PyDecRef( value );

                PyDecRef( dict );
                break;
            }

            IncreaseIndex( field_len );

            dict->SetItem( key, value );
        }

        *res = dict;
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
        uint32 type_len = UnmarshalData(state, packet, len, &type, n.c_str());
        if(type == NULL)
            break;
		IncreaseIndex( type_len );

        if( !type->IsString() )
        {
            _log( NET__UNMARSHAL_ERROR, "Expected type string, got %s.", type->TypeString() );
            break;
        }

        n = pfx;
        n += "  Args: ";
        uint32 value_len = UnmarshalData(state, packet, len, &arguments, n.c_str());
        if(arguments == NULL)
            break;
		IncreaseIndex( value_len );

        *res = new PyObject( &type->AsString(), arguments );
        break; }

    case 0x18:
        { _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
            phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
        break; }

    case Op_PySubStruct: {

        // This is really a remote object specification

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PySubStruct", pfx, opcode);

        std::string n(pfx);
        n += "  ";

        PyRep *ss;
        uint32 value_len = UnmarshalData(state, packet, len, &ss, n.c_str());
        if(ss == NULL)
            break;
		IncreaseIndex( value_len );

        *res = new PySubStruct(ss);
        break; }

    case 0x1A:
        { _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
            phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
        break; }

    case Op_PySavedStreamElement:
        {
            if(len < 1)
            {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for saved reference index.");
                break;
            }
            uint32 reference_index = Getuint8();
            if( reference_index == 0xFF )
			{
				if( len < sizeof( uint32 ) )
				{
					_log( NET__UNMARSHAL_ERROR, "Not enough data for extended saved reference index." );
					break;
				}
                reference_index = Getuint32();
			}

            //_log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PySavedStreamElement with index 0x%x", pfx, opcode, reference_index);

			PyRep* obj = state.GetStoredObject( reference_index );
			if( obj == NULL )
			{
				_log( NET__UNMARSHAL_ERROR, "Failed to obtain stored object for index %u.", reference_index );
				break;
			}

            *res = obj->Clone();
        } break;

    case Op_PyChecksumedStream: {
        if(len < sizeof(uint32)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for checksum argument\n");
            break;
        }
		uint32 sum = Getuint32();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyChecksumedStream w/ sum %u", pfx, opcode, sum);

        std::string n(pfx);
        n += "  ";

        PyRep *stream;
        uint32 sslen = UnmarshalData(state, packet, len, &stream, n.c_str());
        if(stream == NULL)
            break;
		IncreaseIndex( sslen );

        *res = new PyChecksumedStream(stream, sum);
        break; }

    case Op_PyTrue: {   //_Py_TrueStruct (which is Py_True)
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyTrue", pfx, opcode);
        *res = new PyBool(true);
        break; }

    case Op_PyFalse: {  //_Py_ZeroStruct (which is Py_False)
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyFalse", pfx, opcode);
        *res = new PyBool(false);
        break; }

    case 0x21:
        { _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x\n", opcode);
            phex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
        break; }

    case Op_ObjectEx1:
    case Op_ObjectEx2:
        {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_ObjectEx", pfx, opcode);

        PyRep *header;

        std::string n(pfx);
        n += "    ";
        uint32 clen = UnmarshalData(state, packet, len, &header, n.c_str());
        if(header == NULL)
            break;
		IncreaseIndex( clen );

        PyObjectEx* obj = new PyObjectEx( ( opcode == Op_ObjectEx2 ), header );

        n = pfx;
        n += "  ListData: ";
        while( *packet != Op_PackedTerminator )
		{
            PyRep *el;
            clen = UnmarshalData(state, packet, len, &el, n.c_str());
            if(el == NULL)
                break;
			IncreaseIndex( clen );

            obj->list().AddItem( el );
        }
		IncreaseIndex( 1 );

        n = pfx;
        n += "  DictData: ";
        while( *packet != Op_PackedTerminator )
        {
            PyRep *key;
            PyRep *value;

            clen = UnmarshalData(state, packet, len, &key, n.c_str());
            if(key == NULL)
                break;
			IncreaseIndex( clen );

            clen = UnmarshalData(state, packet, len, &value, n.c_str());
            if(value == NULL)
                break;
			IncreaseIndex( clen );

			obj->dict().SetItem( key, value );
        }
		IncreaseIndex( 1 );

        *res = obj;
        break;
        }

    case Op_PyEmptyTuple: {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyTuple", pfx, opcode);
        *res = new PyTuple(0);
        break; }

    case Op_PyOneTuple: {
        //TUPLE1

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyOneTuple", pfx, opcode);
        std::string n(pfx);
        n += "  [0] ";

        PyRep *i;
        uint32 key_len = UnmarshalData(state, packet, len, &i, n.c_str());
        if(i == NULL)
            break;
        IncreaseIndex( key_len );

        PyTuple* tuple = new PyTuple(1);
        tuple->SetItem( 0, i );

        *res = tuple;
        break; }

    case Op_PyEmptyList: {
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyList", pfx, opcode);
        *res = new PyList();
        break; }

    //single element list.
    case Op_PyOneList: {

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyOneList", pfx, opcode);
        std::string n(pfx);
        n += "  [0] ";

        PyRep *i;
        uint32 key_len = UnmarshalData(state, packet, len, &i, n.c_str());
        if(i == NULL)
            break;
        IncreaseIndex( key_len );

        PyList* l = new PyList();
        l->AddItem( i );

        *res = l;
        break; }

    //empty unicode string.
    case Op_PyEmptyUnicodeString: { //'('
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyEmptyUnicodeString", pfx, opcode);
        *res = new PyString("");
        break; }

    //single wchar_t unicode string
    case Op_PyUnicodeCharString: {  //')'
        if(len < sizeof(uint16)) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for unicode char arg\n");
            break;
        }
		uint16 lval = Getuint16();

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeCharString 0x%x", pfx, opcode, lval);

		*res = new PyString( std::string( (const char *)&lval, sizeof(uint16) ), false);
        break; }

    case Op_PyPackedRow:    //*
        {
        /*
         * As far as I can tell, this opcode is really a packed
         * form of blue.DBRow, which takes a row descriptor and a
         * data stream (the work_buffer here)
         *
         */
        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyPackedRow", pfx, opcode);

        std::string n(pfx);
        n += "  Header ";
        PyRep *header_element;
        uint32 header_len = UnmarshalData(state, packet, len, &header_element, n.c_str());
        if(header_element == NULL)
            break;
        IncreaseIndex( header_len );

		// This is only an assumption, though PyPackedRow does not
		// support anything else ....
        PyPackedRow *row = new PyPackedRow( (DBRowDescriptor *)header_element );

        if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for packed length (missing length and data)\n");
            break;
        }
        uint32 data_len = Getuint8();

        //assumed, not actually observed:
        if(data_len == 0xFF) {
            //extended length
            if(len < sizeof(uint32)) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for packed length 4-byte element");
                break;
            }
            data_len = Getuint32();
        }

        if(len < data_len) {
            _log(NET__UNMARSHAL_ERROR, "Ran out of data in packed of length %d, %d bytes remain.\n", data_len, len);
            break;
        }

        _log(NET__UNMARSHAL_BUFHEX, "%s  Packed Contents: len=%u", pfx, data_len);
        phex(NET__UNMARSHAL_BUFHEX, packet, data_len);

        if( data_len > len )
        {
            _log( NET__UNMARSHAL_ERROR, "Not enough data to unmarshal packed row; need %u, got %u.", data_len, len );
            break;
        }

        std::vector<uint8> unpacked;
        UnpackZeroCompressed( packet, data_len, unpacked );
        IncreaseIndex( data_len );

        // Create size map, sorted from the greatest to the smallest value
        std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
        uint32 cc = row->header().ColumnCount();
		uint32 sum = 0;
        for(uint32 i = 0; i < cc; i++)
		{
			uint8 size = DBTYPE_SizeOf( row->header().GetColumnType( i ) );

            sizeMap.insert( std::make_pair( size, i ) );
			sum += size;
		}

		// make sure there is enough data in buffer
		sum = ( ( sum + 7 ) >> 3 );
		unpacked.resize( sum, 0 );

        // current offset in unpacked:
        uint8 off = 0;

        std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
        cur = sizeMap.begin();
        end = sizeMap.lower_bound( 1 );
        for(; cur != end; cur++)
        {
            union
            {
                int64 i8;
				int32 i4;

                double r8;
                float r4;
            } v;
            v.i8 = 0;

            uint8 len = (cur->first >> 3);
            memcpy( &v, &unpacked[ off ], len );
            off += len;

            switch( row->header().GetColumnType( cur->second ) )
            {
                case DBTYPE_I8:
                case DBTYPE_UI8:
                case DBTYPE_CY:
                case DBTYPE_FILETIME:
					row->SetField( cur->second, new PyLong( v.i8 ) );
					break;

                case DBTYPE_I4:
                case DBTYPE_UI4:
                case DBTYPE_I2:
                case DBTYPE_UI2:
                case DBTYPE_I1:
                case DBTYPE_UI1:
                    row->SetField( cur->second, new PyInt( v.i4 ) );
                    break;

                case DBTYPE_R8:
                    row->SetField( cur->second, new PyFloat( v.r8 ) );
                    break;

                case DBTYPE_R4:
                    row->SetField( cur->second, new PyFloat( v.r4 ) );
                    break;
                case DBTYPE_BOOL:
                case DBTYPE_BYTES:
                case DBTYPE_STR:
                case DBTYPE_WSTR:
                    //! TODO: handle this DB types.
                    break;
            }
        }

        cur = sizeMap.lower_bound( 1 );
        end = sizeMap.lower_bound( 0 );
        for(uint8 bit_off = 0; cur != end; cur++)
        {
            if( bit_off > 7 )
            {
                off++;
                bit_off = 0;
            }

            row->SetField( cur->second, new PyBool( ( unpacked[ off ] >> bit_off) & 0x1 ) );

            bit_off++;
        }

        cur = sizeMap.lower_bound( 0 );
        end = sizeMap.end();
        for(; cur != end; cur++)
        {
            PyRep *el;
            uint32 el_len = UnmarshalData( state, packet, len, &el, n.c_str() );
            if( el == NULL )
                break;
            IncreaseIndex( el_len );

            row->SetField( cur->second, el );
        }

        *res = row;
        break; }

    case Op_PySubStream: {  //'+'

        if(len < 6) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for substream\n");
            break;
        }

        uint32 data_length = Getuint8();

        //special extended length substream
        if(data_length == 0xFF) {
            if(len < 10) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for a long substream\n");
                break;
            }

            data_length = Getuint32();
        }

        if(*packet != MarshalHeaderByte) {
            _log(NET__UNMARSHAL_ERROR, "Invalid header byte on substream data 0x%02x\n", *packet);
            break;
        }

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PySubStream of length %u", pfx, opcode, data_length);

        *res = new PySubStream( new PyBuffer( packet, data_length ) );

		IncreaseIndex( data_length );

        break; }

    //TUPLE2
    case Op_PyTwoTuple: {

        _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyTwoTuple", pfx, opcode);

        std::string n(pfx);
        n += "  [0] ";

        PyRep *i;
        uint32 key_len = UnmarshalData(state, packet, len, &i, n.c_str());
        if(i == NULL)
            break;
        IncreaseIndex( key_len );

        n = pfx;
        n += "  [1] ";

        PyRep *j;
        uint32 val_len = UnmarshalData(state, packet, len, &j, n.c_str());
        if(j == NULL) {
            PyDecRef( i );
            break;
        }
        IncreaseIndex( val_len );

        PyTuple *tuple = new PyTuple(2);
        tuple->SetItem( 0, i );
        tuple->SetItem( 1, j );

        *res = tuple;
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
        *res = new PyNone();


/* total crap... */
/*      if(len < 1) {
            _log(NET__UNMARSHAL_ERROR, "Not enough data for saved reference index\n");
            break;
        }
        uint8 save_index = *packet;
        packet++;
        len--;
        len_used++;

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


        res = new PyNone();
        */
//          _log(NET__UNMARSHAL_ERROR, "Unhandled field type 0x%x (maybe code related)", opcode);
//          _hex(NET__UNMARSHAL_ERROR, packet, len>32?32:len);
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

		PyString *r = new PyString( std::string( (const char *)packet, data_length ), false);

        if(ContainsNonPrintables( r )) {
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeString(len=%d, <binary>)", pfx, opcode, data_length);
            _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
            phex(NET__UNMARSHAL_BUFHEX, packet, data_length);
        } else
            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyUnicodeString(len=%d, \"%s\")", pfx, opcode, data_length, r->content().c_str());


        *res = r;
        break; }


    case Op_PyVarInteger: {

        /* this is one of the stranger fields I have found, it seems to be a variable
         * length integer field (somewhat of a 'bigint' style data type), but it gets
         * used at times for integers which would fit into the other primitive data
         * types.... I would really like to see the criteria they use to determine
         * what gets marshales as what...
         */

        uint32 data_length = Getuint8();
        //special extended length (I assume this wont happen for integers..., at least I hope not..)
        if(data_length == 0xFF) {
            if(len < sizeof(uint32)) {
                _log(NET__UNMARSHAL_ERROR, "Not enough data for an 0x2F\n");
                break;
            }

            data_length = Getuint32();
        }

        if(len < data_length) {
            _log(NET__UNMARSHAL_ERROR, "Ran out of data in 0x2F of length %d, %d bytes remain.\n", data_length, len);
            break;
        }

		if( data_length <= sizeof( int32 ) )
		{
            int32 intval = 0;
            memcpy( &intval, packet, data_length );

            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyVarInteger(len=%d) = %d", pfx, opcode, data_length, intval);
            *res = new PyInt(intval);
		}
		else if( data_length <= sizeof( int64 ) )
		{
            int64 intval = 0;
            memcpy( &intval, packet, data_length );

            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyVarInteger(len=%d) = "I64d, pfx, opcode, data_length, intval);
            *res = new PyLong(intval);
        }
		else
		{
            //uint64 is not big enough
            //just pass it up to the application layer as a buffer...
            PyBuffer *r = new PyBuffer(packet, data_length);

            _log(NET__UNMARSHAL_TRACE, "%s(0x%x)Op_PyVarInteger(len=%d)", pfx, opcode, data_length);
            _log(NET__UNMARSHAL_BUFHEX, "%s  Buffer Contents:", pfx);
            _hex(NET__UNMARSHAL_BUFHEX, packet, data_length);

            *res = r;
        }

        IncreaseIndex( data_length );

        break; }

    default:
        { _log(NET__UNMARSHAL_ERROR, "Unhandled (default) field type 0x%x\n", opcode);
            phex(NET__UNMARSHAL_ERROR, packet-1, len>32?32:len);
        } break;
    }   //end switch

    if( index != 0 )
    {
        //save off this pointer. We do not need to clone it since this function is the only
        //thing with a pointer to it, and we know it will remain valid until we are done.
        state.StoreReferencedObject( index, *res );
    }

    return len_used;
}
