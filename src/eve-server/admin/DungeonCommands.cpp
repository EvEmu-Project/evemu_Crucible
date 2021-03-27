
/**
 * @name DungeonCommands.cpp
 *     Dungeon Command system for EVEmu
 *
 * @Author:        Allan
 * @date:          20 August 2019
 *
 */


#include <stdio.h>
#include "eve-server.h"

#include "admin/AllCommands.h"
#include "admin/CommandDB.h"

#include "system/SolarSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"



PyResult Command_savedungeon(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    /**  the idea here is to save a dungeon room template while ingame.
     *  the client calling this is used as "warp-in point" which is the origin
     *  all items in bubble will be queried and their positions calculated, then saved.
     * we should expect a roomID here, and check for existing roomIDs to avoid overwrite.
     *      - maybe later when system matures, we can allow overwrite.  u/k at this time.
     */

    if (pClient == nullptr) // should never hit,
        throw PyException(MakeCustomError("SaveDungeon: Invalid Caller - Client sent NULL."));

    if (args.argCount() < 2)
        throw PyException(MakeCustomError("SaveDungeon: Missing Arguments (use '.savedungeon help' for usage)"));


    if (args.argCount() == 2) { //
        if (args.isNumber(1)) {

        }
    }

    if (strcmp(args.arg(1).c_str(), "help") == 0) {
        //  {.savedungeon help}  will display the following information in notification window
        std::ostringstream str; // for 'help' printing
        str << ".savedungeon [groupID] [group name/description]<br>"; //55
        int size = 55;
        char reply[size];
        snprintf(reply, size, str.str().c_str());
        pClient->SendInfoModalMsg(reply);
        return nullptr;
    } else if (args.isNumber(1)) {

    }

    throw PyException(MakeCustomError("SaveDungeon: - This command is currently incomplete."));
}

PyResult Command_printdungeon(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    /*  this will print all items in current dungeon, along with it's templateID and roomID
     *
     */

    throw PyException(MakeCustomError("PrintDungeon: - This command is currently incomplete."));
}

// this is outdated
PyResult Command_spawndungeon(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test dungeon spawn system - wip.   -allan 21Feb15
     */

    if (args.argCount() != 2)
        throw PyException(MakeCustomError("Correct Usage: .spawndungeon <dungeonTemplateID>"));

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument must be a template ID."));

    // pClient->SystemMgr()->GetDungMgr()->Create(args.arg(1));   // this is missing sig data.

    throw PyException(MakeCustomError("SpawnDungeon: - This command is currently incomplete."));
    return nullptr;
}

// this was never completed
PyResult Command_removedungeon(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test dungeon spawn system - wip.   -allan 21Feb15
     *
     * upon execution, this command will spawn a random dungeon from db in callers solarSystem,
     *   then create a bookmark in their PnP/BM window
     */
    throw PyException(MakeCustomError("RemoveDungeon: - This command is currently incomplete."));
    return nullptr;
}
