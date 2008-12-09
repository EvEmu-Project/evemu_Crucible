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

#ifndef _MISC_H
#define _MISC_H

#include <stdio.h>
#include <string>
#include <map>

using namespace std;

#ifdef WIN32
int64 strtoll(const char *str, const char **end, int8 base);
uint64 strtoull(const char *str, const char **end, int8 base);
#else
int print_stacktrace();
#endif

void dump_message_column(unsigned char *buffer, unsigned long length, string leader="", FILE *to = stdout);
string string_from_time(string pattern, time_t now=0);
string timestamp(time_t now=0);
string long2ip(unsigned long ip);
string pop_arg(string &s, string seps, bool obey_quotes);
string generate_key(int length);
void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding=4);
bool ContainsNonPrintables(const char *str);
bool ContainsNonPrintables(const char *str, uint32 length);
void EscapeStringSequence(std::string &subject,  const std::string &find, const std::string &replace);
uint32 CalcCRC16(uint8 *bin_data, int len, uint32 initial_crc);

uint32 GetFileLength(const char *file);

#endif
