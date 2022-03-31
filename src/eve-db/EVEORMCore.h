//
// Created by andy on 2022-01-22.
//

#ifndef EVEMU_EVEORMCORE_H
#define EVEMU_EVEORMCORE_H

#include <odb/database.hxx>

#ifdef HAVE_ODB_PGSQL_H

#include <odb/pgsql/database.hxx>

#endif
#ifdef HAVE_ODB_MYSQL_H

#include <odb/mysql/database.hxx>

#endif

#include "utils/Singleton.h"

using namespace std;
using namespace odb::core;

class EVEORMCore
    : public Singleton<EVEORMCore> {

public:
    EVEORMCore() = default;

    ~EVEORMCore() = default;

    typedef enum {
        PGSQL,
        MYSQL,
    } dbtype;

    void Initialize(dbtype type,
                    string &host,
                    string &user,
                    string &password,
                    string &database,
                    bool compress = false,
                    bool SSL = false,
                    int16_t port = 3306,
                    bool socket = false,
                    bool reconnect = false,
                    bool profile = false);
    shared_ptr<odb::database> Get() {
        return db;
    }
private:
    shared_ptr<odb::database> db;
};

#define odbDB (EVEORMCore::get().Get())
#endif //EVEMU_EVEORMCORE_H
