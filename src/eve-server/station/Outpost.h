/**
 * @name Outpost.h
 *   Class for Outposts.
 *
 * @Author:           James
 * @date:   17 October 2021
 */

#ifndef EVEMU_POS_OUTPOST_H_
#define EVEMU_POS_OUTPOST_H_

#include "DataClasses.h"
#include "pos/Structure.h"
#include "station/Station.h"

// Class for Construction Platform (egg)
class PlatformSE
: public StructureSE
{
public:
    PlatformSE(StructureItemRef structure, PyServiceMgr &services, SystemManager *system, const FactionData &fData)
    : StructureSE(structure, services, system, fData)   { /* do nothing here */ }

    virtual ~PlatformSE()                               { /* do nothing here */ }

    /* class type pointer querys. */
    virtual PlatformSE*         GetPlatformSE()         { return this; }

    /* class type tests. */
    virtual bool                IsPlatformSE()          { return true; }
    virtual bool                isGlobal()              { return true; }
    virtual bool                IsOperSE()              { return true; }

};

// Class for Outpost (StationSE derivative)
class OutpostSE
: public StationSE
{
public:
    OutpostSE(StationItemRef station, PyServiceMgr &services, SystemManager* system);
    virtual ~OutpostSE()                                { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual OutpostSE* GetOutpostSE()                   { return this; }
    /* Static */
    virtual bool IsOutpostSE()                          { return true; }

};

#endif  // EVEMU_POS_OUTPOST_H_
