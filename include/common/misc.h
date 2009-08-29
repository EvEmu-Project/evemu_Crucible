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

#ifndef _MISC_H
#define _MISC_H

#include <stdio.h>
#include <string>
#include <map>

using namespace std;

#ifndef WIN32
int print_stacktrace();
#endif

class PyString;

//void dump_message_column(unsigned char *buffer, unsigned long length, string leader="", FILE *to = stdout);
string generate_key(int length);
void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding=4);

void EscapeStringSequence(std::string &subject,  const std::string &find, const std::string &replace);
uint32 CalcCRC16(uint8 *bin_data, int len, uint32 initial_crc);

uint32 GetFileLength(const char *file);

#endif
