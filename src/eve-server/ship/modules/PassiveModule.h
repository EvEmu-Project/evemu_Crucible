
 /**
  * @name PassiveModule.h
  *   passive module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#ifndef _EVE_SHIP_MODULES_PASSIVE_MODULE_H_
#define _EVE_SHIP_MODULES_PASSIVE_MODULE_H_

#include "ship/modules/GenericModule.h"


class PassiveModule : public GenericModule
{
public:
    PassiveModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~PassiveModule() { }

    virtual PassiveModule*      GetPassiveModule()      { return this; }

    bool IsPassiveModule() const                        { return true; }

protected:

};

#endif  // _EVE_SHIP_MODULES_PASSIVE_MODULE_H_