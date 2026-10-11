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
    Author:     Aknor Jaden
    Rewrite:    Allan
*/

#ifndef __DRONE__H__INCL__
#define __DRONE__H__INCL__

#include "Client.h"
#include "npc/DroneAI.h"
#include "system/SystemEntity.h"

/* A drone in space.
 *
 * A drone is owned by a character (ownerID) and may be controlled by a ship (controllerID, flown by controllerOwnerID).
 *  Normally owner and controller are the same pilot, unless control was delegated to a fleet member.
 *  Control is lost when the controlling ship leaves the system, docks, is destroyed or its pilot changes ship.
 *  The drone then stays in space, still owned, and its owner can reconnect to it.
 *
 * The client keeps a list of the drones each pilot controls (the SetState droneState rowset, kept up to date with
 *  OnDroneStateChange).  Drones it owns that are not in that list are offered for reconnection.
 *
 * Nothing here holds a pointer to the owner or controller; they are looked up by ID when needed.
 */

class EVEServiceManager;
class Item;
class DestinyManager;
class SystemManager;
class Damage;

// a drone's working numbers, from its type and the controlling pilot's skills
struct DroneStats {
    uint8 action;               // DroneAI::Action
    uint16 effectID;            // main effect (weapon, mining laser, repairer...)
    bool offensive;
    bool assistance;
    std::string effectGuid;
    double maxVelocity;         // approach speed
    double orbitVelocity;       // speed while orbiting a target or the ship
    double orbitRange;          // orbit distance from a target's surface
    double idleOrbitRange;      // orbit distance from the ship's surface
    double optimalRange;
    double falloff;
    double effectRange;         // max range to use the main effect
    double tracking;
    double sigResolution;
    double cycleTime;           // ms
    double damageMultiplier;
    double miningAmount;        // m3 per cycle
    double repairAmount;        // shield or armor HP per cycle
    double bandwidth;
};

class DroneSE
: public DynamicSystemEntity
{
public:
    DroneSE(InventoryItemRef drone, EVEServiceManager& services, SystemManager* pSystem, const FactionData& data);
    virtual ~DroneSE();

    /* class type pointer querys. */
    virtual DroneSE* GetDroneSE()                       { return this; }
    /* class type tests. */
    virtual bool IsDroneSE()                            { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict* MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &damage);
    virtual void Abandon();     // reset all owner info and bubblecast new data

    // from TargetManager
    void TargetLost(SystemEntity* who);
    void TargetedAdd(SystemEntity* who);

    /* control */
    bool IsControlled() const                           { return (m_controllerID != 0); }
    uint32 GetControllerID() const                      { return m_controllerID; }
    uint32 GetControllerOwnerID() const                 { return m_controllerOwnerID; }
    ShipSE* GetController();                            // nullptr if the controlling ship is gone
    Client* GetControllerClient();
    Client* GetOwnerClient();
    // takes control of the drone for this ship.  works out the drone's stats from the pilot's skills
    void SetController(ShipSE* pShip);
    // control lost.  the drone stops where it is and is dropped from the pilot's drone list
    void ReleaseControl();
    // tells the owner and controller about our current state
    void SendStateChange();

    // distance between our surface and the controlling ship's, or -1 if not controlled
    double ControllerDistance();
    double ControlRange();                              // controlling pilot's drone control range

    /* return to bay.  the AI asks for this when it reaches the ship; the system does the move after its tick */
    void RequestReturnToBay();
    // moves the drone into the controlling ship's drone bay.  sends errors to the pilot if it can't
    bool FinishReturnToBay();
    // moves the drone into this ship's hold (drone bay or cargo).  throws UserError/CustomError if it can't
    void ScoopTo(ShipSE* pShip, EVEItemFlags flag);

    DroneAIMgr* GetAI()                                 { return m_AI; }
    const DroneStats& Stats() const                     { return m_stats; }

    uint8 GetState()                                    { return m_AI->GetState(); }
    uint32 GetTargetID()                                { return m_AI->GetTargetID(); }

    uint32 GetBounty();
    float GetSecurityRating();

    /* drones controlled by a pilot or ship, in this system */
    static uint8 CountControlledBy(SystemManager* pSystem, uint32 charID);
    static double BandwidthUsedBy(SystemManager* pSystem, uint32 shipID);
    // a ship has left the system/docked/died/been left by its pilot: its drones lose control
    static void ReleaseDronesOf(SystemManager* pSystem, uint32 shipID);
    // a ship or drone was attacked.  tells the drones that should respond
    static void OnAggression(SystemEntity* pVictim, SystemEntity* pAttacker);
    // updates the ship's drone bandwidth load for the client
    static void UpdateBandwidthLoad(ShipSE* pShip);

protected:
    DroneAIMgr* m_AI;           //we do own this
    SystemManager* m_system;    //we do not own this

private:
    void UpdateStats(Client* pController);
    void SetResists();
    void SendStateChangeTo(Client* pClient, bool remove);

    uint32 m_controllerID;
    uint32 m_controllerOwnerID;
    bool m_returnRequested;

    DroneStats m_stats;
};

#endif /* !__DRONE__H__INCL__ */
