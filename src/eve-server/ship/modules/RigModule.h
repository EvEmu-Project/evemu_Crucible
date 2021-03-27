
 /**
  * @name RigModule.h
  *   rig module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#ifndef _EVE_SHIP_MODULES_RIG_MODULE_H_
#define _EVE_SHIP_MODULES_RIG_MODULE_H_

#include "ship/modules/PassiveModule.h"

class RigModule
: public PassiveModule
{
public:
    RigModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~RigModule()                                { /* do nothing here */ }

    virtual RigModule*          GetRigModule()          { return this; }

    bool                        IsRigModule() const     { return true; }

    int8 GetModulePowerLevel();

    //  not real sure what to do here yet.
    virtual void RemoveRig();
    virtual void DestroyRig();
};

#endif  // _EVE_SHIP_MODULES_RIG_MODULE_H_
