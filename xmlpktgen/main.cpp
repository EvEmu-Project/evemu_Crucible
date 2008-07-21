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


#include "../common/common.h"
#include "../common/logsys.h"
#include "XMLPacketGen.h"
#include <string>
#include <sys/stat.h>


int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s [packet xml file]\n", argv[0]);
		return(1);
	}
	
	const char *file = argv[1];
	
	std::string basename = file;
	basename = basename.substr(0, basename.length()-5);

	std::string xml = basename + ".xmlp";
	std::string h = basename + ".h";
	std::string cpp = basename + ".cpp";

	if(xml != file) {
		printf("Error: bad input filename '%s'.\n", file);
		return(1);
	}

#ifdef WIN32
	//this is a hack to deal with the issue that we cannot seem to get
	//VC++ to properly track the dependancies for custom build rules,
	//which results in it always rebuilding the XML packets, which makes
	// build times incredibly long. This checks file modification dates
	// similar to `make` before actually building anything... however,
	// on unix, where make is doing this for us, we also set ourself
	// dependant on the xmlpktgen binary itself, so if it changes, we
	// will rebuild all the xmlp files... so this check is not adequate.
	// It would be better to have this code preform the same check, but
	// for now, I dont feel like writting it, so I just exclude this check
	// on unix, and let make do its job.
	struct stat xml_stat, cpp_stat, h_stat;
	if(stat(xml.c_str(), &xml_stat) < 0) {
		fprintf(stderr, "Error: Unable to stat '%s'.\n", xml.c_str());
		return(1);
	}
	if(stat(h.c_str(), &h_stat) >= 0 && stat(cpp.c_str(), &cpp_stat) >= 0) {
		//all three files exist, check times.
		if(xml_stat.st_mtime < h_stat.st_mtime && xml_stat.st_mtime < cpp_stat.st_mtime) {
			printf("%s is older than %s and %s, not rebuilding.\n", xml.c_str(), h.c_str(), cpp.c_str());
			return(0);
		}
	}
#endif
	
	XMLPacketGen gen;
	if(!gen.GenPackets(xml.c_str(), h.c_str(), cpp.c_str()))
		return(1);
	
	return(0);
}




