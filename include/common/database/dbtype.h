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
    Author:     Bloody.Rabbit
*/

#ifndef __DBTYPE_H__INCL__
#define __DBTYPE_H__INCL__

/*
0x10 = "DBTYPE_I1"
0x11 = "DBTYPE_UI1"
0x0B = "DBTYPE_BOOL"
0x02 = "DBTYPE_I2"
0x12 = "DBTYPE_UI2"
0x03 = "DBTYPE_I4"
0x13 = "DBTYPE_UI4"
0x04 = "DBTYPE_R4"
0x14 = "DBTYPE_I8"
0x05 = "DBTYPE_R8"
0x15 = "DBTYPE_UI8"
0x06 = "DBTYPE_CY"
0x40 = "DBTYPE_FILETIME"
0x80 = "DBTYPE_BYTES"
0x81 = "DBTYPE_STR"
0x82 = "DBTYPE_WSTR"
*/

enum DBTYPE
{
    //DBTYPE_EMPTY        = 0x00, // not supported
    //DBTYPE_NULL         = 0x01, // not supported
    DBTYPE_I2             = 0x02,
    DBTYPE_I4             = 0x03,
    DBTYPE_R4             = 0x04,
    DBTYPE_R8             = 0x05,
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
    DBTYPE_I1             = 0x10,
    DBTYPE_UI1            = 0x11,
    DBTYPE_UI2            = 0x12,
    DBTYPE_UI4            = 0x13,
    DBTYPE_I8             = 0x14,
    DBTYPE_UI8	          = 0x15,
    DBTYPE_FILETIME       = 0x40, // 64-bit timestamp
    //DBTYPE_GUID         = 0x48, // not supported
    DBTYPE_BYTES          = 0x80,
    DBTYPE_STR            = 0x81,
    DBTYPE_WSTR           = 0x82
    //DBTYPE_NUMERIC      = 0x83, // not supported
    //DBTYPE_UDT          = 0x84, // not supported
    //DBTYPE_DBDATE       = 0x85, // not supported
    //DBTYPE_DBTIME       = 0x86, // not supported
    //DBTYPE_DBTIMESTAMP  = 0x87, // not supported
    //DBTYPE_HCHAPTER     = 0x88, // not supported
    //DBTYPE_PROPVARIANT  = 0x8A, // not supported
    //DBTYPE_VARNUMERIC   = 0x8B  // not supported
};

/** DBTYPE type flags
 *  DBTYPE_ARRAY        = 0x2000
 *  DBTYPE_BYREF        = 0x4000
 *  DBTYPE_VECTOR       = 0x1000
 *  DBTYPE_RESERVED     = 0x8000
 */

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
