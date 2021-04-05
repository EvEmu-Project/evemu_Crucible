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

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "faction/WarRegistryService.h"

/*
 * FACWAR__ERROR
 * FACWAR__WARNING
 * FACWAR__INFO
 * FACWAR__MESSAGE
 * FACWAR__TRACE
 * FACWAR__CALL
 * FACWAR__CALL_DUMP
 * FACWAR__RSP_DUMP
 */

class WarRegistryBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(WarRegistryBound)

    WarRegistryBound(PyServiceMgr *mgr, uint32 corporationID)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)),
      m_corporationID(corporationID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "WarRegistryBound";

        PyCallable_REG_CALL(WarRegistryBound, GetWars);
        /*
     return self.GetMoniker().RetractWar(againstID)
     return self.GetMoniker().DeclareWarAgainst(againstID)
     return self.GetMoniker().ChangeMutualWarFlag(warID, mutual)
     return self.GetMoniker().GetCostOfWarAgainst(ownerID)
     */
    }
    ~WarRegistryBound()
    {
        delete m_dispatch;
    }

    void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetWars);

private:
    Dispatcher *const m_dispatch;

    uint32 m_corporationID;
};

PyCallable_Make_InnerDispatcher(WarRegistryService)

WarRegistryService::WarRegistryService(PyServiceMgr *mgr)
: PyService(mgr, "warRegistry"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

WarRegistryService::~WarRegistryService()
{
    delete m_dispatch;
}

PyBoundObject *WarRegistryService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    Call_TwoIntegerArgs args;
    //crap
    PyRep* tmp(bind_args->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode bind args.", GetName());
        return nullptr;
    }
    //arg1 = corporationID
    //arg2 = ???
    return new WarRegistryBound(m_manager, args.arg1);
}

PyResult WarRegistryBound::Handle_GetWars( PyCallArgs& call )
{
    sLog.Debug( "WarRegistryBound", "Called GetWars stub." );

    util_IndexRowset irowset;

    irowset.header.push_back( "warID" );
    irowset.header.push_back( "declaredByID" );
    irowset.header.push_back( "againstID" );
    irowset.header.push_back( "timeDeclared" );
    irowset.header.push_back( "timeFinished" );
    irowset.header.push_back( "retracted" );
    irowset.header.push_back( "retractedBy" );
    irowset.header.push_back( "billID" );
    irowset.header.push_back( "mutual" );

    irowset.idName = "warID";

    return irowset.Encode();
}

/*
            [PyTuple 2 items]
              [PyTuple 1 items]
                [PyToken dbutil.CIndexedRowset]
              [PyDict 2 kvp]
                [PyString "header"]
                [PyObjectEx Normal]
                  [PyTuple 2 items]
                    [PyToken blue.DBRowDescriptor]
                    [PyTuple 1 items]
                      [PyTuple 9 items]
                        [PyTuple 2 items]
                          [PyString "warID"]
                          [PyInt 3]
                        [PyTuple 2 items]
                          [PyString "declaredByID"]
                          [PyInt 3]
                        [PyTuple 2 items]
                          [PyString "againstID"]
                          [PyInt 3]
                        [PyTuple 2 items]
                          [PyString "timeDeclared"]
                          [PyInt 64]
                        [PyTuple 2 items]
                          [PyString "timeFinished"]
                          [PyInt 64]
                        [PyTuple 2 items]
                          [PyString "retracted"]
                          [PyInt 64]
                        [PyTuple 2 items]
                          [PyString "retractedBy"]
                          [PyInt 3]
                        [PyTuple 2 items]
                          [PyString "billID"]
                          [PyInt 3]
                        [PyTuple 2 items]
                          [PyString "mutual"]
                          [PyInt 11]
                [PyString "columnName"]
                [PyString "warID"]
                */
