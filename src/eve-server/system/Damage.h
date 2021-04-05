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
    Updates:    Allan
*/
#ifndef __DAMAGE_H_INCL__
#define __DAMAGE_H_INCL__

#include "PyServiceMgr.h"
#include "inventory/InventoryItem.h"
#include "system/SystemManager.h"

class Damage {
public:
    Damage(SystemEntity* pSE, InventoryItemRef wRef, float mod, uint16 eID);
    Damage(SystemEntity* pSE, InventoryItemRef wRef, InventoryItemRef cRef, uint16 eID);
    Damage(SystemEntity* pSE, InventoryItemRef wRef, float kin, float ther, float emp, float exp, float mod, uint16 eID);
    // constructor for Killed() methods of derived SystemEntity objects with no weapon
    Damage(SystemEntity *pSE, bool fatal_blow=false);

    ~Damage()                                           { /* do nothing here */ }

    float GetThermal()                                  { return thermal; }
    float GetEM()                                       { return em; }
    float GetKinetic()                                  { return kinetic; }
    float GetExplosive()                                { return explosive; }
    float GetModifier()                                 { return modifier; }
    float GetTotal() const                              { return (kinetic + thermal + em + explosive); }

    Damage MultiplyDup( float kinetic_multiplier,
                        float thermal_multiplier,
                        float em_multiplier,
                        float explosive_multiplier ) const
                        {   // NOTE:  remember, these come in BACKWARD from 'normal' fuzzy logic..  0=full and 1=none
                            // added checks here for > 95% resists, and < 1% to avoid crazy damage shit.
                            // also added checks for missing resists (some npcs have no hull resist in db which = 100% resist)
                            if (kinetic_multiplier > 1.0) kinetic_multiplier = 1.0;
                            if (kinetic_multiplier < 0.01) kinetic_multiplier = 0.01;
                            if (thermal_multiplier > 1.0) thermal_multiplier = 1.0;
                            if (thermal_multiplier < 0.01) thermal_multiplier = 0.01;
                            if (em_multiplier > 1.0) em_multiplier = 1.0;
                            if (em_multiplier < 0.01) em_multiplier = 0.01;
                            if (explosive_multiplier > 1.0) explosive_multiplier = 1.0;
                            if (explosive_multiplier < 0.01) explosive_multiplier = 0.01;
                            return Damage( srcSE, weaponRef,
                                           kinetic   * kinetic_multiplier,
                                           thermal   * thermal_multiplier,
                                           em        * em_multiplier,
                                           explosive * explosive_multiplier,
                                           modifier,
                                           effectID );
    }

    Damage &operator *=(float factor)
    {
        kinetic     *= factor;
        thermal     *= factor;
        em          *= factor;
        explosive   *= factor;

        return *this;
    }

    SystemEntity*           srcSE;     //we do not own this.
    uint16                  effectID;   // why is this here?
    InventoryItemRef        weaponRef;
    InventoryItemRef        chargeRef;    // May be null.

private:
    float kinetic;
    float thermal;
    float em;
    float explosive;
    float modifier;
};

#endif
