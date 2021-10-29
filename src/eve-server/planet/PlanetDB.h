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
    Author:        Allan
*/


#ifndef EVEMU_PLANET_PLANETDB_H_
#define EVEMU_PLANET_PLANETDB_H_

#include "../eve-server.h"
#include "../POD_containers.h"
//#include "../DataClasses.h"

class CommandCenterPin;

class PlanetDB
{
public:
    static PyRep* GetPlanetsForChar(uint32 charID);
    static PyRep* GetMyLaunchesDetails(uint32 charID);

    // must send data.launchID
    static void GetLaunchDetails(Launch::Data &data);
    static GPoint GetLaunchPos(uint32 launchID);
    static uint32 GetLaunchItemID(uint32 launchID);
    static uint32 GetLaunchPlanet(uint32 launchID);
    static void UpdateLaunchStatus(uint32 itemID, uint8 status);

    void GetPlanetData(DBQueryResult& res);
    void GetSchematicData(DBQueryResult& res);
    void GetSchematicTimes(DBQueryResult& res);

    void UpdatePins(uint32 pinID, PI_CCPin* ccPin);
    void UpdateECUPin(uint32 pinID, PI_CCPin* ccPin);
    void SavePins(PI_CCPin* ccPin); // this does NOT save contents, heads, or schematic data
    void SaveHeads(uint32 ccPinID, uint32 ownerID, uint32 ecuID, std::map< uint16, PI_Heads >& heads);
    void SaveLinks(PI_CCPin* ccPin);
    void SaveRoutes(PI_CCPin* ccPin);
    void SaveContents(PI_CCPin* ccPin);
    void SavePinContents(uint32 ccPinID, uint32 pinID, std::map< uint16, uint32 >& contents);
    void RemovePin(uint32 pinID);
    void RemoveHead(uint32 ecuID, uint32 headID);
    void RemoveLink(uint32 linkID);
    void RemoveRoute(uint16 routeID);
    void RemoveContents(uint32 pinID);
    void DeleteColony(uint32 ccPinID, uint32 planetID, uint32 charID);
    void LoadPins(uint32 ccPinID, std::map<uint32, PI_Pin>& pins);
    void LoadHeads(uint32 ecuID, std::map< uint16, PI_Heads >& heads);
    void LoadLinks(uint32 ccPinID, std::map<uint32, PI_Link>& links);
    void LoadRoutes(uint32 ccPinID, std::map< uint16, PI_Route >& routes);
    void LoadContents(uint32 pinID, std::map< uint16, uint32 >& contents);
    void SaveCCLevel(uint32 pinID, uint8 level);
    void SavePinLevel(uint32 pinID, uint8 level);
    void SaveLinkLevel(uint32 linkID, uint8 level);
    void SaveCommandCenter(uint32 pinID, uint32 charID, uint32 planetID, uint32 typeID, double latitude, double longitude);
    static void GetExtractorsForPlanet(uint32 planetID, DBQueryResult& res);

    void SaveLaunch(uint32 contID, uint32 charID, uint32 systemID, uint32 planetID, GPoint& pos);
    static void DeleteLaunch(uint32 contID);
    void UpdatePinTimes(PI_CCPin* ccPin);
    void AddPlanetForChar(uint32 solarSystemID, uint32 planetID, uint32 charID, uint32 ccPinID, uint16 typeID);
    void UpdatePlanetPins(uint32 ccPinID, uint8 pins=1);

    bool LoadColony(uint32 charID, uint32 planetID, PI_CCPin* ccPin);

    uint16 SaveRoute(uint32 ccPinID, PI_Route& route);
};

#endif  // EVEMU_PLANET_PLANETDB_H_
