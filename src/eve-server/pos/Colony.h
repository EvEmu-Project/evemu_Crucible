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

#ifndef __COLONY_H_INCL__
#define __COLONY_H_INCL__

#include "eve-server.h"
#include "PyCallable.h"

class Colony {

public:
    Colony(uint32 charID, uint32 planetID);
    void CreateCommandPin(uint32 pinID, uint32 typeID, float latitude, float longitude);

    PyResult GetColony();
private:
    struct Pin {
        uint32 id;
        uint32 typeID;
        uint32 ownerID;
        float latitude;
        float longitude;
        long lastRunTime;
        uint32 state;
        PyDict *contents;
        bool isCommandCenter;

        // Command/Spaceport
        long lastLaunchTime;

        // Process
        uint32 schematicID;
        uint32 hasRecievedInputs;
        uint32 recievedInputsLastCycle;

        //Extractor
        uint32 heads;
        uint32 programType;
        uint32 cycleTime;
        long expiryTime;
        uint32 qtyPerCycle;
        float headRadius;
        long installTime;
    };

    struct Link {
        uint32 typeID;
        uint32 endpoint1;
        uint32 endpoint2;
        uint32 level;
        bool commandCenterConnected;
    };

    struct Route {
        uint32 destID;
        PyList *path;
        uint32 comodityTypeID;
        uint32 commodityQuantity;
        bool destIsCommandCenter;
    };

    struct CommandCenterPin {
        uint32 level;
        long currentSimTime;
        std::list<Pin> pins;
        std::list<Link> links;
        std::list<Route> routes;
    };
    CommandCenterPin ccPin;

    const int STATE_EDITMODE = -2;
    const int STATE_DISABLED = -1;
    const int STATE_IDLE = 0;
    const int STATE_ACTIVE = 1;
protected:

};


#endif
