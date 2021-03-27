/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
    Author:        Aknor Jaden
    Updates:    Allan
*/

#ifndef __DRONE__H__INCL__
#define __DRONE__H__INCL__

#include "Client.h"
#include "npc/DroneAI.h"
#include "system/SystemEntity.h"

/**
 * DynamicSystemEntity which represents drone object in space
 */

class PyServiceMgr;
class Item;
class DestinyManager;
class SystemManager;
class ServiceDB;
class NPCAIMgr;
class Damage;

class DroneSE
: public DynamicSystemEntity
{
public:
    DroneSE(InventoryItemRef drone, PyServiceMgr& services, SystemManager* pSystem, const FactionData& data);
    virtual ~DroneSE();

    /* class type pointer querys. */
    virtual DroneSE* GetDroneSE()                         { return this; }
    /* class type tests. */
    virtual bool IsDroneSE()                            { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict* MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);
    virtual void Abandon();     // reset all owner info and bubblecast new data

    virtual void TargetAdded(SystemEntity *who);
    virtual void TargetLost(SystemEntity *who);
    virtual void TargetedAdd(SystemEntity *who);
    virtual void TargetedLost(SystemEntity *who);

    /* specific functions handled here. */
    Client* GetOwner()                                  { return m_pClient; }
    DroneAIMgr* GetAI()                                 { return m_AI; }

    void Launch(ShipSE* pShipSE);           //add drone entity to system
    void Online(ShipSE* pShipSE=nullptr);         //  if nullptr sent, assign to controlling ship
    void Offline();
    void StateChange();
    //begin idle orbit around assigned ship
    void IdleOrbit(ShipSE* pShipSE=nullptr);

    void SaveDrone();
    void RemoveDrone();
    void SetResists();
    void SetOwner(Client* pClient);

    uint32 GetBounty() const                            { return (m_pClient == nullptr ? 0 : m_pClient->GetChar()->bounty()); }

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }
    float GetSecurityRating() const                     { return (m_pClient == nullptr ? 0.0 : m_pClient->GetChar()->GetSecurityRating()); }

    double GetOrbitRange()                              { return m_orbitRange; }

    /* for destiny setstate */
    uint8 GetState()                                    { return m_AI->GetState(); }
    uint32 GetControllerID()                            { return m_controllerID; }
    uint32 GetControllerOwnerID()                       { return m_controllerOwnerID; }
    uint32 GetTargetID()                                { return m_targetID/*m_targMgr->GetFirstTarget()->GetID()*/; }

    /* misc methods */
    void Enable()                                       { m_online = true; }
    void Disable()                                      { m_online = false; }
    bool IsEnabled()                                    { return m_online; }

    void AssignShip(ShipSE* pSE)                        { m_AI->AssignShip(pSE); }
    void SetTarget(SystemEntity* pSE = nullptr)         { (pSE == nullptr ? 0 : m_targetID = pSE->GetID()); }

    ShipSE* GetHomeShip()                               { return m_pShipSE; }

protected:
    Client* m_pClient;          //we do not own this
    DroneAIMgr* m_AI;           //we do own this
    ShipSE* m_pShipSE;            //we do not own this
    SystemManager* m_system;    //we do not own this

private:
    bool m_online;              // is drone within ship's control range?
    uint32 m_targetID;
    uint32 m_controllerID;
    uint32 m_controllerOwnerID;

    double m_orbitRange;
    double m_emDamage;
    double m_expDamage;
    double m_kinDamage;
    double m_therDamage;
    double m_hullDamage;
    double m_armorDamage;
    double m_shieldCharge;
    double m_shieldCapacity;
};

#endif /* !__DRONE__H__INCL__ */


