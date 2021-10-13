/**
 * @name CynoGenerator.h
 *   Class for Cynosural Generator Arrays
 *
 * @Author:         James
 * @date: 13 October 2021
 */

#ifndef EVEMU_POS_CYNOGENERATOR_H_
#define EVEMU_POS_CYNOGENERATOR_H_


#include "DataClasses.h"
#include "pos/Structure.h"


class CynoGeneratorSE
: public StructureSE
{
public:
    CynoGeneratorSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~CynoGeneratorSE();

    /* class type pointer querys. */
    virtual CynoGeneratorSE*          GetCynoGeneratorSE()          { return this; }

    /* class type tests. */
    virtual bool                IsCynoGeneratorSE()           { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();

};


#endif  // EVEMU_POS_CYNOGENERATOR_H_
