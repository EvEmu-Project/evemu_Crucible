/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Allan
*/


#ifndef EVEMU_SYSTEM_SEARCH_H_
#define EVEMU_SYSTEM_SEARCH_H_

#include "PyService.h"
#include "PyServiceCD.h"
#include "search/SearchDB.h"

class Search
: public PyService {
  public:
    Search(PyServiceMgr* mgr);
    virtual ~Search();

  protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(Query);
    PyCallable_DECL_CALL(QuickQuery);

  private:
    SearchDB* m_db;

    // this is specific to Search class.  replaces EvE wildcard (*) with MYSQL wildcard (%)
	void Replace(std::string &s);

};

#endif      // EVEMU_SYSTEM_SEARCH_H_

