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
#ifndef __SYSTEMBUBBLE_H_INCL__
#define __SYSTEMBUBBLE_H_INCL__

#include <map>
#include <vector>

#include "eve-core.h"


class Client;
class SetState;
class PyTuple;
class ContainerSE;
class SystemEntity;
class SystemManager;
class Timer;
class TowerSE;
class TCUSE;
class SBUSE;
class IHubSE;
class DroneSE;
class PyObject;

class SystemBubble {
public:
    SystemBubble(SystemManager* pSystem, const GPoint& center, double radius);
    ~SystemBubble() noexcept;

    SystemEntity* const GetEntity(uint32 entityID) const;
    SystemManager* const GetSystem() const              { return m_system; }

    /* for spawn system     -allan 15July15 */
    void Process();
    void SetBelt(InventoryItemRef itemRef);
    void SetGate(uint32 gateID);
    void ResetBubbleRatSpawn();

    bool IsIce()                                        { return m_ice; }
    bool IsBelt()                                       { return m_belt; }
    bool IsGate()                                       { return m_gate; }
    bool IsAnomaly()                                    { return m_anomaly; }
    bool IsMission()                                    { return m_mission; }
    bool IsIncursion()                                  { return m_incursion; }
    bool IsSpawned()                                    { return m_spawned; }

    // these are set in spawnMgr.
    void SetAnomaly(bool set=true)                      { m_anomaly = set; }
    void SetMission(bool set=true)                      { m_mission = set; }
    void SetIncursion(bool set=true)                    { m_incursion = set; }
    void SetSpawned(bool set=true)                      { m_spawned = set; }
    void SetSpawnTimer(bool isBelt=false);

    /* various count queries */
    uint32 CountNPCs();
    uint32 CountPlayers()                               { return static_cast<uint32>(m_players.size()); }
    uint32 CountDynamics()                              { return static_cast<uint32>(m_dynamicEntities.size()); }

    /* used for bubble management */
    bool IsEmpty() const                                { return (m_entities.empty() ? m_dynamicEntities.empty() : false); }
    bool HasPlayers() const                             { return !m_players.empty(); }
    bool HasStatics() const                             { return !m_entities.empty(); }
    bool HasDynamics() const                            { return !m_dynamicEntities.empty(); }
    double x() const                                    { return m_center.x; }
    double y() const                                    { return m_center.y; }
    double z() const                                    { return m_center.z; }
    uint16 GetID()                                      { return m_bubbleID; }
    uint32 GetSystemID()                                { return m_systemID; }
    GPoint GetCenter()                                  { return m_center; }
    ContainerSE* GetCenterMarker()                      { return m_centerSE; }

    void clear();
    void PrintEntityList();

    void Add(SystemEntity* pSE);
    /**
     * Provides a means to remove an entity from the SystemBubble's map of
     * tracked dynamic entities without unsetting the SystemEntity's bubble.
     *
     * See `SystemBubble::Remove` for calling this and also unsetting the
     * `SystemEntity`'s bubble.
     */
    void Untrack(SystemEntity* pSE);
    void Remove(SystemEntity* pSE);
    void ProcessWander(std::vector< SystemEntity* >& wanderers);

    void SendAddBalls(SystemEntity* to_who);
    void SendAddBalls2(SystemEntity* to_who);
    void RemoveExclusive(SystemEntity* pSE);
    void AddBallExclusive(SystemEntity* about_who);

    //send a set of destiny events and updates to every client in the bubble.
    void BubblecastDestiny(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, const char* desc) const;
    //send a set of destiny updates to every client in the bubble.
    void BubblecastDestinyUpdate(std::vector<PyTuple*> &updates, const char* desc) const;
    //send a set of destiny events to every client in the bubble.
    void BubblecastDestinyEvent(std::vector<PyTuple*> &events, const char* desc) const;
    //send a destiny update to every client in the bubble.
    void BubblecastDestinyUpdate(PyTuple** payload, const char* desc) const;
    //send a destiny event to every client in the bubble.
    void BubblecastDestinyEvent(PyTuple** payload, const char* desc) const;
    void BubblecastSendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);
    //send a destiny update to every client in the bubble EXCLUDING the given SystemEntity 'pSE'
    void BubblecastDestinyUpdateExclusive(PyTuple** payload, const char* desc, SystemEntity* pSE) const;

    bool InBubble(const GPoint &pt, bool inWarp=false) const;
    bool IsOverlap(const GPoint &pt) const;
    void MarkCenter();
    void RemoveMarkers();

    /* for warp bubble checks */
    bool HasWarpBubble()                                { return m_hasBubble; }
    void SetWarpBubble(bool set=false)                  { m_hasBubble = set; }
    /* for SetState */
    void GetEntities(std::map< uint32, SystemEntity* >& into) const;    // this one only sends visible entities
    /* for ??? */
    void GetAllEntities(std::map< uint32, SystemEntity* >& into) const; // this one gets all entities regardless of visibility
    /* for targeting purposes */
    void GetPlayers(std::vector<Client*> &into) const;
    /* for scanning */
    void GetEntityVec(std::vector<SystemEntity*> &into) const;
    SystemEntity* GetRandomEntity();

    /* for towers/ship abandoning */
    bool HasTower()                                     { return (m_towerSE != nullptr); }
    TowerSE* GetTowerSE()                               { return m_towerSE; }
    void SetTowerSE(TowerSE* pTower)                    { m_towerSE = pTower; }

    /* for setting TCU in bubble */
    bool HasTCU()                                     { return (m_tcuSE != nullptr); }
    TCUSE* GetTCUSE()                               { return m_tcuSE; }
    void SetTCUSE(TCUSE* pTCU)                        { m_tcuSE = pTCU; }

    bool HasSBU()                                     { return (m_sbuSE != nullptr); }
    SBUSE* GetSBUSE()                               { return m_sbuSE; }
    void SetSBUSE(SBUSE* pSBU)                        { m_sbuSE = pSBU; }

    bool HasIHub()                                     { return (m_ihubSE != nullptr); }
    IHubSE* GetIHubSE()                               { return m_ihubSE; }
    void SetIHubSE(IHubSE* pIHub)                        { m_ihubSE = pIHub; }

    /* for system setstate */
    PyObject* GetDroneState() const;

    /* for command .syncloc - updates all players in bubble with positions of all dSE */
    void SyncPos();
    /* for command dropLoot - commands all npcs in bubble to jettison loot */
    void CmdDropLoot();

protected:
    const GPoint m_center;
    const double m_radius;

    // remove all balls in bubble for this SE
    void RemoveBall(SystemEntity* about_who);
    void RemoveBalls(SystemEntity* to_who);
    // remove this ball from bubble.  update all clients in bubble this SE has left.
    void RemoveBallExclusive(SystemEntity* about_who);

    void MarkBubble(const GPoint& position, std::string& name, std::string& desc, bool center=false);

private:
    TCUSE* m_tcuSE;
    SBUSE* m_sbuSE;
    IHubSE* m_ihubSE;
    TowerSE* m_towerSE;
    SystemManager* m_system;
    ContainerSE* m_centerSE;

    bool m_hasMarkers :1;
    bool m_hasBubble :1;       // for warp disruption bubbles (placeholder for later)

    uint16 m_bubbleID;
    uint32 m_systemID;

    std::map<uint32, Client*> m_players;                // testing with bubble player list (in std::map)
    std::map<uint32, SystemEntity*> m_markers;          // bubble marker cans.  we do own these.
    std::map<uint32, SystemEntity*> m_dynamicEntities;  //entities which may/may not move. we do not own these.
    std::map<uint32, SystemEntity*> m_entities;         //we do not own these.
    std::map<uint32, DroneSE*> m_drones;                //we do not own these.

    // for spawn system     -allan 15July15
    Timer m_spawnTimer;
    bool m_ice :1;
    bool m_belt :1;
    bool m_gate :1;
    bool m_anomaly :1;
    bool m_mission :1;
    bool m_incursion :1;
    bool m_spawned :1;
};

#endif
