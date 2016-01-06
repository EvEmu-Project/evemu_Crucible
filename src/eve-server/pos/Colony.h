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

#ifndef __COLONY_H_INCL__
#define __COLONY_H_INCL__

#include "PyCallable.h"

class Colony {

public:
    Colony(uint32 charID, uint32 planetID);
    bool CreateCommandPin(uint32 pinID, uint32 typeID, float latitude, float longitude);
    bool CreateSpaceportPin(uint32 pinID, uint32 typeID, float latitude, float longitude);
    bool CreateProcessPin(uint32 pinID, uint32 typeID, float latitude, float longitude);
    bool CreateExtractorPin(uint32 pinID, uint32 typeID, float latitude, float longitude);

    bool CreateLink(uint32 src, uint32 dest, uint32 level, bool ccConnected);

    void UpgradeCommandCenter(uint32 pinID, uint32 level);
    bool UpgradeLink(uint32 src, uint32 dest, uint32 level, bool ccConnected);

    bool RemovePin(uint32 pinID);
    bool RemoveLink(uint32 src, uint32 dest, bool ccConnected);

    PyResult GetColony();
private:
    struct Pin {
        uint32 id = 0;
        uint32 typeID = 0;
        uint32 ownerID = 0;
        float latitude = 0.0;
        float longitude = 0.0;
        long lastRunTime = 0L;
        uint32 state = 0;
        PyDict *contents = NULL;
        bool isCommandCenter = false;

        bool isLaunchable = false;
        // Command/Spaceport
        long lastLaunchTime = 0L;

        bool isProcess = false;
        // Process
        uint32 schematicID = 0;
        uint32 hasRecievedInputs = 0;
        uint32 recievedInputsLastCycle = 0;

        bool isExtractor = false;
        //Extractor
        uint32 heads = 0;
        uint32 programType = 0;
        uint32 cycleTime = 0;
        long expiryTime = 0L;
        uint32 qtyPerCycle = 0;
        float headRadius = 0.0;
        long installTime = 0;
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
