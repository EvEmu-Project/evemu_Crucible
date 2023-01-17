
 /**
  * @name PlanetMgr.cpp
  *   Specific Class for managing planet resources
  *         this is based on preliminary work by Comet0
  * @Author:         Allan
  * @date:   30 April 2016
  * @update:  4 November 2016  - began rewrite
  *          16 November 2016  - basic system working
  *          19 July 2019      - all systems working properly
  */


#include "eve-server.h"

#include "Client.h"

#include "account/AccountService.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "packets/Planet.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/PlanetDataMgr.h"
#include "system/SystemManager.h"


PlanetMgr::PlanetMgr(Client* pClient, PlanetSE* pPlanet, Colony* pColony) :
 m_client(pClient),
 m_colony(pColony),
m_planet(pPlanet)
{
}

PyRep* PlanetMgr::UpdateNetwork(PyList* commandList)
{
    using namespace PI;
    bool cancel = false;
    for (int i = 0; i < commandList->size(); ++i) {
        if (cancel)
            return m_colony->GetColony();
        UUNCommand uunc;
        if (!uunc.Decode(commandList->GetItem(i)->AsTuple())) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCommand");
            commandList->Dump(PLANET__WARNING, "      ");
            m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04508.");
            return nullptr;
        }
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork() - loop: %u, command: %s(%i)", i, sPlanetDataMgr.GetCommandName(uunc.command), uunc.command);
        switch (uunc.command) {
            case Command::CreatePin:                cancel = CreatePin(uunc);               break;
            case Command::RemovePin:                RemovePin(uunc);                        break;
            case Command::CreateLink:               CreateLink(uunc);                       break;
            case Command::RemoveLink:               RemoveLink(uunc);                       break;
            case Command::CreateRoute:              CreateRoute(uunc);                      break;
            case Command::SetLinkLevel:             SetLinkLevel(uunc);                     break;
            case Command::UpgradeCommandCenter:     cancel = UpgradeCommandCenter(uunc);    break;
            case Command::SetSchematic:             SetSchematic(uunc);                     break;
            case Command::RemoveRoute:              RemoveRoute(uunc);                      break;
            case Command::AddExtractorHead:         AddExtractorHead(uunc);                 break;
            case Command::MoveExtractorHead:        MoveExtractorHead(uunc);                break;
            case Command::InstallProgram:           InstallProgram(uunc);                   break;
            case Command::KillExtractorHead:        KillExtractorHead(uunc);                break;
            case Command::PrioritizeRoute:          PrioritizeRoute(uunc);                  break;
            default: {
                _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork() %s command %i", sPlanetDataMgr.GetCommandName(uunc.command), uunc.command);
            } break;
        }
    }

    m_colony->Save();

    return m_colony->GetColony();
}

bool PlanetMgr::UpgradeCommandCenter(UUNCommand& nc)
{
    // the return here is used to cancel loop in UpdateNetwork.  return false = continue

    int8 oldLevel = m_colony->GetLevel(), newLevel = (int8)PyRep::IntegerValue(nc.command_data->GetItem(1));
    int32 cost = 0;
    using namespace PI::Pin;
    while (oldLevel != newLevel) {
        //  calculate total upgrade cost in cases where upgrading multiple levels at once
        switch (oldLevel) {
            case Level0: cost += 580000; break;
            case Level1: cost += 930000; break;
            case Level2: cost += 1200000; break;
            case Level3: cost += 1500000; break;
            case Level4: cost += 2100000; break;
        }
        ++oldLevel;
    }
    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC:  Command Center upgrade on ";
    reason += m_planet->GetName();
    uint32 ownerID = corpCONCORD;
    if (m_planet->SystemMgr()->GetSystemSecurityRating() < 0.5)
        ownerID = corpInterbus;
    AccountService::TranserFunds(
                    m_client->GetCharacterID(),
                    ownerID,  // concord in empire, interbus otherwise
                    cost,
                    reason.c_str(),
                    Journal::EntryType::PlanetaryConstruction,
                    m_planet->GetID(),
                    Account::KeyType::Cash);

    m_colony->UpgradeCommandCenter(PyRep::IntegerValue(nc.command_data->GetItem(0)), newLevel);
    return false;
}

bool PlanetMgr::CreatePin(UUNCommand& nc)
{
    // the return here is used to break out of loop if needed.  return false = continue
    using namespace EVEDB::invGroups;
    uint32 typeID = PyRep::IntegerValueU32(nc.command_data->GetItem(1));
    uint32 groupID = sItemFactory.GetType(typeID)->groupID();
    switch (groupID) {
        case Command_Centers: {
            //take the money, send wallet blink event record the transaction in their journal.
            std::string reason = "DESC:  Command Center construction on ";
            reason += m_planet->GetName();
            uint32 ownerID = corpCONCORD;
            if (m_planet->SystemMgr()->GetSystemSecurityRating() < 0.5)
                ownerID = corpInterbus;
            AccountService::TranserFunds(
                        m_client->GetCharacterID(),
                        ownerID,  // concord in empire, interbus otherwise
                        90000,
                        reason.c_str(),
                        Journal::EntryType::PlanetaryConstruction,
                        m_planet->GetID(),
                        Account::KeyType::Cash);

            UUNCCommandCenter uunccc;
            if (!uunccc.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCCommandCenter");
                nc.command_data->Dump(PLANET__WARNING, "      ");
                m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04508.");
                return true;
            }
            m_colony->CreateCommandPin(uunccc.pinID, uunccc.typeID, uunccc.latitude, uunccc.longitude);
            if (m_planet->GetCustomsOffice() == nullptr)
                m_planet->CreateCustomsOffice();
            return false;
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases:{
            // Not Supported yet
            _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::CreatePin() Planet Bases (type/group %u/%u) not supported.", typeID, groupID);
            m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04109.");
            return true;
        } break;
    }
    uint32 cost = 0;
    std::string pinString = "";
    switch (groupID) {
        case Storage_Facilities: {
            cost = 250000;
            pinString = "Silo";
        } break;
        case Processors: {
            switch (typeID) {
                case 2469:   //   Lava Basic Industry Facility
                case 2471:   //   Plasma Basic Industry Facility
                case 2473:   //   Barren Basic Industry Facility
                case 2481:   //   Temperate Basic Industry Facility
                case 2483:   //   Storm Basic Industry Facility
                case 2490:   //   Oceanic Basic Industry Facility
                case 2492:   //   Gas Basic Industry Facility
                case 2493: { //   Ice Basic Industry Facility
                    cost = 75000;
                    pinString = "Basic Plant";
                } break;
                case 2470:   //   Lava Advanced Industry Facility
                case 2472:   //   Plasma Advanced Industry Facility
                case 2474:   //   Barren Advanced Industry Facility
                case 2480:   //   Temperate Advanced Industry Facility
                case 2484:   //   Storm Advanced Industry Facility
                case 2485:   //   Oceanic Advanced Industry Facility
                case 2491:   //   Ice Advanced Industry Facility
                case 2494: { //   Gas Advanced Industry Facility
                    cost = 250000;
                    pinString = "Advanced Plant";
                } break;
                case 2475:   //   Barren High-Tech Production Plant
                case 2482: { //   Temperate High-Tech Production Plant
                    cost = 525000;
                    pinString = "High-Tech Plant";
                } break;
            }
        } break;
        case Extractor_Control_Units: {
            cost = 45000;
            pinString = "ECU";
        } break;
        case Spaceports: {
            cost = 900000;
            pinString = "LaunchPad";
        } break;
        case Planetary_Links: {
            cost = 1000;
            pinString = "Link";
        } break;
        case Extractors: {
            cost = 100;
            pinString = "Extractor Head";
        } break;
    }

    if (cost < 1) {
        // error here....
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 02001.");
        return true;
    }

    UUNCStandardPin uuncsp;
    if (!uuncsp.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin");
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
        return true;
    }

    m_colony->CreatePin(groupID, uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude);

    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC:  ";
    reason += pinString.c_str();
    reason += " Construction on ";
    reason += m_planet->GetName();
    uint32 ownerID = corpCONCORD;
    if (m_planet->SystemMgr()->GetSystemSecurityRating() < 0.5)
        ownerID = corpInterbus;
    AccountService::TranserFunds(
                m_client->GetCharacterID(),
                ownerID,  // concord in empire, interbus otherwise
                cost,
                reason.c_str(),
                Journal::EntryType::PlanetaryConstruction,
                m_planet->GetID(),
                Account::KeyType::Cash);

    return false;
}

void PlanetMgr::CreateLink(UUNCommand& nc)
{
    uint32 src = 0, dest = 0, level = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        if (nc.command_data->GetItem(1)->IsInt()) {
            UUNCLinkExist uuncle;
            if (!uuncle.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkExist");
                nc.command_data->Dump(PLANET__WARNING, "      ");
                m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
                return;
            }
            src = uuncle.src;
            dest = uuncle.dest;
            level = uuncle.level;
        } else {
            UUNCLinkCommand uunclc;
            if (!uunclc.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkCommand");
                nc.command_data->Dump(PLANET__WARNING, "      ");
                m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
                return;
            }
            src = uunclc.src;
            dest = uunclc.dest2;
            level = uunclc.level;
        }
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        UUNCLinkStandard uuncls;
        if (!uuncls.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkStandard");
            nc.command_data->Dump(PLANET__WARNING, "      ");
            m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
            return;
        }
        src = uuncls.src2;
        dest = uuncls.dest2;
        level = uuncls.level;
    } else {
        //Invalid...
        _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::CreateLink() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04508.");
        return;
    }
    m_colony->CreateLink(src, dest, level);
}

void PlanetMgr::CreateRoute(UUNCommand& nc)
{
    Call_CreateRoute args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_CreateRoute");
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
        return;
    }

    PyIncRef(args.path);
    m_colony->CreateRoute(args.routeID, args.typeID, args.qty, args.path);
}

void PlanetMgr::RemovePin(UUNCommand& nc)
{
    uint32 pinID = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        pinID = PyRep::IntegerValue(nc.command_data->GetItem(0));
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        pinID = PyRep::IntegerValue(nc.command_data->GetItem(0)->AsTuple()->GetItem(1));
    } else {
        //Invalid...
        _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::RemovePin() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04508.");
        return;
    }
    m_colony->RemovePin(pinID);
}

void PlanetMgr::RemoveLink(UUNCommand& nc)
{
    _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::RemoveLink()");
    nc.Dump(PLANET__WARNING, "      ");

    m_colony->RemoveLink(PyRep::IntegerValue(nc.command_data->GetItem(0)), PyRep::IntegerValue(nc.command_data->GetItem(1)));
}

void PlanetMgr::RemoveRoute(UUNCommand& nc)
{
    uint32 routeID = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        routeID = PyRep::IntegerValue(nc.command_data->GetItem(0));
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        routeID = PyRep::IntegerValue(nc.command_data->GetItem(0)->AsTuple()->GetItem(1));
    } else {
        //Invalid...
        _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::RemoveRoute() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04508.");
        return;
    }
    m_colony->RemoveRoute(routeID);
}

void PlanetMgr::SetLinkLevel(UUNCommand& nc)
{
    m_colony->UpgradeLink(PyRep::IntegerValue(nc.command_data->GetItem(0)),
                          PyRep::IntegerValue(nc.command_data->GetItem(1)),
                          PyRep::IntegerValue(nc.command_data->GetItem(2)));
}

void PlanetMgr::SetSchematic(UUNCommand& nc)
{
    uint32 pinID = 0;
    if (nc.command_data->GetItem(0)->IsTuple()) {
        pinID = PyRep::IntegerValue(nc.command_data->GetItem(0)->AsTuple()->GetItem(1));
    } else if (nc.command_data->GetItem(0)->IsInt()) {
        pinID = PyRep::IntegerValue(nc.command_data->GetItem(0));
    }
    m_colony->SetSchematic(pinID, PyRep::IntegerValue(nc.command_data->GetItem(1))); // 65 - 137
}

void PlanetMgr::AddExtractorHead(UUNCommand& nc)
{
    Call_AddMoveExtractorHead args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_AddMoveExtractorHead");
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
        return;
    }

    m_colony->AddExtractorHead(args.ecuID, (uint16)args.headID, args.latitude, args.longitude);
}

void PlanetMgr::MoveExtractorHead(UUNCommand& nc)
{
    Call_AddMoveExtractorHead args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_AddMoveExtractorHead");
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
        return;
    }

    m_colony->MoveExtractorHead(args.ecuID, (uint16)args.headID, args.latitude, args.longitude);
}

void PlanetMgr::InstallProgram(UUNCommand& nc)
{
    Call_InstallProgram args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_InstallProgram");
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
        return;
    }

    m_colony->InstallProgram(args.ecuID, (uint16)args.typeID, args.headRadius, this);
}

void PlanetMgr::KillExtractorHead(UUNCommand& nc)
{
    m_colony->KillExtractorHead(PyRep::IntegerValue(nc.command_data->GetItem(0)), PyRep::IntegerValue(nc.command_data->GetItem(1)));
}

void PlanetMgr::PrioritizeRoute(UUNCommand& nc)
{
    _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::PrioritizeRoute()");
    nc.Dump(COLONY__PKT_TRACE, "      ");

    Call_PrioritizeRoute args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_PrioritizeRoute");
        nc.command_data->Dump(PLANET__WARNING, "      ");
        m_client->SendErrorMsg("Internal Server Error.  Ref: ServerError 04588.");
        return;
    }

    m_colony->PrioritizeRoute(args.routeID, args.priority);
}

/** @note   link distance formula from client
 *
 *    def GetDistance(self, otherInstallation):
 *        diffLong = self.longitude - otherInstallation.longitude
 *        cosDiffLong = math.cos(diffLong)
 *        cosMyLat = math.cos(self.latitude)
 *        sinMyLat = math.sin(self.latitude)
 *        cosOthLat = math.cos(otherInstallation.latitude)
 *        sinOthLat = math.sin(otherInstallation.latitude)
 *        nom1 = (cosMyLat + math.sin(diffLong)) ** 2
 *        nom2 = (cosMyLat * sinOthLat - sinMyLat * cosOthLat * cosDiffLong) ** 2
 *        denom = sinMyLat * sinOthLat + cosMyLat * cosOthLat * cosDiffLong
 *        return math.atan2(math.sqrt(nom1 + nom2), denom)
 */

/** @note CC Info
 *
commandCenterInfoPerLevel = {0: util.KeyVal(powerOutput=6000, cpuOutput=1675, upgradeCost=0),
 1: util.KeyVal(powerOutput=9000, cpuOutput=7057, upgradeCost=580000),
 2: util.KeyVal(powerOutput=12000, cpuOutput=12136, upgradeCost=1510000),
 3: util.KeyVal(powerOutput=15000, cpuOutput=17215, upgradeCost=2710000),
 4: util.KeyVal(powerOutput=17000, cpuOutput=21315, upgradeCost=4210000),
 5: util.KeyVal(powerOutput=19000, cpuOutput=25415, upgradeCost=6310000)}
 */

/** @note routing info (from client)
 *
    def FindShortestPath(self, sourcePin, destinationPin):
        """
            Simple shortest-path/undirected-graph thing.
            Uses a Dijkstra subfunction.
        """
        if sourcePin is None or destinationPin is None:
            return
        if sourcePin == destinationPin:
            return []
        distanceDict, predecessorDict = self.Dijkstra(sourcePin, destinationPin)
        if destinationPin not in distanceDict or destinationPin not in predecessorDict:
            return []
        path = []
        currentPin = destinationPin
        while currentPin is not None:
            path.append(currentPin.id)
            if currentPin is sourcePin:
                break
            if currentPin not in predecessorDict:
                raise RuntimeError("CurrentPin not in predecessor dict. There's no path. How did we get here?")
            currentPin = predecessorDict[currentPin]

        path.reverse()
        if path[0] != sourcePin.id:
            return []
        return path

    def FindShortestPathIDs(self, sourcePinID, destinationPinID):
        return self.FindShortestPath(self.GetPin(sourcePinID), self.GetPin(destinationPinID))

    def Dijkstra(self, sourcePin, destinationPin):
        """
        (from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/119466)
        Find shortest paths from the start vertex to all
        vertices nearer than or equal to the end.

        This particular implementation is adapted from the starMapSvc's
        copy of the algorithm, which is apparently unused.
        However, it matches up with the algorithm.
        """
        D = {}
        P = {}
        Q = planetCommon.priority_dict()
        Q[sourcePin] = 0.0
        while len(Q) > 0:
            vPin = Q.smallest()
            D[vPin] = Q[vPin]
            if vPin == destinationPin:
                break
            Q.pop_smallest()
            for wDestinationID in self.colonyData.GetLinksForPin(vPin.id):
                wLink = self.GetLink(vPin.id, wDestinationID)
                wPin = self.GetPin(wDestinationID)
                vwLength = D[vPin] + self._GetLinkWeight(wLink, wPin, vPin)
                if wPin in D:
                    if vwLength < D[wPin]:
                        raise ValueError, 'Dijkstra: found better path to already-final vertex'
                elif wPin not in Q or vwLength < Q[wPin]:
                    Q[wPin] = vwLength
                    P[wPin] = vPin

        return (D, P)
 */