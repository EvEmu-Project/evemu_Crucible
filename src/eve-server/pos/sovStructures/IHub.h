/**
 * @name IHub.h
 *   Class for Infrastructure Hubs.
 *
 * @Author:         James
 * @date:   8 April 2021
 */


#ifndef EVEMU_POS_IHub_H_
#define EVEMU_POS_IHub_H_

#include "pos/Structure.h"

class IHubSE
: public StructureSE
{
public:
    IHubSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData);
    virtual ~IHubSE()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual IHubSE*             GetIHubSE()            { return this; }

    /* class type tests. */
    virtual bool                IsIHubSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();


protected:

private:


};

#endif  // EVEMU_POS_IHub_H_
