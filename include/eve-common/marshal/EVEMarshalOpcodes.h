/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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

#ifndef EVE_MARSHAL_OPCODES_H
#define EVE_MARSHAL_OPCODES_H

enum PyRepOpcode
{
	Op_PyNone				= 0x01,
	Op_PyToken				= 0x02,
	Op_PyLongLong			= 0x03,
	Op_PyLong				= 0x04,
	Op_PySignedShort		= 0x05,
	Op_PyByte				= 0x06,
	Op_PyMinusOne			= 0x07,
	Op_PyZeroInteger		= 0x08,
	Op_PyOneInteger			= 0x09,
	Op_PyReal				= 0x0A,
	Op_PyZeroReal			= 0x0B,
	//Op_PyUnused1			= 0x0C,
	Op_PyBuffer				= 0x0D,
	Op_PyEmptyString		= 0x0E,	//not 100% sure.
	Op_PyCharString			= 0x0F,	//a single character string
	Op_PyShortString		= 0x10,	//string limited to 255 chars
	Op_PyStringTableItem	= 0x11,
	Op_PyWStringUCS2		= 0x12,
	Op_PyLongString			= 0x13, //string with length extension support
	Op_PyTuple				= 0x14,
	Op_PyList				= 0x15,
	Op_PyDict				= 0x16,
	Op_PyObject				= 0x17,
	//Op_Pyunk2				= 0x18,
	Op_PySubStruct			= 0x19,	//another stream nested in the current stream, length implied by marshaling a single element.
	//Op_Pyunk3				= 0x1A, //"load"
	Op_PySavedStreamElement = 0x1B,
	Op_PyChecksumedStream	= 0x1C,
	//Op_PyUnused2			= 0x1D,
	//Op_PyUnused3			= 0x1E,
	Op_PyTrue				= 0x1F,
	Op_PyFalse				= 0x20,
	Op_cPicked				= 0x21,
	Op_PyObjectEx1			= 0x22,
	Op_PyObjectEx2			= 0x23, // still not 100% completed
	Op_PyEmptyTuple			= 0x24,
	Op_PyOneTuple			= 0x25,
	Op_PyEmptyList			= 0x26,
	Op_PyOneList			= 0x27,
	Op_PyEmptyWString       = 0x28,
	Op_PyWStringUCS2Char	= 0x29,	// a single character string
	Op_PyPackedRow			= 0x2A,	// still working on this one...
	Op_PySubStream			= 0x2B,
	Op_PyTwoTuple			= 0x2C,
	Op_PackedTerminator		= 0x2D,
	Op_PyWStringUTF8		= 0x2E,
	Op_PyVarInteger			= 0x2F, // variable length integer field??? // PyLong from Byte array
	PyRepOpcodeMask			= 0x3F
};	//6 bits

struct ZeroCompressOpcode
{
	uint8 firstLen : 3;
	bool firstIsZero : 1;

	uint8 secondLen : 3;
	bool secondIsZero : 1;
};

static const uint8 PyRepSaveMask = 0x40;
static const uint8 PyRepUnknownMask = 0x80;
static const uint8 MarshalHeaderByte = 0x7E;	//'~'

#endif//EVE_MARSHAL_OPCODES_H





