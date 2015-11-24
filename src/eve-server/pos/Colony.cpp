/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Cometo
*/
#include "eve-server.h"
#include "Colony.h"

PyTuple *testContainer = new_tuple(2268, 1);
uint32 charID;
uint32 planetID;

Colony::Colony(uint32 cID, uint32 pID) {
    charID = cID;
    planetID = pID;
}

void Colony::CreateCommandPin(uint32 pinID, uint32 typeID, float latitude, float longitude) {
    Pin cc;
    cc.id = pinID;
    cc.typeID = typeID;
    cc.latitude = latitude;
    cc.longitude = longitude;
    cc.ownerID = charID;
    cc.state = STATE_IDLE; // ?
    cc.lastRunTime = 0L;
    cc.lastLaunchTime = 0L;
    cc.isCommandCenter = true;
    ccPin.level = 0;
    ccPin.currentSimTime = CurrentBlueTime();
    ccPin.pins.push_back(cc);
}


PyResult Colony::GetColony() {
    PyTuple *pins = new PyTuple(ccPin.pins.size());
    PyTuple *links = new PyTuple(ccPin.links.size());
    PyTuple *routes = new PyTuple(ccPin.routes.size());
    int index = 0; // used by each for loop.

    index = 0;
    for(auto i:ccPin.pins) {
        PyDict *dict = new PyDict();
        if(i.isCommandCenter) {
            dict->SetItem("id", new PyInt(i.id));
        }else {
            dict->SetItem("id", new_tuple(1, i.id));
        }
        dict->SetItem("typeID", new PyInt(i.typeID));
        dict->SetItem("ownerID", new PyInt(i.ownerID));
        dict->SetItem("latitude", new PyFloat(i.latitude));
        dict->SetItem("longitude", new PyFloat(i.longitude));
        dict->SetItem("lastRunTime", new PyLong(i.lastRunTime));
        dict->SetItem("state", new PyInt(i.state));
        dict->SetItem("contents", testContainer);
        dict->SetItem("schematicID", i.schematicID ? new PyInt(i.schematicID) : (PyRep*) new PyNone());
        dict->SetItem("hasRecievedInputs", i.hasRecievedInputs ? new PyBool(i.hasRecievedInputs) : (PyRep*) new PyNone());
        dict->SetItem("recievedInputsLastCycle", i.recievedInputsLastCycle ? new PyBool(i.recievedInputsLastCycle) : (PyRep*) new PyNone());
        dict->SetItem("lastLaunchTime", i.lastLaunchTime ? new PyLong(i.lastLaunchTime) : (PyRep*) new PyNone());
        dict->SetItem("heads", i.heads ? new PyInt(i.heads) : (PyRep*) new PyNone());
        dict->SetItem("programType", i.programType ? new PyInt(i.programType) : (PyRep*) new PyNone());
        dict->SetItem("cycleTime", i.cycleTime ? new PyInt(i.cycleTime) : (PyRep*) new PyNone());
        dict->SetItem("qtyPerCycle", i.qtyPerCycle ? new PyInt(i.qtyPerCycle) : (PyRep*) new PyNone());
        dict->SetItem("headRadius", i.headRadius ? new PyFloat(i.headRadius) : (PyRep*) new PyNone());
        dict->SetItem("installTime", i.installTime ? new PyLong(i.installTime) : (PyRep*) new PyNone());

        PyObject *obj = new PyObject("util.KeyVal", dict);
        pins->SetItem(index++, obj);
    }

    PyDict *args = new PyDict();
    PyObject *rtn = new PyObject("util.KeyVal", args);
    args->SetItem("level", new PyInt(ccPin.level));
    args->SetItem("currentSimTime", new PyLong(ccPin.currentSimTime));
    args->SetItem("pins", pins);
    args->SetItem("links", links);
    args->SetItem("routes", routes);

    rtn->Dump(stdout, "[DEBUG] GC:  ");
    return rtn;
}