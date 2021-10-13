/**
 * @name CynoJammer.cpp
 *   Class for Cynosural Jammer Arrays
 *
 * @Author:         James
 * @date: 13 October 2021
 */

#include "pos/sovStructures/CynoJammer.h"

CynoJammerSE::CynoJammerSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

CynoJammerSE::~CynoJammerSE()
{
}

void CynoJammerSE::Init()
{
    StructureSE::Init();
}

void CynoJammerSE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
}