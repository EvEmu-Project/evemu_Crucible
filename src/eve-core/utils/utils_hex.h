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
    Author:     Zhur
*/

#ifndef UTILS_HEX_H
#define UTILS_HEX_H

/** The maximal number of bytes printed by pfxHexDumpPreview. */
extern const uint32 HEX_DUMP_PREVIEW_LIMIT;

/**
 * @brief Build a printable line suitable for hex dump.
 *
 * @param[in]  buffer  Buffer which is being dumped.
 * @param[in]  length  Length of buffer.
 * @param[in]  offset  Offset at which the dump should start.
 * @param[out] ret     Array where result is stored.
 * @param[in]  padding Padding to be used when printing offset.
 */
void build_hex_line( const uint8* buffer, size_t length, size_t offset, char* ret, unsigned int padding = 4 );

void pfxHexDump( const char* pfx, FILE* into, const uint8* data, uint32 length );
void pfxHexDump( const char* pfx, LogType type, const uint8* data, uint32 length );
void pfxHexDumpPreview( const char* pfx, FILE* into, const uint8* data, uint32 length );
void pfxHexDumpPreview( const char* pfx, LogType type, const uint8* data, uint32 length );

#endif // UTILS_HEX_H
