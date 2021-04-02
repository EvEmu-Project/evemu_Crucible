
/**
 * @name Battery.h
 *   Class for POS Battery Modules.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */


#ifndef EVEMU_POS_BATTERY_H_
#define EVEMU_POS_BATTERY_H_


#include "pos/Structure.h"


class BatterySE
: public StructureSE
{
public:
    BatterySE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~BatterySE()                                { /* do nothing here */ }

    /* class type pointer querys. */
    virtual BatterySE*          GetBatterySE()          { return this; }

    /* class type tests. */
    virtual bool                IsBatterySE()           { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void Init();


};

#endif  // EVEMU_POS_BATTERY_H_
