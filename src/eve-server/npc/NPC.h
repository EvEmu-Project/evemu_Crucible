/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Updates:        Allan
*/


#ifndef __NPC_H_INCL__
#define __NPC_H_INCL__

#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/SystemEntity.h"

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class Missile;
class NPCAIMgr;
class SystemManager;

class NPC
: public DynamicSystemEntity
{
public:
    NPC(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr = nullptr);
    virtual ~NPC();

    /* class type pointer querys. */
    virtual NPC* GetNPCSE()                             { return this; }
    /* class type tests. */
    virtual bool IsNPCSE()                              { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void TargetLost(SystemEntity* who);
    virtual void TargetedAdd(SystemEntity* who);
    virtual void EncodeDestiny(Buffer& into);

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);
    virtual bool Load();  // sets orbit range and initalizes the AIMgr

    /* virtual functions to be overridden in derived classes */
    virtual void MissileLaunched(Missile* pMissile);  // tells AI a missle has been launched at us.  allows defender missile code

    /* specific functions handled here. */
    void SaveNPC();
    void RemoveNPC();
    void SetResists();
    void UseHullRepairer();
    void UseArmorRepairer();
    void UseShieldRecharge();
    void Orbit(SystemEntity* who);
    void ForceSetSpawner(SpawnMgr* spawnMgr)            { m_spawnMgr = spawnMgr; }

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }

    NPCAIMgr* GetAIMgr()                                { return m_AI; }
    SpawnMgr* GetSpawnMgr()                             { return m_spawnMgr; }

    /* for command dropLoot - commands all npcs in bubble to jettison loot */
    void CmdDropLoot();

protected:
    NPCAIMgr* m_AI;
    SpawnMgr* m_spawnMgr;

private:
    uint32 m_orbitingID;

    float m_emDamage;
    float m_expDamage;
    float m_kinDamage;
    float m_therDamage;
    float m_hullDamage;
    float m_armorDamage;
    float m_shieldCharge;
    float m_shieldCapacity;
};

#endif
