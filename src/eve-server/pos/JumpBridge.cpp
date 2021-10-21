
/**
 * @name JumpBridge.cpp
 *   Class for POS JumpBridge Modules.
 *
 * @Author:           James
 * @date:   13 October 2021
 */

#include "pos/JumpBridge.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "system/SystemManager.h"

JumpBridgeSE::JumpBridgeSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

void JumpBridgeSE::Init()
{
    StructureSE::Init();
}

void JumpBridgeSE::Process()
{
    StructureSE::Process();
}

void JumpBridgeSE::SetOnline()
{
    _log(SOV__DEBUG, "Onlining Jump Bridge...");

    // Checks to see if bridge can be brought online

    // Make sure the system is not being jammed
    SovereigntyData sovData = svDataMgr.GetSovereigntyData(m_self->locationID());
    if (sovData.jammerID != 0) {
        throw CustomError("This system is currently being jammed.");
    }

    // Make sure player is not in high-sec (configurable)
    if (!sConfig.world.highSecCyno) {
        if (m_system->GetSecValue() >= 0.5f) {
            throw CustomError("This module may not be used in high security space.");
        }
    }

    StructureSE::SetOnline();
}

void JumpBridgeSE::SetOffline()
{
    _log(SOV__DEBUG, "Offlining TCU... Resetting claim's hubID.");
    StructureSE::SetOffline();
}
