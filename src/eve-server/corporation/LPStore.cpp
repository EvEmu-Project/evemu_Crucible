/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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

#include "PyServiceCD.h"
#include "corporation/LPStore.h"

PyCallable_Make_InnerDispatcher(LPStore)

LPStore::LPStore(PyServiceMgr *mgr)
: PyService(mgr, "storeServer"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(LPStore, AcceptOffer);
    PyCallable_REG_CALL(LPStore, GetAvailableOffers);

}

LPStore::~LPStore()
{
    delete m_dispatch;
}


PyResult LPStore::Handle_AcceptOffer( PyCallArgs& call ) {
  /**
            return sm.RemoteSvc('storeServer').AcceptOffer(offerID, quantity)
            */
  sLog.White( "LPStore::Handle_GetLPsForCharacter()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyList;
}

PyResult LPStore::Handle_GetAvailableOffers( PyCallArgs& call ) {
  /**
            availableOffers = sm.RemoteSvc('storeServer').GetAvailableOffers()
            */
    //no args
  sLog.White( "LPStore::Handle_GetLPsForCharacter()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyList;
}

