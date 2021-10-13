/**
 * @name JumpBridge.cpp
 *   Class for Jump Bridges
 *
 * @Author:         James
 * @date: 13 October 2021
 */

#include "pos/sovStructures/JumpBridge.h"

JumpBridgeSE::JumpBridgeSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

JumpBridgeSE::~JumpBridgeSE()
{
}

void JumpBridgeSE::Init()
{
    StructureSE::Init();

}

void JumpBridgeSE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
}