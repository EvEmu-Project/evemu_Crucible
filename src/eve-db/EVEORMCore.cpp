//
// Created by andy on 2022-01-22.
//
#include "EVEORMCore.h"

#ifdef HAVE_ODB_PGSQL_H

#include <odb/pgsql/database.hxx>

#endif
#ifdef HAVE_ODB_MYSQL_H

#include <odb/mysql/database.hxx>

#endif

void
EVEORMCore::Initialize(dbtype type,
                      std::string &host,
                      std::string &user,
                      std::string &password,
                      std::string &database,
                      bool compress,
                      bool SSL,
                      int16_t port,
                      bool socket,
                      bool reconnect,
                      bool profile) {
    if ( type == dbtype::PGSQL ) {
        db.reset(new odb::pgsql::database(user, password, database, host, port));
    } else if ( type == dbtype::MYSQL ) {
        db.reset(new odb::mysql::database(user, password, database, host, port));
    }
}
