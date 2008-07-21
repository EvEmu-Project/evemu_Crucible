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

#ifndef CRC32_H
#define CRC32_H
#include "types.h"

class CRC32 {
public:
	// one buffer CRC32
	static uint32			Generate(const int8* buf, uint32 bufsize);
	static uint32			GenerateNoFlip(const int8* buf, uint32 bufsize); // Same as Generate(), but without the ~
	
	// Multiple buffer CRC32
	static uint32			Update(const int8* buf, uint32 bufsize, uint32 crc32 = 0xFFFFFFFF);
	static inline uint32	Finish(uint32 crc32)	{ return ~crc32; }
	static inline void		Finish(uint32* crc32)	{ *crc32 = ~(*crc32); }
	
private:
	static inline void		Calc(const int8 byte, uint32& crc32);
};
#endif
