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

#include "services/Service.h"

class ContractProxy : public Service <ContractProxy>
{
public:
    ContractProxy();

protected:
    PyResult CreateContract(PyCallArgs &call, PyInt* contractType, PyBool* isPrivate, std::optional <PyNone*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID, PyInt* price, PyInt* reward, PyInt* collateral, PyString* title, PyString* description);
    PyResult CreateContract(PyCallArgs &call, PyInt* contractType, PyBool* isPrivate, std::optional <PyInt*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID, PyInt* price, PyInt* reward, PyInt* collateral, PyString* title, PyString* description);
    PyResult CreateContract(PyCallArgs &call, PyInt* contractType, PyBool* isPrivate, std::optional <PyInt*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID, PyInt* price, PyInt* reward, PyInt* collateral, PyWString* title, PyString* description);
    PyResult CreateContract(PyCallArgs &call, PyInt* contractType, PyBool* isPrivate, std::optional <PyNone*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyInt*> endStationID, PyInt* price, PyInt* reward, PyInt* collateral, PyString* title, PyString* description);
    PyResult CreateContract(PyCallArgs &call, PyInt* contractType, PyBool* isPrivate, std::optional <PyNone*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyNone*> endStationID, PyInt* price, PyInt* reward, PyInt* collateral, PyWString* title, PyString* description);
    PyResult CreateContract(PyCallArgs& call, PyInt* contractType, PyInt* isPrivate, std::optional <PyInt*> assigneeID, PyInt* expireTime, PyInt* duration, PyInt* startStationID, std::optional<PyInt*> endStationID, PyInt* price, PyInt* reward, PyInt* collateral, PyWString* title, PyWString* description);
    PyResult GetContract(PyCallArgs& call, PyInt* contractID);
    PyResult AcceptContract(PyCallArgs& call, PyInt* contractID);
    PyResult CompleteContract(PyCallArgs& call, PyInt* contractID, PyInt* completionStatus);
    PyResult DeleteContract(PyCallArgs& call, PyInt* contractID);
    PyResult NumOutstandingContracts(PyCallArgs& call);
    PyResult GetItemsInStation(PyCallArgs& call, PyInt* stationID, std::optional<PyInt*> forCorp);
    PyResult GetLoginInfo(PyCallArgs& call);
    PyResult SearchContracts(PyCallArgs& call);
    PyResult CollectMyPageInfo(PyCallArgs& call);
    PyResult GetMyExpiredContractList(PyCallArgs& call);
    PyResult GetContractListForOwner(PyCallArgs& call, PyInt* ownerID, PyInt* contractStatus, std::optional <PyInt*> contractType, std::optional <PyBool*> issuedToBy);
};

#endif /* !__CONTRACT_PROXY_H__INCL__ */
