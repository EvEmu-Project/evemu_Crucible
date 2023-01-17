
 /**
  * @name DBCleaner.cpp
  *   Database cleaner module
  * @Author: James
  * @date:   10 October 2021
  */

#ifndef EVEMU_EVESERVER_DBCLEANER_H_
#define EVEMU_EVESERVER_DBCLEANER_H_

#include "eve-common.h"
#include "utils/Singleton.h"

class DBCleaner
: public Singleton<DBCleaner>
{
  public:
    DBCleaner()                                    { /* do nothing here */ }
    ~DBCleaner()                                   { /* do nothing here */ }

    void Initialize();
  private:
    void CleanEntity(uint32 type); // Clean entities by typeID from DB everywhere
    void CleanGroupFromSpace(uint32 groupID); // Clean entities by groupID in space only
    void CleanOrphanedWormholes();
    void CleanDungeonEntities();
};

//Singleton
#define dbClean \
    ( DBCleaner::get() )

#endif  // EVEMU_EVESERVER_DBCLEANER_H_

