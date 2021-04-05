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
    Author:        Zhur
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "contract/ContractMgr.h"

PyCallable_Make_InnerDispatcher(ContractMgr)

ContractMgr::ContractMgr(PyServiceMgr *mgr)
: PyService(mgr, "contractMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ContractMgr, NumRequiringAttention);
}

ContractMgr::~ContractMgr()
{
    delete m_dispatch;
}

PyResult ContractMgr::Handle_NumRequiringAttention( PyCallArgs& call )
{
    sLog.Warning( "ContractMgr", "Called NumRequiringAttention stub." );

    PyDict* args = new PyDict;
    args->SetItemString( "n", new PyInt( 0 ) );
    args->SetItemString( "ncorp", new PyInt( 0 ) );

    return new PyObject( "util.KeyVal", args );
}


/*
conAvailMyAlliance = 3
conAvailMyCorp = 2
conAvailMyself = 1
conAvailPublic = 0
conStatusOutstanding = 0
conStatusInProgress = 1
conStatusFinishedIssuer = 2
conStatusFinishedContractor = 3
conStatusFinished = 4
conStatusCancelled = 5
conStatusRejected = 6
conStatusFailed = 7
conStatusDeleted = 8
conStatusReversed = 9
conTypeNothing = 0
conTypeItemExchange = 1
conTypeAuction = 2
conTypeCourier = 3
conTypeLoan = 4
*/