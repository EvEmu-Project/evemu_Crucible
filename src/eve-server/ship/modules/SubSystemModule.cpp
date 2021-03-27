
 /**
  * @name SubSystemModule.cpp
  *   SubSystem module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#include "ship/modules/SubSystemModule.h"


SubSystemModule::SubSystemModule(ModuleItemRef mRef, ShipItemRef sRef)
: PassiveModule(mRef, sRef)
{

}

int8 SubSystemModule::GetModulePowerLevel()
{
    return Module::Bank::Subsystem;
}

//not much to do here... this will be for t3 ships, which arent implented yet

//{'FullPath': u'UI/Messages', 'messageID': 257702, 'label': u'CannotFitSubSystemNotShipBody'}(u"You can't fit {subSystemName}. It only fits to {validShipName} but you're trying to fit it to {shipName}.", None, {u'{subSystemName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'subSystemName'}, u'{validShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'validShipName'}, u'{shipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipName'}})
