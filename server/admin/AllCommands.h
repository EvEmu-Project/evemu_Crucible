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
#ifndef __ALLCOMMANDS_H_INCL__
#define __ALLCOMMANDS_H_INCL__

#include "../common/types.h"
#include "../common/packet_types.h"

//these are not absolutely essential to be included, we could 'class' them all,
//doing this to make the implementation files need less includes
#include "../common/logsys.h"
#include "../common/seperator.h"
#include "CommandDB.h"
#include "../PyServiceMgr.h"
#include "../PyCallable.h"
#include "../Client.h"
#include "../EntityList.h"
#include "../mining/Asteroid.h"

/* this file includes all of the macro headers for each class of commands and
 * actually prototypes them
 *
 * if you add a new command header/file, you need to add it to the
 * AllCommandsList.h file.
 */

#define COMMAND(name, role, description) \
	PyResult Command_##name(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args);
#include "AllCommandsList.h"


class CommandDispatcher;
extern void RegisterAllCommands(CommandDispatcher *into);
void GetAsteroid(Client *who, uint32 typeID, double radius, const Ga::GaVec3 & position);

#endif




