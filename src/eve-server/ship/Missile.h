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
    Author:     Allan
*/

#ifndef EVE_SHIP_MISSILE_H
#define EVE_SHIP_MISSILE_H

#include "system/SystemEntity.h"

class EVEServiceManager;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ShipItem;
class GenericModule;

class Missile
: public DynamicSystemEntity {
public:
    Missile(InventoryItemRef self, EVEServiceManager& services, SystemManager* pSystem, InventoryItemRef modRef, SystemEntity* tSE, SystemEntity* pSE, GenericModule* pMod=nullptr);
    virtual ~Missile()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual Missile* GetMissileSE()                     { return this; }
    /* Dynamic */
    virtual bool IsMissileSE()                          { return true; }

    /* SystemEntity interface */
    virtual void Delete();
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict *MakeSlimItem();

    /* specific functions handled here. */
    uint32 GetLauncherID()                              { return m_fromSE->GetID(); }
    SystemEntity* GetTargetSE()                         { return m_targetSE; }

    void SetHitTimer(uint32 setTime)                    { m_hitTimer.Start(setTime); }
    void SetSpeed(double speed)                         { m_speed = speed; }

    bool IsAlive()                                      { return m_alive; }
    bool IsOverloaded()                                 { return false; }

    double GetSpeed()                                   { return m_speed; }

protected:
    SystemEntity* m_targetSE;
    SystemEntity* m_fromSE;
    InventoryItemRef m_modRef;

    void HitTarget();
    void EndOfLife();

    Timer m_hitTimer;
    Timer m_lifeTimer;

    bool m_alive;

    uint32 m_orbitingID;

    float m_damageMod;

    double m_speed;
    double m_hullHP;

};

#endif  //EVE_SHIP_MISSILE_H


