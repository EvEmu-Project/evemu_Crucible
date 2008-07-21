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

#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "EVECollectDisp.h"

extern int EVE_NIDS_main(EVECollectDispatcher *disp, int argc, char *argv[]);

int main(int argc, char *argv[]) {

	if(!load_log_settings("log.ini"))
		_log(COLLECT__MESSAGE, "Warning: Unable to read %s", "log.ini");
	else
		_log(COLLECT__MESSAGE, "Log settings loaded from %s", "log.ini");

	if(!PyRepString::LoadStringFile("strings.txt"))
		_log(COLLECT__ERROR, "Unable to open ./strings.txt, I need it to decode string table elements!");

	EVECollectDispatcher dispatch;
	dispatch.lookResolver.LoadIntFile("intres.txt");
	dispatch.lookResolver.LoadStringFile("strres.txt");
	
	return(EVE_NIDS_main(&dispatch, argc, argv));
}




