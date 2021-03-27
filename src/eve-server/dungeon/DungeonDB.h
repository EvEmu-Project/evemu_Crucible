
/**
 * @name DungeonDB.h
 *   Dungeon DataBase methods
 *   Copyright 2018  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      13 December 2018
 *
 */

#ifndef _DUNGEON_DATABASE_H
#define _DUNGEON_DATABASE_H

class DungeonDB {
public:
    DungeonDB()                                         { /* do nothing here */ }
    ~DungeonDB()                                        { /* do nothing here */ }



    static void GetDunTemplates(DBQueryResult& res);
    

protected:

private:

};


#endif  // _DUNGEON_DATABASE_H