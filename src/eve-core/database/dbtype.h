/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:     Bloody.Rabbit
*/

#ifndef __DBTYPE_H__INCL__
#define __DBTYPE_H__INCL__

/*
 0x02(02) = "DBTYPE_I2"
 0x03(03) = "DBTYPE_I4"
 0x04(04) = "DBTYPE_R4"
 0x05(05) = "DBTYPE_R8"
 0x06(06) = "DBTYPE_CY"
 0x0B(11) = "DBTYPE_BOOL"
 0x10(16) = "DBTYPE_I1"
 0x11(17) = "DBTYPE_UI1"
 0x12(18) = "DBTYPE_UI2"
 0x13(19) = "DBTYPE_UI4"
 0x14(20) = "DBTYPE_I8"
 0x15(21) = "DBTYPE_UI8"
 0x40(64) = "DBTYPE_FILETIME"
 0x80(128) = "DBTYPE_BYTES"
 0x81(129) = "DBTYPE_STR"
 0x82(130) = "DBTYPE_WSTR"
 ....from client......
 DBTYPE_BOOL = 11
 DBTYPE_I1 = 16
 DBTYPE_UI1 = 17
 DBTYPE_I2 = 2
 DBTYPE_UI2 = 18
 DBTYPE_I4 = 3
 DBTYPE_UI4 = 19
 DBTYPE_R4 = 4
 DBTYPE_I8 = 20
 DBTYPE_UI8 = 21
 DBTYPE_R5 = 5
 DBTYPE_CY = 6
 DBTYPE_FILETIME = 64
 DBTYPE_DBTIMESTAMP = 135
 DBTYPE_STR = 129
 DBTYPE_WSTR = 130
 DBTYPE_BYTES = 508
 DBTYPE_EMPTY = 0
*/

enum DBTYPE
{
    DBTYPE_EMPTY          = 0x00,
    //DBTYPE_NULL         = 0x01, // not supported
    DBTYPE_I2             = 0x02, // int16
    DBTYPE_I4             = 0x03, // int32
    DBTYPE_R4             = 0x04, // float
    DBTYPE_R8             = 0x05, // double
    DBTYPE_CY             = 0x06, // money
    //DBTYPE_DATE         = 0x07, // not supported
    //DBTYPE_BSTR         = 0x08, // not supported
    //DBTYPE_IDISPATCH    = 0x09, // not supported
    DBTYPE_ERROR          = 0x0A,
    DBTYPE_BOOL           = 0x0B,
    //DBTYPE_VARIANT      = 0x0C, // not supported
    //DBTYPE_IUNKNOWN     = 0x0D, // not supported
    //DBTYPE_DECIMAL      = 0x0E, // not supported
    //DBTYPE_BLOB         = 0x0F, // not supported
    DBTYPE_I1             = 0x10, // int8
    DBTYPE_UI1            = 0x11, // uint8
    DBTYPE_UI2            = 0x12, // uint16
    DBTYPE_UI4            = 0x13, // uint32
    DBTYPE_I8             = 0x14, // int64
    DBTYPE_UI8            = 0x15, // uint64  *not used pre-incursion*
    DBTYPE_FILETIME       = 0x40, // 64-bit timestamp
    //DBTYPE_GUID         = 0x48, // not supported
    DBTYPE_BYTES          = 0x80, // client defined as 0x1fc, but error when used: Exception:  <type 'exceptions.TypeError'>, DBRowDescriptor doesn't support data type 508
    DBTYPE_STR            = 0x81,
    DBTYPE_WSTR           = 0x82,
    //DBTYPE_NUMERIC      = 0x83, // not supported
    //DBTYPE_UDT          = 0x84, // not supported
    //DBTYPE_DBDATE       = 0x85, // not supported
    //DBTYPE_DBTIME       = 0x86, // not supported
    //DBTYPE_DBTIMESTAMP  = 0x87, // not supported  -client has this one defined
    //DBTYPE_HCHAPTER     = 0x88, // not supported
    //DBTYPE_PROPVARIANT  = 0x8A, // not supported
    //DBTYPE_VARNUMERIC   = 0x8B  // not supported
};

/**
 * Returns length of field of given type.
 *
 * @param[in] type Type of fields.
 * @return Length of field (in bits).
 */
extern uint8 DBTYPE_GetSizeBits( DBTYPE type );

/**
 * Returns length of field of given type.
 *
 * @param[in] type Type of fields.
 * @return Length of field (in bytes).
 */
extern uint8 DBTYPE_GetSizeBytes( DBTYPE type );

#endif /* !__DBTYPE_H__INCL__ */
