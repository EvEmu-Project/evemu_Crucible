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
    Author:        Bloody.Rabbit
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "account/UserService.h"

PyCallable_Make_InnerDispatcher(MovementService)

MovementService::MovementService(PyServiceMgr *mgr)
: PyService(mgr, "movementServer"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MovementService, ResolveNodeID);
}

MovementService::~MovementService() {
    delete m_dispatch;
}

PyResult MovementService::Handle_ResolveNodeID( PyCallArgs& call )
{
    sLog.Yellow( "MovementService", "Handle_ResolveNodeID" );
    call.Dump(CHARACTER__DEBUG);

    return new PyInt(888444);
}

PyCallable_Make_InnerDispatcher(UserService)

UserService::UserService(PyServiceMgr *mgr)
: PyService(mgr, "userSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(UserService, GetRedeemTokens);
    PyCallable_REG_CALL(UserService, ReverseRedeem);
    PyCallable_REG_CALL(UserService, GetCreateDate);
    PyCallable_REG_CALL(UserService, ReportISKSpammer);
    PyCallable_REG_CALL(UserService, ReportBot);
    PyCallable_REG_CALL(UserService, ApplyPilotLicence);
}

UserService::~UserService() {
    delete m_dispatch;
}

PyResult UserService::Handle_GetRedeemTokens( PyCallArgs& call )
{
    /*
    sLog.Yellow( "UserService", "Handle_GetRedeemTokens" );
    call.Dump(SERVICE__CALL_DUMP);
     * ==================== Sent from Client 84 bytes
     *
     * [PyObjectData Name: macho.CallReq]
     *  [PyTuple 7 items]
     *    [PyInt 6]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 2]
     *        [PyInt 0]
     *        [PyIntegerVar 13]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 3 items]
     *        [PyInt 8]
     *        [PyString "userSvc"]
     *        [PyNone]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 31 bytes]
     *          [PyTuple 4 items]
     *            [PyInt 1]
     *            [PyString "GetRedeemTokens"]
     *            [PyTuple 0 items]
     *            [PyDict 1 kvp]
     *              [PyString "machoVersion"]
     *              [PyInt 1]
     *    [PyNone]
     *    [PyNone]
     *
     *
     *
     * ==================== Sent from Server 200 bytes [Compressed]
     *
     * [PyObjectData Name: macho.CallRsp]
     *  [PyTuple 7 items]
     *    [PyInt 7]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 3 items]
     *        [PyInt 8]
     *        [PyString "userSvc"]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 2]
     *        [PyIntegerVar 241241000001103]
     *        [PyIntegerVar 13]
     *        [PyNone]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PySubStream 173 bytes]
     *        [PyObjectEx Type2]
     *          [PyTuple 2 items]
     *            [PyTuple 1 items]
     *              [PyToken dbutil.CRowset]
     *            [PyDict 1 kvp]
     *              [PyString "header"]
     *              [PyObjectEx Normal]
     *                [PyTuple 2 items]
     *                  [PyToken blue.DBRowDescriptor]
     *                  [PyTuple 1 items]
     *                    [PyTuple 10 items]
     *                      [PyTuple 2 items]
     *                        [PyString "tokenID"]
     *                        [PyInt 3]
     *                      [PyTuple 2 items]
     *                        [PyString "massTokenID"]
     *                        [PyInt 3]
     *                      [PyTuple 2 items]
     *                        [PyString "typeID"]
     *                        [PyInt 3]
     *                      [PyTuple 2 items]
     *                        [PyString "quantity"]
     *                        [PyInt 3]
     *                      [PyTuple 2 items]
     *                        [PyString "label"]
     *                        [PyInt 130]
     *                      [PyTuple 2 items]
     *                        [PyString "description"]
     *                        [PyInt 130]
     *                      [PyTuple 2 items]
     *                        [PyString "dateTime"]
     *                        [PyInt 64]
     *                      [PyTuple 2 items]
     *                        [PyString "expireDateTime"]
     *                        [PyInt 64]
     *                      [PyTuple 2 items]
     *                        [PyString "availableDateTime"]
     *                        [PyInt 64]
     *                      [PyTuple 2 items]
     *                        [PyString "stationID"]
     *                        [PyInt 3]
     *    [PyNone]
     *    [PyNone]
     *
     */

    return new PyList();
}

PyResult UserService::Handle_ReverseRedeem( PyCallArgs& call )
{
    //sm.RemoteSvc('userSvc').ReverseRedeem(item.itemID)
    sLog.Yellow( "UserService", "Handle_ReverseRedeem" );
    call.Dump(CHARACTER__DEBUG);

    return nullptr;
    
    /* {'messageKey': 'ReverseRedeemingCompleted', 'dataID': 17877681, 'suppressable': False, 'bodyID': 257344, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257343, 'messageID': 2917}
     * {'messageKey': 'ReverseRedeemingFatalError', 'dataID': 17877443, 'suppressable': False, 'bodyID': 257253, 'messageType': 'error', 'urlAudio': '', 'urlIcon': '', 'titleID': 257252, 'messageID': 2918}
     * {'messageKey': 'ReverseRedeemingIllegalType', 'dataID': 17877511, 'suppressable': False, 'bodyID': 257279, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257278, 'messageID': 2913}
     * {'messageKey': 'ReverseRedeemingOnlyInStation', 'dataID': 17877516, 'suppressable': False, 'bodyID': 257281, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257280, 'messageID': 2911}
     * {'messageKey': 'ReverseRedeemingYouDoNotOwnItem', 'dataID': 17877433, 'suppressable': False, 'bodyID': 257249, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257248, 'messageID': 2914}
     */
}

PyResult UserService::Handle_GetCreateDate( PyCallArgs& call )
{
    return new PyLong(call.client->GetChar()->createDateTime());
}

PyResult UserService::Handle_ReportISKSpammer( PyCallArgs& call )
{
    // sm.RemoteSvc('userSvc').ReportISKSpammer(charID, channelID, spamEntries)
    sLog.Yellow( "UserService", "Handle_ReportISKSpammer" );
    call.Dump(CHARACTER__DEBUG);

    return nullptr;
}

PyResult UserService::Handle_ReportBot( PyCallArgs& call )
{
    sLog.Yellow( "UserService", "Handle_ReportBot" );
    call.Dump(CHARACTER__DEBUG);

    return nullptr;
}

PyResult UserService::Handle_ApplyPilotLicence( PyCallArgs& call )
{
    //sm.RemoteSvc('userSvc').ApplyPilotLicence(itemID, justQuery=True)
    sLog.Yellow( "UserService", "Handle_ApplyPilotLicence" );
    call.Dump(CHARACTER__DEBUG);

    return nullptr;
}
