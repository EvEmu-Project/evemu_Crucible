
 /**
  * @name SubSystemModule.h
  *   SubSystem module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#ifndef _EVE_SHIP_SUBSYSTEM_MODULE_H
#define _EVE_SHIP_SUBSYSTEM_MODULE_H

#include "ship/modules/PassiveModule.h"

class SubSystemModule
: public PassiveModule
{
public:
    SubSystemModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~SubSystemModule()                          { /* do nothing here */ }

    virtual SubSystemModule*    GetSubSystemModule()    { return this; }

    virtual bool IsSubSystemModule() const              { return true; }

    int8 GetModulePowerLevel();
};

#endif  // _EVE_SHIP_SUBSYSTEM_MODULE_H
