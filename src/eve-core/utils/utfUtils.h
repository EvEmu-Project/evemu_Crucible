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
    Author:     eve-moo
 */

#ifndef UTFUTILS_H
#define	UTFUTILS_H

#include <string>

/**
 * Convert a UTF8 string to a UTF16 string
 * @param str The UTF8 string.
 * @return The UTF16 string. Returns an empty string on failure.
 */
std::u16string utf8to16(std::string &str);
/**
 * Convert a UTF16 string to a UTF8 string
 * @param str The UTF16 string.
 * @return The UTF8 string. Returns an empty string on failure.
 */
std::string utf16to8(std::u16string &str);

#endif	/* UTFUTILS_H */
