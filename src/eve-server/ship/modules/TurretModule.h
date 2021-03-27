
 /**
  * @name TurretModule.h
  *   turret module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#ifndef __EVESERVER_SHIPMODULES_ACTIVE_MODULES_TURRENTMODULE_H
#define __EVESERVER_SHIPMODULES_ACTIVE_MODULES_TURRENTMODULE_H

#include "ship/modules/ActiveModule.h"
#include "ship/modules/TurretFormulas.h"


class TurretModule : public ActiveModule
{
public:
    TurretModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~TurretModule()                            { /* do nothing here */ }

    virtual TurretModule*       GetTurretModule()       { return this; }
    //  class type helpers.  public for anyone to access.
    virtual bool                IsTurretModule()        { return true; }

    /* ActiveModule overrides */
    virtual void LoadCharge(InventoryItemRef charge);
    virtual void UnloadCharge();

    //  functions to be handled in derived classes as needed
    virtual void ApplyDamage();

protected:
    TurretFormulas m_formula;

    float m_crystalDmg;
    float m_crystalDmgAmount;
    float m_crystalDmgChance;

};


#endif  // __EVESERVER_SHIPMODULES_ACTIVE_MODULES_TURRENTMODULE_H