
/**
 * @name Array.cpp
 *   Class for POS Array Modules.
 *
 * @Author:         Allan
 * @date:   28 December 17
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


#include "pos/Array.h"


ArraySE::ArraySE(StructureItemRef structure, EVEServiceManager& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

void ArraySE::Init()
{
    StructureSE::Init();
}

void ArraySE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
}

