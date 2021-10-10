
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
    void CleanEntity(uint32 type); // Clean old cynosural fields which are lurking in the DB

};

//Singleton
#define dbClean \
    ( DBCleaner::get() )

#endif  // EVEMU_EVESERVER_DBCLEANER_H_

