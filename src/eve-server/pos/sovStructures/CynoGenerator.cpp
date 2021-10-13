/**
 * @name CynoGenerator.cpp
 *   Class for Cynosural Generator Arrays
 *
 * @Author:         James
 * @date: 13 October 2021
 */

#include "pos/sovStructures/CynoGenerator.h"

CynoGeneratorSE::CynoGeneratorSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

CynoGeneratorSE::~CynoGeneratorSE()
{
}

void CynoGeneratorSE::Init()
{
    StructureSE::Init();
}

void CynoGeneratorSE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
}