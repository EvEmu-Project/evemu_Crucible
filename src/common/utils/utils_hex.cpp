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
    Author:     Zhur
*/

#include "CommonPCH.h"

#include "log/logsys.h"
#include "utils/misc.h"
#include "utils/utils_hex.h"

void pfxHexDump(const char *pfx, FILE *into, const uint8 *data, uint32 length)
{
    char buffer[80];
    uint32 offset;
    for(offset=0;offset<length;offset+=16) {
        build_hex_line((const char *)data,length,offset,buffer,4);
        fprintf(into, "%s%s\n", pfx, buffer);
    }
}

void pfxHexDump(const char *pfx, LogType type, const uint8 *data, uint32 length)
{
    char buffer[80];
    uint32 offset;
    for(offset=0;offset<length;offset+=16) {
        build_hex_line((const char *)data,length,offset,buffer,4);
        _log(type, "%s%s\n", pfx, buffer);
    }
}

void pfxPreviewHexDump(const char *pfx, FILE *into, const uint8 *data, uint32 length)
{
    char buffer[80];

    if(length > 1024) {
        pfxHexDump(pfx, into, data, 1024-32);
        fprintf(into, "%s ... truncated ...\n", pfx);
        build_hex_line((const char *)data,length,length-16,buffer,4);
        fprintf(into, "%s%s\n", pfx, buffer);
    } else {
        pfxHexDump(pfx, into, data, length);
    }
}

void pfxPreviewHexDump(const char *pfx, LogType type, const uint8 *data, uint32 length)
{
    char buffer[80];

    if(length > 1024) {
        pfxHexDump(pfx, type, data, 1024-32);
        _log(type, "%s ... truncated ...", pfx);
        build_hex_line((const char *)data,length,length-16,buffer,4);
        _log(type, "%s%s", pfx, buffer);
    } else {
        pfxHexDump(pfx, type, data, length);
    }
}
