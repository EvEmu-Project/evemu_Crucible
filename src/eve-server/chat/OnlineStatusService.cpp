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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "EntityList.h"
#include "chat/OnlineStatusService.h"

PyCallable_Make_InnerDispatcher(OnlineStatusService)

OnlineStatusService::OnlineStatusService(PyServiceMgr *mgr)
: PyService(mgr, "onlineStatus"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(OnlineStatusService, GetInitialState);
    PyCallable_REG_CALL(OnlineStatusService, GetOnlineStatus);
}

OnlineStatusService::~OnlineStatusService() {
    delete m_dispatch;
}

/** @todo finish this */
PyResult OnlineStatusService::Handle_GetInitialState(PyCallArgs &call) {
/*
21:35:16 L OnlineStatusService::Handle_GetInitialState(): size= 0
21:35:16 [SvcCall]   Call Arguments:
21:35:16 [SvcCall]       Tuple: Empty
21:35:16 [SvcCall]   Call Named Arguments:
21:35:16 [SvcCall]     Argument 'machoVersion':
21:35:16 [SvcCall]         Integer field: 1
  sLog.White( "OnlineStatusService::Handle_GetInitialState()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);


      [PySubStream 1861 bytes]
        [PyObjectEx Type2]
          [PyTuple 2 items]
            [PyTuple 1 items]
              [PyToken dbutil.CRowset]
            [PyDict 1 kvp]
              [PyString "header"]
              [PyObjectEx Normal]
                [PyTuple 2 items]
                  [PyToken blue.DBRowDescriptor]
                  [PyTuple 1 items]
                    [PyTuple 2 items]
                      [PyTuple 2 items]
                        [PyString "contactID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "online"]
                        [PyInt 3]
          [PyPackedRow 9 bytes]
            ["contactID" => <108985089> [I4]]
            ["online" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contactID" => <116081192> [I4]]
            ["online" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contactID" => <116844755> [I4]]
            ["online" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contactID" => <118879785> [I4]]
            ["online" => <1> [I4]]
  */
    // this is used to query the initial online state of all contacts.

    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("contactID", DBTYPE_I4);
    header->AddColumn("online", DBTYPE_BOOL);
    CRowSet *rowset = new CRowSet( &header );
    // loop thru contact list and fill following row accordingly
    //PyPackedRow* row = rowset->NewRow();
    //row->SetField(new PyInt(*charID*), sEntityList.PyIsOnline(PyRep::IntegerValue(charID)))); // charID/online
    return rowset;
}

PyResult OnlineStatusService::Handle_GetOnlineStatus(PyCallArgs &call) {
    // this is used to query the online state of a character by charID.
     return sEntityList.PyIsOnline(PyRep::IntegerValue(call.tuple->GetItem(0)));
}

/*
==================== Sent from Server 81 bytes

[PyObjectData Name: macho.Notification]
  [PyTuple 7 items]
    [PyInt 12]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 806438]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 4]
        [PyString "OnContactLoggedOn"]
        [PyList 0 items]
        [PyString "clientID"]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 15 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 1 items]
                [PyInt 649670823]
    [PyNone]
    [PyNone]

==================== Sent from Server 81 bytes

[PyObjectData Name: macho.Notification]
  [PyTuple 6 items]
    [PyInt 12]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 698462]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 4]
        [PyString "OnContactLoggedOff"]
        [PyList 0 items]
        [PyString "clientID"]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 15 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 1 items]
                [PyInt 1610990724]
    [PyNone]


*/