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

#ifndef EVE_MARSHAL_OPCODES_H
#define EVE_MARSHAL_OPCODES_H


typedef enum {	//PyRepOpcodes
	Op_PyNone = 0x1,
	Op_PyByteString = 0x2,	//a string with a single byte length... "Object Name"?
	Op_PyLongLong = 0x3,
	Op_PyLong = 0x4,
	Op_PySignedShort = 0x5,
	Op_PyByte = 0x6,
	Op_PyInfinite = 0x7,
	Op_PyZeroInteger = 0x8,
	Op_PyOneInteger = 0x9,
	Op_PyReal = 0xA,
	Op_PyZeroReal = 0xB,
	//unknown 0xC (unused)
	Op_PyBuffer = 0xD,
	Op_PyEmptyString = 0xE,	//not 100% sure.
	Op_PyCharString = 0xF,	//a single character string
	Op_PyByteString2 = 0x10,	//not sure the difference between this and Op_PyByteString
	Op_PyStringTableItem = 0x11,
	Op_PyUnicodeByteString = 0x12,
	//unknown 0x13
	Op_PyTuple = 0x14,
	Op_PyList = 0x15,
	Op_PyDict = 0x16,
	Op_PyObject = 0x17,
	//unknown 0x18
	Op_PySubStruct = 0x19,	//another stream nested in the current stream, length implied by marshaling a single element.
	//unknown 0x1A	"load"
	Op_PySavedStreamElement = 0x1B,
	Op_PyChecksumedStream = 0x1C,
	//unknown 0x1D (unused)
	//unknown 0x1E (unused)
	Op_PyTrue = 0x1F,
	Op_PyFalse = 0x20,				//  
	//unknown 0x21					// !
	Op_PackedObject1 = 0x22,		// "
	Op_PackedObject2 = 0x23,		// # still not 100% comleted
	Op_PyEmptyTuple = 0x24,			// $
	Op_PyOneTuple = 0x25,			// %
	Op_PyEmptyList = 0x26,			// &
	Op_PyOneList = 0x27,			// '
	Op_PyEmptyUnicodeString = 0x28,	// (
	Op_PyUnicodeCharString = 0x29,	// )  a single character string
	Op_PyPackedRow = 0x2A,				// *  still working on this one...
	Op_PySubStream = 0x2B,			// +
	Op_PyTwoTuple = 0x2C,			// ,
	Op_PackedTerminator = 0x2D,		// -
	Op_PyUnicodeString = 0x2E,		// .
	Op_PyVarInteger = 0x2F,			// /  variable length integer field???
	PyRepOpcodeMask = 0x3F
} PyRepOpcodes;	//6 bits

static const byte PyRepSaveMask = 0x40;
static const byte PyRepUnknownMask = 0x80;
	
	
static const uint8 SubStreamHeaderByte = 0x7E;	//'~'


#endif





