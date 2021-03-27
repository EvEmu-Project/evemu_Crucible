
 /**
  * @name PassiveModule.cpp
  *   passive module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#include "eve-server.h"

#include "ship/modules/PassiveModule.h"

PassiveModule::PassiveModule(ModuleItemRef mRef, ShipItemRef sRef)
: GenericModule(mRef, sRef)
{
}

//  set up cargo expanders to expand specialized cargo on higher-tier ships


