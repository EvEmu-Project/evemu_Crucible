
/**
 * @name DungeonCommands.h
 *     Dungeon Command system for EVEmu
 *
 * @Author:        Allan
 * @date:          20 August 2019
 *
 */




COMMAND( savedungeon, Acct::Role::CONTENT,
         " - saves dungeon <dungeonID> to server database")
COMMAND( printdungeon, Acct::Role::CONTENT,
         " - not sure what this is supposed to do yet")
COMMAND( spawndungeon, Acct::Role::CONTENT,
         " - spawns dungeon <dungeonID> in your system, with BM to location (outdated)")
COMMAND( removedungeon, Acct::Role::CONTENT,
         " - removes identified dungeon(unimplemented)")


