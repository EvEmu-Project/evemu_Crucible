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
    Author:        Captnoord
    Updates:    Allan
*/

#ifndef __CONTRACT_PROXY_H__INCL__
#define __CONTRACT_PROXY_H__INCL__

#include "PyService.h"

class ContractProxy
: public PyService
{
public:
    ContractProxy(PyServiceMgr *mgr);
    ~ContractProxy();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(CreateContract);
    PyCallable_DECL_CALL(GetContract);
    PyCallable_DECL_CALL(AcceptContract);
    PyCallable_DECL_CALL(CompleteContract);
    PyCallable_DECL_CALL(DeleteContract);
    PyCallable_DECL_CALL(NumOutstandingContracts);
    PyCallable_DECL_CALL(GetItemsInStation);
    PyCallable_DECL_CALL(GetLoginInfo);
    PyCallable_DECL_CALL(SearchContracts);
    PyCallable_DECL_CALL(CollectMyPageInfo);
    PyCallable_DECL_CALL(GetMyExpiredContractList);
    PyCallable_DECL_CALL(GetContractListForOwner);
};

#endif /* !__CONTRACT_PROXY_H__INCL__ */
