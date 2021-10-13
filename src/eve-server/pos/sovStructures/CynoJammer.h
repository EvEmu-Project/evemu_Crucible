/**
 * @name CynoJammer.h
 *   Class for Cynosural Jammer Arrays
 *
 * @Author:         James
 * @date: 13 October 2021
 */

#ifndef EVEMU_POS_CYNOJAMMER_H_
#define EVEMU_POS_CYNOJAMMER_H_


#include "DataClasses.h"
#include "pos/Structure.h"


class CynoJammerSE
: public StructureSE
{
public:
    CynoJammerSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~CynoJammerSE();

    /* class type pointer querys. */
    virtual CynoJammerSE*          GetCynoJammerSE()          { return this; }

    /* class type tests. */
    virtual bool                IsCynoJammerSE()           { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();

};


#endif  // EVEMU_POS_CYNOJAMMER_H_
