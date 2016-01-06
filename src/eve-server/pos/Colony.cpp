/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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

PyDict *testContainer;
uint32 charID;
uint32 planetID;

Colony::Colony(uint32 cID, uint32 pID) {
    charID = cID;
    planetID = pID;
    testContainer = new PyDict();
    testContainer->SetItem(new PyInt(2268), new PyInt(1));
}

bool Colony::CreateCommandPin(uint32 pinID, uint32 typeID, float latitude, float longitude) {
    Pin cc;
    cc.id = pinID;
    cc.typeID = typeID;
    cc.latitude = latitude;
    cc.longitude = longitude;
    cc.ownerID = charID;
    cc.state = STATE_IDLE; // ?
    cc.lastRunTime = 0L;
    cc.lastLaunchTime = 0L;
    cc.isLaunchable = true;
    cc.isCommandCenter = true;
    ccPin.level = 0;
    ccPin.currentSimTime = CurrentBlueTime();
    ccPin.pins.push_back(cc);
    return true;    // if we get this far, assume it worked
}

bool Colony::CreateSpaceportPin(uint32 pinID, uint32 typeID, float latitude, float longitude) {
    Pin sp;
    sp.id = pinID;
    sp.typeID = typeID;
    sp.latitude = latitude;
    sp.longitude = longitude;
    sp.ownerID = charID;
    sp.state = STATE_IDLE; // ?
    sp.lastRunTime = 0L;
    sp.lastLaunchTime = 0L;
    sp.isLaunchable = true;
    ccPin.pins.push_back(sp);
    return true;    // if we get this far, assume it worked
}

bool Colony::CreateProcessPin(uint32 pinID, uint32 typeID, float latitude, float longitude) {
    Pin pp;
    pp.id = pinID;
    pp.typeID = typeID;
    pp.latitude = latitude;
    pp.longitude = longitude;
    pp.ownerID = charID;
    pp.state = STATE_IDLE; // ?
    pp.lastRunTime = 0L;
    pp.schematicID = 0;
    pp.hasRecievedInputs = 0;
    pp.recievedInputsLastCycle = 0;
    pp.isProcess = true;
    ccPin.pins.push_back(pp);
    return true;    // if we get this far, assume it worked
}

bool Colony::CreateExtractorPin(uint32 pinID, uint32 typeID, float latitude, float longitude) {
    Pin ep;
    ep.id = pinID;
    ep.typeID = typeID;
    ep.latitude = latitude;
    ep.longitude = longitude;
    ep.ownerID = charID;
    ep.state = STATE_IDLE; // ?
    ep.lastRunTime = 0L;
    ep.isExtractor = true;
    ccPin.pins.push_back(ep);
    return true;    // if we get this far, assume it worked
}

bool Colony::CreateLink(uint32 src, uint32 dest, uint32 level, bool ccConnected) {
    Link link;
    link.level = level;
    link.endpoint1 = src;
    link.endpoint2 = dest;
    link.typeID = 2280; // Only link type in the game.
    link.commandCenterConnected = ccConnected;
    ccPin.links.push_back(link);
    return true;
}
void Colony::UpgradeCommandCenter(uint32 pinID, uint32 level) {
    ccPin.level = level;
}

bool Colony::UpgradeLink(uint32 src, uint32 dest, uint32 level, bool ccConnected) {
    bool rtn = false;
    for (int i = 0; i < ccPin.links.size(); i++) {
        Link tmp = ccPin.links.front();
        ccPin.links.pop_front();
        if((tmp.endpoint1 == src || tmp.endpoint1 == dest) && (tmp.endpoint2 == src || tmp.endpoint2 == dest)) {
            rtn = true;
            tmp.level = level;
        }
        ccPin.links.push_back(tmp);
    }
    return rtn;}

bool Colony::RemovePin(uint32 pinID) {
    bool rtn = false;
    for(int i = 0;i < ccPin.pins.size();i++) {
        Pin tmp = ccPin.pins.front();
        ccPin.pins.pop_front();
        if(tmp.id != pinID)
            ccPin.pins.push_back(tmp);
        else
            rtn = true;
    }
    return rtn;
}

bool Colony::RemoveLink(uint32 src, uint32 dest, bool ccConnected) {
    bool rtn = false;
    for (int i = 0; i < ccPin.links.size(); i++) {
        Link tmp = ccPin.links.front();
        ccPin.links.pop_front();
        if((tmp.endpoint1 == src || tmp.endpoint1 == dest) && (tmp.endpoint2 == src || tmp.endpoint2 == dest)) {
            rtn = true;
            continue;
        }
        ccPin.links.push_back(tmp);
    }
    return rtn;
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

        if(i.isLaunchable) {
            dict->SetItem("lastLaunchTime", new PyLong(i.lastLaunchTime));
        }else if(i.isProcess) {
            dict->SetItem("schematicID", i.schematicID ? new PyInt(i.schematicID) : (PyRep*)new PyNone());
            dict->SetItem("hasRecievedInputs", new PyBool(i.hasRecievedInputs));
            dict->SetItem("recievedInputsLastCycle", new PyBool(i.recievedInputsLastCycle));
        }else if(i.isExtractor) {
            dict->SetItem("installTime", new PyLong(i.installTime));
            dict->SetItem("programType", new PyInt(i.programType));
            dict->SetItem("qtyPerCycle", new PyInt(i.qtyPerCycle));
            dict->SetItem("headRadius", new PyFloat(i.headRadius));
            dict->SetItem("expiryTime", new PyLong(i.expiryTime));
            dict->SetItem("cycleTime", new PyInt(i.cycleTime));
            dict->SetItem("heads", i.heads ? new PyInt(i.heads) : (PyRep*)new PyNone());
        }

        PyObject *obj = new PyObject("util.KeyVal", dict);
        pins->SetItem(index++, obj);
    }
    index = 0;

    for(auto i:ccPin.links) {
        PyDict *dict = new PyDict();

        if(i.commandCenterConnected) {
            dict->SetItem("endpoint1", new PyInt(i.endpoint1));
        }else {
            dict->SetItem("endpoint1", new_tuple(1, i.endpoint1));
        }
        dict->SetItem("endpoint2", new_tuple(1, i.endpoint2));
        dict->SetItem("level", new PyInt(i.level));
        dict->SetItem("typeID", new PyInt(i.typeID));

        PyObject *obj = new PyObject("util.KeyVal", dict);
        links->SetItem(index++, obj);
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
