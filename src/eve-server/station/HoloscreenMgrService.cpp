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
    Author:        Ubiquitatis
*/

#include "eve-server.h"

#include "station/HoloscreenMgrService.h"

HoloscreenMgrService::HoloscreenMgrService() :
    Service("holoscreenMgr")
{
    this->Add("GetRecentEpicArcCompletions", &HoloscreenMgrService::GetRecentEpicArcCompletions);
    this->Add("GetTwoHourCache", &HoloscreenMgrService::GetTwoHourCache);
    this->Add("GetRuntimeCache", &HoloscreenMgrService::GetRuntimeCache);
}

//those objects should be cached

PyResult HoloscreenMgrService::GetRecentEpicArcCompletions(PyCallArgs& call)
{       //  this is cached object!!!
    sLog.Debug("HoloscreenMgrService", "Called GetRecentEpicArcCompletions stub.");

    /*
     *        characterID
     *        completionDate
     */

    return nullptr;
}

PyResult HoloscreenMgrService::GetTwoHourCache(PyCallArgs& call)
{
    PyDict* agents = new PyDict();
        agents->SetItemString("Agent_DUMMY", new PyDict());
    PyDict* args = new PyDict();
        args->SetItemString("careerAgents", agents);
        args->SetItemString("incursionReport", new PyList());
        args->SetItemString("epicArcAgents", new PyDict());
        args->SetItemString("sovChangesReport", new PyList());
    return new PyObject("util.KeyVal", args);
}

PyResult HoloscreenMgrService::GetRuntimeCache(PyCallArgs& call)
{
    PyDict* agents = new PyDict();
        agents->SetItemString("Agent_DUMMY", new PyDict());
    PyDict* args = new PyDict();
        args->SetItemString("careerAgents", agents);
        args->SetItemString("incursionReport", new PyList());
        args->SetItemString("epicArcAgents", new PyDict());
        args->SetItemString("sovChangesReport", new PyList());
    return new PyObject("util.KeyVal", args);
}

/*  sovChangesReport
      solarSystemID
      oldOwnerID
      newOwnerID
    */

/*  incursionReport
      influence
      stagingSolarSystemID
    */

/*
ideas for holoscreen in CQ:
    - corp recruit ads
    - incursion data
    - relevant system data for player's current system
    - others?
    - more?

    */



/*
 *
        [PySubStream 112 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "GetCachableObject"]
            [PyTuple 4 items]
              [PyInt 1]
              [PyTuple 3 items]
                [PyString "Method Call"]
                [PyString "server"]
                [PyTuple 2 items]
                  [PyString "holoscreenMgr"]
                  [PyString "GetRuntimeCache"]
              [PyTuple 2 items]
                [PyIntegerVar 129519218650427395]
                [PyInt 53737]
              [PyInt 700318]
              
      [PySubStream 774 bytes]
        [PyObjectData Name: objectCaching.CachedObject]
          [PyTuple 7 items]
            [PyTuple 2 items]
              [PyIntegerVar 129519242704509384]
              [PyInt 9603]
            [PyNone]
            [PyInt 700322]
            [PyInt 1]
            [PyString "xm?Kka??L????m?T
 ? ??6TP.*?%?k?b??4??*?2I?n?????i??h????????C?? ?_?}G:?B????????i????f????D?5r8[??5-??1??-_?7^N,K?2???`9m?UF???x`??<pK?g???P}?6HE???2rI?_????9mJ?
 ???? }N?A?9???   ?}??Nd^???? ?l???# ???0?Q?+ ?      ?`X@??UwA?UiD?????????&d??DJe?;Ad???
 "u3?? 2V??"uS]/?T?U

 */
