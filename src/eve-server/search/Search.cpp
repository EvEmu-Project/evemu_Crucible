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


#include "eve-server.h"

#include "search/Search.h"

PyCallable_Make_InnerDispatcher(Search)

Search::Search(PyServiceMgr* mgr)
: PyService(mgr, "search"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(Search, Query);
    PyCallable_REG_CALL(Search, QuickQuery);
}

Search::~Search() {
    delete m_dispatch;
}

PyResult Search::Handle_Query( PyCallArgs& call ) {
    Call_SearchQuery args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        call.client->SendErrorMsg("Search Failed.  Try using a different search string.");
        return nullptr;
    }

    std::string str = args.str;
    Replace(str);

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(args.str, cur))
            throw PyException( MakeCustomError("Search String contains invalid characters"));

    return m_db->Query( str, &args.ids, call.client->GetCharacterID() );
}


PyResult Search::Handle_QuickQuery( PyCallArgs& call )  {
/*         QuickQuery(query, groupIDList, hideNPC=hideNPC, onlyAltName=onlyAltName)
*/
    Call_SearchQuery args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        call.client->SendErrorMsg("Search Failed.  Try using a different search string.");
        return nullptr;
    }

    std::string str = args.str;
	Replace(str);

    bool hideNPC = true, onlyAltName = false;
    if (call.byname.find("hideNPC") != call.byname.end())
        hideNPC = (PyRep::IntegerValue(call.byname.find("hideNPC")->second) != 0);

    if (call.byname.find("onlyAltName") != call.byname.end())
        onlyAltName = (PyRep::IntegerValue(call.byname.find("onlyAltName")->second) != 0);

    // this hits db directly, so test for possible sql injection code
    for (const auto cur : badCharsSearch)
        if (EvE::icontains(args.str, cur))
            throw PyException( MakeCustomError("Search String contains invalid characters"));

    return m_db->QuickQuery( str, &args.ids, call.client->GetCharacterID(), hideNPC, onlyAltName);
}

void Search::Replace(std::string &str) {
    for (uint i = 0; i < str.length(); ++i)
        switch (str[i]) {
			case '*':
                str[i] = '%';
		}
}
