
/**
 * @name Weapon.cpp
 *   Class for POS Weapon Modules.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */


#ifndef EVEMU_POS_WEAPON_H_
#define EVEMU_POS_WEAPON_H_


#include "pos/Structure.h"


class WeaponSE
: public StructureSE
{
public:
    WeaponSE(StructureItemRef structure, EVEServiceManager& services, SystemManager* system, const FactionData& data);
    virtual ~WeaponSE()                                 { /* do nothing here */ }

    /* class type pointer querys. */
    virtual WeaponSE*           GetWeaponSE()           { return this; }

    /* class type tests. */
    virtual bool                IsWeaponSE()            { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void Init();


};

#endif  // EVEMU_POS_WEAPON_H_
