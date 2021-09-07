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

#ifndef __SYSTEMENTITY_H_INCL__
#define __SYSTEMENTITY_H_INCL__

#include "SystemDB.h"
#include "inventory/InventoryItem.h"
#include "pos/PosMgrDB.h"
#include "system/DestinyManager.h"
#include "system/TargetManager.h"

class BeltMgr;
class Character;
class Client;
class Concord;
class ContainerSE;
class Damage;
class DroneSE;
class NPC;
class Sentry;
class SystemBubble;
class SystemManager;
class WreckSE;
class FieldSE;
class ProbeSE;

class StationSE;
class StaticSystemEntity;
class PlanetSE;
class MoonSE;
class StargateSE;
class BeltSE;
class DynamicSystemEntity;
class ItemSystemEntity;
class ObjectSystemEntity;
class WormholeSE;
class AnomalySE;
class StructureSE;
class CustomsSE;
class DeployableSE;
class AsteroidSE;
class ShipSE;
class DungeonSE;

class TowerSE;
class TCUSE;
class SBUSE;
class IHubSE;
class ArraySE;
class BatterySE;
class ModuleSE;
class WeaponSE;
class ReactorSE;

/*
 * base class for all SystemEntities  - no TargetMgr or DestinyMgr
 * complete rewrite of entity class system  - allan  9 January 2016
 */
class SystemEntity {
    friend class SystemBubble;    /* only to update m_bubble */
public:
    SystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~SystemEntity()                             { /* do nothing here */ }

    /* Process Calls - Overridden as needed in derived classes */
    virtual void                Process();
    virtual bool                ProcessTic()            { return true; }   // not used yet

    /* (Allan) the next two sections eliminate the overhead of RTTI static casting.  */
    /* class type pointer querys, grouped by base class.  public for anyone to access. */
    /* Base */
    virtual SystemEntity*       GetSE()                 { return this; }
    /* Static */
    virtual StaticSystemEntity* GetStaticSE()           { return nullptr; }
    virtual StationSE*          GetStationSE()          { return nullptr; }
    virtual PlanetSE*           GetPlanetSE()           { return nullptr; }
    virtual MoonSE*             GetMoonSE()             { return nullptr; }
    virtual StargateSE*         GetGateSE()             { return nullptr; }
    virtual BeltSE*             GetBeltSE()             { return nullptr; }
    /* Item */
    virtual ItemSystemEntity*   GetItemSE()             { return nullptr; }
    virtual ContainerSE*        GetContSE()             { return nullptr; }
    virtual WreckSE*            GetWreckSE()            { return nullptr; }
    virtual AnomalySE*          GetAnomalySE()          { return nullptr; }
    virtual WormholeSE*         GetWormholeSE()         { return nullptr; }
    virtual FieldSE*            GetFieldSE()            { return nullptr; }
    virtual ProbeSE*            GetProbeSE()            { return nullptr; }
    /* Object */
    virtual ObjectSystemEntity* GetObjectSE()           { return nullptr; }
    virtual AsteroidSE*         GetAsteroidSE()         { return nullptr; }
    virtual StructureSE*        GetPOSSE()              { return nullptr; }
    virtual StructureSE*        GetJammerSE()           { return nullptr; }
    virtual StructureSE*        GetJumpBridgeSE()       { return nullptr; }
    virtual StructureSE*        GetOutpostSE()          { return nullptr; }
    virtual TowerSE*            GetTowerSE()            { return nullptr; }
    virtual ArraySE*            GetArraySE()            { return nullptr; }
    virtual WeaponSE*           GetWeaponSE()           { return nullptr; }
    virtual BatterySE*          GetBatterySE()          { return nullptr; }
    virtual DeployableSE*       GetDeployableSE()       { return nullptr; }
    virtual Sentry*             GetSentrySE()           { return nullptr; }
    virtual ModuleSE*           GetModuleSE()           { return nullptr; }
    virtual ReactorSE*          GetReactorSE()          { return nullptr; }
    virtual CustomsSE*          GetCOSE()               { return nullptr; }
    virtual TCUSE*              GetTCUSE()              { return nullptr; }
    virtual SBUSE*              GetSBUSE()              { return nullptr; }
    virtual IHubSE*             GetIHubSE()             { return nullptr; }
    /* Dynamic */
    virtual DynamicSystemEntity* GetDynamicSE()         { return nullptr; }
    virtual NPC*                GetNPCSE()              { return nullptr; }
    virtual DroneSE*            GetDroneSE()            { return nullptr; }
    virtual Missile*            GetMissileSE()          { return nullptr; }
    virtual ShipSE*             GetShipSE()             { return nullptr; }
    virtual Concord*            GetConcordSE()          { return nullptr; }

    /* class type tests, grouped by base class.  public for anyone to access. */
    /* Base */
    virtual bool                isGlobal()              { return true; } //m_self->isGlobal(); }    // not all items have this attribute set
    virtual bool                IsSystemEntity()        { return true; }
    virtual bool                IsInanimateSE()         { return false; }
    /* Static */
    virtual bool                IsStaticEntity()        { return false; }
    virtual bool                IsBeltSE()              { return false; }
    virtual bool                IsGateSE()              { return false; }
    virtual bool                IsPlanetSE()            { return false; }
    virtual bool                IsMoonSE()              { return false; }
    virtual bool                IsStationSE()           { return false; }
    /* Item */
    virtual bool                IsItemEntity()          { return false; }
    virtual bool                IsAnomalySE()           { return false; }
    virtual bool                IsWormholeSE()          { return false; }
    virtual bool                IsCelestialSE()         { return false; }
    virtual bool                IsContainerSE()         { return false; }
    virtual bool                IsFieldSE()             { return false; }
    virtual bool                IsProbeSE()             { return false; }
    /* Object */
    virtual bool                IsObjectEntity()        { return false; }
    virtual bool                IsSentrySE()            { return false; }
    virtual bool                IsPOSSE()               { return false; }
    virtual bool                IsCOSE()                { return false; }
    virtual bool                IsTCUSE()               { return false; }
    virtual bool                IsSBUSE()               { return false; }
    virtual bool                IsIHubSE()              { return false; }
    virtual bool                IsTowerSE()             { return false; }
    virtual bool                IsArraySE()             { return false; }
    virtual bool                IsJammerSE()            { return false; }
    virtual bool                IsWeaponSE()            { return false; }
    virtual bool                IsBatterySE()           { return false; }
    virtual bool                IsModuleSE()            { return false; }
    virtual bool                IsMoonMiner()           { return false; }
    virtual bool                IsOutpostSE()           { return false; }
    virtual bool                IsAsteroidSE()          { return false; }
    virtual bool                IsDeployableSE()        { return false; }
    virtual bool                IsJumpBridgeSE()        { return false; }
    virtual bool                IsReactorSE()           { return false; }
    virtual bool                IsOperSE()              { return false; }
    /* Dynamic */
    virtual bool                IsDynamicEntity()       { return false; }
    virtual bool                IsLogin()               { return false; }
    virtual bool                IsInvul()               { return false; }
    virtual bool                IsFrozen()              { return false; }
    virtual bool                IsNPCSE()               { return false; }
    virtual bool                IsDroneSE()             { return false; }
    virtual bool                IsWreckSE()             { return false; }
    virtual bool                IsMissileSE()           { return false; }
    virtual bool                IsShipSE()              { return false; }
    virtual bool                IsConcord()             { return false; }

    /* generic functions handled here */
    PyServiceMgr&               GetServices()           { return m_services; }
    SystemBubble*               SysBubble()             { return m_bubble; }
    SystemManager*              SystemMgr()             { return m_system; }
    TargetManager*              TargetMgr()             { return m_targMgr; }
    DestinyManager*             DestinyMgr()            { return m_destiny; }

    /* common functions for all entities handled here */
    /* public data queries  */
    InventoryItemRef            GetSelf()               { return m_self; }
    uint16                      GetTypeID()             { return m_self->typeID(); }
    uint32                      GetGroupID()            { return m_self->groupID(); }
    uint8                       GetCategoryID()         { return m_self->categoryID(); }
    EVEItemFlags                GetFlag()               { return m_self->flag(); }
    uint32                      GetID()                 { return m_self->itemID(); }
    double                      GetRadius()             { return m_radius; }
    uint32                      GetLocationID()         { return m_self->locationID(); }
    const char*                 GetName() const         { return m_self->name(); }
    const GPoint&               GetPosition() const     { return m_self->position(); }
    void                  SetPosition(const GPoint &pos){ m_self->SetPosition(pos); }
    inline double               x()                     { return m_self->position().x; }
    inline double               y()                     { return m_self->position().y; }
    inline double               z()                     { return m_self->position().z; }
    int32                       GetAllianceID()         { return m_allyID; }
    int32                       GetWarFactionID()       { return m_warID; }
    uint32                      GetCorporationID()      { return m_corpID; }
    uint32                      GetOwnerID()            { return m_ownerID; }
    uint32                      GetFleetID()            { return m_fleetID; }
    void                        SetFleetID(uint32 set)  { m_fleetID = set; }

    int8                        GetHarmonic()           { return m_harmonic; }
    void                        SetHarmonic(int8 set)   { m_harmonic = set; }


    /* public generic functions handled in base class. */
    void                        DropLoot(WreckContainerRef wreckRef, uint32 groupID, uint32 owner);
    void                        AwardSecurityStatus(InventoryItemRef iRef, Character* pChar);
    void                        SendDamageStateChanged();  /* this uses targetMgr update to send to all interested parties */
    bool                        ApplyDamage(Damage &d); /* This method is defined in Damage.cpp */
    double                      DistanceTo2(const SystemEntity* other);
    PyTuple*                    MakeDamageState();

    /* public specific functions handled in base class. */
    virtual void                Abandon();

    /* generic functions handled here, but set elsewhere */
    const bool                  IsDead()                { return m_killed; }
    const GVector&              GetVelocity()           { return (m_destiny != nullptr ? m_destiny->GetVelocity() : NULL_ORIGIN_V); }

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow);
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual void     MissileLaunched(Missile* pMissile) { /* Do nothing here */ }
    virtual void                UpdateDamage()          { /* Do nothing here */ }
    virtual bool                LoadExtras()            { return true; }
    // this will remove SE* from system and call Delete() on it's itemRef for non-containers.
    //caller MUST call SafeDelete() on SE after this returns.
    virtual void                Delete();

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void              SetPilot(Client* pClient) { /* Do nothing here */ }
    virtual bool                HasPilot()              { return false; }
    virtual Client*             GetPilot()              { return nullptr; }

protected:
    SystemBubble*               m_bubble;               /* we do not own this. never NULL in space */
    SystemManager*              m_system;               /* we do not own this  never NULL in space */
    TargetManager*              m_targMgr;              /* we do not own this. only Destructible items will have it */
    DestinyManager*             m_destiny;              /* we do not own this. only mobile items will have it */

    PyServiceMgr&               m_services;

    InventoryItemRef            m_self;

    bool                        m_killed;

    double                      m_radius;

    /* this is POS ForceField status */
    int32                       m_harmonic;

    /* ease of access to common data for ownable objects */
    int32                       m_warID;
    int32                       m_allyID;   // this is salvage factionID for npc wrecks
    uint32                      m_corpID;
    uint32                      m_fleetID;
    uint32                      m_ownerID;

};


/* Static / Non-Mobile / Non-Destructable / Celestial Objects
 * - Suns, Planets, Moons, Belts, Gates, Static NPC Stations
 *- no TargetMgr or DestinyMgr*/
class StaticSystemEntity : public SystemEntity {
public:
    StaticSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~StaticSystemEntity()                       { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual StaticSystemEntity* GetStaticSE()           { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                isGlobal()              { return true; }    // just in case item->isGlobal() fails here...which it may
    virtual bool                IsInanimateSE()         { return true; }
    /* Static */
    virtual bool                IsStaticEntity()        { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras();

};

class BeltSE
: public StaticSystemEntity
{
public:
    BeltSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~BeltSE()                                   { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual BeltSE*             GetBeltSE()             { return this; }
    /* class type tests. */
    virtual bool                IsBeltSE()              { return true; }

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras();

    /* generic access functions handled here */
    BeltMgr*                    GetBeltMgr()            { return m_beltMgr; }

    /* specific functions handled in this class. */
    void                   SetBeltMgr(BeltMgr* beltMgr) { m_beltMgr = beltMgr; }

protected:
    BeltMgr*                    m_beltMgr;

};

class StargateSE
: public StaticSystemEntity
{
public:
    StargateSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~StargateSE()                               { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual StargateSE*         GetGateSE()             { return this; }
    /* class type tests. */
    virtual bool                IsGateSE()              { return true; }

    /* SystemEntity interface */
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras();

    /* specific functions handled in this class. */
    StructureSE* GetMySBU()                             { return m_sbuSE; }
    bool HasSBU()                                       { return (m_sbuSE != nullptr); }
    void SetSBU(StructureSE* pSE)                       { m_sbuSE = pSE; }

protected:
    PyRep*                      m_jumps;
    StructureSE*                m_sbuSE;

};


/* Non-Static / Non-Mobile / Non-Destructible / Celestial Objects
 * - Containers, DeadSpace, ForceFields, ScanProbes
 *- no TargetMgr or DestinyMgr*/
class ItemSystemEntity : public SystemEntity {
public:
    ItemSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~ItemSystemEntity()                         { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual ItemSystemEntity*   GetItemSE()             { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                isGlobal()              { return false; }
    virtual bool                IsInanimateSE()         { return true; }
    /* Item */
    virtual bool                IsItemEntity()          { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );
    virtual void                MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict*             MakeSlimItem();

private:
    uint16 m_keyType;
};

/* POS ForceField */
class FieldSE
: public ItemSystemEntity
{
public:
    FieldSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~FieldSE()                             { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual FieldSE*            GetFieldSE()            { return this; }
    /* class type tests. */
    virtual bool                IsFieldSE()             { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );

    virtual PyDict*             MakeSlimItem();

};

/* Non-Static / Non-Mobile / Destructible / Celestial Objects
 * - POS Structures, Outposts, Deployables, empty Ships, Asteroids
 *- has TargetMgr  has DestinyMgr*/
class ObjectSystemEntity : public SystemEntity {
public:
    ObjectSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~ObjectSystemEntity();

    /* class type pointer querys. */
    virtual ObjectSystemEntity* GetObjectSE()           { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                isGlobal()              { return false; }
    virtual bool                IsInanimateSE()         { return true; }
    /* Object */
    virtual bool                IsObjectEntity()        { return true; }

    /* SystemEntity interface */
    virtual void                UpdateDamage();
    virtual void                EncodeDestiny( Buffer& into );
    virtual void                MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow);
    virtual bool                IsInvul()               { return m_invul; }

    /* specific functions handled here. */
    void                    SetInvul(bool invul=false)  { m_invul = invul; }

private:
    bool m_invul;
};

/* Mobile Warp Disruptors */
class DeployableSE
: public ObjectSystemEntity
{
public:
    DeployableSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~DeployableSE()                             { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual DeployableSE*       GetDeployableSE()       { return this; }
    /* class type tests. */
    virtual bool                IsDeployableSE()        { return true; }
};



/* Non-Static / Mobile / Destructible / Celestial Objects
 *- Drones, Ships, Missiles, Wrecks
 * - has TargetMgr and DestinyMgr*/
class DynamicSystemEntity : public SystemEntity {
public:
    DynamicSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~DynamicSystemEntity();

    /* class type pointer querys. */
    virtual DynamicSystemEntity* GetDynamicSE()         { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                isGlobal()              { return false; }
    /* Dynamic */
    virtual bool                IsDynamicEntity()       { return true; }

    /* SystemEntity interface */
    virtual void                UpdateDamage();
    virtual void                EncodeDestiny( Buffer& into );
    virtual void                MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual bool                Load()                  { return true; }
    virtual bool                IsInvul()               { return m_invul; }
    virtual bool                IsFrozen()              { return m_frozen; }


    /* specific functions handled here. */
    void                        AwardBounty(Client* pClient);
    void                    SetInvul(bool invul=false)  { m_invul = invul; }
    void                   SetFrozen(bool frozen=false) { m_frozen = frozen; }

private:
    bool m_invul;
    bool m_frozen;

};


#endif
