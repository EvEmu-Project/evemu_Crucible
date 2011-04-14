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

#include "CommonPCH.h"

#include "database/dbtype.h"

uint8 DBTYPE_GetSizeBits( DBTYPE type )
{
    switch( type )
    {
        case DBTYPE_I8:
        case DBTYPE_UI8:
        case DBTYPE_R8:
        case DBTYPE_CY:
        case DBTYPE_FILETIME:
            return 64;
        case DBTYPE_I4:
        case DBTYPE_UI4:
        case DBTYPE_R4:
            return 32;
        case DBTYPE_I2:
        case DBTYPE_UI2:
            return 16;
        case DBTYPE_I1:
        case DBTYPE_UI1:
            return 8;
        case DBTYPE_BOOL:
            return 1;
        case DBTYPE_BYTES:
        case DBTYPE_STR:
        case DBTYPE_WSTR:
            return 0;
    }

    return 0;
}

uint8 DBTYPE_GetSizeBytes( DBTYPE type )
{
    return ( ( DBTYPE_GetSizeBits( type ) + 7 ) >> 3 );
}

