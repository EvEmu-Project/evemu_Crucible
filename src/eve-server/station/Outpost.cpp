/**
 * @name Outpost.cpp
 *   Class for Outposts.
 *
 * @Author:           James
 * @date:   17 October 2021
 */

#include "station/Outpost.h"

OutpostSE::OutpostSE(StationItemRef station, PyServiceMgr &services, SystemManager* system)
: StationSE(station, services, system) 
{
    // Create default dynamic attributes in the AttributeMap:
    station->SetAttribute(AttrOnline,             EvilOne, false);
    station->SetAttribute(AttrCapacity,           STATION_HANGAR_MAX_CAPACITY, false);
    station->SetAttribute(AttrInertia,            EvilOne, false);
    station->SetAttribute(AttrDamage,             EvilZero, false);
    station->SetAttribute(AttrShieldCapacity,     20000000.0, false);
    station->SetAttribute(AttrShieldCharge,       station->GetAttribute(AttrShieldCapacity), false);
    station->SetAttribute(AttrArmorHP,            station->GetAttribute(AttrArmorHP), false);
    station->SetAttribute(AttrArmorUniformity,    station->GetAttribute(AttrArmorUniformity), false);
    station->SetAttribute(AttrArmorDamage,        EvilZero, false);
    station->SetAttribute(AttrMass,               station->type().mass(), false);
    station->SetAttribute(AttrRadius,             station->type().radius(), false);
    station->SetAttribute(AttrVolume,             station->type().volume(), false);
}
