
/**
 * @name DataClasses.cpp
 *  data container classes that cannot be trivially constructed/destructed
 *
 * @author: allan
 * @date 4 January 2018
 */


#include "DataClasses.h"


ReactorData::ReactorData()
{
    Init();
}

ReactorData::~ReactorData()
{
    Clear();
}

void ReactorData::Clear()
{
    demands.clear();
    supplies.clear();
    connections.clear();
}

void ReactorData::Init()
{
    Clear();
    // not sure what 'default' init will be yet.
    active = false;
    itemID = 0;
    reaction = 0;
}

/*
PI_CCPin::PI_CCPin()
{
    Init();
}

PI_CCPin::~PI_CCPin()
{
    Clear();
}

void PI_CCPin::Clear()
{
    pins.clear();
    links.clear();
    plants.clear();
    routes.clear();
}

void PI_CCPin::Init()
{
    Clear();
    m_level = 0;
    m_ccPinID = 0;
    m_currentSimTime = 0;
}

*/

