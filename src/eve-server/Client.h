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
    Rewrite:    Allan
*/


#ifndef EVE_CLIENT_H
#define EVE_CLIENT_H


#include "ClientSession.h"

#include "character/Character.h"
#include "inventory/InventoryItem.h"
#include "inventory/ItemRef.h"
#include "packets/Crypto.h"
#include "packets/Destiny.h"
#include "packets/LSCPkts.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleManager.h"
#include "system/SystemEntity.h"

#include "../eve-common/EVE_Missions.h"
#include "../eve-common/EVE_Player.h"

#include "chat/LSCService.h"
// #include "services/ServiceManager.h"

class CryptoChallengePacket;
class EVENotificationStream;
class PySubStream;
class InventoryItem;
class SystemManager;
class EVEServiceManager;
class PyCallStream;
class PyTuple;
class LSCChannel;
class PyAddress;
class PyList;
class PyDict;
class PyPacket;
class PyRep;
class Scan;
class TradeSession;

//DO NOT INHERIT THIS OBJECT!
class Client
: protected EVEClientSession,
  protected EVEPacketDispatcher
{
public:
    Client(EVEServiceManager& services, EVETCPConnection** con);
    // copy c'tor
    Client(const Client& oth) =delete;
    // move c'tor
    Client(Client&& oth) =delete;
    // copy assignment
    Client& operator= (const Client& oth) =delete;
    // move assignment
    Client& operator= (Client&& oth) =delete;

    ~Client();

    // called from main() loop
    bool                    ProcessNet();
    // called by EntityList on 1Hz tic
    void                    ProcessClient();

    bool                    IsLoaded()                  { return m_loaded; }

    EVEServiceManager&      services() const            { return m_services; }
    // this should never be null
    SystemManager*          SystemMgr() const           { return m_system; }
    // used in msgs and other places where a const char* is needed instead of me forgetting to use .c_str()
    const char*             GetName() const             { return (m_char.get() != nullptr ? m_char->name() : "(null)"); }

    bool                    IsAFK()                     { return m_afk; }
    void                    SetAFK(bool set=true)       { m_afk = set; }


    /********************************************************************/
    /* Session values                                                   */
    /********************************************************************/
    bool IsValidSession()                               { return m_validSession; }
    ClientSession* GetSession()                         { return pSession; }
    // these dont always work...still dont know why.  fixed.  was bad _comp method in PyDict
    std::string GetAddress() const                      { return pSession->GetCurrentString( "address" ); }
    std::string GetLanguageID() const                   { return pSession->GetCurrentString( "languageID" ); }

    int32 GetUserID() const                             { return pSession->GetCurrentInt( "userid" ); }
    int32 GetAccountType() const                        { return pSession->GetCurrentInt( "userType" ); }

    // these below need Session initialized before use (which we dont do for char creation due to errors)
    int32 GetCharacterID() const                        { return pSession->GetCurrentInt( "charid" ); }
    int32 GetStationID() const                          { return pSession->GetCurrentInt( "stationid" ); }
    int32 GetStationID2() const                         { return pSession->GetCurrentInt( "stationid2" ); }
    int32 GetCloneStationID() const                     { return pSession->GetCurrentInt( "cloneStationID" ); }

    int64 GetAccountRole() const                        { return pSession->GetCurrentLong( "role" ); }
    int64 GetClientID() const                           { return pSession->GetCurrentLong( "clientID" ); }
    //int64 GetSessionID() const                          { return pSession->GetCurrentLong( "sessionID" ); }

    double GetCorpTaxRate()                             { return (m_char.get() != nullptr ? m_char->corpTaxRate() : 0.0); }
    int32 GetCorporationID() const                      { return pSession->GetCurrentInt( "corpid" ); }
    int32 GetCorpHQ() const                             { return pSession->GetCurrentInt( "hqID" ); }
    int32 GetAllianceID() const                         { return pSession->GetCurrentInt( "allianceid" ); }
    int32 GetWarFactionID() const                       { return pSession->GetCurrentInt( "warfactionid" ); }
    int32 GetCorpAccountKey() const                     { return pSession->GetCurrentInt( "corpAccountKey" ); }
    // corporation management-type roles (manager, officer, trader)  also has container roles
    int64 GetCorpRole() const                           { return pSession->GetCurrentLong( "corprole" ); }
    // access roles everywhere.  is joined with other access roles
    int64 GetRolesAtAll() const                         { return pSession->GetCurrentLong( "rolesAtAll" ); }
    // access roles at base. overrides hq if same location
    int64 GetRolesAtBase() const                        { return pSession->GetCurrentLong( "rolesAtBase" ); }
    // access roles at corp HQ.
    int64 GetRolesAtHQ() const                          { return pSession->GetCurrentLong( "rolesAtHQ" ); }
    // access roles for non-station containers with corp hangars
    int64 GetRolesAtOther() const                       { return pSession->GetCurrentLong( "rolesAtOther" ); }

    // fleet data
    int8 GetFleetRole()                                 { return pSession->GetCurrentInt("fleetrole"); }

    bool InFleet()                                      { return IsFleetID(m_fleet); }
    bool IsFleetBoss()                                  { return (IsFleetID(m_fleet) ? ((GetFleetRole() == Fleet::Role::FleetLeader) ? true : false) : false); }
    bool IsFleetBooster()                               { return (IsFleetID(m_fleet) ? ((GetFleetRole() == Fleet::Booster::No) ? false : true) : false); }

    uint32 GetFleetID() const                           { return m_fleet; }
    int32 GetWingID() const                             { return m_wing; }
    int32 GetSquadID() const                            { return m_squad; }

    uint32 GetShipID() const                            { return m_shipId; }
    uint32 GetLocationID() const                        { return m_locationID; }
    uint32 GetSystemID() const                          { return m_systemData.systemID; }
    uint32 GetConstellationID() const                   { return m_systemData.constellationID; }
    uint32 GetRegionID() const                          { return m_systemData.regionID; }
    std::string GetSystemName() const                   { return m_systemData.name; }

    //  public functions to update client session when char's roles are changed
    void UpdateCorpSession(CorpData& data);
    void UpdateFleetSession(CharFleetData& fleet);

    // character data used before session data is initialized
    uint32 GetLoyaltyPoints(uint32 corpID);
    void SetChar(CharacterRef charRef)                  { m_char = charRef; }   // only used during char creation
    CharacterRef GetChar() const                        { return m_char; }
    std::string GetCharName()                           { return m_char->itemName(); }
    uint32 GetCharID()                                  { return (m_char.get() != nullptr ? m_char->itemID() : 0); }   // only used during char creation
    ShipItemRef GetShip() const                         { return m_ship; }
    ShipSE* GetShipSE()                                 { return pShipSE; }
    ShipItemRef GetPod() const                          { return m_pod; }
    uint32 GetPodID() const                             { return m_pod->itemID(); }
    float GetBounty() const                             { return m_char->bounty(); }
    float GetSecurityRating() const                     { return m_char->GetSecurityRating(); }

    bool AddBalance(double amount, uint8 type=Account::CreditType::ISK) // ---marketbot update
                                                        { return m_char->AlterBalance(amount, type); }
    float GetBalance(uint8 type=Account::CreditType::ISK)
                                                        { return m_char->balance(type); }

    // ship functions
    void SetPodItem();
    void CreateShipSE();
    void SetShip(ShipItemRef shipRef);
    void CreateNewPod();
    void UndockFromStation();
    void DockToStation();
    void PickAlternateShip();
    void ResetAfterPodded();
    void ResetAfterPopped(GPoint& position);  //  delete killed ship, reset player to pod, add pod to system
    void Eject();       // only called in space
    void Board(ShipSE* newShipSE); // only called when in space
    void BoardShip(ShipItemRef newShipRef); // only called when docked

private:
    // ship MUST be added to system before update (need sysMgr, sysBubble, DestinyMgr)
    void UpdateNewShip();    //  calls destiny update methods
    // called by Board methods
    void CheckShipRef(ShipItemRef newShipRef);  // can throw

public:
    // misc char functions
    bool SelectCharacter(int32 char_id=0);
    bool IsHangarLoaded(uint32 stationID);

    void UpdateBubble();
    void WarpIn();
    void WarpOut();
    void SaveLocationData(const char* customInfoPrefix = nullptr);  // Shared helper for saving location data
    void EnterSystem(uint32 systemID);     // only called by gm command, and only if (bubble == null)
    // this will accept null coords and adjust position to random moon
    void MoveToLocation(uint32 location, const GPoint &pt);
    void MoveToPosition(const GPoint &pt);
    void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
    void SetCloakTimer(uint32 time=Player::Timer::Default);     // send time=0 to disable
    void SetInvulTimer(uint32 time=Player::Timer::Default);     // send time=0 to disable
    void SetUncloakTimer(uint32 time=Player::Timer::Default);     // send time=0 to disable
    void SetBallParkTimer(uint32 time=Player::Timer::Default);     // send time=0 to disable
    void SetStateTimer(int8 state, uint32 time=Player::Timer::Default);     // send time=0 to disable
    void SetDestiny(const GPoint& pt, bool update=false);
    ShipItemRef SpawnNewRookieShip(uint32 stationID);
    void LoadStationHangar(uint32 stationID);
    void AddStationHangar(uint32 stationID);
    void RemoveStationHangar(uint32 stationID);

    //destiny stuff...
    void SetDockStationID(uint32 stationID)             { m_dockStationID = stationID; };
    void SetDockPoint(GPoint &pt)                       { m_dockPoint = pt; }
    uint32 GetDockStationID()                           { return m_dockStationID; };
    GPoint GetDockPoint()                               { return m_dockPoint; }
    bool InPod()                                        { return (m_ship->groupID() == EVEDB::invGroups::Capsule); }
    bool IsInSpace()                                    { return sDataMgr.IsSolarSystem(m_locationID); }
    bool IsDocked()                                     { return sDataMgr.IsStation(m_locationID); }
    bool IsDock()                                       { return (m_clientState == Player::State::Dock); }
    bool IsIdle()                                       { return (m_clientState == Player::State::Idle); }
    bool IsGateJump()                                   { return (m_clientState == Player::State::Jump); }
    bool IsDriveJump()                                  { return (m_clientState == Player::State::DriveJump); }
    bool IsWormholeJump()                               { return (m_clientState == Player::State::WormholeJump); }
    bool IsJump()                                       { return ((m_clientState == Player::State::DriveJump) or (m_clientState == Player::State::Jump) or (m_clientState == Player::State::WormholeJump)); } //Gate and Drive are both forms of jumping
    bool IsBoard()                                      { return (m_clientState == Player::State::Board); }
    bool IsInvul()                                      { return m_invul; }
    bool IsLogin()                                      { return m_login; }
    bool IsUndock()                                     { return m_undock; }
    bool IsUncloak()                                    { return m_uncloak; }
    bool HasBeyonce()                                   { return m_beyonce; }
    bool IsBubbleWait()                                 { return m_bubbleWait; }
    bool IsSetStateSent()                               { return m_setStateSent; }
    bool IsSessionChange()                              { return m_sessionChangeActive; }
    bool IsLoginWarping();
    uint32 GetSessionChangeTime()                       { return m_sessionTimer.GetRemainingTime() / 1000; }

    void SetInvul(bool invul=false)                     { m_invul = invul; }
    void SetUndock(bool undock=false)                   { m_undock = undock; }
    void SetBeyonce(bool beyonce=false)                 { m_beyonce = beyonce; }
    void SetUncloak(bool uncloak=false)                 { m_uncloak = uncloak; }
    void SetBubbleWait(bool wait=false)                 { m_bubbleWait = wait; }
    void SetLoginWarpComplete();
    void SetStateSent(bool set=false)                   { m_setStateSent = set; }
    void SetSessionTimer()                              { m_sessionChangeActive = true; m_sessionTimer.Start(Player::Timer::Session); }
    void SetSessionChange(bool set=false)               { m_sessionChangeActive = set; }
    void SetBallPark();
    void StargateJump(uint32 fromGate, uint32 toGate);
    void CynoJump(InventoryItemRef beacon);
    void WormholeJump(InventoryItemRef wormhole);

    bool IsAutoPilot()                                  { return m_autoPilot; }
    void SetAutoPilot(bool set=false);

    void JumpInEffect();
    void JumpOutEffect(uint32 locationID);

    void CheckBallparkTimer();

    //jetcan timer
    bool IsJetcanAvalible();
    // return time remaining in seconds
    uint32 JetcanTime()                                 { return (m_jetcanTimer.GetRemainingTime() / 1000); }
    void StartJetcanTimer()                             { m_jetcanTimer.Start(Player::Timer::Jetcan); }

    void SetShowAll(bool set=false)                     { m_showall = set; }
    bool IsShowall()                                    { return m_showall; }

    void SetAutoStop(bool set=false)                    { m_autoStop = set; }
    bool AutoStop()                                     { return m_autoStop; }

    //messages and LSC
    // error requires dismissal (click 'ok')
    void SendErrorMsg(const char *fmt, ...);
    void SendErrorMsg(const char *fmt, va_list args);
    // notify self-removes after delay
    void SendNotifyMsg(const char *fmt, ...);
    void SendNotifyMsg(const char *fmt, va_list args);
    // info requires dismissal (click 'ok')
    void SendInfoModalMsg(const char *fmt, ...);
    void SelfChatMessage(const char *fmt, ...);
    void SelfEveMail(const char *subject, const char *fmt, ...);
    void ChannelJoined(LSCChannel *chan);
    void ChannelLeft(LSCChannel *chan);
    void UpdateSessionInt( const char *sessionType, int value );

    PyRep *GetAggressors() const;
    void QueueDestinyUpdate(PyTuple** update, bool DoPackage=false, bool IsSetState=false);
    void QueueDestinyEvent(PyTuple** multiEvent);
    void FlushQueue();

    //  mission
    void RemoveMissionItem(uint16 typeID, uint32 qty);
    bool ContainsTypeQty(uint16 typeID, uint32 qty) const;
    bool IsMissionComplete(MissionOffer& data);

    //  scan
    Scan* scan()                                        { return m_scan; }
    void SetScan(Scan* pScan)                           { m_scan = pScan; }
    // set scan timer in ms
    // this is used in scan.cpp after time calc's are done
    void SetScanTimer(uint16 time, bool useProbe=false) { m_scanTimer.Start(time);  m_scanProbe = useProbe; }

    //  trade
    void SetTradeSession(TradeSession* ts)              { m_TS = ts; }
    void ClearTradeSession()                            { m_TS = nullptr; }
    TradeSession* GetTradeSession()                     { return m_TS; }

    // character notification messages
    void CharNowInStation();
    void CharNoLongerInStation();       // clears m_StationData and remove char from station guestList

    // portrait stuff....
    bool RecPic()                                       { return m_portrait; }
    void SetPicRec(bool set=false)                      { m_portrait = set; }

    /********************************************************************/
    /* Server Administration Interface                                  */
    /********************************************************************/
    void DisconnectClient();
    void BanClient();

    // this will add clone alpha if no clone is found
    void InitSession( int32 characterID  );

    // skill training timer shit
    void SetTrainingEndTime(int64 endTime)              { m_skillTimer = endTime; }

protected:
    ServiceDB m_sDB;
    StationData m_stationData;
    SystemData m_systemData;
    ShipItemRef m_ship;
    ShipItemRef m_pod;
    CharacterRef m_char;
    EVEServiceManager& m_services;
    LSCService* m_lsc;
    Scan* m_scan;
    ShipSE* pShipSE;
    TradeSession* m_TS;
    ClientSession* pSession;
    SystemManager* m_system;    //we do not own this

    void ExecuteJump();
    void ExecuteDriveJump();
    void ExecuteWormholeJump();
    void DestroyShipSE();

    //void _AwardBounty(SystemEntity *who);

    bool m_afk;             // for map info (pilots docked and active)
    bool m_invul;
    bool m_login;
    bool m_undock;
    bool m_loaded;
    bool m_beyonce;
    bool m_uncloak;
    bool m_showall;         // boolean for showing all dynamics in system on ships scanner (ROLE_GMH)
    bool m_autoStop;        // boolean for auto-stopping modules when target attrib is full (ROLE_PLAYER)
    bool m_packaged;        // used to correctly package updates into a PackagedAction list
    bool m_portrait;        // used to verify new char pic received
    bool m_autoPilot;       // set true for using autopilot.
    bool m_scanProbe;       // scanning with probes
    bool m_bubbleWait;
    bool m_setStateSent;
    bool m_sessionChangeActive; // used to delay actions requiring destiny updates

    int32 m_wing;
    int32 m_squad;

    uint32 m_fleet;
    uint32 m_shipId;
    //uint32 m_toGate;
    uint32 m_locationID;
    uint32 m_moveSystemID;  // holder for jumping to 'systemID'.    timer based.
    uint32 m_dockStationID; // holder for docking to 'stationID'.  timer based.

    Timer m_stateTimer;      // state timer to consolidate timers
    Timer m_pingTimer;
    Timer m_scanTimer;       // used to delay scan results based on skills, items, and other shit
    Timer m_cloakTimer;      // used for tracking jump cloak
    Timer m_uncloakTimer;    // used for tracking ship actions after cloak module deactivated
    Timer m_invulTimer;
    Timer m_fleetTimer;      // used to apply fleet boost on undock and jump when applicable
    Timer m_clientTimer;     // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
    Timer m_jetcanTimer;     // used to delay jetcan creation.  3min default
    Timer m_logoutTimer;     // used to hold client object until WarpOut finishes
    Timer m_sessionTimer;    // used to prevent multiple session changes from occurring too fast
    Timer m_ballparkTimer;   // this is to properly send SetState data after a delay (cant do it correctly otherwise)

    // this is GPoint on jump and dock heading on undock
    GPoint m_movePoint;
    // dock location in space (absolute)
    GPoint m_dockPoint;
    // upon login, the player needs to warp to a location, but the warp
    // has to be queued to the next tick after login events have been processed
    GPoint m_loginWarpPoint;
    GPoint m_loginWarpRandomPoint;

    std::set<LSCChannel*>   m_channels;    //we do not own these.
    std::map<uint32, bool>  m_hangarLoaded;

    int64                   m_skillTimer;

    int8                    m_clientState;

    /********************************************************************/
    /* EVEClientSession interface                                       */
    /********************************************************************/
    void _GetVersion( VersionExchangeServer& version );
    uint32 GetUserCount();
    uint32 _GetQueuePosition()                          { /* hack */ return 1; }

    /********************************************************************/
    /* EVEClientLogin statemachine                                      */
    /********************************************************************/
    bool _LoginFail(std::string fail_msg);
    bool _VerifyVersion( VersionExchangeClient& version );
    bool _VerifyCrypto( CryptoRequestPacket& cr );
    bool _VerifyLogin( CryptoChallengePacket& ccp );
    bool _VerifyVIPKey( const std::string& vipKey )     { /* do nothing */ return true; }
    bool _VerifyFuncResult( CryptoHandshakeResult& result );

    /********************************************************************/
    /* EVEPacketDispatcher interface                                    */
    /********************************************************************/
public:
    void SendSessionChange();
    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple *payload, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

    // this is to check Throw status, to avoid throws/segfault when not applicable  (should use try/catch block)
    bool CanThrow()                                     { return m_canThrow; }

    bool IsCharCreation()                               { return m_charCreation; }
    void CreateChar(bool set)                           { m_charCreation = set; }

    void AddBindID(uint32 bindID)                       { m_bindSet.emplace(bindID); }

private:
    bool m_canThrow;
    bool m_validSession;
    bool m_charCreation;

    std::set<uint32> m_bindSet;

protected:
    void SendInitialSessionStatus ();
    void UpdateSession();
    void _SendPingRequest();
    void _SendException( const PyAddress& source, int64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload );
    void _SendCallReturn( const PyAddress& source, int64 callID, PyResult& rsp);
    void _SendPingResponse( const PyAddress& source, int64 callID );

    bool Handle_CallReq( PyPacket* packet, PyCallStream& req );
    bool Handle_Notify( PyPacket* packet );
    bool Handle_PingReq( PyPacket* packet )             { _SendPingResponse( packet->dest, packet->source.callID ); return true; }
    bool Handle_PingRsp( PyPacket* packet )             { /* do nothing */ return true; }

private:
    //queues for destiny updates:
    PyList* m_destinyEventQueue;    //we own these. These are events as used in OnMultiEvent
    PyList* m_destinyUpdateQueue;    //we own these. They are the `update` which go into DoDestinyAction
    void _SendQueuedUpdates();

    uint32 m_nextNotifySequence;

    std::map<uint32, uint32>    m_lpMap;    // corpID/points

    std::string GetStateName(int8 state);
};

#endif
