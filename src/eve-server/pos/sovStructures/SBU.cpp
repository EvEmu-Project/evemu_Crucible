
/**
 * @name SBU.cpp
 *   Class for Sovereignty Blockade Units.
 *
 * @Author:         James
 * @date:   8 April 2021
 */

/*
 * POS__ERROR
 * POS__WARNING
 * POS__MESSAGE
 * POS__DUMP
 * POS__DEBUG
 * POS__DESTINY
 * POS__SLIMITEM
 * POS__TRACE
 */


#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "planet/Planet.h"
#include "pos/sovStructures/SBU.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

SBUSE::SBUSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData)
: StructureSE(structure, services, system, fData)
{
}

void SBUSE::Init()
{
    StructureSE::Init();

    // set SBU in bubble
    if (m_bubble == nullptr)
        assert(0);
    m_bubble->SetSBUSE(this);
}


void SBUSE::Process()
{
    StructureSE::Process();
}
