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
    Author:     Zhur
    Updates:    Allan (rewrite), AlTahir(DaVinci)
*/

#include "EVE_Consts.h"
#include "EVE_Player.h"
#include "eve-server.h"
#include "../eve-common/EVEVersion.h"
#include "../eve-common/EVE_Character.h"
//#include "../../eve-common/EVE_Skills.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EVEServerConfig.h"
#include "LiveUpdateDB.h"

#include "StaticDataMgr.h"
#include "chat/LSCService.h"
#include "character/CharUnboundMgrService.h"
#include "corporation/CorporationDB.h"
#include "fleet/FleetService.h"
#include "imageserver/ImageServer.h"
#include "inventory/Inventory.h"
#include "map/MapData.h"
#include "map/MapDB.h"
#include "missions/MissionDataMgr.h"
#include "npc/NPC.h"
//#include "npc/Drone.h"
//#include "npc/DroneAI.h"
#include "station/StationDataMgr.h"
#include "station/StationOffice.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "exploration/Scan.h"
#include "station/Station.h"
#include "station/TradeService.h"
#include "pos/Tower.h"
#include "system/cosmicMgrs/WormholeMgr.h"

static const uint32 PING_INTERVAL_MS = 600000; //10m

Client::Client(EVEServiceManager& services, EVETCPConnection** con)
: EVEClientSession(con),
  m_TS(nullptr),
  m_char(CharacterRef(nullptr)),
  m_scan(nullptr),
  pShipSE(nullptr),
  pSession(new ClientSession()),
  m_system(nullptr),
  m_services(services),
  m_movePoint(NULL_ORIGIN),
  m_clientState(Player::State::Idle),
  m_stateTimer(0),
  m_ballparkTimer(0),
  m_pingTimer(PING_INTERVAL_MS),
  m_scanTimer(0),
  m_cloakTimer(0),
  m_fleetTimer(0),
  m_invulTimer(0),
  m_clientTimer(0),
  m_logoutTimer(0),
  m_jetcanTimer(0),
  m_sessionTimer(0),
  m_uncloakTimer(0),
  m_destinyEventQueue(new PyList()),
  m_destinyUpdateQueue(new PyList()),
  m_nextNotifySequence(0)
{
    m_pod = ShipItemRef(nullptr);
    m_ship = ShipItemRef(nullptr);

    m_systemData = SystemData();
    m_stationData = StationData();

    m_afk = false;
    m_login = true;
    m_invul = true;
    m_wing = false;
    m_fleet = false;
    m_squad = false;
    m_loaded = false;
    m_undock = false;
    m_showall = false;
    m_uncloak = false;
    m_beyonce = false;
    m_autoStop = false;
    m_canThrow = false;
    m_packaged = false;
    m_portrait = false;
    m_autoPilot = false;
    m_bubbleWait = false;     // allow client processing of subsquent destiny msgs
    m_charCreation = false;
    m_setStateSent = false;
    m_validSession = false;
    m_sessionChangeActive = false;

    //m_toGate = 0;
    m_locationID = 0;
    m_moveSystemID = 0;
    m_skillTimer = 0;
    m_dockStationID = 0;

    m_lpMap.clear();
    m_channels.clear();
    m_hangarLoaded.clear();

    this->m_lsc = m_services.Lookup <LSCService>("LSC");

    // Start handshake
    Reset();
}

Client::~Client() {
    if (!m_loaded)
        return;

    m_loaded = false;

    if (pShipSE != nullptr)
        WarpOut();      // need to make tests for this...it will segfault if m_char is invalid

    // LSC logout
    for (auto cur : m_channels)
        cur->LeaveChannel(this);

    if (m_char.get() != nullptr) {   // we have valid character
        /** @todo  - for warping to random point when client logs out in space...
         *      1)  check client IsInSpace(?)
         *      2)  set timer to delay removing bubble/sysmgr/destiny...or check based on destiny->isstopped() or timer on destiny->ismoving()
         *      3)  set current position (DB::chrCharacters.logoutPosition?)  initial code in place for warp-in on login
         *      4)  generate random point to warp to ** use sMapData.GetRandPointInSystem(systemID, distance)
         *      5)  _warp to random point, but DONT make/update new bubble with entering ship
         *      6)  remove client from sysmgr/destiny/server
         */

        sLog.Green("  Client::Logout()","%s (Acct:%u) logging out.", m_char->name(), GetUserID());

        // Save character's current position information
        if (sDataMgr.IsSolarSystem(m_locationID) && pShipSE != nullptr) {
            GPoint currentPosition = pShipSE->GetPosition();
            sLog.Blue("Client::Logout()", "Saving character position for %s(%u) at %.2f,%.2f,%.2f in system %u", 
                      m_char->name(), m_char->itemID(), currentPosition.x, currentPosition.y, currentPosition.z, m_locationID);
            
            // Update character data with position information
            m_char->SetLocation(m_locationID, m_systemData);
        }

        if (!sConsole.IsDbError()) {
            ServiceDB::SetAccountOnlineStatus(GetUserID(), false);
            CharacterDB::SetCharacterOnlineStatus(m_char->itemID(), false);
        }

        if (!sConsole.IsShutdown()) {
            if (IsDocked()) {
                if (GetTradeSession()) {
                    this->services().Lookup <TradeService>("trademgr")->CancelTrade(this);
                }
                CharNoLongerInStation();
                // remove char from station
                sEntityList.GetStationByID(m_locationID)->RemoveItem(m_char);
            }
        }
        // remove fleet data, remove char from ItemFactory cache, save SP and set logout time
        m_char->LogOut();
    }

    // save shipstate and remove from ItemFactory
    m_ship->LogOut();

    m_system->RemoveClient(this, true);
    // remove char from entitylist
    sEntityList.RemovePlayer(this);

    // reverse this so we destroy from newest to older
    // this prevents use after free
    auto cur = m_bindSet.rbegin ();
    auto end = m_bindSet.rend ();

    for (; cur != end; ++cur)
        m_services.ClearBoundObject(*cur, this);

    m_system = nullptr; // DO NOT delete m_system here

    SafeDelete(m_TS);
    SafeDelete(m_scan);
    SafeDelete(pShipSE);
    SafeDelete(pSession);
    PyDecRef(m_destinyEventQueue);
    PyDecRef(m_destinyUpdateQueue);
}

bool Client::ProcessNet()
{
    state_t state = GetState();
    if (state != TCPConnection::STATE_CONNECTED)
        return false;

    PyPacket *p(nullptr);
    while ((p = PopPacket())) {
        try {
            if (!DispatchPacket(p))
                sLog.Error("Client", "%s: Failed to dispatch packet of type %s (%i).", m_char->name(), MACHONETMSG_TYPE_NAMES[ p->type ], (int)p->type);
        }
        catch(PyException& e) {
            _SendException(p->dest, p->source.callID, p->type, WRAPPEDEXCEPTION, &e.ssException);
        }

        p = nullptr;
    }

    // cleanup
    SafeDelete(p);
    // send queue
    _SendQueuedUpdates();

    return true;
}

bool Client::SelectCharacter(int32 charID/*0*/)
{
    if (sEntityList.IsOnline(charID)) {
        sLog.Error("Client::SelectCharacter()", "Char %u already online.", charID);
        SendErrorMsg("That Character is already online.  Selection Failed.");
        CloseClientConnection();
        return false;
    }

    InitSession(charID);
    if (!m_validSession){
        sLog.Error("Client::SelectCharacter()", "Failed to init session for char %u.", charID);
        SendErrorMsg("Unable to Initalize Character session.  Selection Failed.");
        CloseClientConnection();
        return false;
    }

    sEntityList.AddPlayer(this);
    sItemFactory.SetUsingClient(this);

    m_system = sEntityList.FindOrBootSystem(m_systemData.systemID);
    if (m_system == nullptr) {
        sLog.Error("Client::SelectCharacter()", "Failed to boot system %u for char %u.", m_systemData.systemID, charID);
        SendErrorMsg("SolarSystem %s(%u) - Boot Failure.", m_systemData.name.c_str(), m_systemData.systemID);
        CloseClientConnection();
        return false;
    }

    m_char = sItemFactory.GetCharacterRef(charID);
    if (m_char.get() == nullptr) {
        sLog.Error("Client::SelectCharacter()", "GetChar for %u = nullptr", charID);
        SendErrorMsg("Unable to locate Character.  Selection Failed.");
        sItemFactory.UnsetUsingClient();
        CloseClientConnection();
        return false;
    }

    m_char->VerifySP();
    m_char->SetLoginTime();
    m_char->SetClient(this);
    m_char->SkillQueueLoop(false);

    // register with our system manager AFTER character is constructed and initialized
    m_system->AddClient(this, true);

    // this will eventually check for d/c timer and rejoin existing fleet if applicable
    //  fleet data is zeroed when char item is created

    SetPodItem();

    m_ship = sItemFactory.GetShipRef(m_shipId);
    if (m_ship.get() == nullptr) {
        sLog.Error("Client::SelectCharacter()", "shipID %u invalid for %u.  Selecting new ship...", m_shipId, charID);
        PickAlternateShip();    // incase shipID wasnt set correctly in db (seen on 'bad' Damage::Killed())
        m_ship = sItemFactory.GetShipRef(m_shipId);
        if (m_ship.get() == nullptr) {
            sLog.Error("Client::SelectCharacter()", "shipID %u for %u also invalid.  Loading Pod.", m_shipId, charID);
            m_ship = m_pod;
        }
        SetShip(m_ship);
    }

    m_ship->SetPlayer(this);

    GPoint pos(NULL_ORIGIN);

    if (sDataMgr.IsSolarSystem(m_locationID)) {
        pos = m_ship->position();

        m_loginWarpPoint = pos;
        m_loginWarpRandomPoint = m_ship->position();
        m_loginWarpRandomPoint.MakeRandomPointOnSphere(0.5*ONE_AU_IN_METERS);

        MoveToLocation(m_locationID, m_loginWarpRandomPoint);

        // Cloak the player and uncloak them as soon as they start the login
        // warp.
        pShipSE->DestinyMgr()->Cloak();
    } else {
        MoveToLocation(m_locationID, pos);
        if (m_ship->typeID() == itemTypeCapsule) {
            if (sConfig.server.NoobShipCheck) {
                StationItemRef sRef = m_system->GetStationFromInventory(m_locationID);
                if (sRef.get() == nullptr) {
                    // error here...
                } else if (!sRef->HasShip(this)) {
                    SpawnNewRookieShip(m_locationID);
                }
            } else {
                SpawnNewRookieShip(m_locationID);
            }
        }
    }

    //create corp and ally chat channels (if not already created)
    this->m_lsc->CharacterLogin(this);

    // load char LPs
    //m_lpMap

    // update account online status, increase login count, set last login timestamp
    ServiceDB::IncrementLoginCount(GetUserID());
    ServiceDB::SetAccountOnlineStatus(GetUserID(), true);

    //johnsus - characterOnline mod
    CharacterDB::SetCharacterOnlineStatus(charID, true);
    sItemFactory.UnsetUsingClient();

    // if applicable, the login warp gets triggered here
    SetStateTimer(Player::State::Login, Player::Timer::Login);
    SetInvulTimer(Player::Timer::WarpInInvul);
    //SetCloakTimer(Player::Timer::LoginCloak);

    // set ship cap and shields to full
    m_ship->SetShipShield(1.0);
    m_ship->SetShipCapacitorLevel(1.0);

    // send MOTD and server data to 'local' chat channel
    this->m_lsc->SendServerMOTD(this);

    return (m_loaded = true);
}

void Client::ProcessClient() {
    if (m_charCreation)
        return;

    double profileStartTime(GetTimeUSeconds());

    // wtf is this for?
    if (m_pingTimer.Check()) {
        _SendPingRequest();  //10m
        m_char->SetLogonMinutes();
    }

    if (m_skillTimer > 0)
        if (m_skillTimer < GetFileTimeNow())
            m_char->SkillQueueLoop();

    if (m_sessionTimer.Check(false)) {
        _log(CLIENT__TIMER, "Client::ProcessClient():  SetSessionChange to false for %s(%u)", m_char->name(), m_char->itemID());
        m_sessionTimer.Disable();
        m_sessionChangeActive = false;
    }

    /* Check Character Save Timer Expiry:  (not currently used  -allan 17May16)
    if (m_char->CheckSaveTimer()) {
        _log(CLIENT__TIMER, "Client::ProcessClient():  SaveTimer for %s(%u)", m_char->name(), m_char->itemID());
        m_char->SaveCharacter();
        m_ship->SaveShip();
    }
    */
    if (sDataMgr.IsStation(m_locationID)) {
        if (m_stateTimer.Enabled())
            if (m_stateTimer.Check(false)) {
                m_stateTimer.Disable();
                switch (m_clientState) {
                    case Player::State::Login: {
                        _log(CLIENT__TIMER, "ProcessClient()::IsDocked()::CheckState():  case: Login");
                        m_login = false;
                    } break;
                    case Player::State::Idle: {
                        _log(CLIENT__TIMER, "ProcessClient()::IsDocked()::CheckState():  case: Idle");
                    } break;
                    case Player::State::Logout: {
                        _log(CLIENT__TIMER, "ProcessClient()::IsDocked()::CheckState():  case: Logout");
                    } break;
                    case Player::State::Killed: {
                        _log(CLIENT__TIMER, "ProcessClient()::IsDocked()::CheckState():  case: Killed");
                    } break;
                }
                m_clientState = Player::State::Idle;
                _log(AUTOPILOT__TRACE, "ProcessClient()::IsDocked() - m_clientState set to Idle");
            }
        if (sConfig.debug.UseProfiling)
            sProfiler.AddTime(Profile::client, GetTimeUSeconds() - profileStartTime);
        return;
    }

    if (pShipSE == nullptr) {
        sLog.Error("ProcessClient()","%s: InSpace with no shipSE.  LocationID %u", m_char->name(), m_locationID);
        return;
    }

    if (m_invulTimer.Enabled()/*m_invul*/)
        if (m_invulTimer.Check(false)) {
            _log(CLIENT__TIMER, "ProcessClient():  SetInvul to false for %s(%u)", m_char->name(), m_char->itemID());
            m_invulTimer.Disable();
            m_invul = false;
            m_undock = false;
        }

    if (m_scanTimer.Enabled())
        if (m_scanTimer.Check(false)) {
            _log(CLIENT__TIMER, "ProcessClient():  Scan Timer hit for %s(%u).", m_char->name(), m_char->itemID());
            m_scanTimer.Disable();
            m_scan->ProcessScan(m_scanProbe);
        }

    if (m_ballparkTimer.Enabled())
        if (m_ballparkTimer.Check(false)) {
            _log(CLIENT__TIMER, "ProcessClient():  Ballpark Timer hit for %s(%u).", m_char->name(), m_char->itemID());
            m_ballparkTimer.Disable();
            SetBallPark();
        }

    if (pShipSE->DestinyMgr()->IsCloaked())
        if (m_cloakTimer.Check(false)) {
            _log(CLIENT__TIMER, "ProcessClient():  SetCloak to false for %s(%u)", m_char->name(), m_char->itemID());
            m_cloakTimer.Disable();
            pShipSE->DestinyMgr()->UnCloak();
            //m_clientState = Player::State::Idle;
            //_log(AUTOPILOT__TRACE, "ProcessClient() - Cloaked - m_clientState set to Idle");
        }

    if (m_uncloak)
        if (m_uncloakTimer.Check(false)) {
            _log(CLIENT__TIMER, "ProcessClient():  SetUncloak to false for %s(%u)", m_char->name(), m_char->itemID());
            m_uncloakTimer.Disable();
            m_uncloak = false;
        }

    if (m_stateTimer.Enabled())
        if (m_stateTimer.Check(false)) {
            _log(CLIENT__TIMER, "ProcessClient(): state timer hit.  current state time is %ums", m_stateTimer.GetCurrentTime());
            m_stateTimer.Disable();
            switch (m_clientState) {
                case Player::State::Idle: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Idle");
                    // this shouldnt hit...error
                } break;
                case Player::State::Dock: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Dock");
                    DockToStation();
                } break;
                case Player::State::Undock: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Undock");
                    pShipSE->DestinyMgr()->Undock(m_movePoint);
                    SetBallPark();
                    m_clientState = Player::State::Idle;
                } break;
                case Player::State::Killed: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Killed");
                    // live does NOT resend destiny state when killed.  see csBoard notes.
                    SendSessionChange();
                    m_clientState = Player::State::Idle;
                } break;
                case Player::State::Board: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Board");
                    // calling OnSessionChanged() in client with shipid in change will update ego with new ship.
                    // NOTE: all items must be in same bubble.
                    SendSessionChange();
                    m_clientState = Player::State::Idle;
                } break;
                case Player::State::Login: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Login");
                    m_login = false;
                    SetBallPark();
                    if (sDataMgr.IsSolarSystem(m_locationID)) {
                        WarpIn();
                    }
                    m_ship->GetModuleManager()->UpdateChargeQty();  //  <<<< huge hack here....cant find another way to do it yet.
                    } break;
                case Player::State::LoginWarp: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: LoginWarp");
                    pShipSE->DestinyMgr()->UnCloak();
                    pShipSE->DestinyMgr()->WarpTo(m_loginWarpPoint);
                    } break;
                case Player::State::Jump: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Jump");
                    ExecuteJump();
                } break;
                case Player::State::DriveJump: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: DriveJump");
                    ExecuteDriveJump();
                } break;
                case Player::State::WormholeJump: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: WormholeJump");
                    ExecuteWormholeJump();
                } break;
                case Player::State::Logout: {
                    _log(CLIENT__TIMER, "ProcessClient()::CheckState():  case: Logout");
                    // can we use this to allow WarpOut?
                } break;
                default: {
                    sLog.Error("ProcessClient()","%s: State timer expired with invalid state: %s.", m_char->name(), GetStateName(m_clientState).c_str());
                    //SendErrorMsg("Server Error - Move not initalized properly.  You may need to relog.  Ref: ServerError 10928");
                } break;
            }
        }

    // only set for location change
    if (m_fleetTimer.Enabled())
        if (m_fleetTimer.Check(false)) {
            m_fleetTimer.Disable();
            BoostData bData = BoostData();
            if (IsSquadID(m_squad)) {
                SquadData sData = SquadData();
                sFltSvc.GetSquadData(m_squad, sData);
                if ((sData.leader != nullptr) and (sData.booster != nullptr))
                    if ((sData.leader->IsInSpace()) and (sData.booster->IsInSpace()))
                        bData = sData.boost;
            } else if (IsWingID(m_wing)) {
                WingData wData = WingData();
                sFltSvc.GetWingData(m_wing, wData);
                if ((wData.leader != nullptr) and (wData.booster != nullptr))
                    if ((wData.leader->IsInSpace()) and (wData.booster->IsInSpace()))
                        bData = wData.boost;
            } else if (IsFleetID(m_fleet)) {
                FleetData fData = FleetData();
                sFltSvc.GetFleetData(m_fleet, fData);
                if ((fData.leader != nullptr) and (fData.booster != nullptr))
                    if ((fData.leader->IsInSpace()) and (fData.booster->IsInSpace())) {
                        CharacterRef bRef = fData.booster->GetChar();
                        bData.leader    = fData.leader->GetChar()->GetSkillLevel(EvESkill::Leadership);
                        bData.armored   = bRef->GetSkillLevel(EvESkill::ArmoredWarfare);
                        bData.info      = bRef->GetSkillLevel(EvESkill::InformationWarfare);
                        bData.mining    = bRef->GetSkillLevel(EvESkill::MiningForeman);
                        bData.siege     = bRef->GetSkillLevel(EvESkill::SiegeWarfare);
                        bData.skirmish  = bRef->GetSkillLevel(EvESkill::SkirmishWarfare);
                    }
            }
            pShipSE->ApplyBoost(bData);
        }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::client, GetTimeUSeconds() - profileStartTime);
}

// `UpdateBubble` is a reusable function that synchronizes the player's position
// and ensures that their bubble exists. Do not call this if the player is
// docked.
void Client::UpdateBubble() {
    if (GetShipSE()->DestinyMgr() == nullptr) {
        SetDestiny(NULL_ORIGIN);
    }

    if (GetShipSE()->SysBubble() == nullptr) {
        EnterSystem(GetSystemID());
    }

    GetShipSE()->DestinyMgr()->SetPosition(GetShipSE()->GetPosition(), true);

    SystemBubble *pBubble = GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(GetShipSE());
        pBubble = GetShipSE()->SysBubble();
    }

    pBubble->SendAddBalls(GetShipSE());

    SetStateSent(false);
    GetShipSE()->DestinyMgr()->SendSetState();
    // SetSessionTimer(); // TODO: leaving here for reference, but probably isn't needed in the future
}

// `WarpIn` is executed upon player login, but should only be executed if the
// player was in space when they last logged out.
//
// The login warp-in process is a multi-step process. First, the player's ship
// is immediately moved to a position around 0.5 AU away from their logged-out
// position.
//
// However, merely setting the position isn't enough. The client needs to
// subsequently synchronize the state of the player & the surrounding bubble
// with the server, so `UpdateBubble` is called. This is important because
// during the process of establishing the warp vector, the player's ship is
// going to be aligning to warp for a few seconds, and so the client needs to
// know what's in its current bubble for that duration.
//
// During the development of `WarpIn()`, it was observed that the behavior of
// the ship was inconsistent if `Destiny->WarpTo()` was called immediately after
// the above position change and `UpdateBubble` calls. So instead, the session
// state timer is set to 0, and the player's client state is set to `LoginWarp`,
// which allows the warp to get processed on the next server tick.
//
// See: `Client::IsLoginWarping`
//
// See: `Client::SetLoginWarpComplete`
void Client::WarpIn() {
    sLog.Blue("Client::WarpIn()", "%s(%u) called WarpIn().", GetName(), m_char->itemID());
    char ci[45];
    snprintf(ci, sizeof(ci), "InSpace: %s(%u)", GetName(), m_char->itemID());
    m_ship->SetCustomInfo(ci);
    if (!InPod()) {
        m_ship->SetFlag(flagNone);
    }

    UpdateBubble();

    // Only trigger login warp if the saved position and current position are significantly different
    if ((m_ship->position() - m_loginWarpPoint).length() > 1.0) {
        SetStateTimer(0);
        m_clientState = Player::State::LoginWarp;
    } else {
        // If already at the correct position, skip warp and go idle
        m_clientState = Player::State::Idle;
    }
}

void Client::WarpOut() {
    sLog.Blue("Client::WarpOut()", "Client Destructor for %s(%u) called WarpOut().  Finish code here.", GetName(), m_char->itemID());
    char ci[45];
    snprintf(ci, sizeof(ci), "Logout: %s(%u)", GetName(), m_char->itemID());
    m_ship->SetCustomInfo(ci);
    if (!InPod())
        m_ship->SetFlag(flagShipOffline);

    // Critical fix: Save the latest ship position to database
    if (pShipSE != nullptr) {
        GPoint currentPosition = pShipSE->GetPosition();
        sLog.Blue("Client::WarpOut()", "Saving ship position for %s(%u) at %.2f,%.2f,%.2f", 
                  GetName(), m_char->itemID(), currentPosition.x, currentPosition.y, currentPosition.z);
        m_ship->SetPosition(currentPosition);
        m_ship->SaveShip();
    }

    pShipSE->SetPosition(m_ship->position());
    DestroyShipSE();
    return;
    /*
    SetInvulTimer(Player::Timer::WarpOutInvul);
    // We are logging out, so we need to warp to a random spot 1Mm away:
    GPoint warpToPoint(m_ship->position());
    warpToPoint.MakeRandomPointOnSphere(0.5*ONE_AU_IN_METERS);
    if (sConsole.IsShutdown())      // if server is being shutdown, set ship to WarpOut point, as if they warped there.
        pShipSE->SetPosition(warpToPoint);
    else
        pShipSE->DestinyMgr()->WarpTo(warpToPoint);
    */
}

void Client::SetAutoPilot(bool set/*false*/)
{
    // itemID=10644  flag=*module*  not published - not in client data
    if (m_autoPilot == set)
        return;

    m_autoPilot = set;
    _log(AUTOPILOT__MESSAGE, "%s called SetAutoPilot to %s", GetName(), (set ? "true" : "false"));
}

void Client::EnterSystem(uint32 systemID)
{
    MoveToLocation(systemID, m_ship->position());
}

void Client::MoveToLocation(uint32 locationID, const GPoint& pt) {
    // process ALL location changes here.
    if (!sDataMgr.IsStation(locationID) and !sDataMgr.IsSolarSystem(locationID)) {
        SendErrorMsg("Move requested to unsupported location %u", locationID);
        return;
    }

    _log(AUTOPILOT__TRACE, "MoveToLocation() - m_autoPilot = %s", (m_autoPilot ? "true" : "false"));

    if (!m_login and (m_locationID == locationID) and !sDataMgr.IsStation(locationID)) {
        _log(PLAYER__WARNING, "MoveToLocation() - m_locationID == location");
        // This is a simple movement
        SetDestiny(pt, true);
        return;
    }

    bool count(m_login);
    bool wasDocked(sDataMgr.IsStation(m_locationID));
    m_locationID = locationID;
    // get data for new system.  this checks for stationID sent as locationID, so is safe here.
    sDataMgr.GetSystemData(m_locationID, m_systemData);

    m_bubbleWait = false;           // allow client processing of subsequent destiny msgs

    // location changed...verify current system and set session data for current system.
    if (IsJump() or ((m_system != nullptr) and (m_system->GetID() != m_systemData.systemID))) {
        //we have different m_system
        _log(PLAYER__WARNING, "MoveToLocation() - current m_system is %s, systemData is for %s, m_system->GetID(%u) != locationID(%u)", \
                    m_system->GetName(), m_systemData.name.c_str(), m_system->GetID(), m_locationID);
        // if docked, update guestlist
        if (wasDocked) {
            CharNoLongerInStation();
            wasDocked = false;  // dont update station again on this call (redundant check later in this method)
        }
        if (pShipSE != nullptr)
            if ((IsJump()) and !m_autoPilot)
                pShipSE->DestinyMgr()->Halt();

        // remove from current system before resetting system vars
        m_system->RemoveEntity(pShipSE);

        m_system->RemoveClient(this, (count = true), IsJump());

        m_system = nullptr;
    }

    if (m_system == nullptr) {
        _log(PLAYER__WARNING, "MoveToLocation() - m_system == NULL, m_locationID = %u", m_locationID);
        // find our new system's manager
        sItemFactory.SetUsingClient(this);
        m_system = sEntityList.FindOrBootSystem(m_systemData.systemID);
        sItemFactory.UnsetUsingClient();
        if (m_system == nullptr) {
            sLog.Error("Client", "Failed to boot system %u for char %s (%u)", m_systemData.systemID, m_char->name(), m_char->itemID());
            SendErrorMsg("Unable to boot system.  Relog and try again.");
            return;
        }

        m_beyonce = false;
        m_setStateSent = false;

        // register ourselves with new system manager (this wont hit on login)
        m_system->AddClient(this, count, IsJump());
    }

    if (sDataMgr.IsStation(m_locationID)) {
        _log(PLAYER__WARNING, "MoveToLocation() - Character %s (%u) Docked in %u.", m_char->name(), m_char->itemID(), m_locationID);
        stDataMgr.GetStationData(m_locationID, m_stationData);
        char ci[45];
        snprintf(ci, sizeof(ci), "Docked: %s(%u)", GetName(), m_char->itemID());
        m_ship->SetCustomInfo(ci);
        m_char->Move(m_locationID, flagNone, true);
        m_ship->Move(m_locationID, flagHangar, true);

        if (IsFleetID(m_fleet)) {
            m_fleetTimer.Disable();
            if (IsFleetBooster()) {
                std::list<int32> wing, squad;
                wing.clear();
                squad.clear();
                if (IsSquadID(m_squad)) {
                    squad.emplace(squad.end(), m_squad);
                } else if (IsWingID(m_wing)) {
                    wing.emplace(wing.end(), m_wing);
                }
                sFltSvc.UpdateBoost(m_fleet, IsFleetBoss(), wing, squad);
            }
        }

        if (!IsHangarLoaded(m_locationID))
            LoadStationHangar(m_locationID);

        CharNowInStation();
        DestroyShipSE();
        StationItemRef sRef = sEntityList.GetStationByID(m_locationID);
        if (sRef.get() != nullptr) {
            sRef->LoadStationOffice(GetCorporationID());
            sRef->AddGuest(this);
        }
        m_bubbleWait = true;     // deny client processing of subsquent destiny msgs
    } else {
        _log(PLAYER__WARNING, "MoveToLocation() - Character %s(%u) InSpace in %u. (setState %s, beyonce %s)", \
        m_char->name(), m_char->itemID(), m_locationID, m_setStateSent ? "true" : "false", m_beyonce ? "true" : "false");
        char ci[45];
        snprintf(ci, sizeof(ci), "InSpace: %s(%u)", GetName(), m_char->itemID());
        m_ship->SetCustomInfo(ci);

        // if docked, update guestlist
        if (wasDocked and m_undock)
            CharNoLongerInStation();

        if (IsFleetID(m_fleet)) {
            m_fleetTimer.Start(Player::Timer::Fleet);
            if (IsFleetBooster()) {
                std::list<int32> wing, squad;
                wing.clear();
                squad.clear();
                if (IsSquadID(m_squad)) {
                    squad.emplace(squad.end(), m_squad);
                } else if (IsWingID(m_wing)) {
                    wing.emplace(wing.end(), m_wing);
                }
                sFltSvc.UpdateBoost(m_fleet, IsFleetBoss(), wing, squad);
            }
        }

        if (InPod()) {
            m_ship->Move(m_locationID, flagCapsule, true);
        } else {
            m_pod->Move(m_systemData.systemID, flagCapsule, false);
            m_ship->Move(m_locationID, flagNone, true);
        }

        if (m_char->flag() != flagPilot)
            m_char->Move(m_shipId, flagPilot, true);

        if (pShipSE != nullptr)
            pShipSE->ResetShipSystemMgr(m_system);

        // once systemData.radius implemented, remove this in favor of below check
        m_ship->SetPosition(pt);

        /* comment this block for later use...
         * m_systemData.radius is not populated yet, and this does weird things with ships
        // verify 'pt' is within system boundaries
        if (pt.length() < m_systemData.radius) {
            m_ship->SetPosition(pt);
        } else {
            ;  // oob
        }
        */

        SetDestiny(pt);

        if (IsJump() and !m_autoPilot)
            pShipSE->DestinyMgr()->Stop();
    }

    if (!m_login)
        m_ship->SaveShip(); // this saves everything on ship

    m_char->SetLocation((sDataMgr.IsStation(m_locationID) ? m_locationID : 0), m_systemData);

    UpdateSession();
    SendSessionChange();
}

void Client::SetDestiny(const GPoint& pt, bool update/*false*/) {
    if (!sDataMgr.IsSolarSystem(m_locationID)) {
        _log(CLIENT__ERROR, "%s(%u) - Calling SetDestiny() when not in space.", GetName(), m_char->itemID());
        return;
    }
    m_bubbleWait = false;        // allow client processing of subsquent destiny msgs
    m_setStateSent = false;

    bool updateShip = false;
    if (pShipSE == nullptr) {
        updateShip = true;
        CreateShipSE();
    }

    if (pShipSE->SystemMgr()->GetID() != m_system->GetID())
        _log(CLIENT__ERROR, "%s(%u) - Ship SysID of %u != Client SysID of %u.", GetName(), m_char->itemID(), pShipSE->SystemMgr()->GetID(), m_system->GetID());

    _log(PLAYER__AP_TRACE, "Client::SetDestiny():  shipSystemID: %u, SystemID: %u, update: %s, updateShip: %s, jump: %s, cloak: %s", \
            pShipSE->SystemMgr()->GetID(), m_system->GetID(), update?"true":"false", \
            updateShip?"true":"false", IsJump()?"true":"false", pShipSE->DestinyMgr()->IsCloaked()?"true":"false");

    if (pt.isZero()) {
        if (pShipSE->GetPosition().isZero()) {
            pShipSE->DestinyMgr()->SetPosition(sMapData.GetRandPointOnPlanet(m_system->GetID()), update);
        } else {
            pShipSE->DestinyMgr()->SetPosition(pShipSE->GetPosition(), update);
        }
    } else {
        pShipSE->DestinyMgr()->SetPosition(pt, update);
    }

    //if (m_login)
    //    pShipSE->DestinyMgr()->SetCloak(true);

    m_system->AddEntity(pShipSE);

    if (updateShip)
        UpdateNewShip();
}

void Client::SetBallPark() {
    _log(PLAYER__AP_TRACE, "Client::SetBallPark():  State: %s, SetState: %s, Beyonce: %s", \
                GetStateName(m_clientState).c_str(), m_setStateSent?"true":"false", m_beyonce?"true":"false");
    m_bubbleWait = false;   // allow client processing of subsequent destiny msgs
    if (pShipSE->SysBubble() == nullptr)
        m_system->AddEntity(pShipSE);
    if (!m_beyonce and !m_login and !m_undock) {
        m_bubbleWait = true;    // wait on proc destiny msgs
        CheckBallparkTimer();
        SetBallParkTimer(Player::Timer::Default);    // set timer 1s to wait for beyonce
        return;
    }
    if (!m_setStateSent and m_beyonce) {  // MUST have beyonce before sending state data.
        pShipSE->DestinyMgr()->SendSetState();
        m_ballparkTimer.Disable();
        if (IsGateJump()) {
            SetInvulTimer(Player::Timer::JumpInvul);
            // dont use timer method here...(jumping ship will flash at destination)
            m_cloakTimer.Start(Player::Timer::JumpCloak);
            m_clientState = Player::State::Idle;
        }
        if (IsDriveJump()) {
            SetInvulTimer(Player::Timer::JumpInvul);
            m_clientState = Player::State::Idle;
            JumpInEffect();
        }
        if (IsWormholeJump()) {
            SetInvulTimer(Player::Timer::JumpInvul);
            m_cloakTimer.Start(Player::Timer::JumpCloak);
            m_clientState = Player::State::Idle;
        }
    }
    if (m_undock)
        pShipSE->DestinyMgr()->SetSpeedFraction();
}

void Client::CheckBallparkTimer() {
    if (!m_ballparkTimer.Enabled()) {
        sLog.Error("CheckBallparkTimer()", "BallPark Timer is disabled.");
    } else {
        sLog.Warning("CheckBallparkTimer()", "BallPark Time remaining %ums", m_ballparkTimer.GetRemainingTime());
    }

    _log(CLIENT__TIMER, "CheckBallparkTimer():  State: %s, SetState: %s, Beyonce: %s, Login: %s", \
            GetStateName(m_clientState).c_str(), m_setStateSent?"true":"false", \
            m_beyonce?"true":"false", m_login?"true":"false");
    _log(CLIENT__TIMER, "CheckBallparkTimer():  invul: %s, cloak: %s, bubblewait: %s", m_invul?"true":"false", \
        pShipSE == nullptr? "ship null": pShipSE->DestinyMgr() == nullptr? "destiny null": pShipSE->DestinyMgr()->IsCloaked()?"true":"false", \
        m_bubbleWait?"true":"false");
}

void Client::MoveToPosition(const GPoint &pt) {
    if ((pShipSE == nullptr) or (pShipSE->DestinyMgr() == nullptr)) {
        CreateShipSE();
        pShipSE->SetPilot(this);
        m_system->AddEntity(pShipSE);
        UpdateNewShip();
    }
    pShipSE->DestinyMgr()->SetPosition(pt, true);
    if (m_undock)
        return;
    if (pShipSE->DestinyMgr()->IsMoving())
        pShipSE->DestinyMgr()->Halt();
}

void Client::DockToStation() {
    pShipSE->Dock();
    // ap cleared on client side when docking.
    m_autoPilot = false;
    m_setStateSent = false;
    m_clientState = Player::State::Idle;
    _log(AUTOPILOT__TRACE, "DockToStation()() - m_clientState set to Idle");
    pShipSE->DestinyMgr()->DockingAccepted();
    m_bubbleWait = true;     // deny client processing of subsquent destiny msgs

    //Check if player is in pod and have no ships in hangar, in which case they get a rookie ship for free
    //  on live, SCC sends mail about the loss of the players ship, and offers a shiny, new, fully-fitted ship as replacement.  we dont....yet
    // this needs to be done before player is docked
    if (m_ship->typeID() == itemTypeCapsule) {
        if (sConfig.server.NoobShipCheck) {
            StationItemRef sRef = m_system->GetStationFromInventory(m_dockStationID);
            if (sRef.get() == nullptr) {
                _log(CLIENT__ERROR, "%s(%u): DockToStation() - Station %u not found in inventory of %s(%u).", \
                        GetName(), m_char->itemID(), m_dockStationID, m_system->GetName(), m_system->GetID());
            } else if (!sRef->HasShip(this)) {   // need to get hangar items (flagHangar) by owner
                SpawnNewRookieShip(m_dockStationID);
            }
        } else {
            SpawnNewRookieShip(m_dockStationID);
        }
    }

    MoveToLocation(m_dockStationID, NULL_ORIGIN);

    SetSessionTimer();
    m_ship->SetDocked();
}

void Client::UndockFromStation() {
    if (m_TS != nullptr) {
        this->services().Lookup <TradeService>("trademgr")->CancelTrade(this);
    }

    m_invul = true;
    m_undock = true;
    //set position and direction of docking ramp for later use
    m_dockPoint = m_stationData.dockPosition;
    m_movePoint = m_stationData.dockOrientation;

    m_ship->Undock();

    /** @todo  this needs a bit of work to match live....
     * @update we are really close now.  02Jan19
     *  Undock Request -> GetCriminalTimeStamps -> Undock -> OnItemsChanged (Undocking:xxxxxxxx) -> OnCharNoLongerInStation ->
     *  GetAllInfo -> GetNPCStandings -> GetFormations -> AddBalls2 (slim, not ball, wait=true)
     * -> GotoDirection(etc, etc) -> SetState (dmg, ego, ball, slim)
     *  ***** 9sec from hitting undock to space view on live. *****
     */
    MoveToLocation(m_systemData.systemID, m_dockPoint);
    SetInvulTimer(Player::Timer::UndockInvul);
    SetStateTimer(Player::State::Undock, Player::Timer::Undock);
    SetSessionTimer();

    m_ship->SetUndocking(false);
}

void Client::CreateShipSE() {
    FactionData data = FactionData();
        data.allianceID = GetAllianceID();
        data.corporationID = GetCorporationID();
        data.factionID = GetWarFactionID();
        data.ownerID = GetCharacterID();
    pShipSE = new ShipSE(m_ship, m_system->GetServiceMgr(), m_system, data);
    _log(PLAYER__MESSAGE, "CreateShipSE() - pShipSE %p created for %s(%u)", pShipSE, m_char->name(), m_char->itemID());
}

void Client::DestroyShipSE() {
    if (pShipSE != nullptr) {
        _log(PLAYER__MESSAGE, "DestroyShipSE() - pShipSE %p (%s) destroyed for %s(%u)", pShipSE, m_ship->name(), m_char->name(), m_char->itemID());
        m_system->RemoveEntity(pShipSE);
        SafeDelete(pShipSE);
    } else {
        _log(PLAYER__WARNING, "DestroyShipSE() - pShipSE = null for %s(%u)", m_char->name(), m_char->itemID());
    }
    pShipSE = nullptr;
}

void Client::UpdateNewShip()
{
    sItemFactory.SetUsingClient(this);
    pShipSE->SetPilot(this);
    pShipSE->DestinyMgr()->UpdateNewShip(m_ship);
    sItemFactory.UnsetUsingClient();

    char ci[45];
    snprintf(ci, sizeof(ci), "InSpace: %s(%u)", GetName(), m_char->itemID());
    m_ship->SetCustomInfo(ci);
    SetSessionTimer();
}

void Client::SetPodItem() {
    if (!IsPlayerItem(m_char->capsuleID())) {
        CreateNewPod();
    } else {
        m_pod = sItemFactory.GetShipRef(m_char->capsuleID());
    }
    if (m_pod.get() == nullptr)
        CreateNewPod();
}

// can throw
void Client::CheckShipRef(ShipItemRef newShipRef)
{
    if (newShipRef.get() == nullptr) {
        _log(PLAYER__ERROR, "CheckShipRef() - %s: newShipRef == NULL.", m_char->name());
        throw CustomError ("Could not find ship's ItemRef.  Cannot Board.   Ref: ServerError 12321.");
    } else if (!newShipRef->isSingleton()) {
        _log(PLAYER__MESSAGE, "%s tried to board ship %u, which is not assembled.", m_char->name(), newShipRef->itemID());
        throw CustomError ("You cannot board a ship which is not assembled!");
    } else if ((m_ship == newShipRef) and !m_login) {
        // if char is loging in, this will hit.  unknown about any other time.
        _log(PLAYER__MESSAGE, "%s tried to board active ship %u.", m_char->name(), newShipRef->itemID());
        throw CustomError ("You are already aboard this ship.");
    }
}

void Client::BoardShip(ShipItemRef newShipRef)
{
    CheckShipRef(newShipRef);

    if (m_login) {
        _log(PLAYER__MESSAGE, "%s boarding active ship %u on login.", m_char->name(), newShipRef->itemID());
    } else if (m_ship->typeID() == itemTypeCapsule) {
        m_ship->SetPosition(NULL_ORIGIN);
        m_ship->Move(m_system->GetID(), flagCapsule, true);
        m_ship->SetCustomInfo(nullptr);
    } else {
        char ci[45];
        snprintf(ci, sizeof(ci), "Inactive: %s(%u)", GetName(), m_char->itemID());
        m_ship->SetCustomInfo(ci);
        m_ship->GetModuleManager()->CharacterLeavingShip();
        m_ship->SaveShip();
    }

    SetShip(newShipRef);
    SetSessionTimer();
}

void Client::Board(ShipSE* newShipSE)
{
    CheckShipRef(newShipSE->GetShipItemRef());

    if (m_ship->typeID() == itemTypeCapsule) {
        m_ship->SetPosition(NULL_ORIGIN);
        m_ship->Move(m_system->GetID(), flagCapsule, true);
        // cannot use DestroyShipSE() for this.  it removes current shipSE, with pilot, like pilot is leaving bubble.
        ShipSE* oldShipSE = pShipSE;
        // set vars to new ship
        SetShip(newShipSE->GetShipItemRef());
        pShipSE = newShipSE;
        // remove pod entity
        m_system->RemoveEntity(oldShipSE);
        SafeDelete(oldShipSE);
    } else {    // you can xfer direct from one ship from another.
        //  check for POS/FF in bubble.  check for ship in FF.  if so, then not abandoned.
        bool abandoned = true;
        if (pShipSE->SysBubble()->HasTower()) {
            TowerSE* ptSE = pShipSE->SysBubble()->GetTowerSE();
            if (ptSE->HasForceField())
                if (pShipSE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                    abandoned = false;
        }

        char ci[45];
        if (abandoned) {
            pShipSE->Abandon();
            snprintf(ci, sizeof(ci), "Abandoned: %s(%u)", GetName(), m_char->itemID());
        } else
            snprintf(ci, sizeof(ci), "Ejected: %s(%u)", GetName(), m_char->itemID());

        m_ship->SetCustomInfo(ci);
        m_ship->SetFlag(flagShipOffline);
        m_ship->Eject();

        pShipSE->DestinyMgr()->Eject();

        SetShip(newShipSE->GetShipItemRef());
        pShipSE = newShipSE;
    }

    UpdateNewShip();
    //SetStateTimer(Player::State::Board, Player::Timer::Board);
    SendSessionChange();
}

void Client::Eject()
{
    if (m_pod.get() == nullptr)
        CreateNewPod();

    if (m_pod.get() == nullptr) {
        _log(SHIP__ERROR, "Handle_Eject() - Failed to get podItem for %s.", GetName());
        if (m_canThrow) {
            throw CustomError ("Something bad happened as you prepared to eject.  Ref: ServerError 25107.");
        } else {
            return;
        }
    }
    // this should NEVER happen...
    if (pShipSE->SysBubble() == nullptr) {
        _log(SHIP__ERROR, "Handle_Eject() - Bubble is null for %s.", GetName());
        if (m_canThrow) {
            throw CustomError ("Something bad happened as you prepared to eject.  Ref: ServerError 25107+1.");
        } else {
            return;
        }
    }

    //  check for POS/FF in bubble.  check for ship in FF.  if so, then not abandoned.
    bool abandoned = true;
    if (pShipSE->SysBubble()->HasTower()) {
        TowerSE* ptSE = pShipSE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField())
            if (pShipSE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                abandoned = false;
    }

    char ci[45];
    if (abandoned) {
        pShipSE->Abandon();
        snprintf(ci, sizeof(ci), "Abandoned: %s(%u)", GetName(), m_char->itemID());
    } else {
        snprintf(ci, sizeof(ci), "Ejected: %s(%u)", GetName(), m_char->itemID());
    }

    m_ship->SetCustomInfo(ci);
    m_ship->SetPlayer(nullptr);
    m_ship->SetFlag(flagShipOffline);
    m_ship->Eject();

    pShipSE->DestinyMgr()->Eject();

    GPoint capsulePosition(pShipSE->GetPosition());
    capsulePosition.MakeRandomPointOnSphere(m_ship->radius() + m_pod->radius() + MakeRandomInt(30, 120));
    m_pod->SetPosition(capsulePosition);
    m_pod->Move(m_locationID, flagCapsule, true);

    FactionData data = FactionData();
        data.ownerID = GetCharacterID();
        data.factionID = GetWarFactionID();
        data.allianceID = GetAllianceID();
        data.corporationID = GetCorporationID();
    ShipSE* newShipSE = new ShipSE(m_pod, m_system->GetServiceMgr(), m_system, data);
    if (newShipSE == nullptr) {
        _log(PLAYER__ERROR, "%s Eject() - pShipSE = NULL for shipID %u.", m_char->name(), m_pod->itemID());
        // we should probably send char to their clone station if this happens....
        MoveToLocation(GetCloneStationID(), NULL_ORIGIN);
        throw CustomError ("There was a problem creating your pod in space.<br>You have been transfered to your home station.<br>Ref: ServerError 15107.");
    }

    newShipSE->SetLauncherID(pShipSE->GetID());
    //  set shipSE to null.  this allows sending AddBalls when pod added to system
    SetShip(m_pod);
    m_system->AddEntity(newShipSE);
    pShipSE = newShipSE;

    UpdateNewShip();
    //SetStateTimer(Player::State::Board, Player::Timer::Board);
    SendSessionChange();
}

void Client::ResetAfterPopped(GPoint& position)
{
    m_autoPilot = false;
    m_bubbleWait = false;    // allow client processing of subsquent destiny msgs

    if (m_pod.get() == nullptr)
        CreateNewPod();

    m_pod->SetPosition(position);

    FactionData data = FactionData();
        data.allianceID = GetAllianceID();
        data.corporationID = GetCorporationID();
        data.factionID = GetWarFactionID();
        data.ownerID = GetCharacterID();
    ShipSE* newShipSE = new ShipSE(m_pod, m_system->GetServiceMgr(), m_system, data);
    if (newShipSE == nullptr) {
        _log(PLAYER__ERROR, "%s ResetAfterPopped() - pShipSE = NULL for shipID %u.", m_char->name(), m_pod->itemID());
        // we should probably send char to their clone station if this happens....
        MoveToLocation(GetCloneStationID(), NULL_ORIGIN);
        SpawnNewRookieShip(m_locationID);
        throw CustomError ("There was a problem creating your pod in space.<br>You have been transfered to your home station.<br>Ref: ServerError 15107.");
    }

    newShipSE->SetLauncherID(pShipSE->GetID());
    pShipSE->DestinyMgr()->Eject();
    // nullify pilot before removing from bubble, which removes player from bubble map
    pShipSE->SetPilot(nullptr);
    // remove dead ship from bubble before calling SetShip() (it deletes pShipSE)
    sBubbleMgr.Remove(pShipSE);
    //  set shipSE to null.  this allows sending AddBalls when pod added to system
    SetShip(m_pod);
    // just in case something's a bit off, reset pod to full
    m_pod->Heal();
    // add pod to system
    m_system->AddEntity(newShipSE);
    pShipSE = newShipSE;

    UpdateNewShip();
    //SetStateTimer(Player::State::Killed, Player::Timer::Killed);
    SendSessionChange();
}

void Client::ResetAfterPodded() {
    /** @todo
     * destroy all implants
     * check skillpoints vs. clone grade and adjust accordingly.
     * reset skill effects if clone != current SP and skills lost
     */

    m_autoPilot = false;

    CreateNewPod();
    SetShip(m_pod);

    // verify clone station hangar isnt loaded yet.
    RemoveStationHangar(GetCloneStationID());

    MoveToLocation(GetCloneStationID(), NULL_ORIGIN);

    m_ship->Move(m_locationID, flagHangar);
    m_ship->SaveShip();
    m_char->ResetClone();
    m_char->SaveCharacter();

    SpawnNewRookieShip(m_locationID);

    //update session with new values
    UpdateSession();
    SendSessionChange();
}

void Client::SetShip(ShipItemRef shipRef) {
    shipRef->ChangeOwner(m_char->itemID());
    if (pShipSE != nullptr)
        pShipSE->SetPilot(nullptr);

    // nullify ship pointer, but do NOT delete...most callers need existing ship for system and destiny pointers
    pShipSE = nullptr;

    m_ship = shipRef;
    m_shipId = shipRef->itemID();
    m_char->SetActiveShip(m_shipId);
    if (sDataMgr.IsSolarSystem(m_locationID)) {
        m_char->Move(m_shipId, flagPilot, true);
        pSession->SetInt("shipid", m_shipId); // update shipID in session
    }

    if (m_validSession or m_charCreation)
        m_ship->SetPlayer(this);
}

void Client::PickAlternateShip() {
    if (m_char.get() != nullptr)
        m_shipId = m_char->PickAlternateShip(m_locationID);
}

void Client::CreateNewPod() {
    std::string pod_name = m_char->itemName() + "'s Capsule";
    ItemData podItem( itemTypeCapsule, m_char->itemID(), locTemp, flagNone, pod_name.c_str() );
    m_pod = sItemFactory.SpawnShip( podItem );
    // make sure this is singleton
    m_pod->ChangeSingleton(true);
    m_pod->Move(m_char->solarSystemID(), flagCapsule);
    m_pod->SaveShip();
    m_char->SetActivePod(m_pod->itemID());  // is this used?
}

ShipItemRef Client::SpawnNewRookieShip(uint32 stationID) {
    /** @todo  create/send mail from scc about lost ship as needed....create char uses this method also */
    //create rookie ship of appropriate type
    using namespace Char;
    uint16 shipID(0), gunID(0);
    switch (m_char->race()) {
        case Race::Caldari: {
            gunID = Rookie::Weapon::Caldari;
            shipID = Rookie::Ship::Caldari;
        } break;
        case Race::Gallente: {
            gunID = Rookie::Weapon::Gallente;
            shipID = Rookie::Ship::Gallente;
        } break;
        case Race::Minmatar: {
            gunID = Rookie::Weapon::Minmatar;
            shipID = Rookie::Ship::Minmatar;
        } break;
        case Race::Amarr: {
            gunID = Rookie::Weapon::Amarr;
            shipID = Rookie::Ship::Amarr;
        } break;
        default: {
            // invalid race
            _log(CLIENT__ERROR, "SpawnNewRookieShip() - Invalid Race of %s(%u) for %s(%u)",
                 sDataMgr.GetRaceName(m_char->race()), m_char->race(), m_char->name(),
                 m_char->itemID());
            return ShipItemRef(nullptr);
        }
    }

    //create data for new rookie ship
    std::string name =  m_char->itemName() + "'s Noob Ship";
    ItemData sData(shipID, m_char->itemID(), locTemp, flagNone, name.c_str());
    //spawn rookie ship
    ShipItemRef sRef = sItemFactory.SpawnShip(sData);
    if (sRef.get() != nullptr) {
        // noob ships come pre-assembled (and "fully fit")
        sRef->ChangeSingleton(true);
        sRef->Move(stationID, flagHangar);
    }
    // create and fit noob items in ship
    ItemData mData(itemCivilianMiner, m_char->itemID(), locTemp, flagNone);
    InventoryItemRef mRef = sItemFactory.SpawnItem(mData);
    if (mRef.get() != nullptr) {
        mRef->ChangeSingleton(true);
        mRef->Move(sRef->itemID(), flagHiSlot0);
        mRef->SetAttribute(AttrOnline, EvilOne, false);
    }
    ItemData wData(gunID, m_char->itemID(), locTemp, flagNone);
    InventoryItemRef wRef = sItemFactory.SpawnItem(wData);
    if (wRef.get() != nullptr) {
        wRef->ChangeSingleton(true);
        wRef->Move(sRef->itemID(), flagHiSlot1);
        wRef->SetAttribute(AttrOnline, EvilOne, false);
    }
    ItemData cData(itemTypeTrit, m_char->itemID(), locTemp, flagNone, 100);
    InventoryItemRef cRef = sItemFactory.SpawnItem(cData);
    if (cRef.get() != nullptr)
        cRef->Move(sRef->itemID(), flagCargoHold);
    // save new ship and items
    sRef->SaveShip();

    // in case caller needs ref to new ship
    return sRef;
}

bool Client::IsJetcanAvalible() {
    if (m_jetcanTimer.Enabled()) {
        return (m_jetcanTimer.Check(false));
    } else {
        return true;
    }
}

PyRep *Client::GetAggressors() const {
    PyDict* dict(nullptr);
    /*
     *            for aggressorID, aggressor in aggressors.iteritems():
     *                for aggresseeID, lastAggression in aggressor.iteritems():
     *                    lastAggression = int(lastAggression / SEC) * SEC
     *                    when = lastAggression + const.aggressionTime * MIN
     *                    self.clearAggressions[aggressorID, aggresseeID, solarsystemID] = when
     *
     *                    aggressionTime = 15
     */
    /*  items are set here as
     *               [PyInt 90971469]                    <- entity (aggressorID)
     *               [PyDict 1 kvp]                      <- dictionary
     *                  [PyInt 1000127]                     <- entity (aggresseeID)
     *                  [PyIntegerVar 129550906897224125]   <- beginning timestamp (lastAggression)
     */

    return dict;
}

void Client::StargateJump(uint32 fromGate, uint32 toGate) {
    if ((m_clientState != Player::State::Idle) or m_stateTimer.Enabled()) {
        sLog.Error("Client","%s: StargateJump called when a move is already pending. Ignoring.", m_char->name());
        /** @todo  send error to client here */
        return;
    }

    // add jump to mapDynamicData for showing in StarMap (F10)    -allan 06Mar14
    MapDB::AddJump(m_systemData.systemID);

    // call Stop() per packet sniff - shuts off AP.  Halt() does also.  try not calling any movement updates
    //pShipSE->DestinyMgr()->Halt();  // Stop() disables ap.  try Halt() to reset ship movement to null
    pShipSE->DestinyMgr()->SendJumpOut(fromGate);
    //  show gate animation in from gate.   -working -allan 15Nov15
    pShipSE->DestinyMgr()->SendGateActivity(fromGate);

    //m_toGate = toGate;
    StaticData toData = StaticData();
    if (!sDataMgr.GetStaticInfo(toGate, toData)) {
        _log(DATA__ERROR, "Failed to retrieve data for stargate %u", toGate);
        /** @todo  send error to client here */
        return;
    }

    // this is where we can put the msgs about system closed or w/e

    // add jump to mapDynamicData for showing in StarMap (F10)    -allan 06Mar14
    MapDB::AddJump(toData.systemID);
    // used for showing Visited Systems in StarMap(F10)  -allan 30Jan14
    m_char->VisitSystem(toData.systemID);

    m_movePoint = toData.position;
    // Make Jump-In point a random spot on ~10km radius sphere about the stargate radius
    m_movePoint.MakeRandomPointOnSphereLayer(toData.radius + 6500, toData.radius + 9500);
    m_moveSystemID = toData.systemID;
/*
    char ci[25];
    snprintf(ci, sizeof(ci), "Jumping:%u", toGate);
    m_ship->SetCustomInfo(ci);
*/
    //delay the move 4sec so they can see the JumpOut animation
    SetStateTimer(Player::State::Jump, Player::Timer::Jumping);
}

void Client::CynoJump(InventoryItemRef beacon) {
    if ((m_clientState != Player::State::Idle) or m_stateTimer.Enabled()) {
        sLog.Error("Client","%s: CynoJump called when a move is already pending. Ignoring.", m_char->name());
        /** @todo  send error to client here */
        return;
    }

    MapDB::AddJump(m_locationID);

    m_moveSystemID = beacon->locationID();
    MapDB::AddJump(m_moveSystemID);
    m_char->VisitSystem(m_moveSystemID);

    JumpOutEffect(GetShipID());

    m_movePoint = beacon->position();
    m_movePoint.MakeRandomPointOnSphere(2000);

    SetStateTimer(Player::State::DriveJump, Player::Timer::Jumping);
}

void Client::WormholeJump(InventoryItemRef wormhole) {
    if ((m_clientState != Player::State::Idle) or m_stateTimer.Enabled()) {
        sLog.Error("Client","%s: WormholeJump called when a move is already pending. Ignoring.", m_char->name());
        /** @todo  send error to client here */
        return;
    }

    MapDB::AddJump(m_locationID);
    pShipSE->DestinyMgr()->SendJumpOutWormhole(wormhole->itemID());
    pShipSE->DestinyMgr()->SendWormholeActivity(wormhole->itemID());

    m_moveSystemID = wormhole->GetAttribute(AttrWormholeTargetSystem1).get_int();
    MapDB::AddJump(m_moveSystemID);
    m_char->VisitSystem(m_moveSystemID);


    // Get destination wormhole position and start jump timer
    InventoryItemRef destWh;
    destWh = sItemFactory.GetItemRefFromID(wormhole->GetAttribute(AttrWormholeTargetSystem2).get_int());

    m_movePoint = destWh->position();
    m_movePoint.MakeRandomPointOnSphere(2000);
    SetStateTimer(Player::State::WormholeJump, Player::Timer::Jumping);

}

void Client::ExecuteJump() {
    if (m_movePoint == NULL_ORIGIN) {
        m_clientState = Player::State::Idle;
        _log(AUTOPILOT__TRACE, "ExecuteJump() - movePoint = null; state set to Idle");
        /** @todo  send error to client here */
        return;
    }

    //OnScannerInfoRemoved  - no args.  flushes current scan data in client
    SendNotification("OnScannerInfoRemoved", "charid", new PyTuple(0), true);  // this is sequenced
    pShipSE->Jump();

    MoveToLocation(m_moveSystemID, m_movePoint);

    SetBallParkTimer(Player::Timer::Jump);

    m_movePoint = NULL_ORIGIN;
    m_moveSystemID = 0;
}

void Client::ExecuteDriveJump() {
    if (m_movePoint == NULL_ORIGIN) {
        m_clientState = Player::State::Idle;
        _log(AUTOPILOT__TRACE, "ExecuteDriveJump() - movePoint = null; state set to Idle");
        /** @todo  send error to client here */
        return;
    }

    //OnScannerInfoRemoved  - no args.  flushes current scan data in client
    SendNotification("OnScannerInfoRemoved", "charid", new PyTuple(0), true);  // this is sequenced
    pShipSE->Jump(false);

    MoveToLocation(m_moveSystemID, m_movePoint);
    SetBallParkTimer(Player::Timer::Jump);

    m_movePoint = NULL_ORIGIN;
    m_moveSystemID = 0;
}

void Client::ExecuteWormholeJump() {
    if (m_movePoint == NULL_ORIGIN) {
        m_clientState = Player::State::Idle;
        _log(AUTOPILOT__TRACE, "ExecuteWormholeJump() - movePoint = null; state set to Idle");
        /** @todo  send error to client here */
        return;
    }

    //OnScannerInfoRemoved  - no args.  flushes current scan data in client
    SendNotification("OnScannerInfoRemoved", "charid", new PyTuple(0), true);  // this is sequenced
    pShipSE->Jump(false);

    MoveToLocation(m_moveSystemID, m_movePoint);
    SetBallParkTimer(Player::Timer::Jump);

    m_movePoint = NULL_ORIGIN;
    m_moveSystemID = 0;
}

void Client::SetBallParkTimer(uint32 time/*Player::Timer::Default*/)
{
    if (time == 0) {
        m_ballparkTimer.Disable();
        _log(CLIENT__TIMER, "%s: Ballpark Timer Disabled", m_char->name());
        return;
    }

    if (m_ballparkTimer.Enabled()) {
        _log(CLIENT__ERROR, "%s: Ballpark Timer called but timer already enabled with %ums remaining.", m_char->name(), m_ballparkTimer.GetRemainingTime());
        EvE::traceStack();
        return;
    }

    _log(CLIENT__TIMER, "%s: Ballpark Timer set at %ums.  current state time is %ums", m_char->name(), time, m_ballparkTimer.GetCurrentTime());
    m_ballparkTimer.Start(time);
}

void Client::SetCloakTimer(uint32 time/*Player::Timer::Default*/)
{
    if (time == 0) {
        m_cloakTimer.Disable();
        if (pShipSE != nullptr)
            if (pShipSE->DestinyMgr() != nullptr)
                pShipSE->DestinyMgr()->UnCloak();
        _log(CLIENT__TIMER, "%s: Cloak Timer Disabled", m_char->name());
        return;
    }

    if (m_cloakTimer.Enabled()) {
        _log(CLIENT__ERROR, "%s: Cloak Timer called but timer already enabled with %ums remaining.", m_char->name(), m_cloakTimer.GetRemainingTime());
        EvE::traceStack();
        return;
    }

    _log(CLIENT__TIMER, "%s: Cloak Timer set at %ums.   current state time is %ums", m_char->name(), time, m_cloakTimer.GetCurrentTime());
    m_cloakTimer.Start(time);
    if (m_login)
        return;
    if (pShipSE != nullptr)
        if (pShipSE->DestinyMgr() != nullptr)
            pShipSE->DestinyMgr()->Cloak();
}

void Client::SetUncloakTimer(uint32 time/*Player::Timer::Default*/)
{
    if (time == 0) {
        m_uncloakTimer.Disable();
        SetUncloak(false);
        _log(CLIENT__TIMER, "%s: Uncloak Timer Disabled", m_char->name());
        return;
    }

    if (m_uncloakTimer.Enabled()) {
        _log(CLIENT__ERROR, "%s: Uncloak Timer called but timer already enabled with %ums remaining.", m_char->name(), m_cloakTimer.GetRemainingTime());
        EvE::traceStack();
        return;
    }

    _log(CLIENT__TIMER, "%s: Uncloak Timer set at %ums.   current state time is %ums", m_char->name(), time, m_uncloakTimer.GetCurrentTime());
    m_uncloakTimer.Start(time);
    SetUncloak(true);
}

void Client::SetInvulTimer(uint32 time/*Player::Timer::Default*/)
{
    if (time == 0) {
        SetInvul(false);
        m_invulTimer.Disable();
        _log(CLIENT__TIMER, "%s: Invul Timer Disabled", m_char->name());
        return;
    }

    if (m_invulTimer.Enabled()) {
        _log(CLIENT__ERROR, "%s: Invul Timer called but timer already enabled with %ums remaining.", m_char->name(), m_invulTimer.GetRemainingTime());
        EvE::traceStack();
        return;
    }

    _log(CLIENT__TIMER, "%s: Invul Timer set at %ums.   current state time is %ums", m_char->name(), time, m_invulTimer.GetCurrentTime());
    m_invulTimer.Start(time);
    SetInvul(true);
}

void Client::SetStateTimer( int8 state, uint32 time/*Player::Timer::Default*/)
{
    if (time == 0) {
        m_stateTimer.Disable();
        _log(CLIENT__TIMER, "%s: State Timer Disabled", m_char->name());
        return;
    }

    if (m_stateTimer.Enabled()) {
        _log(CLIENT__ERROR, "%s: State Timer called but timer already enabled with %ums remaining.", m_char->name(), m_stateTimer.GetRemainingTime());
        EvE::traceStack();
        return;
    }

    _log(CLIENT__TIMER, "%s: Client Timer set from %s to %s at %ums.  current state time: %u", m_char->name(), \
            GetStateName(m_clientState).c_str(), GetStateName(state).c_str(), time, m_stateTimer.GetCurrentTime());
    m_clientState = state;
    m_stateTimer.Start(time);
}

// these next two are for sending jump effects (easier to test here)
void Client::JumpInEffect()
{
    if (pShipSE != nullptr)
        if (pShipSE->DestinyMgr() != nullptr)
            if (!pShipSE->DestinyMgr()->IsCloaked())
                pShipSE->DestinyMgr()->SendJumpInEffect("effects.JumpIn");
}

void Client::JumpOutEffect(uint32 locationID)
{
    if (pShipSE != nullptr)
        if (pShipSE->DestinyMgr() != nullptr)
            if (!pShipSE->DestinyMgr()->IsCloaked())
                pShipSE->DestinyMgr()->SendJumpOutEffect("effects.JumpOut", locationID);
}

std::string Client::GetStateName(int8 state)
{
    switch (state) {
        case Player::State::Idle:      return "Idle";
        case Player::State::Jump:      return "Jump";
        case Player::State::DriveJump: return "DriveJump";
        case Player::State::WormholeJump: return "WormholeJump";
        case Player::State::Dock:      return "Dock";
        case Player::State::Undock:    return "Undock";
        case Player::State::Killed:    return "Killed";
        case Player::State::Logout:    return "Logout";
        case Player::State::Board:     return "Board";
        case Player::State::Login:     return "Login";
        case Player::State::LoginWarp: return "LoginWarp";
    }
    return "Undefined";
}

void Client::AddStationHangar(uint32 stationID) {
    m_hangarLoaded.insert(std::make_pair(stationID, true));
}

void Client::RemoveStationHangar(uint32 hangarID) {
    m_hangarLoaded.erase(hangarID);
}

bool Client::IsHangarLoaded(uint32 hangarID) {
    std::map<uint32, bool>::const_iterator itr = m_hangarLoaded.find(hangarID);
    if (itr != m_hangarLoaded.end())
        return itr->second;
    return false;
}

void Client::LoadStationHangar(uint32 stationID) {
    _log(PLAYER__INFO, "Client::LoadStationHangar() is loading personal hangar for %s(%u) in stationID %u",  m_char->name(), m_char->itemID(), stationID);
    sItemFactory.SetUsingClient(this);
    m_system->GetStationFromInventory(stationID)->GetMyInventory()->LoadContents();
    sItemFactory.UnsetUsingClient();
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag)
{
    sItemFactory.SetUsingClient(this);
    InventoryItemRef iRef = sItemFactory.GetItemRef(itemID);
    if (iRef.get() == nullptr) {
        _log(INV__ERROR, "Client::MoveItem() - %s Unable to load item %u", m_char->name(), itemID);
        return;
    }

    EVEItemFlags oldflag = flagIllegal;
    oldflag = iRef->flag();

    iRef->Move(location, flag, true);

    if (IsPlayerItem(location)) {
        if (IsModuleSlot(iRef->flag())) {
            m_ship->UpdateModules(iRef->flag());
        } else if (IsCargoHoldFlag(iRef->flag()) or (iRef->flag() == flagDroneBay)) {
            // do nothing here.  this is to avoid throwing error msg below
        } else {
            _log(INV__WARNING, "Client::MoveItem() - %s Unhandled PlayerItem %s (%u) from flag %s to flag %s.", \
                    m_char->name(), iRef->name(), itemID, sDataMgr.GetFlagName(oldflag), sDataMgr.GetFlagName(flag));
        }
    } else {
        _log(INV__WARNING, "Client::MoveItem() - %s Unhandled NonPlayerItem %s (%u) from flag %s to flag %s.", \
        m_char->name(), iRef->name(), itemID, sDataMgr.GetFlagName(oldflag), sDataMgr.GetFlagName(flag));
    }
    sItemFactory.UnsetUsingClient();
}

uint32 Client::GetLoyaltyPoints(uint32 corpID) {
    std::map<uint32, uint32>::iterator itr = m_lpMap.find(corpID);
    if (itr != m_lpMap.end())
        return itr->second;
    return 0;
}

void Client::RemoveMissionItem(uint16 typeID, uint32 qty)
{
    uint16 count = qty;
    InventoryItemRef iRef(nullptr);
    if (sDataMgr.IsStation(m_locationID)) {
        iRef = sItemFactory.GetStationRef(m_locationID)->GetMyInventory()->GetByTypeFlag(typeID, flagHangar);
        if (iRef.get() != nullptr) {
            if (count < iRef->quantity()) {
                iRef->AlterQuantity(count, true);
                count = 0;
            } else {
                count -= iRef->quantity();
                iRef->Delete();
            }
        }
    }

    if (count) {
        iRef = GetShip()->GetMyInventory()->GetByTypeFlag(typeID, flagCargoHold);
        if (iRef.get() != nullptr){
            if (count < iRef->quantity()) {
                iRef->AlterQuantity(count, true);
                count = 0;
            } else {
                count -= iRef->quantity();
                iRef->Delete();
            }
        }
    }
    if (count)
        ;  // make error here for not enough?
}

bool Client::ContainsTypeQty(uint16 typeID, uint32 qty) const
{
    uint16 count = 0;
    InventoryItemRef iRef(nullptr);
    // this is for missions....we will have to determine if we have the TOTAL qty desired, in both cargo and hangar
    if (sDataMgr.IsStation(m_locationID)) {
        iRef = sItemFactory.GetStationRef(m_locationID)->GetMyInventory()->GetByTypeFlag(typeID, flagHangar);
        if (iRef.get() != nullptr)
            count = iRef->quantity();
    }

    iRef = GetShip()->GetMyInventory()->GetByTypeFlag(typeID, flagCargoHold);
    if (iRef.get() != nullptr)
        count += iRef->quantity();

    if (count >= qty)
        return true;
    return false;
}

bool Client::IsMissionComplete(MissionOffer& data)
{
    // dont know all the stipulations of "completion" yet, but skeleton code for starters...
    switch (data.typeID) {
        case Mission::Type::Tutorial: {
        } break;
        case Mission::Type::Encounter: {
        } break;
        case Mission::Type::Courier: {
            if (m_locationID == data.destinationID)
                if (ContainsTypeQty(data.courierTypeID, data.courierAmount))
                    return true;
        } break;
        case Mission::Type::Trade: {
        } break;
        case Mission::Type::Mining: {
        } break;
        case Mission::Type::Research: {
        } break;
        case Mission::Type::Data: {
        } break;
        case Mission::Type::Storyline: {
        } break;
        case Mission::Type::Cosmos: {
        } break;
        case Mission::Type::EpicArc: {
        } break;
        case Mission::Type::Anomic: {
        } break;
    }

    return false;
}


void Client::ChannelJoined(LSCChannel *chan) {
    m_channels.insert(chan);
}

void Client::ChannelLeft(LSCChannel *chan) {
    if (m_loaded)
        m_channels.erase(chan);
}

/************************************************************************/
/* character notification messages wrapper                              */
/************************************************************************/
void Client::CharNoLongerInStation() {
    // clear station data
    // remove client from station guest list
    sEntityList.GetStationByID(m_stationData.stationID)->RemoveGuest(this);
    m_system->SetDockCount(this, false);
    OnCharNoLongerInStation ocnis;
        ocnis.charID = m_char->itemID();
        ocnis.corpID = GetCorporationID();
        ocnis.allianceID = GetAllianceID();
        ocnis.factionID = GetWarFactionID();
    PyTuple* tmp = ocnis.Encode();
    if (tmp == nullptr)
        return;
    std::vector<Client*> clients;
    clients.clear();
    sEntityList.GetStationGuestList(m_stationData.stationID, clients);
    for (auto cur : clients) {
        PyIncRef(tmp);
        cur->SendNotification("OnCharNoLongerInStation", "stationid", &tmp); //consumed
    }
    PyDecRef(tmp);

    // delete current station data
    m_stationData = StationData();
}

void Client::CharNowInStation() {
    m_system->SetDockCount(this, true);
    OnCharNowInStation ocnis;
        ocnis.charID = m_char->itemID();
        ocnis.corpID = GetCorporationID();
        ocnis.allianceID = GetAllianceID();
        ocnis.warFactionID = GetWarFactionID();
    PyTuple* tmp = ocnis.Encode();
    std::vector<Client*> clients;
    clients.clear();
    sEntityList.GetStationGuestList(m_locationID, clients);
    for (auto cur : clients) {
        PySafeIncRef(tmp);
        cur->SendNotification("OnCharNowInStation", "stationid", &tmp);
    }
    PySafeDecRef(tmp);
}

/**********************************************************************
 *  session shit and other non-player-related things
 * ****************************************************************/
void Client::InitSession(int32 characterID)
{
    if (!IsCharacterID(characterID)) {
        sLog.Error("Client::InitSession()", "characterID is not valid");
        return;
    }

    std::map<std::string, int64> characterDataMap;
    CharacterDB::GetCharacterDataMap(characterID, characterDataMap);
    if (characterDataMap.size() < 1) {
        sLog.Error("Client::InitSession()", "characterDataMap.size() returned zero.");
        return;
    }

    int32 stationID         = (int32)(characterDataMap["stationID"]);
    int32 solarSystemID     = (int32)(characterDataMap["solarSystemID"]);
    m_shipId                = (int32)(characterDataMap["shipID"]);

    pSession->SetInt("genderID",         (int32)(characterDataMap["gender"]));
    pSession->SetInt("bloodlineID",      (int32)(characterDataMap["bloodlineID"]));
    pSession->SetInt("raceID",           (int32)(characterDataMap["raceID"]));
    pSession->SetInt("charid",           characterID);
    pSession->SetInt("corpid",           (int32)(characterDataMap["corporationID"]));

    pSession->SetInt("cloneStationID",   (int32)(characterDataMap["cloneStationID"]));
    pSession->SetInt("solarsystemid2",   solarSystemID);
    pSession->SetInt("constellationid",  (int32)(characterDataMap["constellationID"]));
    pSession->SetInt("regionid",         (int32)(characterDataMap["regionID"]));

    pSession->SetInt("hqID",             (int32)(characterDataMap["corporationHQ"]));
    pSession->SetInt("baseID",           characterDataMap["baseID"]);
    pSession->SetInt("corpAccountKey",   characterDataMap["corpAccountKey"]);

    //Only set allianceID if it is not 0
    if (characterDataMap["allianceID"] != 0){
        pSession->SetInt("allianceid", characterDataMap["allianceID"]);
    }

    pSession->SetInt("warfactionid",     characterDataMap["warFactionID"]);

    pSession->SetLong("corprole",        characterDataMap["corpRole"]);
    pSession->SetLong("rolesAtAll",      characterDataMap["rolesAtAll"]);
    pSession->SetLong("rolesAtBase",     characterDataMap["rolesAtBase"]);
    pSession->SetLong("rolesAtHQ",       characterDataMap["rolesAtHQ"]);
    pSession->SetLong("rolesAtOther",    characterDataMap["rolesAtOther"]);

    /*  solarSystemID != 0  -character in space
     *   also used as current system in following menus:
     *  JumpPortalBridgeMenu, GetHybridBeaconJumpMenu, GetHybridBridgeMenu
     */
    if (sDataMgr.IsStation(stationID)) {
        m_locationID = stationID;
        pSession->Clear("solarsystemid");    //must be 0 in station
        pSession->Clear("shipid");    //must be 0 in station
        pSession->SetInt("stationid", stationID);
        pSession->SetInt("stationid2", stationID);
        pSession->SetInt("locationid", stationID);
        pSession->SetInt("worldspaceid", stationID);
    } else {
        m_locationID = solarSystemID;
        pSession->Clear("stationid");   //must be 0 in space
        pSession->Clear("stationid2");  //must be 0 in space
        pSession->Clear("worldspaceid");  //must be 0 in space
        pSession->SetInt("shipid", m_shipId);
        pSession->SetInt("solarsystemid", solarSystemID);
        pSession->SetInt("locationid", solarSystemID);
    }

    sDataMgr.GetSystemData(m_locationID, m_systemData);
    if ((sDataMgr.IsSolarSystem(m_systemData.systemID))
    and (IsConstellationID(m_systemData.constellationID))
    and (IsRegionID(m_systemData.regionID)))
    {
        m_validSession = true;
    }
}

void Client::UpdateSession()
{
    if (m_char.get() == nullptr)
        return;
    uint32 stationID = m_char->stationID();
    uint32 solarsystemID = m_char->solarSystemID();
    if (sDataMgr.IsStation(stationID)) {
        pSession->Clear("solarsystemid");    //must be 0 in station
        pSession->Clear("shipid");    //must be 0 in station
        //pSession->Clear("worldspaceid");    //not used here (yet)

        pSession->SetInt("stationid", stationID);
        pSession->SetInt("stationid2", stationID);   // client uses this for continer location checks
        pSession->SetInt("worldspaceid", stationID);
        pSession->SetInt("locationid", stationID);
    } else {
        pSession->Clear("stationid");
        pSession->Clear("stationid2");
        pSession->Clear("worldspaceid");
        pSession->SetInt("solarsystemid", solarsystemID); //  used to tell client they are in space
        pSession->SetInt("locationid", solarsystemID);
        pSession->SetInt("shipid", m_shipId);
    }

    // solarsystemid2 is used by client to determine current system.  NOTE:  *MUST* be set to current system.
    pSession->SetInt("solarsystemid2", solarsystemID);
    pSession->SetInt("constellationid", m_char->constellationID());
    pSession->SetInt("regionid", m_char->regionID());
}

void Client::UpdateSessionInt(const char *id, int value)
{
    pSession->SetInt(id, value);
}

void Client::UpdateCorpSession(CorpData& data)
{
    // session.Set* methods only update on change
    pSession->SetInt("corpid", data.corporationID);
    pSession->SetInt("baseID", data.baseID);
    pSession->SetInt("hqID", data.corpHQ);

    //Only set allianceID if it is not 0
    if (data.allianceID != 0){
        pSession->SetInt("allianceid", data.allianceID);
    }

    pSession->SetInt("warfactionid", data.warFactionID);
    pSession->SetInt("corpAccountKey", data.corpAccountKey);
    pSession->SetLong("corprole", data.corpRole);
    pSession->SetLong("rolesAtAll", data.rolesAtAll);
    pSession->SetLong("rolesAtBase", data.rolesAtBase);
    pSession->SetLong("rolesAtHQ", data.rolesAtHQ);
    pSession->SetLong("rolesAtOther", data.rolesAtOther);
    SendSessionChange();
}

void Client::UpdateFleetSession(CharFleetData& fleet)
{
    m_fleet = fleet.fleetID;
    m_wing = fleet.wingID;
    m_squad = fleet.squadID;

    pSession->SetInt("fleetjob", fleet.job);
    pSession->SetInt("fleetrole", fleet.role);
    pSession->SetInt("fleetbooster", fleet.booster);
    pSession->SetInt("fleetid", m_fleet);
    pSession->SetInt("wingid", m_wing);
    pSession->SetInt("squadid", m_squad);
    SendSessionChange();
}

void Client::SendInitialSessionStatus ()
{
    SessionInitialState scn;
    scn.initialstate = new PyDict();

    pSession->EncodeInitialState (scn.initialstate);

    if (is_log_enabled(CLIENT__SESSION)) {
        _log(CLIENT__SESSION, "Session initialized.  Sending initial session state");
        scn.initialstate->Dump(CLIENT__SESSION, "   Changes: ");
    }

    scn.sessionID = pSession->GetSessionID();

    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.SessionInitialStateNotification";
    packet->type = SESSIONINITIALSTATENOTIFICATION;

    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();
    packet->source.callID = 0;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = 0;

    packet->userid = GetUserID();

    packet->payload = scn.Encode();
    packet->named_payload = nullptr;

    if (is_log_enabled(CLIENT__SESSION_DUMP)) {
        _log(CLIENT__SESSION_DUMP, "Sending Session packet:");
        PyLogDumpVisitor dumper(CLIENT__SESSION_DUMP, CLIENT__SESSION_DUMP);
        packet->Dump(CLIENT__SESSION_DUMP, dumper);
    }

    QueuePacket(packet);
}

void Client::SendSessionChange()
{
    if (!pSession->isDirty())
        return;

    // this should never happen now.  -allan 3Aug16
    if (m_locationID == 0) {
        if (m_char.get() != nullptr) {
            codelog(CLIENT__ERROR, "Session::LocationID == 0 for %s(%u)", m_char->name(), m_char->itemID());
            EvE::traceStack();
            if (sDataMgr.IsStation(m_char->stationID())) {
                m_locationID = m_char->stationID();
            } else {
                m_locationID = m_systemData.systemID;
            }
            /* a `session.locationid` change will trigger a ballpark update (add/delete bp) */
            pSession->SetInt("locationid", m_locationID);
        }
    }

    SessionChangeNotification scn;
    scn.changes = new PyDict();

    pSession->EncodeChanges(scn.changes);
    if (scn.changes->empty())
        return;

    if (is_log_enabled(CLIENT__SESSION)) {
        _log(CLIENT__SESSION, "Session updated.  Sending session change");
        scn.changes->Dump(CLIENT__SESSION, "   Changes: ");
    }

    scn.sessionID = 0; //pSession->GetSessionID();
    scn.clueless = 0;
    scn.nodesOfInterest.push_back(-1);  /* this means 'all nodes' */
    scn.nodesOfInterest.push_back(m_services.GetNodeID());  /* add current node to list */
    /* if other nodes are created, add those that are 'live' for this client here */
    //   will need *some way* to track active nodes for each client
    //scn.nodesOfInterest.push_back(m_services.GetNodeID());

    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.SessionChangeNotification";
    packet->type = SESSIONCHANGENOTIFICATION;

    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();
    packet->source.callID = 0;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = 0;

    packet->userid = GetUserID();

    packet->payload = scn.Encode();
    packet->named_payload = nullptr;

    if (is_log_enabled(CLIENT__SESSION_DUMP)) {
        _log(CLIENT__SESSION_DUMP, "Sending Session packet:");
        PyLogDumpVisitor dumper(CLIENT__SESSION_DUMP, CLIENT__SESSION_DUMP);
        packet->Dump(CLIENT__SESSION_DUMP, dumper);
    }

    QueuePacket(packet);

    // clean up packet after being created by 'new'
    //SafeDelete(packet);
}

void Client::FlushQueue() {
    if ((!m_destinyUpdateQueue->empty())
        or (!m_destinyEventQueue->empty()))
        _SendQueuedUpdates();
}

void Client::QueueDestinyEvent(PyTuple** event) {
    if ((event == nullptr) or ((*event) == nullptr))
        return;
    m_destinyEventQueue->AddItem(*event);
    //PyDecRef(*event);
}

void Client::QueueDestinyUpdate(PyTuple **update, bool DoPackage /*false*/, bool IsSetState /*false*/) {
    if ((update == nullptr) or ((*update) == nullptr))
        return;
    if (sDataMgr.IsStation(m_locationID))
        return;
    DoDestinyAction act;
        act.stamp = sEntityList.GetStamp();
    if (DoPackage/* or m_packaged*/) {
        if (IsSetState) {
            // send the setstate buffer alone
            act.update = *update;
        } else {
            // this will package all current updates (and those coming in before next flush) into
            //   a single PackagedAction packet, which is then inserted into the DoDestinyAction packet.
            PyList* paList = new PyList();
                paList->AddItem(*update);
            if (!m_destinyUpdateQueue->empty())
                paList->AddItem(m_destinyUpdateQueue);
            PackagedAction pa;
                pa.substream = new PySubStream(paList);
            act.update = pa.Encode();
            m_packaged = false;
        }
        DoDestinyUpdateMain_2 dum;
            dum.updates = new PyList();
            dum.updates->AddItem(act.Encode());
            dum.waitForBubble = m_bubbleWait;
        PyTuple* t = dum.Encode();
        if (is_log_enabled(CLIENT__QUEUE_DUMP))
            t->Dump(CLIENT__QUEUE_DUMP, "");
        SendNotification("DoDestinyUpdate", "clientID", &t, false);
        PyDecRef(t);
    } else {
        act.update = *update;
        m_packaged = true;
        m_destinyUpdateQueue->AddItem(act.Encode());
    }
}

void Client::_SendQueuedUpdates() {
    if (!m_destinyUpdateQueue->empty()) {
        if (m_destinyEventQueue->empty()) {
            DoDestinyUpdateMain_2 dum;
                dum.updates = m_destinyUpdateQueue;
                dum.waitForBubble = m_bubbleWait;
            PyTuple* t = dum.Encode();
            if (is_log_enabled(CLIENT__QUEUE_DUMP))
                t->Dump(CLIENT__QUEUE_DUMP, "");
            SendNotification("DoDestinyUpdate", "clientID", &t);
        } else {
            DoDestinyUpdateMain dum;
                dum.updates = m_destinyUpdateQueue;
                dum.events = m_destinyEventQueue;
                dum.waitForBubble = m_bubbleWait;
            PyTuple* t = dum.Encode();
            if (is_log_enabled(CLIENT__QUEUE_DUMP))
                t->Dump(CLIENT__QUEUE_DUMP, "");
            SendNotification("DoDestinyUpdate", "clientID", &t);
        }
    } else if (!m_destinyEventQueue->empty()) {
        Notify_OnMultiEvent nom;
            nom.events = m_destinyEventQueue;
        PyTuple* t = nom.Encode();
        if (is_log_enabled(CLIENT__QUEUE_DUMP))
            t->Dump(CLIENT__QUEUE_DUMP, "");
        SendNotification("OnMultiEvent", "charid", &t);
    } //else nothing to be sent ...

    // clear the queues now, after the packets have been sent
    m_destinyUpdateQueue->clear();
    m_destinyEventQueue->clear();
    m_packaged = false;
}

void Client::SendNotification(const char *notifyType, const char *idType, PyTuple *payload, bool seq /*true*/) {
    //build a little notification out of it.
    EVENotificationStream notify;
    notify.notifyType = notifyType;
    notify.remoteObject = 1;
    notify.args = payload;

    PyAddress dest;
    // are all of these 'Broadcast'?
    dest.type = PyAddress::Broadcast;
    dest.service = notifyType;
    dest.bcast_idtype = idType;
    /*
    if (dest.bcast_idtype.compare("clientID") == 0)
        dest.objectID = GetClientID();*/

    //now send it to the client
    SendNotification(dest, notify, seq);
}

void Client::SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq /*true*/) {
    if ((*payload) == nullptr)
        return;
    //build a little notification out of it.
    EVENotificationStream notify;
        notify.notifyType = notifyType;
        notify.remoteObject = 1;
        notify.args = (*payload);

    PyAddress dest;
    // are all of these 'Broadcast'?
        dest.type = PyAddress::Broadcast;
        dest.service = notifyType;
        dest.bcast_idtype = idType;
        dest.objectID = GetClientID();

    //now send it to the client
    SendNotification(dest, notify, seq);
}

void Client::SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq/*true*/) {
    //build the packet:
    PyPacket *packet = new PyPacket();
    packet->type_string = "macho.Notification";
    packet->type = NOTIFICATION;

    // is source type right here?
    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();

    packet->dest = dest;

    packet->userid = GetUserID();

    packet->payload = noti.Encode();

    if (seq) {
        packet->named_payload = new PyDict();
        packet->named_payload->SetItemString("sn", new PyInt(++m_nextNotifySequence));
    }

    if (is_log_enabled(CLIENT__NOTIFY_DUMP)) {
        _log(CLIENT__NOTIFY_REP, "Sending notify of type %s with ID type %s to %s", dest.service.c_str(), dest.bcast_idtype.c_str(), GetName());
        PyLogDumpVisitor dumper(CLIENT__NOTIFY_DUMP, CLIENT__NOTIFY_REP, "", true, true);
        packet->Dump(CLIENT__NOTIFY_DUMP, dumper);
    }

    QueuePacket(packet);
}

/************************************************************************/
/* EVEAdministration Interface                                          */
/************************************************************************/
    // only used by GM Command
void Client::DisconnectClient()
{
    SendNotifyMsg("You have been kicked from this server and will be disconnected shortly.");

    //initiate closing the client TCP Connection
    CloseClientConnection();
}
// only used by GM Command
void Client::BanClient()
{
    //send message to client
    SendNotifyMsg("You have been banned from this server and will be disconnected shortly.  You will no longer be able to log in");

    //ban the client
    ServiceDB::SetAccountBanStatus(GetUserID(), true);
}

/************************************************************************/
/* EVEClientSession interface                                           */
/************************************************************************/
void Client::_GetVersion(VersionExchangeServer& version)
{
    version.birthday = EVEBirthday;
    version.macho_version = MachoNetVersion;
    version.user_count = sEntityList.GetClientCount();
    version.version_number = EVEVersionNumber;
    version.build_version = EVEBuildVersion;
    version.project_version = EVEProjectVersion;
}

uint32 Client::GetUserCount()
{
    return sEntityList.GetClientCount();
}

bool Client::_VerifyVersion(VersionExchangeClient& version)
{
    version.Dump(NET__PRES_REP, "    ");
    if (version.birthday != EVEBirthday)
        sLog.Error("Client","%s: Client's birthday does not match ours!", GetAddress().c_str());
    if (version.macho_version != MachoNetVersion)
        sLog.Error("Client","%s: Client's macho_version not match ours!", GetAddress().c_str());
    if (version.version_number != EVEVersionNumber)
        sLog.Error("Client","%s: Client's version_number not match ours!", GetAddress().c_str());
    if (version.build_version != EVEBuildVersion)
        sLog.Error("Client","%s: Client's build_version not match ours!", GetAddress().c_str());
    if (version.project_version != EVEProjectVersion)
        sLog.Error("Client","%s: Client's project_version not match ours!", GetAddress().c_str());
    return true;
}

bool Client::_VerifyCrypto(CryptoRequestPacket& cr)
{
    if (cr.keyVersion != "placebo") {
        //I'm sure cr.keyVersion can specify either CryptoAPI or PyCrypto, but its all binary so im not sure how.
        CryptoAPIRequestParams car;
        if (!car.Decode(cr.keyParams)) {
            sLog.Error("Client","%s: Received invalid CryptoAPI request!", GetAddress().c_str());
        } else {
            sLog.Error("Client","%s: Unhandled CryptoAPI request: hashmethod=%s sessionkeylength=%d provider=%s sessionkeymethod=%s", GetAddress().c_str(), car.hashmethod.c_str(), car.sessionkeylength, car.provider.c_str(), car.sessionkeymethod.c_str());
            SendErrorMsg("Invalid CryptoAPI request - You must change your client to use Placebo crypto in common.ini to talk to this server.");
        }

        return false;
    } else {
        sLog.Debug("Client","%s: Received Placebo crypto request, accepting.", GetAddress().c_str());

        //send out accept response
        PyRep* rsp = new PyString("OK CC");
        mNet->QueueRep(rsp);
    }

    return true;
}

bool Client::_VerifyLogin(CryptoChallengePacket& ccp)
{
    /* send passwordVersion required: 1=plain, 2=hashed */
    // this doesnt work as i want it to.
    //  sending '2' will have client use hashed pass.
    //  sending '1' will have client send hashed pass first, then a second authentication packet using plain pass
    PyRep* res = new PyInt(2);
    mNet->QueueRep(res);

    std::string failMsg = "Login Authorization Invalid.";

    // test account name for invalid chars (which may allow sql injection)
    if (!ServiceDB::ValidateAccountName(ccp, failMsg))
        return _LoginFail(failMsg);

    AccountData aData = AccountData();
    if (!ServiceDB::GetAccountInformation(ccp, aData, failMsg))
        return _LoginFail(failMsg);

    if (aData.banned) {
        failMsg = "Your account is banned. Contact Allan for further support";
        return _LoginFail(failMsg);
    }

    if (aData.online) {
        failMsg = "This account is currently online.";
        return _LoginFail(failMsg);
    }

    if (!ccp.user_password.empty()) {
        sLog.Warning("  Client::Login()", "%s(%u) - Using Plain Password", aData.name.c_str(), aData.clientID);
        if (strcmp(aData.password.c_str(), ccp.user_password.c_str()) != 0) {
            failMsg = "The plain Password you entered is incorrect for this account.";
            return _LoginFail(failMsg);
        }
    } else {
        //sLog.Warning("  Client::Login()", "%s(%u) - Using Hashed Password", aData.name.c_str(), aData.clientID);
        if (strcmp(aData.hash.c_str(), ccp.user_password_hash.c_str()) != 0) {
            failMsg = "The Password you entered is incorrect for this account.";
            return _LoginFail(failMsg);
        }

        if (!ccp.user_password.empty())
            ServiceDB::UpdatePassword(aData.id, ccp.user_password.c_str());
    }

    /** @todo  check this character/account for newbie status and revoke as needed before account update.  */

    /* send our handshake */
    CryptoServerHandshake server_shake;
    //server_shake.context = ??
    server_shake.serverChallenge = "";
    server_shake.func_marshaled_code = new PyBuffer(marshaledNone, marshaledNone + sizeof(marshaledNone));
    server_shake.verification = new PyBool(false);
    server_shake.cluster_usercount = sEntityList.GetClientCount(); //GetUserCount();
    server_shake.proxy_nodeid = 0xFFAA; //888444
    server_shake.user_logonqueueposition = _GetQueuePosition();
    // binascii.crc_hqx of marshaled single-element tuple containing 64 zero-bytes string
    server_shake.challenge_responsehash = "55087";

    // the image server to be used by the client to download images
    server_shake.imageserverurl = sImageServer.url();

    server_shake.macho_version = MachoNetVersion;
    server_shake.boot_version = EVEVersionNumber;
    server_shake.boot_build = EVEBuildVersion;
    server_shake.boot_codename = EVEProjectCodename;
    server_shake.boot_region = EVEProjectRegion;
    res = server_shake.Encode();
    mNet->QueueRep(res);

    // Setup session, but don't send the change yet.
    pSession->SetString("address", EVEClientSession::GetAddress().c_str());
    pSession->SetString("languageID", ccp.user_languageid.c_str());

    pSession->SetInt("userType", Acct::Type::Mammon);     //aData.type  - incomplete (db fields done)
    pSession->SetInt("userid", aData.id);
    pSession->SetLong("role", aData.role);
    pSession->SetLong("clientID", 1000000L * aData.clientID + 888444);  // kinda arbitrary
    pSession->SetLong("sessionID", 0 /*pSession->GetSessionID()*/);

    sLog.Green("  Client::Login()","Account %u (%s) logging in from %s", aData.id, aData.name.c_str(), EVEClientSession::GetAddress().c_str());

    return true;
}

bool Client::_LoginFail(std::string fail_msg)
{
    GPSTransportClosed* except = new GPSTransportClosed(fail_msg);
    mNet->QueueRep(except);
    return false;
}

bool Client::_VerifyFuncResult(CryptoHandshakeResult& result)
{
    _log(NET__PRES_DEBUG, "%s: Handshake result received.", GetAddress().c_str());

    //send this before session change
    CryptoHandshakeAck ack;
        ack.jit = GetLanguageID();
        ack.userid = GetUserID();   //5654387 accountID?
        ack.maxSessionTime = PyStatic.NewNone();        // set this for an auto-logout time?
        ack.userType = Acct::Type::Mammon;      //GetAccountType()  - not written yet
        ack.role = Acct::Role::PLAYER | Acct::Role::NEWBIE | Acct::Role::LOGIN; /*  live returns these */
        ack.address = GetAddress();
        ack.inDetention = PyStatic.NewNone();   // dont know what this is or what it's for
        ack.client_hash = PyStatic.NewNone();
        ack.user_clientid = GetClientID();  //241241000001103
        ack.live_updates = sLiveUpdateDB.GetUpdates();
        ack.sessionID = 0; //pSession->GetSessionID();   //398773966249980114
    PyRep* res(ack.Encode());
    if (is_log_enabled(CLIENT__CALL_DUMP))
        res->Dump(CLIENT__CALL_DUMP, "    ");
    mNet->QueueRep(res, false);

    // send out the initial session status
    SendInitialSessionStatus();

    return true;
}

void Client::_SendCallReturn(const PyAddress& source, int64 callID, PyResult &rsp)
{
    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.CallRsp";
    packet->type = CALL_RSP;

    packet->source = source;     /* address should be 'ship' for warpto response */

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = callID;

    packet->userid = GetUserID();

    packet->payload = new PyTuple(1);
    packet->payload->SetItem(0, new PySubStream(rsp.ssResult));
    packet->named_payload = rsp.ssNamedResult;

    if (is_log_enabled(COLLECT__PACKET_DUMP)) {
        _log(COLLECT__PACKET_DUMP, "_SendCallReturn: Dump()");
        PyLogDumpVisitor dumper(COLLECT__PACKET_DUMP, COLLECT__PACKET_DUMP, "", true, true);
        packet->Dump(COLLECT__PACKET_DUMP, dumper);
    }

    QueuePacket(packet);
}

void Client::_SendException(const PyAddress& source, int64 callID, MACHONETMSG_TYPE msgType, MACHONETERR_TYPE errCode, PyRep** payload)
{
    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.ErrorResponse";
    packet->type = ERRORRESPONSE;

    packet->source = source;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = callID;

    packet->userid = GetUserID();

    ErrorResponse e;
        e.MsgType = msgType;
        e.ErrorCode = errCode;
        e.payload = *payload;
    payload = nullptr;

    packet->payload = e.Encode();
    QueuePacket(packet);
}

void Client::_SendPingRequest()
{
    PyPacket *packet = new PyPacket();

    packet->type = PING_REQ;
    packet->type_string = "macho.PingReq";

    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();
    packet->source.service = "ping";
    packet->source.callID = 0;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = 0;

    packet->userid = GetUserID();

    packet->payload = new_tuple(new PyList()); //times
    packet->named_payload = new PyDict();

    QueuePacket(packet);
}

void Client::_SendPingResponse(const PyAddress& source, int64 callID)
{
    PyPacket* packet = new PyPacket();
    packet->type = PING_RSP;
    packet->type_string = "macho.PingRsp";

    packet->source = source;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = callID;

    packet->userid = GetUserID();

    /*  Here the hacking begins, the ping packet handles the timestamps of various packet handling steps.
     *        To really simulate/emulate that we need the various packet handlers which in fact we don't have (:P).
     *        So the next piece of code "fake's" it, with a slight delay on the received packet time.
     */
    PyList* pingList = new PyList();
    PyTuple* pingTuple(nullptr);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));        // this should be the time the packet was received (we cheat here a bit)
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));             // this is the time the packet is (handled/written) by the (proxy/server) so we're cheating a bit again.
    pingTuple->SetItem(2, new PyString("proxy::handle_message"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::writing"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("server::handle_message"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("server::turnaround"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::handle_message"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::writing"));
    pingList->AddItem(pingTuple);

    // Set payload
    packet->payload = new PyTuple(1);
    packet->payload->SetItem(0, pingList);

    // Don't clone so it eats the ret object upon sending.
    QueuePacket(packet);
}

/************************************************************************/
/* EVEPacketDispatcher interface                                        */
/************************************************************************/
bool Client::Handle_CallReq(PyPacket* packet, PyCallStream& req)
{
    // build arguments
    PyCallArgs args(this, req.arg_tuple, req.arg_dict);
    PyResult result;
    uint32 nodeID = 0, bindID = 0;

    // try to handle with the new service handler and fallback to the old version
    try
    {
        if (packet->dest.service == "") {
            if (sscanf(req.remoteObjectStr.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
                sLog.Error("Client::CallReq", "Failed to parse bind string '%s'.", req.remoteObjectStr.c_str());
                return false;
            }

            if (nodeID != m_services.GetNodeID()) {
                sLog.Error("Client::CallReq", "Unknown nodeID - received %u but expected %u.", nodeID, m_services.GetNodeID());
                return false;
            }

            _log(SERVICE__CALLS_BOUND, "%s::%s()", req.remoteObjectStr.c_str(), req.method.c_str());
            m_canThrow = true;
            result = m_services.Dispatch(bindID, req.method, args);
            m_canThrow = false;
        } else {
            _log(SERVICE__CALLS, "%s::%s()", packet->dest.service.c_str(), req.method.c_str());
            m_canThrow = true;
            result = m_services.Dispatch(packet->dest.service, req.method, args);
            m_canThrow = false;
        }
    }
    catch (method_not_found ex)
    {
        sLog.Error("Client::CallReq", "Unable to find method to handle call to: %s::%s", packet->dest.service.c_str(), req.method.c_str());

        if (sConfig.debug.IsTestServer) {
            if (packet->dest.service == "") {
                sLog.Error("Client::CallReq", m_services.DebugDispatch(bindID, req.method, args).c_str());
            } else {
                sLog.Error("Client::CallReq", m_services.DebugDispatch(packet->dest.service, req.method, args).c_str());
            }
        }

        // ensure some data is returned
        result = nullptr;
    }
    catch (service_not_found)
    {
        sLog.Error("Client::CallReq", "Unable to find service to handle call to: %s", packet->dest.service.c_str());
        packet->dest.Dump(CLIENT__CALL_DUMP, "    ");
        throw UserError("ServiceNotFound"); // this message is invalid (message not found)
    }

    SendSessionChange();  //send out the session change before the return.
    if (is_log_enabled(CLIENT__OUT_ALL)) {
        if (result.ssResult != nullptr)
            result.ssResult->Dump(CLIENT__OUT_ALL, "    ");
        if (result.ssNamedResult != nullptr)
            result.ssNamedResult->Dump(CLIENT__OUT_ALL, "    ");
    }

    _SendCallReturn(packet->dest, packet->source.callID, result);

    return true;
}

bool Client::Handle_Notify(PyPacket* packet)
{
    //turn this thing into a notify stream:
    ServerNotification notify;
    if (!notify.Decode(packet->payload)) {
        sLog.Error("Client::Notify","Failed to convert rep into a notify stream");
        return false;
    }

    if (notify.method == "ClientHasReleasedTheseObjects") {
        _log(SERVICE__MESSAGE, "Client Has Released These Objects:");
        ServerNotification_ReleaseObj element;

        uint32 nodeID(0), bindID(0);
        PyList::const_iterator cur = notify.elements->begin();
        for (; cur != notify.elements->end(); ++cur) {
            if (!element.Decode(*cur)) {
                sLog.Error("Client::Notify","Notification '%s' from %s: Failed to decode element. Skipping.", notify.method.c_str(),  m_char->name());
                continue;
            }

            if (sscanf(element.boundID.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
                sLog.Error("Client::Notify","Notification '%s' from %s: Failed to parse bind string '%s'. Skipping.", \
                           notify.method.c_str(), m_char->name(), element.boundID.c_str());
                continue;
            }

            if (nodeID != m_services.GetNodeID()) {
                sLog.Error("Client::Notify","Notification '%s' from %s: Unknown nodeID %u received (expected %u). Skipping.", \
                           notify.method.c_str(), m_char->name(), nodeID, m_services.GetNodeID());
                continue;
            }

            // clear bindID from internal map
            m_bindSet.erase(bindID);
            m_services.ClearBoundObject(bindID, this);
        }
    } else {
        sLog.Error("Client::Notify","Unhandled notification from %s: unknown method '%s'", m_char->name(), notify.method.c_str());
        return false;
    }

    return true;
}

// NOTE: 'OnRemoteMessage' can be disabled in client
//this displays a modal error dialog on the client side.
void Client::SendErrorMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str(nullptr);
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    Notify_OnRemoteMessage n;
    n.msgType = "CustomError";
    n.args[ "error" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

void Client::SendErrorMsg(const char* fmt, va_list args)
{
    char* str(nullptr);
    vasprintf(&str, fmt, args);
    assert(str);

    Notify_OnRemoteMessage n;
    n.msgType = "CustomError";
    n.args[ "error" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

//this displays a modal info dialog on the client side.
void Client::SendInfoModalMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str(nullptr);
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    Notify_OnRemoteMessage n;
    n.msgType = "ServerMessage";
    n.args[ "msg" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

//this displays a little notice (like combat messages)
void Client::SendNotifyMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str(nullptr);
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    Notify_OnRemoteMessage n;
    n.msgType = "CustomNotify";
    n.args[ "notify" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

void Client::SendNotifyMsg(const char* fmt, va_list args)
{
    char* str(nullptr);
    vasprintf(&str, fmt, args);
    assert(str);

    Notify_OnRemoteMessage n;
    n.msgType = "CustomNotify";
    n.args[ "notify" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

//there may be a less hackish way to do this.
void Client::SelfChatMessage(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str(nullptr);
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    if (m_channels.empty()) {
        if (m_char.get() != nullptr)
            sLog.Error("Client", "%s: Tried to send self chat, but we are not joined to any channels: %s", m_char->name(), str);
        free(str);
        return;
    }

    if (m_char.get() != nullptr)
        sLog.White("Client","%s: Self message on all channels: %s", m_char->name(), str);

    //this is such a pile of crap, but im not sure whats better.
    //maybe a private message...
    std::set<LSCChannel*>::iterator itr = m_channels.begin();
    for (; itr != m_channels.end(); ++itr)
        (*itr)->SendMessage(this, str, true);

    //m_channels[

    //just send it to the first channel we are in..
    /*LSCChannel *chan = *(m_channels.begin());
     *    char self_id[24];   //such crap..
     *    snprintf(self_id, sizeof(self_id), "%u", m_char->itemID());
     *    if (chan->GetName() == self_id) {
     *        if (m_channels.size() > 1) {
     *            chan = *(++m_channels.begin());
}
}*/

    SafeFree(str);
}

// `IsLoginWarping` returns `true` if the player is performing the warp that
// occurs during login, or is just about to. The player's state will either be
// `LoginWarp`, or it will still have `loginWarpPoint` set to a non-zero value.
// Upon terminating the login warp bubble, Destiny should reset the login warp
// point and, if the player's state is set to `LoginWarp`, it should be reset to
// idle, if possible.
//
// The activation of this warp bubble cannot be stopped under any circumstance,
// unless of course the player's ship has not despawned between logout and login
// (in which case the login warp would not have been triggered in the first
// place).
bool Client::IsLoginWarping() {
    return m_clientState == Player::State::Login || m_clientState == Player::State::LoginWarp || !m_loginWarpPoint.isZero() || !m_loginWarpRandomPoint.isZero();
}

// For context and guidelines on how to use this function, see the code
// documentation for `Client::IsLoginWarping`.
//
// It is safe to repeatedly call this function without consequence.
void Client::SetLoginWarpComplete() {
    if (m_clientState == Player::State::LoginWarp) {
        m_clientState = Player::State::Idle;
    }

    m_loginWarpPoint = NULL_ORIGIN;
    m_loginWarpRandomPoint = NULL_ORIGIN;
}
