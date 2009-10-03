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
	Author:		Zhur
*/

#ifndef CRC32_H
#define CRC32_H

class CRC32 {
public:
	// one buffer CRC32
	static uint32			Generate(const uint8* buf, uint32 bufsize);
	static uint32			GenerateNoFlip(const uint8* buf, uint32 bufsize); // Same as Generate(), but without the ~
	
	// Multiple buffer CRC32
	static uint32			Update(const uint8* buf, uint32 bufsize, uint32 _crc32 = 0xFFFFFFFF);
	static inline uint32	Finish(uint32 crc32)	{ return ~crc32; }
	static inline void		Finish(uint32* crc32)	{ *crc32 = ~(*crc32); }
	
private:
	static inline void		Calc(const uint8 byte, uint32& crc32);
};
#endif
